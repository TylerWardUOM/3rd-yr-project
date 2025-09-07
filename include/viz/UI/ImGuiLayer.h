#pragma once
struct GLFWwindow;

class ImGuiLayer {
public:
    // glslVersion like "#version 330" (match your context)
    void init(GLFWwindow* window, const char* glslVersion = "#version 330");
    void begin();     // NewFrame for backends + ImGui::NewFrame()
    void end();       // ImGui::Render() + backend render
    void shutdown();

    // Forward ImGuiIO capture flags for input gating
    bool wantCaptureMouse() const;
    bool wantCaptureKeyboard() const;

private:
    GLFWwindow* window_ = nullptr;
    bool inited_ = false;
};
