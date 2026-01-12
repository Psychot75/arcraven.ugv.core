use crate::sensors::SensorReading;

#[derive(Debug, Clone)]
pub struct SensorFrame {
    pub timestamp_ns: u64,
    pub readings: Vec<SensorReading>,
}
