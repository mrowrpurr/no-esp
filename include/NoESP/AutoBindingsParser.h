#pragma once

#include <filesystem>
#include <format>
#include <functional>
#include <regex>
#include <string>
#include <sstream>
#include <fstream>

#include "BindingDefinition.h"
#include "Utilities.h"

namespace NoESP::AutoBindingsFile {

    // https://stackoverflow.com/a/40903508
    std::string ReadTextFile(const std::filesystem::path& path) {
        // Open the stream to 'lock' the file.
        std::ifstream f(path, std::ios::in | std::ios::binary);

        // Obtain the size of the file.
        const auto sz = std::filesystem::file_size(path);

        // Create a buffer.
        std::string result(sz, '\0');

        // Read the whole file into the buffer.
        f.read(result.data(), sz);

        return result;
    }

    EditorIdMatcher ParseEditorIdMatchText(const std::string& editorIdText) {
        EditorIdMatcher matcher;
        auto editorId = Utilities::ToLowerCase(editorIdText);
        if (editorId.starts_with('*') && editorId.ends_with('*') && editorId.length() > 2) {
            matcher.Type = EditorIdMatcherType::PrefixAndSuffixMatch;
            matcher.Text = editorId.substr(1, editorId.length() - 2);
        } else if (editorId.starts_with('*')) {
            matcher.Type = EditorIdMatcherType::SuffixMatch;
            matcher.Text = editorId.substr(1);
        } else if (editorId.ends_with('*')) {
            matcher.Type = EditorIdMatcherType::PrefixMatch;
            matcher.Text = editorId.substr(0, editorId.length() - 1);
        } else if (editorId.starts_with('/') && editorId.ends_with('/') && editorId.length() > 2) {
            matcher.Type = EditorIdMatcherType::RegularExpression;
            matcher.RegularExpression = std::regex(editorId.substr(1, editorId.length() - 2), std::regex_constants::icase);
        } else {
            matcher.Type = EditorIdMatcherType::Exact;
            matcher.Text = editorId;
        }
        return matcher;
    }

    // Recurse me!
    void ParsePropertiesFromLine(FormPropertyMap& properties, std::string& line) {
        // from the right side of the line, look for: A="B C" or D=1 or E=true etc
        static auto rightHandSideQuotedPropertyAssignment = std::regex("\\s+([^=\\s]+)=\"([^\"]+)\"$", std::regex_constants::icase);
        static auto rightHandSidePropertyAssignment = std::regex(R"(\s+([^=\s]+)=([^=]+)$)", std::regex_constants::icase);
        static auto slashNregex = std::regex("\\\\n");
        static auto slashTregex = std::regex("\\\\t");
        try {
            std::smatch matches;
            if (std::regex_search(line, matches, rightHandSideQuotedPropertyAssignment)) {
                PropertyValue property;
                property.PropertyName = Utilities::ToLowerCase(matches[1].str());
                property.PropertyValueText = std::regex_replace(matches[2].str(), slashNregex, "\n");
                property.PropertyValueText = std::regex_replace(property.PropertyValueText, slashTregex, "\t");
                properties.insert_or_assign(property.PropertyName, property);

                line = line.substr(0, matches.position());
                ParsePropertiesFromLine(properties, line);
            } else if (std::regex_search(line, matches, rightHandSidePropertyAssignment)) {
                PropertyValue property;
                property.PropertyName = Utilities::ToLowerCase(matches[1].str());
                property.PropertyValueText = std::regex_replace(matches[2].str(), slashNregex, "\n");
                property.PropertyValueText = std::regex_replace(property.PropertyValueText, slashTregex, "\t");
                properties.insert_or_assign(property.PropertyName, property);

                line = line.substr(0, matches.position());
                ParsePropertiesFromLine(properties, line);
            }
        } catch (...) {
            Log("Error parsing properties from line '{}'", line);
        }
    }

