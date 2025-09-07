#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "viz/Shader.h"
#include "viz/Window.h"
#include <iostream>
#include <stdlib.h>
#include "env/primitives//PlaneEnv.h"
#include "env/Mesher.h"
#include "viz/Renderable.h"


void processInput(GLFWwindow* window, Camera* cam);


int main()
{
    // glfw: initialize and configure
    // ------------------------------
    Window win({});

    Camera camera;
    Shader shader("shaders/general.vert", "shaders/basic.frag");
    PlaneEnv plane({ 1.0,0.0,0.0 }, 0.0);
    Mesh m = Mesher::makeMeshMC(plane, { -1.5,-1.5,-1.5 }, { 1.5,1.5,1.5 }, 64, 64, 64, 0.0);
    std::vector<float> interleavedPN;
    std::vector<unsigned> indices;

    Mesher::packPosNrmIdx(m, interleavedPN, indices);
    MeshGPU gpu;
    gpu.upload(interleavedPN, indices);

    int fbw = 0, fbh = 0;
    glfwGetFramebufferSize(win.handle(), &fbw, &fbh);
    glViewport(0, 0, fbw, fbh);

    Renderable planeRend{&gpu, Transform{}, &shader};

    while (win.isOpen())
    {
        processInput(win.handle(),&camera);

        glEnable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE); // debug: disable culling first
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        planeRend.render(camera);

        win.swap();
        win.poll();
    }

    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window, Camera* cam)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
    float speed = 0.0001f;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cam->eye += cam->front * speed;     // forward
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cam->eye -= cam->front * speed;     // backward
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cam->eye -= cam->right * speed;     // strafe left
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cam->eye += cam->right * speed;     // strafe right

}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}