#pragma once
#include "env/env_interface.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class SphereEnv : public EnvInterface {
public:
    // Construct sphere in LOCAL coordinates: center c_in, radius r_in (>0)
    SphereEnv(const glm::dvec3& c_in, double r_in);

    // Signed distance in WORLD coordinates: phi(x) = ||x - c_world|| - r_world
    double phi(const glm::dvec3& x_world) const override;

    // Gradient in WORLD coordinates: unit outward normal
    glm::dvec3 grad(const glm::dvec3& x_world) const override;

    // Orthogonal projection of a WORLD point onto the sphere
    glm::dvec3 project(const glm::dvec3& x_world) const override;

    // Update world-space parameters using the given LOCAL->WORLD transform
    void update(const glm::mat4& world_from_local) override; // add override

private:
    // LOCAL-space definition
    glm::dvec3 c_local; // center (local)
    double     r_local; // radius  (local, > 0)

    // WORLD-space cached values (after update)
    glm::dvec3 c_world; // center (world)
    double     r_world; // radius (world)

    glm::mat4 M{}; // local -> world transform (cached)

};
