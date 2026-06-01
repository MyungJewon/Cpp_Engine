#pragma once
#include "renderer/Shader.h"
#include "renderer/ShadowMap.h"
#include "resource/Texture.h"
#include "scene/Light.h"
#include "math/Mat4.h"
#include "math/Vec3.h"

struct Mesh;

// Phong + Shadow Map + Normal Map + 텍스처 통합 셰이더
struct PhongShader : IShader {
    const Mesh*      mesh      = nullptr;
    const Texture*   albedo    = nullptr;  // diffuse 텍스처
    const Texture*   normalMap = nullptr;  // tangent-space 노말맵
    const ShadowMap* shadowMap = nullptr;
    Mat4             mvp;
    Mat4             modelMat;
    Vec3             cameraPos;
    Light            light;

    VertexOut Vertex(int idx) override;
    Color Fragment(const Varying& v) override;
};
