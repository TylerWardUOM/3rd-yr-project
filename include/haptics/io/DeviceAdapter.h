#pragma once
#include "SerialLink.h"
#include "world/DoubleBuffer.h"
#include "haptics/ToolState.h"
#include "haptics/HapticBuffers.h"
#include "haptics/io/Packets.h"
#include "world/Pose.h"
#include <vector>

//Output command structure to device firmware
struct ForceCommand {
    float torque_cmd[2];  // torque per joint
};


/// DeviceAdapter connects firmware I/O to the simulation’s haptic buffers
class DeviceAdapter {
public:
    DeviceAdapter(HapticsBuffers& bufs);

    bool connect(const std::string& port, int baud = 115200);
    void update(double timeNow);

private:
    HapticsBuffers& bufs_;
    SerialLink link_;

    std::vector<uint8_t> incomingBuffer_;
    float latestAngles_[2];

    bool parseIncoming(std::vector<uint8_t>& newData, DeviceStatePacket& stateOut);

    Pose anglesToPose(const float jointAngles[2]);

    // Internal caching of latest data
    ToolIn currentIn_;
    ToolOut lastOut_;
};
