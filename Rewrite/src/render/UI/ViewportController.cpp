// ViewportController.cpp
#include "render/UI/ViewportController.h"
#include "util/CameraUtils.h"
#include "data/core/Math.h"
#include "data/Commands.h"
#include <iostream>

/*
ARCHITECTURE NOTE (IMPORTANT):

This file currently contains TEMPORARY functionality that directly manipulates
the haptic device and tool pose (object dragging).

This is intentionally kept during the transition to the new modular architecture.

FINAL DESIGN INTENT:
- ViewportController should ONLY manipulate the cam_era.
- Object dragging / tool motion should be implemented in DebugUI.
- User input should emit EditObjectCommand / ToolPoseCommand.
- WorldManager + HapticEngine should consume commands asynchronously.

All direct HapticEngine access in this file MUST be removed before final submission.
*/



ViewportController::ViewportController(Window& window, Camera& camera)
    : win_(&window),
      cam_(&camera)
{
}

void ViewportController::update(float /*dt*/, bool uiCapturing) {

    // ---------------------------------------------------------------------
    // TEMPORARY: Null checks kept for legacy compatibility
    // FINAL DESIGN: ViewportController will always own valid win_dow & cam_era
    // ---------------------------------------------------------------------

    // Get framebuffer size
    win_->getFramebufferSize(width, height);

    // ---------------------------------------------------------------------
    // Keyboard navigation (cam_era movement only)
    // FINAL DESIGN: This logic remains here
    // ---------------------------------------------------------------------
    if (!uiCapturing) {
        if (win_->isKeyDown(Key::W)) cam_->eye += cam_->front * moveSpeed_;
        if (win_->isKeyDown(Key::S)) cam_->eye -= cam_->front * moveSpeed_;
        if (win_->isKeyDown(Key::A)) cam_->eye -= cam_->right * moveSpeed_;
        if (win_->isKeyDown(Key::D)) cam_->eye += cam_->right * moveSpeed_;
        if (win_->isKeyDown(Key::LShift)) cam_->eye -= cam_->up * moveSpeed_;
        if (win_->isKeyDown(Key::Space))  cam_->eye += cam_->up * moveSpeed_;
    }

    // ---------------------------------------------------------------------
    // Mouse position
    // ---------------------------------------------------------------------
    double x, y;
    win_->getCursorPos(x, y);

    // ---------------------------------------------------------------------
    // Mouse button state handling
    // ---------------------------------------------------------------------
    if (!uiCapturing) {

        // ------------------------------
        // RMB: mouse-look
        // FINAL DESIGN: remains here
        // ------------------------------
        if (win_->isMouseButtonDown(MouseButton::Right) && !rmbDown_) {
            rmbDown_ = true;
            firstMouse_ = true;
            win_->setCursorMode(CursorMode::Disabled);
        } 
        else if (!win_->isMouseButtonDown(MouseButton::Right) && rmbDown_) {
            rmbDown_ = false;
            win_->setCursorMode(CursorMode::Normal);
        }

        // -----------------------------------------------------------------
        // LMB: object dragging (TEMPORARY)
        //
        // FINAL DESIGN:
        // - This block must be moved to DebugUI.
        // - Ray casting + drag logic should emit EditObjectCommand.
        // - ViewportController should NOT read HapticSnapshot.
        // -----------------------------------------------------------------
        if (win_->isMouseButtonDown(MouseButton::Left) && !lmbDown_) {
            lmbDown_   = true;
            dragging_ = true;

            // Ray through cursor
            Ray ray = makeRayAtCursor(x, y, width, height, *cam_);

            // TEMPORARY: using haptic snapshot to define drag target
            // FINAL DESIGN: drag target should come from selected ObjectID
            // HapticSnapshot snap = haptic_.readSnapshot();
            // glm::dvec3 spherePos = snap.devicePose_ws.p;
            glm::dvec3 spherePos = glm::dvec3(0.0, 0.0, 0.0); // TEMPORARY
            // Convert ray to double precision
            glm::dvec3 rayOrigin = glm::dvec3(ray.o);
            glm::dvec3 rayDir    = glm::normalize(glm::dvec3(ray.d));

            // Measure depth along ray to sphere position
            glm::dvec3 cam_ToSphere = spherePos - rayOrigin;
            double depth = glm::dot(cam_ToSphere, rayDir);

            dragDepth_ = (depth > 0.05) ? static_cast<float>(depth) : 0.05f;

        } 
        else if (!win_->isMouseButtonDown(MouseButton::Left) && lmbDown_) {
            lmbDown_   = false;
            dragging_ = false;
        }

    } else {
        // UI is capturing mouse — release cam_era controls
        if (rmbDown_) {
            rmbDown_ = false;
            win_->setCursorMode(CursorMode::Normal);
        }
        lmbDown_    = false;
        dragging_  = false;
        firstMouse_ = true;
    }

    // ---------------------------------------------------------------------
    // Mouse-driven cam_era look
    // FINAL DESIGN: remains here
    // ---------------------------------------------------------------------
    if (rmbDown_) handleMouseLook(x, y);

    // ---------------------------------------------------------------------
    // Mouse-driven object dragging (TEMPORARY)
    //
    // FINAL DESIGN:
    // - Must emit command instead of direct haptic submission
    // ---------------------------------------------------------------------
    if (lmbDown_) handleMouseDrag(x, y);

    // ---------------------------------------------------------------------
    // Scroll handling
    // ---------------------------------------------------------------------
    if (pendingScrollY_ != 0.0) {
        double sy = pendingScrollY_;
        pendingScrollY_ = 0.0;

        if (!uiCapturing) {

            // Zoom cam_era via FOV
            if (rmbDown_) {
                cam_->fovDeg = glm::clamp(
                    cam_->fovDeg - float(sy) * scrollZoomSpeed_,
                    20.f, 90.f
                );

            // -----------------------------------------------------------------
            // TEMPORARY: scroll-adjusted object dragging
            //
            // FINAL DESIGN:
            // - This logic must dispatch a command to WorldManager
            // - ViewportController must not submit tool poses
            // -----------------------------------------------------------------
            } else if (lmbDown_ && dragging_) {
                Ray ray = makeRayAtCursor(x, y, width, height, *cam_);

                float step = float(sy) * 0.2f * glm::max(0.5f, dragDepth_);
                dragDepth_ = glm::max(0.05f, dragDepth_ + step);

                glm::vec3 pos = ray.o + dragDepth_ * ray.d;

                // TEMPORARY direct haptic submission
                // HapticSnapshot snap = haptic_.readSnapshot();
                // Pose spherePose = snap.devicePose_ws;
                // haptic_.submitToolPose({glm::dvec3(pos), spherePose.q}, snap.t_sec);
            }
        }
    }
}

