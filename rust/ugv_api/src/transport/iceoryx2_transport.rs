use std::fs::{self, OpenOptions};
use std::io::{BufRead, BufReader, Seek, SeekFrom, Write};
use std::path::{Path, PathBuf};

use base64::{engine::general_purpose, Engine as _};

use crate::commands::{CommandEnvelope, CommandResult, CommandResultEvent, CommandStatus, RejectReason};
use crate::sensors::SensorPayload;
use crate::telemetry::{JointState, TelemetryFrame};
use crate::transport::Transport;

pub struct Iceoryx2Transport {
    command_path: PathBuf,
    telemetry_path: PathBuf,
    telemetry_offset: u64,
    pending_telemetry: Vec<TelemetryFrame>,
    pending_results: Vec<CommandResultEvent>,
}

impl Iceoryx2Transport {
    pub fn new(base_dir: impl AsRef<Path>) -> Self {
        let base_dir = base_dir.as_ref();
        let command_path = base_dir.join("commands.in");
        let telemetry_path = base_dir.join("telemetry.out");
        let _ = fs::create_dir_all(base_dir);
        let _ = OpenOptions::new().create(true).append(true).open(&command_path);
        let _ = OpenOptions::new().create(true).append(true).open(&telemetry_path);
        Self {
            command_path,
            telemetry_path,
            telemetry_offset: 0,
            pending_telemetry: Vec::new(),
            pending_results: Vec::new(),
        }
    }

    fn encode_command(&self, command: &CommandEnvelope) -> String {
            let payload = general_purpose::STANDARD.encode(command.payload_json.as_bytes());
        format!(
            "C|{}|{}|{}|{}|{}|{}|{}|{}",
            command.command_id,
            command.command as u16,
            command.domain as u16,
            command.priority as u16,
            command.authority as u16,
            command.issued_ns,
            command.ttl_ns,
            payload
        )
    }

    fn parse_telemetry_line(&self, line: &str) -> Option<TelemetryFrame> {
        let mut parts = line.split('|');
        if parts.next()? != "T" {
            return None;
        }
        let timestamp_ns = parts.next()?.parse().ok()?;
        let joint_count: usize = parts.next()?.parse().ok()?;
        let mut joints = Vec::with_capacity(joint_count);
        for _ in 0..joint_count {
            let id = parts.next()?.to_string();
            let name = parts.next()?.to_string();
            let position = parts.next()?.parse().ok()?;
            let velocity = parts.next()?.parse().ok()?;
            let load = parts.next()?.parse().ok()?;
            joints.push(JointState {
                id,
                name,
                position,
                velocity,
                load,
            });
        }
        let sensor_count: usize = parts.next()?.parse().ok()?;
        let mut payloads = Vec::with_capacity(sensor_count);
        for _ in 0..sensor_count {
            let id = parts.next()?.to_string();
            let sensor_type = parts.next()?.to_string();
            let payload_b64 = parts.next()?.to_string();
            let payload_bytes = general_purpose::STANDARD.decode(payload_b64).ok()?;
            let payload = String::from_utf8(payload_bytes).ok()?;
            payloads.push(SensorPayload {
                id,
                sensor_type,
                payload,
            });
        }
        Some(TelemetryFrame {
            timestamp_ns,
            joints,
            sensors: Vec::new(),
            payloads,
        })
    }

    fn parse_result_line(&self, line: &str) -> Option<CommandResultEvent> {
        let mut parts = line.split('|');
        if parts.next()? != "R" {
            return None;
        }
        let command_id = parts.next()?.parse().ok()?;
        let status_raw: u16 = parts.next()?.parse().ok()?;
        let reject_raw: u16 = parts.next()?.parse().ok()?;
        let message = parts.next().unwrap_or_default().to_string();
        Some(CommandResultEvent {
            command_id,
            result: CommandResult {
                status: Self::status_from_u16(status_raw),
                reject_reason: Self::reject_from_u16(reject_raw),
                message,
            },
        })
    }

    fn status_from_u16(value: u16) -> CommandStatus {
        match value {
            1 => CommandStatus::Received,
            2 => CommandStatus::Accepted,
            3 => CommandStatus::Rejected,
            4 => CommandStatus::Running,
            5 => CommandStatus::Succeeded,
            6 => CommandStatus::Failed,
            7 => CommandStatus::Aborted,
            8 => CommandStatus::Preempted,
            9 => CommandStatus::Expired,
            _ => CommandStatus::None,
        }
    }

    fn reject_from_u16(value: u16) -> RejectReason {
        match value {
            1 => RejectReason::NotAuthorized,
            2 => RejectReason::InvalidPayload,
            3 => RejectReason::Unsupported,
            4 => RejectReason::Unsafe,
            5 => RejectReason::Busy,
            6 => RejectReason::PreconditionsFail,
            7 => RejectReason::Timeout,
            8 => RejectReason::StaleCommand,
            _ => RejectReason::None,
        }
    }

    fn drain_lines(&mut self) {
        let file = match fs::File::open(&self.telemetry_path) {
            Ok(f) => f,
            Err(_) => return,
        };
        let mut reader = BufReader::new(file);
        if reader.seek(SeekFrom::Start(self.telemetry_offset)).is_err() {
            return;
        }
        let mut line = String::new();
        while let Ok(bytes) = reader.read_line(&mut line) {
            if bytes == 0 {
                break;
            }
            self.telemetry_offset += bytes as u64;
            if let Some(frame) = self.parse_telemetry_line(line.trim_end()) {
                self.pending_telemetry.push(frame);
            } else if let Some(result) = self.parse_result_line(line.trim_end()) {
                self.pending_results.push(result);
            }
            line.clear();
        }
    }
}

impl Transport for Iceoryx2Transport {
    fn send_command(&mut self, command: CommandEnvelope) -> bool {
        let line = self.encode_command(&command);
        let mut file = match OpenOptions::new().append(true).open(&self.command_path) {
            Ok(f) => f,
            Err(_) => return false,
        };
        if writeln!(file, "{}", line).is_err() {
            return false;
        }
        true
    }

    fn receive_telemetry(&mut self) -> Vec<TelemetryFrame> {
        self.drain_lines();
        std::mem::take(&mut self.pending_telemetry)
    }

    fn receive_command_results(&mut self) -> Vec<CommandResultEvent> {
        self.drain_lines();
        std::mem::take(&mut self.pending_results)
    }
}
