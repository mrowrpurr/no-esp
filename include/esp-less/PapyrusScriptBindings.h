#pragma once

#include <RE/T/TESForm.h>
#include <RE/T/TESObjectREFR.h>

#include "BindingDefinition.h"

using namespace RE::BSScript;
using namespace RE::BSScript::Internal;

namespace ESPLess::PapyrusScriptBindings {

    void BindToObjectReference(const std::string& scriptName, RE::TESObjectREFR* ref) {
        try {
            auto* vm = VirtualMachine::GetSingleton();
            auto* handlePolicy = vm->GetObjectHandlePolicy();
            RE::VMHandle handle = handlePolicy->GetHandleForObject(ref->GetFormType(), ref);
            if (handle) {
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

    void Bind(const BindingDefinition& def) {
        try {
            if (def.Type == BindingDefinitionType::EditorID) {
                auto* form = RE::TESForm::LookupByEditorID(def.EditorID);
                if (form) {
                    auto* ref = form->AsReference();
                    if (ref) {
                        BindToObjectReference(def.ScriptName, ref);
                    } else {
                        RE::ConsoleLog::GetSingleton()->Print(std::format("[Binding] Form not a reference for Editor ID: '{}' for script '{}'", def.EditorID, def.ScriptName).c_str());
                    }
                } else {
                    RE::ConsoleLog::GetSingleton()->Print(std::format("[Binding] Could not find Form via Editor ID: '{}' for script '{}'", def.EditorID, def.ScriptName).c_str());
                }
            } else if (def.Type == BindingDefinitionType::FormID) {
                RE::ConsoleLog::GetSingleton()->Print(std::format("[Binding] UNSUPPORTED: Form ID binding for script '{}'", def.ScriptName).c_str());
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