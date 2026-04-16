

# Core Classes (Phase 1)

# High Level Overview

- Goal now (Phase 1): proxy + projection (god-object) + virtual coupling at ~1 kHz.  
- Later (Phase 4): add Khatib’s plant dynamics with snapshots $\{J, \Lambda, \mu, p\}$.  
- Separation: haptics loop (1 kHz) independent of Unity/sim frame rate.

---

# Classes

## Proxy
**Responsibility:** Owns proxy state and integrates one 1 kHz tick.  

**Inputs:**  
- $x_{\text{ref}}$ (mapped from device)  
- $x_d, \dot{x}_d$ (device state)  
- Environment queries  

**Outputs:**  
- Updated $x_p, \dot{x}_p$ (proxy state)  
- $F_{\text{cpl}}$ (to device)  

**Does:**  
- Integrate proxy ODE (semi-implicit Euler).  
- Hard contact via projection (optionally zero normal velocity).  
- (Optional) soft shells via $-\nabla V_{\text{env}}$.  
- Calls Coupling to compute $F_{\text{cpl}}$.

---

## Coupling
**Responsibility:** Computes virtual coupling force.  

**Equation:**  
$$
F_{\text{cpl}} = K_c(x_p - x_d) + B_c(\dot{x}_p - \dot{x}_d)
$$

**Output:** $F_{\text{cpl}}$ passed back to the device.

---

## Env Interface
**Responsibility:** Query environment for proxy integration.  

**API:**  
- $\phi(x)$ → signed distance  
- $\nabla \phi(x)$ → gradient (surface normal)  
- $\text{project}(x)$ → closest feasible surface point  

**Notes:**  
- Environment defined at runtime (analytic shapes at first).  
- In the future, environment updated by simulation (Unity or robotics sim).  
- Used by Proxy to resolve contacts.

---

## Env Classes
**Responsibility:** Store and evaluate specific environments.  

- Start with analytic primitives: Plane, Sphere, Box.  
- Provide $\phi(x)$, $\nabla \phi(x)$, and projection.  
- Later: Signed Distance Field (SDF) for arbitrary meshes.  
- Composite environment: combine shapes with min/max.

---

## Device Interface
**Responsibility:** Abstract the device I/O.  

**API:**  
- `poll()` → returns $(x_d, \dot{x}_d)$ (end-effector pose/velocity).  
- `sendTorque(\tau)$` → sends joint torques to actuators.  

**Notes:**  
- First implement `DummyDevice` (mouse, sine wave).  
- Later implement `RealDevice` (ODrive, SimpleFOC, etc.).

---

## Logger
**Responsibility:** Log high-rate data for debugging/tuning.  

**What to log (1 kHz):**  
- $x_d, \dot{x}_d$  
- $x_p, \dot{x}_p$  
- $F_{\text{cpl}}$  
- Contact flags (inside/outside, object ID)

---

# Future Classes

## PlantSnapshot (Khatib)
**Purpose:** Carry simulation dynamics into proxy loop.  

**Fields:**  
- $\Lambda$ (task-space inertia)  
- $\mu$ (Coriolis/centrifugal wrench)  
- $p$ (gravity wrench)  
- $J$ (Jacobian)  
- Timestamp  

**Used by:** Proxy (switches from massless ODE → plant-aware ODE).  

**Equation (with plant):**  
$$
\Lambda \ddot{x}_p + \mu + p = F_p
$$

---

## SimulationAdapter
**Responsibility:** Provide latest environment & plant snapshot.  

- Now: only transforms for objects.  
- Later: also $\{J, \Lambda, \mu, p\}$.  
- Runs at 200–500 Hz; proxy consumes at 1 kHz (zero-order hold).  

---

## Protocol (IPC)
**Responsibility:** Define message formats for Unity/sim ↔ haptics loop.  

- Scene init: object type + parameters.  
- Transform update: id + pose.  
- Telemetry back: $x_d, x_p, F_{\text{cpl}}, \text{contacts}$.

---
Now:** can be env-only (transforms).  
**Later:** publish {J,Λ,μ,p}\{J,\Lambda,\mu,p\}{J,Λ,μ,p} at 200–500 Hz.  
**Contract:** non-blocking “latest complete snapshot” (ZOH).

### `Protocol` (IPC)

**Responsibility:** Message layouts for scene init, transforms, and telemetry.  
**Variants:** UDP (easy), shared memory ring buffer (low-latency).



