use std::collections::HashMap;

#[derive(Debug, Clone)]
pub struct SensorReading {
    pub sensor: String,
    pub values: HashMap<String, f64>,
}
