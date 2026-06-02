#include "app/DemoApp.h"
#include "core/Path.h"
#include "renderer/MeshRenderer.h"
#include "resource/AssetManager.h"
#include "scene/Transform.h"
#include "systems/InputSystem.h"
#include <array>
#include <cstdio>
#include <memory>
#include <string>

namespace {
constexpr const char* kProjectModelPath = "/Users/deepfine/C++Project/Cpp_Engine/model.obj";
constexpr const char* kProjectRoot      = "/Users/deepfine/C++Project/Cpp_Engine";

std::string JoinPath(const std::string& base, const std::string& name) {
    if (base.empty()) return name;
    const char last = base.back();
    if (last == '/' || last == '\\') return base + name;
    return base + "/" + name;
}

Mesh MakeFallbackCube() {
    Mesh mesh;

    // 각 면마다 법선과 탄젠트가 다르므로 정점을 공유하지 않는다.
    auto addFace = [&](Vec3 p0, Vec3 p1, Vec3 p2, Vec3 p3, Vec3 normal, Vec3 tangent) {
        const int base = static_cast<int>(mesh.vertices.size());
        mesh.vertices.push_back({ p0, { 0.0f, 0.0f }, normal, tangent });
        mesh.vertices.push_back({ p1, { 1.0f, 0.0f }, normal, tangent });
        mesh.vertices.push_back({ p2, { 1.0f, 1.0f }, normal, tangent });
        mesh.vertices.push_back({ p3, { 0.0f, 1.0f }, normal, tangent });
        mesh.indices.insert(mesh.indices.end(), {
            base, base + 1, base + 2,
            base, base + 2, base + 3
        });
    };

    addFace({ -1.0f, -1.0f,  1.0f }, {  1.0f, -1.0f,  1.0f },
            {  1.0f,  1.0f,  1.0f }, { -1.0f,  1.0f,  1.0f },
            { 0.0f, 0.0f, 1.0f }, { 1.0f, 0.0f, 0.0f });
    addFace({  1.0f, -1.0f, -1.0f }, { -1.0f, -1.0f, -1.0f },
            { -1.0f,  1.0f, -1.0f }, {  1.0f,  1.0f, -1.0f },
            { 0.0f, 0.0f, -1.0f }, { -1.0f, 0.0f, 0.0f });
    addFace({  1.0f, -1.0f,  1.0f }, {  1.0f, -1.0f, -1.0f },
            {  1.0f,  1.0f, -1.0f }, {  1.0f,  1.0f,  1.0f },
            { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, -1.0f });
    addFace({ -1.0f, -1.0f, -1.0f }, { -1.0f, -1.0f,  1.0f },
            { -1.0f,  1.0f,  1.0f }, { -1.0f,  1.0f, -1.0f },
            { -1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f });
    addFace({ -1.0f,  1.0f,  1.0f }, {  1.0f,  1.0f,  1.0f },
            {  1.0f,  1.0f, -1.0f }, { -1.0f,  1.0f, -1.0f },
            { 0.0f, 1.0f, 0.0f }, { 1.0f, 0.0f, 0.0f });
    addFace({ -1.0f, -1.0f, -1.0f }, {  1.0f, -1.0f, -1.0f },
            {  1.0f, -1.0f,  1.0f }, { -1.0f, -1.0f,  1.0f },
            { 0.0f, -1.0f, 0.0f }, { 1.0f, 0.0f, 0.0f });

    return mesh;
}

const Mesh* LoadFirstModel(const std::string& exeDir) {
    const std::array<std::string, 3> candidates = {
        JoinPath(exeDir, "model.obj"),
        JoinPath(exeDir, "../../../model.obj"),
        kProjectModelPath
    };

    for (const std::string& path : candidates) {
        if (const Mesh* mesh = AssetManager::Get().LoadMesh(path)) return mesh;
    }

    return nullptr;
}

const Texture* LoadFirstTexture(const std::string& exeDir, const std::array<const char*, 4>& names) {
    const std::array<std::string, 3> bases = {
        exeDir,
        JoinPath(exeDir, "../../.."),
        kProjectRoot
    };

    for (const std::string& base : bases) {
        for (const char* name : names) {
            if (const Texture* texture = AssetManager::Get().LoadTexture(JoinPath(base, name))) return texture;
        }
    }

    return nullptr;
}
}

DemoApp::DemoApp(int width, int height, const char* title)
    : Application(width, height, title)
    , m_renderer(width, height)
    , m_world(m_scene.GetRegistry()) {
}

