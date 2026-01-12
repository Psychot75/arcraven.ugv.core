#pragma once
#include <cstddef>
#include <cstdint>
#include <deque>
#include <functional>
#include <mutex>
#include <optional>
#include <unordered_map>

#include "command/CommandTypes.hpp"

namespace arcraven::ugv {

// Handler signature: execute should be fast and non-blocking. Heavy work must be staged
// into other subsystems (e.g., set an atomic target for the control thread).
using CommandHandler = std::function<CommandResult(const CommandEnvelope&)>;

struct CommandRouterConfig {
    size_t max_queue = 256;
};

class CommandRouter final {
public:
    explicit CommandRouter(CommandRouterConfig cfg = {});

    // Registration: allows you to "make it possible" to call commands now,
    // while implementing actual logic later.
    void register_handler(arcraven::ugv::UgvCommand cmd, CommandHandler handler);
    bool has_handler(arcraven::ugv::UgvCommand cmd) const;

    // Thread-safe enqueue (typically IO thread).
    // Returns Accepted/Rejected with reason. If accepted, it is queued for processing.
    CommandResult submit(CommandEnvelope cmd);

    // Processing:
    // - can be called from control thread or a dedicated "command thread"
    // - returns an optional (cmd, result) for ack/telemetry
    std::optional<std::pair<CommandEnvelope, CommandResult>> process_one(uint64_t now_ns);

    // Convenience: drain up to N commands.
    template <typename Fn>
    void process_some(uint64_t now_ns, size_t max_n, Fn&& on_processed) {
        for (size_t i = 0; i < max_n; ++i) {
            auto r = process_one(now_ns);
            if (!r) break;
            on_processed(*r);
        }
    }

    size_t queued() const;

private:
    CommandRouterConfig cfg_;

    mutable std::mutex mu_;
    std::deque<CommandEnvelope> q_;
    std::unordered_map<uint16_t, CommandHandler> handlers_;
};

} // namespace arcraven::ugvcore
