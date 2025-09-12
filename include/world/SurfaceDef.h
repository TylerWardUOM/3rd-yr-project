#pragma once
#include "world/Pose.h"
#include <cstdint>


using EntityId = uint32_t;
using Colour = glm::vec3; // RGB

enum class Role : uint8_t { None=0, Tool=1, Proxy=2, Reference=3 };
enum class SurfaceType : uint8_t { None, Plane, Sphere, TriMesh };

struct SurfacePlane { /* plane in local frame: n=(0,1,0), d=0 by convention */ };
struct SurfaceSphere { double radius = 0.5; };

// TriMesh is referenced by a mesh resource id (owned by scene/physics cookers)
using MeshId = uint32_t;

struct SurfaceDef {
    EntityId    id;             // entity this surface belongs to
    SurfaceType type;           // type of surface Plane/Sphere/TriMesh
    Pose        T_ws;           // world pose
    Colour      colour = {0.8f,0.8f,0.8f}; //  colour
    union {
        SurfacePlane plane;
        SurfaceSphere sphere;
    };
    MeshId      mesh = 0;       // used iff type==TriMesh
    Role        role = Role::None; // special role in haptics
};
