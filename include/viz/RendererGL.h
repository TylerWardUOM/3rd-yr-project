#ifndef RENDERERGL_H
#define RENDERERGL_H

#include "IRenderer.h"
#include "Shader.h"

class RendererGL : public IRenderer {
    public:
        RendererGL();
        ~RendererGL() override;

        bool init(GLFWwindow* window) override; // initialize the renderer with the given window


        void beginFrame() override; // start the rendering of a new frame
        void endFrame() override; // end the rendering of the current frame
        void clearScreen(float r, float g, float b, float a) override; // clear the screen (color and depth buffers)

        void draw() override; // perform the actual drawing commands

        Shader& getShader() { return shaderProgram; }

    private:
        unsigned int VAO, VBO;
        Shader shaderProgram;
        Shader createShader(const std::string& vertPath, const std::string& fragPath) override;
        GLFWwindow* window_ = nullptr;
};

#endif // RENDERERGL_H