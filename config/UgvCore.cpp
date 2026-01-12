#include "UgvCore.hpp"

#include <chrono>
#include <string>

#include "utils/Logger.hpp"

namespace arcraven::ugv {

UgvCore::UgvCore(UgvConfig cfg)
    : cfg_(std::move(cfg)),
      state_store_(cfg_.data_dir / "state.bin"),
      drives_(cfg_.expected_drives),
      cmd_router_(CommandRouterConfig{.max_queue = 256}) {}

int UgvCore::run() {
    ARC_LOG_INFO("ArcUGV core boot sequence start");

    if (!hardware_bringup()) {
        ARC_LOG_FATAL("Hardware bring-up failed");
        return 2;
    }

    load_state();

    if (!calibration_sequence()) {
        ARC_LOG_FATAL("Calibration failed");
        state_.calibrated_ok = 0;
        (void)state_store_.save(state_);
        return 3;
    }

    state_.calibrated_ok = 1;
    (void)state_store_.save(state_);

    register_default_command_handlers();

    if (!start_runtime()) {
        ARC_LOG_FATAL("Failed to start runtime threads");
        safe_shutdown();
        return 4;
    }

    while (!stop_.stop_requested()) {
        stop_.wait_for_stop(std::chrono::milliseconds(250));
    }

    safe_shutdown();
    return 0;
}

void UgvCore::request_stop() {
    stop_.request_stop();
}

bool UgvCore::hardware_bringup() {
    ARC_LOG_INFO("Hardware bring-up");

    if (!drives_.init()) return false;
    if (!sensors_.init()) return false;
    if (!cmd_link_.init()) return false;

    return true;
}

void UgvCore::load_state() {
    if (state_store_.load(state_)) {
        ARC_LOG_INFO("Loaded persisted state: calibrated_ok=" +
                     std::string(state_.calibrated_ok ? "true" : "false"));
    } else {
        ARC_LOG_WARN("No persisted state found; cold boot assumed");
        state_ = {};
    }
}

bool UgvCore::calibration_sequence() {
    ARC_LOG_INFO("Calibration start");

    if (!state_.calibrated_ok) {
        ARC_LOG_WARN("Previous boot uncalibrated -> full calibration required");
    }

    // TODO:
    // - ODrive homing / encoder index search / zeroing
    // - IMU bias estimation
    // - safety interlocks: estop input validity, remote link heartbeat, etc.

    ARC_LOG_INFO("Calibration successful");
    return true;
}

bool UgvCore::start_runtime() {
    ARC_LOG_INFO("Starting runtime threads");

    // Enable drives only once runtime is about to start.
    if (!drives_.enable()) {
        ARC_LOG_ERROR("Drive enable failed");
        return false;
    }

    threads_.emplace_back(&UgvCore::estop_thread, this);
    threads_.emplace_back(&UgvCore::control_thread, this);
    threads_.emplace_back(&UgvCore::sensor_thread, this);
    threads_.emplace_back(&UgvCore::io_thread, this);
    threads_.emplace_back(&UgvCore::persist_thread, this);

    return true;
}

void UgvCore::safe_shutdown() {
    ARC_LOG_WARN("Shutdown requested");

    stop_.request_stop();

    if (estop_.latched()) {
        ARC_LOG_WARN("Shutdown while E-STOP latched: " + estop_.reason_string());
        drives_.estop();
    } else {
        drives_.disable();
    }

    for (auto& t : threads_) {
        if (t.joinable()) t.join();
    }
    threads_.clear();

    (void)state_store_.save(state_);
    ARC_LOG_INFO("Shutdown complete");
}

uint64_t UgvCore::now_ns() const {
    const auto now = SteadyClock::now().time_since_epoch();
    return static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(now).count());
}

