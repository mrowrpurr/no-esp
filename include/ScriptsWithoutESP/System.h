#pragma once

#include <atomic>
#include <chrono>

#include <RE/C/ConsoleLog.h>
#include <RE/T/TESDataHandler.h>

#include "Log.h"
#include "AutoBindingsFile.h"
#include "OnActorLocationChangeEventSink.h"
#include "OnCellFullyLoadedEventSink.h"
#include "OnObjectLoadedEventSink.h"
#include "PapyrusScriptBindings.h"

using namespace ScriptsWithoutESP;
using namespace std::chrono_literals;
using namespace RE::BSScript;
using namespace RE::BSScript::Internal;

namespace ScriptsWithoutESP {

    class System {

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

        void AddFormIdForScript(RE::FormID formId, const std::string& scriptName) {
            _formIdsToScriptNames.try_emplace(formId, scriptName);
        }

        void AddBaseFormIdForScript(RE::FormID formId, const std::string& scriptName) {
            _baseFormIdsToScriptNames.try_emplace(formId, scriptName);
        }

        void BindFormIdsToScripts() {
            Log("BIND FORM IDS to SCRIPTS {}", _formIdsToScriptNames.size());
            for (const auto& [formId, scriptName] : _formIdsToScriptNames) {
                PapyrusScriptBindings::BindToFormId(scriptName, formId);
            }
        }

        static void ListenForEvents() {
//            SKSE::GetMessagingInterface()->RegisterListener([](SKSE::MessagingInterface::Message* message){
//                if (message->type == SKSE::MessagingInterface::kNewGame || message->type == SKSE::MessagingInterface::kPostLoadGame) {
//                    Log("NEW GAME or LOAD GAME");
//                    System::GetSingleton().BindFormIdsToScripts();
//                }
//            });
//            auto* scriptEvents = RE::ScriptEventSourceHolder::GetSingleton();
//            scriptEvents->AddEventSink<RE::TESActorLocationChangeEvent>(new OnActorLocationChangeEventSink([](const RE::TESActorLocationChangeEvent* event){
//                if (event->actor->formID == 20) { // The player reference
//                    System::GetSingleton().Reload();
//                }
//            }));
//            scriptEvents->AddEventSink<RE::TESCellFullyLoadedEvent>(new OnCellFullyLoadedEventSink([](const RE::TESCellFullyLoadedEvent* event){
//                System::GetSingleton().Reload();
//            }));
//            scriptEvents->AddEventSink<RE::TESObjectLoadedEvent>(new OnObjectLoadedEventSink([](const RE::TESObjectLoadedEvent* event){
//                auto* form = RE::TESForm::LookupByID(event->formID);
//                auto* ref = form->AsReference();
//                if (ref) {
//                    auto* baseForm = ref->GetBaseObject();
//                    RE::ConsoleLog::GetSingleton()->Print(std::format("Object Loaded {}", baseForm->GetName()).c_str());
//                }
//            }));
        }

        static void Start() {
            Log("START");
            AutoBindingsFile::Read([](const BindingDefinition& entry){
                Log("ENTRY {} {}", entry.Filename, entry.ScriptName);
                RE::TESForm* form;
                if (entry.Type == BindingDefinitionType::FormID && entry.Plugin.empty()) {
                    form = RE::TESForm::LookupByID(entry.FormID);
                    if (! form) Log("({}:{}) Form not found: '{:x}'", entry.Filename, entry.ScriptName, entry.FormID);
                } else if (entry.Type == BindingDefinitionType::FormID && ! entry.Plugin.empty()) {
                    form = RE::TESDataHandler::GetSingleton()->LookupForm(entry.FormID, entry.Plugin);
                    if (!form) Log("({}:{}) Form not found from plugin '{}': '{:x}'", entry.Filename, entry.ScriptName, entry.Plugin, entry.FormID);
                } else if (entry.Type == BindingDefinitionType::EditorID) {
                    form = RE::TESForm::LookupByEditorID(entry.EditorID);
                    if (! form) Log("({}:{}) Form not found by editor ID: '{}'", entry.Filename, entry.ScriptName, entry.EditorID);
                }
                if (form) {
                    auto& system = System::GetSingleton();
                    system.AddFormIdForScript(form->formID, entry.ScriptName);
                    if (! form->AsReference()) {
                        system.AddBaseFormIdForScript(entry.FormID, entry.ScriptName);
                    }
                }
            });
        }
    };
}
