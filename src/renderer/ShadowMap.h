// 그림자 판정에 사용할 광원 깊이 맵을 선언합니다.
#pragma once
#include "math/Mat4.h"
#include "math/Vec3.h"
#include "math/MathUtils.h"
#include <vector>

class ShadowMap {
public:
    ShadowMap(int width, int height);

    int   Width()  const { return m_width;  }
    int   Height() const { return m_height; }

    void  Clear();
    bool  TestAndSet(int x, int y, float depth);
    float Sample(int x, int y) const;

    void  SetLightVP(const Mat4& vp) { m_lightVP = vp; }
    Mat4  GetLightVP() const         { return m_lightVP; }

    Vec3  WorldToLightNDC(const Vec3& worldPos) const;

private:
    int   m_width, m_height;
    std::vector<float> m_depth;
    Mat4  m_lightVP;
};
