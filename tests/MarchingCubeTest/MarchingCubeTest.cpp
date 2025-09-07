#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "viz/Shader.h"
#include "viz/Window.h"
#include <iostream>
#include <stdlib.h>
#include "env/primitives/SphereEnv.h"
#include "env/primitives/PlaneEnv.h"
#include "scene/Body.h"


void processInput(GLFWwindow* window, Camera* cam, float dx, float dy, Body* bod);


int main()
{
    // glfw: initialize and configure
    // ------------------------------
    Window win({});

    Camera camera;
    Shader shader("shaders/general.vert", "shaders/basic.frag");
	Mesher mesher;
    //auto prim = std::make_unique<SphereEnv>(glm::dvec3{ 0.0,0.0,0.0 }, 1.0);
	auto prim = std::make_unique<PlaneEnv>(glm::dvec3{ 0.0,1.0,0.0 }, 0.0);
    auto mesh = std::make_unique<MeshGPU>();
    Body sphereBody(std::move(prim), std::move(mesh), &mesher, &shader);
    //std::cout << "Sphere mesh: " << m.vertices.size()
    //    << " verts, " << m.indices.size() / 3 << " tris\n";
    //std::vector<float> interleavedPN;
    //std::vector<unsigned> indices;


    int fbw = 0, fbh = 0;
    glfwGetFramebufferSize(win.handle(), &fbw, &fbh);
    glViewport(0, 0, fbw, fbh);


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
        processInput(win.handle(),&camera, dx, dy,&sphereBody);

        glEnable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE); // debug: disable culling first
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        sphereBody.render(camera);

        win.swap();
        win.poll();
    }

    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window, Camera* cam,float dx,float dy,Body* bod)
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
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
		bod->rotate(0.0005f, { 0.0f,1.0f,0.0f });

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