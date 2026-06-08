// Phong 조명과 노말맵 및 그림자를 처리하는 셰이더를 선언합니다.
#pragma once
#include "renderer/Shader.h"
#include "renderer/ShadowMap.h"
#include "resource/Texture.h"
#include "scene/Light.h"
#include "math/Mat4.h"
#include "math/Vec3.h"

struct Mesh;

struct PhongShader : IShader {
    const Mesh*      mesh      = nullptr;
    const Texture*   albedo    = nullptr;
    const Texture*   normalMap = nullptr;
    const ShadowMap* shadowMap = nullptr;
    Mat4             mvp;
    Mat4             modelMat;
    Vec3             cameraPos;
    Vec3             tint = { 1.0f, 1.0f, 1.0f };
    Light            light;

    VertexOut Vertex(int idx) override;
    Color Fragment(const Varying& v) override;
};
