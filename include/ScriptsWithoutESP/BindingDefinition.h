#pragma once

namespace ScriptsWithoutESP {

    enum BindingDefinitionType { EditorID, FormID, Invalid };

    struct BindingDefinition {
        bool AddOnce = true; // Currently ALWAYS set to trye
        std::string Filename;
        BindingDefinitionType Type = BindingDefinitionType::Invalid;
        std::string ScriptName;
        std::string EditorID;
        int FormID = 0;
        std::string Plugin;
    };
}
