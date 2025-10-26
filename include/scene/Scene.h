#ifndef SCENE_H
#define SCENE_H

#include "viz/Window.h"
#include "viz/Camera.h"
#include "viz/Shader.h"
#include "viz/UI/ImGuiLayer.h"

#include "scene/ui/Ui.h"
#include "scene/ui/ViewportController.h"
#include "scene/ISceneRenderer.h"

#include "world/world.h"
#include "haptics/HapticEngine.h" //temp for mouse pos
#include "physics/PhysXEngine.h"

/// @defgroup scene Scene management and rendering
/// @brief Manages the main application loop, scene objects, and rendering

/// @ingroup scene
/// @brief Main scene class: manages the application loop, scene objects, and rendering
class Scene {
public:
    /// @brief Construct a scene with window, world, renderer, camera, and haptic engine references.
    /// @param win Window reference
    /// @param world Shared world reference
    /// @param renderer Scene renderer reference
    /// @param cam Camera reference
    /// @param haptic Haptic engine reference (temp for mouse pos)
    explicit Scene(Window& win, World& world, ISceneRenderer& renderer, Camera& cam, HapticEngine& haptic/*temp for mouse pos*/, PhysicsEnginePhysX& physics);
    
    /// @brief Destructor
    ~Scene();

    /// @brief Main application loop (call on render thread)
    /// @details Runs until the window is closed.
    void run();

    /// @brief Get current mouse position in window coordinates
    /// @param x Output x position
    /// @param y Output y position
    void getMousePos(double& x, double& y) const { win_.getCursorPos(x,y); }

    /// @brief Set the currently selected entity by ID
    /// @param id Entity ID to select
    void setSelected(EntityId id) { selected_ = id; }

    // --- scene management ---
    /// @brief Add a plane to the scene
    /// @param pose Plane pose
    /// @param colour Plane colour
    EntityId addPlane(Pose pose, glm::vec3 colour);

    /// @brief Add a sphere to the scene
    /// @param pose Sphere pose
    /// @param radius Sphere radius
    /// @param colour Sphere colour
    EntityId addSphere(Pose pose, float radius, glm::vec3 colour);

    /// @brief Load a scene from a file (not implemented)
    bool loadFromFile(const std::string& filepath);

    World& world_; ///< Shared world reference

private:
    // --- core loop functions ---
    /// @brief Render the scene
    void render();
    
    /// @brief Update scene state by one timestep
    /// @param dt Timestep in seconds
    /// @param uiCapturing True if UI is capturing input
    void update(float dt, bool uiCapturing);

    EntityId selected_{0}; ///< Currently selected entity ID

    // --- external references ---
    Window&          win_; ///< Window reference
    ISceneRenderer&  renderer_; ///< Scene renderer reference
    Camera&           cam_; ///< Camera reference
    HapticEngine&    haptic_; ///< Haptic engine reference (temp for mouse pos)
    PhysicsEnginePhysX& physics_; ///< Physics engine reference

    // --- owned resources ---
    ImGuiLayer       imgui_; ///< ImGui layer
    UI               ui_; ///< UI manager
    ViewportController vpCtrl_; ///< Viewport controller

    // --- UI state snapshots ---
    UITransformState bodyState_{}; ///< Body panel state
    UICameraState    camState_{}; ///< Camera panel state
    UISceneStats     stats_{}; ///< Scene stats
    UIPanelConfig    cfg_{}; ///< UI panel config
    UIControllerState ctrlState_{}; ///< Controller panel state

    // --- helper functions ---
    void init_Ui(); ///< Initialize UI and bind commands
};;

#endif