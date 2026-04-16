#include "hardware/SerialLink.h"
#include "hardware/Packets.h"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <string>
#include <thread>
#include <vector>

using Clock = std::chrono::steady_clock;

static constexpr uint16_t RATE_HEADER = 0xAA55;
static constexpr uint16_t PING_HEADER = 0xCC33;


// ------------------------------------------------------------
// Utility
// ------------------------------------------------------------

uint16_t computeChecksum(const void* data, size_t len) {
    const uint8_t* bytes = static_cast<const uint8_t*>(data);
    uint32_t sum = 0;
    for (size_t i = 0; i < len; ++i) {
        sum += bytes[i];
    }
    return static_cast<uint16_t>(sum & 0xFFFF);
}

template<typename T>
bool validateChecksum(const T& pkt) {
    T temp = pkt;
    temp.checksum = 0;
    uint16_t expected = computeChecksum(&temp, sizeof(T));
    return expected == pkt.checksum;
}

uint64_t nowNs() {
    return std::chrono::duration_cast<std::chrono::nanoseconds>(
        Clock::now().time_since_epoch()).count();
}

double nsToMs(uint64_t ns) {
    return static_cast<double>(ns) / 1e6;
}

double mean(const std::vector<double>& v) {
    if (v.empty()) return 0.0;
    double s = std::accumulate(v.begin(), v.end(), 0.0);
    return s / static_cast<double>(v.size());
}

double stddev(const std::vector<double>& v) {
    if (v.size() < 2) return 0.0;
    double m = mean(v);
    double accum = 0.0;
    for (double x : v) {
        double d = x - m;
        accum += d * d;
    }
    return std::sqrt(accum / static_cast<double>(v.size() - 1));
}

double percentile(std::vector<double> v, double p) {
    if (v.empty()) return 0.0;
    if (p <= 0.0) return *std::min_element(v.begin(), v.end());
    if (p >= 100.0) return *std::max_element(v.begin(), v.end());

    std::sort(v.begin(), v.end());
    double pos = (p / 100.0) * static_cast<double>(v.size() - 1);
    size_t idx = static_cast<size_t>(std::floor(pos));
    double frac = pos - static_cast<double>(idx);

    if (idx + 1 < v.size()) {
        return v[idx] * (1.0 - frac) + v[idx + 1] * frac;
    }
    return v[idx];
}

void printUsage() {
    std::cout << "\nUsage:\n"
              << "  serial_tests rate <COMx> <baud> <duration_sec> <csv_file>\n"
              << "  serial_tests rtt  <COMx> <baud> <count> <interval_ms> <csv_file>\n\n"
              << "Examples:\n"
              << "  serial_tests rate COM4 460800 30 rate_500hz.csv\n"
              << "  serial_tests rtt  COM4 460800 1000 2 rtt.csv\n\n";
}

// ------------------------------------------------------------
// Parser helpers
// ------------------------------------------------------------

bool tryParseRatePacket(std::vector<uint8_t>& buffer, RateTestPacket& outPkt) {
    const size_t pktSize = sizeof(RateTestPacket);

    while (buffer.size() >= pktSize) {
        // Search for header
        size_t start = 0;
        bool found = false;
        for (; start + 1 < buffer.size(); ++start) {
            uint16_t hdr = static_cast<uint16_t>(buffer[start])
                         | (static_cast<uint16_t>(buffer[start + 1]) << 8);
            if (hdr == RATE_HEADER) {
                found = true;
                break;
            }
        }

        if (!found) {
            // Keep at most 1 trailing byte in case header split across reads
            if (buffer.size() > 1) {
                buffer.erase(buffer.begin(), buffer.end() - 1);
            }
            return false;
        }

        // Drop junk before header
        if (start > 0) {
            buffer.erase(buffer.begin(), buffer.begin() + static_cast<long long>(start));
        }

        if (buffer.size() < pktSize) {
            return false; // wait for more bytes
        }

        std::memcpy(&outPkt, buffer.data(), pktSize);

        if (!validateChecksum(outPkt)) {
            // Bad packet, skip one byte and search again
            buffer.erase(buffer.begin());
            continue;
        }

        // Valid packet
        buffer.erase(buffer.begin(), buffer.begin() + static_cast<long long>(pktSize));
        return true;
    }

    return false;
}

bool tryParsePingPacket(std::vector<uint8_t>& buffer, PingPacket& outPkt) {
    const size_t pktSize = sizeof(PingPacket);

    while (buffer.size() >= pktSize) {
        size_t start = 0;
        bool found = false;
        for (; start + 1 < buffer.size(); ++start) {
            uint16_t hdr = static_cast<uint16_t>(buffer[start])
                         | (static_cast<uint16_t>(buffer[start + 1]) << 8);
            if (hdr == PING_HEADER) {
                found = true;
                break;
            }
        }

        if (!found) {
            if (buffer.size() > 1) {
                buffer.erase(buffer.begin(), buffer.end() - 1);
            }
            return false;
        }

        if (start > 0) {
            buffer.erase(buffer.begin(), buffer.begin() + static_cast<long long>(start));
        }

        if (buffer.size() < pktSize) {
            return false;
        }

        std::memcpy(&outPkt, buffer.data(), pktSize);

        if (!validateChecksum(outPkt)) {
            buffer.erase(buffer.begin());
            continue;
        }

        buffer.erase(buffer.begin(), buffer.begin() + static_cast<long long>(pktSize));
        return true;
    }

    return false;
}

