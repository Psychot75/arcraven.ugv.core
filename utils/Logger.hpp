#pragma once

#include <atomic>
#include <cstdint>
#include <fstream>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <string_view>

namespace arcraven::utils {

class Logger final {
public:
    enum class Level : uint8_t {
        Trace = 0,
        Debug,
        Info,
        Warn,
        Error,
        Fatal
    };

    struct Config {
        Level min_level = Level::Info;

        // If set, logs will also be appended to this file.
        // Parent directories are created automatically.
        std::optional<std::string> file_path;

        // Console output toggle
        bool console = true;

        // ANSI colors (safe on most Linux terminals; disable for journald/syslog pipelines)
        bool ansi_colors = false;

        // Flush on every message (safer for robots; slightly slower)
        bool flush_always = true;

        // Include extras
        bool include_thread_id = true;
        bool include_source = true; // file:line
    };

    // ---- lifetime ----
    explicit Logger(Config cfg);
    ~Logger();

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    // ---- config ----
    void set_min_level(Level lvl);
    Level min_level() const;

    // ---- logging ----
    void log(Level lvl,
             std::string_view msg,
             const char* file = nullptr,
             int line = 0);

    // Force flush (useful before controlled shutdown)
    void flush();

private:
    Config cfg_;
    std::mutex mu_;
    std::ofstream file_;
    std::atomic<Level> min_level_;

    std::string format_line(Level lvl,
                            std::string_view msg,
                            const char* file,
                            int line) const;

    static const char* level_tag(Level lvl);
    static const char* level_color(Level lvl);
    static std::string now_local_iso8601();
    static std::string thread_id_string();
    static void ensure_parent_dirs(const std::string& file_path);
};

// -------------------------
// Global logger (simple)
// -------------------------
void init_logger(Logger::Config cfg);
Logger& logger();          // throws if not initialized
bool logger_ready() noexcept;
void shutdown_logger() noexcept;

// Convenience macros (source location)
#define ARC_LOG_TRACE(msg) ::arcraven::utils::logger().log(::arcraven::utils::Logger::Level::Trace, (msg), __FILE__, __LINE__)
#define ARC_LOG_DEBUG(msg) ::arcraven::utils::logger().log(::arcraven::utils::Logger::Level::Debug, (msg), __FILE__, __LINE__)
#define ARC_LOG_INFO(msg)  ::arcraven::utils::logger().log(::arcraven::utils::Logger::Level::Info,  (msg), __FILE__, __LINE__)
#define ARC_LOG_WARN(msg)  ::arcraven::utils::logger().log(::arcraven::utils::Logger::Level::Warn,  (msg), __FILE__, __LINE__)
#define ARC_LOG_ERROR(msg) ::arcraven::utils::logger().log(::arcraven::utils::Logger::Level::Error, (msg), __FILE__, __LINE__)
#define ARC_LOG_FATAL(msg) ::arcraven::utils::logger().log(::arcraven::utils::Logger::Level::Fatal, (msg), __FILE__, __LINE__)

} // namespace arcraven::utils
