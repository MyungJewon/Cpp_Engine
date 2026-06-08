// OBJ 로딩에 쓰는 Mesh 데이터 구조와 로더를 선언합니다.
#pragma once
#include "math/Vec2.h"
#include "math/Vec3.h"
#include <vector>
#include <string>

struct MeshVertex {
    Vec3 pos;
    Vec2 uv;
    Vec3 normal;
    Vec3 tangent;
};

struct Mesh {
    std::vector<MeshVertex> vertices;
    std::vector<int>        indices;
};

class ObjLoader {
public:

    static Mesh Load(const std::string& path);

    static void Normalize(Mesh& mesh);
};
