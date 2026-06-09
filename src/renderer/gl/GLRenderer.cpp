// OpenGL 기반 그림자 패스와 불투명 렌더 패스를 구현합니다.
#include "renderer/gl/GLRenderer.h"
#include "core/Path.h"
#include "math/MathUtils.h"
#include "renderer/Material.h"
#include "renderer/MeshRenderer.h"
#include "resource/Texture.h"
#include "scene/Camera.h"
#include "scene/Light.h"
#include "scene/Scene.h"
#include "scene/Transform.h"
#include "platform/IWindow.h"
#include <iostream>

GLRenderer::GLRenderer(int width, int height)
    : m_width(width)
    , m_height(height)
    , m_lightVP(Mat4::Identity()) {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
}

void GLRenderer::InitShadowMap() {
    if (m_shadowFBO == 0) glGenFramebuffers(1, &m_shadowFBO);
    if (m_shadowTexture == 0) glGenTextures(1, &m_shadowTexture);

    glBindTexture(GL_TEXTURE_2D, m_shadowTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24,
                 m_shadowSize, m_shadowSize, 0,
                 GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    const float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    glBindFramebuffer(GL_FRAMEBUFFER, m_shadowFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_shadowTexture, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "섀도우 프레임버퍼 생성 실패\n";
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void GLRenderer::InitShaders() {
    const std::string shaderDir = Path::GetExecutableDir() + "/assets/shaders/";
    m_phongShader.Load(shaderDir + "phong.vert", shaderDir + "phong.frag");
    m_shadowShader.Load(shaderDir + "shadow.vert", shaderDir + "shadow.frag");
}

GLMesh& GLRenderer::GetOrUploadMesh(const Mesh* mesh) {
    auto it = m_glMeshCache.find(mesh);
    if (it != m_glMeshCache.end()) return it->second;

    auto [inserted, wasInserted] = m_glMeshCache.try_emplace(mesh);
    (void)wasInserted;
    inserted->second.Upload(*mesh);
    return inserted->second;
}

GLuint GLRenderer::GetOrUploadTexture(const Texture* texture) {
    if (!texture || !texture->IsValid()) return 0;

    auto it = m_glTexCache.find(texture);
    if (it != m_glTexCache.end()) return it->second;

    GLuint handle = 0;
    glGenTextures(1, &handle);
    glBindTexture(GL_TEXTURE_2D, handle);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8,
                 texture->Width(), texture->Height(), 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, texture->Pixels());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);

    m_glTexCache.emplace(texture, handle);
    return handle;
}

void GLRenderer::ShadowPass(Registry& reg, const Light& light) {

    m_lightVP = Mat4::Perspective(DegToRad(90.0f), 1.0f, 0.1f, 20.0f)
              * Mat4::LookAt(light.position, { 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f });

    glViewport(0, 0, m_shadowSize, m_shadowSize);
    glBindFramebuffer(GL_FRAMEBUFFER, m_shadowFBO);
    glClear(GL_DEPTH_BUFFER_BIT);

    m_shadowShader.Use();
    for (auto [transform, meshRenderer] : reg.view<Transform, MeshRenderer>()) {
        if (!meshRenderer.visible || !meshRenderer.mesh) continue;

        const Mat4 model = transform.GetWorldMatrix(reg);
        m_shadowShader.SetMat4("uLightMVP", m_lightVP * model);
        GetOrUploadMesh(meshRenderer.mesh).Draw();
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void GLRenderer::OpaquePass(Registry& reg, const Camera& camera, const Light& light) {

    glViewport(0, 0, m_width, m_height);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(0.08f, 0.08f, 0.08f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_phongShader.Use();
    m_phongShader.SetMat4("uView", camera.GetView());
    m_phongShader.SetMat4("uProjection", camera.GetProjection());
    m_phongShader.SetVec3("uCameraPos", camera.eye);
    m_phongShader.SetVec3("uLightPos", light.position);
    m_phongShader.SetVec3("uLightColor", light.color);
    m_phongShader.SetFloat("uAmbient", light.ambient);
    m_phongShader.SetFloat("uDiffuse", light.diffuse);
    m_phongShader.SetFloat("uSpecular", light.specular);
    m_phongShader.SetInt("uAlbedo", 0);
    m_phongShader.SetInt("NormalMap", 1);
    m_phongShader.SetInt("ShadowMap", 2);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, m_shadowTexture);

    for (auto [transform, meshRenderer] : reg.view<Transform, MeshRenderer>()) {
        if (!meshRenderer.visible || !meshRenderer.mesh) continue;

        const Mat4 model = transform.GetWorldMatrix(reg);
        const Material& material = meshRenderer.material;

        m_phongShader.SetMat4("uModel", model);
        m_phongShader.SetMat4("uLightMVP", m_lightVP * model);
        m_phongShader.SetVec3("uTint", material.tint);
        m_phongShader.SetFloat("uShininess", material.shininess);

        GLuint albedoTex = GetOrUploadTexture(material.albedo);
        m_phongShader.SetInt("uHasAlbedo", albedoTex != 0 ? 1 : 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, albedoTex != 0 ? albedoTex : m_whiteTex);

        GLuint normalTex = GetOrUploadTexture(material.normalMap);
        m_phongShader.SetInt("uHasNormalMap", normalTex != 0 ? 1 : 0);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, normalTex != 0 ? normalTex : m_flatNormalTex);

        GetOrUploadMesh(meshRenderer.mesh).Draw();
    }

    glActiveTexture(GL_TEXTURE0);
}

void GLRenderer::Render(Scene& scene, IWindow& window) {
    Render(scene.GetRegistry(), scene.GetActiveCamera(), scene.GetActiveLight(), window);
}

void GLRenderer::Render(Registry& reg, const Camera& camera, const Light& light, IWindow& window) {

    m_width  = window.PixelWidth();
    m_height = window.PixelHeight();

    if (!m_initialized) {

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        InitShadowMap();
        InitShaders();

        uint8_t white[4] = {255, 255, 255, 255};
        glGenTextures(1, &m_whiteTex);
        glBindTexture(GL_TEXTURE_2D, m_whiteTex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, white);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        uint8_t flatN[4] = {128, 128, 255, 255};
        glGenTextures(1, &m_flatNormalTex);
        glBindTexture(GL_TEXTURE_2D, m_flatNormalTex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, flatN);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        m_initialized = true;
    }

    ShadowPass(reg, light);
    OpaquePass(reg, camera, light);
}
