## Type
Shared Identity Type #Shared_Identity_Type

## Purpose
Identifies a geometry definition stored in the GeometryDatabase, linking SDFs, physics colliders, and render meshes across all engines without duplicating geometry data or coupling engines together.

## Definition
```cpp
using GeometryID = uint32_t;
```

## Fields
| Field | Type       | Description |
| ----- | ---------- | ----------- |
| value | `uint32_t` |             |

## Produced By
- [[GeometryDatabase]]

## Consumed By
- [[PhysicsEngine]], [[HapticEngine]], [[RenderingEngine]]

## Invariants
- Immutable after publication
- Thread-safe
- Deterministic

## Related Structures
- [[ObjectID]]
