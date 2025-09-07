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
        if (ImGui::DragFloat3("Position", p, 0.01f) && cmds.setBodyPosition) {
            cmds.setBodyPosition(p[0], p[1], p[2]);
        }


    }

    if (cfg.showCamera) {
        ImGui::SeparatorText("Camera");

        float fov = camState.fovDeg;
        if (ImGui::SliderFloat("FOV (deg)", &fov, 10.f, 120.f) && cmds.setCameraFov) {
            cmds.setCameraFov(fov);
        }

        float nearP = camState.znear;
        float farP  = camState.zfar;

        bool changed = false;
        changed |= ImGui::DragFloat("Near", &nearP, 0.01f, 0.001f, farP - 0.01f);
        changed |= ImGui::DragFloat("Far",  &farP,  0.1f, nearP + 0.01f, 10000.f);
        if (changed) {
            if (cmds.setCameraNear) cmds.setCameraNear(nearP);
            if (cmds.setCameraFar)  cmds.setCameraFar(farP);
        }

        float yaw = camState.yawDeg, pitch = camState.pitchDeg;
        if (ImGui::DragFloat("Yaw (deg)", &yaw, 0.2f) |
            ImGui::DragFloat("Pitch (deg)", &pitch, 0.2f)) {
            if (cmds.setCameraAngles) cmds.setCameraAngles(yaw, pitch);
        }
    }

    ImGui::End();
}

void UI::drawBodyPanel(const UITransformState& bodyState, const UICommands& cmds) {
    ImGui::Begin("Body");

    float p[3] = { bodyState.position.x, bodyState.position.y, bodyState.position.z };
    if (ImGui::DragFloat3("Position", p, 0.01f) && cmds.setBodyPosition) {
        cmds.setBodyPosition(p[0], p[1], p[2]);
    }

    ImGui::End();
}


void UI::drawCameraPanel(const UICameraState& camState, const UICommands& cmds) {
    ImGui::Begin("Camera");
    float fov = camState.fovDeg;
    if (ImGui::SliderFloat("FOV (deg)", &fov, 10.f, 120.f) && cmds.setCameraFov) cmds.setCameraFov(fov);
    float yaw = camState.yawDeg, pitch = camState.pitchDeg;
    if (ImGui::DragFloat("Yaw (deg)", &yaw, 0.2f) | ImGui::DragFloat("Pitch (deg)", &pitch, 0.2f)) {
        if (cmds.setCameraAngles) cmds.setCameraAngles(yaw, pitch);
    }
    float nearP = camState.znear, farP = camState.zfar;
    bool changed = false;
    changed |= ImGui::DragFloat("Near", &nearP, 0.01f, 0.001f, farP - 0.01f);
    changed |= ImGui::DragFloat("Far",  &farP,  0.1f, nearP + 0.01f, 10000.f);
    if (changed) {
        if (cmds.setCameraNear) cmds.setCameraNear(nearP);
        if (cmds.setCameraFar)  cmds.setCameraFar(farP);
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

void UI::drawControllerPanel(const UIControllerState& s, const UICommands& cmds)
{
    if (ImGui::Begin("Controller")) {
        float move = s.moveSpeed;
        if (ImGui::SliderFloat("Move speed", &move, 0.001f, 1.0f, "%.3f")) {
            if (cmds.setMoveSpeed) cmds.setMoveSpeed(move);
        }

        float sens = s.mouseSensitivity;
        if (ImGui::SliderFloat("Mouse sensitivity", &sens, 0.01f, 1.0f, "%.3f")) {
            if (cmds.setMouseSensitivity) cmds.setMouseSensitivity(sens);
        }

        float zoom = s.scrollZoomSpeed;
        if (ImGui::SliderFloat("Scroll zoom speed", &zoom, 0.1f, 5.0f, "%.2f")) {
            if (cmds.setScrollZoomSpeed) cmds.setScrollZoomSpeed(zoom);
        }

        bool invY = s.invertY;
        if (ImGui::Checkbox("Invert Y look", &invY)) {
            if (cmds.setInvertY) cmds.setInvertY(invY);
        }

        bool rmb = s.rmbToLook;
        if (ImGui::Checkbox("RMB to look", &rmb)) {
            if (cmds.setRmbToLook) cmds.setRmbToLook(rmb);
        }
    }
    ImGui::End();
}