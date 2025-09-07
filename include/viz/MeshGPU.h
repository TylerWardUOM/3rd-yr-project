#pragma once
#include <glad/glad.h>
#include <vector>

class MeshGPU {
public:
    MeshGPU();
    ~MeshGPU();

    // no copy
    MeshGPU(const MeshGPU&) = delete;
    MeshGPU& operator=(const MeshGPU&) = delete;

    // move
    MeshGPU(MeshGPU&& o) noexcept;
    MeshGPU& operator=(MeshGPU&& o) noexcept;

    // Upload interleaved [pos.xyz | nrm.xyz], triangles via indices
    void upload(const std::vector<float>& interleavedPosNorm,
                const std::vector<unsigned>& indices);

    void draw() const;

private:
    GLuint VAO{}, VBO{}, EBO{};
    GLsizei count{};
};
