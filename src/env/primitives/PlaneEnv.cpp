#include "env/primitives/PlaneEnv.h"

PlaneEnv::PlaneEnv(const glm::dvec3& n_in, double b_in)
    : n_local(glm::normalize(n_in)), b_local(b_in),
      n_world(n_local), b_world(b_in) // default until update() is called
{
}

double PlaneEnv::phi(const glm::dvec3& x_world) const {
    return glm::dot(n_world, x_world) - b_world;
}

glm::dvec3 PlaneEnv::grad(const glm::dvec3& /*x_world*/) const {
    return n_world; // gradient is constant for a plane
}

glm::dvec3 PlaneEnv::project(const glm::dvec3& x_world) const {
    double t = phi(x_world);
    return x_world - n_world * t;
}

void PlaneEnv::update(const glm::mat4& world_from_local) {
    M = world_from_local;

    // --- Transform the normal ---
    glm::vec4 n_local_homogeneous(n_local, 0.0); // Homogeneous vector for normal
    glm::vec4 n_world_homogeneous = M * n_local_homogeneous;
    n_world = glm::normalize(glm::dvec3(n_world_homogeneous));

    // --- Recompute b_world using a transformed point ---
    glm::dvec3 p_local = n_local * b_local; // point on plane in LOCAL
    glm::vec4 p_local_homogeneous(p_local, 1.0); // Homogeneous point
    glm::vec4 p_world_homogeneous = M * p_local_homogeneous;
    glm::dvec3 p_world = glm::dvec3(p_world_homogeneous);

    b_world = glm::dot(n_world, p_world);
}
