#pragma once
#include "ecs/Registry.hpp"
#include "ecs/Entity.hpp"
#include "scene/Camera.h"
#include "scene/Light.h"

// 씬 — Registry + 활성 카메라/라이트 Entity를 하나로 묶어 관리
class Scene {
public:
    Scene();
    Entity CreateEntity();
    void   DestroyEntity(Entity e);
    void SetActiveCamera(Entity e);
    void SetActiveLight (Entity e);
    Registry& GetRegistry();
    const Registry& GetRegistry() const;
    Camera& GetActiveCamera();
    Light&  GetActiveLight();
    Entity GetActiveCameraEntity() const { return m_activeCameraEntity; }
    Entity GetActiveLightEntity()  const { return m_activeLightEntity;  }
private:
    Registry m_registry;
    Entity   m_activeCameraEntity;
    Entity   m_activeLightEntity;
    Camera m_defaultCamera;
    Light  m_defaultLight;
};
