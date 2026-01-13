#[derive(Debug, Clone)]
pub struct SensorPayload {
    pub id: String,
    pub sensor_type: String,
    pub payload: String,
}
