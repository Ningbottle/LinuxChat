#pragma once
// log_utils.h — Shared logging utilities for LinuxChat server
// Uses spdlog for thread-safe, colored console logging.

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

inline void init_logger() {
    // Set global format: [YYYY-MM-DD HH:MM:SS.mmm] [Level] Message
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");
    spdlog::set_level(spdlog::level::info);
}