void ViewportController::handleMouseLook(double x, double y) {
    if (firstMouse_) {
        firstMouse_ = false;
        lastX_ = x;
        lastY_ = y;
        return;
    }

    float dx = float(x - lastX_);
    float dy = float(lastY_ - y);

    lastX_ = x;
    lastY_ = y;

    if (invertY_) dy = -dy;

    cam_->addYawPitch(
        dx * mouseSensitivity_,
        dy * mouseSensitivity_
    );
}

void ViewportController::handleMouseDrag(double x, double y) {

    /*
    TEMPORARY FUNCTION

    FINAL DESIGN:
    - This function should not exist in ViewportController.
    - Drag logic belongs in DebugUI or a dedicated interaction system.
    - Output should be a World/Edit command, not a device pose update.
    */

    if (width <= 0 || height <= 0) return;

    Ray ray = makeRayAtCursor(x, y, width, height, *cam_);
    glm::vec3 pos = ray.o + dragDepth_ * ray.d;

    // TEMPORARY direct device manipulation
    // HapticSnapshot snap = haptic_.readSnapshot();
    // Pose spherePose = snap.devicePose_ws;
    // SetToolPoseCommand cmd;
    // cmd.toolId = toolObjectId;
    // cmd.pose_ws = pose;
    // cmd.t_sec = now;
    // commandQueue.push(cmd);
}
