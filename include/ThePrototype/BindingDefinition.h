#pragma once

#include <any>
#include <regex>
#include <set>

#include "Utilities.h"

namespace ThePrototype {

    enum BindingDefinitionType { EditorID, FormID, FormType, Invalid };
    enum EditorIdMatcherType { Exact, PrefixMatch, SuffixMatch, PrefixAndSuffixMatch, RegularExpression, None };

    // Note: this can be removed and switch back to using the direct raw type. No longer using unit tests... :'(
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
        std::set<RE::FormType> FormTypes;
    };

    // Yo gurl, move this to a better place!
    static bool DoesEditorIdMatch(const EditorIdMatcher& matcher, const std::string& editorIdText) {
        if (editorIdText.empty()) return false;
        std::string editorId = Utilities::ToLowerCase(editorIdText);
        switch (matcher.Type) {
            case EditorIdMatcherType::Exact:
                return editorId == matcher.Text;
            case EditorIdMatcherType::PrefixMatch:
                return editorId.starts_with(matcher.Text);
            case EditorIdMatcherType::SuffixMatch:
                return editorId.ends_with(matcher.Text);
            case EditorIdMatcherType::PrefixAndSuffixMatch:
                return editorId.find(matcher.Text) != std::string::npos;
            case EditorIdMatcherType::RegularExpression:
                return std::regex_match(editorId, matcher.RegularExpression);
            default:
                return false;
        }
    }
}
