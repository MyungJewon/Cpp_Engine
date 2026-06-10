// TGA 텍스처 로드와 픽셀 샘플링을 구현합니다.
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "resource/Texture.h"
#include <fstream>
#include <cstring>
#include <algorithm>
#include <cctype>

#pragma pack(push, 1)
struct TGAHeader {
    uint8_t  idLength;
    uint8_t  colorMapType;
    uint8_t  imageType;
    uint8_t  colorMapSpec[5];
    uint16_t originX;
    uint16_t originY;
    uint16_t width;
    uint16_t height;
    uint8_t  bitsPerPixel;
    uint8_t  imageDesc;
};
#pragma pack(pop)

Texture Texture::Load(const std::string& path) {
    std::string ext;
    size_t dot = path.find_last_of('.');
    if (dot != std::string::npos) {
        ext = path.substr(dot);
        for (auto& c : ext) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }

    if (ext != ".tga") {
        int w = 0;
        int h = 0;
        int channels = 0;
        stbi_set_flip_vertically_on_load(true);
        unsigned char* data = stbi_load(path.c_str(), &w, &h, &channels, 4);
        if (!data) return Texture{};

        Texture tex;
        tex.m_width = w;
        tex.m_height = h;
        tex.m_pixels.resize(static_cast<size_t>(w) * static_cast<size_t>(h));
        for (int i = 0; i < w * h; ++i) {
            tex.m_pixels[static_cast<size_t>(i)] = Color(
                data[i * 4 + 0],
                data[i * 4 + 1],
                data[i * 4 + 2],
                data[i * 4 + 3]
            );
        }

        stbi_image_free(data);
        return tex;
    }

    Texture tex;
    std::ifstream f(path, std::ios::binary);
    if (!f.is_open()) return tex;

    TGAHeader hdr;
    f.read(reinterpret_cast<char*>(&hdr), sizeof(hdr));
    if (!f) return tex;

    f.seekg(hdr.idLength, std::ios::cur);

    if (hdr.imageType != 2 && hdr.imageType != 3) return tex;
    if (hdr.bitsPerPixel != 24 && hdr.bitsPerPixel != 32) return tex;

    int w = hdr.width, h = hdr.height;
    int bytesPerPixel = hdr.bitsPerPixel / 8;
    tex.m_width  = w;
    tex.m_height = h;
    tex.m_pixels.resize(w * h);

    std::vector<uint8_t> raw(w * h * bytesPerPixel);
    f.read(reinterpret_cast<char*>(raw.data()), raw.size());

    bool topDown = (hdr.imageDesc & 0x20) != 0;

    for (int y = 0; y < h; ++y) {
        int srcRow = topDown ? y : (h - 1 - y);
        for (int x = 0; x < w; ++x) {
            int     idx = (srcRow * w + x) * bytesPerPixel;
            uint8_t b   = raw[idx + 0];
            uint8_t g   = raw[idx + 1];
            uint8_t r   = raw[idx + 2];
            uint8_t a   = (bytesPerPixel == 4) ? raw[idx + 3] : 255;
            tex.m_pixels[y * w + x] = Color(r, g, b, a);
        }
    }

    return tex;
}

Texture Texture::FromPixels(int w, int h, const std::vector<Color>& pixels) {
    Texture tex;
    tex.m_width  = w;
    tex.m_height = h;
    tex.m_pixels = pixels;
    return tex;
}

Color Texture::Sample(float u, float v) const {
    if (m_pixels.empty()) return Color(255, 0, 255);

    u = u - std::floor(u);
    v = v - std::floor(v);

    float fx = u * (m_width  - 1);
    float fy = v * (m_height - 1);
    int   x0 = (int)fx, y0 = (int)fy;
    int   x1 = std::min(x0 + 1, m_width  - 1);
    int   y1 = std::min(y0 + 1, m_height - 1);
    float tx = fx - x0, ty = fy - y0;

    auto get = [&](int x, int y) -> Color {
        return m_pixels[y * m_width + x];
    };

    auto lerp = [](float a, float b, float t) { return a + (b - a) * t; };
    auto lerpC = [&](Color a, Color b, float t) -> Color {
        return Color(
            (uint8_t)lerp(a.r, b.r, t),
            (uint8_t)lerp(a.g, b.g, t),
            (uint8_t)lerp(a.b, b.b, t)
        );
    };

    Color c00 = get(x0, y0), c10 = get(x1, y0);
    Color c01 = get(x0, y1), c11 = get(x1, y1);
    return lerpC(lerpC(c00, c10, tx), lerpC(c01, c11, tx), ty);
}
