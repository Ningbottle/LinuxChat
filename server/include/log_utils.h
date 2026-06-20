#pragma once
// log_utils.h — Shared logging utilities for LinuxChat server
//
// Thread-safe timestamp formatting. Replaces the 4 duplicate now_stamp()
// implementations that used std::localtime() (not thread-safe).
// Uses stack-allocated buffer (snprintf) instead of heap-allocated string
// (ostringstream) for minimal allocation overhead in hot logging paths.

#include <chrono>
#include <ctime>
#include <cstdio>
#include <string>

/// Returns current time as "YYYY-MM-DD HH:MM:SS.mmm" for log lines.
/// Uses localtime_r() for thread safety. Stack-allocated buffer — no heap alloc.
inline std::string now_stamp() {
    auto now = std::chrono::system_clock::now();
    auto t   = std::chrono::system_clock::to_time_t(now);
    auto ms  = std::chrono::duration_cast<std::chrono::milliseconds>(
                   now.time_since_epoch()) % 1000;
    struct tm tm_buf;
    localtime_r(&t, &tm_buf);
    char buf[24]; // "YYYY-MM-DD HH:MM:SS.mmm" = 23 chars + null
    std::snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d:%02d.%03d",
                  tm_buf.tm_year + 1900, tm_buf.tm_mon + 1, tm_buf.tm_mday,
                  tm_buf.tm_hour, tm_buf.tm_min, tm_buf.tm_sec,
                  static_cast<int>(ms.count()));
    return std::string(buf);
}
