#ifndef WINDOW_H
#define WINDOW_H
#include <string>
struct GLFWwindow;

class Window {
public:
    struct Config{
        int width = 800;
        int height = 600;
        std::string title = "Haptic Render Test";
        int glMajor = 3;
        int glMinor = 3;
        bool coreProfile = true;
    };

    explicit Window(const Config& cfg = {});
    ~Window();

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    bool isOpen() const;
    void poll();
    void swap();

    // getters
    GLFWwindow* handle() const { return win_; }
    int width()  const { return width_; }   // framebuffer width (kept in sync)
    int height() const { return height_; }  // framebuffer height

    // pop accumulated scroll (consumes and returns)
    double popScrollY() { double v = scrollY_; scrollY_ = 0.0; return v; }

    // setters
    void setShouldClose(bool v = true);
    void setClearColor(float r, float g, float b, float a);
    void clear(bool color = true, bool depth = false);

private:
    GLFWwindow* win_ = nullptr;
    int width_ = 0, height_ = 0;
    double scrollY_ = 0.0;   // <- accumulates from GLFW scroll callback

    // static GLFW callbacks → instance methods
    static void framebufferSizeCB(GLFWwindow* w, int width, int height);

    void onFramebufferSize(int width, int height);
    void initGLFW(const Config& cfg);
    void createWindow(const Config& cfg);
    void initGLAD();
    void setCallbacks();
};

#endif // WINDOW_H
