#pragma once
#include <map>
#include <utility>
#include <string>

#include "../interfaces.hpp"

namespace Netvars {
    inline std::map<std::pair<std::string_view, std::string_view>, std::pair<RecvProp*, uintptr_t>> netvars;
    void init();
}

#define NETVAR(table, var, name, type) type& name() {return *reinterpret_cast<type*>(uintptr_t(this) + Netvars::netvars[std::pair<std::string_view, std::string_view>(table, var)].second);}