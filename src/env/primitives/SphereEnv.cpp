#include "env/primitives/SphereEnv.h"
#include <algorithm>
#include <cmath>



SphereEnv::SphereEnv(const glm::dvec3& c_in, double r_in)
    : c_local(c_in), r_local(std::max(0.0, r_in)), c_world(c_in), r_world(std::max(0.0, r_in)), M(1.0)
{
}

double SphereEnv::phi(const glm::dvec3& x_world) const {
    return glm::length(x_world - c_world) - r_world;
}

glm::dvec3 SphereEnv::grad(const glm::dvec3& x_world) const {
    glm::dvec3 v = x_world - c_world;
    double L = glm::length(v);
    if (L > 1e-12) return v / L;     // outward unit normal
    // Gradient undefined at center; return any stable unit (e.g., +X)
    return glm::dvec3(1.0, 0.0, 0.0);
}

glm::dvec3 SphereEnv::project(const glm::dvec3& x_world) const {
    glm::dvec3 v = x_world - c_world;
    double L = glm::length(v);
    if (L < 1e-12) {
        // Arbitrary direction if at the center
        return c_world + glm::dvec3(r_world, 0.0, 0.0);
    }
    return c_world + (r_world / L) * v;
}



void SphereEnv::update(const glm::mat4& world_from_local) {
    M = world_from_local;

    // Transform center as a point (w=1)
    glm::dvec4 c4 = M * glm::dvec4(c_local, 1.0);
    c_world = glm::dvec3(c4);

    r_world = r_local; // Note: uniform scaling only; non-uniform not supported
    
}
