#pragma once
#include <string>

class Path {
public:
    static std::string GetExecutableDir();
    static std::string Resolve(const std::string& path);
};
