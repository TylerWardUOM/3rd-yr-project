#Subsystem

## Overview
The GeometryFactory is responsible for constructing and registering all geometry
definitions used by the system.

It replaces legacy helper functions such as `createPlane()` and `createSphere()`
with a centralized, extensible mechanism that preserves clean architectural
boundaries.

---

## Responsibilities
- Create analytic geometry (planes, spheres, etc.)
- Instantiate Signed Distance Fields (SDFs)
- Assign render mesh handles
- Register geometry definitions into the [[GeometryDatabase]]
- Cache geometry so it is created exactly once

---

## Public API
Typical usage:

``` cpp
GeometryID plane = geometryFactory.getPlane();  
GeometryID sphere = geometryFactory.getSphere(radius);
```


These calls:
- return stable GeometryIDs
- lazily create geometry if it does not yet exist
- reuse existing geometry definitions when available

---

## Geometry Creation Policy
GeometryFactory encapsulates all geometry construction logic.

Examples:
- planes are associated with PlaneSDF
- spheres are associated with SphereSDF(radius)
- future mesh geometry may be loaded from assets

GeometryFactory never creates object instances.

---
## Interaction with GeometryDatabase
- GeometryFactory constructs [[GeometryDatabase#Geometry Entry Model|Geometry Entry]] objects
- [[GeometryDatabase]] stores geometry immutably
- Geometry entries are registered exactly once

---

## Interaction with Rendering
GeometryFactory assigns a [[RenderMeshHandle]] to each geometry entry.

Actual GPU mesh creation is delegated to the [[RenderMeshRegistry]], which:
- verifies whether a mesh already exists
- creates GPU resources lazily if required

---

## Design Invariants
- GeometryFactory is the only system that creates geometry
- GeometryFactory does not own GPU resources
- GeometryFactory does not store world-instance data
- Geometry creation is explicit and deterministic

---

## Links
- [[GeometryDatabase]]
- [[GeometryID]]
- [[RenderMeshRegistry]]
- [[SDF]]
