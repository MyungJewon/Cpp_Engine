// 셰이더와 래스터라이저를 연결하는 소프트웨어 렌더링 파이프라인을 선언합니다.
#pragma once
#include "renderer/Framebuffer.h"
#include "renderer/Rasterizer.h"
#include "renderer/Shader.h"
#include <vector>

class Pipeline {
public:
    Pipeline(Framebuffer& fb) : m_rasterizer(fb) {}

    void DrawIndexed(IShader& shader, const std::vector<int>& indices);

private:
    Rasterizer m_rasterizer;
};
