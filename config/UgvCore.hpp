#pragma once
#include <thread>
#include <vector>

#include "UgvConfig.hpp"
#include "command/CommandRouter.hpp"
#include "core/EStopLatch.hpp"
#include "core/StateStore.hpp"
#include "core/StopController.hpp"
#include "subsystems/Stubs.hpp"

namespace arcraven::ugv {

class UgvCore final {
public:
    explicit UgvCore(UgvConfig cfg);

    int run();
    void request_stop();

private:
    bool hardware_bringup();
    void load_state();
    bool calibration_sequence();
    bool start_runtime();
    void safe_shutdown();

    // ---- Command integration ----
    void register_default_command_handlers();
    uint64_t now_ns() const;

    // ---- Threads ----
    void estop_thread();
    void control_thread();
    void sensor_thread();
    void io_thread();
    void persist_thread();

private:
    UgvConfig cfg_;
    StopController stop_;
    EStopLatch estop_;

    StateStore state_store_;
    PersistentStateV1 state_{};

    // Replace stubs with real subsystems.
    DriveSystemStub drives_;
    SensorSuiteStub sensors_;
    CommandLinkStub cmd_link_;

    CommandRouter cmd_router_;

    std::vector<std::thread> threads_;
};

} // namespace arcraven::ugvcore
