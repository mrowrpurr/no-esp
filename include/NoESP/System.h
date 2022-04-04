#pragma once

#include <atomic>
#include <chrono>
#include <regex>
#include <utility>
#include <vector>

#include <RE/C/ConsoleLog.h>
#include <RE/T/TES.h>
#include <RE/T/TESDataHandler.h>
#include <RE/T/TESObjectREFR.h>
#include <RE/U/UI.h>
#include <REL/Relocation.h>

#include "Log.h"
#include "AutoBindingsFile.h"
#include "Events/OnActorLocationChangeEventSink.h"
#include "Events/OnMenuOpenCloseEventSink.h"
#include "PapyrusScriptBindings.h"
#include "Utilities.h"

using namespace std::chrono_literals;

using namespace SKSE::stl;
using namespace REL;
using namespace RE::BSScript;
using namespace RE::BSScript::Internal;

// TODO: split into some lovely organized files <3

// I want this file under like 50 LOC plz.

namespace NoESP {

    // From Base Object Swapper - play around, understand it, rewrite it in my style.
    // https://github.com/powerof3/BaseObjectSwapper/blob/f636b8cc7079ddebd63eb761e99af95b29884ccc/src/PCH.h#L17-L27
    template <class F, class T>
    void vfunc() {
        REL::Relocation<std::uintptr_t> vtable{ F::VTABLE[0] };
        T::func = vtable.write_vfunc(T::size, T::thunk);
    }

    class System {

        // Whether or not the system has loaded for the current game - specifically, looking for game references.
        std::atomic<bool> _loaded = false;

        // Because I'm still learning locking...
        // This silly bool just used for only enumerating ALL objects in the game once at a time
        // TODO: add locking to the function below for each form when it modifies it (by attaching script)
        std::atomic<bool> _lookingAtAllScripts = false;

        // Cache of scripts which have been linked (by .pex name)
        std::unordered_map<std::string, bool> _linkedScriptsWorkOK;

        // [Generic Forms]
        // This is taken directly from the AutoBindings and maps to whatever is defined in the file.
        // The collections BELOW are inferred from these raw form bindings.
        std::unordered_map<RE::FormID, std::set<std::string>> _formIdsToScriptNames;

        // [Base Forms]
        // Map reference base form IDs --> scripts to attach on object load
        std::unordered_map<RE::FormID, std::set<std::string>> _baseFormIdsToScriptNames;

        // [Keywords]
        std::unordered_map<RE::BGSKeyword*, std::set<std::string>> _keywordIdsToScriptNames;

        // [Form lists]
        std::unordered_map<RE::BGSListForm*, std::set<std::string>> _formListIdsToScriptNames;

        System() = default;

    public:
        System(const System&) = delete;
        System &operator=(const System&) = delete;
        static System& GetSingleton() {
            static System system;
            return system;
        }

        bool IsLoaded() { return _loaded; }
        bool IsLoadedOrSetLoaded() { return _loaded.exchange(true); }
        void SetLoaded(bool value = true) { _loaded = value; }

        bool IsLookingAtAllScriptsOrSet() { return _lookingAtAllScripts.exchange(true); }
        void SetLookingAtAllScripts(bool value = true) { _lookingAtAllScripts = value; }

        void SetScriptLinkWorkedOK(const std::string& scriptName, bool value) { _linkedScriptsWorkOK.try_emplace(scriptName, value); }
        bool HasScriptBeenLinked(const std::string& scriptName) { return _linkedScriptsWorkOK.contains(scriptName); }
        bool ScriptLinkWasSuccessful(const std::string& scriptName) { return _linkedScriptsWorkOK[scriptName]; }
        bool TryLinkScript(const std::string& scriptName) {
            if (HasScriptBeenLinked(scriptName)) return ScriptLinkWasSuccessful(scriptName);
            auto* vm = VirtualMachine::GetSingleton();
             try {
                 vm->linker.Process(scriptName); // This returns a bool, do we want it? What does it return for already loaded scripts, true or false?
                 SetScriptLinkWorkedOK(scriptName, true);
                 return true;
             } catch (...) {
                 SetScriptLinkWorkedOK(scriptName, false);
                 return false;
             }
        }

