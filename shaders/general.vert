#version 330 core

// ---------- Attributes ----------
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;

// ---------- Uniforms ----------
uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProj;
uniform mat3 uNormalMatrix;
uniform vec3 uColour;

// ---------- Varyings to fragment ----------
out vec3 vWorldPos;
out vec3 vNormal;
out vec3 vColour;

void main()
{
    mat4 M =  uModel;

    // World-space position
    vec4 worldPos4 = M * vec4(aPos, 1.0);

    vWorldPos = worldPos4.xyz;

    vNormal = normalize(uNormalMatrix * aNormal);

    vColour = uColour;

    gl_Position = uProj * uView * worldPos4;
}