void DemoApp::OnInit() {
    const std::string exeDir = Path::GetExecutableDir();

    // 실행 파일 주변과 프로젝트 루트에서 모델을 찾고, 실패하면 내장 큐브를 사용한다.
    m_mesh = LoadFirstModel(exeDir);
    if (!m_mesh) {
        std::printf("model.obj not found, using fallback cube\n");
        m_fallbackMesh = MakeFallbackCube();
        m_mesh = &m_fallbackMesh;
    }

    // 텍스처는 있으면 연결하고 없으면 셰이더의 기본 재질 색상을 사용한다.
    m_albedo = LoadFirstTexture(exeDir, { "albedo.tga", "diffuse.tga", "basecolor.tga", "texture.tga" });
    m_normalMap = LoadFirstTexture(exeDir, { "normal.tga", "normal_map.tga", "normalmap.tga", "model_normal.tga" });

    m_cameraEntity = m_scene.CreateEntity();
    Camera camera;
    camera.eye = { 0.0f, 1.0f, 4.0f };
    camera.target = { 0.0f, 0.0f, 0.0f };
    camera.aspect = GetWindow().Width() / static_cast<float>(GetWindow().Height());
    m_scene.GetRegistry().add<Camera>(m_cameraEntity, camera);
    m_scene.SetActiveCamera(m_cameraEntity);

    // 모델 위쪽 오른편에서 비추는 흰색 광원을 둔다.
    Entity lightEntity = m_scene.CreateEntity();
    Light light;
    light.position = { 3.0f, 4.0f, 3.0f };
    light.color = { 1.0f, 1.0f, 1.0f };
    light.ambient = 0.18f;
    light.diffuse = 0.78f;
    light.specular = 0.45f;
    light.shininess = 32.0f;
    m_scene.GetRegistry().add<Light>(lightEntity, light);
    m_scene.SetActiveLight(lightEntity);

    // 로드한 모델과 텍스처를 ECS 컴포넌트로 등록한다.
    m_modelEntity = m_scene.CreateEntity();
    MeshRenderer meshRenderer;
    meshRenderer.mesh = m_mesh;
    meshRenderer.material.albedo = m_albedo;
    meshRenderer.material.normalMap = m_normalMap;
    meshRenderer.material.shininess = light.shininess;
    meshRenderer.material.tint = { 1.0f, 0.72f, 0.58f };

    Transform parentTransform;
    parentTransform.localPos = { -2.5f, 0.0f, 0.0f };
    m_scene.GetRegistry().add<Transform>(m_modelEntity, parentTransform);
    m_scene.GetRegistry().add<MeshRenderer>(m_modelEntity, meshRenderer);
    m_scene.GetRegistry().add<ScriptComponent>(m_modelEntity, ScriptComponent{ std::make_shared<RotatorScript>(40.0f) });

    m_childEntity = m_scene.CreateEntity();
    MeshRenderer childRenderer = meshRenderer;
    childRenderer.material.tint = { 0.58f, 0.86f, 1.0f };
    Transform childTransform;
    childTransform.localPos = { 0.0f, 1.5f, 0.0f };
    childTransform.localScale = { 0.5f, 0.5f, 0.5f };
    m_scene.GetRegistry().add<Transform>(m_childEntity, childTransform);
    m_scene.GetRegistry().add<MeshRenderer>(m_childEntity, childRenderer);
    m_scene.GetRegistry().get<Transform>(m_childEntity).SetParent(m_childEntity, m_modelEntity, m_scene.GetRegistry());

    m_entity2 = m_scene.CreateEntity();
    MeshRenderer meshRenderer2 = meshRenderer;
    meshRenderer2.material.tint = { 0.78f, 1.0f, 0.62f };
    Transform transform2;
    transform2.localPos = { 2.5f, 0.0f, 0.0f };
    m_scene.GetRegistry().add<Transform>(m_entity2, transform2);
    m_scene.GetRegistry().add<MeshRenderer>(m_entity2, meshRenderer2);

    // 모델 아래에 회색 바닥 격자를 추가한다.
    m_gridMesh = MeshGenerator::CreateGrid(20, 1.0f);
    m_gridEntity = m_scene.CreateEntity();
    MeshRenderer gridRenderer;
    gridRenderer.mesh = &m_gridMesh;
    gridRenderer.material.tint = { 0.3f, 0.3f, 0.3f };
    gridRenderer.material.albedo = nullptr;
    gridRenderer.material.normalMap = nullptr;
    Transform gridTransform;
    gridTransform.localPos = { 0.0f, -1.0f, 0.0f };
    m_scene.GetRegistry().add<Transform>(m_gridEntity, gridTransform);
    m_scene.GetRegistry().add<MeshRenderer>(m_gridEntity, gridRenderer);

    m_world.add_system<InputSystem>();
    m_world.add_system<ScriptSystem>();
    m_world.add_system<TransformSystem>();
    m_world.add_system<CameraSystem>(m_cameraController, m_cameraEntity);
    m_world.add_system<RenderSystem>(m_renderer, m_scene, GetWindow());
}

void DemoApp::OnUpdate(float dt) {
    m_world.update(dt);
}

void DemoApp::OnRender() {
}
