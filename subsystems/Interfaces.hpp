#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace arcraven::ugv {

struct SensorFrame {
    uint64_t timestamp_ns = 0;
    std::vector<std::string> ids;
    std::vector<std::string> types;
    std::vector<std::string> payloads;
};

struct JointState {
    std::string id;
    std::string name;
    double position = 0.0;
    double velocity = 0.0;
    double load = 0.0;
};

class IDriveSystem {
public:
    virtual ~IDriveSystem() = default;
    virtual bool init() = 0;
    virtual bool enable() = 0;
    virtual void disable() = 0;
    virtual void estop() = 0;
    virtual bool read_joint_states(std::vector<JointState>& out) = 0;
};

class ISensorSuite {
public:
    virtual ~ISensorSuite() = default;
    virtual bool init() = 0;
    virtual void poll() = 0; // non-blocking poll of sensor updates
    virtual bool read_frame(SensorFrame& out) = 0;
};

class ICommandLink {
public:
    virtual ~ICommandLink() = default;
    virtual bool init() = 0;
    virtual bool pump_rx() = 0; // non-blocking; should enqueue into CommandRouter
    virtual bool pump_tx() = 0;
};

} // namespace arcraven::ugv
