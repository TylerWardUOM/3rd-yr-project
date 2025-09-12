#include "physics/PhysicsEngine.h"
#include "physics/PhysicsCommands.h"
#include <chrono>
#include <cmath>
#include <iostream>

PhysicsEngine::PhysicsEngine(World& world, PhysicsBuffers& pbufs)
    : world_(world), pbufs_(pbufs) {}

void PhysicsEngine::step(double dt) {
    // 1) Read commands from haptics
    PhysicsCommands cmds = pbufs_.cmdBuf.read();

    // 2) Apply them to our “toy” backend (position nudges)
    for (const auto& c : cmds.wrenches) {
        applyForceAtPoint(c.body, c.F_ws, c.p_ws, c.duration_s);
    }

    // 3) (No real dynamics yet)
    integrate(dt);

    // 4) Publish updated world snapshot for viz/haptics
    world_.publishSnapshot(now());
}

void PhysicsEngine::applyForceAtPoint(World::EntityId id,
                                      const glm::dvec3& F_ws,
                                      const glm::dvec3& /*p_ws*/,
                                      double duration_s)
{
    // Convert force into a small displacement in its direction.
    // dp = mobility * F * duration
    glm::dvec3 dp = linMobility_ * F_ws * duration_s;

    // Clamp displacement to avoid teleports
    const double len2 = glm::dot(dp, dp);
    if (len2 > maxStep_ * maxStep_) {
        const double len = std::sqrt(len2);
        dp *= (maxStep_ / std::max(1e-12, len));
    }

    // Nudge the entity’s pose in the world
    // (assumes World::translate(id, dp) moves the T_ws.p by dp)
    std::cout << "PhysicsEngine: moving entity " << id << " by (" << dp.x << ", " << dp.y << ", " << dp.z << ")\n";
    world_.translate(id, dp);
}

void PhysicsEngine::integrate(double /*dt*/) {
    // No-op for now. Later: step PhysX/Bullet and write poses back into World.
}

double PhysicsEngine::now() const {
    using clock = std::chrono::steady_clock;
    const auto t = clock::now().time_since_epoch();
    return std::chrono::duration<double>(t).count();
}
