#pragma once
#include <chrono>
#include <thread>

namespace arcraven::ugv {

using SteadyClock = std::chrono::steady_clock;

struct Rate {
    explicit Rate(std::chrono::microseconds p) : period(p) {}
    std::chrono::microseconds period;
};

inline void sleep_until_next(SteadyClock::time_point& next, const Rate& r) {
    next += r.period;
    std::this_thread::sleep_until(next);
}

} // namespace arcraven::ugvcore
