#pragma once

#include <atomic>
#include <chrono>

#include <RE/C/ConsoleLog.h>

#include "AutoBindingsFile.h"
#include "OnActorLocationChangeEventSink.h"
#include "OnCellFullyLoadedEventSink.h"
#include "OnObjectLoadedEventSink.h"
#include "PapyrusScriptBindings.h"

using namespace std::chrono_literals;
using namespace RE::BSScript;
using namespace RE::BSScript::Internal;

namespace ScriptsWithoutESP {

    class System {
        std::atomic<bool> _loaded;

        // [Generic Forms]
        // This is taken directly from the AutoBindings and maps to whatever is defined in the file.
        // The collections BELOW are inferred from these raw form bindings.
        std::unordered_map<RE::FormID, std::string> _formIdsToScriptNames;

        // [Base Forms]
        // Map reference base form IDs --> scripts to attach on object load
        std::unordered_map<RE::FormID, std::string> _baseFormIdsToScriptNames;

        // [Keywords]
        // TODO

        // [Form lists]
        // TODO

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
                    RE::ConsoleLog::GetSingleton()->Print(std::format("ENTRY {}", entry.ScriptName).c_str());
                    ScriptsWithoutESP::PapyrusScriptBindings::Bind(entry);
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
            scriptEvents->AddEventSink<RE::TESObjectLoadedEvent>(new OnObjectLoadedEventSink([](const RE::TESObjectLoadedEvent* event){
                auto* form = RE::TESForm::LookupByID(event->formID);
                auto* ref = form->AsReference();
                if (ref) {
                    auto* baseForm = ref->GetBaseObject();
                    RE::ConsoleLog::GetSingleton()->Print(std::format("Object Loaded {}", baseForm->GetName()).c_str());
                }
            }));
            SKSE::GetMessagingInterface()->RegisterListener([](SKSE::MessagingInterface::Message* message){
                if (message->type == SKSE::MessagingInterface::kNewGame || message->type == SKSE::MessagingInterface::kPostLoadGame) {
                    System::GetSingleton().Reload();
                }
            });
        }
    };
}
