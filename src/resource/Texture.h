// 이미지 픽셀 데이터를 보관하고 샘플링하는 텍스처 클래스를 선언합니다.
#pragma once
#include "renderer/Framebuffer.h"
#include "math/Vec2.h"
#include <vector>
#include <string>

class Texture {
public:

    static Texture Load(const std::string& path);
    static Texture FromPixels(int w, int h, const std::vector<Color>& pixels);

    bool IsValid() const { return !m_pixels.empty(); }
    int Width() const { return m_width; }
    int Height() const { return m_height; }
    const Color* Pixels() const { return m_pixels.data(); }

    Color Sample(float u, float v) const;

private:
    std::vector<Color> m_pixels;
    int m_width  = 0;
    int m_height = 0;
};
