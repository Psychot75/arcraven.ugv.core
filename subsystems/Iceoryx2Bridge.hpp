#pragma once

#include <atomic>
#include <filesystem>
#include <string>
#include <vector>

#include "command/CommandRouter.hpp"
#include "command/CommandTypes.hpp"
#include "subsystems/Interfaces.hpp"

namespace arcraven::ugv {

class Iceoryx2Bridge final : public ICommandLink {
public:
    Iceoryx2Bridge();

    void attach_router(CommandRouter* router);
    void configure_paths(std::filesystem::path base_dir);

    bool init() override;
    bool pump_rx() override;
    bool pump_tx() override;

    bool publish_sensor_frame(const SensorFrame& frame);
    bool publish_telemetry(const SensorFrame& frame, const std::vector<JointState>& joints);
    bool publish_command_result(uint64_t command_id, const CommandResult& result);

private:
    std::filesystem::path command_path_;
    std::filesystem::path telemetry_path_;
    uint64_t command_offset_ = 0;

    CommandRouter* router_ = nullptr;
    std::atomic<bool> initialized_{false};
};

} // namespace arcraven::ugv
