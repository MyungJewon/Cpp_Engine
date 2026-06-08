// Registry 컴포넌트 순회를 스레드 풀로 병렬 실행하는 작업 시스템을 정의합니다.
#pragma once

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

    explicit JobSystem(size_t thread_count = std::thread::hardware_concurrency()) {
        for (size_t i = 0; i < thread_count; ++i) {

            workers.emplace_back([this] {
                while (true) {
                    std::function<void()> task;
                    {

                        std::unique_lock<std::mutex> lock(queue_mutex);

                        cv.wait(lock, [this] { return stop || !tasks.empty(); });

                        if (stop && tasks.empty()) return;

                        task = std::move(tasks.front());
                        tasks.pop();
                    }

                    task();

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
        cv.notify_all();
        for (auto& w : workers)
            w.join();
    }

    template<typename... Ts, typename Func>
    void parallel_for(Registry& reg, Func func) {

        auto pools = std::make_tuple(&reg.pool<Ts>()...);
        size_t total = std::get<0>(pools)->size();
        if (total == 0) return;

        size_t num_threads = workers.size();
        size_t chunk    = (total + num_threads - 1) / num_threads;
        size_t num_jobs = (total + chunk - 1) / chunk;

        active_tasks.store(static_cast<int>(num_jobs));

        for (size_t i = 0; i < num_jobs; ++i) {
            size_t start = i * chunk;
            size_t end   = std::min(start + chunk, total);
            {
                std::lock_guard<std::mutex> lock(queue_mutex);
                tasks.push([pools, func, start, end]() {
                    for (size_t j = start; j < end; ++j) {
                        Entity e = std::get<0>(pools)->entity_at(j);

                        if ((std::get<ComponentPool<Ts>*>(pools)->has(e) && ...))
                            func(e, std::get<ComponentPool<Ts>*>(pools)->get(e)...);
                    }
                });
            }
            cv.notify_one();
        }

        std::unique_lock<std::mutex> lock(done_mutex);
        done_cv.wait(lock, [this] { return active_tasks.load() == 0; });
    }

    size_t thread_count() const { return workers.size(); }

private:
    std::vector<std::thread> workers;

    std::queue<std::function<void()>> tasks;
    std::mutex queue_mutex;
    std::condition_variable cv;

    std::atomic<int> active_tasks{0};
    std::condition_variable done_cv;
    std::mutex done_mutex;

    bool stop = false;
};
