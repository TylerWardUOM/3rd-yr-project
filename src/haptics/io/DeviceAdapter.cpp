#include "haptics/io/DeviceAdapter.h"
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


DeviceAdapter::DeviceAdapter(HapticsBuffers& bufs)
    : bufs_(bufs) {}

bool DeviceAdapter::connect(const std::string& port, int baud) {
    return link_.connect(port, baud);
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
    // Read From Firmware
    std::vector<uint8_t> chunk;
    size_t n = link_.readAvailable(chunk);
    if (n>0){
        //std::cout << "[DeviceAdapter] Received " << n << " bytes from device." << std::endl;
        DeviceStatePacket pkt;
        while (parseIncoming(chunk, pkt)) {
            // Read joint angles (radians)
            latestAngles_[0] = pkt.joint_angle[0];
            latestAngles_[1] = pkt.joint_angle[1];
            chunk.clear();
            std::cout << "[DeviceAdapter] Joint angles: " << latestAngles_[0] << ", " << latestAngles_[1] << std::endl;
        }
        //std::cout << "[DeviceAdapter] Remaining buffer size: " << incomingBuffer_.size() << " bytes." << std::endl;
        // Compute Pose using forward kinematics
        Pose devicePose_ws = anglesToPose(latestAngles_);
        // Update ToolIn if new data
        currentIn_.devicePose_ws = devicePose_ws;
        currentIn_.latestAngles[0] = latestAngles_[0];
        currentIn_.latestAngles[1] = latestAngles_[1];
        currentIn_.t_sec = timeNow;
    }

    // Preserve the last refPose_ws from render/UI
    currentIn_.refPose_ws = bufs_.inBuf.read().refPose_ws;
    
    //write to in buffer
    bufs_.inBuf.write(currentIn_);


    // Read From Haptics Buffers send to Firmware
    ToolOut out = bufs_.outBuf.read();
    //if (out.t_sec > lastOut_.t_sec) { // new command
    if (true){
        //std::cout << "[DeviceAdapter] Sending force command: "
          //        << out.force_dev.x << ", " << out.force_dev.y << std::endl;
        // Prepare command
        TorqueCommandPacket pkt_out;
        pkt_out.joint_torque[0] = (float)out.force_dev.x;
        pkt_out.joint_torque[1] = (float)out.force_dev.y;
        pkt_out.checksum = computeChecksum(&pkt_out, sizeof(TorqueCommandPacket) - 2);
        
        link_.sendRaw(reinterpret_cast<uint8_t*>(&pkt_out), sizeof(TorqueCommandPacket));
        
        // Serialize and send command to device
        
        lastOut_ = out;
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