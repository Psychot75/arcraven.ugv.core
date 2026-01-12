#pragma once
#include <cstdint>

//
// Created by EH02 on 2026-01-09.
//

namespace arcraven::ugv {

    // Useful for arbitration/preemption.
    enum class CommandPriority : uint8_t {
        Background = 0,   // e.g., Observe, ScanArea (non-urgent)
        Normal     = 1,   // typical mission commands
        High       = 2,   // security/tactical actions
        Critical   = 3,   // safety actions that preempt everything
    };
}
