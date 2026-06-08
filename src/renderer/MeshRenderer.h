// Mesh와 Material을 Entity에 연결하는 렌더 컴포넌트를 정의합니다.
#pragma once

#include "renderer/Material.h"

struct Mesh;

struct MeshRenderer {
    const Mesh* mesh = nullptr;
    Material material;
    bool visible = true;
};
