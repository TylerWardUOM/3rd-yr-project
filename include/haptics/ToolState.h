#pragma once
#include "world/Pose.h"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

/// @ingroup haptics
/// @brief Tool input state (from device and UI)
struct ToolIn {
    Pose devicePose_ws;  ///< device pose in world frame
    Pose refPose_ws;     ///< reference pose in world frame (from UI)
    double t_sec = 0.0;
};

/// @ingroup haptics
/// @brief Tool output state (to device and rendering)
struct ToolOut {
    Pose  proxyPose_ws;    ///< proxy pose in world frame
    glm::dvec3 force_dev;  ///< force to apply to device in device frame
    double t_sec = 0.0;
};


/// @ingroup haptics
/// @brief Haptics snapshot (for rendering and logging)
struct HapticSnapshot {
    Pose devicePose_ws; ///< device pose in world frame
    Pose refPose_ws;   ///< reference pose in world frame (from UI)
    Pose proxyPose_ws; ///< proxy pose in world frame
    glm::dvec3 force_ws;  ///< force applied to device in world frame
    double t_sec = 0.0; ///< timestamp in seconds
};
