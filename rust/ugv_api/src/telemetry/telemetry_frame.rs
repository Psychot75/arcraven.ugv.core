use crate::sensors::{SensorPayload, SensorReading};
use crate::telemetry::JointState;

#[derive(Debug, Clone)]
pub struct TelemetryFrame {
    pub timestamp_ns: u64,
    pub joints: Vec<JointState>,
    pub sensors: Vec<SensorReading>,
    pub payloads: Vec<SensorPayload>,
}
