#pragma once

#include <set>
#include <string>
#include <unordered_map>
#include <vector>

namespace ESPLess {

    // Unused for now!
    struct SaveGameData {
        std::set<std::string> LinkedScripts;
        std::unordered_map<std::string, std::set<std::string>> CompletedAutoBindings;
    };
}
