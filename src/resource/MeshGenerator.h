#pragma once

#include "resource/ObjLoader.h"

class MeshGenerator {
public:
    static Mesh CreateGrid(int size, float cellSize);
};
