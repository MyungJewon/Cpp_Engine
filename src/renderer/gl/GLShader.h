// OpenGL 셰이더 프로그램 관리 기능을 선언합니다.
#pragma once
#ifdef __APPLE__
  #include <OpenGL/gl3.h>
#else
  #include <glad/glad.h>
#endif
#include "math/Mat4.h"
#include "math/Vec3.h"
#include <string>
#include <unordered_map>

class GLShader {
public:
    GLShader() = default;
    ~GLShader();

    bool Load(const std::string& vertexPath, const std::string& fragmentPath);
    void Use() const;

    void SetMat4(const std::string& name, const Mat4& value) const;
    void SetVec3(const std::string& name, const Vec3& value) const;
    void SetFloat(const std::string& name, float value) const;
    void SetInt(const std::string& name, int value) const;

    GLuint Program() const { return program; }
    bool IsValid() const { return program != 0; }

private:
    std::string ReadFile(const std::string& path) const;
    GLuint CompileShader(GLenum type, const std::string& source, const std::string& path) const;
    GLint GetUniformLocation(const std::string& name) const;

    GLuint program = 0;
    mutable std::unordered_map<std::string, GLint> m_uniformCache;
};
