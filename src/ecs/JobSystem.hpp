#pragma once
// =====================================================================
// JobSystem.hpp — 스레드 풀 기반 병렬 처리 시스템
//
// [이 파일이 필요한 이유]
// ComponentPool의 데이터를 CPU 코어 수만큼 나눠서 동시에 처리
// 코어가 14개면 이론상 최대 14배 빠름
// =====================================================================

#include "ecs/Registry.hpp"
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <functional>
#include <vector>
#include <queue>
#include <algorithm>

class JobSystem {
public:
    // explicit — JobSystem js(14); 처럼 명시적으로만 생성 가능하게 강제
    // hardware_concurrency() — 현재 CPU 코어 수를 운영체제에게 물어봄
    explicit JobSystem(size_t thread_count = std::thread::hardware_concurrency()) {
        for (size_t i = 0; i < thread_count; ++i) {
            // emplace_back — vector에 스레드를 그 자리에서 바로 생성해서 추가
            // [this] — 람다 안에서 JobSystem 멤버(tasks, cv 등)에 접근하기 위해 캡처
            workers.emplace_back([this] {
                while (true) {
                    std::function<void()> task;
                    {
                        // unique_lock — wait() 도중 queue_mutex를 잠시 해제했다가 재획득할 수 있음
                        // lock_guard는 이게 불가능해서 wait()와 함께 쓸 때는 unique_lock 필수
                        std::unique_lock<std::mutex> lock(queue_mutex);

                        // 조건 변수 대기 — tasks가 생기거나 stop 신호가 올 때까지 이 스레드를 대기
                        // 대기 중에는 queue_mutex를 해제해서 다른 스레드가 큐에 접근 가능하게 함
                        cv.wait(lock, [this] { return stop || !tasks.empty(); });

                        if (stop && tasks.empty()) return;

                        task = std::move(tasks.front()); // 큐 맨 앞 작업 꺼냄
                        tasks.pop();                     // 꺼낸 작업을 큐에서 제거
                    } // 블록 끝 → queue_mutex 자동 반환

                    task(); // 작업 실행

                    // 완료 카운터 감소 — atomic이라 여러 스레드가 동시에 감소해도 안전
                    // 0이 되면 모든 작업 완료 → parallel_for 대기 중인 메인 스레드를 깨움
                    if (--active_tasks == 0)
                        done_cv.notify_all();
                }
            });
        }
    }

    ~JobSystem() {
        {
            std::lock_guard<std::mutex> lock(queue_mutex);
            stop = true;
        }
        cv.notify_all(); // 대기 중인 모든 워커 스레드를 깨워서 종료하게 함
        for (auto& w : workers)
            w.join(); // 모든 스레드가 완전히 종료될 때까지 대기
    }

    // Registry에서 Ts... 컴포넌트를 모두 가진 Entity를 스레드 수만큼 나눠서 병렬 처리
    // func — (Entity e, Ts&...) 를 받는 람다
    template<typename... Ts, typename Func>
    void parallel_for(Registry& reg, Func func) {
        // 각 타입의 ComponentPool 포인터를 tuple로 묶음
        auto pools = std::make_tuple(&reg.pool<Ts>()...);
        size_t total = std::get<0>(pools)->size();
        if (total == 0) return;

        size_t num_threads = workers.size();
        size_t chunk    = (total + num_threads - 1) / num_threads; // 올림 나눗셈
        size_t num_jobs = (total + chunk - 1) / chunk;

        // 태스크 제출 전에 카운터 설정 — 나중에 설정하면 워커가 먼저 감소시켜 카운터가 엉킬 수 있음
        active_tasks.store(static_cast<int>(num_jobs));

        for (size_t i = 0; i < num_jobs; ++i) {
            size_t start = i * chunk;
            size_t end   = std::min(start + chunk, total);
            {
                std::lock_guard<std::mutex> lock(queue_mutex);
                tasks.push([pools, func, start, end]() {
                    for (size_t j = start; j < end; ++j) {
                        Entity e = std::get<0>(pools)->entity_at(j);
                        // Ts... 모두 가진 Entity만 처리
                        if ((std::get<ComponentPool<Ts>*>(pools)->has(e) && ...))
                            func(e, std::get<ComponentPool<Ts>*>(pools)->get(e)...);
                    }
                });
            }
            cv.notify_one(); // 태스크 하나 추가했으니 워커 하나만 깨움
        }

        // active_tasks가 0이 될 때까지 메인 스레드 대기
        std::unique_lock<std::mutex> lock(done_mutex);
        done_cv.wait(lock, [this] { return active_tasks.load() == 0; });
    }

    size_t thread_count() const { return workers.size(); }

private:
    std::vector<std::thread> workers;

    std::queue<std::function<void()>> tasks;
    std::mutex queue_mutex;          // tasks 큐 동시 접근 방지 뮤텍스
    std::condition_variable cv;      // 새 작업 도착을 워커에게 알리는 조건 변수

    std::atomic<int> active_tasks{0};
    std::condition_variable done_cv; // 모든 작업 완료를 메인 스레드에 알리는 조건 변수
    std::mutex done_mutex;           // done_cv에 필요한 뮤텍스

    bool stop = false;
};
