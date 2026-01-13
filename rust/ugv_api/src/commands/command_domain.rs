#[derive(Debug, Clone, Copy, PartialEq, Eq)]
#[repr(u16)]
pub enum CommandDomain {
    Mobility = 0,
    Posture = 1,
    Perception = 2,
    Security = 3,
    Mission = 4,
    Interaction = 5,
    Health = 6,
    Authority = 7,
}
