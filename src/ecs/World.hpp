#pragma once
// =====================================================================
// World.hpp — 시스템들을 등록하고 순서대로 실행하는 게임 루프 관리자
//
// [이 파일이 필요한 이유]
// 시스템이 없으면 게임 루프를 이렇게 짜야 함:
//   while (running) {
//       MovementUpdate(reg);
//       HealthUpdate(reg);
//       CollisionUpdate(reg);  // 시스템 추가할 때마다 여기도 직접 수정
//   }
// World를 쓰면 시스템 추가/제거가 한 줄로 끝나고
// 게임 루프는 world.update() 하나로 통일됨
// =====================================================================

#include "ecs/System.hpp"
#include <vector>  // std::vector — C#의 List와 동일
#include <memory>  // unique_ptr — 자동 메모리 해제를 위한 스마트 포인터

class World {
public:
    // 생성자 — Registry의 참조를 받아서 저장
    // & 는 참조 — 복사하지 않고 외부의 Registry를 직접 가리킴
    // C#에서 클래스를 인자로 넘기면 참조로 전달되는 것과 동일
    World(Registry& reg) : reg(reg) {}

    // 시스템 등록 — world.add_system<MovementSystem>() 처럼 사용
    // template<typename T> — T 자리에 MovementSystem, HealthSystem 등이 들어옴
    template<typename T>
    void add_system() {
        // make_unique<T>() — T 타입 시스템을 생성하고 unique_ptr로 감쌈
        // ISystem* 으로 저장되어 다른 타입의 시스템도 같은 리스트에 담을 수 있음
        systems.push_back(std::make_unique<T>());
    }

    // 등록된 모든 시스템을 추가된 순서대로 실행
    // 게임 루프에서 매 프레임 호출
    void update() {
        // auto& system — systems 리스트의 각 요소를 복사 없이 순회
        for (auto& system : systems)
            system->update(reg); // 각 시스템의 update() 실행, Registry를 넘겨줌
    }

private:
    Registry& reg; // 외부 Registry의 참조 — World는 소유하지 않고 빌려서 사용

    // 등록된 시스템 목록
    // ISystem*로 저장해서 MovementSystem, HealthSystem 등 다른 타입을 하나의 리스트에 보관
    // unique_ptr 사용 이유: World가 소멸될 때 모든 시스템도 자동으로 메모리 해제
    std::vector<std::unique_ptr<ISystem>> systems;
};
