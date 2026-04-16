// world/WorldDirty.h
#pragma once
#include <cstdint>

enum class WorldDirty : uint8_t {
    None     = 0,
    Topology = 1 << 0,  // objects created/removed, geometry changed
    Physics  = 1 << 1,  // mass, friction, kinematic flags
};

inline WorldDirty operator|(WorldDirty a, WorldDirty b) {

    return static_cast<WorldDirty>(static_cast<int>(a) | static_cast<int>(b));

}