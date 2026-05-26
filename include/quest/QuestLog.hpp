// EN: QuestLog.hpp briefly defines this Dinotofu module and its responsibilities.
// FR: QuestLog.hpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu.
// Description: Stores accepted guild quests and personal/client quests.

#ifndef INCLUDE_QUEST_QUESTLOG_HPP
#define INCLUDE_QUEST_QUESTLOG_HPP

#include "quest/Quest.hpp"
#include "item/Inventory.hpp"
#include "entity/Monster.hpp"

#include <string>
#include <vector>

class QuestLog
{
private:
    std::vector<Quest> quests;
    std::vector<Quest> guildBoardOffers;
    int guildBoardCreatedAtCombat;
    int guildBoardTargetSize;
    int guildBoardPendingReplacements;
    int guildBoardReplacementDueAtCombat;

public:
    QuestLog();
    // EN: getQuests declares or implements a focused behavior used by this module.
    // FR: getQuests déclare ou implémente un comportement précis utilisé par ce module.
    const std::vector<Quest>& getQuests() const;
    std::vector<Quest>& getQuests();

    // EN: hasQuest declares or implements a focused behavior used by this module.
    // FR: hasQuest déclare ou implémente un comportement précis utilisé par ce module.
    bool hasQuest(const std::string& questId) const;
    // EN: getActiveGuildQuestCount declares or implements a focused behavior used by this module.
    // FR: getActiveGuildQuestCount déclare ou implémente un comportement précis utilisé par ce module.
    int getActiveGuildQuestCount() const;
    // EN: getActivePersonalQuestCountForClient declares or implements a focused behavior used by this module.
    // FR: getActivePersonalQuestCountForClient déclare ou implémente un comportement précis utilisé par ce module.
    int getActivePersonalQuestCountForClient(const std::string& client) const;
    // EN: canAcceptGuildQuest declares or implements a focused behavior used by this module.
    // FR: canAcceptGuildQuest déclare ou implémente un comportement précis utilisé par ce module.
    bool canAcceptGuildQuest() const;
    // EN: canAcceptPersonalQuestForClient declares or implements a focused behavior used by this module.
    // FR: canAcceptPersonalQuestForClient déclare ou implémente un comportement précis utilisé par ce module.
    bool canAcceptPersonalQuestForClient(const std::string& client) const;
    // EN: addQuest declares or implements a focused behavior used by this module.
    // FR: addQuest déclare ou implémente un comportement précis utilisé par ce module.
    bool addQuest(const Quest& quest);
    // EN: progressQuest declares or implements a focused behavior used by this module.
    // FR: progressQuest déclare ou implémente un comportement précis utilisé par ce module.
    bool progressQuest(const std::string& questId, int amount);
    // EN: completeQuest declares or implements a focused behavior used by this module.
    // FR: completeQuest déclare ou implémente un comportement précis utilisé par ce module.
    bool completeQuest(const std::string& questId);
    // EN: progressCombatQuests declares or implements a focused behavior used by this module.
    // FR: progressCombatQuests déclare ou implémente un comportement précis utilisé par ce module.
    int progressCombatQuests(int defeatedEnemyCount);
    // EN: progressCombatQuestsByFamily declares or implements a focused behavior used by this module.
    // FR: progressCombatQuestsByFamily déclare ou implémente un comportement précis utilisé par ce module.
    int progressCombatQuestsByFamily(int defeatedEnemyCount, const std::string& encounterFamily);
    int progressCombatQuestsForMonster(const Monster& monster, int amount = 1);
    // EN: refreshMaterialDeliveryQuests declares or implements a focused behavior used by this module.
    // FR: refreshMaterialDeliveryQuests déclare ou implémente un comportement précis utilisé par ce module.
    int refreshMaterialDeliveryQuests(const class Inventory& inventory);
    // EN: turnInQuest declares or implements a focused behavior used by this module.
    // FR: turnInQuest déclare ou implémente un comportement précis utilisé par ce module.
    bool turnInQuest(const std::string& questId);
    // EN: hasTurnInReadyQuestForClient declares or implements a focused behavior used by this module.
    // FR: hasTurnInReadyQuestForClient déclare ou implémente un comportement précis utilisé par ce module.
    bool hasTurnInReadyQuestForClient(const std::string& client) const;
    // EN: clear declares or implements a focused behavior used by this module.
    // FR: clear déclare ou implémente un comportement précis utilisé par ce module.
    const std::vector<Quest>& getGuildBoardOffers() const;
    std::vector<Quest>& getGuildBoardOffers();
    void ensureGuildBoardReady(int playerLevel, int currentCombatsStarted);
    bool removeGuildBoardOfferAt(int offerIndex, int currentCombatsStarted);
    int getGuildBoardCombatsBeforeRefresh(int currentCombatsStarted) const;
    int getGuildBoardPendingReplacements() const;
    int getGuildBoardTargetSize() const;
    int getGuildBoardCreatedAtCombat() const;
    int getGuildBoardReplacementDueAtCombat() const;
    int getClientQuestCount(const std::string& client) const;
    bool hasRecommendedClientCapacity(const std::string& client) const;
    void setLoadedGuildBoardState(
        const std::vector<Quest>& offers,
        int createdAtCombat,
        int targetSize,
        int pendingReplacements,
        int replacementDueAtCombat
    );
    void clear();
};

#endif
