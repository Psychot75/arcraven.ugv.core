use crate::sensors::SensorField;

#[derive(Debug, Clone)]
pub struct SensorDescriptor {
    pub name: String,
    pub fields: Vec<SensorField>,
}
