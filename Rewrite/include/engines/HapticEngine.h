// haptics/HapticEngine.h
#pragma once
#include "messaging/Channel.h"
#include "data/WorldSnapshot.h"
#include "data/HapticMessages.h"
#include "geometry/GeometryDatabase.h"
#include "messaging/SnapshotChannel.h"
// #include "haptics/io/DeviceAdapter.h"

// Dummy DeviceAdapter for illustration purposes
class DeviceAdapter {
public:
    DeviceAdapter() = default;

    bool connect(const std::string& port, int baud) {
        // Simulate successful connection
        return true;
    }

    void sendWrenchCommand(const HapticWrenchCmd& cmd) {
        // Simulate sending wrench command to device
    }

    HapticSnapshotMsg readSnapshot() {
        // Simulate reading snapshot from device
        return HapticSnapshotMsg{};
    }
};

class HapticEngine {
public:
    HapticEngine(const GeometryDatabase& geomDb,
                msg::SnapshotChannel<WorldSnapshot>& worldSnaps,
                 msg::Channel<ToolStateMsg>& toolIn,
                 msg::Channel<HapticSnapshotMsg>& hapticOut,
                 msg::Channel<HapticWrenchCmd>& wrenchOut);

    void run();        // 1 kHz loop
    void update(float dt);

    bool connectDevice(const std::string& port, int baud = 115200);

private:
    msg::SnapshotChannel<WorldSnapshot>&      worldSnaps_;
    msg::Channel<ToolStateMsg>&       toolIn_;
    msg::Channel<HapticSnapshotMsg>&  hapticOut_;
    msg::Channel<HapticWrenchCmd>&    wrenchOut_;

    DeviceAdapter deviceAdapter_;
    const GeometryDatabase& geometryDb_;

    // cached latest inputs (so update() is deterministic)
    uint64_t worldSnapVersion_ = 0;
    WorldSnapshot latestWorld_{};
    ToolStateMsg  latestTool_{};

    Pose proxyPosePrev_{};
};
