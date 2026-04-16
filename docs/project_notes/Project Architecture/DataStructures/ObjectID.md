## Type
Shared Identity Type #Shared_Identity_Type 

## Purpose
ObjectID is a unique, stable identifier for a **world object instance**.
It decouples all engines by allowing objects to be referenced without sharing pointers, PhysX handles, or memory addresses.

## Definition
```cpp
using ObjectID = uint32_t;
```

## Fields
| Field | Type       | Description                                   |
| ----- | ---------- | --------------------------------------------- |
| value | `uint32_t` | Unique identifier for a world object instance |

## Produced By
- [[WorldManager]]

## Consumed By
- [[PhysicsEngine]], [[HapticEngine]], [[RenderingEngine]]

## Invariants
- Unique across the entire simulation
- Assigned only by WorldManager
- Stable across all WorldSnapshots
- Never reused within a single simulation run

## Related Structures
- [[ObjectID]]
- [[GeometryID]]
- [[CreateObjectCommand]]
- [[RemoveObjectCommand]]

## Notes
- ObjectID must never encode meaning (type, geometry, ownership)
- ObjectID is an identity handle, not a data container
- Enables safe cross-thread and cross-process communication