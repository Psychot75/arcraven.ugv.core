use crate::commands::CommandResult;

#[derive(Debug, Clone)]
pub struct CommandResultEvent {
    pub command_id: u64,
    pub result: CommandResult,
}
