#pragma once
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <chrono>

namespace arcraven::ugv {

class StopController {
public:
    void request_stop() {
        stop_.store(true, std::memory_order_release);
        cv_.notify_all();
    }

    bool stop_requested() const {
        return stop_.load(std::memory_order_acquire);
    }

    void wait_for_stop(std::chrono::milliseconds max_wait) {
        std::unique_lock<std::mutex> lk(mu_);
        (void)cv_.wait_for(lk, max_wait, [&] { return stop_requested(); });
    }

private:
    std::atomic<bool> stop_{false};
    mutable std::mutex mu_;
    std::condition_variable cv_;
};

} // namespace arcraven::ugvcore
