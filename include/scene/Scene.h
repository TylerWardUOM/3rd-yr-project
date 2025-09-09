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

class Scene {
public:
    explicit Scene(Window& win, World& world, ISceneRenderer& renderer, Camera& cam, HapticEngine& haptic/*temp for mouse pos*/);
    ~Scene();
    void run();

    //temp mouse position outputs for haptic loop
    void getMousePos(double& x, double& y) const { win_.getCursorPos(x,y); }

    void setSelected(EntityId id) { selected_ = id; }

    // --- scene management ---
    EntityId addPlane(Pose pose, glm::vec3 colour);
    EntityId addSphere(Pose pose, float radius, glm::vec3 colour);

    // Load scene from file (future additon?)
    bool loadFromFile(const std::string& filepath);

    World& world_; // shared state with haptics/physics thread

private:
    // --- core loop functions ---
    // Render the scene
    void render();
    // Update scene
    void update(float dt, bool uiCapturing);

    EntityId selected_{0}; // currently selected entity

    // --- external references ---
    Window&          win_;
    ISceneRenderer&  renderer_;
    Camera&           cam_;
    HapticEngine&    haptic_; // temp for mouse pos

    // --- owned resources ---
    ImGuiLayer       imgui_;
    UI               ui_;
    ViewportController vpCtrl_;

    // --- UI state snapshots ---
    UITransformState bodyState_{};
    UICameraState    camState_{};
    UISceneStats     stats_{};
    UIPanelConfig    cfg_{};
    UIControllerState ctrlState_{};

    // --- helper functions ---
    void init_Ui();
};;

#endif