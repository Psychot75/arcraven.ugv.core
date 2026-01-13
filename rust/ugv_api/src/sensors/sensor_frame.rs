use crate::sensors::{SensorPayload, SensorReading};

#[derive(Debug, Clone)]
pub struct SensorFrame {
    pub timestamp_ns: u64,
    pub readings: Vec<SensorReading>,
    pub payloads: Vec<SensorPayload>,
}
