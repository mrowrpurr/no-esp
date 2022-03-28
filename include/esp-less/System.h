#pragma once

#include <atomic>
#include <chrono>

#include <RE/C/ConsoleLog.h>

#include "AutoBindingsFile.h"
#include "OnActorLocationChangeEventSink.h"
#include "OnCellFullyLoadedEventSink.h"
#include "PapyrusScriptBindings.h"

using namespace std::chrono_literals;
using namespace RE::BSScript;
using namespace RE::BSScript::Internal;

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
                AutoBindingsFile::Read([](const BindingDefinition& entry){
                    ESPLess::PapyrusScriptBindings::Bind(entry);
                });
            }
        }

        void Reload() {
            _loaded = false;
            Load();
        }

        static void ListenForEvents() {
            auto* scriptEvents = RE::ScriptEventSourceHolder::GetSingleton();
            scriptEvents->AddEventSink<RE::TESActorLocationChangeEvent>(new OnActorLocationChangeEventSink([](const RE::TESActorLocationChangeEvent* event){
                if (event->actor->formID == 20) { // The player reference
                    System::GetSingleton().Reload();
                }
            }));
            scriptEvents->AddEventSink<RE::TESCellFullyLoadedEvent>(new OnCellFullyLoadedEventSink([](const RE::TESCellFullyLoadedEvent* event){
                System::GetSingleton().Reload();
            }));
            SKSE::GetMessagingInterface()->RegisterListener([](SKSE::MessagingInterface::Message* message){
                if (message->type == SKSE::MessagingInterface::kNewGame || message->type == SKSE::MessagingInterface::kPostLoadGame) {
                    System::GetSingleton().Reload();
                }
            });
        }
    };
}
