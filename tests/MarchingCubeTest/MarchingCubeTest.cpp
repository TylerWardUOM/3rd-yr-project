// main.cpp (now tiny)
#include "viz/Window.h"
#include "scene/Scene.h"

int main() {
    Window win({});
    Scene  scene(win);
    scene.run();
    return 0;
}
   