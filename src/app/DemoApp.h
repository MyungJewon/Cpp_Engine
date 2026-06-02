#pragma once

#include "app/Application.h"
#include "ecs/Entity.hpp"
#include "ecs/World.hpp"
#include "renderer/Renderer.h"
#include "resource/MeshGenerator.h"
#include "resource/ObjLoader.h"
#include "resource/Texture.h"
#include "scene/CameraController.h"
#include "scene/Scene.h"
#include "script/RotatorScript.h"
#include "script/ScriptComponent.h"
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
    void OnRender() override;

private:
    Renderer m_renderer;
    Scene m_scene;
    World m_world;
    CameraController m_cameraController;
    Entity m_cameraEntity = NULL_ENTITY;
    Entity m_modelEntity = NULL_ENTITY;
    Entity m_childEntity = NULL_ENTITY;
    Entity m_entity2 = NULL_ENTITY;
    Entity m_gridEntity = NULL_ENTITY;
    const Mesh* m_mesh = nullptr;
    const Texture* m_albedo = nullptr;
    const Texture* m_normalMap = nullptr;
    Mesh m_fallbackMesh;
    Mesh m_gridMesh;
};
