#include "scene/ui/Ui.h"
#include <imgui.h>

void UI::draw(const UIPanelConfig& cfg,
                   const UITransformState& bodyState,
                   const UICameraState& camState,
                   const UIControllerState& ctrlState,
                   const UISceneStats& stats,
                   const UICommands& cmds)
{
    ImGui::Begin(cfg.title.c_str());

    if (cfg.showDebug) {
        ImGui::SeparatorText("Debug");
        ImGui::Text("FPS: %.1f", stats.fps);
        ImGui::Text("Draw Calls: %d", stats.drawCalls);
        ImGui::Text("Triangles: %d", stats.triangles);
    }

    if (cfg.showBody) {
        ImGui::SeparatorText("Body Transform");

        float p[3] = { bodyState.position.x, bodyState.position.y, bodyState.position.z };
        if (ImGui::DragFloat3("Position", p, 0.01f) && cmds_.setBodyPosition) {
            cmds_.setBodyPosition(p[0], p[1], p[2]);
        }


    }

    if (cfg.showCamera) {
        ImGui::SeparatorText("Camera");

        float fov = camState.fovDeg;
        if (ImGui::SliderFloat("FOV (deg)", &fov, 10.f, 120.f) && cmds_.setCameraFov) {
            cmds_.setCameraFov(fov);
        }

        float nearP = camState.znear;
        float farP  = camState.zfar;

        bool changed = false;
        changed |= ImGui::DragFloat("Near", &nearP, 0.01f, 0.001f, farP - 0.01f);
        changed |= ImGui::DragFloat("Far",  &farP,  0.1f, nearP + 0.01f, 10000.f);
        if (changed) {
            if (cmds_.setCameraNear) cmds_.setCameraNear(nearP);
            if (cmds_.setCameraFar)  cmds_.setCameraFar(farP);
        }

        float yaw = camState.yawDeg, pitch = camState.pitchDeg;
        if (ImGui::DragFloat("Yaw (deg)", &yaw, 0.2f) |
            ImGui::DragFloat("Pitch (deg)", &pitch, 0.2f)) {
            if (cmds_.setCameraAngles) cmds_.setCameraAngles(yaw, pitch);
        }
    }

    ImGui::End();
}

void UI::drawBodyPanel(const UITransformState& bodyState) {
    ImGui::Begin("Body");

    float p[3] = { bodyState.position.x, bodyState.position.y, bodyState.position.z };
    if (ImGui::DragFloat3("Position", p, 0.01f) && cmds_.setBodyPosition) {
        cmds_.setBodyPosition(p[0], p[1], p[2]);
    }

    ImGui::End();
}


void UI::drawCameraPanel(const UICameraState& camState) {
    ImGui::Begin("Camera");
    float fov = camState.fovDeg;
    if (ImGui::SliderFloat("FOV (deg)", &fov, 10.f, 120.f) && cmds_.setCameraFov) cmds_.setCameraFov(fov);
    float yaw = camState.yawDeg, pitch = camState.pitchDeg;
    if (ImGui::DragFloat("Yaw (deg)", &yaw, 0.2f) | ImGui::DragFloat("Pitch (deg)", &pitch, 0.2f)) {
        if (cmds_.setCameraAngles) cmds_.setCameraAngles(yaw, pitch);
    }
    float nearP = camState.znear, farP = camState.zfar;
    bool changed = false;
    changed |= ImGui::DragFloat("Near", &nearP, 0.01f, 0.001f, farP - 0.01f);
    changed |= ImGui::DragFloat("Far",  &farP,  0.1f, nearP + 0.01f, 10000.f);
    if (changed) {
        if (cmds_.setCameraNear) cmds_.setCameraNear(nearP);
        if (cmds_.setCameraFar)  cmds_.setCameraFar(farP);
    }
    ImGui::End();
}

void UI::drawDebugPanel(const UISceneStats& stats) {
    ImGui::Begin("Debug");
    ImGui::Text("FPS: %.1f", stats.fps);
    ImGui::Text("Draw Calls: %d", stats.drawCalls);
    ImGui::Text("Triangles: %d", stats.triangles);
    ImGui::End();
}

void UI::drawControllerPanel(const UIControllerState& s)
{
    if (ImGui::Begin("Controller")) {
        float move = s.moveSpeed;
        if (ImGui::SliderFloat("Move speed", &move, 0.001f, 1.0f, "%.3f")) {
            if (cmds_.setMoveSpeed) cmds_.setMoveSpeed(move);
        }

        float sens = s.mouseSensitivity;
        if (ImGui::SliderFloat("Mouse sensitivity", &sens, 0.01f, 1.0f, "%.3f")) {
            if (cmds_.setMouseSensitivity) cmds_.setMouseSensitivity(sens);
        }

        float zoom = s.scrollZoomSpeed;
        if (ImGui::SliderFloat("Scroll zoom speed", &zoom, 0.1f, 5.0f, "%.2f")) {
            if (cmds_.setScrollZoomSpeed) cmds_.setScrollZoomSpeed(zoom);
        }

        bool invY = s.invertY;
        if (ImGui::Checkbox("Invert Y look", &invY)) {
            if (cmds_.setInvertY) cmds_.setInvertY(invY);
        }

        bool rmb = s.rmbToLook;
        if (ImGui::Checkbox("RMB to look", &rmb)) {
            if (cmds_.setRmbToLook) cmds_.setRmbToLook(rmb);
        }
    }
    ImGui::End();
}