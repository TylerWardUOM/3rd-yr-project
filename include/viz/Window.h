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

        // non-copyable, movable if you want (omitted here)
        Window(const Window&) = delete;
        Window& operator=(const Window&) = delete;

        bool isOpen() const; // check if window is open
        void poll(); // poll events
        void swap(); //swap buffers

        // getters
        GLFWwindow* handle() const { return win_; }
        int width()  const { return width_; }
        int height() const { return height_; }

        // setters
        void setShouldClose(bool v = true);
        void setClearColor(float r, float g, float b, float a);
        void clear(bool color = true, bool depth = false);

    private:
        GLFWwindow* win_ = nullptr;
        int width_ = 0, height_ = 0;

        // static GLFW callbacks → instance methods
        static void framebufferSizeCB(GLFWwindow* w, int width, int height);
        void onFramebufferSize(int width, int height);

        void initGLFW(const Config& cfg);
        void createWindow(const Config& cfg);
        void initGLAD();
        void setCallbacks();
};


#endif // WINDOW_H