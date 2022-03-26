#pragma warning(push)

#include <SKSE/SKSE.h>

extern "C" __declspec(dllexport) bool SKSEAPI SKSEPlugin_Load(const SKSE::LoadInterface* skse) {
    SKSE::Init(skse);
    return true;
}

extern "C" __declspec(dllexport) bool SKSEAPI SKSEPlugin_Query(const SKSE::QueryInterface*, SKSE::PluginInfo* info) {
    info->infoVersion = SKSE::PluginInfo::kVersion;
    info->name = "ESPless";
    info->version = 1;
    return true;
}

extern "C" __declspec(dllexport) constinit auto SKSEPlugin_Version = [](){
    SKSE::PluginVersionData version;
    version.PluginName("ESPless");
    version.PluginVersion({ 0, 0, 1 });
    version.CompatibleVersions({ SKSE::RUNTIME_LATEST });
    return version;
}();








//            vm->objectTypeToTypeID()

//            policy->GetHandleForObject(character);

// Find Gold! Then bind a Papyrus script to it!
//            auto* dataHandler = RE::TESDataHandler::GetSingleton();
//            auto* gold = dataHandler->LookupForm(0xf, "Skyrim.esm");
//            gold->
