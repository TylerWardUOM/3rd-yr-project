# World System

Part of the [[Implementation_Index]].

## Quick Navigation

- [[#WorldManager]]
- [[#Object Storage]]
- [[#WorldCommand]]
- [[#WorldSnapshot]]
- [[#WorldDirty]]
- [[#Geometry Layer]]
- [[#SDF Interface]]
- [[#PhysicsProps]]

---

## WorldManager

`WorldManager` (`include/world/WorldManager.h`) is the **authoritative source of world state**.

It owns:
- a map of `WorldObject` records (id → object)
- a reference to `GeometryDatabase` (read-only)
- a reference to `GeometryFactory` (used on object creation)
- a reference to the world command channel (see [[Messaging]])

Key methods:
- `apply(WorldCommand)` — dispatch a command variant to the appropriate handler
- `step(dt)` — drain pending commands from the channel, apply them, advance sim time
- `buildSnapshot()` — produce a read-only `WorldSnapshot` for other threads
- `setPose(id, pose)` — called by [[Physics Engine]] to write back simulated positions
- `getPhysicsProps(id)` — read physics config for a given object
- `consumeDirty(flags)` — used by [[Physics Engine]] to detect topology/physics config changes

---

## Object Storage

Internally stored as:

```cpp
struct WorldObject {
    ObjectID   id;
    GeometryID geom;       // reference into GeometryDatabase
    Pose       pose;       // position + orientation + scale
    Colour     colour;
    Role       role;       // None / Tool / Proxy / Reference
    PhysicsProps physics;  // authoritative physics config
};
```

Objects are stored in an `unordered_map<ObjectID, WorldObject>`, keyed by auto-incrementing integer ID.

---

## WorldCommand

`WorldCommand` (`include/data/Commands.h`) is a `std::variant` of:

| Variant | Purpose |
|---|---|
| `CreateObjectCommand` | Spawn a new object with geometry, pose, colour, role |
| `RemoveObjectCommand` | Delete an object by ID |
| `EditObjectCommand` | Update pose and colour of an existing object |
| `SetPhysicsPropsCommand` | Replace all physics properties for an object |
| `PatchPhysicsPropsCommand` | Partial update of physics properties (uses `optional` fields) |

Commands are published to the `world.commands` channel (see [[Messaging]]) by the UI (see [[Rendering Engine#UI (ImGui)]]) or any other subsystem, and drained + applied during `WorldManager::step()`.

This keeps mutations serialised through the world command channel rather than having multiple threads mutating state directly.

---

## WorldSnapshot

`WorldSnapshot` (`include/data/WorldSnapshot.h`) is a value-copy of the scene state:

```cpp
struct WorldSnapshot {
    uint64_t seq;
    double   simTime;
    std::vector<ObjectState> objects;
};
```

Each `ObjectState` carries:
- `id`, `geom` — identity/geometry references
- `T_ws` (Pose) — world-space transform
- `v_ws`, `w_ws` — velocities (not yet populated from physics)
- `colourOverride` — render colour
- `role` — used by haptics to skip tool/proxy objects
- `physics` — current physics config (for UI display)

Snapshots are published to `world.snapshots` (a [[Messaging#SnapshotChannel]] — latest-value, double-buffered) after each simulation step.  
The [[Rendering Engine]] and [[Physics Engine]] read from this channel independently.

---

## WorldDirty

`WorldDirty` is a bitfield enum used to signal topology or physics property changes to the [[Physics Engine]]:

- `Topology` — objects added or removed → physics must rebuild actors
- `Physics` — physics props changed → physics must rebuild materials/actor config

`WorldManager::markDirty()` is set when create/remove/edit commands are applied.  
`PhysicsEnginePhysX::step()` calls `consumeDirty()` at the start of each tick and rebuilds actors if needed.

---

## Geometry Layer

The geometry layer sits between the world and the rendering/physics/haptics subsystems.

### GeometryEntry

`GeometryEntry` (`include/geometry/GeometryEntry.h`) bundles per-shape data:
- `id` — unique `GeometryID`
- `SurfaceType` — Plane / Sphere / Cube / TriMesh
- `sdf` — shared pointer to an [[#SDF Interface]] (used by [[Haptic Engine]])
- `physicsShape` — handle used by [[Physics Engine]]
- `renderMesh` — handle used by [[Rendering Engine#Geometry and Mesh Registries]]

### GeometryDatabase

`GeometryDatabase` is a registry: `registerGeometry()` at startup, `get(id)` and `getByType(type)` at runtime.

### GeometryFactory

`GeometryFactory` creates and registers the built-in geometry types (plane, sphere, cube) on first request.  
It lazily asks `RenderMeshRegistry` to create the GPU mesh and assembles the full `GeometryEntry`.

### Using geometry IDs

Objects only store a `GeometryID`, not pointers to geometry data.  
Each subsystem looks up geometry details from `GeometryDatabase` as needed.  
This decouples object records from rendering/physics implementation details.

---

## SDF Interface

`SDF` (`include/geometry/sdf/SDF.h`) is an abstract interface:

```cpp
struct SDFQuery { double phi; Vec3 grad; Vec3 proj; bool inside; };
class SDF {
    virtual SDFQuery queryLocal(const Vec3& p_ls) const = 0;
};
```

`phi` is the signed distance (negative = inside).  
`grad` is the gradient of the SDF (points outward).

Implementations:
- `PlaneSDF` — half-space: `phi = dot(n, p) + b`
- `UnitSphereSDF` — `phi = |p| - 1`
- `UnitCubeSDF` — box SDF

The [[Haptic Engine]] uses `queryLocal()` with a point transformed into object-local space to detect and respond to contact.

---

## PhysicsProps

`PhysicsProps` (`include/data/PhysicsProps.h`) stores per-object physics config:

| Field | Default | Meaning |
|---|---|---|
| `dynamic` | `true` | moveable vs. static |
| `kinematic` | `false` | pose set by code, not simulation |
| `density` | 1000 kg/m³ | used to compute mass from volume |
| `mass` | optional | overrides density route |
| `linDamping` | 0.05 | linear drag |
| `angDamping` | 0.05 | angular drag |
| `staticFriction` | 0.6 | |
| `dynamicFriction` | 0.6 | |
| `restitution` | 0.1 | bounciness |

`PhysicsPropsPatch` is the same struct but with all fields wrapped in `optional`, used for partial updates via `PatchPhysicsPropsCommand`.

Physics properties are read by [[Physics Engine]] when it creates or rebuilds PhysX actors.
