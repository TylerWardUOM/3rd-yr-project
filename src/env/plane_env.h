#include "env_interface.h"

class PlaneEnv : public EnvInterface {
public:
    PlaneEnv(const Vector3& n_in, double b_in) : n(normalize(n_in)), b(b_in) {}

    double phi(const Vector3& x) const override {
        return dot(n, x) - b;
    }

    Vector3 grad(const Vector3& /*x*/) const override {
        return n;
    }

    Vector3 project(const Vector3& x) const override {
        double t = phi(x);
        return x - n*t;
    }

private:
    Vector3 n; // unit normal
    double b; // offset (plane equation: n.x = b)
};