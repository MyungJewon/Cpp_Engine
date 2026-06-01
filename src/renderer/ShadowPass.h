#pragma once
#include "renderer/ShadowMap.h"
#include "renderer/shaders/ShadowShader.h"
#include "resource/ObjLoader.h"

// 깊이 버퍼만 채우는 패스
class ShadowPassRenderer {
public:
    explicit ShadowPassRenderer(ShadowMap& sm) : m_sm(sm) {}

    void Render(ShadowShader& shader, const std::vector<int>& indices);

private:
    ShadowMap& m_sm;

    Vec3 Barycentric(Vec2 A, Vec2 B, Vec2 C, Vec2 P);
};
