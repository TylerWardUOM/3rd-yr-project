// physics/PhysicsBuffers.h
#pragma once
#include "world/DoubleBuffer.h"
#include "physics/PhysicsCommands.h"

struct PhysicsBuffers {
    DoubleBuffer<PhysicsCommands> cmdBuf; // written by haptics, read by physics
};
