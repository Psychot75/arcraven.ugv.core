# arcraven.ugv.core

## Projects

This repository is split into two projects that work together:

- **C++ Core**: real-time runtime responsible for command routing, safety, and hardware integration.
- **Rust API**: external control/telemetry API that can publish sensor data and receive commands over a transport.

## Command & Telemetry Flow

1. External clients send commands over the transport (Iceoryx2 planned).
2. The C++ core receives commands via the `Iceoryx2Bridge` and pushes them into the `CommandRouter`.
3. The control thread executes command handlers and publishes acknowledgements/telemetry.
4. Sensor frames are polled, packaged, and published back out through the same transport.

## Rust API Usage

The Rust API is intentionally dynamic so new sensors can be added without static API changes.

### Register sensors

Create descriptors for each sensor and register them with `UgvApi` so your client can manage
expected telemetry fields.

### Publish sensor frames

Send periodic `SensorFrame` payloads, each containing readings with arbitrary key/value fields.

### Receive commands and publish results

Poll the transport for command envelopes, execute them in your application, and publish results
back for telemetry/ACK workflows.

See `rust/ugv_api` for the API types and the `Transport` trait that abstracts Iceoryx2 and any
future transports.
