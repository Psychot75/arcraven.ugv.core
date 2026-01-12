#include "subsystems/Iceoryx2Bridge.hpp"

#include <exception>
#include <fstream>
#include <sstream>
#include <vector>

#include "utils/Base64.hpp"
#include "utils/Logger.hpp"

namespace arcraven::ugv {

Iceoryx2Bridge::Iceoryx2Bridge() = default;

void Iceoryx2Bridge::attach_router(CommandRouter* router) {
    router_ = router;
}

void Iceoryx2Bridge::configure_paths(std::filesystem::path base_dir) {
    command_path_ = base_dir / "commands.in";
    telemetry_path_ = base_dir / "telemetry.out";
}

bool Iceoryx2Bridge::init() {
    if (command_path_.empty() || telemetry_path_.empty()) {
        ARC_LOG_ERROR("Iceoryx2Bridge: missing base paths");
        return false;
    }
    std::error_code ec;
    std::filesystem::create_directories(command_path_.parent_path(), ec);
    if (ec) {
        ARC_LOG_ERROR("Iceoryx2Bridge: failed to create bridge directory");
        return false;
    }

    if (!std::filesystem::exists(command_path_)) {
        std::ofstream create(command_path_);
        create.close();
    }
    if (!std::filesystem::exists(telemetry_path_)) {
        std::ofstream create(telemetry_path_);
        create.close();
    }

    initialized_.store(true, std::memory_order_release);
    ARC_LOG_INFO("Iceoryx2Bridge: init (stub)");
    return true;
}

bool Iceoryx2Bridge::pump_rx() {
    if (!initialized_.load(std::memory_order_acquire)) return false;
    if (!router_) return false;

    std::ifstream in(command_path_);
    if (!in.good()) return false;
    in.seekg(static_cast<std::streamoff>(command_offset_));

    std::string line;
    while (std::getline(in, line)) {
        command_offset_ += static_cast<uint64_t>(line.size() + 1);
        if (line.empty()) continue;
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        if (line.rfind("C|", 0) != 0) continue;

        std::vector<std::string> parts;
        std::stringstream ss(line);
        std::string segment;
        while (std::getline(ss, segment, '|')) {
            parts.push_back(segment);
        }
        if (parts.size() < 9) {
            ARC_LOG_WARN("Iceoryx2Bridge: command line malformed");
            continue;
        }

        CommandEnvelope env{};
        try {
            env.command_id = std::stoull(parts[1]);
            env.command = static_cast<arcraven::ugv::UgvCommand>(std::stoul(parts[2]));
            env.domain = static_cast<arcraven::ugv::CommandDomain>(std::stoul(parts[3]));
            env.priority = static_cast<arcraven::ugv::CommandPriority>(std::stoul(parts[4]));
            env.authority = static_cast<arcraven::ugv::CommandAuthority>(std::stoul(parts[5]));
            env.issued_ns = std::stoull(parts[6]);
            env.ttl_ns = std::stoull(parts[7]);
        } catch (const std::exception&) {
            ARC_LOG_WARN("Iceoryx2Bridge: command parse failed");
            continue;
        }

        bool ok = false;
        env.payload_json = arcraven::utils::base64_decode(parts[8], ok);
        if (!ok) {
            ARC_LOG_WARN("Iceoryx2Bridge: payload decode failed");
        }

        (void)router_->submit(std::move(env));
    }
    return false;
}

bool Iceoryx2Bridge::pump_tx() {
    if (!initialized_.load(std::memory_order_acquire)) return false;

    // TODO: publish command acks over Iceoryx2.
    return false;
}

bool Iceoryx2Bridge::publish_sensor_frame(const SensorFrame& frame) {
    return publish_telemetry(frame, {});
}

bool Iceoryx2Bridge::publish_telemetry(const SensorFrame& frame, const std::vector<JointState>& joints) {
    if (!initialized_.load(std::memory_order_acquire)) return false;

    std::ofstream out(telemetry_path_, std::ios::app);
    if (!out.good()) return false;

    out << "T|" << frame.timestamp_ns << "|" << joints.size();
    for (const auto& joint : joints) {
        out << "|" << joint.id << "|" << joint.name << "|" << joint.position << "|" << joint.velocity << "|" << joint.load;
    }
    const size_t sensor_count = std::min(frame.ids.size(), std::min(frame.types.size(), frame.payloads.size()));
    out << "|" << sensor_count;
    for (size_t i = 0; i < sensor_count; ++i) {
        const auto payload = arcraven::utils::base64_encode(frame.payloads[i]);
        out << "|" << frame.ids[i] << "|" << frame.types[i] << "|" << payload;
    }
    out << "\n";
    return true;
}

bool Iceoryx2Bridge::publish_command_result(uint64_t command_id, const CommandResult& result) {
    if (!initialized_.load(std::memory_order_acquire)) return false;

    std::ofstream out(telemetry_path_, std::ios::app);
    if (!out.good()) return false;

    std::string message = result.message;
    for (auto& c : message) {
        if (c == '|') c = '/';
    }

    out << "R|" << command_id << "|" << static_cast<int>(result.status) << "|"
        << static_cast<int>(result.reject_reason) << "|" << message << "\n";
    return true;
}

} // namespace arcraven::ugv
