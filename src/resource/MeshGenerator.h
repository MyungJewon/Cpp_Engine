// 절차적으로 기본 Mesh를 생성하는 유틸리티를 선언합니다.
#pragma once

#include "resource/ObjLoader.h"

class MeshGenerator {
public:
    static Mesh CreateGrid(int size, float cellSize);
    static Mesh CreateSphere(int stacks = 16, int slices = 16, float radius = 1.0f);
};
