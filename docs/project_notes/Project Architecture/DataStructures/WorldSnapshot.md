## Type
Snapshot #Snapshot

## Purpose
WorldSnapshot represents the **authoritative, read-only state of the world**
at a single physics timestep.

It is the *only* mechanism by which non-physics engines
(Haptics, Rendering) observe the simulated world.

WorldSnapshot defines:
- thread safety
- determinism
- engine decoupling

## Definition
```cpp
struct ObjectState {
    ObjectID id;
    GeometryID geometry;
    Transform transform;
    Vector3 linearVelocity;
    Vector3 angularVelocity;
};

struct WorldSnapshot {
    uint64_t sequenceNumber;
    double simulationTime;
    std::vector<ObjectState> objects;
};
```

## Fields
| Field          | Type       | Description                       |
| -------------- | ---------- | --------------------------------- |
| sequenceNumber | `uint64_t` | Monotonic snapshot version        |
| simulationTime | `double`   | Physics time at snapshot creation |
| objects        | `vector`   | State of all active world objects |
## Produced By
- [[WorldManager]] (inside [[PhysicsEngine]])

## Consumed By
- [[HapticEngine]] [[RenderingEngine]]

## Invariants
- Immutable after publication
- Thread-safe
- Deterministic

## Related Structures
- [[ObjectID]]
