

## High Level Overview

## Classes

#### Proxy Class
- Holds Proxy State ($x_p$,$v_p$)
- Integrate dynamics each step
	- Runs integrations fast ~ 1khz
- Applies projection
- Outputs Force to device via virtual coupling 

#### Env Interface Class
- Queries the environment 
- Environment initially defined at run time
	- In the future environment updated by simulation
	- Maybe use unity as simple "simulation"
- Used inside proxy to get environment information when updating proxy state

#### Env Class
- Classes that store the actual environment 
- To start just analytical primitive shapes
	- e.g plane env or box env

#### Device Interface Class
- Polls encoder data from the device
	- Calculates and returns end effector positions
- Sends Calculated torque to joints based of force from proxy virtual coupling

### Logger Class

#### Future Classes
- In the future include class that uses equation for robot plan defined in khatib to more accurately model robot dynamics
- Communication or protocol class for sharing snapshots between simulation and haptic loop

