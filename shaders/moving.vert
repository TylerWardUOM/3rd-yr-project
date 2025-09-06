#version 330 core
layout (location = 0) in vec3 aPos;

uniform vec2 u_mouse;

void main()
{   
    vec3 pos = aPos + vec3(u_mouse, 0.0f);
    gl_Position = vec4(pos, 1.0f);
}