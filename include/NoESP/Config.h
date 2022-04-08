#pragma once

#include <chrono>
#include <string>
#include <filesystem>
#include <SimpleIni.h>

namespace NoESP::Config {
    bool LogToConsole = false;
    bool SearchObjectReferencesOnStart = true;
    bool LogObjectSearch = false;

    void LoadFromIni(const std::function<void(int searchIndex, double radius, long interval)>& objectSearchConfigEntryCallback) {
        auto iniPath = std::filesystem::current_path() / "Data" / "SKSE" / "Plugins" / "no-esp.ini";
        if (std::filesystem::is_regular_file(iniPath)) {
            try {
                CSimpleIni ini;
                auto loadError = ini.LoadFile(iniPath.string().c_str());
                if (loadError == SI_Error::SI_OK) {
                    // [Logging] bLogToConsole
                    LogToConsole = ini.GetBoolValue(L"Logging", L"bLogToConsole", false);
                    if (LogToConsole) {
                        RE::ConsoleLog::GetSingleton()->Print("Console logging enabled.");
                    }
                    // [Logging] bLogObjectSearch
                    LogObjectSearch = ini.GetBoolValue(L"Logging", L"bLogObjectSearch", false);
                    // [Bindings] bSearchReferencesOnLoad
                    SearchObjectReferencesOnStart = ini.GetBoolValue(L"Bindings", L"bSearchReferencesOnLoad", false);
                    // [ObjectSearch] fSearch1Radius iSearchIntervalMs
                    int i = 1;
                    while (ini.GetValue(L"ObjectSearch", std::format(L"fSearch{}Radius", i).c_str())) {
                        double radius = ini.GetDoubleValue(L"ObjectSearch", std::format(L"fSearch{}Radius", i).c_str(), 0);
                        long interval = ini.GetLongValue(L"ObjectSearch", std::format(L"iSearch{}IntervalMs", i).c_str(), 0);
                        std::thread t([i, radius, interval, objectSearchConfigEntryCallback](){
                            objectSearchConfigEntryCallback(i, radius, interval);
                        });
                        t.detach();
                        i++;
                    }
                } else {
                    SKSE::log::info(std::format("Failed to parse .ini {}", iniPath.string()));
                }
            } catch (...) {
                 SKSE::log::info(std::format("Failed to load .ini {}", iniPath.string()));
            }
        }
    }
}
