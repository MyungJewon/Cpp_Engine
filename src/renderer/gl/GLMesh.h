// OpenGL 메시 버퍼와 드로우 래퍼를 선언합니다.
#pragma once
#ifdef __APPLE__
  #include <OpenGL/gl3.h>
#else
  #include <glad/glad.h>
#endif
#include "resource/ObjLoader.h"

class GLMesh {
public:
    GLMesh() = default;
    ~GLMesh();

    void Upload(const Mesh& mesh);
    void Draw() const;

    bool IsValid() const { return vao != 0 && indexCount > 0; }

private:
    GLuint vao = 0;
    GLuint vbo = 0;
    GLuint ebo = 0;
    GLsizei indexCount = 0;
};
