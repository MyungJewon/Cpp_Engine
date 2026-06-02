#version 330 core

in vec3 vWorldPos;
in vec2 vUV;
in vec3 vNormal;
in vec3 vTangent;
in vec4 vLightClipPos;

uniform vec3 uCameraPos;
uniform vec3 uLightPos;
uniform vec3 uLightColor;
uniform vec3 uTint;
uniform float uAmbient;
uniform float uDiffuse;
uniform float uSpecular;
uniform float uShininess;
uniform bool uHasAlbedo;
uniform bool uHasNormalMap;
uniform sampler2D uAlbedo;
uniform sampler2D NormalMap;
uniform sampler2D ShadowMap;

out vec4 FragColor;

float ShadowFactor(vec3 normal, vec3 lightDir)
{
    vec3 projCoords = vLightClipPos.xyz / vLightClipPos.w;
    projCoords = projCoords * 0.5 + 0.5;

    if (projCoords.x < 0.0 || projCoords.x > 1.0 ||
        projCoords.y < 0.0 || projCoords.y > 1.0 ||
        projCoords.z < 0.0 || projCoords.z > 1.0) {
        return 0.0;
    }

    float closestDepth = texture(ShadowMap, projCoords.xy).r;
    float currentDepth = projCoords.z;
    float bias = max(0.005 * (1.0 - dot(normal, lightDir)), 0.001);
    return currentDepth > closestDepth + bias ? 1.0 : 0.0;
}

void main()
{
    vec3 normal = normalize(vNormal);
    if (uHasNormalMap) {
        vec3 tangentNormal = texture(NormalMap, vUV).rgb * 2.0 - 1.0;
        vec3 tangent = normalize(vTangent);
        vec3 bitangent = normalize(cross(normal, tangent));
        mat3 tbn = mat3(tangent, bitangent, normal);
        normal = normalize(tbn * tangentNormal);
    }

    vec3 lightDir = normalize(uLightPos - vWorldPos);
    vec3 viewDir = normalize(uCameraPos - vWorldPos);
    vec3 reflectDir = normalize(reflect(-lightDir, normal));

    float diff = max(dot(normal, lightDir), 0.0);
    float spec = pow(max(dot(reflectDir, viewDir), 0.0), uShininess);
    float shadow = ShadowFactor(normal, lightDir);

    vec3 base = uHasAlbedo ? texture(uAlbedo, vUV).rgb : uTint;
    float intensity = uAmbient + (1.0 - shadow) * (uDiffuse * diff + uSpecular * spec);
    FragColor = vec4(base * uLightColor * intensity, 1.0);
}
