#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "world/World.h"

/// @ingroup physics
/// @brief Physics command: apply a force and torque (wrench) at a point on a body for a duration
struct ApplyWrenchAtPoint {
    World::EntityId body;    ///< body to apply wrench to
    glm::dvec3      F_ws;    ///< force in world
    glm::dvec3      p_ws;    ///< point of application in world
    double          duration_s; ///< duration to apply force (seconds)
    double          t_sec; ///< timestamp in seconds (for logging)
};

/// @ingroup physics
/// @brief Physics commands: a list of commands to apply this timestep
struct PhysicsCommands {
    std::vector<ApplyWrenchAtPoint> wrenches; ///< list of wrenches to apply
};
