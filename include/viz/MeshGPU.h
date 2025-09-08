#pragma once
#include <glad/glad.h>
#include <vector>

// Simple GPU-resident mesh (VBO, EBO, VAO)
class MeshGPU {
public:
    MeshGPU(); // constructor
    ~MeshGPU(); //destructor

    // no copy
    MeshGPU(const MeshGPU&) = delete;
    MeshGPU& operator=(const MeshGPU&) = delete;

    // move
    MeshGPU(MeshGPU&& o) noexcept;
    MeshGPU& operator=(MeshGPU&& o) noexcept;

    // Upload interleaved [pos.xyz | nrm.xyz], triangles via indices
    void upload(const std::vector<float>& interleavedPosNorm,
                const std::vector<unsigned>& indices);

    // Draw the mesh (assumes shader is bound)
    void draw() const;

private:
    GLuint VAO{}, VBO{}, EBO{};
    GLsizei count{};
};
