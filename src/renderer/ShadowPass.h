// 색상 없이 깊이만 기록하는 그림자 패스 렌더러를 선언합니다.
#pragma once
#include "renderer/ShadowMap.h"
#include "renderer/shaders/ShadowShader.h"
#include "resource/ObjLoader.h"

class ShadowPassRenderer {
public:
    explicit ShadowPassRenderer(ShadowMap& sm) : m_sm(sm) {}

    void Render(ShadowShader& shader, const std::vector<int>& indices);

private:
    ShadowMap& m_sm;

    Vec3 Barycentric(Vec2 A, Vec2 B, Vec2 C, Vec2 P);
};
