#pragma once

#include <RE/T/TESObjectREFR.h>

#include "PapyrusScriptBindings.h"

namespace NoESP::PapyrusInterface {

    void BindScript_ToForm(RE::StaticFunctionTag*, std::string scriptName, RE::TESForm* form, bool addOnce = false) {
        if (!System::GetSingleton().TryLinkScript(scriptName)) return;
        PapyrusScriptBindings::BindToForm(scriptName, form, addOnce);
    }

    void BindScript_ToEditorId(RE::StaticFunctionTag*, std::string scriptName, std::string editorId, bool addOnce = false) {
        if (!System::GetSingleton().TryLinkScript(scriptName)) return;
        PapyrusScriptBindings::BindToEditorId(scriptName, editorId, addOnce);
    }

    void BindScript_RefreshAllObjectReferences(RE::StaticFunctionTag*) {
        System::GetSingleton().CheckForObjectsToAttachScriptsToFromLiterallyEveryFormInTheGame();
    }

    RE::TESForm* FormFromEditorID_Get(RE::StaticFunctionTag*, std::string editorName) {
        return RE::TESForm::LookupByEditorID(editorName);
    }

    bool BIND(RE::BSScript::IVirtualMachine* vm) {
        vm->RegisterFunction("ToForm", "BindScript", BindScript_ToForm);
        vm->RegisterFunction("ToEditorId", "BindScript", BindScript_ToEditorId);
        vm->RegisterFunction("RefreshAllObjectReferences", "BindScript", BindScript_RefreshAllObjectReferences);
        vm->RegisterFunction("Get", "FormFromEditorID", FormFromEditorID_Get);
        return true;
    }
}
