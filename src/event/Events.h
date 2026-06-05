#pragma once

#include "ecs/Entity.hpp"
#include "input/InputCodes.h"

struct CameraModeToggleEvent {};
struct KeyPressedEvent  { KeyCode key; };
struct KeyReleasedEvent { KeyCode key; };
struct CollisionEvent    { Entity a; Entity b; };
struct TriggerEnterEvent { Entity trigger; Entity other; };
