// ViewportController.cpp
#include "scene/ui/ViewportController.h"
#include "util/CameraUtils.h"

void ViewportController::update(float /*dt*/, bool uiCapturing) {
    if (!cam || !win) return;

    // Update aspect each frame (in case of resize)
    int fbw=0, fbh=0; glfwGetFramebufferSize(win, &fbw, &fbh);
    if (fbw>0 && fbh>0) {
        cam->aspect = float(fbw)/float(fbh); 
        setViewport(fbw, fbh); 
    }

    // Keyboard (WASD) — always on if not captured
    if (!uiCapturing) {
        if (glfwGetKey(win, GLFW_KEY_W) == GLFW_PRESS) cam->eye += cam->front * moveSpeed_;
        if (glfwGetKey(win, GLFW_KEY_S) == GLFW_PRESS) cam->eye -= cam->front * moveSpeed_;
        if (glfwGetKey(win, GLFW_KEY_A) == GLFW_PRESS) cam->eye -= cam->right * moveSpeed_;
        if (glfwGetKey(win, GLFW_KEY_D) == GLFW_PRESS) cam->eye += cam->right * moveSpeed_;
        if (glfwGetKey(win, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) cam->eye -= cam->camUp * moveSpeed_;
        if (glfwGetKey(win, GLFW_KEY_SPACE) == GLFW_PRESS)      cam->eye += cam->camUp * moveSpeed_;
    }

    // Mouse buttons
    int rmb = glfwGetMouseButton(win, GLFW_MOUSE_BUTTON_RIGHT);
    int lmb = glfwGetMouseButton(win, GLFW_MOUSE_BUTTON_LEFT);

    if (!uiCapturing) {
        // --- RMB handling
        if (rmb == GLFW_PRESS && !rmbDown) {
            // Start look
            rmbDown = true; firstMouse = true;
            glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // lock cursor while looking
        } else if (rmb == GLFW_RELEASE && rmbDown) {
            // End look
            rmbDown = false;
            glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }

        // --- LMB handling
        if (lmb == GLFW_PRESS && !lmbDown) {
            lmbDown   = true;
            dragging_ = true;

            double x, y; glfwGetCursorPos(win, &x, &y); // current mouse pos

            // Ray through cursor
            Ray ray = makeRayAtCursor(win, x, y, width, height, *cam); 

            // ----- Get drag target position
            glm::dvec3 spherePos;
            if (dragTarget) {
                spherePos = dragTarget->getPosition();             // dvec3
            } else {
                spherePos = glm::dvec3(cam->eye);                  // vec3 -> dvec3
            }

            // --- Convert ray to double precision (maybe change)
            glm::dvec3 rayOrigin = glm::dvec3(ray.o);              // vec3 -> dvec3
            glm::dvec3 rayDir    = glm::normalize(glm::dvec3(ray.d));

            // --- Measure depth along ray to spherePos and store as dragDepth_
            glm::dvec3 camToSphere = spherePos - rayOrigin; // vector from ray origin to sphere center
            double depth = glm::dot(camToSphere, rayDir); // project onto ray direction
            dragDepth_ = (depth > 0.05) ? static_cast<float>(depth) : 0.05f; // keep positive & store as float

        } else if (lmb == GLFW_RELEASE && lmbDown) {
            // End drag
            lmbDown   = false;
            dragging_ = false;
        }

    } else {
        // If UI is capturing, ensure we’re not in a locked state
        if (rmbDown) { rmbDown = false; glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_NORMAL); }
        lmbDown   = false;
        dragging_ = false;
        firstMouse = true;
    }

    // Mouse position
    double x, y; glfwGetCursorPos(win, &x, &y);

    if (rmbDown) handleMouseLook(x, y);
    if (lmbDown) handleMouseDrag(x, y);

// --- Scroll handling ---
    if (pendingScrollY_ != 0.0) {
        // Consume and reset scroll
        double sy = pendingScrollY_;
        pendingScrollY_ = 0.0;

        if (!uiCapturing) {
            if (rmbDown) {
                // Zoom camera via FOV (clamped)
                cam->fovDeg = glm::clamp(cam->fovDeg - float(sy) * scrollZoomSpeed_, 20.f, 90.f);
            } else if (lmbDown && dragging_) {
                // --- Adjust drag distance from camera
                Ray ray = makeRayAtCursor(win, x, y, width, height, *cam);

                // Scale step with current depth for nicer feel
                float step = float(sy) * 0.2f * glm::max(0.5f, dragDepth_); // proportional step
                dragDepth_ = glm::max(0.05f, dragDepth_ + step); // keep positive

                glm::vec3 pos = ray.o + dragDepth_ * ray.d; // new position along ray
                if (dragTarget) dragTarget->setPosition(glm::dvec3(pos));
            }
        }
    }
}

void ViewportController::handleMouseLook(double x, double y) {
    if (firstMouse) { 
        firstMouse = false; 
        lastX = x; lastY = y; 
    };
    float dx = float(x - lastX);
    float dy = float(lastY - y); // invert Y
    lastX = x; lastY = y;

    cam->addYawPitch(dx * mouseSensitivity_, dy * mouseSensitivity_);
}

void ViewportController::handleMouseDrag(double x, double y) {
    if (!dragTarget || !cam || width<=0 || height<=0) return;

    // Ray through cursor
    Ray ray = makeRayAtCursor(win, x, y, width, height, *cam);

    // Place object at current dragDepth_ along the ray
    glm::vec3 pos = ray.o + dragDepth_ * ray.d;
    dragTarget->setPosition(glm::dvec3(pos));
}