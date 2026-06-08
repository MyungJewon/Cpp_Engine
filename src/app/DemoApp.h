#pragma once

#include "app/Application.h"
#include "ecs/Entity.hpp"
#include "ecs/World.hpp"
#include "renderer/Renderer.h"
#include "resource/MeshGenerator.h"
#include "resource/Texture.h"
#include "scene/CameraController.h"
#include "scene/Scene.h"
#include "systems/CameraSystem.h"
#include "systems/RenderSystem.h"
#include "systems/ScriptSystem.h"
#include "systems/TransformSystem.h"

// OBJ 모델을 PhongShader로 렌더링하는 데모 애플리케이션
class DemoApp : public Application {
public:
    DemoApp(int width, int height, const char* title);

protected:
    void OnInit() override;
    void OnUpdate(float dt) override;
    void OnFixedUpdate() override;
    void OnRender() override;

private:
    Renderer m_renderer;
    Scene m_scene;
    World m_world;
    CameraController m_cameraController;
    Entity m_cameraEntity = NULL_ENTITY;
    Entity m_sphereEntity1 = NULL_ENTITY;
    Entity m_sphereEntity2 = NULL_ENTITY;
    Entity m_gridEntity = NULL_ENTITY;
    Mesh m_sphereMesh;
    Mesh m_gridMesh;
    Texture m_checkerTex;
};
