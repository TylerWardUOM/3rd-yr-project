#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "world/World.h"

// Command to apply a force and torque (wrench) at a point on a body for a duration
struct ApplyWrenchAtPoint {
    World::EntityId body;    // target entity (dynamic)
    glm::dvec3      F_ws;    // force in world
    glm::dvec3      p_ws;    // application point in world
    double          duration_s;
    double          t_sec;
};

struct PhysicsCommands {
    std::vector<ApplyWrenchAtPoint> wrenches;
};
