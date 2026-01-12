use crate::commands::{CommandAuthority, CommandDomain, CommandPriority, UgvCommand};

#[derive(Debug, Clone)]
pub struct CommandEnvelope {
    pub command: UgvCommand,
    pub domain: CommandDomain,
    pub priority: CommandPriority,
    pub authority: CommandAuthority,
    pub command_id: u64,
    pub issued_ns: u64,
    pub ttl_ns: u64,
    pub payload_json: String,
}
