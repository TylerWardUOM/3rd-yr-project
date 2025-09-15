#version 330 core
in vec3 vWorldPos;
in vec3 vNormal;
in vec3 vColour;

out vec4 FragColor;

void main() {
    // Grid lines
    float gridSize = 1.0; // Size of each grid cell meters
    float thickness = 0.02; // Thickness of grid lines in meters

    float fx = fract(vWorldPos.x / gridSize);
    float fz = fract(vWorldPos.z / gridSize);

    float distX = min(fx, 1.0 - fx);
    float distZ = min(fz, 1.0 - fz);

    float lineX = step(distX, thickness);
    float lineZ = step(distZ, thickness);

    float isLine = max(lineX, lineZ); // line if close to X or Z line

    // Remap normal from [-1,1] to [0,1]
    vec3 n01 = normalize(vNormal) * 0.5 + 0.5;

    // Tint normal-based color with vertex color
    vec3 rgb = vColour * n01;

    vec3 lineColor = vec3(0.0, 0.0, 0.0); // Black color for grid lines

    // Mix grid line color (black) with the base color
    rgb = mix(rgb, lineColor, isLine);

    FragColor = vec4(rgb, 1.0);
}
