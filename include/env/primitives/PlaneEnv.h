#pragma once
#include "env/env_interface.h"   // still needed for EnvInterface and Matrix4

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


class PlaneEnv : public EnvInterface {
public:
    // Construct plane in LOCAL coordinates: unit normal n_in, offset b_in
    PlaneEnv(const glm::dvec3& n_in, double b_in);

    // Signed distance in WORLD coordinates
    double phi(const glm::dvec3& x_world) const override;

    // Gradient of phi in WORLD coordinates (unit normal)
    glm::dvec3 grad(const glm::dvec3& /*x_world*/) const override;

    // Orthogonal projection of a WORLD point onto the plane
    glm::dvec3 project(const glm::dvec3& x_world) const override;

    // Update world-space parameters using the given transform
    void update(const glm::mat4& world_from_local);

private:
    // LOCAL-space definition
    glm::dvec3 n_local; // unit normal in local space
    double     b_local; // offset (plane eq: dot(n_local, x_local) = b_local)

    // WORLD-space cached values (after update)
    glm::dvec3 n_world; // unit normal in world space
    double     b_world; // offset (plane eq: dot(n_world, x_world) = b_world)

    glm::mat4 M{};        // local -> world transform (cached)
};
