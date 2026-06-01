#pragma once
#include "renderer/Framebuffer.h"
#include "renderer/Rasterizer.h"
#include "renderer/Shader.h"
#include <vector>

class Pipeline {
public:
    Pipeline(Framebuffer& fb) : m_rasterizer(fb) {}

    void DrawIndexed(IShader& shader, const std::vector<int>& indices); // 인덱스 3개씩 묶어 삼각형 드로우

private:
    Rasterizer m_rasterizer;
};
