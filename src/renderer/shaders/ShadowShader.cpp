#include "renderer/shaders/ShadowShader.h"
#include "resource/ObjLoader.h"

VertexOut ShadowShader::Vertex(int idx) {
    VertexOut out;
    out.clipPos = lightMVP * Vec4(mesh->vertices[idx].pos, 1.0f);
    return out;
}

Color ShadowShader::Fragment(const Varying&) {
    return Color(0, 0, 0);  // shadow pass는 색상 불필요
}
