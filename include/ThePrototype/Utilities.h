#pragma once

#include <string>
#include <algorithm>

namespace ThePrototype::Utilities {

    std::string ToLowerCase(const std::string& text) {
        std::string copy{text};
        std::transform(copy.begin(), copy.end(), copy.begin(), [](unsigned char c){ return std::tolower(c); });
        return copy;
    }
}
