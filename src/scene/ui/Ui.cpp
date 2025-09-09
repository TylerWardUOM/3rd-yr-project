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
    }

    if (cfg.showBody) {
        drawBodyPanel(bodyState);
    }

    if (cfg.showCamera) {
        drawCameraPanel(camState);
    }
    if (cfg.showController) {
        drawControllerPanel(ctrlState);
    }
}

void UI::drawBodyPanel(const UITransformState& bodyState) {
    ImGui::Begin("Body");
    // --- Build preview text
    char preview[32] = "None";
    if (bodyState.selectedEntityId) {
        std::snprintf(preview, sizeof(preview), "Entity %u", *bodyState.selectedEntityId);
    }

    // --- Combo
    if (ImGui::BeginCombo("Selected entity", preview)) {
        for (int i = 0; i < (int)bodyState.entityOptions.size(); ++i) {
            uint32_t id = bodyState.entityOptions[i];

            // label for this item
            char label[32];
            std::snprintf(label, sizeof(label), "Entity %u", id);

            bool is_selected = (bodyState.selectedEntityId && id == *bodyState.selectedEntityId);

            ImGui::PushID((int)id); // disambiguate identical labels across frames
            if (ImGui::Selectable(label, is_selected)) {
                if (cmds_.setSelectedEntity) cmds_.setSelectedEntity(id); // notify app
            }
            if (is_selected) ImGui::SetItemDefaultFocus();
            ImGui::PopID();
        }
        ImGui::EndCombo();
    }
    float p[3] = { bodyState.position.x, bodyState.position.y, bodyState.position.z };
    if (ImGui::DragFloat3("Position", p, 0.01f) && cmds_.setBodyPosition) {
        cmds_.setBodyPosition(p[0], p[1], p[2]);
    }

    float c[3] = { bodyState.colour.r, bodyState.colour.g, bodyState.colour.b };
    if (ImGui::SliderFloat3("Colour", c, 0.0, 1.0) && cmds_.setBodyColour) {
        cmds_.setBodyColour(c[0], c[1], c[2]);
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
    float p[3] = { camState.position.x, camState.position.y, camState.position.z };
    if (ImGui::DragFloat3("Position", p, 0.01f) && cmds_.setCameraPosition) {
        cmds_.setCameraPosition(p[0], p[1], p[2]);
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