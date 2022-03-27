#pragma once

#include <RE/T/TESForm.h>
#include <RE/T/TESObjectREFR.h>

#include "BindingDefinition.h"

using namespace RE::BSScript;
using namespace RE::BSScript::Internal;

namespace ESPLess::PapyrusScriptBindings {
    void BindToObjectReference(const std::string& scriptName, RE::TESObjectREFR* ref, bool addOnce = false) {
        try {
            auto* vm = VirtualMachine::GetSingleton();
            auto* handlePolicy = vm->GetObjectHandlePolicy();
            RE::VMHandle handle = handlePolicy->GetHandleForObject(ref->GetFormType(), ref);
            if (handle) {

                // If there is already a script with the same name attached to this object, don't bind a new one
                RE::BSFixedString caseInsensitiveScriptName = scriptName;
                if (addOnce) {
                    if (vm->attachedScripts.contains(handle)) {
                        for (auto& attachedScript : vm->attachedScripts.find(handle)->second) {
                            if (attachedScript->GetTypeInfo()->GetName() == caseInsensitiveScriptName) {
                                RE::ConsoleLog::GetSingleton()->Print(std::format("{} already bound to {}, skipping", scriptName, ref->GetBaseObject()->GetName()).c_str());
                                return; // Don't bind! Already bound!
                            }
                        }
                    }
                }

                RE::BSTSmartPointer<RE::BSScript::Object> objectPtr;
                vm->CreateObject(scriptName, objectPtr);
                auto* bindPolicy = vm->GetObjectBindPolicy();
                bindPolicy->BindObject(objectPtr, handle);
                RE::ConsoleLog::GetSingleton()->Print(std::format("[Bindings] Bound script '{}' to reference!", scriptName).c_str());
            } else {
                RE::ConsoleLog::GetSingleton()->Print(std::format("[Bindings] Error getting handle for script {} to reference", scriptName).c_str());
            }
        } catch (...) {
            RE::ConsoleLog::GetSingleton()->Print(std::format("[Bindings] Error binding script {} to reference", scriptName).c_str());
        }
    }
    void BindToReferencesOfForm(const std::string& scriptName, RE::TESForm* form, bool addOnce = false) {
        auto* ref = form->AsReference();
        if (ref != nullptr) {
            BindToObjectReference(scriptName, ref, addOnce);
        } else {
            RE::ConsoleLog::GetSingleton()->Print(std::format("Form '{}' is not a reference, cannot bind script '{}'", form->GetName(), scriptName).c_str());
        }
    }
    void Bind(const BindingDefinition& def) {
        try {
            if (def.Type == BindingDefinitionType::EditorID) {
                auto* form = RE::TESForm::LookupByEditorID(def.EditorID);
                if (form) {
                    BindToReferencesOfForm(def.ScriptName, form, def.AddOnce);
                } else {
                    RE::ConsoleLog::GetSingleton()->Print(std::format("[Binding] Could not find Form via Editor ID: '{}' for script '{}'", def.EditorID, def.ScriptName).c_str());
                }
            } else if (def.Type == BindingDefinitionType::FormID && def.Plugin.empty()) {
                auto* form = RE::TESForm::LookupByID(def.FormID);
                if (form) {
                    BindToReferencesOfForm(def.ScriptName, form, def.AddOnce);
                } else {
                    RE::ConsoleLog::GetSingleton()->Print(std::format("[Binding] Could not find Form via Form ID: '{}' for script '{}'", def.FormID, def.ScriptName).c_str());
                }
            } else if (def.Type == BindingDefinitionType::FormID && ! def.Plugin.empty()) {
                auto* dataHandler = RE::TESDataHandler::GetSingleton();
                if (dataHandler->GetModIndex(def.Plugin) != 255) {
                    auto* form = dataHandler->LookupForm(def.FormID, def.Plugin);
                    if (form) {
                        BindToReferencesOfForm(def.ScriptName, form, def.AddOnce);
                    } else {
                        RE::ConsoleLog::GetSingleton()->Print(std::format("[Binding] Could not find Form via Form ID: '{}' in plugin '{}' for script '{}'", def.FormID, def.Plugin, def.ScriptName).c_str());
                    }
                } else {
                    RE::ConsoleLog::GetSingleton()->Print(std::format("[Binding] Could not find plugin '{}' for script '{}'", def.Plugin, def.ScriptName).c_str());
                }
            }
        } catch (...) {
            if (def.Filename.empty()) {
                RE::ConsoleLog::GetSingleton()->Print(std::format("[Bindings] Bind() error {} {}", def.EditorID, def.FormID).c_str());
            } else {
                RE::ConsoleLog::GetSingleton()->Print(std::format("[AutoBindings] Bind() error {}:{}", def.Filename, def.ID).c_str());
            }
        }
    }
}