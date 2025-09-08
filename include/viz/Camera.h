// Camera.h
#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct Camera {
    glm::vec3 eye{1,3,3};
    
    // Euler angles (degrees)
    float yawDeg  = -135.f; // pointing roughly toward origin from (1,3,3)
    float pitchDeg= -20.f;
    
    //lens
    float fovDeg{60.f}, aspect{1.f}, znear{0.1f}, zfar{100.f};

    // Derived vectors (updated from yaw/pitch)
    glm::vec3 front{0.f,0.f,-1.f};
    glm::vec3 right{1.f,0.f,0.f};
    glm::vec3 camUp{0.f,1.f,0.f};

    // API to get view/proj matrices
    glm::mat4 view()  const { return glm::lookAt(eye, eye+front, camUp); }
    glm::mat4 proj()  const { return glm::perspective(glm::radians(fovDeg), aspect, znear, zfar); }


    // Update front/right/up from yaw/pitch
    void updateVectors() {
        const float cy = cos(glm::radians(yawDeg));
        const float sy = sin(glm::radians(yawDeg));
        const float cp = cos(glm::radians(pitchDeg));
        const float sp = sin(glm::radians(pitchDeg));
        front = glm::normalize(glm::vec3(cy*cp, sp, sy*cp));  
        right = glm::normalize(glm::cross(front, glm::vec3(0.f,1.f,0.f)));
        camUp = glm::normalize(glm::cross(right, front));
    }

    // Clamp pitch to avoid flipping
    void clampPitch(float minDeg=-89.f, float maxDeg=+89.f) {
        if (pitchDeg < minDeg) pitchDeg = minDeg;
        if (pitchDeg > maxDeg) pitchDeg = maxDeg;
    }

    // Add to yaw/pitch, clamp pitch, update vectors
    void addYawPitch(float dYaw, float dPitch) {
        yawDeg   += dYaw;
        pitchDeg += dPitch;
        clampPitch();
        updateVectors();
    }
};
