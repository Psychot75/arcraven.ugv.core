use crate::commands::{CommandEnvelope, CommandResult};
use crate::sensors::{SensorDescriptor, SensorFrame};
use crate::transport::Transport;

pub struct UgvApi<T: Transport> {
    transport: T,
    sensors: Vec<SensorDescriptor>,
}

impl<T: Transport> UgvApi<T> {
    pub fn new(transport: T) -> Self {
        Self {
            transport,
            sensors: Vec::new(),
        }
    }

    pub fn register_sensor(&mut self, descriptor: SensorDescriptor) {
        self.sensors.push(descriptor);
    }

    pub fn sensors(&self) -> &[SensorDescriptor] {
        &self.sensors
    }

    pub fn publish_sensor_frame(&mut self, frame: SensorFrame) -> bool {
        self.transport.publish_sensor_frame(frame)
    }

    pub fn poll_commands(&mut self) -> Vec<CommandEnvelope> {
        self.transport.receive_commands()
    }

    pub fn publish_command_result(&mut self, command_id: u64, result: CommandResult) -> bool {
        self.transport.publish_command_result(command_id, result)
    }
}
