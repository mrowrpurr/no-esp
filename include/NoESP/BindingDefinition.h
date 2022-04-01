#pragma once

#include <regex>

namespace NoESP {

    enum BindingDefinitionType { EditorID, FormID, Invalid };
    enum EditorIdMatcherType { Exact, PrefixMatch, SuffixMatch, PrefixAndSuffixMatch, RegularExpression };

    struct EditorIdMatcher {
        EditorIdMatcherType Type;
        std::string Text;
        std::regex RegularExpression;
    };

    struct BindingDefinition {
        bool AddOnce = true; // Currently, ALWAYS set to true
        std::string Filename;
        BindingDefinitionType Type = BindingDefinitionType::Invalid;
        std::string ScriptName;
        EditorIdMatcher EditorIdMatcher;
        int FormID = 0;
        std::string Plugin;
    };
}
