#pragma once
#include "hardware/SerialLink.h"

#include "messaging/Channel.h"
#include "data/HapticMessages.h"
#include "hardware/Packets.h"
#include "data/HapticMessages.h" // for ToolStateMsg and HapticWrenchCmd

#include "data/core/Math.h"// for Pose

#include <vector>



/// DeviceAdapter connects firmware I/O to the simulation’s haptic buffers
class DeviceAdapter {
public:
    DeviceAdapter(
        msg::Channel<ToolStateMsg>& deviceIn,
        msg::Channel<HapticWrenchCmd>& deviceCmdOut
    );

    bool connect(const std::string& port, int baud = 460800);
    void update(double timeNow);

private:
    msg::Channel<ToolStateMsg>& deviceIn_;
    msg::Channel<HapticWrenchCmd>& deviceCmdOut_;
    SerialLink link_;

    std::vector<uint8_t> incomingBuffer_;
    float latestAngles_[2];

    bool parseIncoming(std::vector<uint8_t>& newData, DeviceStatePacket& stateOut);
    
    bool tryParseOnePacket(DeviceStatePacket& stateOut);

    Pose anglesToPose(const float jointAngles[2]);

    // Internal caching of latest data
    ToolStateMsg currentIn_;
    HapticWrenchCmd lastOut_;
};