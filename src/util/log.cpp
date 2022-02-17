#include "log.hpp"
#include "../interfaces.hpp"
#include <stdarg.h>
#include <string>
#include <mutex>

namespace Log {
    void init() {
        LOG(R"a(
           _ _                               _    __ 
  ___  ___| (_)_ __  ___  ___      __      _| |_ / _|
 / _ \/ __| | | '_ \/ __|/ _ \     \ \ /\ / / __| |_ 
|  __/ (__| | | |_) \__ \  __/  _   \ V  V /| |_|  _|
 \___|\___|_|_| .__/|___/\___| (_)   \_/\_/  \__|_|  
              |_|
======================================================
[LOG] loading eclipse...
[LOG] Initialising interfaces...)a");
        LOG(" VEngineCvar (VEngineCvar007) %lx", (uintptr_t)Interfaces::cvar);
    }

    std::mutex logLock;
    void log(LogLevel level, const char* fmt, ...) {
        std::lock_guard<std::mutex> lock(logLock);
        
        va_list args;
        va_start(args, fmt);
        char buf[5000];
        vsnprintf(buf, sizeof(buf), fmt, args);

        switch (level) {
            case LOG: {
                fputs("\e[32m[LOG] ", stdout); 
                if (Interfaces::cvar) {
                    Interfaces::cvar->ConsoleColorPrintf({0, 255, 0, 255}, "[LOG] %s\n", buf);
                }
                break;
            }
            case WARN: {
                fputs("\e[33m[WARN] ", stdout); 
                if (Interfaces::cvar) {
                    Interfaces::cvar->ConsoleColorPrintf({255, 255, 0, 255}, "[WARN] %s\n", buf);
                }
                break;
            }
            case ERR: {
                fputs("\e[31m[ERR] ", stdout); 
                if (Interfaces::cvar) {
                    Interfaces::cvar->ConsoleColorPrintf({255, 0, 0, 255}, "[ERR] %s\n", buf);
                }
                break;
            }
            case INFO: {
                return;
            }
        }
        fputs(buf, stdout); 
        fputs("\e[0m\n", stdout);
        va_end(args);
    }
}