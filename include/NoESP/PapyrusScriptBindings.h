#pragma once

#pragma warning(push)
#include <SKSE/SKSE.h>
#include <RE/Skyrim.h>
#include <RE/T/TESForm.h>
#include <RE/T/TESObjectREFR.h>
#pragma warning(pop)

#include "BindingDefinition.h"
#include "ScriptPropertyTypeCache.h"
#include "Utilities.h"

using namespace NoESP;
using namespace RE::BSScript;
using namespace RE::BSScript::Internal;

namespace NoESP::PapyrusScriptBindings {

    RE::TESForm* LookupForm(const std::string& formAsText) {
        static auto formIdWithPluginNamePattern = std::regex(R"(^\s*0x([abcdefABCDEF0123456789]+)\|(.*)\s*$)");
        static auto formIdPattern = std::regex(R"(^\s*0x([abcdefABCDEF0123456789]+)\s*$)");

        std::smatch matches;
        if (std::regex_search(formAsText, matches, formIdWithPluginNamePattern)) {
            try {
                auto formId = std::stoi(matches[1].str(), nullptr, 16);
                auto pluginFile = matches[2].str();
                return RE::TESDataHandler::GetSingleton()->LookupForm(formId, pluginFile);
            } catch (...) {
                Log("Problem looking up form '{}'", formAsText);
                return nullptr;
            }
        } else if (std::regex_search(formAsText, matches, formIdPattern)) {
            try {
                auto formId = std::stoi(matches[1].str(), nullptr, 16);
                return RE::TESForm::LookupByID(formId);
            } catch (...) {
                Log("Problem looking up form '{}'", formAsText);
                return nullptr;
            }
        } else {
            return RE::TESForm::LookupByEditorID(formAsText);
        }
    }

    void AutoFillProperties(const RE::BSTSmartPointer<RE::BSScript::Object>& object, FormPropertyMap& manuallyConfiguredProperties) {
        auto* vm = VirtualMachine::GetSingleton();
        auto* handlePolicy = vm->GetObjectHandlePolicy();
        auto* typeInfo = object->GetTypeInfo();
        if (typeInfo->propertyCount < 1) return;

        auto* properties = typeInfo->GetPropertyIter();
        for (uint32_t i = 0; i < typeInfo->propertyCount; i++) {
            auto propertyName = properties[i].name;

            auto* form = RE::TESForm::LookupByEditorID(propertyName);
            if (! form) return;

            if (manuallyConfiguredProperties.contains(Utilities::ToLowerCase(propertyName.c_str()))) {
                Log("Not using autofill for property {} because it is manually configured", propertyName.c_str());
                continue;
            }

            auto rawType = properties[i].info.type.GetRawType();
            auto rawTypeId = (size_t) rawType;
            if (rawType == TypeInfo::RawType::kArraysEnd || (rawTypeId >= 0 && rawTypeId <= 15)) {
                Log("Property is primitive type, cannot be auto filled {}", propertyName.c_str());
                return;
            }

            auto* propertyTypeInfo = properties[i].info.type.GetTypeInfo();
            auto typeName = propertyTypeInfo->name.c_str();

            auto* propertyVariable = object->GetProperty(propertyName);
            if (propertyVariable->IsObject()) {
                auto* form = RE::TESForm::LookupByEditorID(propertyName);
                if (form) {
                    RE::VMHandle handle = handlePolicy->GetHandleForObject(form->GetFormType(), form);
                    RE::BSTSmartPointer<RE::BSScript::Object> objectPtr;
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

    void SetProperties(const std::string& scriptName, const RE::BSTSmartPointer<RE::BSScript::Object>& object, FormPropertyMap& propertyMap) {
        auto* vm = VirtualMachine::GetSingleton();
        auto* handlePolicy = vm->GetObjectHandlePolicy();
        auto& propertyTypeCache = ScriptPropertyTypeCache::GetSingleton();
        for (const auto& [propertyName, propertyValue] : propertyMap) {
            try {
                // for const auto propertyName 'someform' propertyValue 'Healing'
                auto propertyType = propertyTypeCache.GetOrLookupScriptPropertyType(scriptName, propertyName);
                if (propertyType.has_value()) {
                    auto *property = object->GetProperty(propertyName);
                    if (property) {
                        TypeInfo::RawType rawType = propertyType.value().RawType.value();
                        auto propertyScriptName = propertyType.value().PropertyScriptName;
                        switch (rawType) {
                            case TypeInfo::RawType::kString:
                                property->SetString(propertyValue.PropertyValueText);
                                break;
                            case TypeInfo::RawType::kInt:
                                property->SetSInt(std::stoi(propertyValue.PropertyValueText));
                                break;
                            case TypeInfo::RawType::kFloat:
                                property->SetFloat(std::stof(propertyValue.PropertyValueText));
                                break;
                            case TypeInfo::RawType::kBool:
                                property->SetBool(Utilities::ToLowerCase(propertyValue.PropertyValueText) == "true");
                                break;
                            default:
                                auto *form = LookupForm(propertyValue.PropertyValueText);
                                if (form) {
                                    auto typeName = propertyType->PropertyScriptName;
                                    if (typeName.empty()) {
                                        Log("Could not get a type name for property {}", propertyName);
                                    } else {
                                        RE::VMHandle handle = handlePolicy->GetHandleForObject(form->GetFormType(), form);
                                        RE::BSTSmartPointer<RE::BSScript::Object> objectPtr;
                                        vm->CreateObject(typeName, objectPtr);
                                        auto *bindPolicy = vm->GetObjectBindPolicy();
                                        bindPolicy->BindObject(objectPtr, handle);
                                        property->SetObject(objectPtr);
                                    }
                                } else {
                                    Log("Property (Form?) could not be found: {}", propertyValue.PropertyValueText);
                                }
                                break;
                        }
                    }
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
                SetProperties(scriptName, objectPtr, propertiesToSet);

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
                SetProperties(scriptName, objectPtr, propertiesToSet);

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