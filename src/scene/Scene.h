// Registry와 활성 카메라 및 광원을 묶어 관리하는 Scene을 선언합니다.
#pragma once
#include "ecs/Registry.hpp"
#include "ecs/Entity.hpp"
#include "scene/Camera.h"
#include "scene/Light.h"

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
