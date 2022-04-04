#pragma once

#include <filesystem>
#include <format>
#include <functional>
#include <regex>
#include <string>
#include <sstream>

#include "BindingDefinition.h"
#include "Utilities.h"

namespace NoESP::AutoBindingsFile {

    namespace {
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

        std::unordered_map<std::string, PropertyValue> ParsePropertiesForBinding(const std::string& propertiesTextDefinition) {
            static auto nextPropertyPatternQuoted = std::regex(R"(\s*([^=\s]+)=\"([^\"]+)\".*)");
            static auto nextPropertyPattern = std::regex(R"(\s*([^=\s]+)=([^\s]+).*)");
            std::unordered_map<std::string, PropertyValue> properties;
            std::string text = propertiesTextDefinition;
            std::smatch matches;
            while (true) {
                if (std::regex_search(text, matches, nextPropertyPatternQuoted)) {
                    auto propertyName = matches[1].str();
                    auto propertyValue = matches[2].str();
                    PropertyValue property;
                    property.PropertyName = Utilities::ToLowerCase(propertyName);
                    property.PropertyValueText = propertyValue;
                    properties.insert_or_assign(propertyName, property);
                    text = std::format("{}{}", text.substr(0, matches.position()), text.substr(matches.position() + matches[1].length() + 2 + matches[2].length() + 2)); // 2 for '='? and 2 for '""'
                } else if (std::regex_search(text, matches, nextPropertyPattern) || std::regex_search(text, matches, nextPropertyPattern)) {
                    auto propertyName = matches[1].str();
                    auto propertyValue = matches[2].str();
                    PropertyValue property;
                    property.PropertyName = Utilities::ToLowerCase(propertyName);
                    property.PropertyValueText = propertyValue;
                    properties.insert_or_assign(propertyName, property);
                    text = std::format("{}{}", text.substr(0, matches.position()), text.substr(matches.position() + matches[1].length() + 2 + matches[2].length())); // 2 for '='?
                } else {
                    break;
                }
            }
            return properties;
        }

        EditorIdMatcher ParseEditorIdMatchText(const std::string& editorIdText) {
            EditorIdMatcher matcher;
            auto editorId = Utilities::ToLowerCase(editorIdText);
            if (editorId.starts_with('*') && editorId.ends_with('*') && editorId.length() > 2) {
                matcher.Type = EditorIdMatcherType::PrefixAndSuffixMatch;
                matcher.Text = editorId.substr(1, editorId.length() - 2);
            } else if (editorId.starts_with('*')) {
                matcher.Type = EditorIdMatcherType::PrefixMatch;
                matcher.Text = editorId.substr(1);
            } else if (editorId.ends_with('*')) {
                matcher.Type = EditorIdMatcherType::SuffixMatch;
                matcher.Text = editorId.substr(0, editorId.length() - 2);
            } else if (editorId.starts_with('/') && editorId.ends_with('/') && editorId.length() > 2) {
                matcher.Type = EditorIdMatcherType::RegularExpression;
                matcher.RegularExpression = editorId.substr(1, editorId.length() - 2);
            } else {
                matcher.Type = EditorIdMatcherType::Exact;
                matcher.Text = editorId;
            }
            return matcher;
        }

        BindingDefinition ParseLine(const std::string& line) {
            BindingDefinition entry;
            static auto scriptNameWithPluginFormID = std::regex(R"(^\s*([^\s]+)\s+0x([^\s]+)\s+([^\s]+)\s*([^|]+))");
            static auto scriptNameWithSkyrimFormID = std::regex(R"(^\s*([^\s]+)\s+0x([^\s]+)\s*[^|]+)");
            static auto scriptNameWithEditorID = std::regex(R"(^\s*([^\s]+)\s+([^\s]+)\s*[^|]+)");
            static auto scriptNameOnly = std::regex(R"(^V\s*([^\s]+)\s*[^|]+)");
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
                }
            } catch (...) {
                Log("Error parsing line: '{}'", line);
            }
            auto propertyDefinitionStartIndex = line.find('|');
            if (propertyDefinitionStartIndex != std::string::npos && propertyDefinitionStartIndex != line.length() - 1) {
                auto propertyDefinitionText = line.substr(propertyDefinitionStartIndex + 1); // Everything after the '|'
                entry.PropertyValues = ParsePropertiesForBinding(propertyDefinitionText);
            }
            return entry;
        }
    }

    void Read(std::function<void(const BindingDefinition& entry)> entryCallback, const std::string& bindingFilesDirectory = "Data/Scripts/AutoBindings") {
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
                            auto entry = ParseLine(line);
                            Log("ENTRY WITH {} PROPERTIES", entry.PropertyValues.size());
                            if (entry.Type != BindingDefinitionType::Invalid) {
                                entry.Filename = file.path().string();
                                entryCallback(entry);
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
