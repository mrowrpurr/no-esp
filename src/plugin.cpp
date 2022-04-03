#include <RE/Skyrim.h>
#include <REL/Relocation.h>
#include <SKSE/SKSE.h>

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
                if (Config::SearchObjectReferencesOnStart) {
                    Log("Search all game references to attach scripts");
                    System::CheckForObjectsToAttachScriptsToFromLiterallyEveryFormInTheGame();
                } else {
                    Log("Did not search all game references to attach scripts. Disabled via .ini");
                }
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
