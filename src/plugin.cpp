#include "NoESP/Log.h"
#include "NoESP/Config.h"
#include "NoESP/System.h"
#include "NoESP/PapyrusInterface.h"

SKSEPluginLoad(const SKSE::LoadInterface* skse) {
    SKSE::Init(skse);
    SKSE::GetMessagingInterface()->RegisterListener([](SKSE::MessagingInterface::Message* message){
        if (message->type == SKSE::MessagingInterface::kDataLoaded) {
            Logging::Initialize();
            logger::info("No .esp [NG]");
            Config::LoadFromIni([](int searchIndex, double radius, long interval){
                auto& system = System::GetSingleton();
                while (true) {
                    if (system.IsLoaded()) {
                        if (NoESP::Config::LogObjectSearch) {
                            logger::info("Search for objects! Search thread #{} Radius:{} IntervalMs:{}", searchIndex, radius, interval);
                        }
                        System::CheckForObjectsToAttachScriptsToForObjectsInRangeOfPlayer(radius);
                    }
                    std::this_thread::sleep_for(std::chrono::milliseconds(interval));
                }
            });
            System::Load();
            SKSE::GetPapyrusInterface()->Register(NoESP::PapyrusInterface::BIND);
        } else if (message->type == SKSE::MessagingInterface::kNewGame || message->type == SKSE::MessagingInterface::kPostLoadGame) {
            auto& system = System::GetSingleton();
            if (! system.IsLoadedOrSetLoaded()) {
                logger::info("Binding declared forms/references to Scripts");
                system.BindFormIdsToScripts();
                if (Config::SearchObjectReferencesOnStart) {
                    logger::info("Search all game references to attach scripts");
                    System::CheckForObjectsToAttachScriptsToFromLiterallyEveryFormInTheGame();
                } else {
                    logger::info("Did not search all game references to attach scripts. Disabled via .ini");
                }
            }
        }
    });
    return true;
}
