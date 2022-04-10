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
#include "AutoBindingsParser.h"
#include "Events/OnActorLocationChangeEventSink.h"
#include "Events/OnMenuOpenCloseEventSink.h"
#include "PapyrusScriptBindings.h"
#include "ScriptPropertyTypeCache.h"
#include "Utilities.h"
#include "Config.h"

using namespace std::chrono_literals;

using namespace SKSE::stl;
using namespace REL;
using namespace RE::BSScript;
using namespace RE::BSScript::Internal;

// TODO: split into some lovely organized files <3

// I want this file under like 50 LOC plz.

namespace NoESP {

    // Move this into its own file :)
    // From Base Object Swapper:
    // https://github.com/powerof3/BaseObjectSwapper/blob/f636b8cc7079ddebd63eb761e99af95b29884ccc/src/PCH.h#L17-L27
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
        std::unordered_map<RE::FormID, std::unordered_map<std::string, FormPropertyMap>> _formIdsToScriptNames;

        // [Base Forms]
        // Map reference base form IDs --> scripts to attach on object load
        std::unordered_map<RE::FormID, std::unordered_map<std::string, FormPropertyMap>> _baseFormIdsToScriptNames;

        // [Keywords]
        std::unordered_map<RE::BGSKeyword*, std::unordered_map<std::string, FormPropertyMap>> _keywordIdsToScriptNames;

        // [Form lists]
        std::unordered_map<RE::BGSListForm*, std::unordered_map<std::string, FormPropertyMap>> _formListIdsToScriptNames;

        // [Form Types]
        std::unordered_map<RE::FormType, std::unordered_map<std::string, FormPropertyMap>> _formTypesToScriptNames;

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

        static void MergeProperties(FormPropertyMap& baseMap, FormPropertyMap& overrideMap) {
            for (const auto& [propertyName, propertyValue] : overrideMap) {
                baseMap.insert_or_assign(propertyName, propertyValue);
            }
        }

        void AddFormIdForScript(RE::FormID formId, const std::string& scriptName, FormPropertyMap& propertiesToSet) {
            if (_formIdsToScriptNames.contains(formId)) {
                auto& map = _formIdsToScriptNames[formId];
                if (map.contains(scriptName)) {
                    auto& existingPropertiesForScript = map[scriptName];
                    MergeProperties(existingPropertiesForScript, propertiesToSet);
                } else {
                    map.insert_or_assign(scriptName, propertiesToSet);
                }
            } else {
                std::unordered_map<std::string, FormPropertyMap> scriptNameWithProperties;
                scriptNameWithProperties.insert_or_assign(scriptName, propertiesToSet);
                _formIdsToScriptNames.insert_or_assign(formId, scriptNameWithProperties);
            }
        }
        void AddBaseFormIdForScript(RE::FormID formId, const std::string& scriptName, FormPropertyMap& propertiesToSet) {
            if (_baseFormIdsToScriptNames.contains(formId)) {
                auto& map = _baseFormIdsToScriptNames[formId];
                if (map.contains(scriptName)) {
                    auto& existingPropertiesForScript = map[scriptName];
                    MergeProperties(existingPropertiesForScript, propertiesToSet);
                } else {
                    map.insert_or_assign(scriptName, propertiesToSet);
                }
            } else {
                std::unordered_map<std::string, FormPropertyMap> scriptNameWithProperties;
                scriptNameWithProperties.insert_or_assign(scriptName, propertiesToSet);
                _baseFormIdsToScriptNames.insert_or_assign(formId, scriptNameWithProperties);
            }
        }

        void AddKeywordIdForScript(RE::BGSKeyword* keyword, const std::string& scriptName, FormPropertyMap& propertiesToSet) {
            if (_keywordIdsToScriptNames.contains(keyword)) {
                auto& map = _keywordIdsToScriptNames[keyword];
                if (map.contains(scriptName)) {
                    auto& existingPropertiesForScript = map[scriptName];
                    MergeProperties(existingPropertiesForScript, propertiesToSet);
                } else {
                    map.insert_or_assign(scriptName, propertiesToSet);
                }
            } else {
                std::unordered_map<std::string, FormPropertyMap> scriptNameWithProperties;
                scriptNameWithProperties.insert_or_assign(scriptName, propertiesToSet);
                _keywordIdsToScriptNames.insert_or_assign(keyword, scriptNameWithProperties);
            }
        }

        void AddFormListIdForScript(RE::BGSListForm* formList, const std::string& scriptName, FormPropertyMap& propertiesToSet) {
            if (_formListIdsToScriptNames.contains(formList)) {
                auto& map = _formListIdsToScriptNames[formList];
                if (map.contains(scriptName)) {
                    auto& existingPropertiesForScript = map[scriptName];
                    MergeProperties(existingPropertiesForScript, propertiesToSet);
                } else {
                    map.insert_or_assign(scriptName, propertiesToSet);
                }
            } else {
                std::unordered_map<std::string, FormPropertyMap> scriptNameWithProperties;
                scriptNameWithProperties.insert_or_assign(scriptName, propertiesToSet);
                _formListIdsToScriptNames.insert_or_assign(formList, scriptNameWithProperties);
            }
        }

