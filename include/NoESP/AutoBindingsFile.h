#pragma once

#include <filesystem>
#include <format>
#include <functional>
#include <regex>
#include <string>
#include <sstream>

#include "BindingDefinition.h"

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

        BindingDefinition ParseLine(std::string line) {
            BindingDefinition entry;
            static auto scriptNameWithPluginFormID = std::regex(R"(^\s*([^\s]+)\s+0x([^\s]+)\s+([^\s]+)\s*$)");
            static auto scriptNameWithSkyrimFormID = std::regex(R"(^\s*([^\s]+)\s+0x([^\s]+)\s*$)");
            static auto scriptNameWithEditorID = std::regex(R"(^\s*([^\s]+)\s+([^\s]+)\s*$)");
            static auto scriptNameOnly = std::regex(R"(^\s*([^\s]+)\s*$)");
            std::smatch matches;
            try {
                if (std::regex_search(line, matches, scriptNameWithPluginFormID)) {
                    entry.Type = BindingDefinitionType::FormID;
                    entry.ScriptName = matches[1].str();
                    auto formIdHex = matches[2].str();
                    try {
                        entry.FormID = std::stoi("0x" + formIdHex, nullptr, 16);
                    } catch (...) {
                        RE::ConsoleLog::GetSingleton()->Print(std::format("[AutoBindings] Invalid FormID '0x{}'", formIdHex).c_str());
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
                    entry.EditorID = matches[2].str();
                } else if (std::regex_search(line, matches, scriptNameOnly)) {
                    entry.Type = BindingDefinitionType::FormID;
                    entry.ScriptName = matches[1].str();
                    entry.FormID = 20; // 0x14 which is the PlayerRef
                }
            } catch (...) {
                RE::ConsoleLog::GetSingleton()->Print(std::format("[AutoBindings] Error parsing line: '{}'", line).c_str());
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

                            auto entry = ParseLine(line);
                            if (entry.Type != BindingDefinitionType::Invalid) {
                                entry.Filename = file.path().string();
                                entryCallback(entry);
                            }
                        }
                    }
                } catch (...) {
                    RE::ConsoleLog::GetSingleton()->Print(std::format("[AutoBindings] Failed to read file '{}'", file.path().string()).c_str());
                }
            }
        }
    }
}
