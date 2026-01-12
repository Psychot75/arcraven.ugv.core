#include "Logger.hpp"
#include <chrono>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <thread>

namespace arcraven::utils {

static std::unique_ptr<Logger> g_logger;
static std::mutex g_logger_mu;

Logger::Logger(Config cfg)
    : cfg_(std::move(cfg)),
      min_level_(cfg_.min_level)
{
    if (cfg_.file_path.has_value()) {
        ensure_parent_dirs(*cfg_.file_path);
        file_.open(*cfg_.file_path, std::ios::out | std::ios::app);
        if (!file_.is_open()) {
            throw std::runtime_error("Logger: failed to open log file: " + *cfg_.file_path);
        }
    }
}

Logger::~Logger() {
    try {
        flush();
    } catch (...) {
        // never throw from dtor
    }
}

void Logger::set_min_level(Level lvl) {
    min_level_.store(lvl, std::memory_order_relaxed);
}

Logger::Level Logger::min_level() const {
    return min_level_.load(std::memory_order_relaxed);
}

void Logger::log(Level lvl, std::string_view msg, const char* file, int line) {
    if (static_cast<uint8_t>(lvl) < static_cast<uint8_t>(min_level())) {
        return;
    }

    // Atomic-per-line output: build the whole line, then lock once.
    const std::string line_str = format_line(lvl, msg, file, line);

    std::scoped_lock lock(mu_);

    if (cfg_.console) {
        if (cfg_.ansi_colors) {
            std::cerr << level_color(lvl) << line_str << "\033[0m";
        } else {
            std::cerr << line_str;
        }
        if (cfg_.flush_always) std::cerr.flush();
    }

    if (file_.is_open()) {
        file_ << line_str;
        if (cfg_.flush_always) file_.flush();
    }

    // Fatal should not be silently ignored in a robot context.
    if (lvl == Level::Fatal && cfg_.flush_always) {
        flush();
    }
}

void Logger::flush() {
    std::scoped_lock lock(mu_);
    if (cfg_.console) std::cerr.flush();
    if (file_.is_open()) file_.flush();
}

std::string Logger::format_line(Level lvl,
                                std::string_view msg,
                                const char* file,
                                int line) const
{
    std::ostringstream oss;
    oss << now_local_iso8601() << " ";
    oss << "[" << level_tag(lvl) << "] ";

    if (cfg_.include_thread_id) {
        oss << "tid=" << thread_id_string() << " ";
    }

    if (cfg_.include_source && file != nullptr && line > 0) {
        // Keep this short-ish; you can later strip paths in build system if you want.
        oss << file << ":" << line << " ";
    }

    oss << "- " << msg << "\n";
    return oss.str();
}

const char* Logger::level_tag(Level lvl) {
    switch (lvl) {
        case Level::Trace: return "TRACE";
        case Level::Debug: return "DEBUG";
        case Level::Info:  return "INFO ";
        case Level::Warn:  return "WARN ";
        case Level::Error: return "ERROR";
        case Level::Fatal: return "FATAL";
        default:           return "UNKWN";
    }
}

const char* Logger::level_color(Level lvl) {
    // ANSI colors: dim, cyan, green, yellow, red, bold red
    switch (lvl) {
        case Level::Trace: return "\033[2m";
        case Level::Debug: return "\033[36m";
        case Level::Info:  return "\033[32m";
        case Level::Warn:  return "\033[33m";
        case Level::Error: return "\033[31m";
        case Level::Fatal: return "\033[1;31m";
        default:           return "\033[0m";
    }
}

std::string Logger::now_local_iso8601() {
    using clock = std::chrono::system_clock;
    const auto now = clock::now();
    const auto t = clock::to_time_t(now);

    std::tm tm{};
#if defined(_WIN32)
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif

    const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;

    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S")
        << "." << std::setw(3) << std::setfill('0') << ms.count();
    return oss.str();
}

std::string Logger::thread_id_string() {
    std::ostringstream oss;
    oss << std::this_thread::get_id();
    return oss.str();
}

void Logger::ensure_parent_dirs(const std::string& file_path) {
    std::error_code ec;
    const auto p = std::filesystem::path(file_path).parent_path();
    if (!p.empty()) {
        std::filesystem::create_directories(p, ec);
        // If it fails, open() will fail and we throw there; no need to throw twice.
    }
}

// -------------------------
// Global logger API
// -------------------------
void init_logger(Logger::Config cfg) {
    std::scoped_lock lock(g_logger_mu);
    g_logger = std::make_unique<Logger>(std::move(cfg));
}

Logger& logger() {
    std::scoped_lock lock(g_logger_mu);
    if (!g_logger) {
        throw std::runtime_error("Logger: not initialized. Call init_logger() early in main().");
    }
    return *g_logger;
}

bool logger_ready() noexcept {
    std::scoped_lock lock(g_logger_mu);
    return static_cast<bool>(g_logger);
}

void shutdown_logger() noexcept {
    std::scoped_lock lock(g_logger_mu);
    g_logger.reset();
}

} // namespace arcraven::utils
