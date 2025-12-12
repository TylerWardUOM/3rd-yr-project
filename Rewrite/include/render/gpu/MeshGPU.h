#pragma once
#include <glad/glad.h>
#include <vector>

// Simple GPU-resident mesh (VBO, EBO, VAO)
class MeshGPU {
public:
    MeshGPU() noexcept = default; // constructor
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

    bool isUploaded() const noexcept { return count > 0; }


private:

    void destroy() noexcept;
    void steal(MeshGPU& o) noexcept;

    GLuint VAO{}, VBO{}, EBO{};
    GLsizei count{};
};
