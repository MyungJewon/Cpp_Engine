#pragma once
#include <string>

struct AudioClip {
    std::string path;
    bool IsValid() const { return !path.empty(); }
};
