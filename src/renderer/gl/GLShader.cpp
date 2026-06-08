// OpenGL 셰이더 파일 로드와 컴파일 및 uniform 설정을 구현합니다.
#include "renderer/gl/GLShader.h"
#include <fstream>
#include <iostream>
#include <vector>

std::string GLShader::ReadFile(const std::string& path) const {
    std::ifstream file(path);
    if (!file) {
        std::cerr << "셰이더 파일을 열 수 없습니다: " << path << "\n";
        return {};
    }

    return std::string((std::istreambuf_iterator<char>(file)),
                       std::istreambuf_iterator<char>());
}

GLuint GLShader::CompileShader(GLenum type, const std::string& source, const std::string& path) const {
    GLuint shader = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    GLint success = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (success != GL_TRUE) {
        GLint logLength = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
        std::vector<char> log(static_cast<size_t>(logLength) + 1);
        glGetShaderInfoLog(shader, logLength, nullptr, log.data());
        std::cerr << "셰이더 컴파일 실패: " << path << "\n" << log.data() << "\n";
        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

bool GLShader::Load(const std::string& vertexPath, const std::string& fragmentPath) {
    std::string vertexSource = ReadFile(vertexPath);
    std::string fragmentSource = ReadFile(fragmentPath);
    if (vertexSource.empty() || fragmentSource.empty()) return false;

    GLuint vertexShader = CompileShader(GL_VERTEX_SHADER, vertexSource, vertexPath);
    if (vertexShader == 0) return false;

    GLuint fragmentShader = CompileShader(GL_FRAGMENT_SHADER, fragmentSource, fragmentPath);
    if (fragmentShader == 0) {
        glDeleteShader(vertexShader);
        return false;
    }

    GLuint newProgram = glCreateProgram();
    glAttachShader(newProgram, vertexShader);
    glAttachShader(newProgram, fragmentShader);
    glLinkProgram(newProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    GLint success = GL_FALSE;
    glGetProgramiv(newProgram, GL_LINK_STATUS, &success);
    if (success != GL_TRUE) {
        GLint logLength = 0;
        glGetProgramiv(newProgram, GL_INFO_LOG_LENGTH, &logLength);
        std::vector<char> log(static_cast<size_t>(logLength) + 1);
        glGetProgramInfoLog(newProgram, logLength, nullptr, log.data());
        std::cerr << "셰이더 프로그램 링크 실패\n" << log.data() << "\n";
        glDeleteProgram(newProgram);
        return false;
    }

    if (program != 0) glDeleteProgram(program);
    program = newProgram;
    return true;
}

GLShader::~GLShader() {
    if (program != 0) {
        glDeleteProgram(program);
        program = 0;
    }
}

void GLShader::Use() const {
    glUseProgram(program);
}

void GLShader::SetMat4(const std::string& name, const Mat4& value) const {

    glUniformMatrix4fv(GetUniformLocation(name), 1, GL_TRUE, &value.m[0][0]);
}

void GLShader::SetVec3(const std::string& name, const Vec3& value) const {
    glUniform3f(GetUniformLocation(name), value.x, value.y, value.z);
}

void GLShader::SetFloat(const std::string& name, float value) const {
    glUniform1f(GetUniformLocation(name), value);
}

void GLShader::SetInt(const std::string& name, int value) const {
    glUniform1i(GetUniformLocation(name), value);
}

GLint GLShader::GetUniformLocation(const std::string& name) const {
    return glGetUniformLocation(program, name.c_str());
}
