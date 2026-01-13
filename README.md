# arcraven.ugv.core

## Projects

This repository is split into two projects that work together:

- **C++ Core**: real-time runtime responsible for command routing, safety, and hardware integration.
- **Rust API**: external control/telemetry API that can publish sensor data and receive commands over a transport.

## Command & Telemetry Flow

1. External clients send commands over the transport (Iceoryx2 planned).
2. The C++ core receives commands via the `Iceoryx2Bridge` and pushes them into the `CommandRouter`.
3. The control thread executes command handlers and publishes acknowledgements/telemetry.
4. Sensor frames and joint states are polled, packaged, and published back out through the same transport.

## Rust API Usage

The Rust API is intentionally dynamic so new sensors can be added without static API changes.

### Register sensors

Create descriptors for each sensor and register them with `UgvApi` so your client can manage
expected telemetry fields.

### Send commands

Send `CommandEnvelope` payloads through `UgvApi::send_command` to deliver them immediately to the core.
Commands are line-encoded today so the Rust API can write directly into the bridge transport files.

### Receive telemetry

Poll `UgvApi::poll_telemetry` to read sensor and joint data streamed out of the core.
Telemetry frames include sensor readings and joint states so the client can align data to a URDF.
Sensor payloads are generic `id/type/payload` triples so lidar, radiation, and custom sensors can be transported
without changing the API shape.

### Receive command results

Poll `UgvApi::poll_command_results` to get acknowledgements and rejection reasons for previously
submitted commands.

### Flashlight/Signal command

Use the `Signal` command with payload `flashlight|<id>|<state>` where `state` is `1` (on) or `0` (off).

See `rust/ugv_api` for the API types and the `Transport` trait that abstracts Iceoryx2 and any
future transports.

## Current Transport Encoding

Until Iceoryx2 serialization is wired, the bridge uses a line-based encoding in `data_dir/bridge`:

- `commands.in` receives command lines (enum fields are numeric wire values):
  `C|command_id|command|domain|priority|authority|issued_ns|ttl_ns|payload_base64`
- `telemetry.out` emits telemetry lines:
  `T|timestamp_ns|joint_count|joint_id|joint_name|pos|vel|load|...|sensor_count|sensor_id|sensor_type|payload_base64|...`
- `telemetry.out` also includes command results:
  `R|command_id|status|reject_reason|message`
