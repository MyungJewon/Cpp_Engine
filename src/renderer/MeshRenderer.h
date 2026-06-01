#pragma once

#include "renderer/Material.h"

struct Mesh;

// Mesh와 Material을 함께 보관하는 렌더 컴포넌트
struct MeshRenderer {
    const Mesh* mesh = nullptr;
    Material material;
    bool visible = true;
};
