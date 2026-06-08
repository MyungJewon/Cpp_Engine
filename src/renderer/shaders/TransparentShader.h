// OIT 합성에 사용할 반투명 단색 셰이더를 선언합니다.
#pragma once
#include "renderer/Shader.h"
#include "math/Mat4.h"

struct Mesh;

struct TransparentShader : IShader {
    const Mesh* mesh  = nullptr;
    Mat4        mvp;
    Color       color = Color(100, 180, 255);
    float       alpha = 0.4f;

    VertexOut Vertex(int idx) override;
    Color Fragment(const Varying& v) override;
};