// ------------------------------------------------------------
// Rate / packet integrity test
// ------------------------------------------------------------

int runRateTest(const std::string& port,
                int baud,
                int durationSec,
                const std::string& csvFile)
{
    SerialLink link;
    if (!link.connect(port, baud)) {
        std::cerr << "Failed to connect to " << port << "\n";
        return 1;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(300));

    std::ofstream csv(csvFile);
    if (!csv.is_open()) {
        std::cerr << "Failed to open CSV file: " << csvFile << "\n";
        return 1;
    }

    csv << "arrival_ns,seq,t_mcu_us,q1,q2,dt_arrival_ms,dropped_since_last,out_of_order\n";

    std::vector<uint8_t> parserBuffer;
    parserBuffer.reserve(8192);

    uint64_t startNs = nowNs();
    uint64_t endNs = startNs + static_cast<uint64_t>(durationSec) * 1000000000ULL;

    bool firstPacket = true;
    uint64_t lastArrivalNs = 0;
    uint32_t lastSeq = 0;

    uint64_t validPackets = 0;
    uint64_t checksumRejects = 0; // not directly counted in parser now
    uint64_t outOfOrderCount = 0;
    uint64_t droppedPackets = 0;

    std::vector<double> interArrivalMs;
    interArrivalMs.reserve(200000);

    std::cout << "Running rate test for " << durationSec << " s on " << port
              << " at " << baud << " baud...\n";

    while (nowNs() < endNs) {
        std::vector<uint8_t> rx;
        size_t n = link.readAvailable(rx, 4096);

        if (n > 0) {
            parserBuffer.insert(parserBuffer.end(), rx.begin(), rx.end());
        } else {
            std::this_thread::sleep_for(std::chrono::microseconds(200));
        }

        RateTestPacket pkt;
        while (tryParseRatePacket(parserBuffer, pkt)) {
            uint64_t arrivalNs = nowNs();

            int droppedThisPacket = 0;
            int outOfOrderThisPacket = 0;
            double dtMs = 0.0;

            if (firstPacket) {
                firstPacket = false;
            } else {
                dtMs = nsToMs(arrivalNs - lastArrivalNs);

                if (pkt.seq > lastSeq + 1) {
                    droppedThisPacket = static_cast<int>(pkt.seq - lastSeq - 1);
                    droppedPackets += static_cast<uint64_t>(droppedThisPacket);
                } else if (pkt.seq <= lastSeq) {
                    outOfOrderThisPacket = 1;
                    outOfOrderCount++;
                }

                interArrivalMs.push_back(dtMs);
            }

            csv << arrivalNs << ","
                << pkt.seq << ","
                << pkt.t_mcu_us << ","
                << pkt.q1 << ","
                << pkt.q2 << ","
                << std::fixed << std::setprecision(6) << dtMs << ","
                << droppedThisPacket << ","
                << outOfOrderThisPacket << "\n";

            lastArrivalNs = arrivalNs;
            lastSeq = pkt.seq;
            validPackets++;
        }
    }

    csv.close();

    double elapsedSec = nsToMs(nowNs() - startNs) / 1000.0;
    double recvRateHz = (elapsedSec > 0.0) ? (static_cast<double>(validPackets) / elapsedSec) : 0.0;
    double jitterMs = stddev(interArrivalMs);

    std::cout << "\n=== RATE TEST SUMMARY ===\n";
    std::cout << "Valid packets:       " << validPackets << "\n";
    std::cout << "Dropped packets:     " << droppedPackets << "\n";
    std::cout << "Out-of-order:        " << outOfOrderCount << "\n";
    std::cout << "Elapsed time (s):    " << elapsedSec << "\n";
    std::cout << "Received rate (Hz):  " << recvRateHz << "\n";
    std::cout << "Mean dt (ms):        " << mean(interArrivalMs) << "\n";
    std::cout << "Std dev dt (ms):     " << jitterMs << "\n";
    std::cout << "Min dt (ms):         " << (interArrivalMs.empty() ? 0.0 : *std::min_element(interArrivalMs.begin(), interArrivalMs.end())) << "\n";
    std::cout << "Max dt (ms):         " << (interArrivalMs.empty() ? 0.0 : *std::max_element(interArrivalMs.begin(), interArrivalMs.end())) << "\n";
    std::cout << "CSV saved to:        " << csvFile << "\n";

    (void)checksumRejects;
    return 0;
}

