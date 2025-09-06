// stream.cpp
#include "Enviroment/plane_env.h"
#include <iostream>
#include <thread>
#include <cmath>

int main(){
    PlaneEnv plane({0,1,0}, 2.0);
    for(int i=0;i<500;i++){
        double t = i*0.02;
        Vector3 p{ std::sin(t), -0.5 + 0.5*std::sin(0.3*t), std::cos(t)*0.5 };
        auto q = plane.query(p);
        auto n = plane.grad(p);
        std::cout << n.x << "," << n.y << "," << n.z << ","    // 0..2 normal
                  << 2.0 << ","                                 // 3 offset b
                  << p.x << "," << p.y << "," << p.z << ","     // 4..6 point
                  << q.proj.x << "," << q.proj.y << "," << q.proj.z << "," //7..9 proj
                  << q.phi                                      // 10 phi
                  << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(30));
    }
}