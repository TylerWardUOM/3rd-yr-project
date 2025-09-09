#version 330 core
in vec3 vNormal;
in vec3 vColour;
out vec4 FragColor;

void main() {
    // Remap normal from [-1,1] to [0,1]
    vec3 n01 = normalize(vNormal) * 0.5 + 0.5;

    // Tint normal-based color with vertex color
    vec3 rgb = vColour * n01;

    FragColor = vec4(rgb, 1.0);
}
