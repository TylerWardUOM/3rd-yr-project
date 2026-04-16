#Subsystem 
## Overview
The GeometryDatabase is a **centralized, read-only repository of geometric data**
used throughout the system.  
It provides a unified abstraction over multiple geometric representations of the
same shape, enabling Physics, Haptics, and Rendering to operate on shared geometry
without direct coupling.

Each geometry entry is identified by a [[GeometryID]] and may expose:
- an implicit surface or signed distance field (for haptics),
- a physics collision shape (for rigid-body simulation),
- a render mesh and material (for visualization).

The GeometryDatabase is immutable at runtime and may be safely accessed by all
engines concurrently.

---

## Responsibilities
- Store all geometry definitions used by the simulation
- Provide engine-specific geometry representations via GeometryID lookup
- Maintain consistency between SDFs, physics colliders, and render meshes
- Prevent duplication of geometric data across engines
- Enforce immutability of geometry during simulation runtime

---

## Geometry Entry Model
Each geometry entry represents a **shape definition**, not an object instance.

A single GeometryID may be referenced by many [[ObjectID]]s.

Conceptual structure:
```
GeometryEntry  
├── GeometryID  
├── SDFRepresentation (Haptics)  
├── PhysicsCollisionShape (PhysicsEngine)  
├── RenderMeshHandle (Renderer)
```


All representations correspond to the same underlying geometry.

---
## Data Structures Used
- [[GeometryID]]
- [[ObjectID]]
- Engine-specific geometry handles (opaque outside this subsystem)

---
## Interactions

### Provides Geometry To
- [[PhysicsEngine]]  
  (rigid-body collision shapes)
- [[HapticEngine]]  
  (implicit surfaces / SDFs for proxy collision)
- [[RenderingEngine]]  
  (render meshes and materials)

### Receives Data From
- Offline asset loaders
- Initialization routines
- Configuration files or asset manifests

The GeometryDatabase does **not** receive runtime updates from any engine.

---
## Access Model
- Geometry is accessed via GeometryID lookup
- Returned geometry objects are **read-only**
- No engine is permitted to modify stored geometry
- Geometry lookup must be thread-safe and lock-free

---
## Rendering Integration
The GeometryDatabase does not store GPU resources directly.

Instead, each GeometryEntry contains a lightweight
[[RenderMeshHandle]] that refers to a render-side asset
owned by the RenderingEngine.

This design ensures:
- GeometryDatabase remains renderer-agnostic
- OpenGL / GPU types are never shared across engines
- Rendering resources can be managed, reloaded, or destroyed independently

The mapping:
GeometryID → GeometryEntry → RenderMeshHandle → MeshGPU
is resolved exclusively inside the RenderingEngine.

---

## [[Signed Distance Field (SDF)]] Integration

Each GeometryEntry may expose a Signed Distance Field (SDF) representation.

SDFs:
- are defined in local geometry space
- are immutable and thread-safe
- are shared across all objects referencing the same GeometryID

World-space queries are performed by transforming query points using object poses
from [[WorldSnapshot]].

---
## Relationship to WorldManager
The GeometryDatabase does **not** manage object instances.

Instead:
- The [[WorldManager]] assigns a GeometryID when creating a new world object
- World objects reference geometry indirectly via GeometryID
- The GeometryDatabase remains independent of world state and simulation timing

This separation ensures:
- stable object lifecycles
- clear ownership boundaries
- safe multi-threaded access

---
## Lifetime & Mutability
- Geometry entries are created at initialization time
- Geometry is immutable for the duration of a simulation run
- GeometryIDs are stable and never reused or modified at runtime

Dynamic geometry (e.g. deformable objects) must be handled by a separate system
and is explicitly outside the scope of GeometryDatabase.

---

## Never Allowed To
- Store world-instance data (poses, velocities)
- Modify geometry at runtime
- Reference ObjectIDs directly
- Depend on physics engine internals
- Perform collision detection or simulation
- Store or reference MeshGPU, VAO, VBO, or other GPU resources
- Perform rendering or OpenGL calls

---

## Design Invariants
- Geometry is immutable
- GeometryDatabase is thread-safe
- One GeometryID maps to exactly one geometry definition
- Engines interact with geometry only through this subsystem

---

## Links
- [[GeometryID]]
- [[ObjectID]]
- [[WorldManager]]
- [[PhysicsEngine]]
- [[HapticEngine]]
- [[RenderingEngine]]
- [[RenderMeshHandle]]
- [[RenderMeshRegistry]]
