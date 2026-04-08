#include "hardware/DeviceAdapter.h"
#include <iostream>
#include <chrono>

#if (DEBUG_DEVICE_ADAPTER  == 0)

#define DEBUG_DEVICE_ANGLE_PRINT        0
#define DEBUG_DEVICE_ANGLE_RATE         100

#define DEBUG_DEVICE_TORQUE_PRINT       0
#define DEBUG_DEVICE_TORQUE_RATE        100

#define DEBUG_DEVICE_PARSE_PRINT        0
#define DEBUG_DEVICE_PARSE_RATE         100

#define DEBUG_DEVICE_TIMING_PRINT       0
#define DEBUG_DEVICE_TIMING_RATE        500

#endif

static uint16_t computeChecksum(const void* data, size_t len)
{
    const uint8_t* bytes = static_cast<const uint8_t*>(data);
    uint16_t sum = 0;
    for (size_t i = 0; i < len; ++i)
        sum += bytes[i];
    return sum;
}

static uint64_t nowNs()
{
    return std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::steady_clock::now().time_since_epoch()
    ).count();
}

DeviceAdapter::DeviceAdapter(msg::Channel<ToolStateMsg>& deviceIn, msg::Channel<HapticWrenchCmd>& deviceCmdOut, 
                            msg::Channel<DeviceTimingLogMsg>& timingLogOut, msg::Channel<DeviceStateLogMsg>& stateLogOut)
    : deviceIn_(deviceIn),
      deviceCmdOut_(deviceCmdOut),
      timingLogOut_(timingLogOut),
      stateLogOut_(stateLogOut) {}

bool DeviceAdapter::connect(const std::string& port, int baud) {
    return link_.connect(port, baud);
}

bool DeviceAdapter::tryParseOnePacket(DeviceStatePacket& stateOut) {
    while (incomingBuffer_.size() >= sizeof(DeviceStatePacket)) {
        // Search for header at the front
        if (!(incomingBuffer_[0] == 0xAA && incomingBuffer_[1] == 0x55)) {
            incomingBuffer_.erase(incomingBuffer_.begin());
            continue;
        }

        // We have enough bytes and the header looks right
        DeviceStatePacket pkt;
        memcpy(&pkt, incomingBuffer_.data(), sizeof(DeviceStatePacket));

        uint16_t chk = computeChecksum(&pkt, sizeof(DeviceStatePacket) - 2);

        if (chk == pkt.checksum) {
            stateOut = pkt;
            incomingBuffer_.erase(
                incomingBuffer_.begin(),
                incomingBuffer_.begin() + sizeof(DeviceStatePacket)
            );
            return true;
        } else {
            // Bad checksum: shift by one byte and rescan
            incomingBuffer_.erase(incomingBuffer_.begin());
        }
    }

    return false;
}

bool DeviceAdapter::parseIncoming(std::vector<uint8_t>& newData, DeviceStatePacket& stateOut){
    incomingBuffer_.insert(incomingBuffer_.end(), newData.begin(), newData.end());
    // Check if we have enough data for a full packet
    while (incomingBuffer_.size()>= sizeof(DeviceStatePacket)){
        // Look for packet header 0xAA55

        if (incomingBuffer_[0] == 0x55 && incomingBuffer_[1] == 0xAA) {
            // Shift one byte forward if misaligned
            incomingBuffer_.erase(incomingBuffer_.begin());
            continue;
        }

        if (incomingBuffer_[0] == 0xAA && incomingBuffer_[1] == 0x55){
            // Possible start of packet
            std::vector<uint8_t> pktBytes(incomingBuffer_.begin(),incomingBuffer_.begin()+sizeof(DeviceStatePacket));
            //check sum
            uint16_t chk = computeChecksum(pktBytes.data(), sizeof(DeviceStatePacket) - 2);

            DeviceStatePacket pkt;
            memcpy(&pkt, pktBytes.data(), sizeof(DeviceStatePacket));
            if (chk == pkt.checksum){
                stateOut = pkt;
                incomingBuffer_.erase(incomingBuffer_.begin(), incomingBuffer_.begin()+ sizeof(DeviceStatePacket));
                return true;
            }
            else{
                incomingBuffer_.erase(incomingBuffer_.begin());
            }
        }
        else{
            incomingBuffer_.erase(incomingBuffer_.begin());
        }

    } 
    return false;
}

