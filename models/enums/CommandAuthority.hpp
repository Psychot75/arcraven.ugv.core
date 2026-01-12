#pragma once
#include <cstdint>

//
// Created by EH02 on 2026-01-09.
//

namespace arcraven::ugv {

    // Who is allowed to issue/override commands (wire-safe).
    enum class CommandAuthority : uint8_t {
        Unknown        = 0,
        Autonomy       = 1,  // onboard autonomy stack
        RemoteOperator = 2,  // human operator
        MissionControl = 3,  // orchestration server
        SafetySystem   = 4,  // independent safety controller
    };
}