#include "scene/SceneSerializer.h"

#include "nlohmann/json.hpp"
#include "scene/Scene.h"
#include "scene/Transform.h"
#include "physics/RigidBody.h"
#include "physics/Collider.h"
#include "renderer/MeshRenderer.h"
#include "audio/AudioSource.h"
#include "audio/AudioClip.h"
#include "ui/UIComponent.h"
#include <fstream>
#include <memory>
#include <vector>

namespace {

using json = nlohmann::json;

std::vector<std::unique_ptr<AudioClip>> g_loadedAudioClips;

json Vec3ToJson(const Vec3& value) {
    return {
        {"x", value.x},
        {"y", value.y},
        {"z", value.z}
    };
}

Vec3 JsonToVec3(const json& value, const Vec3& fallback = {0.0f, 0.0f, 0.0f}) {
    return {
        value.value("x", fallback.x),
        value.value("y", fallback.y),
        value.value("z", fallback.z)
    };
}

const char* ColliderShapeToString(ColliderShape shape) {
    return shape == ColliderShape::Sphere ? "Sphere" : "AABB";
}

ColliderShape ColliderShapeFromString(const std::string& shape) {
    return shape == "Sphere" ? ColliderShape::Sphere : ColliderShape::AABB;
}

const char* UITypeToString(UIType type) {
    return type == UIType::Text ? "Text" : "Rect";
}

UIType UITypeFromString(const std::string& type) {
    return type == "Text" ? UIType::Text : UIType::Rect;
}

AudioClip* MakeAudioClip(const std::string& path) {
    if (path.empty()) {
        return nullptr;
    }

    auto clip = std::make_unique<AudioClip>();
    clip->path = path;
    AudioClip* result = clip.get();
    g_loadedAudioClips.push_back(std::move(clip));
    return result;
}

} // namespace

bool SceneSerializer::Save(Scene& scene, const std::string& path) {
    Registry& reg = scene.GetRegistry();

    json root;
    root["entities"] = json::array();

    const auto& transforms = reg.pool<Transform>();
    for (size_t i = 0; i < transforms.size(); ++i) {
        Entity entity = transforms.entity_at(i);

        json entityJson;
        entityJson["id"] = entity;
        json components;

        if (reg.has<Transform>(entity)) {
            const Transform& transform = reg.get<Transform>(entity);
            components["Transform"] = {
                {"localPos", Vec3ToJson(transform.localPos)},
                {"localScale", Vec3ToJson(transform.localScale)}
            };
        }

        if (reg.has<RigidBody>(entity)) {
            const RigidBody& body = reg.get<RigidBody>(entity);
            components["RigidBody"] = {
                {"mass", body.mass},
                {"drag", body.drag},
                {"angularDrag", body.angularDrag},
                {"useGravity", body.useGravity},
                {"isKinematic", body.isKinematic}
            };
        }

        if (reg.has<Collider>(entity)) {
            const Collider& collider = reg.get<Collider>(entity);
            components["Collider"] = {
                {"shape", ColliderShapeToString(collider.shape)},
                {"radius", collider.radius},
                {"halfExtents", Vec3ToJson(collider.halfExtents)},
                {"restitution", collider.restitution},
                {"friction", collider.friction},
                {"isTrigger", collider.isTrigger}
            };
        }

        if (reg.has<MeshRenderer>(entity)) {
            const MeshRenderer& renderer = reg.get<MeshRenderer>(entity);
            components["MeshRenderer"] = {
                {"meshName", renderer.meshName},
                {"tint", Vec3ToJson(renderer.material.tint)},
                {"shininess", renderer.material.shininess}
            };
        }

        if (reg.has<AudioSource>(entity)) {
            const AudioSource& source = reg.get<AudioSource>(entity);
            components["AudioSource"] = {
                {"clipPath", source.clip ? source.clip->path : ""},
                {"volume", source.volume},
                {"pitch", source.pitch},
                {"loop", source.loop},
                {"playOnAwake", source.playOnAwake}
            };
        }

        if (reg.has<UIComponent>(entity)) {
            const UIComponent& ui = reg.get<UIComponent>(entity);
            components["UIComponent"] = {
                {"type", UITypeToString(ui.type)},
                {"x", ui.x},
                {"y", ui.y},
                {"width", ui.width},
                {"height", ui.height},
                {"color", Vec3ToJson(ui.color)},
                {"alpha", ui.alpha},
                {"text", ui.text},
                {"fontSize", ui.fontSize}
            };
        }

        entityJson["components"] = components;
        root["entities"].push_back(entityJson);
    }

    std::ofstream out(path);
    if (!out.is_open()) {
        return false;
    }

    out << root.dump(4);
    return out.good();
}

