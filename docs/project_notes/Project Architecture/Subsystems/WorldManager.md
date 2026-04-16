#Subsystem
## Overview
The WorldManager is the **authoritative owner of the virtual world state**.
It is responsible for creating, deleting, modifying, and tracking all world
objects and serves as the sole interface between high-level world commands
and the underlying physics simulation.

The WorldManager runs exclusively inside the [[PhysicsEngine]] thread to
guarantee determinism, thread safety, and consistent world updates.

---
## Responsibilities
- Create and destroy world objects in response to commands
- Maintain a registry of all active objects and their [[ObjectID]]s
- Bind each object instance to immutable geometry via [[GeometryID]]
- Apply user- and haptics-driven commands at safe physics boundaries
- Interface with the physics backend (e.g., PhysX)
- Produce the authoritative [[WorldSnapshot]] each physics tick

---

## Authority & Ownership
The WorldManager is the **only component** in the system allowed to:
- Allocate new ObjectIDs
- Remove ObjectIDs from the simulation
- Modify object transforms, velocities, and physical properties
- Insert or remove objects from the physics scene

All other engines may only *request* changes via commands.

---

## Data Structures Used
- [[ObjectID]]
- [[GeometryID]]
- [[WorldSnapshot]]
- [[CreateObjectCommand]]
- [[RemoveObjectCommand]]
- [[EditWorldCommand]]
- [[ForceCommand]]

---
## Interactions

### Receives Commands From
- [[RenderingEngine]]  
  (debug tools, UI-driven world edits)
- [[HapticEngine]]  
  (force feedback via ForceCommand)

### Publishes Data To
- [[HapticEngine]]  
- [[RenderingEngine]]  
via immutable [[WorldSnapshot]]s

---
## Command Handling Model
All incoming commands are processed **only during the physics update loop**.

Typical update sequence:
1. Drain command queues
2. Apply Create / Remove / Edit commands
3. Apply external forces (including haptic forces)
4. Step physics simulation
5. Build and publish a new WorldSnapshot

This guarantees:
- No mid-frame world mutation
- Deterministic behaviour
- Consistent snapshots across engines

---

## Object Lifecycle
1. A CreateObjectCommand is received
2. WorldManager:
   - Allocates a new ObjectID
   - Associates it with a GeometryID
   - Creates a corresponding physics body
3. Object appears in the next WorldSnapshot
4. On RemoveObjectCommand:
   - Physics body is destroyed
   - ObjectID is removed
   - Object disappears from subsequent snapshots

ObjectIDs are never reused within a simulation run.

---
## Geometry Integration

The WorldManager does not create or own geometry.

Instead:
- each world object references a [[GeometryID]]
- geometry definitions are resolved via the [[GeometryDatabase]]
- geometry creation is delegated to the [[GeometryFactory]]

On object creation:
- the WorldManager requests that geometry exists
- the GeometryFactory lazily constructs geometry if required
- the WorldManager stores only the GeometryID

This ensures:
- geometry is shared across objects
- WorldManager remains independent of geometry representations
- object lifecycle and geometry lifecycle are decoupled


---
## Never Allowed To
- Access haptic device hardware
- Perform SDF or implicit-surface collision detection
- Modify rendering state or UI elements
- Expose internal physics engine pointers to other threads
- Be accessed directly by Haptics or Rendering threads

---
## Design Invariants
- Single-threaded ownership (Physics thread only)
- Deterministic command application order
- Immutable snapshots after publication
- No shared mutable state with other engines

---
## Links
- [[PhysicsEngine]]
- [[GeometryDatabase]]
- [[WorldSnapshot]]
- [[ObjectID]]
- [[GeometryID]]
