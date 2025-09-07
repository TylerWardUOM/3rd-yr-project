// Renderable.h
#pragma once
#include "viz/MeshGPU.h"
#include "viz/Transform.h"
#include "viz/Camera.h"
#include "viz/MVPUniforms.h"
#include "viz/Shader.h"

struct Renderable {
    MeshGPU* mesh{};
    Transform transform{};
    Shader* shader{};

    void render(const Camera& cam) {
        if (!mesh || !shader) return;
        MVPUniforms u;
        u.model = transform.model;
        u.view  = cam.view();
        u.proj  = cam.proj();
        u.upload(*shader);
        shader->use();
        mesh->draw();
    }
};
