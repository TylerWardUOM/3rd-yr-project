#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

// Simple pose (position + orientation)
struct Pose {
    glm::dvec3 p{0.0,0.0,0.0};
    glm::dquat q{1.0,0.0,0.0,0.0};
};
