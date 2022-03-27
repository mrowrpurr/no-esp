#pragma once

namespace ESPLess {

    enum BindingDefinitionType { EditorID, FormID, Invalid };

    struct BindingDefinition {
        std::string ID; // Not used yet, just * for now
        bool AddOnce = false; // Not used yet, always yet because of '*' for now

        std::string Filename;
        BindingDefinitionType Type = BindingDefinitionType::Invalid;
        std::string ScriptName;
        std::string EditorID;
        int FormID = 0;
        std::string Plugin;
    };
}