        void AddFormIdForScript(RE::FormID formId, const std::string& scriptName) {
            if (_formIdsToScriptNames.contains(formId)) {
                _formIdsToScriptNames[formId].insert(scriptName);
            } else {
                std::set<std::string> scriptNames{scriptName};
                _formIdsToScriptNames.try_emplace(formId, scriptNames);
            }
        }
        void AddBaseFormIdForScript(RE::FormID formId, const std::string& scriptName) {
            if (_baseFormIdsToScriptNames.contains(formId)) {
                _baseFormIdsToScriptNames[formId].insert(scriptName);
            } else {
                std::set<std::string> scriptNames{scriptName};
                _baseFormIdsToScriptNames.try_emplace(formId, scriptNames);
            }
        }
        void AddKeywordIdForScript(RE::BGSKeyword* keyword, const std::string& scriptName) {
            if (_keywordIdsToScriptNames.contains(keyword)) {
                _keywordIdsToScriptNames[keyword].insert(scriptName);
            } else {
                std::set<std::string> scriptNames{scriptName};
                _keywordIdsToScriptNames.try_emplace(keyword, scriptNames);
            }
        }
        void AddFormListIdForScript(RE::BGSListForm* formList, const std::string& scriptName) {
            if (_formListIdsToScriptNames.contains(formList)) {
                _formListIdsToScriptNames[formList].insert(scriptName);
            } else {
                std::set<std::string> scriptNames{scriptName};
                _formListIdsToScriptNames.try_emplace(formList, scriptNames);
            }
        }

        void BindFormIdsToScripts() {
            Log("Binding Form IDs to Scripts");
            for (const auto& [formId, scriptNames] : _formIdsToScriptNames) {
                for (const auto& scriptName : scriptNames) {
                    if (TryLinkScript(scriptName)) {
                        PapyrusScriptBindings::BindToFormId(scriptName, formId);
                    } else {
                        Log("Failed to link script '{}' to bind form 0x{:x} to", scriptName, formId);
                    }
                }
            }
        }

        std::set<std::string>& ScriptsForBaseForm(RE::FormID baseFormId) { return _baseFormIdsToScriptNames[baseFormId]; }
        std::unordered_map<RE::BGSKeyword*, std::set<std::string>>& GetScriptNamesForKeywords() { return _keywordIdsToScriptNames; }
        std::unordered_map<RE::BGSListForm*, std::set<std::string>>& GetScriptNamesForFormLists() { return _formListIdsToScriptNames; }

        // You gurl, move this to a better place!
        static bool DoesEditorIdMatch(const EditorIdMatcher& matcher, const std::string& editorIdText) {
            if (editorIdText.empty()) return false;
            std::string editorId = Utilities::ToLowerCase(editorIdText);
            switch (matcher.Type) {
                case EditorIdMatcherType::Exact:
                    return editorId == matcher.Text;
                case EditorIdMatcherType::PrefixMatch:
                    return editorId.starts_with(matcher.Text);
                case EditorIdMatcherType::SuffixMatch:
                    return editorId.ends_with(matcher.Text);
                case EditorIdMatcherType::PrefixAndSuffixMatch:
                    return editorId.find(matcher.Text) != std::string::npos;
                case EditorIdMatcherType::RegularExpression:
                    return std::regex_match(editorId, matcher.RegularExpression);
                default:
                    return false;
            }
        }

        static void TryBindReferencePointer(RE::TESObjectREFR* ref) {
            if (ref->IsDeleted()) return;

            std::set<std::string> scriptsToBind;
            auto& system = System::GetSingleton();
            auto* baseForm = ref->GetBaseObject();

            // Check 3 things...
            // 1: BaseForm
            for (const auto& scriptName : system.ScriptsForBaseForm(baseForm->formID)) {
                scriptsToBind.insert(scriptName);
            }
            // 2: Keywords
            for (const auto& [keyword, scriptNames] : system.GetScriptNamesForKeywords()) {
                if (ref->HasKeyword(keyword)) {
                    for (const auto& scriptName : scriptNames) {
                        scriptsToBind.insert(scriptName);
                    }
                }
            }
            // 3: FormList presence
            for (const auto& [formList, scriptNames] : system.GetScriptNamesForFormLists()) {
                if (formList->HasForm(ref) || formList->HasForm(baseForm)) {
                    for (const auto& scriptName : scriptNames) {
                        scriptsToBind.insert(scriptName);
                    }
                }
            }

            // Bind the scripts!
            for (const auto& scriptName : scriptsToBind) {
                system.TryLinkScript(scriptName);
                PapyrusScriptBindings::BindToFormPointer(scriptName, ref, true);
            }
        }

