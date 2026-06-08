// 삼각형 래스터화와 무게중심 좌표 계산 기능을 선언합니다.
#pragma once
#include "renderer/Framebuffer.h"
#include "renderer/Shader.h"
#include "math/Vec2.h"
#include "math/Vec3.h"
#include "math/Vec4.h"

class Rasterizer {
public:
    explicit Rasterizer(Framebuffer& fb) : m_fb(fb) {}

    void DrawTriangle(VertexOut v0, VertexOut v1, VertexOut v2, IShader& shader);

private:
    Vec3 Barycentric(Vec2 A, Vec2 B, Vec2 C, Vec2 P);
    bool ClipW(const Vec4& v) { return v.w > 0.0f; }

    static const Vec2* SubSampleOffsets(int count);

    Framebuffer& m_fb;
};
