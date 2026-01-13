#[derive(Debug, Clone, Copy, PartialEq, Eq)]
#[repr(u16)]
pub enum CommandPriority {
    Background = 0,
    Normal = 1,
    High = 2,
    Critical = 3,
}
