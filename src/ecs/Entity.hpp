#pragma once
// =====================================================================
// Entity.hpp — Entity 타입 정의 + ID 발급기
//
// [이 파일이 필요한 이유]
// ECS에서 Entity는 데이터를 직접 갖지 않고 그냥 숫자(ID)로만 존재함
// 이 ID를 통해 ComponentPool에서 해당 Entity의 데이터를 찾음
// EntityManager는 ID가 중복되지 않도록 발급하고,
// 삭제된 Entity의 ID를 재사용해서 메모리 낭비를 막음
// =====================================================================

#include <cstdint>  // uint32_t 타입을 쓰기 위한 헤더 (C#의 using System과 비슷)
#include <limits>   // numeric_limits (타입의 최댓값을 구하기 위해)
#include <vector>   // std::vector (C#의 List와 동일)

// Entity는 그냥 숫자 — uint32_t(0~42억)를 Entity라는 이름으로 재정의
// C#의 "using Entity = uint" 와 동일
using Entity = uint32_t;

// 유효하지 않은 Entity를 표현하는 상수 — C#의 null과 비슷한 역할
// uint32_t의 최댓값(42억)을 사용해서 실제 ID와 절대 겹치지 않게 함
inline constexpr Entity NULL_ENTITY = std::numeric_limits<Entity>::max();

// Entity ID를 발급하고 반납받아 재사용하는 관리자
class EntityManager {
public:
    Entity create() {
        // free_list에 반납된 ID가 있으면 재사용 (메모리 낭비 방지)
        if (!free_list.empty()) {
            Entity e = free_list.back();   // 가장 마지막에 반납된 ID를 꺼냄
            free_list.pop_back();          // 꺼냈으니 리스트에서 제거
            return e;                      // 재사용할 ID 반환
        }
        return next_id++;  // 반납된 ID 없으면 새 ID 발급 후 다음번을 위해 1 증가
    }

    void destroy(Entity e) {
        free_list.push_back(e);  // 삭제된 ID를 반납 목록에 보관해서 나중에 재사용
    }

private:
    Entity next_id = 0;              // 다음에 새로 발급할 ID (0부터 시작)
    std::vector<Entity> free_list;   // 반납된 ID 보관 목록 (C#의 List<uint>와 동일)
};
