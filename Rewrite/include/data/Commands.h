// data/Commands.h
#pragma once
#include "core/Ids.h"
#include "core/Math.h"
#include <variant>
#include "data/WorldSnapshot.h" // for Role enum
#include "data/PhysicsProps.h"


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
    Colour   newColour{};
    bool     teleport{true}; // if false, you might set kinematic target instead
};

struct SetPhysicsPropsCommand {
    ObjectID id{};
    PhysicsProps props{};
};

struct PatchPhysicsPropsCommand {
    ObjectID id{};
    PhysicsPropsPatch patch{};
};

struct SetToolPoseCommand {
    ObjectID toolId{};     // ObjectID of the tool / proxy being controlled
    Pose     pose_ws{};    // Desired tool pose in world space
    double   t_sec{0.0};   // Timestamp (for sync / interpolation)
};


using WorldCommand = std::variant<
    CreateObjectCommand,
    RemoveObjectCommand,
    EditObjectCommand,
    SetPhysicsPropsCommand,
    PatchPhysicsPropsCommand
>;

using ForceCommandMsg = ForceCommand;
using ToolCommandMsg  = SetToolPoseCommand;
