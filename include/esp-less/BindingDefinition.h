#pragma once

namespace ESPLess {

    enum BindingDefinitionType { EditorID, FormID, Invalid };

    struct BindingDefinition {
        bool AddOnce = true; // Currently, always true
        std::string Filename;
        BindingDefinitionType Type = BindingDefinitionType::Invalid;
        std::string ScriptName;
        std::string EditorID;
        int FormID = 0;
        std::string Plugin;
    };
}
