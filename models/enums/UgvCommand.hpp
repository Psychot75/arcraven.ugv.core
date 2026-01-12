#pragma once
#include <cstdint>

//
// Created by EH02 on 2026-01-09.
//

namespace arcraven::ugv {

    // High-level command identifiers (wire-safe, explicit values).
    enum class UgvCommand : uint16_t {
        // -------------------------
        // Mobility & Navigation
        // -------------------------
        FollowPath = 100,
        GoTo = 101,
        Stop = 102,
        HoldPosition = 103,
        Anchor = 104,
        ReplanTo = 105,
        Loiter = 106,
        ReturnToBase = 107,
        FollowTarget = 108,
        Evade = 109,
        Dock = 110,

        // -------------------------
        // Posture & Kinematics
        // -------------------------
        SetSpeedLimit = 200,
        SetStance = 201,
        AlignHeading = 202,
        FaceTarget = 203,
        Stabilize = 204,

        // -------------------------
        // Perception & Sensing
        // -------------------------
        ScanArea = 300,
        Observe = 301,
        FocusSensor = 302,
        TrackEntity = 303,
        CalibrateSensors = 304,

        // -------------------------
        // Security & Tactical
        // -------------------------
        Sentinel = 400,
        SecureArea = 401,
        Escort = 402,
        Checkpoint = 403,
        Investigate = 404,
        Shadow = 405,

        // -------------------------
        // Mission Control & Autonomy
        // -------------------------
        ExecuteMission = 500,
        AbortMission = 501,
        PauseMission = 502,
        ResumeMission = 503,
        Wait = 504,
        HandoffControl = 505,

        // -------------------------
        // Interaction & I/O
        // -------------------------
        Signal = 600,
        Broadcast = 601,
        Acknowledge = 602,
        RequestAssistance = 603,

        // -------------------------
        // Health, Safety & Fault Handling
        // -------------------------
        EmergencyStop = 700,
        SafeMode = 701,
        SelfCheck = 702,
        Recover = 703,
        Shutdown = 704,
        Reboot = 705,

        // -------------------------
        // Authority & ROE
        // -------------------------
        SetRuleset = 800,
        SetOwnership = 801,
        LockCommandSet = 802,
        UnlockCommandSet = 803,
    };
}
