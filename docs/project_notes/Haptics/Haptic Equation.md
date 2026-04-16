
In [[Khatib_1987_RA.pdf]] Khatib defines the equations of motion of the manipulator
					$Λ(x)x¨+μ(x,x˙)+p(x)=F$ 
Khatib defines $\Lambda$(EE inertia), $\mu$(centrifugal/Coriolis), and $p$(gravity).

This equation translates the force applied to the proxy in the simulation and returns the acceleration/pose of the proxy.

$F(x_p​,x˙_p​;x_{ref}​)$ that says “given the proxy’s state and the hand’s target, what force should act on the proxy?” Then you **substitute** that into Khatib to get a closed-loop ODE you can integrate at 1 kHz.

# Proxy Dynamics + Virtual Coupling

## Common, safe proxy force

- **Viscous damping:** $-D_s,\dot{x}_p$
    
- **Contact/obstacle force (penalty):** $-\nabla V_{\text{env}}(x_p)$ _(or use projection for hard walls)_
    
- **Weak tether to hand target:** $-K_{\text{track}},(x_p - x_{\text{ref}})$
    

So, define the proxy’s task-space force:

$F(x_p,x˙_p; x_{ref})=−Ds x˙p−∇Venv(xp)−Ktrack (x_p−x_{ref}).F(x_p,\dot{x}_p;\,x_{\text{ref}}) = -D_s\,\dot{x}_p - \nabla V_{\text{env}}(x_p) - K_{\text{track}}\,(x_p-x_{\text{ref}}).$

## Substitute into Khatib (applied to the proxy)

$Λs x¨p+Ds x˙p+∇Venv(xp)+Ktrack(xp−xref)=−μs−ps.\Lambda_s\,\ddot{x}_p + D_s\,\dot{x}_p + \nabla V_{\text{env}}(x_p) + K_{\text{track}}(x_p - x_{\text{ref}}) = -\mu_s - p_s.$

If the sim/controller compensates Coriolis and gravity (common), the RHS $\approx 0$:

$Λs x¨p+Ds x˙p+∇Venv(xp)+Ktrack(xp−xref)=0.\Lambda_s\,\ddot{x}_p + D_s\,\dot{x}_p + \nabla V_{\text{env}}(x_p) + K_{\text{track}}(x_p - x_{\text{ref}}) = 0.$

Then **project** $x_p$ back to the contact manifold if the tentative step would penetrate _(god-object clamp)_.

## Per-tick loop (high level)

1. Integrate the **proxy ODE** (e.g., semi-implicit Euler).
    
2. **Project** $x_p$ to the surface if penetration is detected.
    
3. Compute and render **virtual coupling** to the hand:
    
    Fcpl=Kc (xp−xd)+Bc (x˙p−x˙d),τd=Jd⊤Fcpl.F_{\text{cpl}} = K_c\,(x_p - x_d) + B_c\,(\dot{x}_p - \dot{x}_d), \qquad \tau_d = J_d^\top F_{\text{cpl}}.