#pragma once

#include <OpenGL/gl3.h>
#include "math/Mat4.h"
#include "renderer/gl/GLMesh.h"
#include "renderer/gl/GLShader.h"
#include <unordered_map>

class Camera;
class IWindow;
class Light;
class Mesh;
class Registry;
class Scene;
class Texture;

class GLRenderer {
public:
    GLRenderer(int width, int height);

    void Render(Scene& scene, IWindow& window);
    void Render(Registry& reg, const Camera& camera, const Light& light, IWindow& window);

private:
    void InitShadowMap();
    void InitShaders();
    GLMesh& GetOrUploadMesh(const Mesh* mesh);
    GLuint GetOrUploadTexture(const Texture* texture);
    void ShadowPass(Registry& reg, const Light& light);
    void OpaquePass(Registry& reg, const Camera& camera, const Light& light);

    int m_width;
    int m_height;
    GLShader m_phongShader;
    GLShader m_shadowShader;
    std::unordered_map<const Mesh*, GLMesh> m_glMeshCache;
    std::unordered_map<const Texture*, GLuint> m_glTexCache;
    GLuint m_shadowFBO = 0;
    GLuint m_shadowTexture = 0;
    int m_shadowSize = 1024;
    Mat4 m_lightVP;
    bool   m_initialized  = false;
    GLuint m_whiteTex     = 0;   // albedo/normalMap 없을 때 바인딩할 더미 흰색 텍스처
    GLuint m_flatNormalTex= 0;   // normalMap 없을 때 바인딩할 평면 법선 텍스처 (0.5,0.5,1)
};
