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
            puts("\e[32m[LOG] "); 
            if (Interfaces::cvar)
                Interfaces::cvar->ConsoleColorPrintf({0, 255, 0, 255}, "[LOG] %s\n", buf);
            break;
        }
        case WARN: {
            puts("\e[33m[WARN] "); 
            if (Interfaces::cvar)
                Interfaces::cvar->ConsoleColorPrintf({255, 255, 0, 255}, "[WARN] %s\n", buf);
            break;
        }
        case ERR: {
            puts("\e[31m[ERR] "); 
            if (Interfaces::cvar)
                Interfaces::cvar->ConsoleColorPrintf({255, 0, 0, 255}, "[ERR] %s\n", buf);
            break;
        }
    }
    puts(buf); 
    puts("\e[0m\n");
    va_end(args);
}