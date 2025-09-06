#include "plane_env.h"
#include <stdio.h>
#include <iostream>

int main() {
    PlaneEnv plane({0,1,0}, 2); // y=0 plane

    Vector3 p{0, -0.5, 0};
    auto q1 = plane.query(p);
    std::cout << "Plane phi=" << q1.phi << " proj=(" << q1.proj.x << "," << q1.proj.y << "," << q1.proj.z << ")\n";

}
