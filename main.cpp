#include <filesystem>
#include <fstream>
#include <string>

#include "config/UgvCore.hpp"
#include "utils/Logger.hpp"

namespace {

struct CliArgs {
    std::filesystem::path data_dir;
};

static CliArgs parse_args(int argc, char** argv) {
    CliArgs a{};
    a.data_dir = std::filesystem::path("/var/lib/arcraven-ugv");

    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--data-dir" && (i + 1) < argc) {
            a.data_dir = std::filesystem::path(argv[++i]);
            continue;
        }
    }

    return a;
}

static std::filesystem::path ensure_writable_data_dir(std::filesystem::path desired) {
    std::error_code ec;
    std::filesystem::create_directories(desired, ec);

    const auto probe = desired / ".probe";
    std::ofstream out(probe.string(), std::ios::binary | std::ios::trunc);
    if (out.good()) {
        out << "ok";
        out.close();
        std::filesystem::remove(probe, ec);
        return desired;
    }

    ARC_LOG_WARN("Data dir not writable: " + desired.string() + " -> falling back to current directory");
    return std::filesystem::current_path();
}

} // namespace

int main(int argc, char** argv) {
    const auto cli = parse_args(argc, argv);

    arcraven::utils::init_logger({
        .min_level = arcraven::utils::Logger::Level::Info,
        .file_path = std::string("robot.log"),
        .console = true,
        .ansi_colors = false,
        .flush_always = true,
        .include_thread_id = true,
        .include_source = true
    });

    ARC_LOG_INFO("UGV init process starting");

    arcraven::ugv::UgvConfig cfg{};
    cfg.data_dir = ensure_writable_data_dir(cli.data_dir);

    ARC_LOG_INFO("Using data dir: " + cfg.data_dir.string());

    arcraven::ugv::UgvCore core(cfg);
    const int rc = core.run();

    ARC_LOG_INFO("UGV core exited with code " + std::to_string(rc));
    arcraven::utils::shutdown_logger();
    return rc;
}
