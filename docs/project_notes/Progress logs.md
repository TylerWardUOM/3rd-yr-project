---

---
---
# 06/09/2025:  *7hr 30min*

---
### Created Interface and Implementations for Primitive Env Shapes [Git Commit](https://github.com/TylerWardUOM/3rd-yr-project/commit/3446893f1678eb3afbb7deaa3af005d03d9802e9)
- Created a Environment Interface class for interacting with shapes in the environment
- This is based on [[Environment]]
- Environment Interface contains virtual functions for querying the SDF of a shape such as:
- `phi(Point x)` and `grad(point x)` these functions return the SDF value at a point as well as the gradient at a point
- Also contains `Point Project(Point x)` this returns coordinates of a point projected to the shapes surface.
- Created a implementation of this interface for a simple plane.

### Learnt OpenGl Basics [Git Commit](https://github.com/TylerWardUOM/3rd-yr-project/commit/c343b9abb0a84fdc95ca317d838d5b1d977f2ce6)
Followed along https://learnopengl.com/ tutorial to learn openGl basics.
- Learnt how to create and manage a window using GLFW
- Learnt how to import and use GLAD to give access to OpenGl API 
- Learnt about VBO/EBO buffers and uploading vertex/index data to GPU
- Learnt about VAO and using them to define attrivutes and their locations in the VBO
- Learnt about the graphics pipeline with vertex and fragment shaders
- Learnt about Vertex Shader taking in attributes from VBO/VAO and converting vertex cords into clip space coordinates and also passing on other attributes to later stages in pipeline e.g fragment shader
- Learnt about using fragment shader to compute colour of each pixel using inputs passed through by vertex shader
- Put what i learnt together to create a basic Hello-Triangle render

### Encapsulated some OpenGl boiler Plate into Shader and Window Class [Git Commit](https://github.com/TylerWardUOM/3rd-yr-project/commit/43aab7c6a8053429d258867974bf2054cf55fb2c)
After working through tutorial I then began to encapsulate and abstract the complexities of using OpenGl.
- Created Shader Class
	- Shader class reads and complies GLSL shader code from a specified file path
		- It returns errors if shader compilation fails
	- Shader class provides API to abstract handling OpenGl shaders such as:
	- Function to set GPU to use the shader
		- `Shader::use()`
	- Along with setters to set shader uniforms e.g.:
		- `Shader::setBool((const std::string &name, bool value))`
- Created Window Class
	- Window Class handles initializing GLFW and creating window
	- Window Class handled initializing GLAD
	- Window Class also handles GLFW callbacks such as framebuffer resize
	- Window class also provides a simple API to wrap GLFW functions e.g.:
		- `Window::isOpen()`
		- `Window::poll()`
		- `Window::swap()`
		- and more.


