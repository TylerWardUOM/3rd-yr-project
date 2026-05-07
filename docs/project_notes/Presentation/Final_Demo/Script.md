
# Slide 1 - Title

This project is about building and evaluating a complete low-cost haptic feedback loop.

The aim was not just to make an input device, but to connect sensing, simulation, force computation, and actuation in real time.

Main focus: system-level integration and evaluation.


# Slide 2 - Motivation

 - Robotic manipulation often relies heavily on visual feedback.
 - Vision does not naturally communicate contact, resistance, or excessive force.
 - Haptic feedback can improve realism and safety in training.
 - Simulation avoids risk and cost during early operator training.
 - Commercial systems exist but are expensive and often less accessible for student/research prototyping.

# Slide 3 — Problem Definition
- The challenge is closing the full bidirectional haptic loop.

- User motion must be sensed, transmitted, interpreted, converted to force, mapped to torque, and returned to the user.

- Each stage introduces constraints.
- Such as:
  - Communication delay between MCU and Host.
  - Limited actuator torque due to low cost motors or transmission imperfections
  - Software processing can introduce latency.
  - and the system must remain safe for user and hardware e.g output clamps watchdog timeouts.

# Slide 4 — Aim and Evaluation Questions
Overall aim:
- Design, prototype, and evaluate a low-cost haptic interface for simulated robotic manipulation.

The three main questions i aimed to answer when evaluating the system were.
1. Can the system close a real-time haptic loop at approximately 1 kHz?
2. Does the haptic rendering model produce predictable and bounded contact forces?
3. What limits the achievable haptic fidelity?

# Slide 5 — System Overview

1. User moves physical device.
2. Encoders measure joint angles.
3. Microcontroller sends state packets.
4. Host computes tool pose.
5. Haptic engine checks contact.
6. Force command is calculated.
7. Force is mapped to joint torque.
8. Torque command is returned to device.
9. Motors provide feedback to user.

# Slide 6 — Hardware and Embedded Platform

- The Device is built as a 2-dof serial mechanism.
- The main structure is 3D printed out of PLA 
- The main joint is a 8mm stainless steel rod that runs on 608 bearings to ensure minimal friction
- The firmware for the device runs on a stm32 Nucelo g431RB microcontroller
- The motors used are gb2208 gimbal motors a gt2 belt reduction is used to increase torque for the base joint.
- The motors are controlled using a simpleFOC mini driver board and accompaning software library
- Magnetic encoders are used both for joint position but also to enable the torque control of motors
- The MCU also handles some additional things such as torque clamps and a watchdog timeout for safety and also a slew-rate limiter to act as a damper reducing oscillations

- The design has a few flaws the belt transmission introduces compliacne/slip this is mainly due to 3d printed pulleys done to save cost was mitigated using a tensioner 
- The simpleFOC boards used do not support current sensing so torque is not physically calibrated (this was done because of issues probs explain in the buisness managment)
- One actuator did fail during some point in development so the final validation and build done with single axis active due to not enough budget to replace

# Slide 7 - Haptic Rendering

The haptic rendering works by first receiving and using forward kinematics to convert joint angles to a end effector pose (Done in the device adapter layer)
A proxy follows this device end effector pose but if contact is made with a object it is constrained to the surface.

This is done using SDF functions where each object in the software has a SDF mathematical representation this is a signed distance function which takes in a point and returns the shortast distance to the surface this value is negative if  inside the shape zero if on the surface and positive if outside the shape.

The surface normal is estimated from the SDF gradient, and the proxy point is computed by projecting the tool position back to the zero-level set of the SDF using equation

A restoring Force is then calculated using this equation this acts like a virtual spring damper couple between the device and the proxy 

The force is then used to calculate individual joint torques using the Jacobian matrix constructed with the current joint angles.  
This is a static force method but is used for its simplicity
Joint torques then sent to the device adapter to packet and send

# Slide 8 — Host Software and Communication

The software is multi threaded with each main process seperated into its own indivdual thread.
All the threads run independantly and use message busses to communicate between them
There are two main types queue channels for events and commands and then snapshots channels that hold the latests state this is what allows for constiance loop speed even with communication latency

Communication is done between host and the MCU using a stm virtual com link. Messages are used to send torque commands to the device and then to recive the angles messages are packaged into discrete packets that contain timestamps aswell as a sequence number used to discard out of order packets and for debugging

A logging system also exits on its own thread again using message channels to communicate threads it saves data in memory and then writes and saves data to csv files after the software closes 

# Slide 9 - Demonstration
Do a quick demo if it works
## Key behaviours to show
1. Free-space movement.
2. Contact with virtual wall/object.
3. Proxy constrained to surface.
4. Force/torque response.
5. Safety behaviour if shown:
   - Saturation.
   - Slew-rate limiting.
   - Watchdog.
