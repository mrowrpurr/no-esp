#include <SkyrimScripting/Plugin.h>

#include "NoESP/Config.h"
#include "NoESP/PapyrusInterface.h"
#include "NoESP/System.h"

_OnInit_ {
    SKSE::GetMessagingInterface()->RegisterListener([](SKSE::MessagingInterface::Message* message) {
        if (message->type == SKSE::MessagingInterface::kDataLoaded) {
            _Log_("No .esp [NG]");
            Config::LoadFromIni([](int searchIndex, double radius, long interval) {
                auto& system = System::GetSingleton();
                while (true) {
                    if (system.IsLoaded()) {
                        if (NoESP::Config::LogObjectSearch) {
                            _Log_("Search for objects! Search thread #{} Radius:{} IntervalMs:{}",
                                  searchIndex, radius, interval);
                        }
                        System::CheckForObjectsToAttachScriptsToForObjectsInRangeOfPlayer(radius);
                    }
                    std::this_thread::sleep_for(std::chrono::milliseconds(interval));
                }
            });
            System::Load();
            SKSE::GetPapyrusInterface()->Register(NoESP::PapyrusInterface::BIND);
        } else if (message->type == SKSE::MessagingInterface::kNewGame ||
                   message->type == SKSE::MessagingInterface::kPostLoadGame) {
            auto& system = System::GetSingleton();
            if (!system.IsLoadedOrSetLoaded()) {
                _Log_("Binding declared forms/references to Scripts");
                system.BindFormIdsToScripts();
                if (Config::SearchObjectReferencesOnStart) {
                    _Log_("Search all game references to attach scripts");
                    System::CheckForObjectsToAttachScriptsToFromLiterallyEveryFormInTheGame();
                } else {
                    _Log_(
                        "Did not search all game references to attach scripts. Disabled via .ini");
                }
            }
        }
    });
}
