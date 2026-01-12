#[derive(Debug, Clone, Copy, PartialEq, Eq)]
#[repr(u16)]
pub enum CommandStatus {
    None = 0,
    Received = 1,
    Accepted = 2,
    Rejected = 3,
    Running = 4,
    Succeeded = 5,
    Failed = 6,
    Aborted = 7,
    Preempted = 8,
    Expired = 9,
}
