#pragma once

#include <RE/T/TESObjectREFR.h>

#include "PapyrusScriptBindings.h"

namespace ScriptsWithoutESP::PapyrusInterface {

    void BindScript_ToForm(RE::StaticFunctionTag*, std::string scriptName, RE::TESForm* form, bool addOnce = false) {
        PapyrusScriptBindings::BindToForm(scriptName, form, addOnce);
    }

    void BindScript_ToEditorId(RE::StaticFunctionTag*, std::string scriptName, std::string editorId, bool addOnce = false) {
        PapyrusScriptBindings::BindToEditorId(scriptName, editorId, addOnce);
    }

    bool BIND(RE::BSScript::IVirtualMachine* vm) {
        vm->RegisterFunction("ToForm", "BindScript", BindScript_ToForm);
        vm->RegisterFunction("ToEditorId", "BindScript", BindScript_ToEditorId);
        return true;
    }
}
