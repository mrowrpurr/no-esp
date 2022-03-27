#pragma once

#include <filesystem>
#include <format>
#include <functional>
#include <regex>
#include <string>
#include <sstream>

namespace ESPLess::AutoBindingsFile {

    enum FileEntryType { EditorID, FormID, Invalid };

    struct FileEntry {
        FileEntryType Type = FileEntryType::Invalid;
        std::string ScriptName;
        std::string EditorID;
        int FormID;
        std::string Plugin;
    };

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

        FileEntry ParseLine(const std::string& line) {
            // TODO require a unique identifier per script (unique per file)
            static auto scriptNameWithPluginFormID = std::regex(R"(^([^\s]+)\s+0x([^\s]+)\s+([^\s]+)$)");
            static auto scriptNameWithSkyrimFormID = std::regex(R"(^([^\s]+)\s+0x([^\s]+)$)");
            static auto scriptNameWithEditorID = std::regex(R"(^([^\s]+)\s+([^\s]+)$)");
            static auto scriptNameOnly = std::regex(R"(^([^\s]+)$)");
            FileEntry entry;
            std::smatch matches;
            try {
                if (std::regex_search(line, matches, scriptNameWithPluginFormID)) {
                    entry.Type = FileEntryType::FormID;
                    entry.ScriptName = matches[1].str();
                    auto formIdHex = matches[2].str();
                    try {
                        entry.FormID = std::stoi("0x" + formIdHex, nullptr, 16);
                    } catch (...) {
                        RE::ConsoleLog::GetSingleton()->Print(std::format("Invalid Hex String {}", formIdHex).c_str());
                    }
                    entry.Plugin = matches[3].str();
                } else if (std::regex_search(line, matches, scriptNameWithSkyrimFormID)) {
                    entry.Type = FileEntryType::FormID;
                    entry.ScriptName = matches[1].str();
                    auto formIdHex = matches[2].str();
                    entry.FormID = std::stoi(formIdHex, nullptr, 16);
                } else if (std::regex_search(line, matches, scriptNameWithEditorID)) {
                    entry.Type = FileEntryType::EditorID;
                    entry.ScriptName = matches[1].str();
                    entry.EditorID = matches[2].str();
                } else if (std::regex_search(line, matches, scriptNameOnly)) {
                    entry.Type = FileEntryType::EditorID;
                    entry.ScriptName = matches[1].str();
                    entry.EditorID = "PlayerRef";
                }
            } catch (...) {
                RE::ConsoleLog::GetSingleton()->Print(std::format("Failed to parse auto binding line: {}", line).c_str());
            }
            return entry;
        }
    }

    void Read(std::function<void(const FileEntry& entry)> entryCallback, const std::string& bindingFilesDirectory = "Data/Scripts/AutoBindings") {
        for (auto& file : std::filesystem::directory_iterator(bindingFilesDirectory)) {
            RE::ConsoleLog::GetSingleton()->Print(std::format("FILE {}", file.path().string().c_str()).c_str());
            if (file.is_regular_file()) {
                try {
                    auto text = ReadTextFile(file.path());
                    RE::ConsoleLog::GetSingleton()->Print(text.c_str());
                    std::istringstream stringStream(text);
                    for (std::string line; std::getline(stringStream, line); ) {
                        if (!line.empty()) {
                            if (line.ends_with("\r")) {
                                line.erase(line.length() - 1);
                            }
                            RE::ConsoleLog::GetSingleton()->Print(std::format("PARSE LINE '{}' ???", line).c_str());
                            auto entry = ParseLine(line);
                            if (entry.Type != FileEntryType::Invalid) {
                                entryCallback(entry);
                            }
                        }
                    }
                } catch (...) {
                    RE::ConsoleLog::GetSingleton()->Print(std::format("Failed to read {}", file.path().string()).c_str());
                }
            }
        }
    }
}
