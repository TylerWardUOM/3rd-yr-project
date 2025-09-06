#include "viz/RendererGL.h"


RendererGL::RendererGL()
    : shaderProgram(createShader("shaders/basic.vert", "shaders/basic.frag"))
{}

Shader RendererGL::createShader(const std::string& vertPath, const std::string& fragPath) {
    return Shader(vertPath.c_str(), fragPath.c_str());
}

void RendererGL::clearScreen(float r = 0.2f, float g = 0.3f, float b = 0.3f, float a = 1.0f) {
    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT);
}

void RendererGL::beginFrame() {
    clearScreen();
}

void RendererGL::endFrame() {
    glfwSwapBuffers(window_);
}