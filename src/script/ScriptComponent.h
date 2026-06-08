// Entity에 스크립트 인스턴스를 연결하는 컴포넌트를 정의합니다.
#pragma once

#include "script/IScript.h"
#include <memory>

struct ScriptComponent {
    std::shared_ptr<IScript> script;
};
