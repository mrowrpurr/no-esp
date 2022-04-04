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
            Config::LoadFromIni([](int searchIndex, double radius, long interval){
                auto& system = System::GetSingleton();
                while (true) {
                    if (system.IsLoaded()) {
                        if (NoESP::Config::LogObjectSearch) {
                            Log("Search for objects! Search thread #{} Radius:{} IntervalMs:{}", searchIndex, radius, interval);
                        }
                        System::CheckForObjectsToAttachScriptsToForObjectsInRangeOfPlayer(radius);
                    }
                    std::this_thread::sleep_for(std::chrono::milliseconds(interval));
                }
            });

            // Test - access properties from Main Menu
//            auto* vm = RE::BSScript::Internal::VirtualMachine::GetSingleton();
//            RE::VMTypeID typeId;
//            Log("Default type ID '{}'", typeId);
//            bool linkedOk = vm->linker.Process("Shrinkify"); // This returns a bool, do we want it? What does it return for already loaded scripts, true or false?
//            bool result = vm->GetTypeIDForScriptObject("Shrinkify", typeId);
//            Log("Filled type ID '{}' GotType:{} Linked:{}", typeId, result, linkedOk);
            //////////////////////////////////////////

            System::Load();
            SKSE::GetPapyrusInterface()->Register(NoESP::PapyrusInterface::BIND);
        } else if (message->type == SKSE::MessagingInterface::kNewGame || message->type == SKSE::MessagingInterface::kPostLoadGame) {

            // Test - access properties from Main Menu
//            Log("NEW GAME OR LOAD");
            auto* vm = RE::BSScript::Internal::VirtualMachine::GetSingleton();
//            RE::VMTypeID typeId;
//            Log("Default type ID '{}'", typeId);
//            bool linkedOk = vm->linker.Process("Shrinkify"); // This returns a bool, do we want it? What does it return for already loaded scripts, true or false?
//            bool result = vm->GetTypeIDForScriptObject("Shrinkify", typeId);
//            Log("Shrinkify - Filled type ID '{}' GotType:{} Linked:{}", typeId, result, linkedOk);
//
//            linkedOk = vm->linker.Process("SayHello"); // This returns a bool, do we want it? What does it return for already loaded scripts, true or false?
//            result = vm->GetTypeIDForScriptObject("SayHello", typeId);
//            Log("SayHello - Filled type ID '{}' GotType:{} Linked:{}", typeId, result, linkedOk);

//            RE::BSTSmartPointer<RE::BSScript::Object> objectPtr;
//            vm->CreateObject("SayHello", objectPtr);
//            try {
//                Log("Getting info for SayHello...");
//                auto* info = objectPtr->GetTypeInfo();
//                if (info) {
//                    Log("Got type info!");
//                    Log("NAME: {}", info->GetName());
//                } else {
//                    Log("Couldn't get type info!");
//                }
//            } catch (...) {
//                Log("KABOOM when trying to get type info of SayHello");
//            }

//            result = vm->GetTypeIDForScriptObject("SayHello", typeId);
//            Log("SayHello - Object Exists - Filled type ID '{}' GotType:{} Linked:{}", typeId, result, linkedOk);

//            vm->linker.Process("SayHello");
//
//            RE::BSTSmartPointer<RE::BSScript::ObjectTypeInfo> objectTypeInfoPtr;
//            vm->GetScriptObjectType("SayHello", objectTypeInfoPtr);
//
//            if (objectTypeInfoPtr) {
//                Log("Got pointer for name {} with {} properties", objectTypeInfoPtr->GetName(), objectTypeInfoPtr->propertyCount);
//                auto* properties = objectTypeInfoPtr->GetPropertyIter();
//                if (properties) {
//                    Log("Loop thru properties...");
//                    for (uint32_t i = 0; i < objectTypeInfoPtr->propertyCount; i++) {
//                        Log("Getting the property name...");
//                        auto propertyName = properties[i].name;
//                        Log("Property name '{}'", propertyName.c_str());
//                        Log("Getting type info...");
////                        auto typeName = properties[i].info.type;
////                        Log("Type name parent object name '{}'", typeName.c_str());
//                    }
//                } else {
//                    Log("No property iterator!");
//                }
//            } else {
//                Log("NO POINTER");
//            }

            /////////////////////////////////////////

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
