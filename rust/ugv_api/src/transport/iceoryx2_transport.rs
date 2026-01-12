use crate::commands::{CommandEnvelope, CommandResult};
use crate::sensors::SensorFrame;
use crate::transport::Transport;

pub struct Iceoryx2Transport;

impl Transport for Iceoryx2Transport {
    fn publish_sensor_frame(&mut self, _frame: SensorFrame) -> bool {
        // TODO: implement Iceoryx2 publish.
        true
    }

    fn receive_commands(&mut self) -> Vec<CommandEnvelope> {
        // TODO: implement Iceoryx2 command receive.
        Vec::new()
    }

    fn publish_command_result(&mut self, _command_id: u64, _result: CommandResult) -> bool {
        // TODO: implement Iceoryx2 command result publish.
        true
    }
}
