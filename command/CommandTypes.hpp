#pragma once
#include <cstdint>
#include <string>
#include <chrono>

#include "models/enums/CommandAuthority.hpp"
#include "models/enums/CommandDomain.hpp"
#include "models/enums/CommandPriority.hpp"
#include "models/enums/CommandStatus.hpp"
#include "models/enums/RejectReason.hpp"
#include "models/enums/UgvCommand.hpp"

namespace arcraven::ugv {

using SteadyClock = std::chrono::steady_clock;

struct CommandEnvelope {
    arcraven::ugv::UgvCommand command = arcraven::ugv::UgvCommand::Stop;
    arcraven::ugv::CommandDomain domain = arcraven::ugv::CommandDomain::Mobility;
    arcraven::ugv::CommandPriority priority = arcraven::ugv::CommandPriority::Normal;
    arcraven::ugv::CommandAuthority authority = arcraven::ugv::CommandAuthority::Unknown;

    uint64_t command_id = 0;     // caller-provided unique id (wire-safe)
    uint64_t issued_ns = 0;      // monotonic timestamp in ns (optional)
    uint64_t ttl_ns = 0;         // 0 = no expiry

    // Placeholder payload. Replace with a variant/flatbuffer/protobuf later.
    // Keep it string for now so you can wire it quickly over Iceoryx2.
    std::string payload_json;
};

struct CommandResult {
    arcraven::ugv::CommandStatus status = arcraven::ugv::CommandStatus::None;
    arcraven::ugv::RejectReason reject_reason = arcraven::ugv::RejectReason::None;
    std::string message; // optional diagnostic, keep short for telemetry
};

inline bool is_expired(const CommandEnvelope& c, uint64_t now_ns) {
    if (c.ttl_ns == 0 || c.issued_ns == 0) return false;
    return (now_ns - c.issued_ns) > c.ttl_ns;
}

} // namespace arcraven::ugv
