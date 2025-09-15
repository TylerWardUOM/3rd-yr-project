#pragma once
#include <functional>
#include <string>
#include <glm/vec3.hpp>
#include <optional>
#include "physics/PhysicsProps.h"

using EntityId = uint32_t;

struct UITransformState {
    glm::vec3 position{0.f, 0.f, 0.f};
    glm::vec3 colour{0.8f, 0.8f, 0.8f};
    std::vector<EntityId> entityOptions;     // built from WorldSnapshot each frame
    std::optional<uint32_t>     selectedEntityId;  // currently selected entity
    PhysicsProps physicsProps{}; 
};


struct UICameraState {
    float fovDeg   = 60.f;
    glm::vec3 position{0.f, 0.f, 5.f};
    float znear    = 0.1f;
    float zfar     = 100.f;
    float yawDeg   = -135.f;
    float pitchDeg = -20.f;
};

struct UIControllerState {
    float moveSpeed        = 0.02f; // WASD units per tick
    float mouseSensitivity = 0.10f; // deg per pixel
    float scrollZoomSpeed  = 1.50f; // fov delta per scroll unit
    bool  invertY          = false; // for look
    bool  rmbToLook        = true;  // require RMB to rotate
};

struct UISceneStats {
    float fps = 0.f;
};

// --- Commands the UI can emit (provided by caller) ---
struct UICommands {
    // Body
    std::function<void(float x, float y, float z)> setBodyPosition = {};
    std::function<void(float r, float g, float b)> setBodyColour = {};
    std::function<void(float density)> setBodyDensity = {};
    std::function<void(bool dynamic)> setBodyDynamic = {};
    std::function<void(float linDamping)> setBodyLinDamping = {};
    std::function<void(float angDamping)> setBodyAngDamping = {};
    std::function<void(float staticFriction)> setBodyStaticFriction = {};
    std::function<void(float dynamicFriction)> setBodyDynamicFriction = {};
    std::function<void(float restitution)> setBodyRestitution = {};
    
    std::function<void(EntityId entityId)> setSelectedEntity = {};

    // Camera
    std::function<void(float fovDeg)> setCameraFov = {};
    std::function<void(float nearPlane)> setCameraNear = {};
    std::function<void(float farPlane)> setCameraFar = {};
    std::function<void(float yawDeg, float pitchDeg)> setCameraAngles = {};
    std::function<void(float x, float y, float z)> setCameraPosition = {};

    //Viewport controller
    std::function<void(float)> setMoveSpeed        = {};
    std::function<void(float)> setMouseSensitivity = {};
    std::function<void(float)> setScrollZoomSpeed  = {};
    std::function<void(bool)>  setInvertY          = {};
    std::function<void(bool)>  setRmbToLook        = {};
};

// --- UI Panel configuration ---
struct UIPanelConfig {
    bool showDebug   = true;
    bool showBody    = true;
    bool showCamera  = true;
    bool showMetrics = true;
    bool showController = true;  
    std::string title = "Controls";
};

class UI {
public:
    // Draw all panels. The UI reads *state* and calls *cmds* when user edits widgets.
    void draw(const UIPanelConfig& cfg,
              const UITransformState& bodyState,
              const UICameraState& camState,
              const UIControllerState& ctrlState,
              const UISceneStats& stats,
              const UICommands& cmds);


    void setCommands(const UICommands& cmds) { cmds_ = cmds; }

    // Convenience: split panels if you want independent windows later
    void drawBodyPanel(const UITransformState& bodyStates);
    void drawCameraPanel(const UICameraState& camState);
    void drawControllerPanel(const UIControllerState& ctrlState); 
    void drawDebugPanel(const UISceneStats& stats);

private:
    UICommands cmds_;
};
