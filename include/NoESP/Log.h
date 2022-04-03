#pragma once

#include <format>
#include <string_view>
#include <RE/C/ConsoleLog.h>
#include <spdlog/sinks/basic_file_sink.h>

#include "Config.h"

// TODO: .ini file
namespace NoESP {

    // TODO try getting rid of this section:
    namespace Logging {
        void Initialize() {
            auto path = SKSE::log::log_directory();
            if (!path) {
                SKSE::stl::report_and_fail("Failed to find standard logging directory");
            }

            *path /= "no-esp.log";
            auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true);
            const auto level = spdlog::level::info;

            auto log = std::make_shared<spdlog::logger>("global log", std::move(sink));
            log->set_level(level);
            log->flush_on(level);

            spdlog::set_default_logger(std::move(log));
            spdlog::set_pattern("%g(%#): [%^%l%$] %v");
        }
    };

    template <class... Types>
    void Log(const std::string text, const Types&... args) {
        SKSE::log::info(std::format(text, args...));
        if (NoESP::Config::LogToConsole) {
            RE::ConsoleLog::GetSingleton()->Print(std::format(std::format("[NoESP] {}", text), args...).c_str());
        }
    };
}
