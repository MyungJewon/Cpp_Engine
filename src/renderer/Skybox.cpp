#include "renderer/Skybox.h"
#include "core/Path.h"
#include "math/Mat4.h"
#include "scene/Camera.h"
#include "stb_image.h"
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace {

std::string ReadFile(const std::string& path) {
    std::ifstream file(path);
    if (!file) {
        std::cerr << "Skybox shader file open failed: " << path << "\n";
        return {};
    }

    return std::string((std::istreambuf_iterator<char>(file)),
                       std::istreambuf_iterator<char>());
}

GLuint CompileShader(GLenum type, const std::string& source, const std::string& label) {
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
        std::cerr << "Skybox shader compile failed: " << label << "\n" << log.data() << "\n";
        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

GLuint LinkProgram(GLuint vertexShader, GLuint fragmentShader) {
    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    GLint success = GL_FALSE;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (success != GL_TRUE) {
        GLint logLength = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
        std::vector<char> log(static_cast<size_t>(logLength) + 1);
        glGetProgramInfoLog(program, logLength, nullptr, log.data());
        std::cerr << "Skybox shader link failed\n" << log.data() << "\n";
        glDeleteProgram(program);
        return 0;
    }

    return program;
}

} // namespace

bool Skybox::Load(const std::string& imagePath) {
    int width = 0;
    int height = 0;
    int channels = 0;
    stbi_set_flip_vertically_on_load(false);
    unsigned char* data = stbi_load(imagePath.c_str(), &width, &height, &channels, 4);
    if (!data) {
        std::cerr << "Skybox image load failed: " << imagePath << "\n";
        return false;
    }

    if (m_texture == 0) glGenTextures(1, &m_texture);
    glBindTexture(GL_TEXTURE_2D, m_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);

    stbi_image_free(data);
    return InitGL();
}

bool Skybox::InitGL() {
    if (m_vao == 0) glGenVertexArrays(1, &m_vao);
    if (m_vbo == 0) glGenBuffers(1, &m_vbo);

    float vertices[] = {
        -1,  1, -1,  -1, -1, -1,   1, -1, -1,   1, -1, -1,   1,  1, -1,  -1,  1, -1,
        -1, -1,  1,  -1, -1, -1,  -1,  1, -1,  -1,  1, -1,  -1,  1,  1,  -1, -1,  1,
         1, -1, -1,   1, -1,  1,   1,  1,  1,   1,  1,  1,   1,  1, -1,   1, -1, -1,
        -1, -1,  1,  -1,  1,  1,   1,  1,  1,   1,  1,  1,   1, -1,  1,  -1, -1,  1,
        -1,  1, -1,   1,  1, -1,   1,  1,  1,   1,  1,  1,  -1,  1,  1,  -1,  1, -1,
        -1, -1, -1,  -1, -1,  1,   1, -1, -1,   1, -1, -1,  -1, -1,  1,   1, -1,  1
    };

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    if (m_shader == 0) {
        const std::string shaderDir = Path::GetExecutableDir() + "/assets/shaders/";
        const std::string vertexPath = shaderDir + "skybox.vert";
        const std::string fragmentPath = shaderDir + "skybox.frag";
        const std::string vertexSource = ReadFile(vertexPath);
        const std::string fragmentSource = ReadFile(fragmentPath);
        if (vertexSource.empty() || fragmentSource.empty()) return false;

        GLuint vertexShader = CompileShader(GL_VERTEX_SHADER, vertexSource, vertexPath);
        if (vertexShader == 0) return false;

        GLuint fragmentShader = CompileShader(GL_FRAGMENT_SHADER, fragmentSource, fragmentPath);
        if (fragmentShader == 0) {
            glDeleteShader(vertexShader);
            return false;
        }

        m_shader = LinkProgram(vertexShader, fragmentShader);
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        if (m_shader == 0) return false;
    }

    return true;
}

void Skybox::Render(const Camera& camera) {
    if (!IsLoaded() || m_shader == 0 || m_vao == 0) return;

    Mat4 view = camera.GetView();
    view.m[0][3] = 0.0f;
    view.m[1][3] = 0.0f;
    view.m[2][3] = 0.0f;

    glDepthFunc(GL_LEQUAL);
    glDepthMask(GL_FALSE);
    glDisable(GL_CULL_FACE);

    glUseProgram(m_shader);
    glUniformMatrix4fv(glGetUniformLocation(m_shader, "uView"), 1, GL_TRUE, &view.m[0][0]);
    Mat4 projection = camera.GetProjection();
    glUniformMatrix4fv(glGetUniformLocation(m_shader, "uProjection"), 1, GL_TRUE, &projection.m[0][0]);
    glUniform1i(glGetUniformLocation(m_shader, "uSkybox"), 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_texture);
    glBindVertexArray(m_vao);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(0);

    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);
}

void Skybox::Shutdown() {
    if (m_texture != 0) {
        glDeleteTextures(1, &m_texture);
        m_texture = 0;
    }
    if (m_vbo != 0) {
        glDeleteBuffers(1, &m_vbo);
        m_vbo = 0;
    }
    if (m_vao != 0) {
        glDeleteVertexArrays(1, &m_vao);
        m_vao = 0;
    }
    if (m_shader != 0) {
        glDeleteProgram(m_shader);
        m_shader = 0;
    }
}