        static void TryBindReference(RE::TESObjectREFR& ref) {
            if (ref.IsDeleted()) return;

            std::set<std::string> scriptsToBind;
            auto& system = System::GetSingleton();
            auto* baseForm = ref.GetBaseObject();

            // Check 3 things...
            // 1: BaseForm
            for (const auto& scriptName : system.ScriptsForBaseForm(baseForm->formID)) {
                scriptsToBind.insert(scriptName);
            }
            // 2: Keywords
            for (const auto& [keyword, scriptNames] : system.GetScriptNamesForKeywords()) {
                if (ref.HasKeyword(keyword)) {
                    for (const auto& scriptName : scriptNames) {
                        scriptsToBind.insert(scriptName);
                    }
                }
            }
            // 3: FormList presence
            for (const auto& [formList, scriptNames] : system.GetScriptNamesForFormLists()) {
                if (formList->HasForm(&ref) || formList->HasForm(baseForm)) {
                    for (const auto& scriptName : scriptNames) {
                        scriptsToBind.insert(scriptName);
                    }
                }
            }

            // Bind the scripts!
            for (const auto& scriptName : scriptsToBind) {
                system.TryLinkScript(scriptName);
                PapyrusScriptBindings::BindToForm(scriptName, ref, true);
            }
        }

        struct OnObjectInitialization {
            static void thunk(RE::TESObjectREFR* ref) {
                TryBindReferencePointer(ref);
                func(ref);
            }
            static inline REL::Relocation<decltype(thunk)> func;
            static inline constexpr std::size_t size = 0x13;
        };

        static void ListenForReferences() {
            vfunc<RE::TESObjectREFR, OnObjectInitialization>();
        }

        static void CheckForObjectsToAttachScriptsToFromLiterallyEveryFormInTheGame() {
            auto& system = System::GetSingleton();
            if (system.IsLookingAtAllScriptsOrSet()) {
                Log("Already checking every object reference in the game, skipping search.");
                return;
            } else {
                Log("Checking every object reference in the game for whether a script should be attached...");
                const auto& [literallyEveryFormInTheGame, lock] = RE::TESForm::GetAllForms();
                for (auto iterator = literallyEveryFormInTheGame->begin(); iterator != literallyEveryFormInTheGame->end(); iterator++) {
                    auto* ref = iterator->second->AsReference();
                    if (ref) TryBindReferencePointer(ref);
                }
                system.SetLookingAtAllScripts(false);
            }
        }

        static void CheckForObjectsToAttachScriptsToForObjectsInRange(RE::TESObjectREFR* center, float radius) {
            RE::TES::GetSingleton()->ForEachReferenceInRange(center, radius, [](RE::TESObjectREFR& ref){
                TryBindReference(ref);
                return true;
            });
        }

        static void CheckForObjectsToAttachScriptsToForObjectsInRangeOfPlayer(float radius) {
            CheckForObjectsToAttachScriptsToForObjectsInRange(RE::PlayerCharacter::GetSingleton(), radius);
        }

        static void ListenForFirstLocationLoad() {
            auto* scriptEvents = RE::ScriptEventSourceHolder::GetSingleton();
            scriptEvents->AddEventSink<RE::TESActorLocationChangeEvent>(new OnActorLocationChangeEventSink([](const RE::TESActorLocationChangeEvent* event){
                if (event->actor->formID == 20) { // The player reference
                    auto& system = System::GetSingleton();
                    if (! system.IsLoadedOrSetLoaded()) {
                        Log("[coc] Binding declared forms/references to Scripts");
                        system.BindFormIdsToScripts();
                        if (Config::SearchObjectReferencesOnStart) {
                            Log("[coc] Search all game references to attach scripts");
                            System::CheckForObjectsToAttachScriptsToFromLiterallyEveryFormInTheGame();
                        } else {
                            Log("[coc] Did not search all game references to attach scripts. Disabled via .ini");
                        }
                    }
                }
            }));
        }

