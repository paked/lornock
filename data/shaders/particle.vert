#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aUV;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform vec2 scale;

out vec2 UV;

void main() {
    // NOTE(harrison): Reset the combination of the model/view matrixes
    // rotation to nothing.  I should really do a linear algebra course...
    mat4 mv = view * model;

    {
        mv[0][0] = 1;
        mv[0][1] = 0;
        mv[0][2] = 0;

        mv[1][0] = 0;
        mv[1][1] = 1;
        mv[1][2] = 0;

        mv[2][0] = 0;
        mv[2][1] = 0;
        mv[2][2] = 1;
    }

    gl_Position = projection * mv * vec4(aPos.x * scale.x, aPos.y * scale.y, aPos.z, 1.0f);

    UV = aUV;
}
