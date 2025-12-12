// core/Math.h
#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

using Vec3   = glm::dvec3;
using Quat   = glm::dquat;
using Colour = glm::vec3;

struct Pose {
    Vec3 p{0.0,0.0,0.0};
    Quat q{1.0,0.0,0.0,0.0};
};
