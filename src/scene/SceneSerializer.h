#pragma once
#include <string>

class Scene;
class Registry;

class SceneSerializer {
public:
    static bool Save(Scene& scene, const std::string& path);
    static bool Load(const std::string& path, Scene& scene);
};
