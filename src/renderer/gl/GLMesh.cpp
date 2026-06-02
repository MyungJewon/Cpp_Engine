#include "renderer/gl/GLMesh.h"
#include <cstddef>

void GLMesh::Upload(const Mesh& mesh) {
    if (vao == 0) glGenVertexArrays(1, &vao);
    if (vbo == 0) glGenBuffers(1, &vbo);
    if (ebo == 0) glGenBuffers(1, &ebo);

    indexCount = static_cast<GLsizei>(mesh.indices.size());

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(mesh.vertices.size() * sizeof(MeshVertex)),
                 mesh.vertices.data(),
                 GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(mesh.indices.size() * sizeof(int)),
                 mesh.indices.data(),
                 GL_STATIC_DRAW);

    // MeshVertex 실제 필드 순서: pos(Vec3), uv(Vec2), normal(Vec3), tangent(Vec3)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(MeshVertex),
                          reinterpret_cast<void*>(offsetof(MeshVertex, pos)));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(MeshVertex),
                          reinterpret_cast<void*>(offsetof(MeshVertex, uv)));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(MeshVertex),
                          reinterpret_cast<void*>(offsetof(MeshVertex, normal)));

    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(MeshVertex),
                          reinterpret_cast<void*>(offsetof(MeshVertex, tangent)));

    glBindVertexArray(0);
}

void GLMesh::Draw() const {
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}

GLMesh::~GLMesh() {
    if (vao != 0) glDeleteVertexArrays(1, &vao);
    if (vbo != 0) glDeleteBuffers(1, &vbo);
    if (ebo != 0) glDeleteBuffers(1, &ebo);
}
