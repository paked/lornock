#version 330 core

in vec2 UV;
out vec4 FragColor;

uniform vec3 color;
uniform sampler2D tex;
uniform vec2 uv_offset;
uniform vec2 uv_range;

void main() {
    float d = texture(tex, (UV * uv_range) + uv_offset).r;

    FragColor = vec4(color, d);

    float gamma = 2.2;
    FragColor.rgb = pow(FragColor.rgb, vec3(1.0/gamma));
}
