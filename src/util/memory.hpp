#pragma once
#include <stdint.h>
#include <cstddef>
#include <string.h>
#include <link.h>
#include <iterator>
#include <sstream>
#include <vector>
#include "log.hpp"

namespace VirtualMethod
{
    template <typename T, int index, typename ...Args>
    constexpr T call(void* classBase, Args... args) noexcept
    {
        return (*reinterpret_cast<T(***)(void*, Args...)>(classBase))[index](classBase, args...);
    }
}

#define VFUNC(returnType, name, idx, args, argsRaw) \
returnType name args noexcept \
{ \
    return VirtualMethod::call<returnType, idx>argsRaw; \
}

#define VFUNC_V(returnType, name, idx, args, argsRaw) VIRTUAL_METHOD(returnType, name, idx + 1, args, argsRaw)


namespace Memory {
    namespace VMT {
        void* hook(void* instance, void* hook, int offset);
        void detour(char* src, char* dest);
    }

    namespace Johannes {
        void* detour(void* original, void* hook, int instructionLength);
    }

    inline void**& getVTable(void* c, size_t offset = 0) {
        return *reinterpret_cast<void***>((size_t)c + offset);
    }

    inline uintptr_t getAbsoluteAddress(uintptr_t ptr, int offset, int size) {
        return ptr + *reinterpret_cast<int32_t*>(ptr + offset) + size;
    };

    template <typename T>
    static constexpr auto relativeToAbsolute(std::uintptr_t address) noexcept {
        return (T)(address + 4 + *reinterpret_cast<std::int32_t*>(address));
    }

    struct DlInfo {
        const char* library = nullptr;
        uintptr_t address = 0;
        size_t size = 0;
    };
    inline std::vector<DlInfo> libraries;
    // taken form aixxe's cstrike-basehook-linux
    inline bool getLibraryInformation(const char* library, uintptr_t* address, size_t* size) {
        if (libraries.size() == 0) {
            dl_iterate_phdr([] (struct dl_phdr_info* info, size_t, void*) {
                DlInfo library_info = {};

                library_info.library = info->dlpi_name;
                library_info.address = info->dlpi_addr + info->dlpi_phdr[0].p_vaddr;
                library_info.size = info->dlpi_phdr[0].p_memsz;

                libraries.push_back(library_info);

                return 0;
            }, nullptr);
        }

        for (const DlInfo& current: libraries) {
            if (!strcasestr(current.library, library))
                continue;

            if (address)
                *address = current.address;

            if (size)
                *size = current.size;

            return true;
        }

        return false;
    }

    inline std::vector<std::pair<uint8_t, bool>> getPatternData(const std::string &pattern) {
        std::istringstream iss(pattern);
        std::vector<std::pair<uint8_t, bool>> data;

        for (auto &it : std::vector<std::string>{ std::istream_iterator<std::string>(iss), std::istream_iterator<std::string>() }) {
            data.emplace_back(std::strtol(it.c_str(), nullptr, 16), it == "?" || it == "??");
        }

        return data;
    }

    inline bool compareBytes(uintptr_t addr, const std::vector<std::pair<uint8_t, bool>> &patternData) {
        for (size_t i = 0; i < patternData.size(); i++) {
            const auto data = patternData.at(i);

            if (data.second) {
                continue;
            }

            if (*(uint8_t*)(addr + i) != data.first) {
                return false;
            }
        }

        return true;
    }

    inline std::vector<uintptr_t> findMatches(const std::string &pattern, uintptr_t addr, size_t size) {
        const auto patternData = getPatternData(pattern);
        const auto firstByte = patternData.front();

        if (firstByte.second) {
            throw std::invalid_argument(R"(First pattern byte can't be '?' or '??')");
        }

        if (size < patternData.size()) {
            throw std::invalid_argument("Pattern size can't be greater than scan size");
        }

        std::vector<uintptr_t> data;

        for (size_t i = 0; i <= size - patternData.size(); i++) {
            if (*(uint8_t *)(addr + i) == firstByte.first && compareBytes(addr + i, patternData)) {
                data.emplace_back(addr + i);
            }
        }

        return data;
    }

    inline std::vector<uintptr_t> findMatchesInModule(const char* moduleName, const std::string &pattern) {
        uintptr_t baseAddress;
        size_t memSize;

        if (!getLibraryInformation(moduleName, &baseAddress, &memSize)){
            ERR("Could Not Get info for Module %s", moduleName);
            return {0};
        }

        return findMatches(pattern, baseAddress, memSize);
    }

    inline uintptr_t patternScan(const char* moduleName, const std::string &pattern) {
        const auto matches = findMatchesInModule(moduleName, pattern);

        if (matches.empty()) {
            return 0;
        }

        return matches.front();
    }
}