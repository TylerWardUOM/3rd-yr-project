// core/Ids.h
#pragma once
#include <cstdint>

using ObjectID   = uint32_t;
using GeometryID = uint32_t;
using RenderMeshHandle = uint32_t;   // engine-agnostic handle
using PhysicsShapeHandle = uint32_t; // engine-agnostic handle (your PhysX wrapper)
