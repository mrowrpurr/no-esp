#pragma once

#include <regex>

namespace NoESP::Expressions {

    enum class SelectorTargetType { FormType, EditorID, Name };
    enum class SelectorSearchType { Prefix, Suffix, PrefixAndSuffix, Regex, Unset };

    struct Selector {
        SelectorTargetType TargetType;
        SelectorSearchType SearchType;
        std::string Text;
        std::regex Pattern;
    };
}
