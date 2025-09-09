#pragma once
#include "world/Pose.h"
#include <cstdint>


using EntityId = uint32_t;

enum class SurfaceType : uint8_t { Plane, Sphere, TriMesh };

struct SurfacePlane { /* plane in local frame: n=(0,1,0), d=0 by convention */ };
struct SurfaceSphere { double radius = 0.5; };

// TriMesh is referenced by a mesh resource id (owned by scene/physics cookers)
using MeshId = uint32_t;

struct SurfaceDef {
    EntityId    id;             // entity this surface belongs to
    SurfaceType type;           // type of surface Plane/Sphere/TriMesh
    Pose        T_ws;           // world pose
    union {
        SurfacePlane plane;
        SurfaceSphere sphere;
    };
    MeshId      mesh = 0;       // used iff type==TriMesh
};
