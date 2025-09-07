#include "env_interface.h"

class PlaneEnv : public EnvInterface {
public:
    PlaneEnv(const Vector3& n_in, double b_in) : n_local(normalize(n_in)), b_local(b_in) {}

    double phi(const Vector3& x_world) const override {
        return dot(n_world, x_world) - b_world;
    }

    Vector3 grad(const Vector3& /*x*/) const override {
        return n_world;
    }

    Vector3 project(const Vector3& x_world) const override {
        double t = phi(x_world);
        return x_world - n_world*t;
    }

    void update(const Matrix4& world_to_local) {
        M = world_to_local;
        // Transform normal (ignore translation)
        n_world = normalize({M.m[0][0]*n_local.x + M.m[1][0]*n_local.y + M.m[2][0]*n_local.z,
                             M.m[0][1]*n_local.x + M.m[1][1]*n_local.y + M.m[2][1]*n_local.z,
                             M.m[0][2]*n_local.x + M.m[1][2]*n_local.y + M.m[2][2]*n_local.z});
        // Transform a point on the plane to world coordinates
        Vector3 p_local = n_local * b_local; // point on plane in local coords
        Vector3 p_world = M * p_local; // transform to world coords
        b_world = dot(n_world, p_world); // new offset in world coords
    }

private:
    Vector3 n_local, n_world; // unit normal
    double b_local, b_world; // offset (plane equation: n.x = b)
    Matrix4 M{}; // world to local transform
};