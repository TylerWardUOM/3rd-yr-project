#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_inverse.hpp> // for glm::inverse + mat3
#include <glm/gtc/type_ptr.hpp>
#include "viz/Shader.h"

struct MVPUniforms {
    glm::mat4 model{1.0f};
    glm::mat4 view{1.0f};
    glm::mat4 proj{1.0f};

    // World-space normal matrix (upper-left 3x3 of model)
    glm::mat3 normalMatrix() const {
        return glm::transpose(glm::inverse(glm::mat3(model)));
    }

    void upload(Shader& sh) const {
        sh.use();
        sh.setMat4("uModel", model);
        sh.setMat4("uView",  view);
        sh.setMat4("uProj",  proj);
        sh.setMat3("uNormalMatrix", normalMatrix());
    }
};
