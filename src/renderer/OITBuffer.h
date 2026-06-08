// 순서 독립 투명도 처리를 위한 프래그먼트 버퍼를 선언합니다.
#pragma once
#include "renderer/Framebuffer.h"
#include <vector>

static constexpr int OIT_MAX_FRAGS = 8;

struct OITFragment {
    float  depth;
    Color  color;
    float  alpha;
};

class OITBuffer {
public:
    OITBuffer(int width, int height);

    void Clear();
    void AddFragment(int x, int y, float depth, Color color, float alpha);
    void Compose(Framebuffer& fb);

private:
    struct Pixel {
        OITFragment frags[OIT_MAX_FRAGS];
        int count = 0;
    };

    int              m_width, m_height;
    std::vector<Pixel> m_pixels;
};
