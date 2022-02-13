#pragma once
#include <fstream>
#include <filesystem>

#define INFO(str, ...) Log::log(Log::INFO, str, ##__VA_ARGS__)
#define LOG(str, ...) Log::log(Log::LOG, str, ##__VA_ARGS__)
#define WARN(str, ...) Log::log(Log::WARN, str, ##__VA_ARGS__)
#define ERR(str, ...) Log::log(Log::ERR, str, ##__VA_ARGS__)

namespace Log {
    enum LogLevel {
        LOG,
        WARN,
        ERR,
        INFO
    };
    void init();
    void log(LogLevel level, const char* fmt, ...);
}