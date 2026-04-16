# Rendering Engine

Part of the [[Implementation_Index]].

## Quick Navigation

- [[#Window and OpenGL Setup]]
- [[#Shader System]]
- [[#MeshGPU]]
- [[#Renderable and MVPUniforms]]
- [[#Pose and Model Matrix]]
- [[#Camera]]
- [[#GlSceneRenderer]]
- [[#Per-Frame Render Loop]]
- [[#Draw Overlays]]
- [[#UI (ImGui)]]
- [[#Geometry and Mesh Registries]]

---

## Window and OpenGL Setup

`Window` (`Rewrite/include/platform/Window.h`) wraps GLFW.

Responsibilities:
- init GLFW, set OpenGL version hints (3.3 core)
- `glfwCreateWindow` + `glfwMakeContextCurrent`
- load OpenGL function pointers with GLAD
- framebuffer resize callback → updates stored size + `glViewport`
- `swap()` / `poll()` for the frame loop
- input helpers: keyboard (`isKeyDown`), mouse button (`isMouseButtonDown`), cursor position
- scroll accumulation via `popScrollY()` — consumed by `ViewportController`

This keeps GLFW details out of the rest of the renderer.

---

## Shader System

`Shader` (`Rewrite/include/render/shader/Shader.h`) handles OpenGL shader programs.

Lifecycle:
1. `loadFile()` reads vertex + fragment source from disk
2. `compile()` compiles each stage, logs errors
3. `link()` links into a program
4. `use()` binds the program for subsequent draws

Uniform API: `setBool`, `setInt`, `setFloat`, `setVec2/3/4`, `setMat3/4`, array variants, `setTexture`.  
Uniform locations are cached after the first lookup to avoid repeated `glGetUniformLocation` calls.

The renderer uses one shader pair:
- `shaders/general.vert` / `shaders/general.frag`

Inputs the shaders expect (assembled by [[#Renderable and MVPUniforms]]):
- `uModel`, `uView`, `uProj` (matrices)
- `uNormalMatrix` (mat3, for lighting)
- `uColour` (vec3)
- `uUseGrid` (float flag, enables grid lines on planes)

---

## MeshGPU

`MeshGPU` (`Rewrite/include/render/gpu/MeshGPU.h`) owns a GPU-resident triangle mesh.

Stores: `VAO`, `VBO`, `EBO`, `count` (index count).

Vertex layout uploaded as interleaved `[pos.x pos.y pos.z nrm.x nrm.y nrm.z]`.

Upload (`upload(vertices, indices)`):
1. gen VAO/VBO/EBO
2. upload float buffer → VBO
3. upload index buffer → EBO
4. configure vertex attrib pointers (location 0 = pos, location 1 = normal)

Draw (`draw()`): bind VAO, `glDrawElements(GL_TRIANGLES, ...)`.

`MeshGPU` is move-only (no copy) to prevent accidental OpenGL handle duplication.  
Instances are managed by [[#Geometry and Mesh Registries]].

---

## Renderable and MVPUniforms

`Renderable` (`Rewrite/include/render/Renderable.h`) is a transient per-draw bundle:
- `mesh*` — pointer to [[#MeshGPU]]
- `shader*` — pointer to [[#Shader System]]
- `colour` — per-object RGB
- `useGridLines` — float flag

`render(cam, model)`:
1. validate mesh + shader not null
2. `shader->use()`
3. build `MVPUniforms` from `cam` and `model`
4. `uniforms.upload(*shader)` — sends all uniforms to GPU
5. `mesh->draw()` — issues draw call

`MVPUniforms` (`Rewrite/include/render/shader/MVPUniforms.h`) holds:
- `model` (mat4) — object → world, from [[#Pose and Model Matrix]]
- `view` (mat4) — world → camera, from [[#Camera]]
- `proj` (mat4) — perspective, from [[#Camera]]
- `colour` (vec3)
- `useGridLines` (float)
- `normalMatrix()` (mat3) — inverse-transpose of upper-left 3×3 of model, for lighting

`upload(Shader&)` writes all of these to the shader uniforms in one call.

---

## Pose and Model Matrix

`Pose` (`Rewrite/include/data/core/Math.h`):
- `p` (Vec3 / dvec3) — world position
- `q` (Quat / dquat) — orientation quaternion
- `s` (double) — uniform scale

`GlSceneRenderer::compose(const Pose&)` converts this to a mat4:
1. normalize quaternion → `glm::mat4_cast` → rotation matrix R
2. `glm::scale(I, vec3(s))` → scale matrix S
3. `M = R * S`
4. `M[3] = vec4(p, 1)` — translation in column 3

This matrix is the `model` fed into [[#Renderable and MVPUniforms]].

---

## Camera

`Camera` (`Rewrite/include/render/Camera.h`):
- `eye`, `front`, `right`, `up`
- `yawDeg`, `pitchDeg` (Euler angles)
- `fovDeg`, `aspect`, `znear`, `zfar`

Key methods:
- `view()` → `glm::lookAt(eye, eye+front, up)`
- `proj()` → `glm::perspective(fovRad, aspect, znear, zfar)`
- `updateVectors()` — recomputes `front`, `right`, `up` from yaw/pitch
- `addYawPitch()` — increments angles, clamps pitch, calls `updateVectors()`

Camera movement is handled by `ViewportController` (WASD + mouse look + scroll zoom) which directly modifies the `Camera` each frame.

---

## GlSceneRenderer

`GlSceneRenderer` (`Rewrite/include/render/GlSceneRenderer.h`) is the central rendering subsystem.

Owns/holds:
- `Window&` — for input, resize, swap/poll
- `Camera` — view/projection state
- `Shader` — the general shader program
- `ImGuiLayer` — ImGui backend setup/frame wrapping
- `UI` — panel logic
- `ViewportController` — camera movement
- `GeometryDatabase&` and `RenderMeshRegistry&` — for geometry/mesh lookup
- Message channels for world/haptic data (see [[Messaging]])

Also stores latest drained state: `latestWorld_`, `latestHaptics_`.

The constructor initialises all of the above, sets camera defaults, and registers UI command callbacks (see [[#UI (ImGui)]]).

---

## Per-Frame Render Loop

`render()` is called every frame from the main thread:

1. **Resize** — query framebuffer size, update `camera_.aspect`
2. **World snapshot** — `worldSnaps_.tryRead(latestWorld_, version_)`
3. **GL state** — depth test on, clear colour + depth buffers
4. **Draw scene objects** — for each `ObjectState` in snapshot:
   - `GeometryDatabase::get(obj.geom)` → geometry entry
   - `RenderMeshRegistry::get(entry.renderMesh)` → GPU mesh
   - build transient `Renderable`, set colour override
   - set `useGridLines = 1` for planes
   - `renderable.render(camera_, compose(obj.T_ws))`
5. **Drain haptic snapshots** — keep only the latest `HapticSnapshotMsg`
6. **Draw Overlays** — see [[#Draw Overlays]]
7. **Viewport controls** — `viewportCtrl_.update()`, consume scroll for zoom
8. **ImGui** — begin frame, build UI state from snapshot, draw panels, end frame
9. **Present** — `window_.swap()` + `window_.poll()`

---

## Draw Overlays

`drawOverlays()` draws debug/haptic visualisation on top of the scene.

Fetches sphere and cube meshes from `RenderMeshRegistry` by `MeshKind`.

Current overlays:
- **Device ghost** (green sphere) — position of physical device end-effector
- **Proxy ghost** (blue sphere) — constrained proxy position (contact-projected)
- **Robot visualisation** — reconstructed from proxy position via inverse kinematics:
  - elbow + end-effector joint positions from forward kinematics
  - link 1 and link 2 drawn as scaled/rotated cubes
  - joints drawn as yellow spheres

All overlays use the same `Renderable`/[[#MeshGPU]] path as scene objects.

The robot overlay is mainly a debug aid for verifying the haptic/control loop is behaving correctly.

---

## UI (ImGui)

UI backend: `ImGuiLayer` handles `ImGui::NewFrame()` / `ImGui::Render()` bookkeeping.

Panels (drawn each frame by `UI`):
- **Debug** — FPS counter
- **Camera** — FOV, yaw/pitch, near/far sliders
- **Controller** — move speed, mouse sensitivity, scroll zoom speed, invert Y, RMB-to-look
- **Body** — object selector, position/scale/colour drag widgets, physics props (density, damping, friction, restitution), "Create new sphere" button

UI interaction model:
- Renderer builds `UITransformState`, `UICameraState`, `UIControllerState`, `UISceneStats` from latest world snapshot + camera state
- UI widgets read from state structs; when a widget changes it calls a callback from `UICommands`
- Callbacks publish `WorldCommand` messages to the world command channel (see [[World System]] and [[Messaging]])
- Camera/controller commands directly modify `Camera` and `ViewportController` state (no message needed)

---

## Geometry and Mesh Registries

These two classes separate geometry *identity* from GPU *resources*.

### GeometryDatabase

`GeometryDatabase` stores `GeometryEntry` records keyed by `GeometryID`.

A `GeometryEntry` bundles:
- `SurfaceType` (Plane, Sphere, Cube, TriMesh)
- `SDF` pointer (for haptic contact queries — see [[Haptic Engine]])
- `physicsShape` handle (for [[Physics Engine]])
- `renderMesh` handle (for this renderer)

This is the **shared geometry contract** across all subsystems.  
See [[World System]] for how objects reference geometry by ID.

### GeometryFactory

`GeometryFactory` is a factory that creates and registers geometry entries.  
For each primitive type it calls `RenderMeshRegistry::getOrCreate(MeshKind)` to get/create the GPU mesh handle, then registers the full entry with `GeometryDatabase`.

### RenderMeshRegistry

`RenderMeshRegistry` owns `MeshGPU` objects, deduplicating by `MeshKind`.

- `getOrCreate(kind)` — returns a handle, building the mesh on first call
- `get(handle)` — returns a pointer to the `MeshGPU`
- Primitive builders: `makePlaneMesh`, `makeSphereMesh`, `makeCubeMesh` — procedurally generate `[pos|nrm]` vertex data and upload to `MeshGPU`

Renderer only stores handles, not direct GPU object references.
