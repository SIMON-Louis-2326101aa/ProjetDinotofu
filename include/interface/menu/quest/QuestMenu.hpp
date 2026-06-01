// EN: QuestMenu.hpp briefly defines this Dinotofu module and its responsibilities.
// FR: QuestMenu.hpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu.
// Description: Terminal menus for guild quests, notable NPCs and quest consultation.

#ifndef INCLUDE_INTERFACE_MENU_QUEST_QUESTMENU_HPP
#define INCLUDE_INTERFACE_MENU_QUEST_QUESTMENU_HPP

#include "entity/Player.hpp"
#include "progression/DifficultyMode.hpp"

#include <string>

class QuestMenu
{
private:
    // EN: displayQuestJournal declares or implements a focused behavior used by this module.
    // FR: displayQuestJournal déclare ou implémente un comportement précis utilisé par ce module.
    static void displayQuestJournal(const Player& player);
    // EN: openGuild declares or implements a focused behavior used by this module.
    // FR: openGuild déclare ou implémente un comportement précis utilisé par ce module.
    static void openGuild(Player& player);
    // EN: acceptGuildQuest declares or implements a focused behavior used by this module.
    // FR: acceptGuildQuest déclare ou implémente un comportement précis utilisé par ce module.
    static void acceptGuildQuest(Player& player);
    // EN: completeQuestAtClient declares or implements a focused behavior used by this module.
    // FR: completeQuestAtClient déclare ou implémente un comportement précis utilisé par ce module.
    static void completeQuestAtClient(Player& player, const std::string& clientName);
    // EN: resolveGuildServiceQuest progresses guild service contracts from the terminal guild desk.
    // FR: resolveGuildServiceQuest fait progresser les contrats de service depuis le comptoir de guilde.
    static void resolveGuildServiceQuest(Player& player);
    // EN: openReadyQuestTurnInMenu opens a focused hand-in menu for ready quests.
    // FR: openReadyQuestTurnInMenu ouvre un menu ciblé pour rendre les quêtes prêtes.
    static void openReadyQuestTurnInMenu(Player& player);
    // EN: openExplorationMenu declares or implements a focused behavior used by this module.
    // FR: openExplorationMenu déclare ou implémente un comportement précis utilisé par ce module.
    static void openExplorationMenu(Player& player, DifficultyMode difficulty);

public:
    // EN: openQuestHub declares or implements a focused behavior used by this module.
    // FR: openQuestHub déclare ou implémente un comportement précis utilisé par ce module.
    static void openQuestHub(Player& player);
    // EN: talkToClient opens a precise vendor/client quest dialogue from a shop.
    // FR: talkToClient ouvre un dialogue de quêtes précis depuis une boutique.
    static void talkToClient(Player& player, const std::string& clientName);
    // EN: consultOnly declares or implements a focused behavior used by this module.
    // FR: consultOnly déclare ou implémente un comportement précis utilisé par ce module.
    static void consultOnly(const Player& player);
    // EN: openExploration declares or implements a focused behavior used by this module.
    // FR: openExploration déclare ou implémente un comportement précis utilisé par ce module.
    static void openExploration(Player& player, DifficultyMode difficulty);
    // EN: openLocations declares or implements a focused behavior used by this module.
    // FR: openLocations déclare ou implémente un comportement précis utilisé par ce module.
    static void openLocations(Player& player);
    // EN: openNotableNpcMenu declares or implements a focused behavior used by this module.
    // FR: openNotableNpcMenu déclare ou implémente un comportement précis utilisé par ce module.
    static void openNotableNpcMenu(Player& player);
    // EN: maybeOfferRandomInterception declares or implements a focused behavior used by this module.
    // FR: maybeOfferRandomInterception déclare ou implémente un comportement précis utilisé par ce module.
    static void maybeOfferRandomInterception(Player& player, DifficultyMode difficulty);
};

#endif
