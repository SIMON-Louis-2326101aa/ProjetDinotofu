// EN: Quest.hpp briefly defines this Dinotofu module and its responsibilities.
// FR: Quest.hpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu.
// Description: Lightweight quest record used by the terminal quest journal.

#ifndef INCLUDE_QUEST_QUEST_HPP
#define INCLUDE_QUEST_QUEST_HPP

#include <string>
#include "economy/Money.hpp"

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
    // Historical field name kept in saves: this is an economy-unit reward (1 unit = 1 PF), not PO.
    int rewardGold = 0;
    // Optional exact physical payout. When at least one stack is non-zero, these exact pieces
    // are awarded as authored and rewardGold is not converted or substituted.
    CoinBreakdown rewardCoins;
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
    // Optional foreign-language source attached to a quest contract.
    std::string requiredLanguage;
    int requiredLanguageLevel = 0;
    std::string sourceLanguageText;

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
