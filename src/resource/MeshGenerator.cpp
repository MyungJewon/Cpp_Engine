#include "resource/MeshGenerator.h"

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