void DeviceAdapter::update(double timeNow) {
    static uint64_t lastUpdateNs = 0;
    uint64_t now = nowNs();

    if (lastUpdateNs != 0) {
        double dtMs = (now - lastUpdateNs) * 1e-6;
        static int updPrint = 0;
        if (++updPrint % DEBUG_DEVICE_TIMING_RATE == 0 && DEBUG_DEVICE_TIMING_PRINT) {
            std::cout << "[DeviceAdapter] update dt = " << dtMs << " ms\n";
        }
    }
    lastUpdateNs = now;
    static int counter = 0;
    DeviceTimingLogMsg logMsg{};

    // Read from firmware
    std::vector<uint8_t> chunk;
    size_t n = link_.readAvailable(chunk);
    if (n > 0) {
        incomingBuffer_.insert(incomingBuffer_.end(), chunk.begin(), chunk.end());
    }

    DeviceStatePacket pkt{};
    DeviceStatePacket newestPkt{};
    uint64_t newestRxParseNs = 0;
    bool gotState = false;

    // Parse everything currently buffered, keep only newest valid packet
    int parsedCount = 0;
    while (tryParseOnePacket(pkt)) {
        uint64_t parseNs = nowNs();
        DeviceStateLogMsg stateMsg{};
        stateMsg.t_rx_parse_ns = parseNs;
        stateMsg.rx_state_seq = pkt.state_seq;
        stateMsg.state_mcu_us = pkt.t_mcu_us;
        stateMsg.q1 = pkt.joint_angle[0];
        stateMsg.q2 = pkt.joint_angle[1];

        stateLogOut_.publish(stateMsg);

        newestPkt = pkt;
        newestRxParseNs = parseNs;
        gotState = true;
        parsedCount++;
    }

    uint64_t toolPublishNs = 0;
    if (parsedCount > 0 && counter % DEBUG_DEVICE_PARSE_RATE == 0 && DEBUG_DEVICE_PARSE_PRINT) {
        std::cout << "[DeviceAdapter] parsed " << parsedCount
                << " packets this update, newest state_seq="
                << newestPkt.state_seq << "\n";
    }
    if (gotState) {
        latestAngles_[0] = newestPkt.joint_angle[0];
        latestAngles_[1] = newestPkt.joint_angle[1];

        latestStateSeq_ = newestPkt.state_seq;
        latestStateMcuUs_ = newestPkt.t_mcu_us;

        if (counter++ % DEBUG_DEVICE_ANGLE_RATE == 0 && DEBUG_DEVICE_ANGLE_PRINT) {
            std::cout << "[DeviceAdapter] Updated joint angles: ("
                      << latestAngles_[0] << ", "
                      << latestAngles_[1] << ")"
                      << " state_seq=" << latestStateSeq_
                      << " t_mcu_us=" << latestStateMcuUs_
                      << "\n";
        }

        Pose devicePose_ws = anglesToPose(latestAngles_);
        currentIn_.toolPose_ws = devicePose_ws;
        currentIn_.t_sec = timeNow;

        toolPublishNs = nowNs();
        deviceIn_.publish(currentIn_);

        logMsg.t_rx_parse_ns = newestRxParseNs;
        logMsg.t_tool_publish_ns = toolPublishNs;

        logMsg.rx_state_seq = newestPkt.state_seq;
        logMsg.state_mcu_us = newestPkt.t_mcu_us;

        logMsg.q1 = latestAngles_[0];
        logMsg.q2 = latestAngles_[1];

    }

    // Read newest haptic command
    HapticWrenchCmd out{};
    HapticWrenchCmd newestOut{};
    bool gotCmd = false;

    while (deviceCmdOut_.tryConsume(out)) {
        newestOut = out;
        gotCmd = true;
    }

    if (gotCmd) {
        logMsg.t_wrench_consume_ns = nowNs();

        TorqueCommandPacket pkt_out{};
        pkt_out.header[0] = 0xAA;
        pkt_out.header[1] = 0x55;

        // Command identity / matching
        pkt_out.cmd_seq = nextCmdSeq_++;
        pkt_out.ref_state_seq = latestStateSeq_;

        //TODO: verify this works as hasnt been tested as a function yet
        computeJacobiansAndTorques(latestAngles_, newestOut, pkt_out, logMsg);

        if (counter % DEBUG_DEVICE_TORQUE_RATE == 0 && DEBUG_DEVICE_TORQUE_PRINT) {
            std::cout << "[DeviceAdapter] Sending torque command: ("
                      << pkt_out.joint_torque[0] << ", "
                      << pkt_out.joint_torque[1] << ")"
                      << " cmd_seq=" << pkt_out.cmd_seq
                      << " ref_state_seq=" << pkt_out.ref_state_seq
                      << "\n";
        }

        pkt_out.checksum = computeChecksum(&pkt_out, sizeof(TorqueCommandPacket) - 2);

        logMsg.t_tx_start_ns = nowNs();
        bool ok = link_.sendRaw(reinterpret_cast<uint8_t*>(&pkt_out), sizeof(TorqueCommandPacket));
        logMsg.t_tx_done_ns = nowNs();

        logMsg.tx_cmd_seq = pkt_out.cmd_seq;
        logMsg.ref_state_seq = pkt_out.ref_state_seq;

        logMsg.tau1 = pkt_out.joint_torque[0];
        logMsg.tau2 = pkt_out.joint_torque[1];

        const bool matchedCycle = gotState && gotCmd && ok &&
                                  (logMsg.rx_state_seq == logMsg.ref_state_seq);

        if (matchedCycle) {
            timingLogOut_.publish(logMsg);
        }

        lastOut_ = newestOut;
    }
}

