#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "viz/Shader.h"
#include "viz/Window.h"
#include <iostream>
#include <stdlib.h>
#include "env/primitives/SphereEnv.h"
#include "env/Mesher.h"
#include "viz/Renderable.h"


void processInput(GLFWwindow* window, Camera* cam, float dx, float dy);


int main()
{
    // glfw: initialize and configure
    // ------------------------------
    Window win({});

    Camera camera;
    Shader shader("shaders/general.vert", "shaders/basic.frag");
    SphereEnv sphere({ 0.0,0.0,0.0 }, 1.0);
    Mesh m = Mesher::makeMeshMC(sphere, { -1.5,-1.5,-1.5 }, { 1.5,1.5,1.5 }, 64, 64, 64, 0.0);
    std::cout << "Sphere mesh: " << m.vertices.size()
        << " verts, " << m.indices.size() / 3 << " tris\n";
    std::vector<float> interleavedPN;
    std::vector<unsigned> indices;

    Mesher::packPosNrmIdx(m, interleavedPN, indices);
    MeshGPU sphereMesh;
    sphereMesh.upload(interleavedPN, indices);

    int fbw = 0, fbh = 0;
    glfwGetFramebufferSize(win.handle(), &fbw, &fbh);
    glViewport(0, 0, fbw, fbh);

    Renderable sphereRend{&sphereMesh, Transform{}, &shader};

    static bool firstMouse = true;
    static double lastX = 0.0, lastY = 0.0;


    while (win.isOpen())
    {
        double x, y;
        glfwGetCursorPos(win.handle(), &x, &y);

        if (firstMouse) { firstMouse = false; lastX = x; lastY = y; }

        double dx = x - lastX;
        double dy = lastY - y; // invert Y

        lastX = x; lastY = y;
        processInput(win.handle(),&camera, dx, dy);

        glEnable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE); // debug: disable culling first
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        sphereRend.render(camera);

        win.swap();
        win.poll();
    }

    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window, Camera* cam,float dx,float dy)
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

    cam->addYaw(dx);
    cam->addPitch(dy);

    cam->clampPitch();
    cam->updateVectors();
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}