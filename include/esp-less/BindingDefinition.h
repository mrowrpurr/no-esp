#pragma once

namespace ESPLess {

    enum BindingDefinitionType { EditorID, FormID, Invalid };

    struct BindingDefinition {
        std::string ID;
        std::string Filename;
        BindingDefinitionType Type = BindingDefinitionType::Invalid;
        std::string ScriptName;
        std::string EditorID;
        int FormID = 0;
        std::string Plugin;
    };
}
