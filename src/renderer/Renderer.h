#pragma once
#include "renderer/Framebuffer.h"
#include "renderer/Pipeline.h"
#include "renderer/ShadowMap.h"
#include "renderer/ShadowPass.h"
#include "renderer/OITBuffer.h"
#include "renderer/shaders/PhongShader.h"
#include "renderer/shaders/ShadowShader.h"
#include "scene/Camera.h"
#include "scene/Light.h"
#include "platform/IWindow.h"
#include "ecs/Registry.hpp"

class Scene;

// 래스터라이저 파이프라인 래퍼
// Registry를 순회하며 MeshRenderer + Transform 컴포넌트를 가진 Entity를 렌더링
class Renderer {
public:
    Renderer(int width, int height);
    void Render(Scene& scene, IWindow& window);
    void Render(Registry& reg, const Camera& camera, const Light& light, IWindow& window);

private:
    Framebuffer m_fb;        // MSAA 4x 프레임버퍼 (Renderer가 소유)
    Pipeline    m_pipeline;
    ShadowMap   m_shadowMap;
    OITBuffer   m_oitBuffer;
};
