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
    // 활성 Camera Entity가 없거나 컴포넌트가 없으면 기본 카메라를 반환한다.
    if (m_activeCameraEntity != NULL_ENTITY && m_registry.has<Camera>(m_activeCameraEntity)) {
        return m_registry.get<Camera>(m_activeCameraEntity);
    }
    return m_defaultCamera;
}

Light& Scene::GetActiveLight() {
    // 활성 Light Entity가 없거나 컴포넌트가 없으면 기본 라이트를 반환한다.
    if (m_activeLightEntity != NULL_ENTITY && m_registry.has<Light>(m_activeLightEntity)) {
        return m_registry.get<Light>(m_activeLightEntity);
    }
    return m_defaultLight;
}
