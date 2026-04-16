#pragma once
#include <optional>

struct PhysicsProps {
    bool  dynamic        = true;     // false => static
    bool  kinematic      = false;    // dynamic + kinematic moves via setKinematicTarget
    float density        = 1000.f;   // kg/m^3 (used if mass not provided)
    std::optional<double> mass;       // kg; if set, overrides density route

    double linDamping     = 0.05f;
    double angDamping     = 0.05f;

    double staticFriction = 0.6f;
    double dynamicFriction= 0.6f;
    double restitution    = 0.1f;
};

struct PhysicsPropsPatch {
    std::optional<bool>  dynamic;
    std::optional<bool>  kinematic;

    std::optional<float> density;
    std::optional<double> mass;

    std::optional<double> linDamping;
    std::optional<double> angDamping;

    std::optional<double> staticFriction;
    std::optional<double> dynamicFriction;
    std::optional<double> restitution;
};
