// 타입 기반 이벤트 구독과 발행을 처리하는 전역 이벤트 버스를 정의합니다.
#pragma once

#include <functional>
#include <unordered_map>
#include <vector>
#include <typeindex>
#include <memory>
#include <any>

class EventBus {
public:
    static EventBus& Get();

    template<typename TEvent>
    static void Subscribe(std::function<void(const TEvent&)> handler) {
        auto& bus = Get();
        auto  key = std::type_index(typeid(TEvent));
        bus.m_handlers[key].push_back(
            [handler](const std::any& e) {
                handler(std::any_cast<const TEvent&>(e));
            }
        );
    }

    template<typename TEvent>
    static void Emit(const TEvent& event) {
        auto& bus = Get();
        auto  key = std::type_index(typeid(TEvent));
        auto  it  = bus.m_handlers.find(key);
        if (it == bus.m_handlers.end()) return;
        for (auto& handler : it->second)
            handler(std::make_any<TEvent>(event));
    }

    static void Clear() { Get().m_handlers.clear(); }

private:
    EventBus() = default;

    std::unordered_map<
        std::type_index,
        std::vector<std::function<void(const std::any&)>>
    > m_handlers;
};

inline EventBus& EventBus::Get() { static EventBus instance; return instance; }