// Helper: Forward Kinematics for 2DOF planar arm
Pose DeviceAdapter::anglesToPose(const float jointAngles[2]){
    double L1 = 0.15;
    double L2 = 0.15;

    double t1 = jointAngles[0];
    double t2 = jointAngles[1];

    double x = L1 * cos(t1) + L2 * cos(t1 + t2);
    double y = L1 * sin(t1) + L2 * sin(t1 + t2);

    Pose p;
    p.p = glm::dvec3(x, y, 0.0);
    p.q = glm::angleAxis(t1 + t2, glm::dvec3(0, 0, 1));
    return p;
}

void DeviceAdapter::computeJacobiansAndTorques(const float jointAngles[2], const HapticWrenchCmd& newestOut, TorqueCommandPacket& pkt_out, DeviceTimingLogMsg& logMsg) {
    double q1 = latestAngles_[0];
    double q2 = latestAngles_[1];

    double L1 = 0.15;
    double L2 = 0.15;

    double J11 = -L1 * sin(q1) - L2 * sin(q1 + q2);
    double J12 = -L2 * sin(q1 + q2);
    double J21 =  L1 * cos(q1) + L2 * cos(q1 + q2);
    double J22 =  L2 * cos(q1 + q2);

    double Fx = newestOut.force_ws.x;
    double Fy = newestOut.force_ws.y;

    logMsg.fx = static_cast<float>(Fx);
    logMsg.fy = static_cast<float>(Fy);

    double tau1 = J11 * Fx + J21 * Fy;
    double tau2 = J12 * Fx + J22 * Fy;

    pkt_out.joint_torque[0] = -(float)tau1;
    pkt_out.joint_torque[1] =  (float)tau2;
}