## Folders

### src/haptics/
- **Proxy** (`proxy.cpp/h`): maintains proxy state, integrates ODE, does projection.
- **Coupling** (`coupling.cpp/h`): computes F_cpl = Kc(x_p-x_d)+Bc(v_p-v_d).
- **Integrator** (`integrator.cpp/h`): numerical methods (semi-implicit Euler, RK2).

### src/env/
- **EnvIface** (`env_iface.h`): abstract interface with phi, grad, project.
- **Plane, Sphere, Box** (`env_plane.cpp`, etc.): analytic primitives.
- **EnvComposite**: combines multiple shapes, handles unions.

### src/device/
- **DummyDevice**: mouse/keyboard/sine-wave driver.
- **RealDevice**: later, hardware encoder+motor driver interface.
- **DeviceIface**: common poll() / sendTorque() API.

### src/ipc/
- **UDP/SharedMemory**: handles snapshots between Unity & servo.
- **Protocol.h**: defines message layouts.

### src/utils/
- **Logger**: CSV output for x_d, x_p, F_cpl.
- **Timing**: high-res clock, profiling utilities.

---


``` cpp
#pragma once
#include <string>

namespace env {

// Minimal 3D vector
struct Vec3 {
    double x, y, z;
    Vec3 operator+(const Vec3& o) const { return {x+o.x, y+o.y, z+o.z}; }
    Vec3 operator-(const Vec3& o) const { return {x-o.x, y-o.y, z-o.z}; }
    Vec3 operator*(double s) const { return {x*s, y*s, z*s}; }
    Vec3 operator/(double s) const { return {x/s, y/s, z/s}; }
};

// Dot product
inline double dot(const Vec3& a, const Vec3& b) {
    return a.x*b.x + a.y*b.y + a.z*b.z;
}

// Norm (length)
inline double norm(const Vec3& a) {
    return std::sqrt(dot(a,a));
}

// Normalize (with fallback)
inline Vec3 normalize(const Vec3& a) {
    double n = norm(a);
    return (n > 1e-9) ? a/n : Vec3{1,0,0};
}

// Result structure
struct QueryResult {
    double phi;   // signed distance
    Vec3 grad;    // gradient (normal, not necessarily unit)
    Vec3 proj;    // projected point
    bool inside;  // phi < 0
    std::string id; // optional label
};

class EnvInterface {
public:
    virtual ~EnvInterface() = default;

    virtual double phi(const Vec3& x) const = 0;
    virtual Vec3 grad(const Vec3& x) const = 0;
    virtual Vec3 project(const Vec3& x) const = 0;

    virtual QueryResult query(const Vec3& x) const {
        double d = phi(x);
        Vec3 g = grad(x);
        Vec3 p = project(x);
        return {d, g, p, d<0.0, ""};
    }
};

} // namespace env

```




``` cpp
#pragma once
#include "env_interface.h"

namespace env {

class PlaneEnv : public EnvInterface {
public:
    PlaneEnv(const Vec3& n_in, double b_in) : n(normalize(n_in)), b(b_in) {}

    double phi(const Vec3& x) const override {
        return dot(n, x) + b;
    }

    Vec3 grad(const Vec3& /*x*/) const override {
        return n;
    }

    Vec3 project(const Vec3& x) const override {
        double d = phi(x);
        return x - n*d;
    }

private:
    Vec3 n; // unit normal
    double b; // offset (plane: nᵀx + b = 0)
};

} // namespace env
```

``` cpp
#pragma once
#include "env_interface.h"

namespace env {

class SphereEnv : public EnvInterface {
public:
    SphereEnv(const Vec3& c_in, double R_in) : c(c_in), R(R_in) {}

    double phi(const Vec3& x) const override {
        Vec3 r = x - c;
        return norm(r) - R;
    }

    Vec3 grad(const Vec3& x) const override {
        Vec3 r = x - c;
        double d = norm(r);
        return (d > 1e-9) ? r/d : Vec3{1,0,0};
    }

    Vec3 project(const Vec3& x) const override {
        Vec3 r = x - c;
        double d = norm(r);
        if (d < 1e-9) return Vec3{c.x+R, c.y, c.z}; // arbitrary
        return c + r*(R/d);
    }

private:
    Vec3 c; // center
    double R; // radius
};

} // namespace env
```
