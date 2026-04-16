// data/HapticsMessages.h
#pragma once
#include "data/core/Math.h"
#include "data/WorldSnapshot.h"

struct ToolStateMsg {
    Pose   toolPose_ws{};
    Vec3   toolVel_ws{0,0,0};   // optional
    bool   button{false};       // optional
    double t_sec{0.0};
};

struct HapticSnapshotMsg {
    Pose devicePose_ws{};
    Pose proxyPose_ws{};
    Vec3 force_ws{0,0,0};
    double t_sec{0.0};
};

// What haptics outputs (to physics OR directly to device)
struct HapticWrenchCmd {
    ObjectID targetId{};         // 0 if “device only”
    Vec3     force_ws{0,0,0};
    Vec3     torque_ws{0,0,0};
    Vec3     point_ws{0,0,0};    // optional contact point
    double   duration_s{0.0};
    double   t_sec{0.0};
};
