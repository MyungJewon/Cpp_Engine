// Mesh와 Texture 캐시를 제공하는 싱글톤 자산 관리자를 선언합니다.
#pragma once

#include "resource/ObjLoader.h"
#include "resource/Texture.h"
#include <string>
#include <unordered_map>

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
