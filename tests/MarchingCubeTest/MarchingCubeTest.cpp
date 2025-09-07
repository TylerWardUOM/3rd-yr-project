#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "viz/Shader.h"
#include "viz/Window.h"
#include <iostream>
#include <stdlib.h>
#include "env/primitives//PlaneEnv.h"
#include "env/Mesher.h"

// --- minimal vec/mat helpers (column-major, OpenGL-style) ---
struct Vec3f { float x, y, z; };
static Vec3f vsub(Vec3f a, Vec3f b) { return { a.x - b.x,a.y - b.y,a.z - b.z }; }
static Vec3f vcross(Vec3f a, Vec3f b) {
    return { a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x };
}
static float vdot(Vec3f a, Vec3f b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
static Vec3f vnorm(Vec3f a) { float L = std::sqrt(vdot(a, a)); return (L > 1e-8f) ? Vec3f{ a.x / L,a.y / L,a.z / L } : Vec3f{ 0,0,1 }; }

// Column-major 4x4
static void matIdent(float m[16]) {
    for (int i = 0; i < 16; ++i) m[i] = 0.0f; m[0] = m[5] = m[10] = m[15] = 1.0f;
}
static void matMul(float out[16], const float A[16], const float B[16]) {
    float r[16];
    for (int c = 0; c < 4; ++c)
        for (int rI = 0; rI < 4; ++rI)
            r[c * 4 + rI] = A[0 * 4 + rI] * B[c * 4 + 0] + A[1 * 4 + rI] * B[c * 4 + 1] + A[2 * 4 + rI] * B[c * 4 + 2] + A[3 * 4 + rI] * B[c * 4 + 3];
    for (int i = 0; i < 16; ++i) out[i] = r[i];
}
static void matPerspective(float out[16], float fovyRadians, float aspect, float znear, float zfar) {
    float f = 1.0f / std::tan(fovyRadians * 0.5f);
    for (int i = 0; i < 16; ++i) out[i] = 0.0f;
    out[0] = f / aspect;
    out[5] = f;
    out[10] = (zfar + znear) / (znear - zfar);
    out[11] = -1.0f;
    out[14] = (2.0f * zfar * znear) / (znear - zfar);
}
static void matLookAt(float out[16], Vec3f eye, Vec3f center, Vec3f up) {
    Vec3f f = vnorm(vsub(center, eye));
    Vec3f s = vnorm(vcross(f, up));
    Vec3f u = vcross(s, f);

    // column-major
    out[0] = s.x; out[4] = s.y; out[8] = s.z; out[12] = -vdot(s, eye);
    out[1] = u.x; out[5] = u.y; out[9] = u.z; out[13] = -vdot(u, eye);
    out[2] = -f.x; out[6] = -f.y; out[10] = -f.z; out[14] = vdot(f, eye);
    out[3] = 0.0f; out[7] = 0.0f; out[11] = 0.0f; out[15] = 1.0f;
}


void processInput(GLFWwindow* window);

PlaneEnv plane({1.0,0.0,0.0},0.0);
Mesh m = Mesher::makeMeshMC(plane, { -1.5,-1.5,-1.5 }, { 1.5,1.5,1.5 }, 64, 64, 64, 0.0);
auto vertices = Mesher::packPosNrmFloat(m);

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    Window win({});

    // build and compile our shader program
    // ------------------------------------
    Shader ourShader("shaders/sphere.vert", "shaders/basic.frag"); // you can name your shader files however you like


    // 1) set viewport once (handles HiDPI properly)
    int fbw = 0, fbh = 0;
    glfwGetFramebufferSize(win.handle(), &fbw, &fbh);
    glViewport(0, 0, fbw, fbh);

    // 2) build MVP (camera at (0,0,3) looking at origin)
    float P[16], V[16], M[16], VP[16], MVP[16];
    matPerspective(P, /*fov*/ 60.0f * 3.1415926f / 180.0f, (float)fbw / (float)fbh, 0.1f, 100.0f);
    matLookAt(V, /*eye*/{ -10,0,0 }, /*center*/{ 0,0,0 }, /*up*/{ 0,1,0 });
    matIdent(M);
    matMul(VP, P, V);
    matMul(MVP, VP, M);

    // 3) upload MVP uniform (choose ONE path)

 

    // b) If not, do raw GL (Shader needs public .ID program handle)
    ourShader.use();
    GLint loc = glGetUniformLocation(ourShader.ID, "uMVP");
    glUniformMatrix4fv(loc, 1, GL_FALSE, MVP); // GL_FALSE: already column-major
    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------

    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
    // VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
    // glBindVertexArray(0);


    // render loop
    // -----------
	float offset = 0.0f;

    while (win.isOpen())
    {
        processInput(win.handle());

        glEnable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE); // debug: disable culling first
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ourShader.use();
        // (MVP is constant here; if you animate, recompute and re-upload)

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(vertices.size() / 6));

        win.swap();
        win.poll();
    }


    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}