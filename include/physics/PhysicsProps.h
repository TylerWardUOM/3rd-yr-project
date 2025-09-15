#pragma once
#include <optional>

struct PhysicsProps {
    bool  dynamic        = true;     // false => static
    bool  kinematic      = false;    // dynamic + kinematic moves via setKinematicTarget
    float density        = 1000.f;   // kg/m^3 (used if mass not provided)
    std::optional<float> mass;       // kg; if set, overrides density route

    float linDamping     = 0.05f;
    float angDamping     = 0.05f;

    float staticFriction = 0.6f;
    float dynamicFriction= 0.6f;
    float restitution    = 0.1f;
};
