#include <glm/glm.hpp>
#include <functional>
#include <vector>

struct Vertex { glm::dvec3 pos; glm::dvec3 nrm; };
struct Mesh   { std::vector<Vertex> vertices; std::vector<uint32_t> indices; };

class MarchingCubes {
public:
    Mesh generateMeshFromSDF(
        const std::function<double(const glm::dvec3&)>& sdf,
        const glm::dvec3& minBound, const glm::dvec3& maxBound,
        int nx, int ny, int nz, double iso = 0.0) const;

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
