## Type
Shared Identity Type

## Purpose
RenderMeshHandle is a lightweight identifier used by the [[RenderingEngine]]
to reference GPU mesh resources stored in the [[RenderMeshRegistry]].

It allows GeometryDatabase to reference renderable geometry without
depending on GPU or OpenGL types.

## Produced By
- [[RenderMeshRegistry]]

## Consumed By
- [[RenderingEngine]]

## Notes
- RenderMeshHandle has no meaning outside rendering
- It is opaque to Physics and Haptics
