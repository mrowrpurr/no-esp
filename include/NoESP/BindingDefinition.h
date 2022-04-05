#pragma once

#include <any>
#include <regex>

namespace NoESP {

    enum BindingDefinitionType { EditorID, FormID, Invalid };
    enum EditorIdMatcherType { Exact, PrefixMatch, SuffixMatch, PrefixAndSuffixMatch, RegularExpression };

    struct EditorIdMatcher {
        EditorIdMatcherType Type;
        std::string Text;
        std::regex RegularExpression;
    };

    struct PropertyValue {
        std::string PropertyName;
        std::string PropertyValueText;
        bool PropertyTypeHasBeenLoaded = false;
        RE::BSScript::TypeInfo::RawType PropertyType;
        std::string PropertyTypeScriptName;
    };

    typedef std::unordered_map<std::string, PropertyValue> FormPropertyMap;

    struct BindingDefinition {
        bool AddOnce = true; // Currently, ALWAYS set to true
        std::string Filename;
        std::string ScriptName;
        std::string Plugin;
        int FormID = 0;
        FormPropertyMap PropertyValues;
        BindingDefinitionType Type = BindingDefinitionType::Invalid;
        EditorIdMatcher EditorIdMatcher;
    };
}
