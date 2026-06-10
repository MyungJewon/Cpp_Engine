// OpenGL-only 2D screen-space UI renderer.
#pragma once

#include "math/Vec3.h"
#include <OpenGL/gl3.h>
#include <string>

class UIRenderer {
public:
    bool Init(int width, int height);
    void Shutdown();

    void BeginFrame(int width, int height);
    void DrawRect(float x, float y, float width, float height, const Vec3& color, float alpha);
    void DrawText(float x, float y, const std::string& text, int fontSize, const Vec3& color, float alpha);
    void EndFrame();

private:
    bool CompileShaders();
    bool CreateQuad();
    bool CreateFontTexture();
    void CacheUniforms();

    GLuint m_shader = 0;
    GLuint m_vao = 0;
    GLuint m_vbo = 0;
    GLuint m_fontTexture = 0;
    int m_width = 0;
    int m_height = 0;

    GLint m_uProj      = -1;
    GLint m_uOffset    = -1;
    GLint m_uSize      = -1;
    GLint m_uTexOffset = -1;
    GLint m_uTexSize   = -1;
    GLint m_uMode      = -1;
    GLint m_uColor     = -1;
    GLint m_uAlpha     = -1;
    GLint m_uFont      = -1;
};
