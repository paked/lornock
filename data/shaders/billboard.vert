/*
 * Adapted from: https://github.com/opengl-tutorials/ogl/blob/master/tutorial18_billboards_and_particles/Billboard.vertexshader
 */

#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

uniform mat4 view;
uniform mat4 projection;

uniform vec3 cameraRight;
uniform vec3 cameraUp;
uniform vec3 billboardPos;
uniform vec2 billboardSize;

out vec2 TexCoord;

void main() {
    vec3 pos = 
        billboardPos
        + cameraRight * aPos.x * billboardSize.x
        + cameraUp * aPos.y * billboardSize.y;

    gl_Position = projection * view * vec4(pos, 1.0f);

    TexCoord = aTexCoord;
}
