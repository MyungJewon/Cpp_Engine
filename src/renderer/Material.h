#pragma once

#include "math/Vec3.h"
#include "resource/Texture.h"

// 셰이더에 전달할 기본 재질 정보
struct Material {
    const Texture* albedo = nullptr;
    const Texture* normalMap = nullptr;
    float shininess = 32.0f;
    Vec3 tint = { 1.0f, 1.0f, 1.0f };
};
