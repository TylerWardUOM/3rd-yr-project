Research and notes on Virtual Coupling

Virtual Coupling is a virtual spring-damper that links the physical controller to a virtual proxy.
This means that we will render the difference between the physical controller and virtual proxy instead of raw simulator forces.

It is needed to keep the haptic loop stable and handle model mismatch, while still giving believable responses.

The virtual proxy will act as a buffer between the device and the simulator. This is important for rendering haptics without issues caused by simulation delays and discontinuous forces.


1. Device where the proxy wants to go
2. Proxy is constrained by the environment
3. Virtual coupling produces a force that pulls the device towards the proxy
Simulation that may be operating at a slower frequency updates the proxies environment when it can.

Virtual Coupling Force
The virtual couple between the position of the device and virtual proxy can be represented as a spring damper coupling
$Fcpl​=Kc​(xp​−xd​)+Bc​(x˙p​−x˙d​).$ 
### Coordinate Frames
#### Device Frame
The "Device Frame" is what I am going to call the real world where the physical device exists.
In this frame the devices end effector coordinates are going to be represented by the vector:
								$x_d$ 


### Haptic Frame
Virtual Proxy exists in the haptic environment
Haptic environment receives a snapshot from simulator whenever available 
	snapshot contains simple information such as position of shapes of rigid bodies environment parameters for those bodies such as surface friction and also the feel of the robot such as operation space inertia and task space damping 

Haptic then sends proxy position to update simulation when it can and also values to be fed to real controller

#### Simulation Frame
The "Simulation Frame" is what I am going to call the simulated environment where the manipulator that is being controlled exists.

In the "Simulation Frame" a virtual proxy of the physical device exists, as well as the simulated manipulator that is being controlled.

The devices coordinates from "Device Frame" $x_d$ need to be mapped to the simulation frame to give reference coordinates $x_{ref}$  that the simulated manipulator will try to reach.

