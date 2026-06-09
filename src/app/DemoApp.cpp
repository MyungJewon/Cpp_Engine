// 데모 씬 구성과 프레임별 시스템 실행을 구현합니다.
#include "app/DemoApp.h"
#include "physics/Collider.h"
#include "physics/RigidBody.h"
#include "renderer/MeshRenderer.h"
#include "scene/Transform.h"
#include "systems/CollisionSystem.h"
#include "systems/InputSystem.h"
#include "systems/PhysicsSystem.h"
#include "systems/UISystem.h"
#include "ui/UIComponent.h"

DemoApp::DemoApp(int width, int height, const char* title)
    : Application(width, height, title)
    , m_renderer(width, height)
    , m_world(m_scene.GetRegistry()) {
}

void DemoApp::OnInit() {
    m_uiRenderer.Init(GetWindow().PixelWidth(), GetWindow().PixelHeight());

    m_cameraEntity = m_scene.CreateEntity();
    Camera camera;
    camera.eye = { 0.0f, 3.0f, 9.0f };
    camera.target = { 0.0f, 2.0f, 0.0f };
    camera.aspect = GetWindow().Width() / static_cast<float>(GetWindow().Height());
    m_scene.GetRegistry().add<Camera>(m_cameraEntity, camera);
    m_scene.SetActiveCamera(m_cameraEntity);

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

    m_sphereMesh = MeshGenerator::CreateSphere(16, 16, 1.0f);

    {
        std::vector<Color> pixels(8 * 8);
        for (int y = 0; y < 8; y++)
            for (int x = 0; x < 8; x++)
                pixels[y * 8 + x] = ((x + y) % 2 == 0)
                    ? Color(255, 255, 255, 255)
                    : Color(220, 50, 50, 255);
        m_checkerTex = Texture::FromPixels(8, 8, pixels);
    }

    MeshRenderer sphereRenderer;
    sphereRenderer.mesh = &m_sphereMesh;
    sphereRenderer.material.albedo = &m_checkerTex;
    sphereRenderer.material.normalMap = nullptr;
    sphereRenderer.material.shininess = light.shininess;

    m_sphereEntity1 = m_scene.CreateEntity();
    MeshRenderer sphereRenderer1 = sphereRenderer;
    sphereRenderer1.material.tint = { 1.0f, 0.72f, 0.58f };
    Transform sphereTransform1;
    sphereTransform1.localPos = { 0.0f, 5.0f, 0.0f };
    m_scene.GetRegistry().add<Transform>(m_sphereEntity1, sphereTransform1);
    m_scene.GetRegistry().add<MeshRenderer>(m_sphereEntity1, sphereRenderer1);
    RigidBody sphereBody1;
    sphereBody1.useGravity = true;
    sphereBody1.mass = 1.0f;
    m_scene.GetRegistry().add<RigidBody>(m_sphereEntity1, sphereBody1);
    Collider sphereCollider1;
    sphereCollider1.shape = ColliderShape::Sphere;
    sphereCollider1.radius = 1.0f;
    sphereCollider1.restitution = 0.2f;
    sphereCollider1.friction = 0.5f;
    m_scene.GetRegistry().add<Collider>(m_sphereEntity1, sphereCollider1);

    m_sphereEntity2 = m_scene.CreateEntity();
    MeshRenderer sphereRenderer2 = sphereRenderer;
    sphereRenderer2.material.tint = { 0.58f, 0.86f, 1.0f };
    Transform sphereTransform2;
    sphereTransform2.localPos = { 0.3f, 2.0f, 0.0f };
    m_scene.GetRegistry().add<Transform>(m_sphereEntity2, sphereTransform2);
    m_scene.GetRegistry().add<MeshRenderer>(m_sphereEntity2, sphereRenderer2);
    RigidBody sphereBody2;
    sphereBody2.useGravity = true;
    sphereBody2.mass = 1.0f;
    m_scene.GetRegistry().add<RigidBody>(m_sphereEntity2, sphereBody2);
    Collider sphereCollider2;
    sphereCollider2.shape = ColliderShape::Sphere;
    sphereCollider2.radius = 1.0f;
    sphereCollider2.restitution = 0.2f;
    sphereCollider2.friction = 0.5f;
    m_scene.GetRegistry().add<Collider>(m_sphereEntity2, sphereCollider2);

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
    Collider gridCollider;
    gridCollider.shape = ColliderShape::AABB;
    gridCollider.center = { 0.0f, 0.0f, 0.0f };
    gridCollider.halfExtents = { 10.0f, 0.1f, 10.0f };
    m_scene.GetRegistry().add<Collider>(m_gridEntity, gridCollider);

    m_world.add_system<InputSystem>();
    m_world.add_system<ScriptSystem>();
    m_world.add_system<TransformSystem>();
    m_world.add_system<CameraSystem>(m_cameraController, m_cameraEntity);
    m_world.add_system<RenderSystem>(m_renderer, m_scene, GetWindow());
    m_world.add_system<UISystem>(m_uiRenderer, GetWindow());
    m_world.add_fixed_system<PhysicsSystem>();
    m_world.add_fixed_system<CollisionSystem>();

    Entity uiEntity = m_scene.CreateEntity();
    UIComponent ui;
    ui.type     = UIType::Text;
    ui.text     = "Cpp_Engine";
    ui.x        = 10.0f;
    ui.y        = 10.0f;
    ui.color    = { 1.0f, 1.0f, 1.0f };
    ui.fontSize = 2;
    m_scene.GetRegistry().add<UIComponent>(uiEntity, ui);
}

void DemoApp::OnUpdate(float dt) {
    m_world.update(dt);
}

void DemoApp::OnFixedUpdate() {
    m_world.fixed_update(1.0f / 60.0f);
}

void DemoApp::OnRender() {
    GetWindow().SwapBuffers();
}
