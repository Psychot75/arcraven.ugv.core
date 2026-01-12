#pragma once
#include <cstdint>

//
// Created by EH02 on 2026-01-09.
//
namespace arcraven::ugv {

    // Keep this stable. Any change = bump schema/version in your wire protocol.
    enum class CommandDomain : uint8_t {
        Mobility   = 0,
        Posture    = 1,
        Perception = 2,
        Security   = 3,
        Mission    = 4,
        Interaction= 5,
        Health     = 6,
        Authority  = 7,
    };
}