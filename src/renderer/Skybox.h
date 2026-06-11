#pragma once
#include <string>
#ifdef __APPLE__
  #include <OpenGL/gl3.h>
#else
  #include <glad/glad.h>
#endif

class Camera;

class Skybox {
public:
    bool Load(const std::string& imagePath);
    void Render(const Camera& camera);
    void Shutdown();
    bool IsLoaded() const { return m_texture != 0; }

private:
    bool InitGL();
    GLuint m_texture = 0;
    GLuint m_vao = 0;
    GLuint m_vbo = 0;
    GLuint m_shader = 0;
};
