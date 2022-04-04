#pragma once

#include <any>
#include <regex>

namespace NoESP {

    enum BindingDefinitionType { EditorID, FormID, Invalid };
    enum EditorIdMatcherType { Exact, PrefixMatch, SuffixMatch, PrefixAndSuffixMatch, RegularExpression };
    enum PropertyType { IntProperty, FloatProperty, BoolProperty, StringProperty, ScriptProperty, UnknownPropertyType };

    struct EditorIdMatcher {
        EditorIdMatcherType Type;
        std::string Text;
        std::regex RegularExpression;
    };

    struct PropertyValue {
        std::string PropertyName;
        std::string PropertyValueText;
        PropertyType PropertyType = PropertyType::UnknownPropertyType;
    };

    struct BindingDefinition {
        bool AddOnce = true; // Currently, ALWAYS set to true
        std::string Filename;
        std::string ScriptName;
        std::string Plugin;
        int FormID = 0;
        std::unordered_map<std::string, PropertyValue> PropertyValues;
        BindingDefinitionType Type = BindingDefinitionType::Invalid;
        EditorIdMatcher EditorIdMatcher;
    };
}
