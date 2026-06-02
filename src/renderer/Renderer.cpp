#include "renderer/Renderer.h"
#include "math/MathUtils.h"
#include "renderer/MeshRenderer.h"
#include "scene/Scene.h"
#include "scene/Transform.h"

Renderer::Renderer(int width, int height)
    : m_fb(width, height, 4)
    , m_pipeline(m_fb)
    , m_shadowMap(1024, 1024)
    , m_oitBuffer(width, height) {
}

void Renderer::Render(Scene& scene, IWindow& window) {
    Render(scene.GetRegistry(), scene.GetActiveCamera(), scene.GetActiveLight(), window);
}

void Renderer::Render(Registry& reg, const Camera& camera, const Light& light, IWindow& window) {
    m_fb.Clear(Color(20, 20, 20));
    m_oitBuffer.Clear();

    const Mat4 lightView = Mat4::LookAt(light.position, Vec3(0.0f, 0.0f, 0.0f), Vec3(0.0f, 1.0f, 0.0f));
    const Mat4 lightProj = Mat4::Perspective(DegToRad(90.0f), 1.0f, 0.1f, 20.0f);
    const Mat4 lightVP = lightProj * lightView;

    // Shadow Pass: 광원 시점의 깊이 맵을 먼저 채운다.
    m_shadowMap.Clear();
    m_shadowMap.SetLightVP(lightVP);
    ShadowPassRenderer shadowPass(m_shadowMap);
    for (auto [transform, meshRenderer] : reg.view<Transform, MeshRenderer>()) {
        if (!meshRenderer.visible || !meshRenderer.mesh) continue;

        const Mat4 worldMatrix = transform.GetWorldMatrix(reg);
        ShadowShader shadowShader;
        shadowShader.mesh = meshRenderer.mesh;
        shadowShader.lightMVP = lightVP * worldMatrix;
        shadowPass.Render(shadowShader, meshRenderer.mesh->indices);
    }

    const Mat4 vp = camera.GetProjection() * camera.GetView();

    // Opaque Pass: Phong 셰이더로 불투명 메시를 렌더링한다.
    for (auto [transform, meshRenderer] : reg.view<Transform, MeshRenderer>()) {
        if (!meshRenderer.visible || !meshRenderer.mesh) continue;

        const Mat4 worldMatrix = transform.GetWorldMatrix(reg);
        Light materialLight = light;
        materialLight.shininess = meshRenderer.material.shininess;

        PhongShader phongShader;
        phongShader.mesh = meshRenderer.mesh;
        phongShader.albedo = meshRenderer.material.albedo;
        phongShader.normalMap = meshRenderer.material.normalMap;
        phongShader.shadowMap = &m_shadowMap;
        phongShader.mvp = vp * worldMatrix;
        phongShader.modelMat = worldMatrix;
        phongShader.cameraPos = camera.eye;
        phongShader.tint = meshRenderer.material.tint;
        phongShader.light = materialLight;

        m_pipeline.DrawIndexed(phongShader, meshRenderer.mesh->indices);
    }

    // MSAA 버퍼를 최종 색상 버퍼로 해석한 뒤 투명 프래그먼트를 합성한다.
    m_fb.Resolve();
    m_oitBuffer.Compose(m_fb);
    window.Present(m_fb);
}
