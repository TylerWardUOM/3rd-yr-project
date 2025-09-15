// physics/PhysicsBuffers.h
#pragma once
#include "world/DoubleBuffer.h"
#include "physics/PhysicsCommands.h"

/// @ingroup physics
/// @brief Physics buffers: double buffer for commands from haptics to physics
struct PhysicsBuffers {
    DoubleBuffer<PhysicsCommands> cmdBuf; ///< commands from haptics to physics
};
