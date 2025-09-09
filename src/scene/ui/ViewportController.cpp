// ViewportController.cpp
#include "scene/ui/ViewportController.h"
#include "util/CameraUtils.h"
#include <iostream>

void ViewportController::update(float /*dt*/, bool uiCapturing) {
    if (!cam || !win) return;

    // Update aspect each frame (in case of resize)
    // int fbw=0, fbh=0; 
    // win->getFramebufferSize(fbw, fbh);
    // if (fbw>0 && fbh>0) {
    //     cam->aspect = float(fbw)/float(fbh); 
    //     setViewport(fbw, fbh); 
    // }
    win->getFramebufferSize(width, height);
    //  -- Keyboard (WASD) — always on if not captured
    if (!uiCapturing) {
        if (win->isKeyDown(Key::W)) cam->eye += cam->front * moveSpeed_;
        if (win->isKeyDown(Key::S)) cam->eye -= cam->front * moveSpeed_;
        if (win->isKeyDown(Key::A)) cam->eye -= cam->right * moveSpeed_;
        if (win->isKeyDown(Key::D)) cam->eye += cam->right * moveSpeed_;
        if (win->isKeyDown(Key::LShift)) cam->eye -= cam->up * moveSpeed_;
        if (win->isKeyDown(Key::Space)) cam->eye += cam->up * moveSpeed_;
    }

    // -- Mouse handling --

    // Mouse position
    double x, y;
    win->getCursorPos(x,y); // current mouse pos

    // --- RMB/LMB handling ---
    if (!uiCapturing) {
        // --- RMB handling
        if (win->isMouseButtonDown(MouseButton::Right) && !rmbDown) {
            // Start look
            rmbDown = true; firstMouse = true;
            win->setCursorMode(CursorMode::Disabled); // lock cursor while looking
        } else if (!win->isMouseButtonDown(MouseButton::Right) && rmbDown) {
            // End look
            rmbDown = false;
            win->setCursorMode(CursorMode::Normal); // unlock cursor
        }

        // --- LMB handling
        if (win->isMouseButtonDown(MouseButton::Left) && !lmbDown) {
            lmbDown   = true;
            dragging_ = true;

            // Ray through cursor
            Ray ray = makeRayAtCursor(x, y, width, height, *cam); 
            //std::cout << width;
            // ----- Get drag target position
            glm::dvec3 spherePos;
            if (dragTarget) {
                WorldSnapshot snap = world_.readSnapshot();
                spherePos = snap.surfaces[dragTarget].T_ws.p;
                
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

        } else if (!win->isMouseButtonDown(MouseButton::Left) && lmbDown) {
            // End drag
            lmbDown   = false;
            dragging_ = false;
        }

    } else {
        // If UI is capturing, ensure we’re not in a locked state
        if (rmbDown) { 
            rmbDown = false; 
            win->setCursorMode(CursorMode::Normal); }
        lmbDown   = false;
        dragging_ = false;
        firstMouse = true;
    }

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
                Ray ray = makeRayAtCursor(x, y, width, height, *cam);

                // Scale step with current depth for nicer feel
                float step = float(sy) * 0.2f * glm::max(0.5f, dragDepth_); // proportional step
                dragDepth_ = glm::max(0.05f, dragDepth_ + step); // keep positive

                glm::vec3 pos = ray.o + dragDepth_ * ray.d; // new position along ray
                WorldSnapshot snap = world_.readSnapshot();
                Pose spherePose = snap.surfaces[dragTarget].T_ws;
                if (dragTarget) world_.setPose(dragTarget, {(glm::dvec3(pos)),spherePose.q});
            }
        }
    }
    world_.publishSnapshot(0.0); // publish any changes
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
    Ray ray = makeRayAtCursor(x, y, width, height, *cam);

    // Place object at current dragDepth_ along the ray
    glm::vec3 pos = ray.o + dragDepth_ * ray.d;
    WorldSnapshot snap = world_.readSnapshot();
    Pose spherePose = snap.surfaces[dragTarget].T_ws;
    if (dragTarget) world_.setPose(dragTarget, {(glm::dvec3(pos)),spherePose.q});
}