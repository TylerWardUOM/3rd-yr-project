## Status
Planned (not yet implemented)

## Overview
RenderMeshRegistry is a rendering-side asset registry responsible for
mapping lightweight [[RenderMeshHandle]] identifiers to concrete GPU
mesh resources.

It allows geometry definitions to reference renderable assets without
introducing rendering or GPU dependencies into other subsystems.

---

## Responsibilities
- Own all MeshGPU objects
- Verify mesh existence for requested geometry
- Create GPU meshes lazily if required
- Prevent duplicate GPU resource creation
- Manage GPU resource lifetimes

---

## Relationship to GeometryDatabase
The GeometryDatabase stores RenderMeshHandle values but does not own
rendering resources.

The planned resolution chain is:
GeometryID → GeometryDatabase → RenderMeshHandle → RenderMeshRegistry → GPU Mesh

When a geometry requests a mesh:

1. GeometryFactory requests a mesh for a geometry type
2. RenderMeshRegistry checks whether a mesh already exists
3. If not, the registry creates and uploads a MeshGPU
4. A RenderMeshHandle is returned and cached

This ensures deterministic, minimal GPU usage.

---

## Never Allowed To
- Be accessed by PhysicsEngine or HapticEngine
- Store world-instance data (poses, velocities)
- Perform geometry queries (SDF, collision)
- Modify geometry definitions

---

## Links
- [[GeometryDatabase]]
- [[MeshGPU]]
- [[RenderMeshHandle]]
- [[RenderingEngine]]
