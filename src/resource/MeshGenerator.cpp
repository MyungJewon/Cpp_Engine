#include "resource/MeshGenerator.h"
#include <cmath>

namespace {
constexpr float PI = 3.14159265358979323846f;
}

Mesh MeshGenerator::CreateGrid(int size, float cellSize) {
    Mesh mesh;
    if (size <= 0) return mesh;

    const int vertexCountPerSide = size + 1;
    const float halfW = size * cellSize * 0.5f;

    mesh.vertices.reserve(vertexCountPerSide * vertexCountPerSide);
    mesh.indices.reserve(size * size * 6);

    // 격자 정점을 중앙 원점 기준으로 XZ 평면에 배치한다.
    for (int z = 0; z <= size; ++z) {
        for (int x = 0; x <= size; ++x) {
            MeshVertex vertex;
            vertex.pos = {
                x * cellSize - halfW,
                0.0f,
                z * cellSize - halfW
            };
            vertex.normal = { 0.0f, 1.0f, 0.0f };
            vertex.uv = {
                x / static_cast<float>(size),
                z / static_cast<float>(size)
            };
            vertex.tangent = { 1.0f, 0.0f, 0.0f };
            mesh.vertices.push_back(vertex);
        }
    }

    // 각 셀을 두 개의 삼각형으로 나눠 인덱스를 만든다.
    for (int z = 0; z < size; ++z) {
        for (int x = 0; x < size; ++x) {
            const int i0 = z * vertexCountPerSide + x;
            const int i1 = i0 + 1;
            const int i2 = i0 + vertexCountPerSide;
            const int i3 = i2 + 1;

            mesh.indices.insert(mesh.indices.end(), {
                i0, i2, i1,
                i1, i2, i3
            });
        }
    }

    return mesh;
}

Mesh MeshGenerator::CreateSphere(int stacks, int slices, float radius) {
    Mesh mesh;
    if (stacks <= 0 || slices <= 0 || radius <= 0.0f) return mesh;

    mesh.vertices.reserve((stacks + 1) * (slices + 1));
    mesh.indices.reserve(stacks * slices * 6);

    for (int i = 0; i <= stacks; ++i) {
        const float phi = PI * i / static_cast<float>(stacks);
        for (int j = 0; j <= slices; ++j) {
            const float theta = 2.0f * PI * j / static_cast<float>(slices);

            const float sinPhi = std::sinf(phi);
            const float x = radius * sinPhi * std::cosf(theta);
            const float y = radius * std::cosf(phi);
            const float z = radius * sinPhi * std::sinf(theta);

            const Vec3 pos = { x, y, z };
            const Vec3 normal = { x / radius, y / radius, z / radius };
            const Vec2 uv = {
                j / static_cast<float>(slices),
                i / static_cast<float>(stacks)
            };
            const Vec3 tangent = { -std::sinf(theta), 0.0f, std::cosf(theta) };

            mesh.vertices.push_back({ pos, uv, normal, tangent });
        }
    }

    for (int i = 0; i < stacks; ++i) {
        for (int j = 0; j < slices; ++j) {
            const int a = i * (slices + 1) + j;
            const int b = a + 1;
            const int c = a + (slices + 1);
            const int d = c + 1;

            mesh.indices.insert(mesh.indices.end(), {
                a, c, b,
                b, c, d
            });
        }
    }

    return mesh;
}
