Development of a Low-Cost Haptic Interface and Custom Simulation Engine for Manipulator Training
## Background
- Introduce the role haptic interfaces plan in modern industry
	- teleoperation
	- robot assisted surgery
	- operator training
- Why: Visual feedback alone isn't enough for developing accurate motor skills.
	- Back this up using source:  Burdea 1996 [[Haptic_Feedback_for_Virtual_Reality.pdf]]
		- this source discusses applications of haptics
	- The benefits of haptic feedback in robot assisted surgery and their moderators: a meta‑analysis [[s41598-023-46641-8.pdf]]
		- This source shows that haptic provided benefits in nearly all surgery tasks
		- the effect was shown greatest in less experienced surgeons backing up my point on reducing training time
- Discuss how despite these benefits and demand for haptics controllers remain expensive
	- phantom omni in the thousands.
	- Haptic controllers also closed source
	- Commercial devices bound to propitary software


- Research in real-time haptic simulation has advanced collision based models such as those spoken about in Zilles & Salisbury, 1995[[A_constraint-based_god-object_method_for_haptic_display.pdf]]
	- these method require high update rates
- Discuss force mapping from the coupling to actuator using jacobians
	- *add jacobian source*
- limited low cost open source implemntation of this allowing for modular hardwar-software systems.
- discuss lack of unified communication protocol that would allow custom haptic device to interface with differnt simulators / haptic engine without substanial rewriting.

## Motivation
- abscense of a flexible low cost and simjlato agnostic haptic interface limits the ability of reseraches and educaters and students to prototype /evaluate robotic manipulation systems in a safe, virtual environment
- Need for a system that provided
	- affordable haptic hardware - constructed with widely avaiable componts
	- an open and hardware independant commuication protocol 
		- enavling integration with any simulation or robot platform
	- A fully transparent haptic rendering engine where latency, force compuation and stability can be controller
- A modular architecture that can later be interfaced with a physical manipulator with minimal redesign.
This project addresses this gap by developing a complete 2-DOF haptic input device and a custom simulation platform designed to explore real-time haptic rendering for manipulator training.


### Aim
The aim of this project is to design, implement, and evaluate a low-cost, open-architecture 2-DOF haptic interface and simulator that provides real-time, stable force feedback for robotic manipulator training.
### Objectives

- Design and construct a 2-DOF haptic device using low-cost BLDC actuators and magnetic encoders.
- Develop embedded firmware implementing field-oriented control, encoder acquisition, and a bidirectional communication protocol.
- Implement an open, device-agnostic packet protocol for streaming joint states and receiving force/torque commands.
- Create a custom physics and rendering engine supporting collision detection, virtual proxy methods, and force computation.
- Implement Jacobian-based force mapping from end-effector space to joint torques.
- Evaluate the system’s latency, stability, and force accuracy against real-time haptic rendering requirements.
- *Demonstrate modular integration with external simulators (e.g., Isaac Sim, MATLAB/Simulink).*


## **1. Introduction**

Haptic interfaces play a central role in modern teleoperation, robot-assisted surgery, and operator training, where visual feedback alone is insufficient for developing accurate motor skills. Studies in human–robot interaction demonstrate that force feedback significantly improves task performance, situational awareness, and user learning rates compared to purely visual systems (MacLean, 1996; Burdea, 1996). However, despite the increasing use of robotic manipulators in education and industry, haptic training systems remain expensive, closed, and inaccessible to researchers without specialist hardware (Salisbury & Tarr, 2003). Commercial devices such as the Geomagic Touch are priced for laboratory environments, and their software stacks are tightly bound to proprietary development kits.

Parallel research in real-time haptic simulation has advanced collision-based interaction models, including the widely adopted virtual proxy or virtual coupling method for stable force rendering in deformable and rigid-body environments (Zilles & Salisbury, 1995; Ruspini et al., 1997). These methods require high update rates (typically ≥1 kHz), consistent end-effector position tracking, and stable mappings from interaction forces to actuator torques. In robotic systems, such mappings are typically performed using manipulator Jacobians (Murray et al., 1994; Siciliano et al., 2010). Yet despite this extensive knowledge base, there is limited work combining these models into **low-cost, open, and fully modular hardware–software systems** that can be integrated with arbitrary simulators.

Recent literature highlights a growing need for accessible open-source haptic devices for robotics education and research (Kim et al., 2019; Knoop et al., 2021). Existing platforms either rely on specialised controllers or lack a unified communication protocol that would allow a custom-built haptic device to interface with different simulators without substantial rewriting. Furthermore, most research relies on established environments such as CHAI3D, SOFA, or Unity plugins, which constrain timing architecture, restrict modification, and obscure the end-to-end latency sources critical for haptic performance.

### **Motivation**

The absence of a flexible, low-cost, and simulator-agnostic haptic interface limits the ability of researchers, educators, and students to prototype and evaluate robotic manipulation systems in a safe, virtual environment. There is a clear need for a system that provides:

1. **Affordable haptic hardware** that can be constructed using widely available components;
    
2. **An open and hardware-independent communication protocol** enabling integration with any simulation or robot platform;
    
3. **A fully transparent haptic rendering engine**, where latency, force computation, and stability can be explicitly controlled; and
    
4. **A modular architecture** that can later be interfaced with a physical manipulator with minimal redesign.
    

This project addresses this gap by developing a complete 2-DOF haptic input device and a custom simulation platform designed to explore real-time haptic rendering for manipulator training.

### **Aim**

**The aim of this project is to design, implement, and evaluate a low-cost, open-architecture 2-DOF haptic interface and simulator that provides real-time, stable force feedback for robotic manipulator training.**

### **Objectives**

- Design and construct a 2-DOF haptic device using low-cost BLDC actuators and magnetic encoders.
    
- Develop embedded firmware implementing field-oriented control, encoder acquisition, and a bidirectional communication protocol.
    
- Implement an open, device-agnostic packet protocol for streaming joint states and receiving force/torque commands.
    
- Create a custom physics and rendering engine supporting collision detection, virtual proxy methods, and force computation.
    
- Implement Jacobian-based force mapping from end-effector space to joint torques.
    
- Evaluate the system’s latency, stability, and force accuracy against real-time haptic rendering requirements.
    
- Demonstrate modular integration with external simulators (e.g., Isaac Sim, MATLAB/Simulink).


Haptic interfaces play a central role in modern teleoperation, robot-assisted surgery, and operator training, where visual feedback alone is insufficient for developing accurate motor skills. Early work by Burdea (1996) demonstrated that force feedback improves user perception and task precision across a range of virtual environments, and more recent evidence continues to reinforce these findings. A large-scale meta-analysis on robot-assisted surgery confirmed that introducing haptic feedback leads to consistent improvements in surgical task performance, with the largest benefits observed in less experienced operators, highlighting its potential to accelerate training and skill acquisition. \