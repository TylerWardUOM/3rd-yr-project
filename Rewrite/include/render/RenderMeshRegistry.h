#pragma once

#include "render/gpu/MeshGPU.h"
#include <unordered_map>
#include <cstdint>

using RenderMeshHandle = uint32_t;

enum class MeshKind : uint8_t {
    Plane,
    Sphere,
    Cube,
    TriMesh
};

class RenderMeshRegistry {
public:
    RenderMeshRegistry() = default;

    // GeometryFactory-facing API
    RenderMeshHandle getOrCreate(MeshKind kind);

    // RenderingEngine-facing API
    const MeshGPU* get(RenderMeshHandle handle) const;

private:
    RenderMeshHandle createMesh(MeshKind kind);

private:
    RenderMeshHandle nextHandle_{1};

    // Handle → GPU mesh
    std::unordered_map<RenderMeshHandle, MeshGPU> meshes_;

    // MeshKind → Handle (deduplication)
    std::unordered_map<MeshKind, RenderMeshHandle> kindToHandle_;
};
