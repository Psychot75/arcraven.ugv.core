use crate::commands::{CommandStatus, RejectReason};

#[derive(Debug, Clone)]
pub struct CommandResult {
    pub status: CommandStatus,
    pub reject_reason: RejectReason,
    pub message: String,
}
