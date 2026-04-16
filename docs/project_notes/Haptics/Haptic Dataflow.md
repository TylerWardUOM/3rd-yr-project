

* User moves "master" device
* Encoders Measure joint positions
	* Firmware estimates joint velocities
* Microcontroller sends joint positions and velocities to the hots PC
* PC Computes forward kinematics
* Simulator Calculates and applies force to virtual twin
* [[Virtual Coupling]] Force calculated 
* Use Jacobians of "master" device to compute joint torques
* send torques back to micro controller
* Microcontroller runs FOC to drive motors
* User feels Haptics`

``` mermaid
flowchart LR
    subgraph DeviceLoop [Haptic Device Loop @ 1 kHz]
        D["User Hand / Handle<br/>(x_d, ẋ_d)"]
        DC["Device Sensors<br/>(Encoders, IMU)"]
        VC["Virtual Coupling<br/>F_cpl = K_c(x_p - x_d) + B_c(ẋ_p - ẋ_d)"]
        ACT["Motor Drivers<br/>(τ_d applied)"]
    end

    subgraph ProxyLoop [Proxy Dynamics Integration @ 1 kHz]
        Xref["Reference Mapping<br/>x_ref = T(x_d)"]
        PF["Proxy Force Law<br/>F_p = -D_s ẋ_p - ∇V_env(x_p) - K_track(x_p - x_ref)"]
        KH["Khatib Task-Space Dynamics<br/>Λ ẍ_p + μ + p = F"]
        XP["Proxy State<br/>(x_p, ẋ_p)"]
        ENV["Environment Potential / Constraints<br/>V_env(x)"]
    end

    subgraph Simulation [Manipulator Simulation @ 200-500 Hz]
        SIMJ["Jacobian J(q)"]
        SIML["Λ(x), μ(x, ẋ), p(x)"]
        SIMENV["Environment Model<br/>(meshes, SDF, walls)"]
    end

    D --> DC --> Xref
    Xref --> PF
    XP --> PF
    PF --> KH
    KH --> XP
    XP --> VC
    DC --> VC
    VC --> ACT

    SIMJ --> SIML
    SIML --> KH
    SIMENV --> ENV --> PF

```


## Phase 1 — Core Haptics (no Unity dependency)

- Run **1 kHz haptic loop**:
    
    - Proxy + projection (god-object) for contact.
        
    - Virtual coupling for device forces.
        
- Environment = simple **analytic primitives** (plane, sphere, box).
    
- Goal: crisp contacts, stable loop, logging xd,xp,Fcplx_d, x_p, F_{\text{cpl}}xd​,xp​,Fcpl​.
    

---

## Phase 2 — Unity Visualization + Transforms

- Unity builds scene (walls, spheres, etc.).
    
- Unity **sends object transforms** (position + rotation) → haptic loop.
    
- Haptic loop still owns **contact math** (proxy, projection).
    
- Unity draws **gizmos**:
    
    - Device handle xdx_dxd​
        
    - Proxy point xpx_pxp​
        
    - Contact normals, collision indicators
        
- Communication (pick one):
    
    - **Easy:** UDP on localhost (127.0.0.1).
        
    - **Better:** shared memory ring buffer (lower latency, deterministic).
        

---

## Phase 3 — Environment Upgrade (SDF)

- Replace analytic shapes with **SDF module** in haptic loop.
    
- Unity still just sends transforms; servo handles ϕ(x),∇ϕ(x),project(x)\phi(x), \nabla\phi(x), project(x)ϕ(x),∇ϕ(x),project(x).
    
- Benefit: arbitrary shapes (fixtures, curved surfaces).
    
- No Unity code changes needed.
    

---

## Phase 4 — Extra UI in Unity (optional)

- Add task layer:
    
    - Timers, scores, “ghost path” for training.
        
    - Replay visualization.
        
- Unity = purely **visual and scoring**, not physics.
    

---

## Phase 5 — Robot Plant (Khatib)

- Simulation (Pinocchio/RBDL/Isaac) computes snapshots:  
    {J,Λ,μ,p}\{J, \Lambda, \mu, p\}{J,Λ,μ,p}.
    
- Haptic loop swaps from massless ODE to **plant-aware ODE**.
    
- Unity overlays “heaviness cues” or shows robot sim state.