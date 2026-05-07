# Final Presentation Slide Plan

## Presentation Target

Total presentation/demonstration time: 15 minutes.

Recommended structure:
- 8–9 minutes presentation.
- 3–4 minutes demonstration.
- 2–3 minutes results, limitations, conclusion.

Main audience:
- Supervisor.
- Independent marker who may not have read the report.
- Independent marker may not be a haptics specialist.

Main strategy:
- Keep slides visual.
- Use speaker notes for interpretation.
- Use hidden slides for detailed technical defence.
- Avoid turning slides into report pages.

---

# Slide 1 — Title

## Slide title
A Modular Real-Time Haptic Rendering System

## Suggested visual
- Photo of final prototype.
- Small subtitle:
  - Low-cost 2-DOF haptic interface for robotic manipulation training.

## On-slide bullets
- Low-cost physical haptic device.
- Real-time simulation and visualisation.
- Bidirectional force-feedback loop.

## Speaker focus
Introduce the project as a complete mechatronic system.

## Time
20–30 seconds.

---

# Slide 2 — Motivation

## Slide title
Why Haptic Feedback for Robotic Training?

## Suggested visual
Simple comparison graphic:

| Visual feedback only | Visual + haptic feedback |
|---|---|
| See motion/contact | Feel resistance/contact |
| Limited force awareness | Physical contact cue |
| Safer than real robot testing | More realistic training |

## On-slide bullets
- Manipulator training often relies heavily on vision.
- Contact and force are difficult to communicate visually.
- Simulation allows safe and low-risk training.
- Commercial haptic devices are expensive and less flexible.

## Speaker focus
Explain why haptics matters and why low-cost modularity matters.

## Time
1 minute.

---

# Slide 3 — Engineering Problem

## Slide title
The Challenge: Closing the Haptic Loop

## Suggested visual
Simple loop diagram:

User → Device Sensors → MCU → Host Simulation → Haptic Engine → Torque Command → Motors → User

## On-slide bullets
- Real-time bidirectional interaction.
- Motion must be sensed and transmitted.
- Contact force must be computed and returned.
- Delay, actuator limits, and safety affect fidelity.

## Speaker focus
Frame the work as a system-level integration problem.

## Time
1 minute.

---

# Slide 4 — Aim and Evaluation Questions

## Slide title
Aim and Evaluation Questions

## On-slide content

### Aim
Design, prototype, and evaluate a low-cost 2-DOF haptic interface for simulated robotic manipulation.

### Evaluation questions
1. Can the system operate around 1 kHz?
2. Does the force model produce predictable bounded contact?
3. What limits achievable haptic fidelity?

## Suggested visual
Three large numbered boxes.

## Speaker focus
Use this slide to structure the rest of the presentation.

## Time
1 minute.

---

# Slide 5 — System Architecture

## Slide title
System Architecture

## Suggested visual
Use/report-adapt your high-level architecture diagram.

Show:
- User.
- Haptic device.
- Embedded controller.
- Device adapter.
- Haptic engine.
- Physics simulation.
- OpenGL visualisation.
- Force/torque command path.

## On-slide bullets
Minimal:
- Device state → host simulation.
- Contact force → torque command.
- Modular subsystem separation.

## Speaker focus
Walk through the bidirectional loop.

## Time
1.5 minutes.

---

# Slide 6 — Hardware and Embedded Platform

## Slide title
Physical Haptic Device

## Suggested visual
Annotated photo or CAD image of prototype.

Label:
- End-effector handle.
- Base motor and belt stage.
- Elbow joint.
- Magnetic encoders.
- STM32 and motor drivers.

## On-slide bullets
- 2-DOF planar serial mechanism.
- BLDC gimbal motors.
- Magnetic encoder position sensing.
- STM32 embedded control.
- Torque clamp, slew-rate limiter, watchdog.

## Speaker focus
Explain the practical hardware design and safety features.

## Mention briefly
- No current sensing.
- Final motor-enabled validation was single-axis due to actuator failure.

## Time
1.5 minutes.

---

# Slide 7 — Haptic Rendering Method

## Slide title
From Virtual Contact to Motor Torque

## Suggested visual
Diagram showing:
- Tool point inside wall/object.
- Proxy projected to surface.
- Spring-damper force between tool and proxy.
- Force mapped to joint torques.

## On-slide equations
\[
F = K(x_p - x_t) + D(v_p - v_t)
\]

$\tau = J^T F$

## On-slide bullets
- SDF contact query.
- Proxy-based virtual coupling.
- Jacobian transpose torque mapping.
- Conservative damping and limiting for stability.

## Speaker focus
Explain the haptic rendering method in simple terms.

## Time
1.5 minutes.

---

# Slide 8 — Host Software and Communication

## Slide title
Real-Time Host Architecture

## Suggested visual
Thread/block diagram:

- Renderer thread.
- Physics thread.
- Haptic engine, 1 kHz.
- Device adapter, 1 kHz.
- Message bus between them.

Add callout:
- Snapshot channels use newest state.

