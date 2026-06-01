#pragma once
#include "input/InputCodes.h"

struct KeyEvent {
    KeyCode key = KeyCode::Count;
    bool repeat = false;
};

struct MouseEvent {
    MouseButton button = MouseButton::Count;
    int x = 0;
    int y = 0;
};

struct MouseMoveEvent {
    int x = 0;
    int y = 0;
    int dx = 0;
    int dy = 0;
};
