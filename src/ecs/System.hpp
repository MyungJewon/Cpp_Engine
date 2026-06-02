#pragma once
// =====================================================================
// System.hpp — 모든 시스템이 따라야 할 틀 정의
//
// [이 파일이 필요한 이유]
// MovementSystem, HealthSystem 등은 서로 다른 클래스라
// World가 하나의 리스트에 담아서 관리할 수 없음
// ISystem이라는 공통 부모를 만들면
// vector<ISystem*>처럼 하나의 리스트에 모든 시스템을 담을 수 있음
//
// [C#에서 비슷한 것]
// C#의 interface와 완전히 동일
// interface ISystem { void Update(Registry reg); }
// =====================================================================

#include "ecs/Registry.hpp"

// 모든 시스템이 반드시 상속해야 하는 인터페이스
// C#의 interface ISystem과 동일
class ISystem {
public:
    // virtual — 자식 클래스가 이 함수를 override할 수 있게 허용
    // = 0    — 반드시 자식이 구현해야 함, 구현 안 하면 컴파일 에러
    // C#의 abstract void Update(Registry reg)와 동일
    virtual void update(Registry& reg, float dt) = 0;

    // virtual 소멸자 — 부모 포인터로 자식을 delete할 때 자식 소멸자가 제대로 호출되도록 필수
    // 없으면 메모리 누수 발생 가능
    virtual ~ISystem() = default;
};
