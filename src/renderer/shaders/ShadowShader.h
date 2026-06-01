#pragma once
#include "renderer/Shader.h"
#include "math/Mat4.h"

struct Mesh;

// 깊이만 기록하는 Shadow Pass 셰이더
struct ShadowShader : IShader {
    const Mesh* mesh = nullptr;
    Mat4 lightMVP;

    VertexOut Vertex(int idx) override;
    Color Fragment(const Varying& v) override;
};
