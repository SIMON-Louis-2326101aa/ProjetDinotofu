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
    int rewardExperience = 0;
    int rewardGold = 0;
    std::string rewardMaterialId;
    std::string rewardMaterialName;
    int rewardMaterialQuantity = 0;
    std::string rewardNote;
    std::string requiredMaterialId;
    std::string requiredMaterialName;
    int requiredMaterialQuantity = 0;
    int progress = 0;
    int target = 0;
    bool guildQuest = false;
    bool guildChallenge = false;
    std::string challengeCondition;
    int challengeMarkReward = 0;
    int availableFromDay = 0;
    int expiresAtDay = -1;
    bool accepted = false;
    bool completed = false;
    bool turnedIn = false;
    bool failed = false;
    std::string failureReason;

    // EN: Optional dependency metadata for multi-step or aggregate quests.
    // A pipe-separated list keeps save files simple and backward compatible.
    // FR: Métadonnées optionnelles pour les quêtes à étapes ou de synthèse.
    // Une liste séparée par | garde les sauvegardes simples et rétrocompatibles.
    std::string linkedQuestIds;
    std::string stageLabels;
    // EN: Pipe-separated ids of already completed service micro-challenges.
    // FR: Identifiants séparés par | des micro-épreuves de service déjà proposées.
    std::string serviceChallengeHistory;
    std::string linkedQuestRequiredState = "turned_in";
    bool retroactiveProgress = false;
    bool hideFutureSteps = false;
};

#endif
