// data/Commands.h
#pragma once
#include "core/Ids.h"
#include "core/Math.h"
#include <variant>

struct ForceCommand {
    ObjectID id{};
    Vec3 force_ws{0,0,0};
    Vec3 torque_ws{0,0,0};
};

struct CreateObjectCommand {
    GeometryID geom{};
    Pose       initialPose{};
    Colour     colour{0.8f,0.8f,0.8f};
    Role       role{Role::None};
    double     mass{1.0};
    bool       dynamic{true};
};

struct RemoveObjectCommand {
    ObjectID id{};
};

struct EditObjectCommand {
    ObjectID id{};
    Pose     newPose{};
    bool     teleport{true}; // if false, you might set kinematic target instead
};

using PhysicsCommand = std::variant<
    ForceCommand,
    CreateObjectCommand,
    RemoveObjectCommand,
    EditObjectCommand
>;
