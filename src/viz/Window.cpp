#include "viz/Window.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdexcept>
#include <iostream>

Window::Window(const Config& cfg) {
    initGLFW(cfg);
    createWindow(cfg);
    initGLAD();
    setCallbacks();
}

Window::~Window() {
    if (win_) { glfwDestroyWindow(win_); win_ = nullptr; }
    glfwTerminate();
}

void Window::initGLFW(const Config& cfg) {
    if (!glfwInit()) throw std::runtime_error("glfwInit failed");
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, cfg.glMajor);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, cfg.glMinor);
    if (cfg.coreProfile) glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
}

void Window::createWindow(const Config& cfg) {
    width_ = cfg.width; height_ = cfg.height;
    win_ = glfwCreateWindow(width_, height_, cfg.title.c_str(), nullptr, nullptr);
    if (!win_) throw std::runtime_error("glfwCreateWindow failed");
    glfwMakeContextCurrent(win_);
    glfwSetWindowUserPointer(win_, this);
}

void Window::initGLAD() {
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        throw std::runtime_error("gladLoadGLLoader failed");
}

void Window::setCallbacks() {
    // framebuffer size
    glfwSetFramebufferSizeCallback(win_, [](GLFWwindow* w, int W, int H){
        if (auto* self = static_cast<Window*>(glfwGetWindowUserPointer(w))) {
            self->onFramebufferSize(W, H);
        }
    });

    // scroll accumulates into Window (NOT into ViewportController!)
    glfwSetScrollCallback(win_, [](GLFWwindow* w, double /*xoff*/, double yoff){
        if (auto* self = static_cast<Window*>(glfwGetWindowUserPointer(w))) {
            self->scrollY_ += yoff;
        }
    });
}

void Window::framebufferSizeCB(GLFWwindow* w, int width, int height) {
    auto* self = static_cast<Window*>(glfwGetWindowUserPointer(w));
    if (self) self->onFramebufferSize(width, height);
}

void Window::onFramebufferSize(int width, int height) {
    width_ = width; height_ = height;
    glViewport(0, 0, width_, height_);
}

bool Window::isOpen() const { return !glfwWindowShouldClose(win_); }
void Window::poll() { glfwPollEvents(); }
void Window::swap() { glfwSwapBuffers(win_); }
void Window::setShouldClose(bool v) { glfwSetWindowShouldClose(win_, v); }

void Window::setClearColor(float r, float g, float b, float a) { glClearColor(r,g,b,a); }
void Window::clear(bool color, bool depth) {
    GLbitfield mask = 0;
    if (color) mask |= GL_COLOR_BUFFER_BIT;
    if (depth) mask |= GL_DEPTH_BUFFER_BIT;
    glClear(mask);
}
