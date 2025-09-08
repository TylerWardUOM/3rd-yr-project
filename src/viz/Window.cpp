#include "viz/Window.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdexcept>
#include <iostream>

// -- key input conversion --
static int toGLFW(Key k) {
    switch (k) {
        case Key::W:      return GLFW_KEY_W;
        case Key::A:      return GLFW_KEY_A;
        case Key::S:      return GLFW_KEY_S;
        case Key::D:      return GLFW_KEY_D;
        case Key::Space:  return GLFW_KEY_SPACE;
        case Key::LShift: return GLFW_KEY_LEFT_SHIFT;
        case Key::Escape: return GLFW_KEY_ESCAPE;
        case Key::Left:   return GLFW_KEY_LEFT;
        case Key::Right:  return GLFW_KEY_RIGHT;
        case Key::Up:     return GLFW_KEY_UP;
        case Key::Down:   return GLFW_KEY_DOWN;
        default:          return GLFW_KEY_UNKNOWN;
    }
}

// -- mouse button conversion --
static int toGLFW(MouseButton b) {
    switch (b) {
        case MouseButton::Left:   return GLFW_MOUSE_BUTTON_LEFT;
        case MouseButton::Right:  return GLFW_MOUSE_BUTTON_RIGHT;
        case MouseButton::Middle: return GLFW_MOUSE_BUTTON_MIDDLE;
        default:                  return -1;
    }
}


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

void Window::onFramebufferSize(int width, int height) {
    //update stored size
    width_ = width; 
    height_ = height;
    // update OpenGL viewport
    glViewport(0, 0, width_, height_);
}

// get framebuffer size
void Window::getFramebufferSize(int& w, int& h) const { glfwGetFramebufferSize(win_, &w, &h); }
// get time
void Window::getTime(double& t) const { t = glfwGetTime(); }

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

// -- input wrappers --
void Window::getCursorPos(double& x, double& y) const { glfwGetCursorPos(win_, &x, &y); }
bool Window::isKeyDown(Key k) const { return glfwGetKey(win_, toGLFW(k)) == GLFW_PRESS; }
bool Window::isMouseButtonDown(MouseButton b) const { return glfwGetMouseButton(win_, toGLFW(b)) == GLFW_PRESS; }

void Window::setCursorMode(CursorMode m) {
    int mode = (m==CursorMode::Disabled)?GLFW_CURSOR_DISABLED:
               (m==CursorMode::Hidden)?GLFW_CURSOR_HIDDEN:GLFW_CURSOR_NORMAL;
    glfwSetInputMode(win_, GLFW_CURSOR, mode);
}