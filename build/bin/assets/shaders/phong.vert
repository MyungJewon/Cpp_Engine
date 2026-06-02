#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aUV;
layout (location = 2) in vec3 aNormal;
layout (location = 3) in vec3 aTangent;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;
uniform mat4 uLightMVP;

out vec3 vWorldPos;
out vec2 vUV;
out vec3 vNormal;
out vec3 vTangent;
out vec4 vLightClipPos;

void main()
{
    vec4 worldPos = uModel * vec4(aPos, 1.0);
    mat3 normalMat = mat3(uModel);

    vWorldPos = worldPos.xyz;
    vUV = aUV;
    vNormal = normalize(normalMat * aNormal);
    vTangent = normalize(normalMat * aTangent);
    vLightClipPos = uLightMVP * vec4(aPos, 1.0);

    gl_Position = uProjection * uView * worldPos;
}
