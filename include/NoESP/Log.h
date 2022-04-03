#pragma once

#include <format>
#include <string_view>
#include <RE/C/ConsoleLog.h>
#include <spdlog/sinks/basic_file_sink.h>

// TODO: .ini file
namespace NoESP {

    // TODO try getting rid of this section:
    namespace Logging {
        bool LogToConsole = false;

        void Initialize() {
            auto path = logger::log_directory();
            if (!path) {
                SKSE::stl::report_and_fail("Failed to find standard logging directory"sv);
            }

            *path /= "no-esp.log"sv;
            auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true);
            const auto level = spdlog::level::info;

            auto log = std::make_shared<spdlog::logger>("global log"s, std::move(sink));
            log->set_level(level);
            log->flush_on(level);

            spdlog::set_default_logger(std::move(log));
            spdlog::set_pattern("%g(%#): [%^%l%$] %v"s);
        }
    };

    template <class... Types>
    void Log(const std::string text, const Types&... args) {
        logger::info(std::format(text, args...));
        if (Logging::LogToConsole) {
            RE::ConsoleLog::GetSingleton()->Print(std::format(std::format("[NoESP] {}", text), args...).c_str());
        }
    };
}
