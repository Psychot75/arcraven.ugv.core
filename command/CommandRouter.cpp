
#include "CommandRouter.hpp"

#include <cstdint>

#include "CommandTypes.hpp"

namespace arcraven::ugv {

static inline uint16_t key(arcraven::ugv::UgvCommand c) {
    return static_cast<uint16_t>(c);
}

CommandRouter::CommandRouter(CommandRouterConfig cfg) : cfg_(cfg) {}

void CommandRouter::register_handler(arcraven::ugv::UgvCommand cmd, CommandHandler handler) {
    std::lock_guard<std::mutex> lk(mu_);
    handlers_[key(cmd)] = std::move(handler);
}

bool CommandRouter::has_handler(arcraven::ugv::UgvCommand cmd) const {
    std::lock_guard<std::mutex> lk(mu_);
    return handlers_.find(key(cmd)) != handlers_.end();
}

CommandResult CommandRouter::submit(CommandEnvelope cmd) {
    std::lock_guard<std::mutex> lk(mu_);

    if (q_.size() >= cfg_.max_queue) {
        return {arcraven::ugv::CommandStatus::Rejected, arcraven::ugv::RejectReason::Busy, "queue full"};
    }

    // Minimal early validation: you can harden this later.
    if (cmd.command_id == 0) {
        return {arcraven::ugv::CommandStatus::Rejected, arcraven::ugv::RejectReason::InvalidPayload, "command_id=0"};
    }

    q_.push_back(std::move(cmd));
    return {arcraven::ugv::CommandStatus::Accepted, arcraven::ugv::RejectReason::None, ""};
}

std::optional<std::pair<CommandEnvelope, CommandResult>> CommandRouter::process_one(uint64_t now_ns) {
    CommandEnvelope cmd{};
    CommandHandler handler;

    {
        std::lock_guard<std::mutex> lk(mu_);
        if (q_.empty()) return std::nullopt;

        cmd = std::move(q_.front());
        q_.pop_front();

        auto it = handlers_.find(key(cmd.command));
        if (it != handlers_.end()) handler = it->second;
    }

    if (is_expired(cmd, now_ns)) {
        return std::make_pair(std::move(cmd),
                              CommandResult{arcraven::ugv::CommandStatus::Rejected, arcraven::ugv::RejectReason::StaleCommand, "expired"});
    }

    if (!handler) {
        return std::make_pair(std::move(cmd),
                              CommandResult{arcraven::ugv::CommandStatus::Rejected, arcraven::ugv::RejectReason::Unsupported, "no handler"});
    }

    // Execute handler (expected to be fast and non-blocking).
    CommandResult r = handler(cmd);
    if (r.status == arcraven::ugv::CommandStatus::None) {
        r.status = arcraven::ugv::CommandStatus::Received;
    }
    return std::make_pair(std::move(cmd), std::move(r));
}

size_t CommandRouter::queued() const {
    std::lock_guard<std::mutex> lk(mu_);
    return q_.size();
}

} // namespace arcraven::ugv