// ------------------------------------------------------------
// RTT ping-echo test
// ------------------------------------------------------------

int runRttTest(const std::string& port,
               int baud,
               int count,
               int intervalMs,
               const std::string& csvFile)
{
    SerialLink link;
    if (!link.connect(port, baud)) {
        std::cerr << "Failed to connect to " << port << "\n";
        return 1;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(300));

    std::ofstream csv(csvFile);
    if (!csv.is_open()) {
        std::cerr << "Failed to open CSV file: " << csvFile << "\n";
        return 1;
    }

    csv << "seq,send_ns,recv_ns,rtt_ms,success\n";

    std::vector<uint8_t> parserBuffer;
    parserBuffer.reserve(4096);

    std::vector<double> rttMsValues;
    rttMsValues.reserve(static_cast<size_t>(count));

    int successCount = 0;
    int timeoutCount = 0;

    std::cout << "Running RTT test: " << count << " pings on " << port
              << " at " << baud << " baud...\n";

    for (uint32_t seq = 0; seq < static_cast<uint32_t>(count); ++seq) {
        PingPacket pkt{};
        pkt.header = PING_HEADER;
        pkt.seq = seq;
        pkt.t_host_send_ns = nowNs();
        pkt.checksum = 0;
        pkt.checksum = computeChecksum(&pkt, sizeof(pkt));

        if (!link.sendRaw(&pkt, sizeof(pkt))) {
            std::cerr << "Failed to send ping seq " << seq << "\n";
            csv << seq << "," << pkt.t_host_send_ns << ",0,0,0\n";
            timeoutCount++;
            std::this_thread::sleep_for(std::chrono::milliseconds(intervalMs));
            continue;
        }

        bool gotReply = false;
        uint64_t deadlineNs = nowNs() + 200000000ULL; // 200 ms timeout

        while (nowNs() < deadlineNs) {
            std::vector<uint8_t> rx;
            size_t n = link.readAvailable(rx, 1024);
            if (n > 0) {
                parserBuffer.insert(parserBuffer.end(), rx.begin(), rx.end());
            } else {
                std::this_thread::sleep_for(std::chrono::microseconds(100));
            }

            PingPacket reply{};
            while (tryParsePingPacket(parserBuffer, reply)) {
                if (reply.seq == seq && reply.t_host_send_ns == pkt.t_host_send_ns) {
                    uint64_t recvNs = nowNs();
                    double rttMs = nsToMs(recvNs - pkt.t_host_send_ns);

                    csv << seq << ","
                        << pkt.t_host_send_ns << ","
                        << recvNs << ","
                        << std::fixed << std::setprecision(6) << rttMs << ",1\n";

                    rttMsValues.push_back(rttMs);
                    successCount++;
                    gotReply = true;
                    break;
                }
            }

            if (gotReply) {
                break;
            }
        }

        if (!gotReply) {
            csv << seq << "," << pkt.t_host_send_ns << ",0,0,0\n";
            timeoutCount++;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(intervalMs));
    }

    csv.close();

    std::cout << "\n=== RTT TEST SUMMARY ===\n";
    std::cout << "Success count:   " << successCount << "\n";
    std::cout << "Timeout count:   " << timeoutCount << "\n";
    std::cout << "Mean RTT (ms):   " << mean(rttMsValues) << "\n";
    std::cout << "Median RTT (ms): " << percentile(rttMsValues, 50.0) << "\n";
    std::cout << "p95 RTT (ms):    " << percentile(rttMsValues, 95.0) << "\n";
    std::cout << "p99 RTT (ms):    " << percentile(rttMsValues, 99.0) << "\n";
    std::cout << "Min RTT (ms):    " << (rttMsValues.empty() ? 0.0 : *std::min_element(rttMsValues.begin(), rttMsValues.end())) << "\n";
    std::cout << "Max RTT (ms):    " << (rttMsValues.empty() ? 0.0 : *std::max_element(rttMsValues.begin(), rttMsValues.end())) << "\n";
    std::cout << "CSV saved to:    " << csvFile << "\n";

    return 0;
}

// ------------------------------------------------------------
// Main
// ------------------------------------------------------------

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printUsage();
        return 1;
    }

    std::string mode = argv[1];

    if (mode == "rate") {
        if (argc != 6) {
            printUsage();
            return 1;
        }

        std::string port = argv[2];
        int baud = std::stoi(argv[3]);
        int durationSec = std::stoi(argv[4]);
        std::string csvFile = argv[5];

        return runRateTest(port, baud, durationSec, csvFile);
    }

    if (mode == "rtt") {
        if (argc != 7) {
            printUsage();
            return 1;
        }

        std::string port = argv[2];
        int baud = std::stoi(argv[3]);
        int count = std::stoi(argv[4]);
        int intervalMs = std::stoi(argv[5]);
        std::string csvFile = argv[6];

        return runRttTest(port, baud, count, intervalMs, csvFile);
    }

    printUsage();
    return 1;
}