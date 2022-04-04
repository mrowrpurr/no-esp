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

    void BindToForm(std::string scriptName, const RE::TESForm& form, bool addOnce = false) {
        bool autoFillProperties = true;
        if (scriptName.starts_with('!')) {
            autoFillProperties = false;
            scriptName = scriptName.substr(1); // Remove '!'
        }

        try {
            auto* vm = VirtualMachine::GetSingleton();
            auto* handlePolicy = vm->GetObjectHandlePolicy();
            RE::VMHandle handle = handlePolicy->GetHandleForObject(form.GetFormType(), (RE::TESForm*) &form);
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

                auto* ref = form.AsReference();
                if (ref) {
                    auto* baseForm = ref->GetBaseObject();
                    Log("Bound script '{}' to reference '{}' 0x{:x} (base '{}' 0x{:x})!", scriptName, form.GetName(), form.formID, baseForm->GetName(), baseForm->formID);
                } else {
                    Log("Bound script '{}' to form '{}' 0x{:x}!", scriptName, form.GetName(), form.formID);
                }
            } else {
                Log("Error getting handle for script {} to reference", scriptName);
            }
        } catch (...) {
            Log("Error binding script {} to reference", scriptName);
        }
    }

    void BindToFormPointer(std::string scriptName, RE::TESForm* form, bool addOnce = false) {
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

                auto* ref = form->AsReference();
                if (ref) {
                    auto* baseForm = ref->GetBaseObject();
                    Log("Bound script '{}' to reference '{}' 0x{:x} (base '{}' 0x{:x})!", scriptName, form->GetName(), form->formID, baseForm->GetName(), baseForm->formID);
                } else {
                    Log("Bound script '{}' to form '{}' 0x{:x}!", scriptName, form->GetName(), form->formID);
                }
            } else {
                Log("Error getting handle for script {} to reference", scriptName);
            }
        } catch (...) {
            Log("Error binding script {} to reference", scriptName);
        }
    }

    void BindToEditorId(const std::string& scriptName, const std::string& editorId, bool addOnce = false) {
        auto* form = RE::TESForm::LookupByEditorID(editorId);
        if (form) {
            BindToFormPointer(scriptName, form, addOnce);
        } else {
            Log("Could not find Form via Editor ID: '{}' for script '{}'", editorId, scriptName);
        }
    }

    void BindToFormId(const std::string& scriptName, RE::FormID formId, const std::string optionalPluginFile = "", bool addOnce = false) {
        if (optionalPluginFile.empty()) {
            auto* form = RE::TESForm::LookupByID(formId);
            if (form) {
                BindToFormPointer(scriptName, form, addOnce);
            } else {
                Log("Could not find Form via Form ID: '{}' for script '{}'", formId, scriptName);
            }
        } else {
            auto* dataHandler = RE::TESDataHandler::GetSingleton();
            if (dataHandler->GetModIndex(optionalPluginFile) != 255) {
                auto* form = dataHandler->LookupForm(formId, optionalPluginFile);
                if (form) {
                    BindToFormPointer(scriptName, form, addOnce);
                } else {
                    Log("Could not find Form via Form ID: '{}' in plugin '{}' for script '{}'", formId, optionalPluginFile, scriptName);
                }
            } else {
                Log("Could not find plugin '{}' for script '{}'", optionalPluginFile, scriptName);
            }
        }
    }

    void Bind(const BindingDefinition& def) {
        try {
            if (def.Type == BindingDefinitionType::EditorID && def.EditorIdMatcher.Type == EditorIdMatcherType::Exact) {
                BindToEditorId(def.ScriptName, def.EditorIdMatcher.Text, def.AddOnce);
            } else if (def.Type == BindingDefinitionType::FormID) {
                BindToFormId(def.ScriptName, def.FormID, def.Plugin, def.AddOnce);
            }
        } catch (...) {
            if (def.Filename.empty()) {
                Log("Bind() error {} {} to {}", def.EditorIdMatcher.Text, def.FormID, def.ScriptName);
            } else {
                Log("Bind() error {}", def.Filename);
            }
        }
    }
}