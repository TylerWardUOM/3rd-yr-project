#include "hardware/DeviceAdapter.h"
#include <iostream>
#include <chrono>


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

DeviceAdapter::DeviceAdapter(msg::Channel<ToolStateMsg>& deviceIn, msg::Channel<HapticWrenchCmd>& deviceCmdOut, msg::Channel<DeviceTimingLogMsg>& timingLogOut)
    : deviceIn_(deviceIn),
      deviceCmdOut_(deviceCmdOut),
      timingLogOut_(timingLogOut) {}

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
    static int counter = 0;
    DeviceTimingLogMsg logMsg{};
    // Read From Firmware
    std::vector<uint8_t> chunk;
    size_t n = link_.readAvailable(chunk);
    if (n > 0) {
        incomingBuffer_.insert(incomingBuffer_.end(), chunk.begin(), chunk.end());
    }

    DeviceStatePacket pkt;
    DeviceStatePacket newestPkt;
    uint64_t newestRxParseNs = 0;
    bool gotState = false;

    // Parse everything currently buffered, keep only the newest valid packet
    while (tryParseOnePacket(pkt)) {
        newestPkt = pkt;
        newestRxParseNs = nowNs();
        gotState = true;
    }
    uint64_t toolPublishNs = 0;
    if (gotState) {
        latestAngles_[0] = newestPkt.joint_angle[0];
        latestAngles_[1] = newestPkt.joint_angle[1];

        if (counter++ % 100 == 0) { // Print every 100 updates to avoid spamming
            std::cout << "[DeviceAdapter] Updated joint angles: ("
                    << latestAngles_[0] << ", "
                    << latestAngles_[1] << ")\n";
        }

        Pose devicePose_ws = anglesToPose(latestAngles_);
        currentIn_.toolPose_ws = devicePose_ws;
        currentIn_.t_sec = timeNow;
        toolPublishNs = nowNs();

        deviceIn_.publish(currentIn_);
        logMsg.t_rx_parse_ns = newestRxParseNs;
        logMsg.t_tool_publish_ns = toolPublishNs;
        logMsg.q1 = latestAngles_[0];
        logMsg.q2 = latestAngles_[1];
    }
    
    // //Publish to tool in channel
    // deviceIn_.publish(currentIn_);
    //Moved to gotstate so might not need to publish if we didn't get new state?

    // Read From Haptics Buffers send to Firmware
    HapticWrenchCmd out;
    HapticWrenchCmd newestOut;
    bool gotCmd = false;

    while (deviceCmdOut_.tryConsume(out)) {
        newestOut = out;
        gotCmd = true;
    }

    if (gotCmd) {
        logMsg.t_wrench_consume_ns = nowNs();
        TorqueCommandPacket pkt_out{};
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
        pkt_out.joint_torque[1] = (float)tau2;
        if (counter % 10 == 0) { // Print every 100 updates to avoid spamming
            std::cout << "[DeviceAdapter] Sending torque command: ("
                    << pkt_out.joint_torque[0] << ", "
                    << pkt_out.joint_torque[1] << ")\n";
        }
        pkt_out.checksum = computeChecksum(&pkt_out, sizeof(TorqueCommandPacket) - 2);
        logMsg.t_tx_start_ns = nowNs();
        bool ok = link_.sendRaw(reinterpret_cast<uint8_t*>(&pkt_out), sizeof(TorqueCommandPacket));
        logMsg.t_tx_done_ns = nowNs();

        logMsg.tau1 = pkt_out.joint_torque[0];
        logMsg.tau2 = pkt_out.joint_torque[1];

        const bool matchedCycle = gotState && gotCmd && ok;

        if (matchedCycle) {
            timingLogOut_.publish(logMsg);
        }
        // if (ok) {
        //     timingLogOut_.publish(logMsg);
        // }

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