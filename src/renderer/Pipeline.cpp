// 인덱스 메시를 셰이더와 래스터라이저로 전달하는 드로우 경로를 구현합니다.
#include "renderer/Pipeline.h"

void Pipeline::DrawIndexed(IShader& shader, const std::vector<int>& indices) {
    for (size_t i = 0; i + 2 < indices.size(); i += 3) {
        VertexOut v0 = shader.Vertex(indices[i    ]);
        VertexOut v1 = shader.Vertex(indices[i + 1]);
        VertexOut v2 = shader.Vertex(indices[i + 2]);
        m_rasterizer.DrawTriangle(v0, v1, v2, shader);
    }
}
