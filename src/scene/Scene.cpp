// Entity 생성과 활성 카메라 및 광원 조회를 구현합니다.
#include "scene/Scene.h"

Scene::Scene()
    : m_activeCameraEntity(NULL_ENTITY)
    , m_activeLightEntity(NULL_ENTITY) {
}

Entity Scene::CreateEntity() {
    return m_registry.create();
}

void Scene::DestroyEntity(Entity e) {
    m_registry.destroy(e);
    if (m_activeCameraEntity == e) m_activeCameraEntity = NULL_ENTITY;
    if (m_activeLightEntity == e) m_activeLightEntity = NULL_ENTITY;
}

void Scene::SetActiveCamera(Entity e) {
    m_activeCameraEntity = e;
}

void Scene::SetActiveLight(Entity e) {
    m_activeLightEntity = e;
}

Registry& Scene::GetRegistry() {
    return m_registry;
}

const Registry& Scene::GetRegistry() const {
    return m_registry;
}

Camera& Scene::GetActiveCamera() {

    if (m_activeCameraEntity != NULL_ENTITY && m_registry.has<Camera>(m_activeCameraEntity)) {
        return m_registry.get<Camera>(m_activeCameraEntity);
    }
    return m_defaultCamera;
}

Light& Scene::GetActiveLight() {

    if (m_activeLightEntity != NULL_ENTITY && m_registry.has<Light>(m_activeLightEntity)) {
        return m_registry.get<Light>(m_activeLightEntity);
    }
    return m_defaultLight;
}
