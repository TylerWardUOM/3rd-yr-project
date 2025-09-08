#ifndef WINDOW_H
#define WINDOW_H
#include <string>
#include <unordered_map>
struct GLFWwindow; // forward declare

// -- input enums and wrappers --
enum class Key { W, A, S, D, Space, LShift, Escape, Left, Right, Up, Down };
enum class MouseButton { Left, Right, Middle };
enum class CursorMode { Normal, Disabled, Hidden };

//--- simple window wrapper around GLFW ---
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

    explicit Window(const Config& cfg = {}); // constructor
    ~Window();

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    // window open?
    bool isOpen() const; 
    // poll events 
    void poll();
    // swap buffers 
    void swap();

    // get GLFW window handle
    GLFWwindow* handle() const { return win_; } 
    // get framebuffer width
    int width()  const { return width_; }   // framebuffer width (kept in sync)
    // get framebuffer height
    int height() const { return height_; }  // framebuffer height
    // get framebuffer size (updates w and h passed by reference)
    void getFramebufferSize(int& w, int& h) const;
    // pop accumulated scroll (consumes and returns)
    double popScrollY() { double v = scrollY_; scrollY_ = 0.0; return v; }

    // set should close flag
    void setShouldClose(bool v = true);
    // set clear color
    void setClearColor(float r, float g, float b, float a);
    // clear buffers
    void clear(bool color = true, bool depth = false);

    // --- input function wrappers ---    
    void setCursorMode(CursorMode m);
    bool isKeyDown(Key k) const;    // detect if a key is pressed
    bool isMouseButtonDown(MouseButton b) const;    // Detect if a mouse button is pressed
    // updates positions passed by reference
    void getCursorPos(double& x, double& y) const;
    //updates time passed by reference
    void getTime(double& t) const;

private:
    GLFWwindow* win_ = nullptr; // GLFW window handle
    mutable std::unordered_map<Key, bool> keyStates_; // Track key states
    mutable std::unordered_map<MouseButton, bool> mouseButtonStates_; // Track mouse button states
    int width_ = 0; // framebuffer width
    int height_ = 0; // framebuffer height
    double scrollY_ = 0.0;   // <- accumulates from GLFW scroll callback

    // update stored size and OpenGL viewport
    void onFramebufferSize(int width, int height);
    //internal init steps
    void initGLFW(const Config& cfg);
    // create window
    void createWindow(const Config& cfg);
    // initialize GLAD
    void initGLAD();
    // set GLFW callbacks
    void setCallbacks();
};

#endif // WINDOW_H
