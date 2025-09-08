#pragma once
#include <imgui.h>
#include "viz/Window.h"

class ImGuiLayer {
public:
    explicit ImGuiLayer(Window& w, const char* glslVersion = "#version 330");
    ImGuiLayer(const ImGuiLayer&) = delete;            
    ImGuiLayer& operator=(const ImGuiLayer&) = delete; 
    // glslVersion like "#version 330" 
    void begin();     // NewFrame for backends + ImGui::NewFrame()
    void end();       // ImGui::Render() + backend render
    void shutdown();
    void getFps(float& outFps) const { outFps = ImGui::GetIO().Framerate; }

    // Forward ImGuiIO capture flags for input gating
    bool wantCaptureMouse() const;
    bool wantCaptureKeyboard() const;

private:
    Window* win;
    bool inited_ = false;
};
