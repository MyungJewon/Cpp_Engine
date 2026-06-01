#pragma once
#include "renderer/Shader.h"
#include "math/Mat4.h"

struct Mesh;

// OIT용 반투명 셰이더 (단색 + alpha)
struct TransparentShader : IShader {
    const Mesh* mesh  = nullptr;
    Mat4        mvp;
    Color       color = Color(100, 180, 255);
    float       alpha = 0.4f;

    VertexOut Vertex(int idx) override;
    Color Fragment(const Varying& v) override;
};
