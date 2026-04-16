## Status
Planned (not yet implemented)

## Role
The RenderingEngine is responsible for visualizing the simulated world
and providing user-facing debugging and inspection tools.

It consumes immutable world state from [[WorldSnapshot]] and never
modifies the simulation directly.

All world modifications requested via UI or debug tools are sent as
commands to the [[WorldManager]].

---

## Responsibilities
- Render world objects based on the latest WorldSnapshot
- Visualize object poses, geometry, and debug overlays
- Provide UI tools for inspection and interaction
- Convert user actions into Create / Edit / Remove commands

---

## Inputs
- [[WorldSnapshot]] (read-only)

---

## Outputs
- [[CreateObjectCommand]]
- [[EditWorldCommand]]
- [[RemoveObjectCommand]]

---

## Planned Geometry Resolution
For each object in the WorldSnapshot:
1. Read GeometryID from ObjectState
2. Query [[GeometryDatabase]] for geometry metadata
3. Resolve RenderMeshHandle via [[RenderMeshRegistry]]
4. Issue draw calls using renderer-owned GPU resources

This resolution is planned to occur entirely inside the RenderingEngine.

---
## Renderable Abstraction

Renderable objects are transient, frame-local draw packets constructed by the
RenderingEngine.

Renderable:
- binds a MeshGPU
- selects shader and visual parameters
- applies per-instance transforms
- is discarded after rendering

Renderable objects are not stored in the world state or geometry subsystems.

---

## Never Allowed To
- Modify world state directly
- Access physics engine internals
- Access haptic device state
- Own or define geometry data
- Expose GPU resources to other engines

---

## Design Notes
- RenderingEngine is intentionally decoupled from Physics and Haptics
- Rendering may run at an independent frequency
- Rendering may be moved to a separate process in the future

---

## Links
- [[WorldSnapshot]]
- [[WorldManager]]
- [[GeometryDatabase]]
- [[RenderMeshRegistry]]
