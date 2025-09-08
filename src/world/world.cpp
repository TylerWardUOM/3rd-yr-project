#include "world/World.h"
#include "meshing/Mesher.h"
#include "viz/MeshGPU.h"
#include <iostream>

BodyId addBody(World& W, std::shared_ptr<const EnvInterface> env, Shader* shader, const MCParams& mc) {
    BodyId id = static_cast<BodyId>(W.statics.size()); // new ID is current size
    W.statics.push_back({std::move(env)}); // add new static body to back of array
    for (auto& buf : W.poses.buf) // add a new pose to each buffer
        buf.poses.resize(W.statics.size()); // resize to match new size
        
    W.renderables.emplace_back(); // add new renderable to back of array
    auto& r = W.renderables.back(); // reference to new renderable
    r.shader = shader;           // set shader
    r.mesh = std::make_unique<MeshGPU>(); // create new MeshGPU object

    // Generate mesh from SDF
    std::vector<float> vertices;
    std::vector<unsigned> indices;
    Mesher mesher;
    const auto& env_ref = *W.statics[id].env;
    Mesh mesh = mesher.makeMeshMC(env_ref, mc.minB, mc.maxB, mc.nx, mc.ny, mc.nz, mc.iso); 
    mesher.packPosNrmIdx(mesh, vertices, indices);
    r.mesh->upload(vertices, indices);             // GPU upload (GL thread)
    std::cout << "Added body " << id << " with " 
              << (vertices.size()/6) << " vertices and "
              << (indices.size()/3) << " triangles.\n" << env_ref.phi({0.0,0.0,0.0}) << std::endl;
    return id;
}

// Writer: haptics/physics thread
void writePose(World& W, BodyId id, const Pose& p) {

    uint8_t r = W.poses.read_index.load(std::memory_order_acquire);  // current read index
    uint8_t w = 1 - r; // write into the other buffer
    auto& buf = W.poses.buf[w];

    // Begin write (mark odd)
    buf.seq.fetch_add(1, std::memory_order_acq_rel);

    // Write pose
    buf.poses[id] = p;

    // End write (mark even)
    buf.seq.fetch_add(1, std::memory_order_acq_rel);

    // Publish this buffer as the new read buffer
    W.poses.read_index.store(w, std::memory_order_release);
}

// Reader: render thread
bool readPose(const World& W, BodyId id, Pose& out) {
    uint8_t r = W.poses.read_index.load(std::memory_order_acquire); // current read index
    auto const& buf = W.poses.buf[r];

    // Snapshot with seqlock
    uint64_t s0 = buf.seq.load(std::memory_order_acquire);
    if (s0 & 1ull) return false; // write in progress

    Pose tmp = buf.poses[id];

    uint64_t s1 = buf.seq.load(std::memory_order_acquire);
    if (s0 != s1 || (s1 & 1ull)) return false; // changed mid-read

    out = tmp;
    return true;
}