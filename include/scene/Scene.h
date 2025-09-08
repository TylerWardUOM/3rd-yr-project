#ifndef SCENE_H
#define SCENE_H

#include "viz/Window.h"
#include "viz/Camera.h"
#include "viz/Shader.h"
#include "viz/UI/ImGuiLayer.h"

#include "meshing/Mesher.h"

#include "scene/ui/Ui.h"
#include "scene/ui/ViewportController.h"

#include "env/primitives/SphereEnv.h"
#include "env/primitives/PlaneEnv.h"

#include "world/world.h"

class Scene {
public:
    explicit Scene(Window& win, World& world);
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

    // --- owned resources ---
    Window&          win_;
    Camera           cam_;
    Shader           shader_;
    Mesher           mesher_;
    ImGuiLayer       imgui_;
    UI               ui_;
    ViewportController vpCtrl_;

    // --- scene objects ---
    std::vector<BodyId> body_ids_;
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