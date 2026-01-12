#pragma once

#include <atomic>
#include <string>

#include "command/CommandRouter.hpp"
#include "subsystems/Interfaces.hpp"

namespace arcraven::ugv {

class Iceoryx2Bridge final : public ICommandLink {
public:
    Iceoryx2Bridge();

    void attach_router(CommandRouter* router);

    bool init() override;
    bool pump_rx() override;
    bool pump_tx() override;

    bool publish_sensor_frame(const SensorFrame& frame);

private:
    CommandRouter* router_ = nullptr;
    std::atomic<bool> initialized_{false};
};

} // namespace arcraven::ugv
