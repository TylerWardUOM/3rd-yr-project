// Renderable.h
#pragma once
#include "viz/MeshGPU.h"
#include "viz/Camera.h"
#include "viz/MVPUniforms.h"
#include "viz/Shader.h"

struct Renderable {
    std::unique_ptr<MeshGPU> mesh{};
    Shader* shader{};

    void render(const Camera& cam, const glm::dmat4& model) {
        if (!mesh || !shader) return;
        MVPUniforms u;
        u.model = model;
        u.view  = cam.view();
        u.proj  = cam.proj();
        u.upload(*shader);
        shader->use();
        mesh->draw();
    }
};
