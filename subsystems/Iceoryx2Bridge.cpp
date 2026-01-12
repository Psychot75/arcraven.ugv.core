#include "subsystems/Iceoryx2Bridge.hpp"

#include "utils/Logger.hpp"

namespace arcraven::ugv {

Iceoryx2Bridge::Iceoryx2Bridge() = default;

void Iceoryx2Bridge::attach_router(CommandRouter* router) {
    router_ = router;
}

bool Iceoryx2Bridge::init() {
    initialized_.store(true, std::memory_order_release);
    ARC_LOG_INFO("Iceoryx2Bridge: init (stub)");
    return true;
}

bool Iceoryx2Bridge::pump_rx() {
    if (!initialized_.load(std::memory_order_acquire)) return false;
    if (!router_) return false;

    // TODO: receive commands from Iceoryx2 and call router_->submit(envelope).
    return false;
}

bool Iceoryx2Bridge::pump_tx() {
    if (!initialized_.load(std::memory_order_acquire)) return false;

    // TODO: publish command acks and telemetry over Iceoryx2.
    return false;
}

bool Iceoryx2Bridge::publish_sensor_frame(const SensorFrame& frame) {
    if (!initialized_.load(std::memory_order_acquire)) return false;
    if (frame.channels.empty()) return false;

    // TODO: serialize and publish sensor frame over Iceoryx2.
    return true;
}

} // namespace arcraven::ugv
