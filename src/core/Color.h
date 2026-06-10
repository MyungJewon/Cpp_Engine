// RGBA 8비트 색상 구조체를 정의합니다.
#pragma once
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
