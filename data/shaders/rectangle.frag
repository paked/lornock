#version 330 core

uniform sampler2D ourTexture;
uniform vec4 color;

in vec2 UV;

out vec4 FragColor;

void main() {
    FragColor = color;
}
