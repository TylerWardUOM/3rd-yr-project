// ViewportController.cpp
#include "scene/ui/ViewportController.h"
#include <glm/gtc/matrix_transform.hpp>
//#include "viz/Ray.h"

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
        if (glfwGetKey(win, GLFW_KEY_SPACE) == GLFW_PRESS) cam->eye += cam->camUp * moveSpeed_;
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

        if (lmb == GLFW_PRESS && !lmbDown) { lmbDown = true; }
        else if (lmb == GLFW_RELEASE && lmbDown) { lmbDown = false; }
    } else {
        // If UI is capturing, ensure we’re not in a locked state
        if (rmbDown) { rmbDown = false; glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_NORMAL); }
        lmbDown = false;
        firstMouse = true;
    }

    // Mouse position
    double x, y; glfwGetCursorPos(win, &x, &y);

    if (rmbDown) handleMouseLook(x, y);
    if (lmbDown) handleMouseDrag(x, y);

    // Scroll zoom (only when not rotating & not over UI)
    if (!uiCapturing && !rmbDown) {
        double sx=0.0, sy=0.0;
        // You can add a GLFW scroll callback and cache sy instead; here we poll the IO via ImGui if preferred.
        // For simplicity, omit here. Optionally wire a callback to modify cam->fovDeg.
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
    float ndcX =  2.f * float(x) / float(width) - 1.f;
    float ndcY = -2.f * float(y) / float(height) + 1.f; // flip Y
    glm::vec2 ndc(ndcX, ndcY);

    // Build inverse VP
    glm::mat4 VP = cam->proj() * cam->view();
    glm::mat4 invVP = glm::inverse(VP);

    // // Ray
    // Ray ray = rayFromNDC(ndc, invVP);

    // // Intersect with y=0 plane (ground)
    // float t; glm::vec3 hit;
    // if (intersectRayPlaneY0(ray, t, hit)) {
    //     // Keep sphere at small offset above ground if desired
    //     glm::dvec3 newPos = glm::dvec3(hit.x, hit.y + 0.1, hit.z);
    //     dragTarget->setPosition(newPos);
    // }
}
