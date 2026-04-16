# Haptic Engine

Part of the [[Implementation_Index]].

## Quick Navigation

- [[#Overview]]
- [[#1 kHz Loop]]
- [[#Contact Search (SDF)]]
- [[#Proxy Projection]]
- [[#Virtual Coupling]]
- [[#Force Clamping and Outputs]]
- [[#Coordinate Transforms]]

---

## Overview

`HapticEngine` (`include/engines/HapticEngine.h`) runs the haptic feedback loop.

It runs at **1 kHz** on its own thread.  
Each tick it:
1. Reads the latest world state and tool position
2. Searches for contact with world objects using SDFs
3. Projects a proxy onto the nearest surface when in contact
4. Computes a virtual coupling force between proxy and device
5. Sends force commands to the device and to physics

Inputs (via [[Messaging]] channels):
- `world.snapshots` — latest world state (objects + poses)
- `haptics.tool_in` — tool pose/velocity from `DeviceAdapter` (or mouse)

Outputs:
- `haptics.wrenches` → `PhysicsEngine` (apply reaction force to physics objects)
- `device.wrench_cmd` → `DeviceAdapter` (send torque commands to physical device)
- `haptics.snapshots` → `GlSceneRenderer` (for overlay rendering)

---

## 1 kHz Loop

`run()` uses `std::chrono::steady_clock` and `sleep_until` for timing:

```
loop:
  record loop start time
  compute dt from last loop start
  call update(dt)
  sleep_until next 1ms wake time
```

This achieves ~1 kHz update rate.  
The loop is currently infinite (no stop flag) — the thread is detached on application shutdown.

---

## Contact Search (SDF)

In `update(dt)`, after draining the latest world snapshot and tool state:

1. Iterate all world `ObjectState` records
2. Skip objects with `role == Tool` or `role == Proxy`
3. Look up the `SDF` from `GeometryDatabase` for each object's geometry (see [[World System#SDF Interface]])
4. Transform tool position to object-local space using `toLocal(obj.T_ws, toolPose.p)`
5. Query `sdf->queryLocal(p_ls)` → `SDFQuery { phi, grad }`
6. Scale distance to world units: `phi_ws = q.phi * obj.T_ws.s`
7. Track the object with the **smallest phi** (deepest penetration)

If `bestPhi < 0` — the tool is inside an object:
- compute contact point by projecting along gradient:  
  `proj_ls = p_ls - n_ls * phi` (surface projection in local space)
- transform back to world: `contactPoint_ws = toWorld(obj.T_ws, proj_ls)`
- transform normal to world: `contactNormal_ws = dirToWorld(obj.T_ws, grad_ls)`

---

## Proxy Projection

The proxy is a virtual point that stays on or outside all surfaces.

```
if bestPhi < 0:
    proxyPose.p = contactPoint_ws   ← snapped to surface
else:
    proxyPose.p = toolPose.p        ← free-space, follows device
    contactId = 0                   ← no contact
```

This is the **god-object / proxy** approach for haptic rendering:
- the proxy represents where the virtual tool "would be" if surfaces were solid
- the device position (tool) can penetrate the surface in software
- the difference between proxy and device drives the coupling force

---

## Virtual Coupling

A spring-damper between **proxy** and **device** generates the feedback force:

```
proxyVel = (proxyPose.p - proxyPosePrev.p) / dt
toolVel  = latestTool.toolVel_ws

F = K * (proxyPos - toolPos) + D * (proxyVel - toolVel)
```

Parameters:
- `K = 500 N/m` (spring stiffness)
- `M = 0.02 kg` (virtual mass, used for critical damping calc)
- `D = 0.7 * 2 * sqrt(K * M)` (damper, set to ~70% of critical damping)

This is the virtual coupling model: the spring pulls the device back toward the proxy, giving the user the sensation of pushing against a surface.

---

## Force Clamping and Outputs

Before publishing, the force is clamped to a safe maximum:

```
Fmax = 31 N
if |F| > Fmax:
    F = F * (Fmax / |F|)
```

Then outputs are published:

**If in contact (`contactId != 0`)**:
- `wrenchOut_` (→ [[Physics Engine]]) — publishes `-F` at `contactPoint_ws`  
  (reaction force: device pushes on the object, object gets pushed in the opposite direction)
- `deviceCmdOut_` (→ [[Hardware Interface]]) — publishes `+F`  
  (device feels the contact force)

**If free-space**:
- `deviceCmdOut_` — publishes zero force (explicit zero so device stays quiet)

**Every tick**:
- `hapticOut_` (→ [[Rendering Engine]]) — publishes `HapticSnapshotMsg` containing device pose, proxy pose, and force vector

---

## Coordinate Transforms

Three local helper functions handle frame conversion (all inline, scoped to the .cpp):

| Function | Purpose |
|---|---|
| `toLocal(T_ws, p_ws)` | World point → object-local space (subtract translation, apply inverse rotation, undo scale) |
| `toWorld(T_ws, p_ls)` | Object-local point → world space (scale, rotate, translate) |
| `dirToWorld(T_ws, v_ls)` | Object-local direction → world space (rotate only, no translate/scale) |

These are needed because SDFs are defined in their own unit local space, but the tool position and contact results must be expressed in world space.