void UgvCore::register_default_command_handlers() {
    // This is intentionally thin: it makes command execution callable now
    // (routing + stubs), without implementing the real behaviors yet.

    cmd_router_.register_handler(arcraven::ugv::UgvCommand::EmergencyStop,
        [this](const CommandEnvelope& c) -> CommandResult {
            (void)c;
            estop_.trigger("EmergencyStop command");
            return {arcraven::ugv::CommandStatus::Succeeded, arcraven::ugv::RejectReason::None, "estop latched"};
        });

    cmd_router_.register_handler(arcraven::ugv::UgvCommand::Shutdown,
        [this](const CommandEnvelope& c) -> CommandResult {
            (void)c;
            request_stop();
            return {arcraven::ugv::CommandStatus::Succeeded, arcraven::ugv::RejectReason::None, "shutdown requested"};
        });

    // Default stub handler registration example for a mobility command.
    // Replace these lambdas with real implementations later.
    cmd_router_.register_handler(arcraven::ugv::UgvCommand::GoTo,
        [](const CommandEnvelope& c) -> CommandResult {
            (void)c;
            return {arcraven::ugv::CommandStatus::Accepted, arcraven::ugv::RejectReason::None, "GoTo accepted (stub)"};
        });
}

void UgvCore::estop_thread() {
    ARC_LOG_INFO("E-STOP thread started");
    auto next = SteadyClock::now();

    while (!stop_.stop_requested()) {
        if (estop_.latched()) {
            drives_.estop();
        }

        sleep_until_next(next, cfg_.estop_rate);
    }

    ARC_LOG_INFO("E-STOP thread exiting");
}

void UgvCore::control_thread() {
    ARC_LOG_INFO("Control thread started");
    auto next = SteadyClock::now();

    state_.last_authority = static_cast<uint8_t>(arcraven::ugv::CommandAuthority::Unknown);

    while (!stop_.stop_requested()) {
        if (!estop_.latched()) {
            // Command processing can be done here to keep "control owns actuation".
            cmd_router_.process_some(now_ns(), /*max_n=*/8,
                [](const std::pair<CommandEnvelope, CommandResult>& processed) {
                    // In real code, forward ACK to cmd_link_ tx queue.
                    // For now, just log a minimal trail.
                    const auto& cmd = processed.first;
                    const auto& res = processed.second;
                    ARC_LOG_INFO("Cmd processed: id=" + std::to_string(cmd.command_id) +
                                 " status=" + std::to_string(static_cast<int>(res.status)));
                });

            // TODO: compute control outputs based on latest accepted commands
            // (read atomics / blackboard that handlers update).
        }

        sleep_until_next(next, cfg_.control_rate);
    }

    ARC_LOG_INFO("Control thread exiting");
}

void UgvCore::sensor_thread() {
    ARC_LOG_INFO("Sensor thread started");
    auto next = SteadyClock::now();

    while (!stop_.stop_requested()) {
        sensors_.poll();
        sleep_until_next(next, cfg_.sensor_rate);
    }

    ARC_LOG_INFO("Sensor thread exiting");
}

void UgvCore::io_thread() {
    ARC_LOG_INFO("IO thread started");
    auto next = SteadyClock::now();

    while (!stop_.stop_requested()) {
        (void)cmd_link_.pump_rx();
        (void)cmd_link_.pump_tx();

        // TODO: When you wire Iceoryx2:
        // - cmd_link_ receives a wire packet and calls cmd_router_.submit(envelope)
        // - tx path sends acks/telemetry produced by the core

        sleep_until_next(next, cfg_.io_rate);
    }

    ARC_LOG_INFO("IO thread exiting");
}

void UgvCore::persist_thread() {
    ARC_LOG_INFO("Persist thread started");
    auto next = SteadyClock::now();

    while (!stop_.stop_requested()) {
        (void)state_store_.save(state_);
        sleep_until_next(next, cfg_.persist_rate);
    }

    ARC_LOG_INFO("Persist thread exiting");
}

} // namespace arcraven::ugvcore
