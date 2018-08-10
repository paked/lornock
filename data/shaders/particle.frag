#version 330 core

uniform sampler2D ourTexture;
uniform sampler2D colorMap;

in vec2 UV;

out vec4 FragColor;

void main() {
    vec4 color = texture(ourTexture, UV);

    FragColor = texture(colorMap, vec2(color.a, 0));
    FragColor.a = color.a;

    float gamma = 2.2;
    FragColor.rgb = pow(FragColor.rgb, vec3(1.0/gamma));
}
