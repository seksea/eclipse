#include "log.hpp"
#include "../interfaces.hpp"
#include <stdarg.h>
#include <string>

void Log::log(LogLevel level, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    char buf[512];
    vsnprintf(buf, sizeof(buf), fmt, args);
    switch (level) {
        case LOG: {
            fputs("\e[32m[LOG] ", stdout); 
            if (Interfaces::cvar)
                Interfaces::cvar->ConsoleColorPrintf({0, 255, 0, 255}, "[LOG] %s\n", buf);
            break;
        }
        case WARN: {
            fputs("\e[33m[WARN] ", stdout); 
            if (Interfaces::cvar)
                Interfaces::cvar->ConsoleColorPrintf({255, 255, 0, 255}, "[WARN] %s\n", buf);
            break;
        }
        case ERR: {
            fputs("\e[31m[ERR] ", stdout); 
            if (Interfaces::cvar)
                Interfaces::cvar->ConsoleColorPrintf({255, 0, 0, 255}, "[ERR] %s\n", buf);
            break;
        }
    }
    fputs(buf, stdout); 
    fputs("\e[0m\n", stdout);
    va_end(args);
}