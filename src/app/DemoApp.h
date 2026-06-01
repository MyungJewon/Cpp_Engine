#pragma once

#include "app/Application.h"
#include "renderer/OITBuffer.h"
#include "renderer/MeshRenderer.h"
#include "renderer/Pipeline.h"
#include "renderer/ShadowMap.h"
#include "renderer/shaders/PhongShader.h"
#include "renderer/shaders/ShadowShader.h"
#include "resource/AssetManager.h"
#include "resource/ObjLoader.h"
#include "resource/Texture.h"
#include "scene/Camera.h"
#include "scene/Light.h"
#include <memory>

// OBJ 모델을 PhongShader로 렌더링하는 데모 애플리케이션
class DemoApp : public Application {
public:
    DemoApp(int width, int height, const char* title);

protected:
    void OnInit() override;
    void OnUpdate(float dt) override;
    void OnRender() override;

private:
    // Pipeline은 Application의 Framebuffer 참조가 필요하므로 OnInit에서 지연 생성한다.
    std::unique_ptr<Pipeline> m_pipeline;
    ShadowMap m_shadowMap;
    OITBuffer m_oitBuffer;
    const Mesh* m_mesh = nullptr;
    const Texture* m_albedo = nullptr;
    const Texture* m_normalMap = nullptr;
    Mesh m_fallbackMesh;
    Camera m_camera;
    Light m_light;
    PhongShader m_phongShader;
    ShadowShader m_shadowShader;
    float m_angle = 0.0f;
};
