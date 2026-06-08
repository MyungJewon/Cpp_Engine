// 그림자 패스용 정점 변환 셰이더를 구현합니다.
#include "renderer/shaders/ShadowShader.h"
#include "resource/ObjLoader.h"

VertexOut ShadowShader::Vertex(int idx) {
    VertexOut out;
    out.clipPos = lightMVP * Vec4(mesh->vertices[idx].pos, 1.0f);
    return out;
}

Color ShadowShader::Fragment(const Varying&) {
    return Color(0, 0, 0);
}
