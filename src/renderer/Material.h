// 렌더링 셰이더에 전달되는 재질 속성을 정의합니다.
#pragma once

#include "math/Vec3.h"
#include "resource/Texture.h"

struct Material {
    const Texture* albedo = nullptr;
    const Texture* normalMap = nullptr;
    float shininess = 32.0f;
    Vec3 tint = { 1.0f, 1.0f, 1.0f };
};