# Slide 10 — Key Quantitative Results
Communication:
- Approximately 1 kHz update rate.
- Packet loss negligible at and above 500 Hz in recorded tests.

Host timing:
- Mean host-side latency: 0.299 ms.
- 99th percentile host-side latency: 2.04 ms.

Round-trip latency:
- RTT approximately 15.60–15.67 ms.
- Main timing limitation.

Simulation validation:
- Fitted stiffness: 489.57 N/m.
- Configured stiffness: 500 N/m.
- R² = 0.9973.
Safety:
- Force clamp: 31 N.
- Slew-rate limit: 55 N m/s.
- Watchdog removes torque during communication interruption.
The key result is that the system achieved fast local computation and predictable command-side force generation, but the full haptic loop was limited mainly by communication latency rather than host processing.

# Slide 11 — Communication Finding
I analyzed the commnication of the device first using seperate scripts to test the connection between host and mcu this was done to test performace of pings at differnt rtt and also checking for packet loss at differnt communication rates.

these tests showed intresting results that i attributed to the st virtual com link and usb buffering.
The performace at low frequencies showed some packet lost but this went away above 500hz and then rtt tests showed that a round trip latency existed of around 15ms most of the time 

I then tested end to end timing using the mcu running the firmware and the main host software using the logging thread. this showed intresting results first it verified that i had achived fast host side end-end latency measured as the time for a loop to be completed from joint angle packet arriving to torque command having been sent this was on aveage 0.299 ms with majoirt of the time coming from the time to transfer the packets.

However the full end to end loop time showed packets arrived in bursts predominatly these burst appear to arrive around every 15ms this can be seen in this graph

# Slide 12 — Haptic Rendering Validation
I validate the haptic rendering by testing it with the device but with the acutators dissabled to isolate the system.

I tested by investigating the command force at various penetration depths and also how stable the output force was when holding a consistant depth these results showed that the outputted force was very linearly corelated with the depth and the differnve is explained by the dampening term in the computation of the force and the force remained pretty much stable for penetration even with the high frequency oscillation in the penetration due to encoder noise 

# Slide 13 — Safety 
- Force command saturated at 31 N.
- Firmware torque cap limited applied torque to 6.50 N m.
- Slew-rate limiter bounded torque changes at 55 N m/s.
- Watchdog removed torque during communication interruption.
Safety was treated as part of the control architecture rather than an add-on. Because the system can actively apply torque to the user, force and torque outputs had to be bounded at multiple levels.

On the host side, the Cartesian force command was clamped at 31 N. On the firmware side, the applied torque was also capped, so even if an invalid or excessive command was received, the device output remained bounded.

The slew-rate limiter prevented abrupt torque changes, which improved stability and reduced oscillation, although it also reduced contact sharpness.

Finally, the watchdog removed torque if communication was interrupted, meaning the safe failure mode was zero actuation.

# Slide 14 — Hardware Validation
With motor feedback enabled, the virtual wall produced a clear resisting force when the user attempted to penetrate it. The contact felt bounded and usable, but not perfectly rigid.

This matches the earlier results: the command-side force model was predictable, but the physical interaction was softened by communication delay, conservative stiffness selection, slew-rate limiting, and belt compliance.

Under normal interaction, penetration was limited to around 0.05 m. However, higher applied user force could overcome the belt transmission and cause slip, showing that the mechanical transmission became a limiting factor.

Surface-following tests showed that the device could guide motion along the constraint, although the trajectory was not perfectly on the surface due to compliance, latency, and single-axis active feedbac

# Slide 15 — Design Trade-Offs and Limitations
The main conclusion from the design trade-off analysis is that haptic fidelity was not limited by one component alone.

The force model behaved as intended, and the host computation was fast. However, communication latency limited stable stiffness, while the actuator and belt transmission limited the physical force that could actually be rendered.

The damping, torque clamps, and slew-rate limiter were necessary to make the device bounded and controllable, but they also reduced contact sharpness.

So the final system demonstrates stable low-cost haptic feedback, but not high-fidelity rigid contact. That is the central trade-off of this prototype.
1. No direct physical force measurement.
2. No current sensing, so torque output is not calibrated.
3. Final motor-enabled testing limited to single-axis actuation.
4. ST-Link virtual COM introduced significant latency.
5. Belt transmission introduced compliance and possible slip.


# Slide 16 — Conclusions
- Complete low-cost haptic loop was built and evaluated.
- System achieved real-time sensing, haptic rendering, torque command generation, and physical feedback.
- Contact was bounded and usable.
- Host-side computation was not the main bottleneck.
- Communication latency was the dominant limitation.
- Actuation and transmission design also constrained fidelity

# Slide 17 — Future Work
Link each future step directly to a measured limitation.