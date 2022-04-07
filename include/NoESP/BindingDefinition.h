#pragma once

#include <any>
#include <regex>

namespace NoESP {

    enum BindingDefinitionType { EditorID, FormID, Invalid };
    enum EditorIdMatcherType { Exact, PrefixMatch, SuffixMatch, PrefixAndSuffixMatch, RegularExpression, None };

    enum CommonLibRawType {
        kNone = 0, kObject = 1, kString = 2, kInt = 3, kFloat = 4, kBool = 5,
        kNoneArray = 10, kObjectArray = 11, kStringArray = 12, kIntArray = 13, kFloatArray = 14, kBoolArray = 15,
        kArraysEnd
    };

    struct EditorIdMatcher {
        EditorIdMatcherType Type = EditorIdMatcherType::None;
        std::string Text;
        std::regex RegularExpression;
    };

    struct PropertyValue {
        std::string PropertyName;
        std::string PropertyValueText;
        bool PropertyTypeHasBeenLoaded = false;
        CommonLibRawType PropertyType;
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
