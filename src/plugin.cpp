#pragma warning(push)
#include <SKSE/SKSE.h>
#include <REL/Relocation.h>
#include <RE/Skyrim.h>
#include <RE/C/ConsoleLog.h>
//#pragma warning(pop)

#include "ScriptsWithoutESP/System.h"
#include "ScriptsWithoutESP/PapyrusInterface.h"

extern "C" __declspec(dllexport) bool SKSEAPI SKSEPlugin_Load(const SKSE::LoadInterface* skse) {
    SKSE::Init(skse);
    SKSE::GetMessagingInterface()->RegisterListener([](SKSE::MessagingInterface::Message* message){
        if (message->type == SKSE::MessagingInterface::kDataLoaded) {
            ScriptsWithoutESP::System::ListenForEvents();
            SKSE::GetPapyrusInterface()->Register(ScriptsWithoutESP::PapyrusInterface::BIND);
        }
    });
    return true;
}
//
extern "C" __declspec(dllexport) bool SKSEAPI SKSEPlugin_Query(const SKSE::QueryInterface*, SKSE::PluginInfo* info) {
    info->infoVersion = SKSE::PluginInfo::kVersion;
    info->name = "ScriptsWithoutESP";
    info->version = 1;
    return true;
}

#ifdef SKYRIM_AE
//extern "C" __declspec(dllexport) constinit auto SKSEPlugin_Version = [](){
//    SKSE::PluginVersionData version;
//    version.PluginName("ScriptsWithoutESP");
//    version.PluginVersion({ 0, 0, 1 });
//    version.CompatibleVersions({ SKSE::RUNTIME_LATEST });
//    version.UsesAddressLibrary(true); // Not really necessary or is it?
//    return version;
//}();
#endif
