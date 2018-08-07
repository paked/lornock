#version 330 core

in vec2 UV;
out vec4 FragColor;

uniform sampler2D tex;
uniform vec2 uv_offset;
uniform vec2 uv_range;

void main() {
    float d = texture(tex, (UV * uv_range) + uv_offset).r;

    FragColor = d * vec4(1.0f);
}
