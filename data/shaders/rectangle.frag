#version 330 core

uniform vec4 color;

out vec4 FragColor;

void main() {
    FragColor = color;

    float gamma = 2.2;
    FragColor.rgb = pow(FragColor.rgb, vec3(1.0/gamma));
}
