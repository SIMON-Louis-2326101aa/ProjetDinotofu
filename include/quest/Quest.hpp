// EN: Quest.hpp briefly defines this Dinotofu module and its responsibilities.
// FR: Quest.hpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu.
// Description: Lightweight quest record used by the terminal quest journal.

#ifndef INCLUDE_QUEST_QUEST_HPP
#define INCLUDE_QUEST_QUEST_HPP

#include <string>

struct Quest
{
    std::string id;
    std::string rank;
    std::string title;
    std::string origin;
    std::string client;
    std::string location;
    std::string objective;
    std::string objectiveType;
    std::string targetFamily;
    int rewardExperience;
    int rewardGold;
    std::string requiredMaterialId;
    std::string requiredMaterialName;
    int requiredMaterialQuantity;
    int progress;
    int target;
    bool guildQuest;
    bool accepted;
    bool completed;
    bool turnedIn;
};

#endif
