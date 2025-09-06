
#include <cmath>

// Minimal 3D vector
struct Vector3 {
    double x, y, z;
    Vector3 operator+(const Vector3& o) const { return {x+o.x, y+o.y, z+o.z}; }
    Vector3 operator-(const Vector3& o) const { return {x-o.x, y-o.y, z-o.z}; }
    Vector3 operator*(double s) const { return {x*s, y*s, z*s}; }
    Vector3 operator/(double s) const { return {x/s, y/s, z/s}; }
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