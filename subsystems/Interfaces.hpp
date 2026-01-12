#pragma once

namespace arcraven::ugv {

class IDriveSystem {
public:
    virtual ~IDriveSystem() = default;
    virtual bool init() = 0;
    virtual bool enable() = 0;
    virtual void disable() = 0;
    virtual void estop() = 0;
};

class ISensorSuite {
public:
    virtual ~ISensorSuite() = default;
    virtual bool init() = 0;
    virtual void poll() = 0; // non-blocking poll of sensor updates
};

class ICommandLink {
public:
    virtual ~ICommandLink() = default;
    virtual bool init() = 0;
    virtual bool pump_rx() = 0; // non-blocking; should enqueue into CommandRouter
    virtual bool pump_tx() = 0;
};

} // namespace arcraven::ugvcore
