use crate::commands::{CommandEnvelope, CommandResultEvent};
use crate::telemetry::TelemetryFrame;

pub trait Transport {
    fn send_command(&mut self, command: CommandEnvelope) -> bool;
    fn receive_telemetry(&mut self) -> Vec<TelemetryFrame>;
    fn receive_command_results(&mut self) -> Vec<CommandResultEvent>;
}
