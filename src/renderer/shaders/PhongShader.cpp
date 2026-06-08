// 소프트웨어 Phong 조명과 텍스처 및 그림자 샘플링을 구현합니다.
#include "renderer/shaders/PhongShader.h"
#include "resource/ObjLoader.h"
#include <algorithm>
#include <cmath>

VertexOut PhongShader::Vertex(int idx) {
    const MeshVertex& v = mesh->vertices[idx];
    Mat4 normalMat = modelMat.NormalMatrix();
    VertexOut out;
    out.clipPos          = mvp * Vec4(v.pos,    1.0f);
    out.varying.worldPos = (modelMat * Vec4(v.pos,    1.0f)).xyz();
    out.varying.normal   = (normalMat * Vec4(v.normal, 0.0f)).xyz().normalized();
    out.varying.tangent  = (normalMat * Vec4(v.tangent,0.0f)).xyz().normalized();
    out.varying.uv       = v.uv;
    return out;
}

Color PhongShader::Fragment(const Varying& v) {

    Vec3 N = v.normal.normalized();
    if (normalMap && normalMap->IsValid()) {
        Color ns   = normalMap->Sample(v.uv.x, v.uv.y);
        Vec3  tsN  = Vec3(ns.r/127.5f-1.0f, ns.g/127.5f-1.0f, ns.b/127.5f-1.0f).normalized();
        Vec3  T    = v.tangent;
        Vec3  B    = N.cross(T);
        N = (T * tsN.x + B * tsN.y + N * tsN.z).normalized();
    }

    Vec3 L = (light.position - v.worldPos).normalized();
    Vec3 V = (cameraPos      - v.worldPos).normalized();
    Vec3 R = (N * (N.dot(L) * 2.0f) - L).normalized();

    float diff = std::max(0.0f, N.dot(L));
    float spec = std::pow(std::max(0.0f, R.dot(V)), light.shininess);

    float shadow = 0.0f;
    if (shadowMap) {
        Vec3 lNDC = shadowMap->WorldToLightNDC(v.worldPos);
        int  sx   = (int)((lNDC.x + 1.0f) * 0.5f * (shadowMap->Width()  - 1));
        int  sy   = (int)((1.0f - lNDC.y)  * 0.5f * (shadowMap->Height() - 1));
        float bias = std::max(0.005f * (1.0f - N.dot(L)), 0.002f);
        if (lNDC.z > shadowMap->Sample(sx, sy) + bias) shadow = 1.0f;
    }

    float intensity = light.ambient
                    + (1.0f - shadow) * (light.diffuse * diff + light.specular * spec);

    Color base = (albedo && albedo->IsValid())
               ? albedo->Sample(v.uv.x, v.uv.y)
               : Color::FromFloat(tint.x, tint.y, tint.z);

    Vec3 litColor = light.color * intensity;

    return Color(
        (uint8_t)std::min(255.0f, base.r * litColor.x),
        (uint8_t)std::min(255.0f, base.g * litColor.y),
        (uint8_t)std::min(255.0f, base.b * litColor.z)
    );
}
