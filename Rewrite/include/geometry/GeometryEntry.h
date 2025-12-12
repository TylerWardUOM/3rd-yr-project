#pragma once
#include "data/core/Ids.h"
#include <memory>

enum class SurfaceType : uint8_t {
    None,
    Plane,
    Sphere,
    TriMesh
};

// Forward-declared interfaces / opaque handles
class SDF;

using PhysicsShapeHandle = uint32_t;   // opaque
using RenderMeshHandle   = uint32_t;   // opaque

struct GeometryEntry {
    GeometryID id{};
    SurfaceType type{SurfaceType::None};

    // Haptics
    std::shared_ptr<const SDF> sdf;

    // Physics
    PhysicsShapeHandle physicsShape{0};

    // Rendering
    RenderMeshHandle renderMesh{0};
};
