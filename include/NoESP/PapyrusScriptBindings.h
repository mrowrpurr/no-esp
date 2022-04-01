#pragma once

#pragma warning(push)
#include <SKSE/SKSE.h>
#include <RE/Skyrim.h>
#include <RE/T/TESForm.h>
#include <RE/T/TESObjectREFR.h>
#pragma warning(pop)

#include "BindingDefinition.h"

using namespace NoESP;
using namespace RE::BSScript;
using namespace RE::BSScript::Internal;

namespace NoESP::PapyrusScriptBindings {

    void BindObjectProperties(const RE::BSTSmartPointer<RE::BSScript::Object>& object) {
        auto* typeInfo = object->GetTypeInfo();
        auto* properties = typeInfo->GetPropertyIter();
        for (uint32_t i = 0; i < typeInfo->propertyCount; i++) {
            auto typeName = properties[i].info.type.GetTypeInfo()->GetName();
            auto propertyName = properties[i].name;
            auto* propertyVariable = object->GetProperty(propertyName);
            if (propertyVariable->IsObject()) {
                auto* form = RE::TESForm::LookupByEditorID(propertyName);
                if (form) {
                    auto* vm = VirtualMachine::GetSingleton();
                    auto* handlePolicy = vm->GetObjectHandlePolicy();
                    RE::VMHandle handle = handlePolicy->GetHandleForObject(form->GetFormType(), form);
                    RE::BSTSmartPointer<RE::BSScript::Object> objectPtr;
                    vm->CreateObject(typeName, objectPtr);
                    auto* bindPolicy = vm->GetObjectBindPolicy();
                    bindPolicy->BindObject(objectPtr, handle);
                    propertyVariable->SetObject(objectPtr);
                }
            }
        }
    }

    void BindToForm(std::string scriptName, RE::TESForm* form, bool addOnce = false) {
        if (! form) return;

        bool autoFillProperties = true;
        if (scriptName.starts_with('!')) {
            autoFillProperties = false;
            scriptName = scriptName.substr(1); // Remove '!'
        }

        try {
            auto* vm = VirtualMachine::GetSingleton();
            auto* handlePolicy = vm->GetObjectHandlePolicy();
            RE::VMHandle handle = handlePolicy->GetHandleForObject(form->GetFormType(), form);
            if (handle) {

                // If there is already a script with the same name attached to this object, don't bind a new one
                RE::BSFixedString caseInsensitiveScriptName = scriptName;
                if (addOnce) {
                    if (vm->attachedScripts.contains(handle)) {
                        for (auto& attachedScript : vm->attachedScripts.find(handle)->second) {
                            if (attachedScript->GetTypeInfo()->GetName() == caseInsensitiveScriptName) {
                                return; // Don't bind! Already bound!
                            }
                        }
                    }
                }

                RE::BSTSmartPointer<RE::BSScript::Object> objectPtr;
                vm->CreateObject(scriptName, objectPtr);
                auto* bindPolicy = vm->GetObjectBindPolicy();
                if (autoFillProperties) {
                    BindObjectProperties(objectPtr);
                }
                bindPolicy->BindObject(objectPtr, handle);
                RE::ConsoleLog::GetSingleton()->Print(std::format("[Bindings] Bound script '{}' to reference!", scriptName).c_str());
            } else {
                RE::ConsoleLog::GetSingleton()->Print(std::format("[Bindings] Error getting handle for script {} to reference", scriptName).c_str());
            }
        } catch (...) {
            RE::ConsoleLog::GetSingleton()->Print(std::format("[Bindings] Error binding script {} to reference", scriptName).c_str());
        }
    }

    void BindToEditorId(const std::string& scriptName, const std::string& editorId, bool addOnce = false) {
        auto* form = RE::TESForm::LookupByEditorID(editorId);
        if (form) {
            BindToForm(scriptName, form, addOnce);
        } else {
            RE::ConsoleLog::GetSingleton()->Print(std::format("[Binding] Could not find Form via Editor ID: '{}' for script '{}'", editorId, scriptName).c_str());
        }
    }

    void BindToFormId(const std::string& scriptName, RE::FormID formId, const std::string optionalPluginFile = "", bool addOnce = false) {
        Log("BindToFormId {} {} {}", scriptName, formId, optionalPluginFile);
        if (optionalPluginFile.empty()) {
            auto* form = RE::TESForm::LookupByID(formId);
            if (form) {
                BindToForm(scriptName, form, addOnce);
            } else {
                RE::ConsoleLog::GetSingleton()->Print(std::format("[Binding] Could not find Form via Form ID: '{}' for script '{}'", formId, scriptName).c_str());
            }
        } else {
            auto* dataHandler = RE::TESDataHandler::GetSingleton();
            if (dataHandler->GetModIndex(optionalPluginFile) != 255) {
                auto* form = dataHandler->LookupForm(formId, optionalPluginFile);
                if (form) {
                    BindToForm(scriptName, form, addOnce);
                } else {
                    RE::ConsoleLog::GetSingleton()->Print(std::format("[Binding] Could not find Form via Form ID: '{}' in plugin '{}' for script '{}'", formId, optionalPluginFile, scriptName).c_str());
                }
            } else {
                RE::ConsoleLog::GetSingleton()->Print(std::format("[Binding] Could not find plugin '{}' for script '{}'", optionalPluginFile, scriptName).c_str());
            }
        }
    }

    void Bind(const BindingDefinition& def) {
        try {
            if (def.Type == BindingDefinitionType::EditorID) {
                BindToEditorId(def.ScriptName, def.EditorID, def.AddOnce);
            } else if (def.Type == BindingDefinitionType::FormID) {
                BindToFormId(def.ScriptName, def.FormID, def.Plugin, def.AddOnce);
            }
        } catch (...) {
            if (def.Filename.empty()) {
                RE::ConsoleLog::GetSingleton()->Print(std::format("[Bindings] Bind() error {} {}", def.EditorID, def.FormID).c_str());
            } else {
                RE::ConsoleLog::GetSingleton()->Print(std::format("[AutoBindings] Bind() error {}", def.Filename).c_str());
            }
        }
    }
}