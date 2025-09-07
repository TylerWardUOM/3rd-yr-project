#include <glm/glm.hpp>
#include <functional>
#include <vector>

/// Mesh format used by marching cubes / renderers
struct Vertex {
    glm::dvec3 pos;     // position (double precision)
    glm::dvec3 nrm;     // normal   (double precision)
};

struct Mesh {
    std::vector<Vertex>   vertices;
    std::vector<uint32_t> indices;  // triangle index buffer
};

class MarchingCubes {
public:
    Mesh generateMeshFromSDF(
        const std::function<double(const glm::dvec3&)>& sdf,
        const glm::dvec3& minBound, const glm::dvec3& maxBound,
        int nx, int ny, int nz, double iso,
        const std::function<glm::dvec3(const glm::dvec3&)>& gradFn) const;

private:
    static std::size_t gridIndex(int i,int j,int k,int ny,int nz);
    static std::vector<double> sampleSDFGrid(
        const std::function<double(const glm::dvec3&)>& sdf,
        const glm::dvec3& minB, const glm::dvec3& step,
        int nx,int ny,int nz);
    static glm::dvec3 estimateNormal(
        const std::function<double(const glm::dvec3&)>& sdf,
        const glm::dvec3& p, double h);
    static glm::dvec3 lerpEdge(
        const glm::dvec3& p0, const glm::dvec3& p1,
        double f0, double f1, double iso);
    static int  computeCubeIndex(const double S[8], double iso);
    static void appendOrientedTri(
        Mesh& mesh,
        const glm::dvec3& p0, const glm::dvec3& n0,
        const glm::dvec3& p1, const glm::dvec3& n1,
        const glm::dvec3& p2, const glm::dvec3& n2);
};