bool SceneSerializer::Load(const std::string& path, Scene& scene) {
    std::ifstream in(path);
    if (!in.is_open()) {
        return false;
    }

    json root;
    try {
        in >> root;
    } catch (const json::exception&) {
        return false;
    }

    Registry& reg = scene.GetRegistry();
    for (const auto& entityJson : root.value("entities", json::array())) {
        Entity entity = scene.CreateEntity();
        const json& components = entityJson.value("components", json::object());

        if (components.contains("Transform")) {
            const json& component = components["Transform"];
            Transform transform;
            transform.localPos = JsonToVec3(component.value("localPos", json::object()), transform.localPos);
            transform.localScale = JsonToVec3(component.value("localScale", json::object()), transform.localScale);
            reg.add<Transform>(entity, transform);
        }

        if (components.contains("RigidBody")) {
            const json& component = components["RigidBody"];
            RigidBody body;
            body.mass = component.value("mass", body.mass);
            body.drag = component.value("drag", body.drag);
            body.angularDrag = component.value("angularDrag", body.angularDrag);
            body.useGravity = component.value("useGravity", body.useGravity);
            body.isKinematic = component.value("isKinematic", body.isKinematic);
            reg.add<RigidBody>(entity, body);
        }

        if (components.contains("Collider")) {
            const json& component = components["Collider"];
            Collider collider;
            collider.shape = ColliderShapeFromString(component.value("shape", std::string("AABB")));
            collider.radius = component.value("radius", collider.radius);
            collider.halfExtents = JsonToVec3(component.value("halfExtents", json::object()), collider.halfExtents);
            collider.restitution = component.value("restitution", collider.restitution);
            collider.friction = component.value("friction", collider.friction);
            collider.isTrigger = component.value("isTrigger", collider.isTrigger);
            reg.add<Collider>(entity, collider);
        }

        if (components.contains("MeshRenderer")) {
            const json& component = components["MeshRenderer"];
            MeshRenderer renderer;
            renderer.mesh = nullptr;
            renderer.material.albedo = nullptr;
            renderer.material.tint = JsonToVec3(component.value("tint", json::object()), renderer.material.tint);
            renderer.meshName = component.value("meshName", std::string());
            renderer.material.shininess = component.value("shininess", renderer.material.shininess);
            reg.add<MeshRenderer>(entity, renderer);
        }

        if (components.contains("AudioSource")) {
            const json& component = components["AudioSource"];
            AudioSource source;
            source.clip = MakeAudioClip(component.value("clipPath", std::string()));
            source.volume = component.value("volume", source.volume);
            source.pitch = component.value("pitch", source.pitch);
            source.loop = component.value("loop", source.loop);
            source.playOnAwake = component.value("playOnAwake", source.playOnAwake);
            reg.add<AudioSource>(entity, source);
        }

        if (components.contains("UIComponent")) {
            const json& component = components["UIComponent"];
            UIComponent ui;
            ui.type = UITypeFromString(component.value("type", std::string("Rect")));
            ui.x = component.value("x", ui.x);
            ui.y = component.value("y", ui.y);
            ui.width = component.value("width", ui.width);
            ui.height = component.value("height", ui.height);
            ui.color = JsonToVec3(component.value("color", json::object()), ui.color);
            ui.alpha = component.value("alpha", ui.alpha);
            ui.text = component.value("text", ui.text);
            ui.fontSize = component.value("fontSize", ui.fontSize);
            reg.add<UIComponent>(entity, ui);
        }
    }

    return true;
}