## On-slide bullets
- Modular multi-threaded C++ application.
- Queue channels for events.
- Snapshot channels for latest state.
- Designed to avoid stale packet backlog.

## Speaker focus
Explain why architecture matters for real-time haptics.

## Time
1.5 minutes.

---

# Slide 9 — Demonstration

## Slide title
Demonstration: Closed-Loop Haptic Interaction

## Suggested visual
Either:
- Live demo placeholder.
- Screenshot/photo of application and device.
- Three-step demo checklist.

## On-slide bullets
1. Free-space motion.
2. Virtual wall contact.
3. Bounded force/torque response.
4. Safety behaviour.

## Speaker focus
Prepare examiners for what they should observe.

## Time
3 minutes.

## Backup
Have a video ready in case live demo fails.

---

# Slide 10 — Key Quantitative Results

## Slide title
Key Results

## Suggested visual
Use a compact results table.

| Area         |                          Result | Interpretation                  |
| ------------ | ------------------------------: | ------------------------------- |
| Update rate  |                          ~1 kHz | Real-time loop achieved         |
| Host latency |                   0.299 ms mean | Host computation not bottleneck |
| RTT          |                  15.60–15.67 ms | Communication dominates         |
| Force model  |         489.57 N/m, R² = 0.9973 | Matches 500 N/m target          |
| Safety       | 31 N clamp, 55 N m/s slew limit | Bounded interaction             |

## Speaker focus
Interpret the numbers rather than read them.

## Time
1.5 minutes.

---

# Slide 11 — Communication Finding

## Slide title
Main Bottleneck: Communication Latency

## Suggested visual
Use one of:
- RTT CDF plot.
- Host latency histogram.
- Side-by-side comparison:

Host processing:
- Mean 0.299 ms.

Communication RTT:
- Mean 15.60–15.67 ms.

## On-slide message
Host computation was fast; transport delay dominated the haptic loop.

## Speaker focus
This is one of the strongest technical findings.

## Optional
Combine this with Slide 10 if you need fewer slides.

## Time
1 minute.

---

# Slide 12 — Haptic Rendering Validation

## Slide title
Did the Force Model Behave as Intended?

## Suggested visual
Use the force–penetration graph from the simulation validation test.

Optional small secondary visual:
- Single-depth wall contact force plot.

## On-slide bullets
- Motors disabled to isolate command-side force model.
- Configured stiffness: 500 N/m.
- Fitted stiffness: 489.57 N/m.
- R² = 0.9973.
- Single-depth contact remained stable: coefficient of variation = 0.026.

## Speaker focus
Show that the haptic rendering model itself behaved predictably before physical actuator limitations were introduced.

## Key interpretation
The force–penetration relationship was close to linear and closely matched the configured virtual coupling stiffness. This supports the claim that the command-side haptic rendering model was implemented correctly.

## Important caveat
This validates the rendered force command, not calibrated physical output force.

## Time
1 minute.

---

# Slide 13 — Safety and Hardware Validation

## Slide title
Was Physical Interaction Bounded and Safe?

## Suggested visual
Use one main plot:
- Torque saturation plot, OR
- Watchdog torque removal plot, OR
- Device/proxy constrained motion plot.

If space allows, use two smaller visuals:
1. Safety behaviour: torque saturation or watchdog.
2. Hardware contact: constrained motion or repeated contact.

## On-slide bullets
- Force command saturated at 31 N.
- Firmware torque cap limited applied torque to 6.50 N m.
- Slew-rate limiter bounded torque changes at 55 N m/s.
- Watchdog removed torque during communication interruption.
- Motor-enabled contact remained bounded.
- Final force-feedback validation was single-axis.

## Speaker focus
Show that the integrated system behaved safely and remained bounded when actuator feedback was enabled.

## Key interpretation
The hardware tests showed usable bounded interaction, but the results should be interpreted as validation of system behaviour and command consistency rather than calibrated physical force accuracy.

## Important caveat
Physical force was not directly measured, and final motor-enabled validation was limited by actuator failure.

## Time
1–1.5 minutes.

---

# Slide 14 — Design Trade-Offs and Limitations

## Slide title
What Limited Haptic Fidelity?

## Suggested visual
Four-factor diagram:

Communication latency ↔ actuator torque ↔ transmission behaviour ↔ control design

## On-slide bullets
- 15–16 ms RTT limited stable virtual stiffness.
- No current sensing limited force calibration.
- Belt stage introduced compliance/slip.
- Damping improved stability but softened contact.
- Slew-rate limiting reduced oscillation but slowed force response.

## Speaker focus
Show engineering judgement and explain why the system produced bounded but compliant contact rather than rigid high-fidelity feedback.

## Key interpretation
The main limitation was not one isolated component. Haptic fidelity was limited by the interaction between communication delay, actuator capability, mechanical transmission, and stabilising control choices.

## Avoid saying
- “The system achieved accurate force feedback.”
- “The device fully validated 2-DOF force rendering.”

## Better phrasing
- “The system achieved bounded and usable haptic interaction.”
- “Full 2-DOF force rendering remains provisional because final motor-enabled testing was single-axis.”

