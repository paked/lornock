#version 330 core

uniform sampler2D ourTexture;

in vec2 UV;

out vec4 FragColor;

void main() {
    FragColor = texture(ourTexture, UV);

    float gamma = 2.2;
    FragColor.rgb = pow(FragColor.rgb, vec3(1.0/gamma));
}
