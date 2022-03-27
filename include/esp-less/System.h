#pragma once

#include <atomic>
#include <chrono>

#include <RE/C/ConsoleLog.h>

#include "AutoBindingsFile.h"
#include "OnActorLocationChangeEventSink.h"
#include "OnCellFullyLoadedEventSink.h"
#include "PapyrusScriptBindings.h"
#include "SaveGameData.h"

using namespace std::chrono_literals;
using namespace RE::BSScript;
using namespace RE::BSScript::Internal;

namespace ESPLess {

    class System {
        std::atomic<bool> _loaded;
        SaveGameData _saveGameData;
        System() = default;

    public:
        System(const System&) = delete;
        System &operator=(const System&) = delete;
        static System& GetSingleton() {
            static System system;
            return system;
        }
        SaveGameData& GetSaveGameData() { return _saveGameData; }

        void Load() {
            if (! _loaded.exchange(true)) {
                AutoBindingsFile::Read([](const BindingDefinition& entry){
                    auto& saveGameData = System::GetSingleton().GetSaveGameData();
                    if (!saveGameData.LinkedScripts.contains(entry.ScriptName)) {
                        saveGameData.LinkedScripts.insert(entry.ScriptName);
                         auto* vm = VirtualMachine::GetSingleton();
                         try {
                             vm->linker.Process(entry.ScriptName);
                         } catch (...) {
                             RE::ConsoleLog::GetSingleton()->Print(std::format("[Bindings] Failed to link script", entry.ScriptName).c_str());
                         }
                    }
                    if (entry.AddOnce && entry.ID.empty()) {
                        ESPLess::PapyrusScriptBindings::Bind(entry);
                    } else {
                        if (saveGameData.CompletedAutoBindings.contains(entry.Filename)) {
                            if (! saveGameData.CompletedAutoBindings[entry.Filename].contains(entry.ID)) {
                                saveGameData.CompletedAutoBindings[entry.Filename].insert(entry.ID);
                                ESPLess::PapyrusScriptBindings::Bind(entry);
                            }
                        } else {
                            std::set<std::string> idsCompletedForThisFile{entry.ID};
                            std::unordered_map<std::string, std::set<std::string>> completedBindingsForThisFile;
                            completedBindingsForThisFile.insert_or_assign(entry.Filename, idsCompletedForThisFile);
                            ESPLess::PapyrusScriptBindings::Bind(entry);
                        }
                    }
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
                    RE::ConsoleLog::GetSingleton()->Print(std::format("Player new location '{}'", event->newLoc->GetName()).c_str());
                    System::GetSingleton().Reload();
                }
            }));
            scriptEvents->AddEventSink<RE::TESCellFullyLoadedEvent>(new OnCellFullyLoadedEventSink([](const RE::TESCellFullyLoadedEvent* event){
                RE::ConsoleLog::GetSingleton()->Print(std::format("Cell loaded {}", event->cell->GetName()).c_str());
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
