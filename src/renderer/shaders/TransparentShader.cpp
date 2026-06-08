// 투명 패스에서 위치 변환과 단색 출력을 구현합니다.
#include "renderer/shaders/TransparentShader.h"
#include "resource/ObjLoader.h"

VertexOut TransparentShader::Vertex(int idx) {
    VertexOut out;
    out.clipPos = mvp * Vec4(mesh->vertices[idx].pos, 1.0f);
    return out;
}

Color TransparentShader::Fragment(const Varying&) {
    return color;
}
