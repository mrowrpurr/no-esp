#pragma once

#include <chrono>
#include <filesystem>
#include <SimpleIni.h>

namespace NoESP::Config {
    bool LogToConsole = false;
    bool SearchObjectReferencesOnStart = true;
    bool LogObjectSearch = false;

    void LoadFromIni(std::function<void(int searchIndex, double radius, long interval)> objectSearchConfigEntryCallback) {
        auto iniPath = std::filesystem::current_path() / "Data" / "SKSE" / "Plugins" / "no-esp.ini";
        if (std::filesystem::is_regular_file(iniPath)) {
            try {
                CSimpleIni ini;
                auto loadError = ini.LoadFile(iniPath.string().c_str());
                if (loadError == SI_Error::SI_OK) {
                    // [Logging] bLogToConsole
                    LogToConsole = ini.GetBoolValue("Logging", "bLogToConsole", false);
                    // [Logging] bLogObjectSearch
                    LogObjectSearch = ini.GetBoolValue("Logging", "bLogObjectSearch", false);
                    // [Bindings] bSearchReferencesOnLoad
                    SearchObjectReferencesOnStart = ini.GetBoolValue("Bindings", "bSearchReferencesOnLoad", false);
                    // [ObjectSearch] fSearch1Radius iSearchIntervalMs
                    int i = 1;
                    while (ini.GetValue("ObjectSearch", std::format("fSearch{}Radius", i).c_str()) && (!std::string(ini.GetValue("ObjectSearch", std::format("fSearch{}Radius", i).c_str())).empty())) {
                        double radius = ini.GetDoubleValue("ObjectSearch", std::format("fSearch{}Radius", i).c_str(), 0);
                        long interval = ini.GetLongValue("ObjectSearch", std::format("iSearch{}IntervalMs", i).c_str(), 0);
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
