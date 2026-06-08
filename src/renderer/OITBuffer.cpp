// 픽셀별 투명 프래그먼트 저장과 합성을 구현합니다.
#include "renderer/OITBuffer.h"
#include <algorithm>

OITBuffer::OITBuffer(int width, int height)
    : m_width(width), m_height(height)
    , m_pixels(width * height)
{}

void OITBuffer::Clear() {
    for (auto& p : m_pixels) p.count = 0;
}

void OITBuffer::AddFragment(int x, int y, float depth, Color color, float alpha) {
    if (x < 0 || x >= m_width || y < 0 || y >= m_height) return;
    auto& px = m_pixels[y * m_width + x];
    if (px.count >= OIT_MAX_FRAGS) return;
    px.frags[px.count++] = { depth, color, alpha };
}

void OITBuffer::Compose(Framebuffer& fb) {
    for (int y = 0; y < m_height; ++y) {
        for (int x = 0; x < m_width; ++x) {
            auto& px = m_pixels[y * m_width + x];
            if (px.count == 0) continue;

            std::sort(px.frags, px.frags + px.count,
                [](const OITFragment& a, const OITFragment& b) {
                    return a.depth > b.depth;
                });

            uint32_t base = fb.ColorData()[y * fb.Width() + x];
            float r = ((base >> 16) & 0xFF) / 255.0f;
            float g = ((base >>  8) & 0xFF) / 255.0f;
            float b = ((base      ) & 0xFF) / 255.0f;

            for (int i = 0; i < px.count; ++i) {
                float a  = px.frags[i].alpha;
                float sr = px.frags[i].color.r / 255.0f;
                float sg = px.frags[i].color.g / 255.0f;
                float sb = px.frags[i].color.b / 255.0f;
                r = sr * a + r * (1.0f - a);
                g = sg * a + g * (1.0f - a);
                b = sb * a + b * (1.0f - a);
            }

            fb.SetPixel(x, y, Color::FromFloat(r, g, b));
        }
    }
}
