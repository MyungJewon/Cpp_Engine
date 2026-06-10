// Screen-space UI component data with a top-left origin.
#pragma once

#include "math/Vec3.h"
#include <functional>
#include <string>

enum class UIType {
    Rect,
    Text
};

struct UIComponent {
    UIType type = UIType::Rect;
    float x = 0.0f;
    float y = 0.0f;
    float width = 0.0f;
    float height = 0.0f;
    Vec3 color = { 1.0f, 1.0f, 1.0f };
    float alpha = 1.0f;
    std::string text;
    int fontSize = 1;
    bool visible = true;
    std::function<void()> onClick = nullptr;
};
