#pragma once

#include "script/IScript.h"
#include <memory>

// Entity에 실행할 스크립트 인스턴스를 연결하는 컴포넌트
struct ScriptComponent {
    std::shared_ptr<IScript> script;
};
