// ViewportController.h
#pragma once

#include "platform/Window.h"
#include "render/Camera.h"

class ViewportController {
public:
    // RenderingEngine owns Window and Camera
    ViewportController(Window& w, Camera& c);

    ViewportController(const ViewportController&) = delete;
    ViewportController& operator=(const ViewportController&) = delete;

    void setViewport(int wpx, int hpx) { width = wpx; height = hpx; }

    // Called once per render frame
    void update(float dt, bool uiCapturing);

    // Scroll callback hook (GLFW → RenderingEngine → here)
    void onScroll(double yoff) { pendingScrollY_ += yoff; }

    // Tunables (exposed to Debug UI)
    void setMoveSpeed(float v)        { moveSpeed_ = v; }
    void setMouseSensitivity(float v) { mouseSensitivity_ = v; }
    void setScrollZoomSpeed(float v)  { scrollZoomSpeed_ = v; }
    void setInvertY(bool v)           { invertY_ = v; }
    void setRmbToLook(bool v)         { rmbToLook_ = v; }

    float moveSpeed()        const { return moveSpeed_; }
    float mouseSensitivity() const { return mouseSensitivity_; }
    float scrollZoomSpeed()  const { return scrollZoomSpeed_; }
    bool  invertY()          const { return invertY_; }
    bool  rmbToLook()        const { return rmbToLook_; }

private:
    Window* win_{nullptr};
    Camera* cam_{nullptr};

    int width{1}, height{1};

    // Input state
    double pendingScrollY_{0.0};
    bool   rmbDown_{false};
    bool   lmbDown_{false};
    double lastX_{0.0}, lastY_{0.0};
    bool   firstMouse_{true};

    // Dragging (purely view manipulation)
    bool  dragging_{false};
    float dragDepth_{1.0f};

    // Internals
    void handleMouseLook(double x, double y);
    void handleMouseDrag(double x, double y);

    // Tunables
    float moveSpeed_         = 0.001f;
    float mouseSensitivity_ = 0.10f;
    float scrollZoomSpeed_  = 1.50f;
    bool  invertY_          = false;
    bool  rmbToLook_        = true;
};
