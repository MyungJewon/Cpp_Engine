// 소프트웨어 렌더링용 색상과 깊이 프레임버퍼를 선언합니다.
#pragma once
#include <vector>
#include <cstdint>

struct Color {
    uint8_t r, g, b, a;

    Color() : r(0), g(0), b(0), a(255) {}
    Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) : r(r), g(g), b(b), a(a) {}

    static Color FromFloat(float r, float g, float b, float a = 1.0f);

    uint32_t ToARGB() const {
        return ((uint32_t)a << 24) | ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
    }
};

inline Color Color::FromFloat(float r, float g, float b, float a) {
    auto c = [](float v) -> uint8_t {
        if (v < 0.0f) v = 0.0f;
        if (v > 1.0f) v = 1.0f;
        return (uint8_t)(v * 255.0f);
    };
    return { c(r), c(g), c(b), c(a) };
}

class Framebuffer {
public:
    Framebuffer(int width, int height, int sampleCount = 1);

    int Width()       const { return m_width;       }
    int Height()      const { return m_height;      }
    int SampleCount() const { return m_sampleCount; }

    void Clear(Color clearColor, float clearDepth = 1.0f);
    void SetPixel(int x, int y, Color color);
    bool TestAndSetDepth(int x, int y, float depth);

    bool TestAndSetMSAA(int x, int y, int sample, float depth, Color color);

    void Resolve();

    const uint32_t* ColorData() const { return m_color.data(); }

private:
    int m_width, m_height, m_sampleCount;
    std::vector<uint32_t> m_color;
    std::vector<float>    m_depth;
    std::vector<float>    m_msaaDepth;
    std::vector<uint32_t> m_msaaColor;
};