    BindingDefinition ParseLine(std::string line) {
        FormPropertyMap properties;
        ParsePropertiesFromLine(properties, line);

        BindingDefinition entry;
        entry.PropertyValues = properties;
        static auto scriptNameWithPluginFormID = std::regex(R"(^\s*([^\s]+)\s+0x([^\s]+)\s+([^\s]+)\s*$)", std::regex_constants::icase);
        static auto scriptNameWithSkyrimFormID = std::regex(R"(^\s*([^\s]+)\s+0x([^\s]+)\s*$)", std::regex_constants::icase);
        static auto scriptNameWithEditorID = std::regex(R"(^\s*([^\s]+)\s+([^\s]+)\s*$)", std::regex_constants::icase);
        static auto scriptNameOnly = std::regex(R"(^\s*([^\s]+)\s*$)", std::regex_constants::icase);
        std::smatch matches;
        try {
            if (std::regex_search(line, matches, scriptNameWithPluginFormID)) {
                entry.Type = BindingDefinitionType::FormID;
                entry.ScriptName = matches[1].str();
                auto formIdHex = matches[2].str();
                try {
                    entry.FormID = std::stoi("0x" + formIdHex, nullptr, 16);
                } catch (...) {
                    Log("Invalid FormID '0x{}'", formIdHex);
                }
                entry.Plugin = matches[3].str();
            } else if (std::regex_search(line, matches, scriptNameWithSkyrimFormID)) {
                entry.Type = BindingDefinitionType::FormID;
                entry.ScriptName = matches[1].str();
                auto formIdHex = matches[2].str();
                entry.FormID = std::stoi(formIdHex, nullptr, 16);
            } else if (std::regex_search(line, matches, scriptNameWithEditorID)) {
                entry.Type = BindingDefinitionType::EditorID;
                entry.ScriptName = matches[1].str();
                entry.EditorIdMatcher = ParseEditorIdMatchText(matches[2].str());
            } else if (std::regex_search(line, matches, scriptNameOnly)) {
                entry.Type = BindingDefinitionType::FormID;
                entry.ScriptName = matches[1].str();
                entry.FormID = 20; // 0x14 which is the PlayerRef
            } else {
                Log("Unknown/Unsupported Entry Line Syntax: '{}'", line);
            }
        } catch (...) {
            Log("Error parsing line: '{}'", line);
        }
        return entry;
    }

    void Read(std::function<void(BindingDefinition& entry)> entryCallback, const std::string& bindingFilesDirectory = "Data/Scripts/AutoBindings") {
        if (! std::filesystem::is_directory(bindingFilesDirectory))
            return;

        for (auto& file : std::filesystem::directory_iterator(bindingFilesDirectory)) {
            if (file.is_regular_file()) {
                try {
                    if (! file.path().string().ends_with(".txt")) continue;
                    auto text = ReadTextFile(file.path());
                    std::istringstream stringStream(text);
                    for (std::string line; std::getline(stringStream, line); ) {
                        if (!line.empty()) {
                            // Strip trailing \r from newline
                            if (line.ends_with("\r")) {
                                line.erase(line.length() - 1);
                            }
                            // Strip anything before comment
                            size_t commentStartIndex = line.find('#');
                            if (commentStartIndex != std::string::npos) {
                                if (commentStartIndex == 0) line = "";
                                else line = line.substr(0, commentStartIndex);
                            }
                            commentStartIndex = line.find(';');
                            if (commentStartIndex != std::string::npos) {
                                if (commentStartIndex == 0) line = "";
                                else line = line.substr(0, commentStartIndex);
                            }
                            commentStartIndex = line.find("//");
                            if (commentStartIndex != std::string::npos) {
                                if (commentStartIndex == 0) line = "";
                                else line = line.substr(0, commentStartIndex);
                            }

                            // Parse the (trimmed) line
                            try {
                                auto entry = ParseLine(line);
                                if (entry.Type != BindingDefinitionType::Invalid) {
                                    entry.Filename = file.path().string();
                                    try {
                                        entryCallback(entry);
                                    } catch (...) {
                                        Log("[Internal] AutoBinding entry discovery callback failed for '{}'", line);
                                    }
                                }
                            } catch (...) {
                                Log("Failed to parse entry line '{}'", line);
                            }
                        }
                    }
                } catch (...) {
                    Log("Failed to read file '{}'", file.path().string());
                }
            }
        }
    }
}
