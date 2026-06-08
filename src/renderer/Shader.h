// 소프트웨어 렌더러의 정점 출력과 셰이더 인터페이스를 정의합니다.
#pragma once
#include "math/Vec2.h"
#include "math/Vec3.h"
#include "math/Vec4.h"
#include "math/Mat4.h"
#include "renderer/Framebuffer.h"

struct Varying {
    Vec3  worldPos;
    Vec3  normal;
    Vec3  tangent;
    Vec2  uv;
    Color color;
};

struct VertexOut {
    Vec4    clipPos;
    Varying varying;
};

struct IShader {
    virtual ~IShader() = default;
    virtual VertexOut  Vertex(int index)          = 0;
    virtual Color      Fragment(const Varying& v) = 0;
};
