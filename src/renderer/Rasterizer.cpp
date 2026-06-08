// 클립 공간 삼각형의 화면 변환과 픽셀 래스터화를 구현합니다.
#include "renderer/Rasterizer.h"
#include "math/MathUtils.h"
#include <algorithm>
#include <cmath>

static const Vec2 kSamples1[1] = { {0.0f, 0.0f} };
static const Vec2 kSamples2[2] = { {-0.25f,-0.25f}, { 0.25f, 0.25f} };
static const Vec2 kSamples4[4] = { {-0.375f, 0.125f}, { 0.125f, 0.375f},
                                   { 0.375f,-0.125f}, {-0.125f,-0.375f} };

const Vec2* Rasterizer::SubSampleOffsets(int count) {
    if (count >= 4) return kSamples4;
    if (count == 2) return kSamples2;
    return kSamples1;
}

Vec3 Rasterizer::Barycentric(Vec2 A, Vec2 B, Vec2 C, Vec2 P) {
    float denom = (B.y - C.y) * (A.x - C.x) + (C.x - B.x) * (A.y - C.y);
    if (std::abs(denom) < 1e-7f) return { -1, 1, 1 };
    float l0 = ((B.y - C.y) * (P.x - C.x) + (C.x - B.x) * (P.y - C.y)) / denom;
    float l1 = ((C.y - A.y) * (P.x - C.x) + (A.x - C.x) * (P.y - C.y)) / denom;
    float l2 = 1.0f - l0 - l1;
    return { l0, l1, l2 };
}

