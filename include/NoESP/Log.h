#pragma once

#include <format>
#include <string_view>
#include <RE/C/ConsoleLog.h>

// TODO: .ini file
namespace NoESP {

    template <class... Types>
    void Log(const std::string_view text, const Types&... args) {
        RE::ConsoleLog::GetSingleton()->Print(std::format(std::format("[NoESP] {}", text), args...).c_str());
    }
}
