// Renderable.h
#pragma once
#include "MeshGPU.h"
#include "Transform.h"
#include "Camera.h"
#include "MVPUniforms.h"
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
