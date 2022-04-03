#pragma once

#include <filesystem>
#include <SimpleIni.h>

namespace NoESP::Config {
    bool LogToConsole = false;
    bool SearchObjectReferencesOnStart = true;

    void LoadFromIni() {
        auto iniPath = std::filesystem::current_path() / "Data" / "SKSE" / "Plugins" / "no-esp.ini";
        if (std::filesystem::is_regular_file(iniPath)) {
            try {
                CSimpleIni ini;
                auto loadError = ini.LoadFile(iniPath.string().c_str());
                if (loadError == SI_Error::SI_OK) {
                    LogToConsole = ini.GetBoolValue("Logging", "bLogToConsole", false);
                    SearchObjectReferencesOnStart = ini.GetBoolValue("Bindings", "bSearchReferencesOnLoad", false);
                } else {
                    logger::info(std::format("Failed to parse .ini {}", iniPath.string()));
                }
            } catch (...) {
                 logger::info(std::format("Failed to load .ini {}", iniPath.string()));
            }
        }
    }
}
