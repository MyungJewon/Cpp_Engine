#include "resource/AssetManager.h"

#include <utility>

AssetManager& AssetManager::Get() {
    static AssetManager instance;
    return instance;
}

const Mesh* AssetManager::LoadMesh(const std::string& path) {
    auto cached = m_meshes.find(path);
    if (cached != m_meshes.end()) return &cached->second;

    Mesh mesh = ObjLoader::Load(path);
    if (mesh.vertices.empty() || mesh.indices.empty()) return nullptr;

    ObjLoader::Normalize(mesh);
    auto [it, inserted] = m_meshes.emplace(path, std::move(mesh));
    (void)inserted;
    return &it->second;
}

const Texture* AssetManager::LoadTexture(const std::string& path) {
    auto cached = m_textures.find(path);
    if (cached != m_textures.end()) return &cached->second;

    Texture texture = Texture::Load(path);
    if (!texture.IsValid()) return nullptr;

    auto [it, inserted] = m_textures.emplace(path, std::move(texture));
    (void)inserted;
    return &it->second;
}

void AssetManager::Clear() {
    m_meshes.clear();
    m_textures.clear();
}
