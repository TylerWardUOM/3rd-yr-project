// ViewportController.h
#pragma once

#include "viz/Camera.h"
#include "scene/Body.h"

class ViewportController {
public:
    explicit ViewportController(GLFWwindow* w) : win(w) {}

    void setCamera(Camera* c) { cam = c; }
    void setDragTarget(Body* b) { dragTarget = b; } // the sphere
    void setViewport(int wpx, int hpx) { width = wpx; height = hpx; }

    void update(float dt, bool uiCapturing);
    void onScroll(double yoff) { pendingScrollY_ += yoff; } // called by GLFW callback

    // setters hooked from UI
    void setMoveSpeed(float v)        { moveSpeed_ = v; }
    void setMouseSensitivity(float v) { mouseSensitivity_ = v; }
    void setScrollZoomSpeed(float v)  { scrollZoomSpeed_ = v; }
    void setInvertY(bool v)           { invertY_ = v; }
    void setRmbToLook(bool v)         { rmbToLook_ = v; }

    // getters for UI state snapshot
    float moveSpeed()        const { return moveSpeed_; }
    float mouseSensitivity() const { return mouseSensitivity_; }
    float scrollZoomSpeed()  const { return scrollZoomSpeed_; }
    bool  invertY()          const { return invertY_; }
    bool  rmbToLook()        const { return rmbToLook_; }

private:
    GLFWwindow* win{};
    Camera* cam{};
    Body* dragTarget{};
    int width{1}, height{1};
    double pendingScrollY_{0.0};    // accumulated from GLFW callback

    bool rmbDown{false};
    bool lmbDown{false};
    double lastX{0.0}, lastY{0.0};
    bool firstMouse{true};

    bool  dragging_{false};
    float dragDepth_{1.0f}; // t along ray from near-point


    void handleMouseLook(double x, double y);
    void handleMouseDrag(double x, double y);

    float moveSpeed_        = 0.001f;
    float mouseSensitivity_ = 0.10f;
    float scrollZoomSpeed_  = 1.50f;
    bool  invertY_          = false;
    bool  rmbToLook_        = true;

};
