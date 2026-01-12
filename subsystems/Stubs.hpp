#pragma once
#include <atomic>
#include <cstddef>
#include <string>

#include "subsystems/Interfaces.hpp"
#include "utils/Logger.hpp"

namespace arcraven::ugv {

class DriveSystemStub final : public IDriveSystem {
public:
    explicit DriveSystemStub(size_t drive_count) : drive_count_(drive_count) {}

    bool init() override {
        ARC_LOG_INFO("DriveSystem: init (stub), expected drives: " + std::to_string(drive_count_));
        return true;
    }

    bool enable() override {
        enabled_.store(true, std::memory_order_release);
        ARC_LOG_INFO("DriveSystem: enabled (stub)");
        return true;
    }

    void disable() override {
        enabled_.store(false, std::memory_order_release);
        ARC_LOG_WARN("DriveSystem: disabled (stub)");
    }

    void estop() override {
        enabled_.store(false, std::memory_order_release);
        ARC_LOG_FATAL("DriveSystem: ESTOP (stub) -> outputs disabled");
    }

    bool read_joint_states(std::vector<JointState>& out) override {
        (void)out;
        return false;
    }

    bool enabled() const { return enabled_.load(std::memory_order_acquire); }

private:
    size_t drive_count_{0};
    std::atomic<bool> enabled_{false};
};

class SensorSuiteStub final : public ISensorSuite {
public:
    bool init() override {
        ARC_LOG_INFO("SensorSuite: init (stub) - cameras/lidars/custom sensors discovery");
        return true;
    }

    void poll() override {
        // TODO: read sensors; publish into shared memory/blackboard
    }

    bool read_frame(SensorFrame& out) override {
        (void)out;
        return false;
    }
};

class CommandLinkStub final : public ICommandLink {
public:
    bool init() override {
        ARC_LOG_INFO("CommandLink: init (stub) - later Iceoryx2 RX/TX");
        return true;
    }

    bool pump_rx() override { return false; }
    bool pump_tx() override { return false; }
};

} // namespace arcraven::ugv
