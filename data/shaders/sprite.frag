#version 330 core

uniform sampler2D ourTexture;

in vec2 UV;

out vec4 FragColor;

void main() {
    float depthValue = texture(ourTexture, UV).r;
    FragColor = vec4(vec3(depthValue), 1.0);
}
