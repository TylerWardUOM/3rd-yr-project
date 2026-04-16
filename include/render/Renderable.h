// Renderable.h
#pragma once
#include "render/gpu/MeshGPU.h"
#include "render/Camera.h"
#include "render/shader/MVPUniforms.h"
#include "render/shader/Shader.h"
#include <memory>

struct Renderable {
    const MeshGPU* mesh{};   // non-owning
    Shader* shader{};
    glm::vec3 colour = {0.8f,0.8f,0.8f}; // default white
    float useGridLines = 0.0; // default no grid

    void render(const Camera& cam, const glm::dmat4& model) {
        if (!mesh || !shader) return;
        shader->use();
        MVPUniforms u;
        u.model = model;
        u.view  = cam.view();
        u.proj  = cam.proj();
        u.colour = colour;
        u.useGridLines = useGridLines;
        u.upload(*shader);
        mesh->draw();
    }
};
