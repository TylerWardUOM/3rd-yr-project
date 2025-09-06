#ifndef IRENDERER_H
#define IRENDERER_H

#include <string>

struct GLFWwindow;
struct Shader;

class IRenderer {
    public:
        IRenderer() = default;
        virtual ~IRenderer() = default;

        virtual bool init(GLFWwindow* window) = 0; // initialize the renderer with the given window


        virtual void beginFrame() = 0; // start the rendering of a new frame
        virtual void endFrame() = 0; // end the rendering of the current frame
        virtual void clearScreen(float r, float g, float b, float a) = 0; // clear the screen (color and depth buffers)

        virtual Shader createShader(const std::string& vertPath, const std::string& fragPath) = 0;
        virtual void draw() = 0; // perform the actual drawing commands
};

#endif