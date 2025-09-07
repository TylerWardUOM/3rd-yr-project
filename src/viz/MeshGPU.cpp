#include "viz/MeshGPU.h"

MeshGPU::MeshGPU() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
}

MeshGPU::~MeshGPU() {
    glDeleteBuffers(1, &EBO);
    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO);
}

MeshGPU::MeshGPU(MeshGPU&& o) noexcept {
    *this = std::move(o);
}

MeshGPU& MeshGPU::operator=(MeshGPU&& o) noexcept {
    if (this != &o) {
        VAO = o.VAO;
        VBO = o.VBO;
        EBO = o.EBO;
        count = o.count;
        o = {};  // reset source
    }
    return *this;
}

void MeshGPU::upload(const std::vector<float>& interleavedPosNorm,
                     const std::vector<unsigned>& indices) {
    count = static_cast<GLsizei>(indices.size());
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER,
                 interleavedPosNorm.size() * sizeof(float),
                 interleavedPosNorm.data(),
                 GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 indices.size() * sizeof(unsigned),
                 indices.data(),
                 GL_STATIC_DRAW);

    // aPos (location = 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                          6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // aNormal (location = 1)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
                          6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

void MeshGPU::draw() const {
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}