# 07/09/2025: *8hr*
---
### Create implementation of Marching Cubes to generate Meshes [Git Commit](https://github.com/TylerWardUOM/3rd-yr-project/commit/8e30040b56eb540ef45195f109dde225045ff3d5)
- Learnt and Implemented the Marching Cubes algorithm to generate meshes from SDF functions.
- Created Mesher class to abstract Marching Cubes and also improved Marching cubes to use Environment Interface [Git Commit](https://github.com/TylerWardUOM/3rd-yr-project/commit/2b886c8b77e1718211e1c53b784cf34f08b1b2e5)
### Implement Transform and Renderable structures; add MeshGPU class for GPU mesh handling; update shader inputs; define camera and MVPUniforms; enhance Mesher function; add vertex and fragment shaders [Git Commit](https://github.com/TylerWardUOM/3rd-yr-project/commit/2b673f252ccc7a83c4dcab26099d9eed8d3a33e6 )
#### MeshGPU
- Created MeshGPU class to abstract complexities of VBO/EBOs and VAO 
- MeshGPU provides APIs to simplify adding meshes to GPU such as:
	- Function to upload vertices and other data to GPU
	- Function to draw current mesh
#### MVPUniforms
- Created MVPUniforms this is a struct that stores the model, view and projection matrices required by vertex shader to convert points to clip space
- MVPUniforms also contains a function to use a specify shader and upload these matrices to the shaders uniforms.
#### Transform
- Basic wrapper for 4x4 transformation matrix for models.
#### Camera
- Created a basic camera struct that stores things like camera eye position vectors representing cameras front, right and up
- Camera also stores FOV and aspect ratio
- Camera also stores yaw and pitch and contains a basic function to update the front, right and up vectors based on the yaw and pitch.
- Camera contains functions to calculate and return view and projection matrices for rendering.
#### Renderable 
- Renderable is a struct that represents each Renderable object it stores MeshGPU, Transform and Shaders.
- Renderable has a render function that takes in a reference to a camera and then uses MVPUniforms to upload transformed model matrix along with view and projection matrix it gets from the camera before uploading shader to GPU and calling the MeshGPU draw function.





### Add SphereEnv Class [Git Commit](https://github.com/TylerWardUOM/3rd-yr-project/commit/601bc337afdc4c2ba503a1293bd1b79db417b788)
- Added a new SphereEnv class works the same as PlaneEnv from previous day
### Create Body Class [Git commit](https://github.com/TylerWardUOM/3rd-yr-project/commit/c82eb532d41d765d21b10791309777d83618f9a9)

### UI and Controls

Implemented the ImGUI library to create GUI elements to interact with the environment
- Created a wrapper to provide simpler API for using ImGUI and abstract boilerplate [Git Commit](https://github.com/TylerWardUOM/3rd-yr-project/commit/70b6a67a859aafa0b9aff884e2627bd37600ab87)
- Integrate ImGUI into existing code creating controls to interact with camera and body [Git Commit](https://github.com/TylerWardUOM/3rd-yr-project/commit/aec8b2467c7747f5b7b17bf5990925acf240834a)
Create Viewport Controller class for handling user inputs in the viewport [Git Commit](https://github.com/TylerWardUOM/3rd-yr-project/commit/dd7f6bf90cd227fbd6d57730bb1b1e9bb04576ad)[Git Commit 2](https://github.com/TylerWardUOM/3rd-yr-project/commit/83dd7edb6bc632ee74c8f1d095ab265a099c0876)
- This Viewport controller allowed use of arrow keys to move the camera around the world.
- Added holding down right click to look around the scene and while holding scroll to zoom.
- Added a simple ray to project when left click was pressed and move the body to a position along the ray.
- The distance along ray can be adjusted by scrolling.
# 08/09/2025: *7hr 30min*
---
To start this day I went back and refactored previous code such as Window and Viewport Controller this was to ensure that Viewport controller did not need to use any GLFW functions and could work with any window. [Git commit](https://github.com/TylerWardUOM/3rd-yr-project/commit/6ac07573bc5ebf7840b29ef876f53b3551103bf5)

This then allowed me to create a agnostic scene class that managed all aspects of rendering the scene. [Git Commit](https://github.com/TylerWardUOM/3rd-yr-project/commit/dd3e917925387d2958979f33ead237884912e5a4)

I then realised that Scene should not own or manage the bodies themselves it should be read only as it only handles rendering whereas body movement and other logic should be handled in other threads. I then did a big refactor created a World struct that owned the poses of each body and could be updated using a buffer to ensure thread safety. [Git Commit](https://github.com/TylerWardUOM/3rd-yr-project/commit/6fed938147df4a72c953fdc2ec71f528c7952d6b)

# 09/09/2025: *7hr 45min*
---
This day started with me continuing on with the refactor that introduced the World struct from the previous day. I adjusted my approach to using a WorldSnapshot buffer.
### World Refactor cont. [Git commit](https://github.com/TylerWardUOM/3rd-yr-project/commit/72a4c239261a0ca902e72ba4737a6b7f81d6ad72)
The WorldSnapshot contains contains a vector of information on each surface. Each surface has a EntityId this is used determine what entity the surface belongs to and will in future be used for other entity information lists such as physics properties. 

The information about each surface that is stored is its Type e.g. Plane, Sphere...  its Pose and then additional information based on its type e.g. sphere.radius

World class contains methods to add entities to the world set pose or translate/rotate a given entity based of its Id. The world class also contains thread safe methods to publish and read snapshots this uses a double buffer.
### Renderer Refactor [Git Commit](https://github.com/TylerWardUOM/3rd-yr-project/commit/c4cad6f593def3ea0892a75459880754eb066c76)
Continuing on with the refactoring I realised that the Scene class was too tied to OpenGl and was also handling a lot of the rendering logic instead of just managing the scene itself.

I created a Scene Renderer interface this defined a basic API that Scene could used and contained a bunch of virtual functions. I then created the actual implementation of this Gl Scene Renderer that inherited from the interface.

This refactor was very large as I wanted the Scene Renderer to take in just the World Snapshot and render the scene based on this without World Snapshot having to contain the meshes and Renderable objects. To do this I made the Renderer in its constructor create MeshGPU objects form the primitive shapes I am using that it owns for its lifetime. 
The render flow now looks like this.
- Render Called
- World Snapshot read
- Loop through all surfaces in snapshot and do:
	- Compose Model matrix from surface.pose
	- Determine what surface type and call relevant helper draw function.
		- Reads additional surface information:
			- colour, radius etc...
		- Creates Renderable object assign mesh shader and other parameters
		- Run Renderable.render()
[Git Commit](https://github.com/TylerWardUOM/3rd-yr-project/commit/c4cad6f593def3ea0892a75459880754eb066c76#diff-40fa99b4ab7c43199dbad05165525714bef5258ce1e1977fe7ab03ccfb19970d)
### Colour Handling [Git Commit]
Added ability to set colour of body by adding to MVPUniforms to include a colour vector.
Colour Uniform passed from Vertex Shader to Fragment Shader for now just tints the colour which is based on surface normal
### Ui Upgrades
Improved the UI to allow user to select which body they are interacting with this was done by listing all entity ids from world snapshot. 
Also added ability to set colour of selected body in body controls ui.
### Basic Haptic Engine start [Git Commit](https://github.com/TylerWardUOM/3rd-yr-project/commit/1479c5afedb9e5f70db31d0075bbeb2cb4a220b0)
Created the start of my Haptic Engine.
Made it so that a entity could be defined a role such as tool ref or proxy. 
In the haptic loop it would read the tool Pose and also the World Snapshot and would loop through all surfaces in the snapshot and create primitive SDF at its location using EnvInterface created on the 6th. It would then check to see if the tool/reference lay inside one of these shapes and if so it would set the proxy pose to the surface of that shape if not proxy just tracked the reference. 
# 12/09/2025: *5hr 30min*
--- 
At the start of this day I wasted time going down the route of having the World Snapshot owning tool, reference and proxy pose and the entities that these belong to being set manually. [Git Commit](https://github.com/TylerWardUOM/3rd-yr-project/commit/b3cb3f622326958e77982801edc51f40d741e10f)

This began to get a bit complicated and required stepping back for a bit to remember my initial plan. The plan is that World Snapshot is only written to by the physics engine and debug features. This meant I should create a separate Haptic buffers to handle tool inputs such as device position and reference position and tool out containing the proxy position along with force to be sent to the device. There is also a HapticSnapshot that contains device, ref and proxy positions and force. 

This Haptic Snapshot is read by the renderer, which I added  some functions to allowing it to draw overlays at these points as basic spheres of varying colours. [Git commit](https://github.com/TylerWardUOM/3rd-yr-project/commit/c7d48d40ba6d1f398489f5991ff655736343fa79)

### Basic Physics Engine start  [Git Commit](https://github.com/TylerWardUOM/3rd-yr-project/commit/29770d5601b6d0bc9bd92a523bbcf352a19b2d0d)
This day I also started on a basic physics engine the plan is to use PhysX but I wanted to first design interface with physics engine and rest of code first.

I created a PhysicsBuffer this for now just contains a PhysicsCommands double buffer the haptic engine has a pointer to this and writes commands to it each haptic loop.

For now I have a general apply wrench at point command that applies a force to a body that the tool contacts.

The force that is applied is equal and opposite to the virtual coupling force between the proxy and the tool/reference which I am now calculating.

I implemented virtual coupling force this required me first to improve the collision detection and surface projection to find the best point to project to if reference is inside multiple shapes. I then defined some initial K and D parameters for the virtual coupling equation and used this to calculate force.
# 13/09/2025: *3hr*
- Reviewed previous work improved comments on existing code and then compiled logs for all previous days.
# 15/09/2025: 5hr
---
### Doxygen Documentation [Git Commit](https://github.com/TylerWardUOM/3rd-yr-project/commit/8700b3a1b8f2059ec9c32a0465067fed7554c7e3)
- Started the day adding Doxygen comments to code.
- Added comments to `haptic/`, `env/` and some of `scene/`
### Rendering Enhancements [Git Commit](https://github.com/TylerWardUOM/3rd-yr-project/commit/125a3ee40bd0936737fda01e4babad4c31858f0c)
- Added support for 1 metre x 1 metre grid lines to be rendered onto planes
- Enhanced fragment shader to improve colour rendering using simple diffusion and fixed light direction.
### Bug Fix [Git Commit](https://github.com/TylerWardUOM/3rd-yr-project/commit/ab3b4c7ceeb0499b9260022daff7baa28279ba5d)
- Fixed bug with incorrect radius of unit sphere in `GLSceneRenderer`
### PhysX Implementation 
- Fixed plane clipping by having PhysX treat it as a think box
- Changed Physics Commands to be applied as impulse
- Improved handling of `PhysicsProps`
- Added flag to rebuild actors if changed via debug UI.

# 20/09/2025: 2hr 30
---
### Add Robot Overlay [Git Commit](https://github.com/TylerWardUOM/3rd-yr-project/pull/11)
- Added Inverse and Forward Kinematics to compute joint angles and positions
- Added a cylinder mesh to renderer
- Render cylinders to represent proxy robot joint and link poses