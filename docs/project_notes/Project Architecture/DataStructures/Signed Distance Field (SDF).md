## Overview
A Signed Distance Field (SDF) represents an implicit surface defined by a scalar
function φ(x), where:

- φ(x) < 0 inside the surface
- φ(x) = 0 on the surface
- φ(x) > 0 outside the surface

SDFs are used primarily by the [[HapticEngine]] for proxy-based collision and
constraint enforcement.

---
## Coordinate Convention
All SDFs operate in **local geometry space.**

**World-space queries must be transformed into local space using object pose data**
supplied via [[WorldSnapshot]].

---

## Interface
Each SDF provides:

- signed distance evaluation
- surface normal estimation
- projection onto the implicit surface

---

## Projection
Projection is defined in local space as
``` cpp
x_proj = x - φ(x) ∇φ(x)
```


This formulation is geometry-agnostic and applies to planes, spheres, and mesh
SDFs.

---

## Design Invariants
- SDFs are immutable
- SDFs do not store world transforms
- SDFs do not depend on simulation state
- One SDF instance may be shared by many objects

---

## Links
- [[GeometryFactory]]
- [[HapticEngine]]
- [[WorldSnapshot]]
