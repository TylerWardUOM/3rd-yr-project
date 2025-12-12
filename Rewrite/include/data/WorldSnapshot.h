// data/WorldSnapshot.h
#pragma once
#include "core/Ids.h"
#include "core/Math.h"
#include <vector>
#include <cstdint>

enum class Role : uint8_t { None=0, Tool=1, Proxy=2, Reference=3 };

struct ObjectState {
    ObjectID   id{};
    GeometryID geom{};
    Pose       T_ws{};
    Vec3       v_ws{0,0,0};
    Vec3       w_ws{0,0,0};
    Colour     colourOverride{0.8f,0.8f,0.8f};
    Role       role{Role::None};
};

struct WorldSnapshot {
    uint64_t seq{0};
    double   simTime{0.0};
    std::vector<ObjectState> objects;
};
