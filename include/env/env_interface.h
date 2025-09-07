
#include <cmath>
#include <vector>
#include <cstdint>

// Minimal 3D vector
struct Vector3 {
    double x, y, z;
    Vector3 operator+(const Vector3& o) const { return {x+o.x, y+o.y, z+o.z}; }
    Vector3 operator-(const Vector3& o) const { return {x-o.x, y-o.y, z-o.z}; }
    Vector3 operator*(double s) const { return {x*s, y*s, z*s}; }
    Vector3 operator/(double s) const { return {x/s, y/s, z/s}; }
    
};

inline Vector3 cross(const Vector3& a, const Vector3& b) {
    return {
        a.y*b.z - a.z*b.y,
        a.z*b.x - a.x*b.z,
        a.x*b.y - a.y*b.x
    };
}

struct Matrix4 {
    double m[4][4];
    Vector3 operator*(const Vector3& v) const {
        double x = m[0][0]*v.x + m[0][1]*v.y + m[0][2]*v.z + m[0][3];
        double y = m[1][0]*v.x + m[1][1]*v.y + m[1][2]*v.z + m[1][3];
        double z = m[2][0]*v.x + m[2][1]*v.y + m[2][2]*v.z + m[2][3];
        double w = m[3][0]*v.x + m[3][1]*v.y + m[3][2]*v.z + m[3][3];
        if (w != 0) { x /= w; y /= w; z /= w; }
        return {x, y, z};
    }
};

// Dot product
inline double dot(const Vector3& a, const Vector3& b) {
    return a.x*b.x + a.y*b.y + a.z*b.z;
}

// Norm (length)
inline double norm(const Vector3& a) {
    return std::sqrt(dot(a,a));
}

// Normalize (with fallback)
inline Vector3 normalize(const Vector3& a) {
    double n = norm(a);
    return (n > 1e-9) ? a/n : Vector3{1,0,0};
}


// Result structure
struct QueryResult {
    double phi;   // signed distance
    Vector3 grad;    // gradient (normal, not necessarily unit)
    Vector3 proj;    // projected point
    bool inside;  // phi < 0
};

struct Vertex {
    Vector3 pos;
    Vector3 nrm;
};
struct Mesh {
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices; // 2 triangles: 6 indices
};


class EnvInterface {
public:
    virtual ~EnvInterface() = default;

    virtual double phi(const Vector3& x) const = 0;
    virtual Vector3 grad(const Vector3& x) const = 0;
    virtual Vector3 project(const Vector3& x) const = 0;

    virtual QueryResult query(const Vector3& x) const {
        QueryResult res;
        res.phi = phi(x);
        res.grad = grad(x);
        res.proj = project(x);
        res.inside = (res.phi < 0);
        return res;
    };

};