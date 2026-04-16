# Rendering Engine Implementation (Rewrite / Clean-Spaghetti)

This note captures how I implemented rendering in the Rewrite architecture (matching the Clean-Spaghetti version).  
It is written for my report’s system implementation section and focuses on **how the parts connect**, not low-level OpenGL trivia.

## Quick Navigation

- [[#System Context]]
- [[#Window and OpenGL Setup]]
- [[#Shader System]]
- [[#MeshGPU (GPU Mesh Abstraction)]]
- [[#Renderable + MVPUniforms]]
- [[#Pose and Model Matrix]]
- [[#Camera]]
- [[#Main Renderer (GlSceneRenderer)]]
- [[#Per-Frame Render Loop]]
- [[#Draw Overlays]]
- [[#UI Integration (ImGui)]]
- [[#Geometry Data Flow (GeometryDatabase + GeometryFactory + RenderMeshRegistry)]]
- [[#Why This Design Works]]

## System Context

Rendering is one subsystem in a multi-threaded architecture:
- world simulation produces [[#World Snapshot]]
- haptics produces tool/proxy state used by [[#Draw Overlays]]
- renderer consumes both streams to draw the current frame

Main wiring is in `Rewrite/src/main.cpp`, where:
- `Window` is created
- `GlSceneRenderer` is constructed
- `GeometryDatabase` and `RenderMeshRegistry` are shared with renderer/world systems
- the render loop calls `renderer.render()` while the window is open

This keeps rendering mostly read-only with respect to world state, and command-based when UI edits are made (see [[#UI Integration (ImGui)]]).

## Window and OpenGL Setup

`Window` (`Rewrite/include/platform/Window.h`, `Rewrite/src/platform/Window.cpp`) is my GLFW wrapper.

It handles:
- GLFW init and OpenGL context creation
- GLAD loading
- swap/poll lifecycle (`swap()`, `poll()`)
- framebuffer resize callback + `glViewport`
- input wrappers for keyboard/mouse
- scroll accumulation (`popScrollY`) for camera zoom

This wrapper keeps GLFW-specific code out of the renderer and controller logic.  
`GlSceneRenderer` uses the wrapper directly for framebuffer size, event pumping, and presentation (see [[#Per-Frame Render Loop]]).

## Shader System

`Shader` (`Rewrite/include/render/shader/Shader.h`, `Rewrite/src/render/shader/Shader.cpp`) abstracts shader program management.

Key responsibilities:
- load shader source from file
- compile vertex/fragment shaders with error reporting
- link and own program ID
- bind program via `use()`
- expose typed uniform setters (`setBool`, `setInt`, `setFloat`, `setVec*`, `setMat*`, array setters)
- cache uniform locations (`getLocation`) to avoid repeated lookups

In this renderer, the main program is loaded in `GlSceneRenderer` as:
- `shaders/general.vert`
- `shaders/general.frag`

The shader pair expects:
- `uModel`, `uView`, `uProj`, `uNormalMatrix`
- `uColour`, `uUseGrid`

Those values are assembled per object through [[#Renderable + MVPUniforms]].

## MeshGPU (GPU Mesh Abstraction)

`MeshGPU` (`Rewrite/include/render/gpu/MeshGPU.h`, `Rewrite/src/render/gpu/MeshGPU.cpp`) owns OpenGL mesh resources:
- `VAO` (vertex layout state)
- `VBO` (interleaved vertex buffer)
- `EBO` (index buffer)
- `count` (index count for draw call)

Uploaded layout is interleaved:
- `[pos.x pos.y pos.z normal.x normal.y normal.z]`

Upload flow:
1. create VAO/VBO/EBO
2. upload vertex/index data
3. configure attributes:
   - location 0 = position
   - location 1 = normal

Draw flow:
- bind VAO
- `glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, 0)`

`MeshGPU` is move-only, which avoids accidental OpenGL handle duplication.  
It is used by [[#Renderable + MVPUniforms]] and managed at system level by [[#Geometry Data Flow (GeometryDatabase + GeometryFactory + RenderMeshRegistry)]].

## Renderable + MVPUniforms

`Renderable` (`Rewrite/include/render/Renderable.h`) is a lightweight per-draw bundle:
- `mesh` pointer
- `shader` pointer
- per-object `colour`
- `useGridLines` flag

`render(cam, model)` does:
1. validate mesh/shader
2. bind shader
3. build `MVPUniforms`
4. fill `model`, `view`, `proj`, colour, grid flag
5. upload uniforms
6. draw mesh

`MVPUniforms` (`Rewrite/include/render/shader/MVPUniforms.h`) is the per-draw shader input container:
- `model`: object-to-world transform
- `view`: world-to-camera transform
- `proj`: perspective projection
- `colour`: per-object colour
- `useGridLines`: enables plane grid shading
- derived `normalMatrix()`: inverse-transpose of model 3x3

`upload(Shader&)` writes all required uniforms for `general.vert`/`general.frag`.

## Pose and Model Matrix

`Pose` (`Rewrite/include/data/core/Math.h`) stores object transform state:
- position `p` (`Vec3`)
- orientation quaternion `q` (`Quat`)
- uniform scale `s`

`GlSceneRenderer::compose(const Pose&)` converts this to a model matrix:
- normalize quaternion and build rotation matrix
- build uniform scale matrix
- combine as `R * S`
- set translation in matrix column 3

That model matrix is passed into [[#Renderable + MVPUniforms]] for final shader upload.

## Camera

`Camera` (`Rewrite/include/render/Camera.h`) stores:
- `eye`, `front`, `right`, `up`
- `yawDeg`, `pitchDeg`
- `fovDeg`, `aspect`, `znear`, `zfar`

Core outputs:
- `view()` via `glm::lookAt(eye, eye+front, up)`
- `proj()` via `glm::perspective(...)`

Camera orientation is maintained through:
- `updateVectors()`
- `clampPitch()`
- `addYawPitch()`

Camera control is driven by `ViewportController` during rendering (see [[#Per-Frame Render Loop]] and [[#UI Integration (ImGui)]]).

## Main Renderer (GlSceneRenderer)

`GlSceneRenderer` (`Rewrite/include/render/GlSceneRenderer.h`, `Rewrite/src/render/GlSceneRenderer.cpp`) is the central render subsystem.

It owns/uses:
- `Window`
- `Camera`
- `Shader`
- `ImGuiLayer` + `UI`
- `ViewportController`
- read channels/snapshots for world/haptics
- references to `GeometryDatabase` and `RenderMeshRegistry`

High-level responsibility:
- consume latest world + haptic state
- draw scene geometry
- draw debug overlays
- run viewport camera controls
- draw ImGui panels
- present frame

## Per-Frame Render Loop

`GlSceneRenderer::render()` sequence:

1. refresh framebuffer size and camera aspect
2. read latest world snapshot from `worldSnaps_` → [[#World Snapshot]]
3. set OpenGL state (`depth test`, clear buffers)
4. iterate objects in snapshot:
   - lookup geometry entry (`GeometryDatabase`)
   - lookup GPU mesh (`RenderMeshRegistry`)
   - build transient [[#Renderable + MVPUniforms]]
   - set colour override
   - enable gridlines for plane surfaces
   - render using `compose(obj.T_ws)`
5. consume latest haptic snapshot(s)
6. call [[#Draw Overlays]]
7. update viewport controls + apply scroll zoom
8. draw ImGui panels (debug/camera/controller/body)
9. swap buffers + poll events

This gives a clean “data in → frame out” path each tick.

## Draw Overlays

`drawOverlays()` adds visual debug/feedback elements on top of world objects.

Current overlays include:
- ghost markers for haptic device/proxy positions (spheres)
- simple robot visualization built from:
  - spheres for joints
  - cubes stretched/oriented as link segments

Overlay meshes come from `RenderMeshRegistry` (`Sphere`, `Cube`), then rendered with the same [[#Renderable + MVPUniforms]] path for consistency.

This is useful for validating haptic/control behavior in real time without needing a separate debug renderer.

## UI Integration (ImGui)

UI is built with:
- `ImGuiLayer` for backend/frame setup
- `UI` panels (`Rewrite/include/render/UI/Ui.h`, `Rewrite/src/render/UI/Ui.cpp`)

Panels used in-frame:
- Debug panel (FPS)
- Camera panel
- Controller panel
- Body/object editing panel

UI interaction model:
- renderer builds UI state from latest world snapshot
- UI emits commands through callbacks (`UICommands`)
- callbacks publish `WorldCommand` messages (e.g., pose/colour/physics edits, create sphere)

So UI is not directly mutating world storage; it sends commands into the same world command path used elsewhere.

## Geometry Data Flow (GeometryDatabase + GeometryFactory + RenderMeshRegistry)

### GeometryDatabase

`GeometryDatabase` stores `GeometryEntry` records keyed by `GeometryID`.

A `GeometryEntry` ties together:
- logical surface type (`Plane`, `Sphere`, `Cube`, ...)
- haptics SDF pointer
- physics shape handle
- render mesh handle

This makes geometry metadata a shared cross-system contract.

### GeometryFactory

`GeometryFactory` creates/registers default geometry entries and asks `RenderMeshRegistry` for the corresponding mesh handles.

Examples:
- plane → `MeshKind::Plane`
- sphere → `MeshKind::Sphere`
- cube → `MeshKind::Cube`

### RenderMeshRegistry

`RenderMeshRegistry` owns actual `MeshGPU` instances and deduplicates by `MeshKind`.

It:
- lazily creates meshes on first request (`getOrCreate`)
- returns stable handles
- supports procedural primitive mesh builders (plane/sphere/cube)

This separation means objects/world data only store geometry IDs, not raw OpenGL objects.

## World Snapshot

`WorldSnapshot` (`Rewrite/include/data/WorldSnapshot.h`) is what rendering consumes each frame.

For each `ObjectState`, renderer uses:
- `geom` for geometry lookup
- `T_ws` pose for model matrix
- `colourOverride` for shading

This keeps rendering decoupled from simulation internals and aligned with the messaging architecture.

## Why This Design Works

The implementation is effective because responsibilities are layered:
- **Window/GL setup** is isolated in `Window`
- **GPU resource details** are isolated in `Shader` + `MeshGPU`
- **Per-draw binding logic** is isolated in `Renderable` + `MVPUniforms`
- **Scene orchestration** lives in `GlSceneRenderer`
- **Geometry identity vs GPU mesh storage** is split across `GeometryDatabase` and `RenderMeshRegistry`
- **UI edits** are command-driven rather than direct state mutation

That makes the renderer reusable, easier to extend, and easier to explain in a report.