void Rasterizer::DrawTriangle(VertexOut v0, VertexOut v1, VertexOut v2, IShader& shader) {
    int W = m_fb.Width(), H = m_fb.Height();
    int N = m_fb.SampleCount();
    bool msaa = (N > 1);

    if (!ClipW(v0.clipPos) || !ClipW(v1.clipPos) || !ClipW(v2.clipPos)) return;

    Vec3 ndc0 = v0.clipPos.PerspectiveDivide();
    Vec3 ndc1 = v1.clipPos.PerspectiveDivide();
    Vec3 ndc2 = v2.clipPos.PerspectiveDivide();

    auto toScreen = [&](Vec3 ndc) -> Vec2 {
        return { (ndc.x + 1.0f) * 0.5f * (W - 1),
                 (1.0f - ndc.y) * 0.5f * (H - 1) };
    };

    Vec2 s0 = toScreen(ndc0), s1 = toScreen(ndc1), s2 = toScreen(ndc2);

    float area = (s1.x - s0.x) * (s2.y - s0.y) - (s1.y - s0.y) * (s2.x - s0.x);
    if (area >= 0.0f) return;

    int minX = (int)std::max(0.0f,       std::floor(std::min({ s0.x, s1.x, s2.x })));
    int maxX = (int)std::min((float)(W-1), std::ceil( std::max({ s0.x, s1.x, s2.x })));
    int minY = (int)std::max(0.0f,       std::floor(std::min({ s0.y, s1.y, s2.y })));
    int maxY = (int)std::min((float)(H-1), std::ceil( std::max({ s0.y, s1.y, s2.y })));

    float rw0 = 1.0f / v0.clipPos.w;
    float rw1 = 1.0f / v1.clipPos.w;
    float rw2 = 1.0f / v2.clipPos.w;

    const Vec2* offsets = SubSampleOffsets(N);

    for (int py = minY; py <= maxY; ++py) {
        for (int px = minX; px <= maxX; ++px) {

            if (msaa) {

                bool  anyCovered = false;
                Color centerColor(0, 0, 0);

                for (int s = 0; s < N; ++s) {
                    Vec2 P = { px + offsets[s].x, py + offsets[s].y };
                    Vec3 bc = Barycentric(s0, s1, s2, P);
                    if (bc.x < 0 || bc.y < 0 || bc.z < 0) continue;

                    float wc0 = bc.x*rw0, wc1 = bc.y*rw1, wc2 = bc.z*rw2;
                    float wSum = wc0 + wc1 + wc2;
                    if (wSum < 1e-9f) continue;
                    wc0 /= wSum; wc1 /= wSum; wc2 /= wSum;
                    float depth = ndc0.z*wc0 + ndc1.z*wc1 + ndc2.z*wc2;

                    if (!anyCovered) {

                        Vec3 bc2 = Barycentric(s0, s1, s2, { (float)px, (float)py });
                        float c0 = bc2.x*rw0, c1 = bc2.y*rw1, c2 = bc2.z*rw2;
                        float cs = c0+c1+c2; if (cs < 1e-9f) cs = 1e-9f;
                        c0/=cs; c1/=cs; c2/=cs;
                        auto lerpV3 = [&](Vec3 a, Vec3 b, Vec3 c) {
                            return Vec3{a.x*c0+b.x*c1+c.x*c2, a.y*c0+b.y*c1+c.y*c2, a.z*c0+b.z*c1+c.z*c2};
                        };
                        Varying frag;
                        frag.worldPos = lerpV3(v0.varying.worldPos, v1.varying.worldPos, v2.varying.worldPos);
                        frag.normal   = lerpV3(v0.varying.normal,   v1.varying.normal,   v2.varying.normal).normalized();
                        frag.tangent  = lerpV3(v0.varying.tangent,  v1.varying.tangent,  v2.varying.tangent).normalized();
                        frag.uv.x     = v0.varying.uv.x*c0 + v1.varying.uv.x*c1 + v2.varying.uv.x*c2;
                        frag.uv.y     = v0.varying.uv.y*c0 + v1.varying.uv.y*c1 + v2.varying.uv.y*c2;
                        centerColor = shader.Fragment(frag);
                        anyCovered = true;
                    }
                    m_fb.TestAndSetMSAA(px, py, s, depth, centerColor);
                }
            } else {

                Vec3 bc = Barycentric(s0, s1, s2, { (float)px, (float)py });
                if (bc.x < 0 || bc.y < 0 || bc.z < 0) continue;

                float wc0 = bc.x*rw0, wc1 = bc.y*rw1, wc2 = bc.z*rw2;
                float wSum = wc0+wc1+wc2;
                if (wSum < 1e-9f) continue;
                wc0/=wSum; wc1/=wSum; wc2/=wSum;

                float depth = ndc0.z*wc0 + ndc1.z*wc1 + ndc2.z*wc2;
                if (!m_fb.TestAndSetDepth(px, py, depth)) continue;

                Varying frag;
                auto lerpV3 = [&](Vec3 a, Vec3 b, Vec3 c) {
                    return Vec3{a.x*wc0+b.x*wc1+c.x*wc2, a.y*wc0+b.y*wc1+c.y*wc2, a.z*wc0+b.z*wc1+c.z*wc2};
                };
                frag.worldPos = lerpV3(v0.varying.worldPos, v1.varying.worldPos, v2.varying.worldPos);
                frag.normal   = lerpV3(v0.varying.normal,   v1.varying.normal,   v2.varying.normal).normalized();
                frag.tangent  = lerpV3(v0.varying.tangent,  v1.varying.tangent,  v2.varying.tangent).normalized();
                frag.uv.x     = v0.varying.uv.x*wc0 + v1.varying.uv.x*wc1 + v2.varying.uv.x*wc2;
                frag.uv.y     = v0.varying.uv.y*wc0 + v1.varying.uv.y*wc1 + v2.varying.uv.y*wc2;
                frag.color.r  = (uint8_t)(v0.varying.color.r*wc0 + v1.varying.color.r*wc1 + v2.varying.color.r*wc2);
                frag.color.g  = (uint8_t)(v0.varying.color.g*wc0 + v1.varying.color.g*wc1 + v2.varying.color.g*wc2);
                frag.color.b  = (uint8_t)(v0.varying.color.b*wc0 + v1.varying.color.b*wc1 + v2.varying.color.b*wc2);

                m_fb.SetPixel(px, py, shader.Fragment(frag));
            }
        }
    }
}
