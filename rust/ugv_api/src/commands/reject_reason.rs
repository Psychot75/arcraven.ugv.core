#[derive(Debug, Clone, Copy, PartialEq, Eq)]
#[repr(u16)]
pub enum RejectReason {
    None = 0,
    NotAuthorized = 1,
    InvalidPayload = 2,
    Unsupported = 3,
    Unsafe = 4,
    Busy = 5,
    PreconditionsFail = 6,
    Timeout = 7,
    StaleCommand = 8,
}