        void AddFormTypeForScript(RE::FormType formType, const std::string& scriptName, FormPropertyMap& propertiesToSet) {
            if (_formTypesToScriptNames.contains(formType)) {
                auto& map = _formTypesToScriptNames[formType];
                if (map.contains(scriptName)) {
                    auto& existingPropertiesForScript = map[scriptName];
                    MergeProperties(existingPropertiesForScript, propertiesToSet);
                } else {
                    map.insert_or_assign(scriptName, propertiesToSet);
                }
            } else {
                std::unordered_map<std::string, FormPropertyMap> scriptNameWithProperties;
                scriptNameWithProperties.insert_or_assign(scriptName, propertiesToSet);
                _formTypesToScriptNames.insert_or_assign(formType, scriptNameWithProperties);
            }
        }

        std::unordered_map<RE::FormID, std::unordered_map<std::string, FormPropertyMap>>& GetScriptsForDirectlyReferencedForms() { return _formIdsToScriptNames; }
        std::unordered_map<std::string, FormPropertyMap>& ScriptsForBaseForm(RE::FormID baseFormId) { return _baseFormIdsToScriptNames[baseFormId]; }
        std::unordered_map<std::string, FormPropertyMap>& GetScriptNamesForKeyword(RE::BGSKeyword* keyword) { return _keywordIdsToScriptNames[keyword]; }
        std::unordered_map<RE::BGSKeyword*, std::unordered_map<std::string, FormPropertyMap>>& GetScriptNamesForKeywords() { return _keywordIdsToScriptNames; }
        std::unordered_map<std::string, FormPropertyMap>& GetScriptNamesForFormList(RE::BGSListForm* formList) { return _formListIdsToScriptNames[formList]; }
        std::unordered_map<RE::BGSListForm*, std::unordered_map<std::string, FormPropertyMap>>& GetScriptNamesForFormLists() { return _formListIdsToScriptNames; }
        std::unordered_map<std::string, FormPropertyMap>& GetScriptNamesForFormType(RE::FormType formType) { return _formTypesToScriptNames[formType]; }
        std::unordered_map<RE::FormType, std::unordered_map<std::string, FormPropertyMap>>& GetScriptNamesForFormTypes() { return _formTypesToScriptNames; }

        void BindFormIdsToScripts() {
            auto& bindingsForForms = GetScriptsForDirectlyReferencedForms();
            for (auto& [formId, scriptNamesAndPropertyMaps] : bindingsForForms) {
                for (auto& [scriptName, propertyMap] : scriptNamesAndPropertyMaps) {
                    if (TryLinkScript(scriptName)) {
                        PapyrusScriptBindings::BindToFormId(scriptName, formId, propertyMap);
                    } else {
                        Log("Failed to link script '{}' to bind form 0x{:x} to", scriptName, formId);
                    }
                }
            }
        }

        static void TryBindReferencePointer(RE::TESObjectREFR* ref) {
            if (ref->IsDeleted()) return;

            std::unordered_map<std::string, FormPropertyMap> scriptsToBindWithProperties;
            auto& system = System::GetSingleton();
            auto* baseForm = ref->GetBaseObject();

            // Check 3 things...
            // 1: BaseForm
            for (auto& [scriptName, propertiesToSet] : system.ScriptsForBaseForm(baseForm->formID)) {
                if (scriptsToBindWithProperties.contains(scriptName)) {
                    auto& existingPropertyValues = scriptsToBindWithProperties[scriptName];
                    MergeProperties(existingPropertyValues, propertiesToSet);
                } else {
                    scriptsToBindWithProperties.insert_or_assign(scriptName, propertiesToSet);
                }
            }

            // 2: Keywords
            for (const auto& [keyword, scriptNames] : system.GetScriptNamesForKeywords()) {
                if (ref->HasKeyword(keyword)) {
                    for (auto& [scriptName, propertiesToSet] : system.GetScriptNamesForKeyword(keyword)) {
                        if (scriptsToBindWithProperties.contains(scriptName)) {
                            auto& existingPropertyValues = scriptsToBindWithProperties[scriptName];
                            MergeProperties(existingPropertyValues, propertiesToSet);
                        } else {
                            scriptsToBindWithProperties.insert_or_assign(scriptName, propertiesToSet);
                        }
                    }
                }
            }

            // 3: FormList presence
            for (const auto& [formList, scriptNames] : system.GetScriptNamesForFormLists()) {
                if (formList->HasForm(ref) || formList->HasForm(baseForm)) {
                    for (auto& [scriptName, propertiesToSet] : system.GetScriptNamesForFormList(formList)) {
                        if (scriptsToBindWithProperties.contains(scriptName)) {
                            auto& existingPropertyValues = scriptsToBindWithProperties[scriptName];
                            MergeProperties(existingPropertyValues, propertiesToSet);
                        } else {
                            scriptsToBindWithProperties.insert_or_assign(scriptName, propertiesToSet);
                        }
                    }
                }
            }

            // Bind the scripts!
            for (auto& [scriptName, propertiesToSet] : scriptsToBindWithProperties) {
                system.TryLinkScript(scriptName);
                PapyrusScriptBindings::BindToFormPointer(scriptName, ref, propertiesToSet, true);
            }
        }

