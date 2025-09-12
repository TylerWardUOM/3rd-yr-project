#pragma once
#include "world/Pose.h"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>


struct ToolIn {
    Pose devicePose_ws;  // from microcontroller
    Pose refPose_ws;     // device pose in world frame/ input from mouse for debug
    double t_sec = 0.0;
};

struct ToolOut {
    Pose  proxyPose_ws;    // for viz; optional for device
    glm::dvec3 force_dev;  
    double t_sec = 0.0;
};

struct HapticSnapshot {
    Pose devicePose_ws;
    Pose refPose_ws;
    Pose proxyPose_ws;
    glm::dvec3 force_ws;   // for overlays/plots
    double t_sec = 0.0;
};
