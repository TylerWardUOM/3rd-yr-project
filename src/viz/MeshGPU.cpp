#include "viz/MeshGPU.h"



MeshGPU::~MeshGPU() { destroy(); }

void MeshGPU::destroy() noexcept {
    if (EBO) glDeleteBuffers(1, &EBO);
    if (VBO) glDeleteBuffers(1, &VBO);
    if (VAO) glDeleteVertexArrays(1, &VAO);
    VAO = VBO = EBO = 0;
    count = 0;
}

void MeshGPU::steal(MeshGPU& o) noexcept {
    VAO   = o.VAO;
    VBO   = o.VBO;
    EBO   = o.EBO;
    count = o.count;
    o.VAO = o.VBO = o.EBO = 0;
    o.count = 0;
}

MeshGPU::MeshGPU(MeshGPU&& o) noexcept { steal(o); }

MeshGPU& MeshGPU::operator=(MeshGPU&& o) noexcept {
    if (this != &o) {
        destroy();   // release current resources
        steal(o);    // take ownership; zero out 'o' fields (no recursion)
    }
    return *this;
}

// Upload interleaved [pos.xyz | nrm.xyz], triangles via indices
void MeshGPU::upload(const std::vector<float>& interleavedPosNorm,
                     const std::vector<unsigned>& indices) {
    count = static_cast<GLsizei>(indices.size()); // number of indices to draw

    glGenVertexArrays(1, &VAO); // create VAO first
    glGenBuffers(1, &VBO);  // create VBO second
    glGenBuffers(1, &EBO); // create EBO last
    // --- Set up buffers and upload data
    glBindVertexArray(VAO); // bind VAO first

    glBindBuffer(GL_ARRAY_BUFFER, VBO); // bind VBO second
    glBufferData(GL_ARRAY_BUFFER, 
                 interleavedPosNorm.size() * sizeof(float),
                 interleavedPosNorm.data(),
                 GL_STATIC_DRAW); // upload data

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO); // bind EBO last
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 
                 indices.size() * sizeof(unsigned),
                 indices.data(),
                 GL_STATIC_DRAW); // upload data

    // --- Set up vertex attributes
    // aPos (location = 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                          6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // aNormal (location = 1)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
                          6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // unbind VAO (safe practice)
    glBindVertexArray(0);
}

// Assumes shader is already bound
void MeshGPU::draw() const {
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}
