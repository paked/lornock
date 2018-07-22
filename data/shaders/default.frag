#version 330 core

uniform sampler2D ourTexture;
uniform vec3 lightColor;
uniform vec3 lightPos;

in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;

out vec4 FragColor;

void main() {
    vec4 objectColor = texture(ourTexture, TexCoord);

    float ambientStrength = 0.2;
    vec3 ambient = ambientStrength * lightColor;

    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);

    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    vec3 result = ambient + diffuse;

    FragColor = vec4(result, 1.0);

	FragColor *= objectColor;
}
