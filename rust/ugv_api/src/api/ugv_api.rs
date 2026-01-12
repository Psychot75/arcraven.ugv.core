use crate::commands::{CommandEnvelope, CommandResultEvent};
use crate::sensors::SensorDescriptor;
use crate::telemetry::TelemetryFrame;
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

    pub fn send_command(&mut self, command: CommandEnvelope) -> bool {
        self.transport.send_command(command)
    }

    pub fn poll_telemetry(&mut self) -> Vec<TelemetryFrame> {
        self.transport.receive_telemetry()
    }

    pub fn poll_command_results(&mut self) -> Vec<CommandResultEvent> {
        self.transport.receive_command_results()
    }
}
