#include "DeviceAdapter.h"
#include <iostream>
#include <chrono>

DeviceAdapter::DeviceAdapter(HapticsBuffers& bufs)
    : bufs_(bufs) {}

bool DeviceAdapter::connect(const std::string& port, int baud) {
    return link_.connect(port, baud);
}

void DeviceAdapter::update(double timeNow) {
    // Read From Firmware
    // Read joint angles (radians)
    // Compute Pose using forward kinematics

    // Convert to world-space pose (2-DOF planar)
    Pose devicePose_ws;

    // Update ToolIn if new data
    currentIn_.devicePose_ws = devicePose_ws;
    currentIn_.t_sec = timeNow;

    // Preserve the last refPose_ws from render/UI
    currentIn_.refPose_ws = bufs_.inBuf.read().refPose_ws;
    
    //write to in buffer
    bufs_.inBuf.write(currentIn_);

    // Read From Haptics Buffers

    ToolOut out = bufs_.outBuf.read();
    if (out.t_sec > lastOut_.t_sec) { // new command
        // Prepare command
        ForceCommand cmd;
        cmd.torque_cmd[0] = (float)out.force_dev.x;
        cmd.torque_cmd[1] = (float)out.force_dev.y;

        // Serialize and send command to device
        
        lastOut_ = out;
    }
}
