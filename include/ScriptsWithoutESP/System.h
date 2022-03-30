#pragma once

#include <atomic>
#include <chrono>

#include <RE/C/ConsoleLog.h>
#include <RE/T/TESDataHandler.h>
#include <RE/T/TESObjectREFR.h>
#include <REL/Relocation.h>

#include "Log.h"
#include "AutoBindingsFile.h"
#include "OnActorLocationChangeEventSink.h"
#include "PapyrusScriptBindings.h"

using namespace ScriptsWithoutESP;
using namespace std::chrono_literals;

using namespace SKSE::stl;
using namespace REL;
using namespace RE::BSScript;
using namespace RE::BSScript::Internal;

namespace ScriptsWithoutESP {

    // From Base Object Swapper - play around, understand it, rewrite it in my style.
    // https://github.com/powerof3/BaseObjectSwapper/blob/f636b8cc7079ddebd63eb761e99af95b29884ccc/src/PCH.h#L17-L27
    template <class F, class T>
    void vfunc() {
        REL::Relocation<std::uintptr_t> vtable{ F::VTABLE[0] };
        T::func = vtable.write_vfunc(T::size, T::thunk);
    }

    class System {

        std::atomic<bool> _loaded = false;

        // [Generic Forms]
        // This is taken directly from the AutoBindings and maps to whatever is defined in the file.
        // The collections BELOW are inferred from these raw form bindings.
        std::unordered_map<RE::FormID, std::string> _formIdsToScriptNames;

        // [Base Forms]
        // Map reference base form IDs --> scripts to attach on object load
        std::unordered_map<RE::FormID, std::string> _baseFormIdsToScriptNames;

        // [Keywords]
        std::unordered_map<RE::FormID, std::string> _keywordIdsToScriptNames;

        // [Form lists]
        std::unordered_map<RE::FormID, std::string> _formListIdsToScriptNames;

        System() = default;

    public:
        System(const System&) = delete;
        System &operator=(const System&) = delete;
        static System& GetSingleton() {
            static System system;
            return system;
        }

        bool IsLoadedOrSetLoaded() { return _loaded.exchange(true); }
        void SetLoaded(bool value = true) { _loaded = value; }

        void AddFormIdForScript(RE::FormID formId, const std::string& scriptName) { _formIdsToScriptNames.try_emplace(formId, scriptName); }
        void AddBaseFormIdForScript(RE::FormID formId, const std::string& scriptName) { _baseFormIdsToScriptNames.try_emplace(formId, scriptName); }
        void AddKeywordIdForScript(RE::FormID formId, const std::string& scriptName) { _keywordIdsToScriptNames.try_emplace(formId, scriptName); }
        void AddFormListIdForScript(RE::FormID formId, const std::string& scriptName) { _formListIdsToScriptNames.try_emplace(formId, scriptName); }

        void BindFormIdsToScripts() {
            for (const auto& [formId, scriptName] : _formIdsToScriptNames) {
                PapyrusScriptBindings::BindToFormId(scriptName, formId);
            }
        }

        bool IsBaseFormRegisteredWithScript(RE::FormID baseFormId) { return _baseFormIdsToScriptNames.contains(baseFormId); }
        std::string ScriptForBaseForm(RE::FormID baseFormId) { return _baseFormIdsToScriptNames[baseFormId]; }

        static void TryBindReference(RE::TESObjectREFR* ref) {
            // Check 3 things...
            // 1: BaseForm
            // 2: Keywords
            // 3: FormList presence
            auto& system = System::GetSingleton();
            auto* baseForm = ref->GetBaseObject();
            if (system.IsBaseFormRegisteredWithScript(baseForm->formID)) {
                auto scriptName = system.ScriptForBaseForm(baseForm->formID);
                PapyrusScriptBindings::BindToFormId(scriptName, ref->formID, "", true);
            }
            // else --> TODO
        }

        struct OnObjectInitialization {
            static void thunk(RE::TESObjectREFR* ref) {
                TryBindReference(ref);
                func(ref);
            }
            static inline REL::Relocation<decltype(thunk)> func;
            static inline constexpr std::size_t size = 0x13;
        };

        static void ListenForReferences() {
            vfunc<RE::TESObjectREFR, OnObjectInitialization>();
        }

        static void CheckForObjectsToAttachScriptsToFromLiterallyEveryFormInTheGame() {
            auto* console = RE::ConsoleLog::GetSingleton();
            auto& system = System::GetSingleton();
            const auto& [literallyEveryFormInTheGame, lock] = RE::TESForm::GetAllForms();
            for (auto iterator = literallyEveryFormInTheGame->begin(); iterator != literallyEveryFormInTheGame->end(); iterator++) {
                auto* ref = iterator->second->AsReference();
                if (ref) TryBindReference(ref);
            }
        }

        static void ListenForFirstLocationLoad() {
            auto* scriptEvents = RE::ScriptEventSourceHolder::GetSingleton();
            scriptEvents->AddEventSink<RE::TESActorLocationChangeEvent>(new OnActorLocationChangeEventSink([](const RE::TESActorLocationChangeEvent* event){
                if (event->actor->formID == 20) { // The player reference
                    auto& system = System::GetSingleton();
                    if (! system.IsLoadedOrSetLoaded()) {
                        System::GetSingleton().BindFormIdsToScripts();
                        // Make this something you must turn ON in an .ini off by default:
                        System::CheckForObjectsToAttachScriptsToFromLiterallyEveryFormInTheGame();
                    }
                }
            }));
        }

        static void ReadAutoBindingsFiles() {
            AutoBindingsFile::Read([](const BindingDefinition& entry){
                RE::TESForm* form = nullptr;
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
                        if (form->GetFormType() == RE::FormType::Keyword) {
                            system.AddKeywordIdForScript(form->formID, entry.ScriptName);
                        } else if (form->GetFormType() == RE::FormType::FormList) {
                            system.AddFormListIdForScript(form->formID, entry.ScriptName);
                        } else {
                            system.AddBaseFormIdForScript(form->formID, entry.ScriptName);
                        }
                    }
                }
            });
        }
    };
}
