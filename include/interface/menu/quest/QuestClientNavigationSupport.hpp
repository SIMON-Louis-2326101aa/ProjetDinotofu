// EN: Shared client/quest navigation helpers used by the modular quest menus.
// FR: Helpers partagés de navigation client/quêtes utilisés par les menus de quêtes modularisés.
#ifndef INCLUDE_INTERFACE_MENU_QUEST_QUESTCLIENTNAVIGATIONSUPPORT_HPP
#define INCLUDE_INTERFACE_MENU_QUEST_QUESTCLIENTNAVIGATIONSUPPORT_HPP

#include "interface/model/MenuOption.hpp"
#include <string>
#include <vector>

class Player;
struct Quest;

namespace QuestClientNavigationSupport
{
    struct ClientQuestCounts
    {
        int active = 0;
        int ready = 0;
        int turnedIn = 0;
        int total = 0;
    };

    bool isMaterialDeliveryQuest(const Quest& quest);
    bool canCompleteMaterialDelivery(const Player& player, const Quest& quest);
    bool hasRequiredQuestMaterial(const Player& player, const Quest& quest);
    bool isReadyToTurnIn(const Player& player, const Quest& quest);

    std::string extractRecommendedClientName(const Quest& quest);
    std::vector<std::string> defaultRecommendedClientNames();
    std::vector<std::string> collectRecommendedClients(const Player& player);
    bool isRecommendedClientName(const std::string& clientName);

    ClientQuestCounts countQuestsForClient(const Player& player, const std::string& clientName);
    std::string clientQuestStatusText(const ClientQuestCounts& counts);
    std::string clientQuestHintText(const ClientQuestCounts& counts);
    MenuOptionItemData makeClientQuestNavigationItemData(
        const std::string& clientName,
        const std::string& section,
        const std::string& detail,
        const ClientQuestCounts& counts
    );
}

#endif
