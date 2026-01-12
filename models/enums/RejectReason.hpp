#pragma once
#include <cstdint>
//
// Created by EH02 on 2026-01-09.
//

namespace arcraven::ugv {

    // Common rejection reasons (good for debugging + audit trails).
    enum class RejectReason : uint8_t {
        None               = 0,
        NotAuthorized      = 1,
        InvalidPayload     = 2,
        Unsupported        = 3,
        Unsafe             = 4,
        Busy               = 5,
        PreconditionsFail  = 6,
        Timeout            = 7,
        StaleCommand       = 8,
    };
}