        static void TryBindReference(RE::TESObjectREFR& ref) {
            if (ref.IsDeleted()) return;

            std::unordered_map<std::string, FormPropertyMap> scriptsToBindWithProperties;
            auto& system = System::GetSingleton();
            auto* baseForm = ref.GetBaseObject();

            // Check 3 things...
            // 1: BaseForm
            for (auto& [scriptName, propertiesToSet] : system.ScriptsForBaseForm(baseForm->formID)) {
                if (scriptsToBindWithProperties.contains(scriptName)) {
                    auto& existingPropertyValues = scriptsToBindWithProperties[scriptName];
                    MergeProperties(existingPropertyValues, propertiesToSet);
                } else {
                    scriptsToBindWithProperties.insert_or_assign(scriptName, propertiesToSet);
                }
            }

            // 2: Keywords
            for (const auto& [keyword, scriptNames] : system.GetScriptNamesForKeywords()) {
                if (ref.HasKeyword(keyword)) {
                    for (auto& [scriptName, propertiesToSet] : system.GetScriptNamesForKeyword(keyword)) {
                        if (scriptsToBindWithProperties.contains(scriptName)) {
                            auto& existingPropertyValues = scriptsToBindWithProperties[scriptName];
                            MergeProperties(existingPropertyValues, propertiesToSet);
                        } else {
                            scriptsToBindWithProperties.insert_or_assign(scriptName, propertiesToSet);
                        }
                    }
                }
            }

            // 3: FormList presence
            for (const auto& [formList, scriptNames] : system.GetScriptNamesForFormLists()) {
                if (formList->HasForm(&ref) || formList->HasForm(baseForm)) {
                    for (auto& [scriptName, propertiesToSet] : system.GetScriptNamesForFormList(formList)) {
                        if (scriptsToBindWithProperties.contains(scriptName)) {
                            auto& existingPropertyValues = scriptsToBindWithProperties[scriptName];
                            MergeProperties(existingPropertyValues, propertiesToSet);
                        } else {
                            scriptsToBindWithProperties.insert_or_assign(scriptName, propertiesToSet);
                        }
                    }
                }
            }

            // Bind the scripts!
            for (auto& [scriptName, propertiesToSet] : scriptsToBindWithProperties) {
                system.TryLinkScript(scriptName);
                PapyrusScriptBindings::BindToForm(scriptName, ref, propertiesToSet, true);
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

        static void SetupFormBindings(RE::TESForm* form, const std::string& scriptName, BindingDefinition& def) {
            auto& system = System::GetSingleton();
            if (form) {
                Log("[Form Binding] Form:{:x} '{}' Script:{}", form->formID, form->GetName(), scriptName);
                system.AddFormIdForScript(form->formID, scriptName, def.PropertyValues);

                if (! form->AsReference()) {
                    // system.AddFormTypeForScript(form->GetFormType(), scriptName, def.PropertyValues);
                    if (form->GetFormType() == RE::FormType::Keyword) {
                        Log("[Form Binding] Keyword:{} Script:{}", form->GetName(), scriptName);
                        system.AddKeywordIdForScript(form->As<RE::BGSKeyword>(), scriptName, def.PropertyValues);
                    } else if (form->GetFormType() == RE::FormType::FormList) {
                        Log("[Form Binding] FormList:{} Script:{}", form->GetName(), scriptName);
                        system.AddFormListIdForScript(form->As<RE::BGSListForm>(), scriptName, def.PropertyValues);
                    } else {
                        Log("[Form Binding] BaseForm:{:x} '{}' Script:{}", form->formID, form->GetName(), scriptName);
                        system.AddBaseFormIdForScript(form->formID, scriptName, def.PropertyValues);
                    }
                }
            }
        }

        static void ReadAutoBindingsFiles() {
            auto& system = System::GetSingleton();
            std::vector<std::tuple<EditorIdMatcher, std::string, BindingDefinition>> editorIdMatchers;

            AutoBindingsFile::Read([&system, &editorIdMatchers](BindingDefinition& entry){
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
                        std::tuple<EditorIdMatcher, std::string, BindingDefinition> values{entry.EditorIdMatcher, entry.ScriptName, entry};
                        editorIdMatchers.emplace_back(values);
                    }
                }
                if (form) SetupFormBindings(form, entry.ScriptName, entry);
            });

            // TODO loop over all thingies and ...

            // If any scrips want to match on editor ID, run all of those matchers!
            if (! editorIdMatchers.empty()) {
                const auto& [map, lock] = RE::TESForm::GetAllFormsByEditorID();
                // O(log(n)) - Hooray! Can't use an inner O(1) lookup because it hast to use text matching / regex for each editor ID to see if it matches
                for (auto iterator = map->begin(); iterator != map->end(); iterator++) {
                    for (auto& [matcher, scriptName, def] : editorIdMatchers) {
                        if (DoesEditorIdMatch(matcher, iterator->first.c_str())) {
                            SetupFormBindings(iterator->second, scriptName, def);
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
