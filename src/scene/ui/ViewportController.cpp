// ViewportController.cpp
#include "scene/ui/ViewportController.h"
#include <glm/gtc/matrix_transform.hpp>
#include "viz/Ray.h"

void ViewportController::update(float /*dt*/, bool uiCapturing) {
    if (!cam || !win) return;

    // Update aspect each frame (in case of resize)
    int fbw=0, fbh=0; glfwGetFramebufferSize(win, &fbw, &fbh);
    if (fbw>0 && fbh>0) { cam->aspect = float(fbw)/float(fbh); setViewport(fbw, fbh); }

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
        if (rmb == GLFW_PRESS && !rmbDown) {
            rmbDown = true; firstMouse = true;
            glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // lock cursor while looking
        } else if (rmb == GLFW_RELEASE && rmbDown) {
            rmbDown = false;
            glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }

        // LMB transitions + initialise drag depth on press
        if (lmb == GLFW_PRESS && !lmbDown) {
            lmbDown   = true;
            dragging_ = true;

            // Cursor → NDC
            double x, y; glfwGetCursorPos(win, &x, &y);
            float ndcX =  2.f * float(x) / float(width)  - 1.f;
            float ndcY = -2.f * float(y) / float(height) + 1.f;
            glm::vec2 ndc(ndcX, ndcY);

            // Ray through cursor
            glm::mat4 invVP = glm::inverse(cam->proj() * cam->view());
            Ray ray = rayFromNDC(ndc, invVP);

            // ----- Avoid ternary: pick spherePos explicitly as dvec3
            glm::dvec3 spherePos;
            if (dragTarget) {
                spherePos = dragTarget->getPosition();             // dvec3
            } else {
                spherePos = glm::dvec3(cam->eye);                  // vec3 -> dvec3
            }

            glm::dvec3 rayOrigin = glm::dvec3(ray.o);              // vec3 -> dvec3
            glm::dvec3 rayDir    = glm::normalize(glm::dvec3(ray.d));

            glm::dvec3 camToSphere = spherePos - rayOrigin;
            double depth = glm::dot(camToSphere, rayDir);

            // keep positive & store as float
            dragDepth_ = (depth > 0.05) ? static_cast<float>(depth) : 0.05f;

        } else if (lmb == GLFW_RELEASE && lmbDown) {
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
        double sy = pendingScrollY_;
        pendingScrollY_ = 0.0;

        if (!uiCapturing) {
            if (rmbDown) {
                // Zoom camera via FOV (clamped)
                cam->fovDeg = glm::clamp(cam->fovDeg - float(sy) * scrollZoomSpeed_, 20.f, 90.f);
            } else if (lmbDown && dragging_) {
                // Adjust drag depth along current mouse ray
                // Recompute ray at current cursor pos
                float ndcX =  2.f * float(x) / float(width)  - 1.f;
                float ndcY = -2.f * float(y) / float(height) + 1.f;
                glm::vec2 ndc(ndcX, ndcY);
                glm::mat4 invVP = glm::inverse(cam->proj() * cam->view());
                Ray ray = rayFromNDC(ndc, invVP);

                // Scale step with current depth for nicer feel
                float step = float(sy) * 0.2f * glm::max(0.5f, dragDepth_);
                dragDepth_ = glm::max(0.05f, dragDepth_ + step);

                glm::vec3 pos = ray.o + dragDepth_ * ray.d;
                if (dragTarget) dragTarget->setPosition(glm::dvec3(pos));
            }
        }
    }
}

void ViewportController::handleMouseLook(double x, double y) {
    if (firstMouse) { firstMouse = false; lastX = x; lastY = y; }
    float dx = float(x - lastX);
    float dy = float(lastY - y); // invert Y
    lastX = x; lastY = y;

    cam->addYawPitch(dx * mouseSensitivity_, dy * mouseSensitivity_);
}

void ViewportController::handleMouseDrag(double x, double y) {
    if (!dragTarget || !cam || width<=0 || height<=0) return;

    // Convert window coords -> NDC
    float ndcX =  2.f * float(x) / float(width)  - 1.f;
    float ndcY = -2.f * float(y) / float(height) + 1.f; // flip Y
    glm::vec2 ndc(ndcX, ndcY);

    // Build inverse VP
    glm::mat4 VP = cam->proj() * cam->view();
    glm::mat4 invVP = glm::inverse(VP);

    // Ray through cursor
    Ray ray = rayFromNDC(ndc, invVP);

    // Place object at current dragDepth_ along the ray
    glm::vec3 pos = ray.o + dragDepth_ * ray.d;
    dragTarget->setPosition(glm::dvec3(pos));
}