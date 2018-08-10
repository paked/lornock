#version 330 core

uniform sampler2D ourTexture;
uniform sampler2D colorMap;

in vec2 UV;

out vec4 FragColor;

void main() {
    float d = texture(ourTexture, UV).r;

    FragColor = texture(colorMap, vec2(d, 0)) * texture(ourTexture, UV).a;

    if (FragColor.a < 0.1) 
        discard;

    float gamma = 2.2;
    FragColor.rgb = pow(FragColor.rgb, vec3(1.0/gamma));
}
