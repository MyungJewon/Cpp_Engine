// 씬 조명 계산에 사용하는 광원 속성을 정의합니다.
#pragma once
#include "math/Vec3.h"

struct Light {
    Vec3  position  = { 2.0f, 3.0f, 3.0f };
    Vec3  color     = { 1.0f, 1.0f, 1.0f };
    float ambient   = 0.1f;
    float diffuse   = 0.8f;
    float specular  = 0.5f;
    float shininess = 32.0f;
};
