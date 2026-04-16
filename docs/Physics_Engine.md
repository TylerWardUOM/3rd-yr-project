# Physics Engine

Part of the [[Implementation_Index]].

## Quick Navigation

- [[#Overview]]
- [[#Initialisation]]
- [[#Building Actors]]
- [[#Per-Step Pipeline]]
- [[#Impulse Application]]
- [[#Integration with World System]]

---

## Overview

`PhysicsEnginePhysX` (`Rewrite/include/engines/PhysicsEnginePhysX.h`) wraps NVIDIA PhysX to provide rigid-body simulation.

It is driven on the simulation thread (see [[Implementation_Index#Architecture Overview]]) and called from `simulationLoop()` in `main.cpp`.

Key design decisions:
- Physics does **not** own world state — it reads from `WorldManager` and writes poses back
- Fixed sub-step accumulator for deterministic simulation independent of frame rate
- Actors are rebuilt when `WorldDirty` flags indicate topology or property changes (see [[World System#WorldDirty]])
- Haptic impulses arrive via the `haptics.wrenches` channel and are applied before each simulation step

---

## Initialisation

`initPhysX_()` runs in the constructor:

1. `PxCreateFoundation` — allocator + error callback
2. `PxCreatePhysics` — main SDK object
3. `PxSceneDesc` — set gravity `(0, -9.81, 0)`, 2-thread CPU dispatcher, CCD enabled
4. `physics_->createScene()` — simulation scene
5. Create default `PxMaterial` (static/dynamic friction 0.6, restitution 0.1)

After init, `buildActorsFromWorld_()` creates PhysX actors for all current world objects.

---

## Building Actors

`buildActorsFromWorld_()` rebuilds all actors from scratch when called:

1. Clear existing actors and materials
2. Call `wm_.buildSnapshot()` to get current world state
3. For each `ObjectState`:
   - look up geometry type from `GeometryDatabase`
   - create a `PxMaterial` from the object's `PhysicsProps` (per-object friction/restitution)
   - create the PhysX actor:

| Geometry | Dynamic | Actor |
|---|---|---|
| Plane | always static | `PxRigidStatic` + large thin `PxBoxGeometry` (1000×0.01×1000) |
| Sphere | true | `PxRigidDynamic` + `PxSphereGeometry`, radius = `obj.T_ws.s` |
| Sphere | false | `PxRigidStatic` + `PxSphereGeometry` |
| Cube | true | `PxRigidDynamic` + `PxBoxGeometry`, half-extent = `0.5 * obj.T_ws.s` |
| Cube | false | `PxRigidStatic` + `PxBoxGeometry` |

Additional actor config for dynamic bodies:
- `setLinearDamping`, `setAngularDamping` from `PhysicsProps`
- mass/inertia: use `p.mass` if provided, else `PxRigidBodyExt::updateMassAndInertia` from density
- CCD enabled (`eENABLE_CCD`) to prevent tunnelling at high velocities
- solver iterations: 8 position / 2 velocity

All actors are stored in `actors_` (map from `ObjectID` to `PxRigidActor*`).

---

## Per-Step Pipeline

`step(double dt)` runs each simulation thread tick:

1. **Check dirty flags** — `wm_.consumeDirty(Topology | Physics)` — if set, call `rebuildActors()`
2. **Consume inputs** — `consumeInputsOnce_()`:
   - drain `haptics.wrenches` channel
   - for each `HapticWrenchCmd`: compute impulse = `force * duration`, call `applyImpulseAtPoint_`
3. **Fixed sub-step** — `simulateFixed_(dt)`:
   - accumulate dt
   - while `accumulator >= fixedDt` (default 1/240 s = 240 Hz):
     - `scene_->simulate(fixedDt)`
     - `scene_->fetchResults(true)` — block until done
     - `accumulator -= fixedDt`
4. **Write-back poses** — `writeBackPoses_()`:
   - iterate dynamic actors only
   - read `PxRigidDynamic::getGlobalPose()`
   - call `wm_.setPose(id, pose)` to update `WorldManager`

After `step()` returns, `WorldManager::buildSnapshot()` will include the updated poses in the next snapshot published to the render/haptic threads.

---

## Impulse Application

`applyImpulseAtPoint_(id, J_ws, p_ws)`:
- looks up the actor by `ObjectID`
- casts to `PxRigidBody`
- calls `PxRigidBodyExt::addForceAtPos(..., eIMPULSE, true)` — applies impulse at world point, auto-wakes the body

The haptic engine computes the contact force in world space and sends it as a `HapticWrenchCmd`.  
The physics engine treats `force * duration` as an impulse (N·s) and applies it each step.  
The equal and opposite force is felt by the haptic device via the `HapticEngine` → `DeviceAdapter` path (see [[Haptic Engine]]).

---

## Integration with World System

- Physics reads initial poses from `WorldManager` when building actors
- Physics **writes** updated poses back to `WorldManager` via `setPose()` each step
- `WorldManager` is the authority — physics treats it as a view into mutable state
- If topology changes (objects created/removed) or physics props change, the dirty flag causes a full actor rebuild on the next step

This is intentionally simple for the project scale.  
A more modular design would have physics publish pose updates through a channel rather than calling `setPose()` directly, but the current approach is sufficient and lower latency.
