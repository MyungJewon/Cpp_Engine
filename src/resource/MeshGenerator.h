#pragma once

#include "resource/ObjLoader.h"

class MeshGenerator {
public:
    static Mesh CreateGrid(int size, float cellSize);
    static Mesh CreateSphere(int stacks = 16, int slices = 16, float radius = 1.0f);
};
