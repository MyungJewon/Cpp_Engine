#pragma once
#import <OpenGL/gl3.h>
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
