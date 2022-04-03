#include <cassert>
#include <cctype>
#include <cerrno>
#include <cfenv>
#include <cfloat>
#include <cinttypes>
#include <climits>
#include <clocale>
#include <cmath>
#include <csetjmp>
#include <csignal>
#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <cuchar>
#include <cwchar>
#include <cwctype>

#include <algorithm>
#include <any>
#include <array>
#include <atomic>
#include <barrier>
#include <bit>
#include <bitset>
#include <charconv>
#include <chrono>
#include <compare>
#include <complex>
#include <concepts>
#include <condition_variable>
#include <deque>
#include <exception>
#include <execution>
#include <filesystem>
#include <format>
#include <forward_list>
#include <fstream>
#include <functional>
#include <future>
#include <initializer_list>
#include <iomanip>
#include <iosfwd>
#include <ios>
#include <iostream>
#include <istream>
#include <iterator>
#include <latch>
#include <limits>
#include <locale>
#include <map>
#include <memory>
#include <memory_resource>
#include <mutex>
#include <new>
#include <numbers>
#include <numeric>
#include <optional>
#include <ostream>
#include <queue>
#include <random>
#include <ranges>
#include <regex>
#include <ratio>
#include <scoped_allocator>
#include <semaphore>
#include <set>
#include <shared_mutex>
#include <source_location>
#include <span>
#include <sstream>
#include <stack>
#include <stdexcept>
#include <streambuf>
#include <string>
#include <string_view>
#include <syncstream>
#include <system_error>
#include <thread>
#include <tuple>
#include <typeindex>
#include <typeinfo>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <valarray>
#include <variant>
#include <vector>
#include <version>

#include <RE/Skyrim.h>
#include <REL/Relocation.h>
#include <SKSE/SKSE.h>

#include <ShlObj_core.h>
#include <Windows.h>


namespace logger = SKSE::log;
namespace string = SKSE::stl::string;

using namespace std::literals;

#include "NoESP/Log.h"
#include "NoESP/Config.h"
#include "NoESP/System.h"
#include "NoESP/PapyrusInterface.h"

extern "C" __declspec(dllexport) bool SKSEAPI SKSEPlugin_Load(const SKSE::LoadInterface* skse) {
    SKSE::Init(skse);
    SKSE::GetMessagingInterface()->RegisterListener([](SKSE::MessagingInterface::Message* message){
        if (message->type == SKSE::MessagingInterface::kDataLoaded) {
            Logging::Initialize();
            Config::LoadFromIni();
            System::ReadAutoBindingsFiles();
            System::ListenForReferences();
            System::ListenForMenuOpenClose();
            System::ListenForFirstLocationLoad();
            SKSE::GetPapyrusInterface()->Register(NoESP::PapyrusInterface::BIND);
        } else if (message->type == SKSE::MessagingInterface::kNewGame || message->type == SKSE::MessagingInterface::kPostLoadGame) {
            auto& system = System::GetSingleton();
            if (! system.IsLoadedOrSetLoaded()) {
                Log("Binding declared forms/references to Scripts");
                system.BindFormIdsToScripts();
                Log("Search all game references to attach scripts");
                System::CheckForObjectsToAttachScriptsToFromLiterallyEveryFormInTheGame();
            }
        }
    });
    return true;
}

extern "C" __declspec(dllexport) bool SKSEAPI SKSEPlugin_Query(const SKSE::QueryInterface*, SKSE::PluginInfo* info) {
    info->infoVersion = SKSE::PluginInfo::kVersion;
    info->name = "NoESP";
    info->version = 1;
    return true;
}

#ifdef SKYRIM_AE
extern "C" __declspec(dllexport) constinit auto SKSEPlugin_Version = [](){
    SKSE::PluginVersionData version;
    version.PluginName("NoESP");
    version.PluginVersion({ 0, 0, 1 });
    version.CompatibleVersions({ SKSE::RUNTIME_LATEST });
    version.UsesAddressLibrary(true); // Not really necessary or is it?
    return version;
}();
#endif
