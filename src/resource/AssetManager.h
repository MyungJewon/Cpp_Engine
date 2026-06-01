#pragma once

#include "resource/ObjLoader.h"
#include "resource/Texture.h"
#include <string>
#include <unordered_map>

// 경로 기반 Mesh/Texture 캐시를 관리하는 싱글톤
class AssetManager {
public:
    static AssetManager& Get();

    const Mesh* LoadMesh(const std::string& path);
    const Texture* LoadTexture(const std::string& path);
    void Clear();

private:
    AssetManager() = default;

    std::unordered_map<std::string, Mesh> m_meshes;
    std::unordered_map<std::string, Texture> m_textures;
};
