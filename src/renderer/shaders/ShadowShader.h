// 깊이 기록에 사용하는 그림자 패스 셰이더를 선언합니다.
#pragma once
#include "renderer/Shader.h"
#include "math/Mat4.h"

struct Mesh;

struct ShadowShader : IShader {
    const Mesh* mesh = nullptr;
    Mat4 lightMVP;

    VertexOut Vertex(int idx) override;
    Color Fragment(const Varying& v) override;
};
