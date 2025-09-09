// Renderable.h
#pragma once
#include "viz/MeshGPU.h"
#include "viz/Camera.h"
#include "viz/MVPUniforms.h"
#include "viz/Shader.h"
#include <memory>

struct Renderable {
    const MeshGPU* mesh{};   // non-owning
    Shader* shader{};

    void render(const Camera& cam, const glm::dmat4& model) {
        if (!mesh || !shader) return;
        shader->use();
        MVPUniforms u;
        u.model = model;
        u.view  = cam.view();
        u.proj  = cam.proj();
        u.upload(*shader);
        mesh->draw();
    }
};
