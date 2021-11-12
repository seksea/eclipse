#define LOG(str, ...) Log::log(Log::LOG, str, ##__VA_ARGS__)
#define WARN(str, ...) Log::log(Log::WARN, str, ##__VA_ARGS__)
#define ERR(str, ...) Log::log(Log::ERR, str, ##__VA_ARGS__)

namespace Log {
    enum LogLevel {
        LOG,
        WARN,
        ERR
    };
    void log(LogLevel level, const char* fmt, ...);
}