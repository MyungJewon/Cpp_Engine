#include "app/DemoApp.h"
#include "core/Path.h"
#include "math/MathUtils.h"
#include "renderer/ShadowPass.h"
#include <array>
#include <cstdio>
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
    , m_shadowMap(1024, 1024)
    , m_oitBuffer(width, height) {
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

    const Framebuffer& fb = GetFramebuffer();
    m_camera.eye = { 0.0f, 1.0f, 4.0f };
    m_camera.target = { 0.0f, 0.0f, 0.0f };
    m_camera.aspect = fb.Width() / static_cast<float>(fb.Height());

    // 모델 위쪽 오른편에서 비추는 흰색 광원을 둔다.
    m_light.position = { 3.0f, 4.0f, 3.0f };
    m_light.color = { 1.0f, 1.0f, 1.0f };
    m_light.ambient = 0.18f;
    m_light.diffuse = 0.78f;
    m_light.specular = 0.45f;
    m_light.shininess = 32.0f;

    // Pipeline은 Framebuffer 참조를 보관하므로 Application 생성 이후 초기화한다.
    m_pipeline = std::make_unique<Pipeline>(GetFramebuffer());
}

void DemoApp::OnUpdate(float dt) {
    // 초당 40도 속도로 모델을 회전한다.
    m_angle += DegToRad(40.0f) * dt;
}

void DemoApp::OnRender() {
    Framebuffer& fb = GetFramebuffer();
    fb.Clear(Color(20, 20, 20));
    m_oitBuffer.Clear();

    const Mat4 modelMatrix = Mat4::Rotate(m_angle, Vec3(0.0f, 1.0f, 0.0f));
    const Mat4 viewProjection = m_camera.GetProjection() * m_camera.GetView();

    // 광원 시점의 깊이 맵을 먼저 채워 그림자 판정에 사용한다.
    const Mat4 lightView = Mat4::LookAt(m_light.position, Vec3(0.0f, 0.0f, 0.0f), Vec3(0.0f, 1.0f, 0.0f));
    const Mat4 lightProjection = Mat4::Perspective(DegToRad(70.0f), 1.0f, 0.1f, 20.0f);
    const Mat4 lightVP = lightProjection * lightView;

    m_shadowMap.Clear();
    m_shadowMap.SetLightVP(lightVP);
    m_shadowShader.mesh = m_mesh;
    m_shadowShader.lightMVP = lightVP * modelMatrix;

    ShadowPassRenderer shadowPass(m_shadowMap);
    shadowPass.Render(m_shadowShader, m_mesh->indices);

    // Phong 셰이더에 현재 프레임의 카메라, 광원, 재질, 그림자 상태를 전달한다.
    m_phongShader.mesh = m_mesh;
    m_phongShader.albedo = m_albedo;
    m_phongShader.normalMap = m_normalMap;
    m_phongShader.shadowMap = &m_shadowMap;
    m_phongShader.mvp = viewProjection * modelMatrix;
    m_phongShader.modelMat = modelMatrix;
    m_phongShader.cameraPos = m_camera.eye;
    m_phongShader.light = m_light;

    if (m_pipeline) {
        m_pipeline->DrawIndexed(m_phongShader, m_mesh->indices);
    }

    // MSAA 버퍼를 최종 색상 버퍼로 해석한 뒤 투명 프래그먼트를 합성한다.
    if (fb.SampleCount() > 1) {
        fb.Resolve();
    }
    m_oitBuffer.Compose(fb);
    GetWindow().Present(fb);
}
