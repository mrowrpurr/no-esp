#pragma once

#include <atomic>
#include <chrono>

#include <RE/C/ConsoleLog.h>

#include "AutoBindingsFile.h"
#include "OnActorLocationChangeEventSink.h"

using namespace std::chrono_literals;

namespace ESPLess {

    class System {
        std::atomic<bool> _loaded;
        System() = default;

    public:
        System(const System&) = delete;
        System &operator=(const System&) = delete;
        static System& GetSingleton() {
            static System system;
            return system;
        }

        void Load() {
            if (! _loaded.exchange(true)) {
                RE::ConsoleLog::GetSingleton()->Print("READING Data/Scripts/AutoBindings");
                AutoBindingsFile::Read([](const AutoBindingsFile::FileEntry& entry){
                    RE::ConsoleLog::GetSingleton()->Print(std::format("ENTRY for Script {}", entry.ScriptName).c_str());
                });
            }
        }

        void Reload() {
            _loaded = false;
            Load();
        }

        static void ListenForEvents() {
            RE::ScriptEventSourceHolder::GetSingleton()->AddEventSink<RE::TESActorLocationChangeEvent>(new OnActorLocationChangeEventSink([](const RE::TESActorLocationChangeEvent*){
                System::GetSingleton().Load();
            }));
            SKSE::GetMessagingInterface()->RegisterListener([](SKSE::MessagingInterface::Message* message){
                if (message->type == SKSE::MessagingInterface::kNewGame || message->type == SKSE::MessagingInterface::kPostLoadGame) {
                    System::GetSingleton().Reload();
                }
            });
        }
    };
}
