// 광원 시점 깊이 버퍼 저장과 샘플링을 구현합니다.
#include "renderer/ShadowMap.h"

ShadowMap::ShadowMap(int width, int height)
    : m_width(width), m_height(height)
    , m_depth(width * height, 1.0f)
    , m_lightVP(Mat4::Identity())
{}

void ShadowMap::Clear() {
    std::fill(m_depth.begin(), m_depth.end(), 1.0f);
}

bool ShadowMap::TestAndSet(int x, int y, float depth) {
    if (x < 0 || x >= m_width || y < 0 || y >= m_height) return false;
    int idx = y * m_width + x;
    if (depth < m_depth[idx]) {
        m_depth[idx] = depth;
        return true;
    }
    return false;
}

float ShadowMap::Sample(int x, int y) const {
    if (x < 0 || x >= m_width || y < 0 || y >= m_height) return 1.0f;
    return m_depth[y * m_width + x];
}

Vec3 ShadowMap::WorldToLightNDC(const Vec3& worldPos) const {
    Vec4 clip = m_lightVP * Vec4(worldPos, 1.0f);
    if (clip.w <= 0.0f) return { 0, 0, 1 };
    return clip.PerspectiveDivide();
}
