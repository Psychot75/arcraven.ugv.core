#pragma once
#include <atomic>
#include <mutex>
#include <optional>
#include <string>

#include "utils/Logger.hpp"

namespace arcraven::ugv {

class EStopLatch {
public:
    void trigger(std::string reason) {
        bool expected = false;
        if (latched_.compare_exchange_strong(expected, true, std::memory_order_acq_rel)) {
            {
                std::lock_guard<std::mutex> lk(mu_);
                reason_ = std::move(reason);
            }
            ARC_LOG_FATAL("E-STOP LATCHED: " + reason_string());
        }
    }

    bool latched() const {
        return latched_.load(std::memory_order_acquire);
    }

    std::string reason_string() const {
        std::lock_guard<std::mutex> lk(mu_);
        return reason_.value_or("unknown");
    }

private:
    std::atomic<bool> latched_{false};
    mutable std::mutex mu_;
    std::optional<std::string> reason_;
};

} // namespace arcraven::ugvcore
