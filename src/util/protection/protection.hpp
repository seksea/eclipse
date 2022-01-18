#pragma once

namespace Protection {
    namespace LoginInfo {
        char* username;
        char* passwd;
        int hwid;
        unsigned int token = 0;
    }

    unsigned int login(const char* username, const char* passwd); // returns login token (0 if login incorrect)
    void protect();
}
