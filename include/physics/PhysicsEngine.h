#pragma once
#include "world/World.h"
#include "physics/PhysicsBuffers.h"
#include <glm/glm.hpp>

class PhysicsEngine {
public:
    explicit PhysicsEngine(World& world, PhysicsBuffers& pbufs);

    // Step the physics simulation by dt seconds:
    // - consumes haptics->physics commands
    // - applies them to the backend
    // - integrates dynamics
    // - writes back poses to World and publishes a fresh snapshot
    void step(double dt);

private:
    World&           world_;
    PhysicsBuffers&  pbufs_;


    // Simple hack parameters (position-only “physics”)
    double linMobility_ = 1e-2;  // [m / (N·s)]  -> tune to taste
    double maxStep_     = 0.05;  // [m] per step clamp

    // Backend hooks you will implement/wrap
    void applyForceAtPoint(World::EntityId id,
                           const glm::dvec3& F_ws,
                           const glm::dvec3& p_ws,
                           double duration_s);

    void integrate(double dt);
    double now() const;
};
