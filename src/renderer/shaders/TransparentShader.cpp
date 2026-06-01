#include "renderer/shaders/TransparentShader.h"
#include "resource/ObjLoader.h"

VertexOut TransparentShader::Vertex(int idx) { // 위치만 변환
    VertexOut out;
    out.clipPos = mvp * Vec4(mesh->vertices[idx].pos, 1.0f);
    return out;
}

Color TransparentShader::Fragment(const Varying&) {
    return color; // 단색 반환 (OIT에서 alpha 적용)
}
