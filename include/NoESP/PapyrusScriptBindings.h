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

    void AutoFillProperties(const RE::BSTSmartPointer<RE::BSScript::Object>& object, FormPropertyMap& manuallyConfiguredProperties) {
//        if (true) return; // Come back to this...

        auto* typeInfo = object->GetTypeInfo();
        auto* properties = typeInfo->GetPropertyIter();
        for (uint32_t i = 0; i < typeInfo->propertyCount; i++) {
            auto propertyName = properties[i].name;

            if (manuallyConfiguredProperties.contains(Utilities::ToLowerCase(propertyName.c_str()))) {
                Log("Not using autofill for property {} because it is manually configured", propertyName.c_str());
                continue;
            }

            Log("Getting type info about property '{}'", propertyName.c_str());
            auto* propertyTypeInfo = properties[i].info.type.GetTypeInfo();
            auto type = propertyTypeInfo->GetRawType();
            if (type == TypeInfo::RawType::kObject) {
                Log("Property {} is OBJECT", propertyName.c_str());
                Log("The property object is of type {}", propertyTypeInfo->name.c_str());
            } else if (type == TypeInfo::RawType::kString) {
                Log("Property {} is String", propertyName.c_str());
            } else if (type == TypeInfo::RawType::kInt) {
                Log("Property {} is Int", propertyName.c_str());
            } else {
                auto typeId = (size_t) type;
                Log("Dunno prop type {}? Let's see here...", typeId);
                Log("Dunno prop type NAME {}? Let's see here...", propertyName.c_str());
                Log("The property object is of type {}", propertyTypeInfo->name.c_str());
            }

            auto typeName = propertyTypeInfo->name.c_str();

            auto* propertyVariable = object->GetProperty(propertyName);
            if (propertyVariable->IsObject()) {
                auto* form = RE::TESForm::LookupByEditorID(propertyName);
                if (form) {
                    auto* vm = VirtualMachine::GetSingleton();
                    auto* handlePolicy = vm->GetObjectHandlePolicy();
                    RE::VMHandle handle = handlePolicy->GetHandleForObject(form->GetFormType(), form);
                    RE::BSTSmartPointer<RE::BSScript::Object> objectPtr;
                    Log("Creating a {} for property {}", typeName, propertyName.c_str());
                    vm->CreateObject(typeName, objectPtr);
                    auto* bindPolicy = vm->GetObjectBindPolicy();
                    bindPolicy->BindObject(objectPtr, handle);
                    propertyVariable->SetObject(objectPtr);
                } else {
                    // TODO - support all other property types! the primitives!
                    Log("Autofill only supports object properties, no primitives (int, string, etc) right now...");
                }
            }
        }
    }

    void SetProperties(const RE::BSTSmartPointer<RE::BSScript::Object>& object, FormPropertyMap& propertyMap) {
        for (const auto& [propertyName, propertyValue] : propertyMap) {
            try {
                auto* property = object->GetProperty(propertyName);
                if (property) {
                    if (property->IsString()) {
                        Log("{} is a string!", propertyName.c_str());
                        property->SetString(propertyValue.PropertyValueText);
                    } else {
                        Log("Unsupported property type for {}", propertyName.c_str());
                    }
                } else {
                    Log("Property not found {}", propertyName.c_str());
                }
            } catch (...) {
                Log("Error setting property {}", propertyName.c_str());
            }
        }
    }

    void BindToForm(std::string scriptName, const RE::TESForm& form, FormPropertyMap& propertiesToSet, bool addOnce = false) {
        // Gross! Move this to a BindingDefinition field! Such hack.
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
                                Log("{} already attached to 0x{:x}, skipping", scriptName, form.formID);
                                return; // Don't bind! Already bound!
                            }
                        }
                    }
                }

                RE::BSTSmartPointer<RE::BSScript::Object> objectPtr;
                vm->CreateObject(scriptName, objectPtr);
                auto* bindPolicy = vm->GetObjectBindPolicy();

                if (autoFillProperties) AutoFillProperties(objectPtr, propertiesToSet);
                SetProperties(objectPtr, propertiesToSet);

                try {
                    bindPolicy->BindObject(objectPtr, handle);
                } catch (...) {
                    Log("Failed to bind object to handle for '{}'", scriptName);
                    return;
                }

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

    void BindToFormPointer(std::string scriptName, RE::TESForm* form, FormPropertyMap& propertiesToSet, bool addOnce = false) {
        if (! form) return;

        // Gross! Move this to a BindingDefinition field! Such hack.
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
                                Log("{} already attached to 0x{:x}, skipping", scriptName, form->formID);
                                return; // Don't bind! Already bound!
                            }
                        }
                    }
                }

                RE::BSTSmartPointer<RE::BSScript::Object> objectPtr;
                vm->CreateObject(scriptName, objectPtr);
                auto* bindPolicy = vm->GetObjectBindPolicy();

                if (autoFillProperties) AutoFillProperties(objectPtr, propertiesToSet);
                SetProperties(objectPtr, propertiesToSet);

                try {
                    bindPolicy->BindObject(objectPtr, handle);
                } catch (...) {
                    Log("Failed to bind object to handle for '{}'", scriptName);
                    return;
                }

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

    void BindToEditorId(const std::string& scriptName, const std::string& editorId, FormPropertyMap& propertiesToSet, bool addOnce = false) {
        auto* form = RE::TESForm::LookupByEditorID(editorId);
        if (form) {
            BindToFormPointer(scriptName, form, propertiesToSet, addOnce);
        } else {
            Log("Could not find Form via Editor ID: '{}' for script '{}'", editorId, scriptName);
        }
    }

    void BindToFormId(const std::string& scriptName, RE::FormID formId, FormPropertyMap& propertiesToSet, const std::string optionalPluginFile = "", bool addOnce = false) {
        Log("Bind script '{}' to form ID 0x{:x}", scriptName, formId);
        if (optionalPluginFile.empty()) {
            auto* form = RE::TESForm::LookupByID(formId);
            if (form) {
                BindToFormPointer(scriptName, form, propertiesToSet, addOnce);
            } else {
                Log("Could not find Form via Form ID: '{}' for script '{}'", formId, scriptName);
            }
        } else {
            auto* dataHandler = RE::TESDataHandler::GetSingleton();
            if (dataHandler->GetModIndex(optionalPluginFile) != 255) {
                auto* form = dataHandler->LookupForm(formId, optionalPluginFile);
                if (form) {
                    BindToFormPointer(scriptName, form, propertiesToSet, addOnce);
                } else {
                    Log("Could not find Form via Form ID: '{}' in plugin '{}' for script '{}'", formId, optionalPluginFile, scriptName);
                }
            } else {
                Log("Could not find plugin '{}' for script '{}'", optionalPluginFile, scriptName);
            }
        }
    }

    void Bind(BindingDefinition& def) {
        try {
            if (def.Type == BindingDefinitionType::EditorID && def.EditorIdMatcher.Type == EditorIdMatcherType::Exact) {
                BindToEditorId(def.ScriptName, def.EditorIdMatcher.Text, def.PropertyValues, def.AddOnce);
            } else if (def.Type == BindingDefinitionType::FormID) {
                BindToFormId(def.ScriptName, def.FormID, def.PropertyValues, def.Plugin, def.AddOnce);
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