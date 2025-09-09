// main.cpp 
#include <glm/glm.hpp>
#include "viz/Window.h"
#include "world/world.h"
#include "viz/Camera.h"
#include "viz/GlSceneRenderer.h"
#include "scene/Scene.h"
#include "scene/ui/ViewportController.h"

int main() {
    Window win({}); //Window Object

    Camera camera; // Camera Object

	World world; // World Object

	GlSceneRenderer renderer(camera); // Renderer Object

	ViewportController vpCtrl(win, world); // Viewport Controller Object

	Scene scene(win, world, renderer, camera); // Scene Object

	scene.run();
	return 0;
}