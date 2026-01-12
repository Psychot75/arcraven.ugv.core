#[derive(Debug, Clone, Copy, PartialEq, Eq)]
#[repr(u16)]
pub enum CommandAuthority {
    Unknown = 0,
    Autonomy = 1,
    RemoteOperator = 2,
    MissionControl = 3,
    SafetySystem = 4,
}
