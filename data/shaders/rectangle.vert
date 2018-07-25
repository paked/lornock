#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aUV;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec2 UV;

void main() {
    vec4 screen_pos = projection * view * model * vec4(aPos, 1);
    gl_Position = screen_pos;

    UV = aUV;
}
