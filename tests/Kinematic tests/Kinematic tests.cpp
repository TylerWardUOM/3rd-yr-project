#include <cmath>
#include <glm/glm.hpp>
#include <vector>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/string_cast.hpp>
#include <iostream>

struct basicRobot {
    double link1; // Length of link 1
    double link2; // Length of link 2
};

struct RobotState {
    double jointAngles[3]; // Joint angles for 3 joints
    double endEffectorPos[3]; // End-effector position (x, y, z)
};

RobotState inverseKinematics(basicRobot robot, glm::vec3 target, glm::dvec3& angles) {
    RobotState state;

    double L1 = robot.link1;
    double L2 = robot.link2;

    // Base rotation
    double theta0 = atan2(target.z, target.x);

    // Project into 2D
    double r = sqrt(target.x * target.x + target.z * target.z);
    double y = target.y;

    // Law of cosines for elbow
    double D = (r * r + y * y - L1 * L1 - L2 * L2) / (2 * L1 * L2);
    if (D < -1.0) D = -1.0;
    else if (D > 1.0) D = 1.0;
    double theta2 = acos(D);

    // Shoulder angle
    double theta1 = atan2(y, r) - atan2(L2 * sin(theta2), L1 + L2 * cos(theta2));

    // Save results
    state.jointAngles[0] = theta0;
    state.jointAngles[1] = theta1;
    state.jointAngles[2] = theta2;

    // End effector pos (optional: forward kinematics with angles)
    //state.endEffectorPos = target;

    angles = glm::dvec3(theta0, theta1, theta2);
    return state;
}



glm::dmat4 homogeneousTransformDH(double a, double alpha, double d, double theta) {
    const double ct = std::cos(theta), st = std::sin(theta);
    const double ca = std::cos(alpha), sa = std::sin(alpha);

    glm::dmat4 T(1.0);
    // Row-major DH  write as column-major: T[col][row]
    T[0][0] = ct;        T[1][0] = -st * ca;  T[2][0] = st * sa;  T[3][0] = a * ct; // col 0
    T[0][1] = st;        T[1][1] = ct * ca;  T[2][1] = -ct * sa;  T[3][1] = a * st; // col 1
    T[0][2] = 0.0;       T[1][2] = sa;       T[2][2] = ca;       T[3][2] = d;      // col 2
    T[0][3] = 0.0;       T[1][3] = 0.0;       T[2][3] = 0.0;       T[3][3] = 1.0;    // col 3
    return T;
}


std::vector<glm::dmat4> forwardKinematic(basicRobot robot, glm::dvec3 angles) {
    std::vector<glm::dmat4> transforms;
    glm::dmat4 T = glm::dmat4(1.0f);
    // Base rotation
    glm::dmat4 T_1 = homogeneousTransformDH(0.0, glm::half_pi<double>(), 0.0, angles[0]);
    //std::cout << "T_1: " << glm::to_string(T_1) << std::endl;
    // Shoulder
    glm::dmat4 T_2 = homogeneousTransformDH(robot.link1, 0.0, 0.0, angles[1]);
    std::cout << "T_2: " << glm::to_string(T_2) << std::endl;

    // Elbow
    glm::dmat4 T_3 = homogeneousTransformDH(robot.link2, 0.0, 0.0, angles[2]);

    transforms.push_back(T_1);
    transforms.push_back(T_2);
    transforms.push_back(T_3);
    return transforms;
}

int main() {
    std::cout << "Kinematic Test" << std::endl;
    basicRobot robot = { 1.0, 1.0 }; // Robot with two 1.0 length links
    glm::vec3 target(1.0, 1.0, 0.0); // Target position for end-effector
    glm::dvec3 angles;
    RobotState state = inverseKinematics(robot, target, angles);
    std::cout << "Joint Angles (radians): " << glm::to_string(angles) << std::endl;
    std::vector<glm::dmat4> transforms = forwardKinematic(robot, angles);
    for (size_t i = 0; i < transforms.size(); ++i) {
		std::cout << "Transform " << i + 1 << ": " << glm::to_string(transforms[i]) << std::endl;
    }
    glm::dmat4 T_tot = transforms[0] * transforms[1] * transforms[2];
    glm::dvec3 p_ee = glm::dvec3(T_tot[3]);   // last column  (x,y,z)
    std::cout << "End Effector Position: (" << p_ee.x << ", " << p_ee.y << ", " << p_ee.z << ")\n";

    std::cout << "Target Position: (" 
              << target.x << ", " 
              << target.y << ", " 
		<< target.z << ")" << std::endl;

}