## Time
1 minute.

---

# Slide 15 — CPD and Project Management

## Slide title
CPD and Project Management

## Suggested visual
Two-column slide:

### CPD
- OpenGL rendering.
- C++ real-time architecture.
- BLDC motor control.
- Haptic rendering theory.
- Latency analysis.

### Project adaptation
- Driver integration issues.
- Pivot to SimpleFOC Mini.
- Hardware delays.
- Actuator failure.
- Staged testing and safety mitigation.

## Speaker focus
Show independent learning and adaptation without spending too long on project management detail.

## Key interpretation
The project required self-directed learning across hardware, embedded control, simulation, haptic theory, and real-time software. The main project management challenge was integration risk, so the plan had to adapt as hardware and driver problems emerged.

## Time
45 seconds to 1 minute.

---

# Slide 16 — Conclusions

## Slide title
Conclusions

## On-slide bullets
- Complete low-cost haptic loop built and evaluated.
- Achieved real-time sensing, rendering, torque command generation, and physical feedback.
- Command-side force model matched intended stiffness.
- Physical interaction was bounded and usable, but not rigid or force-calibrated.
- Communication latency was the dominant limitation.
- Haptic fidelity depends on the whole closed-loop system.

## Suggested final statement on slide
High-rate control alone is not enough: latency, actuation, transmission, and control design jointly determine haptic fidelity.

## Speaker focus
End with the main insight, not just a summary.

## Key interpretation
The project demonstrates a working low-cost haptic system and identifies the practical constraints that must be improved for higher-fidelity interaction.

## Time
1 minute.

---

# Slide 17 — Future Work

## Slide title
Future Work

## Suggested visual
Priority roadmap.

## On-slide bullets
1. Lower-latency communication link.
2. Current sensing and force measurement.
3. Restore full 2-DOF actuation.
4. Improve transmission design.
5. Improve proxy/contact handling.
6. More realistic simulator integration.
7. User/task-based evaluation.

## Speaker focus
Link each future step directly to a measured limitation.

## Key interpretation
The first priority should be reducing communication latency because this was the dominant measured constraint. Current sensing and force measurement should come next because they would allow calibrated physical force validation.

## Time
45 seconds to 1 minute.

---

# Hidden Slide 1 — Driver Choice

## Slide title
Why SimpleFOC Mini?

## On-slide bullets
- Original driver route offered current sensing.
- Integration with encoders/control framework was unreliable.
- SimpleFOC Mini enabled reliable system integration.
- Trade-off: no calibrated torque control.
- Priority: complete working closed-loop prototype.

## Use for questions about
- Driver choice.
- Lack of current sensing.
- Why not ODrive/VESC/B-G431.

---

# Hidden Slide 2 — Latency and Stability

## Slide title
Why Does Latency Limit Haptic Stiffness?

## On-slide bullets
- Haptic force is part of a closed physical feedback loop.
- Delay means force is based on an older state.
- Delay introduces phase lag.
- Phase lag reduces stability margins.
- Lower stiffness/damping/limiting are needed for stable contact.

## Use for questions about
- Why 15–16 ms matters.
- Why contact felt compliant.
- Why not increase stiffness.

---

# Hidden Slide 3 — Force Calibration

## Slide title
What Was and Was Not Validated?

## On-slide bullets
Validated:
- Command-side force model.
- Force-penetration relationship.
- Host/device timing.
- Torque command limiting.
- Watchdog safety.
- Bounded interaction behaviour.

Not validated:
- Calibrated physical output force.
- Full 2-DOF motor-enabled force rendering.

## Use for questions about
- Force accuracy.
- No force sensor.
- No current sensing.

---

# Hidden Slide 4 — Single-Axis Final Validation

## Slide title
Impact of Actuator Failure

## On-slide bullets
- Original device was designed as 2-DOF.
- Both joints remained available for sensing.
- One actuator failed late in development.
- Final motor-enabled force feedback was single-axis.
- Communication, simulation, safety, and timing results remain valid.
- Full 2-DOF force rendering remains future work.

## Use for questions about
- Hardware validation.
- Scope limitation.
- Generalisability.

---

# Hidden Slide 5 — Snapshot Channels

## Slide title
Why Snapshot Channels?

## On-slide bullets
- Host-side packet delivery was bursty.
- Queued old packets can create stale-state latency.
- Snapshot channels retain newest state only.
- Haptic loop consumes latest available data.
- Prevents backlog accumulation.
- Does not remove transport delay.

## Use for questions about
- Software architecture.
- Message bus design.
- Communication mitigation.

---

# Hidden Slide 6 — Future Technical Roadmap

## Slide title
Technical Roadmap

## On-slide priority list
1. Replace ST-Link VCOM / USB CDC path.
2. Add current sensing.
3. Add external force sensor for validation.
4. Restore two-axis actuation.
5. Replace/improve belt transmission.
6. Improve passivity/contact model.
7. Integrate robotic simulator.
8. Conduct user study.

## Use for questions about
- Next steps.
- What you would do differently.
- How to improve fidelity.