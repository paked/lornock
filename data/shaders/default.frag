#version 330 core

uniform sampler2D ourTexture;
uniform sampler2D shadowMap;

uniform vec3 lightColor;
uniform vec3 lightPos;

in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;
in vec4 FragPosLightSpace;

out vec4 FragColor;

float ShadowCalculation(vec4 fragPosLightSpace){
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;

    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMap, projCoords.xy).r; 
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;

	float bias = 0.005;

    // check whether current frag pos is in shadow
    float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;

    if (projCoords.z > 1.0) {
        shadow = 0.0f;
    }

    return shadow;
}

void main() {
    vec4 objectColor = texture(ourTexture, TexCoord);

    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;

    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);

    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    float shadow = ShadowCalculation(FragPosLightSpace);

    vec3 result = ambient + (1 - shadow) * diffuse;

    FragColor = vec4(result, 1.0);

	FragColor *= objectColor;

    float gamma = 2.2;
    FragColor.rgb = pow(FragColor.rgb, vec3(1.0/gamma));
}
