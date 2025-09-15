#version 330 core
in vec3 vWorldPos;
in vec3 vNormal;
in vec3 vColour;
in float vUseGrid;

out vec4 FragColor;

void main() {
    // Grid lines
    float gridSize = 1.0; // Size of each grid cell meters
    float thickness = 0.015; // Thickness of grid lines in meters

    float fx = fract(vWorldPos.x / gridSize);
    float fz = fract(vWorldPos.z / gridSize);

    float distX = min(fx, 1.0 - fx);
    float distZ = min(fz, 1.0 - fz);

    float lineX = step(distX, thickness);
    float lineZ = step(distZ, thickness);

    float isLine = max(lineX, lineZ); // line if close to X or Z line

    // sub-grid lines
    float subGridSize = 0.5; // Size of each sub-grid cell in meters
    float thickness2 = 0.01; // Thickness of sub-grid lines in meters
    float sfx = fract(vWorldPos.x / subGridSize);
    float sfz = fract(vWorldPos.z / subGridSize);
    float sdistX = min(sfx, 1.0 - sfx);
    float sdistZ = min(sfz, 1.0 - sfz);
    float slineX = step(sdistX, thickness2);
    float slineZ = step(sdistZ, thickness2);
    float isSubLine = max(slineX, slineZ); // line if close to X or Z line

    // sub-sub-grid lines
    float subSubGridSize = 0.1; // Size of each sub-sub-grid cell in meters
    float thickness3 = 0.08; // Thickness of sub-sub-grid lines in meters
    float ssfx = fract(vWorldPos.x / subSubGridSize);
    float ssfz = fract(vWorldPos.z / subSubGridSize);
    float ssdistX = min(ssfx, 1.0 - ssfx);
    float ssdistZ = min(ssfz, 1.0 - ssfz);
    float sslineX = step(ssdistX, thickness3);
    float sslineZ = step(ssdistZ, thickness3);
    float isSubSubLine = max(sslineX, sslineZ); // line if close to X or Z line

    // Simple diffuse lighting with fixed light direction
    vec3 norm = normalize(vNormal);
    vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0)); // simple directional light
    float diff = max(dot(norm, lightDir), 0.0);

    vec3 baseColor = vColour;   // per-vertex or uniform color
    vec3 rgb = baseColor * diff;

    vec3 lineColor = vec3(0.1, 0.1, 0.1); //  color for grid lines


    if (vUseGrid > 0.5) {
        if (isLine > 0.5) {
            rgb = mix(rgb,lineColor, isLine); // main line
        } 
        else if (isSubLine > 0.5) {
            rgb = mix(rgb, lineColor, isSubLine*0.7); // medium strength
        } 
        else if (isSubSubLine > 0.5) {
            rgb = mix(rgb, lineColor, isSubSubLine*0.4); // faint
        }
    }
    FragColor = vec4(rgb, 1.0);
}
