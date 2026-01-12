#pragma once
#include <cstdint>

//
// Created by EH02 on 2026-01-09.
//

namespace arcraven::ugv {

    // Standard execution status for ACK/telemetry.
    enum class CommandStatus: uint8_t {
        None = 0,
        Received = 1,
        Accepted = 2,
        Rejected = 3,
        Running = 4,
        Succeeded = 5,
        Failed = 6,
        Aborted = 7,
        Preempted = 8,
        Expired = 9,
    };
}
