#pragma once
#include <functional>
#include <string>
#include <glm/vec3.hpp>

struct UITransformState {
    glm::vec3 position{0.f, 0.f, 0.f};
};


struct UICameraState {
    float fovDeg   = 60.f;
    float znear    = 0.1f;
    float zfar     = 100.f;
    float yawDeg   = -135.f;
    float pitchDeg = -20.f;
};

struct UISceneStats {
    float fps = 0.f;
    int   drawCalls = 0;
    int   triangles = 0;
};

// --- Commands the UI can emit (provided by caller) ---
struct UICommands {
    // Body
    std::function<void(float x, float y, float z)> setBodyPosition = {};

    // Camera
    std::function<void(float fovDeg)> setCameraFov = {};
    std::function<void(float nearPlane)> setCameraNear = {};
    std::function<void(float farPlane)> setCameraFar = {};
    std::function<void(float yawDeg, float pitchDeg)> setCameraAngles = {};
};

// --- UI Panel configuration ---
struct UIPanelConfig {
    bool showDebug   = true;
    bool showBody    = true;
    bool showCamera  = true;
    bool showMetrics = true;
    std::string title = "Controls";
};

class UI {
public:
    // Draw all panels. The UI reads *state* and calls *cmds* when user edits widgets.
    void draw(const UIPanelConfig& cfg,
              const UITransformState& bodyState,
              const UICameraState& camState,
              const UISceneStats& stats,
              const UICommands& cmds);


    void setCommands(const UICommands& cmds) { cmds_ = cmds; }

    // Convenience: split panels if you want independent windows later
    void drawBodyPanel(const UITransformState& bodyState, const UICommands& cmds);
    void drawCameraPanel(const UICameraState& camState, const UICommands& cmds);
    void drawDebugPanel(const UISceneStats& stats);

private:
    UICommands cmds_;
};
