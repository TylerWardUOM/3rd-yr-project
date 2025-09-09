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

class Scene {
public:
    explicit Scene(Window& win, World& world, ISceneRenderer& renderer, Camera& cam);
    ~Scene();
    void run();

    //temp mouse position outputs for haptic loop
    void getMousePos(double& x, double& y) const { win_.getCursorPos(x,y); }

    // Load scene from file (future additon?)
    bool loadFromFile(const std::string& filepath);

    World& world_; // shared state with haptics/physics thread

private:
    // --- core loop functions ---
    // Render the scene
    void render();
    // Update scene
    void update(float dt, bool uiCapturing);


    // --- external references ---
    Window&          win_;
    ISceneRenderer&  renderer_;
    Camera&           cam_;

    // --- owned resources ---
    ImGuiLayer       imgui_;
    UI               ui_;
    ViewportController vpCtrl_;

    // --- scene objects ---
    std::vector<EntityId> entity_ids_;
    // --- UI state snapshots ---
    UITransformState bodyState_{};
    UICameraState    camState_{};
    UISceneStats     stats_{};
    UIPanelConfig    cfg_{};
    UIControllerState ctrlState_{};

    // --- helper functions ---
    void init_Bodies();
    void init_Ui();
};;

#endif