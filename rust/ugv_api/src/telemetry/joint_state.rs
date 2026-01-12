#[derive(Debug, Clone)]
pub struct JointState {
    pub id: String,
    pub name: String,
    pub position: f64,
    pub velocity: f64,
    pub load: f64,
}