        // Set the system back to unloaded when the Main Menu is opened
        static void ListenForMenuOpenClose() {
            RE::UI::GetSingleton()->AddEventSink(
                    new OnMenuOpenCloseEventSink([](const RE::MenuOpenCloseEvent* event){
                        if (event->opening && event->menuName == "Main Menu") {
                            System::GetSingleton().SetLoaded(false);
                            Log("Detected Main Menu. New game or Load game or COC will apply scripts for any matching ObjectReferences in the game.");
                        }
                    })
            );
        }

        static void SetupFormBindings(RE::TESForm* form, const std::string& scriptName) {
            auto& system = System::GetSingleton();
            if (form) {
                system.AddFormIdForScript(form->formID, scriptName);
                if (! form->AsReference()) {
                    if (form->GetFormType() == RE::FormType::Keyword) {
                        system.AddKeywordIdForScript(form->As<RE::BGSKeyword>(), scriptName);
                    } else if (form->GetFormType() == RE::FormType::FormList) {
                        system.AddFormListIdForScript(form->As<RE::BGSListForm>(), scriptName);
                    } else {
                        system.AddBaseFormIdForScript(form->formID, scriptName);
                    }
                }
            }
        }

        static void ReadAutoBindingsFiles() {
            auto& system = System::GetSingleton();
            std::vector<std::pair<EditorIdMatcher, std::string>> editorIdMatcherPairs;
            AutoBindingsFile::Read([&system, &editorIdMatcherPairs](const BindingDefinition& entry){
                RE::TESForm* form = nullptr;
                if (entry.Type == BindingDefinitionType::FormID && entry.Plugin.empty()) {
                    form = RE::TESForm::LookupByID(entry.FormID);
                    if (! form) Log("({}:{}) Form not found: '{:x}'", entry.Filename, entry.ScriptName, entry.FormID);
                } else if (entry.Type == BindingDefinitionType::FormID && ! entry.Plugin.empty()) {
                    form = RE::TESDataHandler::GetSingleton()->LookupForm(entry.FormID, entry.Plugin);
                    if (!form) Log("({}:{}) Form not found from plugin '{}': '{:x}'", entry.Filename, entry.ScriptName, entry.Plugin, entry.FormID);
                } else if (entry.Type == BindingDefinitionType::EditorID) {
                    if (entry.EditorIdMatcher.Type == EditorIdMatcherType::Exact) {
                        form = RE::TESForm::LookupByEditorID(entry.EditorIdMatcher.Text);
                        if (! form) Log("({}:{}) Form not found by editor ID: '{}'", entry.Filename, entry.ScriptName, entry.EditorIdMatcher.Text);
                    } else {
                        std::pair<EditorIdMatcher, std::string> pair{entry.EditorIdMatcher, entry.ScriptName};
                        editorIdMatcherPairs.emplace_back(pair);
                    }
                }
                if (form) SetupFormBindings(form, entry.ScriptName);
            });
            if (! editorIdMatcherPairs.empty()) {
                const auto& [map, lock] = RE::TESForm::GetAllFormsByEditorID();
                // O(log(n)) - Hooray!
                for (auto iterator = map->begin(); iterator != map->end(); iterator++) {
                    for (const auto& [matcher, scriptName] : editorIdMatcherPairs) {
                        if (DoesEditorIdMatch(matcher, iterator->first.c_str())) {
                            SetupFormBindings(iterator->second, scriptName);
                        }
                    }
                }
            }
        }

        static void Load() {
            ReadAutoBindingsFiles();
            ListenForReferences();
            ListenForMenuOpenClose();
            ListenForFirstLocationLoad();
        }
    };
}
