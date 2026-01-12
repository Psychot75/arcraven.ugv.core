use crate::commands::{CommandEnvelope, CommandResult};
use crate::sensors::SensorFrame;

pub trait Transport {
    fn publish_sensor_frame(&mut self, frame: SensorFrame) -> bool;
    fn receive_commands(&mut self) -> Vec<CommandEnvelope>;
    fn publish_command_result(&mut self, command_id: u64, result: CommandResult) -> bool;
}
