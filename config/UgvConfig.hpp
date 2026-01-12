#pragma once
#include <filesystem>

#include "core/Rate.hpp"

namespace arcraven::ugv {

struct UgvConfig {
    std::filesystem::path data_dir;

    size_t expected_drives = 8;
    size_t expected_cameras = 3;
    size_t max_lidars = 5;

    // Thread rates (tune per platform)
    Rate control_rate{std::chrono::microseconds(5000)};    // 200 Hz
    Rate io_rate{std::chrono::microseconds(20000)};        // 50 Hz
    Rate sensor_rate{std::chrono::microseconds(10000)};    // 100 Hz
    Rate persist_rate{std::chrono::microseconds(1000000)}; // 1 Hz
    Rate estop_rate{std::chrono::microseconds(2000)};      // 500 Hz
};

} // namespace arcraven::ugvcore
