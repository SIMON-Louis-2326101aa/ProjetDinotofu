// EN: Player.hpp briefly defines this Dinotofu module and its responsibilities.
// FR: Player.hpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#ifndef INCLUDE_ENTITY_PLAYER_HPP
#define INCLUDE_ENTITY_PLAYER_HPP

#include "entity/Entity.hpp"
#include "class_system/PlayerClass.hpp"
#include "item/Inventory.hpp"
#include "progression/DifficultyMode.hpp"
#include "character/CharacterRace.hpp"
#include "progression/DndAttributes.hpp"
#include "quest/QuestLog.hpp"

#include <string>
#include <vector>

struct PlayerLocalSubscription
{
    std::string id;
    std::string name;
    int expiresAtDay = -1;
    bool cancellationRequested = false;
    int renewalPrice = 0;
};

struct PlayerCurse
{
    std::string id;
    std::string name;
    std::string severity;
    std::string origin;
    std::string description;
    std::string removalHint;
    std::string symptomCategories;
    std::string discoveredSymptomCategories;
    std::string excludedSymptomCategories;
    int diagnosisLevel = 0;
    int appliedAtDay = 0;
    int expiresAtDay = -1;
    int exorcismProgress = 0;
    int exorcismRequiredVisits = 0;
    int curseLevel = 1;
    int maxCurseLevel = 1;
    bool evolvesOverTime = false;
    int escalationIntervalDays = 0;
    int nextEscalationDay = -1;
    int churchRemovalMaxLevel = 99;
    bool becomesSpecialRemovalWhenTooHigh = false;
    std::string highLevelRemovalHint;
    bool removableByChurch = false;
    int bossIdRequiredToBreak = 0;
    bool lifeLong = false;
};

class Player : public Entity
{
public:
    static const int MAX_LEVEL = 255;

private:
    int level;
    int experience;

    Inventory inventory;
    int equippedWeaponIndex;
    int equippedArmorIndex;
    CharacterRace race;
    DndAttributes attributes;
    QuestLog questLog;
    int unspentAttributePoints;

    std::vector<std::string> unlockedPassiveSkills;
    std::vector<std::string> unlockedActiveSkills;
    int daggerKillProgress;
    int bowKillProgress;
    int bareHandKillProgress;
    int staffKillProgress;
    int swordKillProgress;
    int axeKillProgress;
    int hammerKillProgress;
    int spearKillProgress;

    int combatsStarted;
    int worldDaysElapsed;
    int worldDayProgressUnits;
    std::vector<PlayerLocalSubscription> localSubscriptions;
    std::vector<PlayerCurse> activeCurses;
    int localSubscriptionRenewalPaidThisWeek;
    std::vector<std::string> pendingWorldTimeReportLines;
    int victories;
    int defeats;
    int escapes;
    int deaths;
    int enemiesKilled;
    int bossesKilled;
    int pvpVictories;
    int pvpDefeats;
    std::vector<std::string> pvpLethalEliminations;
    std::vector<int> unlockedBossIds;
    std::vector<int> recentBossIds;
    std::vector<int> defeatedBossIds;
    int recentBossCooldownExpiresAtDay;
    std::vector<std::string> recentCombatEquipmentUsage;
    bool bossEquipmentSealActive;
    std::string bossEquipmentSealReason;

    bool zelefCorrosionPresent;
    int zelefMaxHpStolen;
    bool grinkaBossTheftPresent;
    int grinkaStolenGold;
    bool grinkaHasStolenWeapon;
    bool grinkaHasStolenArmor;
    Weapon grinkaStolenWeapon;
    Armor grinkaStolenArmor;

    bool cloneCharacter;

    bool alteredByCheats;
    bool godModeEnabled;
    bool infiniteConsumablesEnabled;
    bool indestructibleEquipmentEnabled;
    bool equipmentProtectionEnabled;
    bool storySkipEnabled;
    bool specialChallengeAccessUnlocked;

    bool godModeKnown;
    bool infiniteConsumablesKnown;
    bool indestructibleEquipmentKnown;
    bool equipmentProtectionKnown;
    bool storySkipKnown;
    bool specialChallengeAccessKnown;
    bool creatorMessageKnown;

    std::string nextAmmunitionChoiceId;
    std::string lastConsumedAmmunitionId;

    int goldCheatUseCount;
    int levelCheatUseCount;
    int maxLevelCheatUseCount;
    int refundCheatUseCount;
    int resetCheatUseCount;
    int switchClassCheatUseCount;
    int lethalCheatAttemptCount;
    int worldGazeCombatsRemaining;
    int worldGazeMaxHpPenalty;

    int refundUsesRemaining;

    std::string createdAtText;
    std::string createdForVersion;
    std::string lastAdaptedVersion;
    std::string creatorAccountName;
    std::string currentOwnerAccountName;
    std::vector<std::string> starterKitLog;
    std::vector<std::string> titles;
    std::string activeTitle;
    std::vector<std::string> activeTitles;
    std::string interfaceHintFrequency;
    int storyChapter;
    int storyStep;
    int storyCityDevelopmentLevel;
    bool storyModeStarted;
    // EN: reduceWorldGazeDurationAfterCombat declares or implements a focused behavior used by this module.
    // FR: reduceWorldGazeDurationAfterCombat déclare ou implémente un comportement précis utilisé par ce module.
    void reduceWorldGazeDurationAfterCombat();

    // EN: getEquippedArmorMaxHpBonus declares or implements a focused behavior used by this module.
    // FR: getEquippedArmorMaxHpBonus déclare ou implémente un comportement précis utilisé par ce module.
    int getEquippedArmorMaxHpBonus() const;
    // EN: applyRaceStartingBonus declares or implements a focused behavior used by this module.
    // FR: applyRaceStartingBonus déclare ou implémente un comportement précis utilisé par ce module.
    void applyRaceStartingBonus(CharacterRace selectedRace);
    void processEndOfWorldDay();
    void maybeAppendCurseMalaiseLine(int salt);
    void appendWeeklyRenewalSummaryIfNeeded();

public:
    // EN: Player declares or implements a focused behavior used by this module.
    // FR: Player déclare ou implémente un comportement précis utilisé par ce module.
    Player();

    Player(
        const std::string& name,
        const PlayerClass& playerClass
    );

    // EN: getLevel declares or implements a focused behavior used by this module.
    // FR: getLevel déclare ou implémente un comportement précis utilisé par ce module.
    int getLevel() const;
    // EN: getExperience declares or implements a focused behavior used by this module.
    // FR: getExperience déclare ou implémente un comportement précis utilisé par ce module.
    int getExperience() const;
    // EN: getUnspentAttributePoints declares or implements a focused behavior used by this module.
    // FR: getUnspentAttributePoints déclare ou implémente un comportement précis utilisé par ce module.
    int getUnspentAttributePoints() const;
    // EN: getAttributes declares or implements a focused behavior used by this module.
    // FR: getAttributes déclare ou implémente un comportement précis utilisé par ce module.
    const DndAttributes& getAttributes() const;

    // EN: getRace declares or implements a focused behavior used by this module.
    // FR: getRace déclare ou implémente un comportement précis utilisé par ce module.
    CharacterRace getRace() const;
    std::string getRaceText() const;
    const std::string& getCreatedAtText() const;
    const std::string& getCreatedForVersion() const;
    const std::string& getLastAdaptedVersion() const;
    void setVersionMetadata(const std::string& createdAt, const std::string& createdFor, const std::string& lastAdapted);

    const std::vector<std::string>& getTitles() const;
    const std::string& getActiveTitle() const;
    const std::vector<std::string>& getActiveTitles() const;
    std::string getActiveTitleSummary() const;
    std::vector<std::string> describeActiveTitleEffects() const;
    const std::string& getInterfaceHintFrequency() const;
    std::string getInterfaceHintFrequencyLabel() const;
    void setInterfaceHintFrequency(const std::string& frequency);
    bool areInterfaceHintsDisabled() const;
    bool areInterfaceHintsFrequent() const;
    int getStoryChapter() const;
    int getStoryStep() const;
    int getStoryCityDevelopmentLevel() const;
    bool hasStoryModeStarted() const;
    std::string getStoryProgressLabel() const;
    void setLoadedStoryProgress(int chapter, int step, int cityDevelopment, bool started);
    bool startStoryMode();
    bool setStoryProgress(int chapter, int step, int cityDevelopment);
    bool hasTitle(const std::string& title) const;
    bool grantTitle(const std::string& title);
    bool setActiveTitle(const std::string& title);
    bool setActiveTitleSlot(int slotIndex, const std::string& title);
    bool unequipActiveTitleSlot(int slotIndex);
    bool equipTitle(const std::string& title);
    void setLoadedTitles(const std::vector<std::string>& loadedTitles, const std::string& loadedActiveTitle);
    void setLoadedTitles(const std::vector<std::string>& loadedTitles, const std::string& loadedActiveTitle, const std::vector<std::string>& loadedActiveTitles);
    void markAdaptedToCurrentVersion();
    const std::string& getCreatorAccountName() const;
    const std::string& getCurrentOwnerAccountName() const;
    void setOwnershipMetadata(const std::string& creatorAccount, const std::string& currentOwnerAccount);
    const std::vector<std::string>& getStarterKitLog() const;
    void setLoadedStarterKitLog(const std::vector<std::string>& log);
    void recordStarterKitEntry(const std::string& entry);
    std::vector<std::string> applyHeavyVersionAdaptation(DifficultyMode difficulty);
    // EN: setRace declares or implements a focused behavior used by this module.
    // FR: setRace déclare ou implémente un comportement précis utilisé par ce module.
    void setRace(CharacterRace selectedRace);
    // EN: applyFlatStatBonus declares or implements a focused behavior used by this module.
    // FR: applyFlatStatBonus déclare ou implémente un comportement précis utilisé par ce module.
    void applyFlatStatBonus(int maxHpBonus, int minDamageBonus, int maxDamageBonus, int criticalDamageBonus);
    // EN: setLoadedProgress declares or implements a focused behavior used by this module.
    // FR: setLoadedProgress déclare ou implémente un comportement précis utilisé par ce module.
    void setLoadedProgress(int loadedLevel, int loadedExperience, int loadedHp);
    void setLoadedCombatStats(
        int loadedMaxHp,
        int loadedMinDamage,
        int loadedMaxDamage,
        int loadedCriticalDamage,
        int loadedHp
    );
    // EN: setLoadedAttributes declares or implements a focused behavior used by this module.
    // FR: setLoadedAttributes déclare ou implémente un comportement précis utilisé par ce module.
    void setLoadedAttributes(const DndAttributes& loadedAttributes, int loadedUnspentPoints);
    // EN: spendAttributePoint declares or implements a focused behavior used by this module.
    // FR: spendAttributePoint déclare ou implémente un comportement précis utilisé par ce module.
    bool spendAttributePoint(int attributeChoice);
    // EN: displayAttributes declares or implements a focused behavior used by this module.
    // FR: displayAttributes déclare ou implémente un comportement précis utilisé par ce module.
    void displayAttributes() const;

    // EN: getUnlockedPassiveSkills declares or implements a focused behavior used by this module.
    // FR: getUnlockedPassiveSkills déclare ou implémente un comportement précis utilisé par ce module.
    const std::vector<std::string>& getUnlockedPassiveSkills() const;
    // EN: getUnlockedActiveSkills declares or implements a focused behavior used by this module.
    // FR: getUnlockedActiveSkills déclare ou implémente un comportement précis utilisé par ce module.
    const std::vector<std::string>& getUnlockedActiveSkills() const;
    // EN: getDaggerKillProgress declares or implements a focused behavior used by this module.
    // FR: getDaggerKillProgress déclare ou implémente un comportement précis utilisé par ce module.
    int getDaggerKillProgress() const;
    // EN: getBowKillProgress declares or implements a focused behavior used by this module.
    // FR: getBowKillProgress déclare ou implémente un comportement précis utilisé par ce module.
    int getBowKillProgress() const;
    // EN: getBareHandKillProgress declares or implements a focused behavior used by this module.
    // FR: getBareHandKillProgress déclare ou implémente un comportement précis utilisé par ce module.
    int getBareHandKillProgress() const;
    // EN: getStaffKillProgress declares or implements a focused behavior used by this module.
    // FR: getStaffKillProgress déclare ou implémente un comportement précis utilisé par ce module.
    int getStaffKillProgress() const;
    int getSwordKillProgress() const;
    int getAxeKillProgress() const;
    int getHammerKillProgress() const;
    int getSpearKillProgress() const;
    // EN: hasPassiveSkill declares or implements a focused behavior used by this module.
    // FR: hasPassiveSkill déclare ou implémente un comportement précis utilisé par ce module.
    bool hasPassiveSkill(const std::string& skillId) const;
    // EN: hasActiveSkill declares or implements a focused behavior used by this module.
    // FR: hasActiveSkill déclare ou implémente un comportement précis utilisé par ce module.
    bool hasActiveSkill(const std::string& skillId) const;
    // EN: unlockPassiveSkill declares or implements a focused behavior used by this module.
    // FR: unlockPassiveSkill déclare ou implémente un comportement précis utilisé par ce module.
    bool unlockPassiveSkill(const std::string& skillId, const std::string& skillName);
    // EN: unlockActiveSkill declares or implements a focused behavior used by this module.
    // FR: unlockActiveSkill déclare ou implémente un comportement précis utilisé par ce module.
    bool unlockActiveSkill(const std::string& skillId, const std::string& skillName);
    // EN: recordGameplaySkillProgressForKills declares or implements a focused behavior used by this module.
    // FR: recordGameplaySkillProgressForKills déclare ou implémente un comportement précis utilisé par ce module.
    void recordGameplaySkillProgressForKills(int amount);
    // EN: refreshLevelAndIdentitySkills declares or implements a focused behavior used by this module.
    // FR: refreshLevelAndIdentitySkills déclare ou implémente un comportement précis utilisé par ce module.
    void refreshLevelAndIdentitySkills();
    // EN: refreshCareerSkillProgress unlocks passive skills from long-term play statistics.
    // FR: refreshCareerSkillProgress débloque des passifs selon les statistiques durables du personnage.
    void refreshCareerSkillProgress();
    void setLoadedSkillState(
        const std::vector<std::string>& passiveSkills,
        const std::vector<std::string>& activeSkills,
        int daggerProgress,
        int bowProgress,
        int bareHandProgress,
        int staffProgress,
        int swordProgress,
        int axeProgress,
        int hammerProgress,
        int spearProgress
    );
    // EN: displaySkillProgress declares or implements a focused behavior used by this module.
    // FR: displaySkillProgress déclare ou implémente un comportement précis utilisé par ce module.
    void displaySkillProgress() const;

    // EN: getCombatsStarted declares or implements a focused behavior used by this module.
    // FR: getCombatsStarted déclare ou implémente un comportement précis utilisé par ce module.
    int getCombatsStarted() const;
    int getWorldDaysElapsed() const;
    int getWorldDayProgressUnits() const;
    int getWorldDayUnitsPerDay() const;
    int getCurrentWorldDayNumber() const;
    std::string getCurrentWeekdayName() const;
    std::string getCurrentDayPartName() const;
    std::string formatWorldDateLine() const;
    std::string formatWorldDayPartLine() const;
    std::string formatWorldDateTimeLine() const;
    std::string formatWorldTimeChange(int beforeDay, int beforeProgress) const;
    void advanceWorldDays(int days);
    void advanceWorldDayUnits(int units);

    const std::vector<PlayerLocalSubscription>& getLocalSubscriptions() const;
    bool hasActiveLocalSubscription(const std::string& subscriptionId) const;
    bool isLocalSubscriptionCancellationRequested(const std::string& subscriptionId) const;
    int getLocalSubscriptionExpiresAtDay(const std::string& subscriptionId) const;
    void activateLocalSubscription(const std::string& subscriptionId, const std::string& name, int durationDays = 7, int renewalPrice = 0);
    bool requestLocalSubscriptionCancellation(const std::string& subscriptionId);
    void removeExpiredLocalSubscriptions();
    void setLoadedLocalSubscriptions(const std::vector<PlayerLocalSubscription>& subscriptions);

    const std::vector<PlayerCurse>& getActiveCurses() const;
    int getActiveCurseCount() const;
    bool hasActiveCurse(const std::string& curseId) const;
    bool addOrRefreshCurse(const PlayerCurse& curse);
    bool removeCurse(const std::string& curseId);
    int removeExpiredCurses();
    void setLoadedCurses(const std::vector<PlayerCurse>& curses);
    bool advanceChurchExorcism(const std::string& curseId);
    bool revealCurseSymptomCategory(const std::string& curseId, const std::string& category);
    bool excludeCurseSymptomCategory(const std::string& curseId, const std::string& category);
    int autoExcludeWrongCurseSymptomCategories(const std::string& curseId, int percentToExclude);
    bool setCurseDiagnosisLevel(const std::string& curseId, int level);
    int processCurseEscalations();
    int removeCursesLockedByBoss(int bossId);
    bool hasCurseEligibleForSpecialSolution(const std::string& solutionId) const;
    int resolveSpecialCurseSolution(const std::string& solutionId);
    int getCursePressureForCategory(const std::string& category) const;
    int getKnownCursePressureForCategory(const std::string& category) const;
    bool hasHighRunicBacklashNeedingEnchanter() const;
    bool stabilizeHighRunicBacklashForEnchanter();
    std::vector<std::string> describeActiveCurses() const;
    void recordLyknirDefeatCurse();

    int getLocalSubscriptionRenewalPaidThisWeek() const;
    void setLocalSubscriptionRenewalPaidThisWeek(int amount);
    const std::vector<std::string>& getPendingWorldTimeReportLines() const;
    std::vector<std::string> consumeWorldTimeReportLines();
    // EN: getVictories declares or implements a focused behavior used by this module.
    // FR: getVictories déclare ou implémente un comportement précis utilisé par ce module.
    int getVictories() const;
    // EN: getDefeats declares or implements a focused behavior used by this module.
    // FR: getDefeats déclare ou implémente un comportement précis utilisé par ce module.
    int getDefeats() const;
    // EN: getEscapes declares or implements a focused behavior used by this module.
    // FR: getEscapes déclare ou implémente un comportement précis utilisé par ce module.
    int getEscapes() const;
    // EN: getDeaths declares or implements a focused behavior used by this module.
    // FR: getDeaths déclare ou implémente un comportement précis utilisé par ce module.
    int getDeaths() const;
    // EN: getEnemiesKilled declares or implements a focused behavior used by this module.
    // FR: getEnemiesKilled déclare ou implémente un comportement précis utilisé par ce module.
    int getEnemiesKilled() const;
    // EN: getBossesKilled declares or implements a focused behavior used by this module.
    // FR: getBossesKilled déclare ou implémente un comportement précis utilisé par ce module.
    int getBossesKilled() const;
    // EN: getPvpVictories declares or implements a focused behavior used by this module.
    // FR: getPvpVictories déclare ou implémente un comportement précis utilisé par ce module.
    int getPvpVictories() const;
    // EN: getPvpDefeats declares or implements a focused behavior used by this module.
    // FR: getPvpDefeats déclare ou implémente un comportement précis utilisé par ce module.
    int getPvpDefeats() const;
    // EN: getPvpLethalEliminations declares or implements a focused behavior used by this module.
    // FR: getPvpLethalEliminations déclare ou implémente un comportement précis utilisé par ce module.
    const std::vector<std::string>& getPvpLethalEliminations() const;
    // EN: getUnlockedBossIds declares or implements a focused behavior used by this module.
    // FR: getUnlockedBossIds déclare ou implémente un comportement précis utilisé par ce module.
    const std::vector<int>& getUnlockedBossIds() const;
    // EN: getRecentBossIds declares or implements a focused behavior used by this module.
    // FR: getRecentBossIds déclare ou implémente un comportement précis utilisé par ce module.
    const std::vector<int>& getRecentBossIds() const;
    int getRecentBossCooldownExpiresAtDay() const;
    // EN: getRecentCombatEquipmentUsage declares or implements a focused behavior used by this module.
    // FR: getRecentCombatEquipmentUsage déclare ou implémente un comportement précis utilisé par ce module.
    const std::vector<std::string>& getRecentCombatEquipmentUsage() const;
    // EN: recordCurrentEquipmentUsage declares or implements a focused behavior used by this module.
    // FR: recordCurrentEquipmentUsage déclare ou implémente un comportement précis utilisé par ce module.
    void recordCurrentEquipmentUsage();
    // EN: setLoadedRecentCombatEquipmentUsage declares or implements a focused behavior used by this module.
    // FR: setLoadedRecentCombatEquipmentUsage déclare ou implémente un comportement précis utilisé par ce module.
    void setLoadedRecentCombatEquipmentUsage(const std::vector<std::string>& usage);
    // EN: hasBossEquipmentSeal declares or implements a focused behavior used by this module.
    // FR: hasBossEquipmentSeal déclare ou implémente un comportement précis utilisé par ce module.
    bool hasBossEquipmentSeal() const;
    std::string getBossEquipmentSealReason() const;
    // EN: activateBossEquipmentSeal declares or implements a focused behavior used by this module.
    // FR: activateBossEquipmentSeal déclare ou implémente un comportement précis utilisé par ce module.
    void activateBossEquipmentSeal(const std::string& reason);
    // EN: clearBossEquipmentSeal declares or implements a focused behavior used by this module.
    // FR: clearBossEquipmentSeal déclare ou implémente un comportement précis utilisé par ce module.
    void clearBossEquipmentSeal();
    std::vector<int> getAvailableBossIds() const;
    // EN: isBossUnlocked declares or implements a focused behavior used by this module.
    // FR: isBossUnlocked déclare ou implémente un comportement précis utilisé par ce module.
    bool isBossUnlocked(int bossId) const;
    // EN: isBossRecentlyDefeated declares or implements a focused behavior used by this module.
    // FR: isBossRecentlyDefeated déclare ou implémente un comportement précis utilisé par ce module.
    bool isBossRecentlyDefeated(int bossId) const;
    bool isBossDefeated(int bossId) const;
    const std::vector<int>& getDefeatedBossIds() const;
    // EN: unlockNextBossVariation declares or implements a focused behavior used by this module.
    // FR: unlockNextBossVariation déclare ou implémente un comportement précis utilisé par ce module.
    bool unlockNextBossVariation();
    // EN: recordBossVictoryInRegistry declares or implements a focused behavior used by this module.
    // FR: recordBossVictoryInRegistry déclare ou implémente un comportement précis utilisé par ce module.
    bool recordBossVictoryInRegistry(int bossId);
    // EN: setLoadedBossRegistry declares or implements a focused behavior used by this module.
    // FR: setLoadedBossRegistry déclare ou implémente un comportement précis utilisé par ce module.
    void setLoadedBossRegistry(const std::vector<int>& unlockedIds, const std::vector<int>& recentIds, const std::vector<int>& defeatedIds = {}, int recentCooldownExpiresAtDay = -1);

    // EN: hasZelefCorrosionPresent declares or implements a focused behavior used by this module.
    // FR: hasZelefCorrosionPresent déclare ou implémente un comportement précis utilisé par ce module.
    bool hasZelefCorrosionPresent() const;
    // EN: getZelefMaxHpStolen declares or implements a focused behavior used by this module.
    // FR: getZelefMaxHpStolen déclare ou implémente un comportement précis utilisé par ce module.
    int getZelefMaxHpStolen() const;
    // EN: hasGrinkaBossTheftPresent declares or implements a focused behavior used by this module.
    // FR: hasGrinkaBossTheftPresent déclare ou implémente un comportement précis utilisé par ce module.
    bool hasGrinkaBossTheftPresent() const;
    // EN: getGrinkaStolenGold declares or implements a focused behavior used by this module.
    // FR: getGrinkaStolenGold déclare ou implémente un comportement précis utilisé par ce module.
    int getGrinkaStolenGold() const;
    // EN: hasGrinkaStolenWeapon declares or implements a focused behavior used by this module.
    // FR: hasGrinkaStolenWeapon déclare ou implémente un comportement précis utilisé par ce module.
    bool hasGrinkaStolenWeapon() const;
    // EN: hasGrinkaStolenArmor declares or implements a focused behavior used by this module.
    // FR: hasGrinkaStolenArmor déclare ou implémente un comportement précis utilisé par ce module.
    bool hasGrinkaStolenArmor() const;
    // EN: getGrinkaStolenWeapon declares or implements a focused behavior used by this module.
    // FR: getGrinkaStolenWeapon déclare ou implémente un comportement précis utilisé par ce module.
    Weapon getGrinkaStolenWeapon() const;
    // EN: getGrinkaStolenArmor declares or implements a focused behavior used by this module.
    // FR: getGrinkaStolenArmor déclare ou implémente un comportement précis utilisé par ce module.
    Armor getGrinkaStolenArmor() const;
    // EN: recordZelefCorrosionLoss declares or implements a focused behavior used by this module.
    // FR: recordZelefCorrosionLoss déclare ou implémente un comportement précis utilisé par ce module.
    void recordZelefCorrosionLoss(int maxHpLoss);
    // EN: restoreZelefCorrosionLoss declares or implements a focused behavior used by this module.
    // FR: restoreZelefCorrosionLoss déclare ou implémente un comportement précis utilisé par ce module.
    void restoreZelefCorrosionLoss();
    // EN: recordGrinkaGoldTheft declares or implements a focused behavior used by this module.
    // FR: recordGrinkaGoldTheft déclare ou implémente un comportement précis utilisé par ce module.
    void recordGrinkaGoldTheft(int goldAmount);
    // EN: recordGrinkaStolenWeapon declares or implements a focused behavior used by this module.
    // FR: recordGrinkaStolenWeapon déclare ou implémente un comportement précis utilisé par ce module.
    void recordGrinkaStolenWeapon(const Weapon& weapon);
    // EN: recordGrinkaStolenArmor declares or implements a focused behavior used by this module.
    // FR: recordGrinkaStolenArmor déclare ou implémente un comportement précis utilisé par ce module.
    void recordGrinkaStolenArmor(const Armor& armor);
    // EN: restoreGrinkaBossTheft declares or implements a focused behavior used by this module.
    // FR: restoreGrinkaBossTheft déclare ou implémente un comportement précis utilisé par ce module.
    void restoreGrinkaBossTheft();
    void setLoadedBossConsequences(
        bool zelefCorrosion,
        int zelefMaxHpLoss,
        bool grinkaTheft,
        int grinkaGoldLoss,
        bool hasStolenWeapon,
        const Weapon& stolenWeapon,
        bool hasStolenArmor,
        const Armor& stolenArmor
    );

    // EN: isClone declares or implements a focused behavior used by this module.
    // FR: isClone déclare ou implémente un comportement précis utilisé par ce module.
    bool isClone() const;
    // EN: setClone declares or implements a focused behavior used by this module.
    // FR: setClone déclare ou implémente un comportement précis utilisé par ce module.
    void setClone(bool clone);

    void setLoadedStatistics(
        int loadedCombatsStarted,
        int loadedWorldDaysElapsed,
        int loadedWorldDayProgressUnits,
        int loadedVictories,
        int loadedDefeats,
        int loadedEscapes,
        int loadedDeaths,
        int loadedEnemiesKilled,
        int loadedBossesKilled,
        int loadedPvpVictories = 0,
        int loadedPvpDefeats = 0
    );

    // EN: recordCombatStarted declares or implements a focused behavior used by this module.
    // FR: recordCombatStarted déclare ou implémente un comportement précis utilisé par ce module.
    void recordCombatStarted();
    // EN: recordVictory declares or implements a focused behavior used by this module.
    // FR: recordVictory déclare ou implémente un comportement précis utilisé par ce module.
    void recordVictory();
    // EN: recordDefeat declares or implements a focused behavior used by this module.
    // FR: recordDefeat déclare ou implémente un comportement précis utilisé par ce module.
    void recordDefeat();
    // EN: recordEscape declares or implements a focused behavior used by this module.
    // FR: recordEscape déclare ou implémente un comportement précis utilisé par ce module.
    void recordEscape();
    // EN: recordDeath declares or implements a focused behavior used by this module.
    // FR: recordDeath déclare ou implémente un comportement précis utilisé par ce module.
    void recordDeath();
    // EN: recordEnemyKills declares or implements a focused behavior used by this module.
    // FR: recordEnemyKills déclare ou implémente un comportement précis utilisé par ce module.
    void recordEnemyKills(int amount);
    // EN: recordBossKill declares or implements a focused behavior used by this module.
    // FR: recordBossKill déclare ou implémente un comportement précis utilisé par ce module.
    void recordBossKill();
    // EN: recordPvpVictory declares or implements a focused behavior used by this module.
    // FR: recordPvpVictory déclare ou implémente un comportement précis utilisé par ce module.
    void recordPvpVictory();
    // EN: recordPvpDefeat declares or implements a focused behavior used by this module.
    // FR: recordPvpDefeat déclare ou implémente un comportement précis utilisé par ce module.
    void recordPvpDefeat();
    // EN: recordPvpLethalElimination declares or implements a focused behavior used by this module.
    // FR: recordPvpLethalElimination déclare ou implémente un comportement précis utilisé par ce module.
    void recordPvpLethalElimination(const std::string& characterName, const std::string& accountName);
    // EN: setLoadedPvpLethalEliminations declares or implements a focused behavior used by this module.
    // FR: setLoadedPvpLethalEliminations déclare ou implémente un comportement précis utilisé par ce module.
    void setLoadedPvpLethalEliminations(const std::vector<std::string>& eliminations);
    // EN: displayCareerStatistics declares or implements a focused behavior used by this module.
    // FR: displayCareerStatistics déclare ou implémente un comportement précis utilisé par ce module.
    void displayCareerStatistics(DifficultyMode difficulty) const;

    // EN: isAlteredByCheats declares or implements a focused behavior used by this module.
    // FR: isAlteredByCheats déclare ou implémente un comportement précis utilisé par ce module.
    bool isAlteredByCheats() const;
    // EN: isGodModeEnabled declares or implements a focused behavior used by this module.
    // FR: isGodModeEnabled déclare ou implémente un comportement précis utilisé par ce module.
    bool isGodModeEnabled() const;
    // EN: hasInfiniteConsumables declares or implements a focused behavior used by this module.
    // FR: hasInfiniteConsumables déclare ou implémente un comportement précis utilisé par ce module.
    bool hasInfiniteConsumables() const;
    // EN: hasIndestructibleEquipment declares or implements a focused behavior used by this module.
    // FR: hasIndestructibleEquipment déclare ou implémente un comportement précis utilisé par ce module.
    bool hasIndestructibleEquipment() const;
    // EN: hasEquipmentProtection declares or implements a focused behavior used by this module.
    // FR: hasEquipmentProtection déclare ou implémente un comportement précis utilisé par ce module.
    bool hasEquipmentProtection() const;
    // EN: hasStorySkip declares or implements a focused behavior used by this module.
    // FR: hasStorySkip déclare ou implémente un comportement précis utilisé par ce module.
    bool hasStorySkip() const;
    // EN: hasSpecialChallengeAccess declares or implements a focused behavior used by this module.
    // FR: hasSpecialChallengeAccess déclare ou implémente un comportement précis utilisé par ce module.
    bool hasSpecialChallengeAccess() const;
    // EN: getRefundUsesRemaining declares or implements a focused behavior used by this module.
    // FR: getRefundUsesRemaining déclare ou implémente un comportement précis utilisé par ce module.
    int getRefundUsesRemaining() const;

    // EN: isGodModeKnown declares or implements a focused behavior used by this module.
    // FR: isGodModeKnown déclare ou implémente un comportement précis utilisé par ce module.
    bool isGodModeKnown() const;
    // EN: isInfiniteConsumablesKnown declares or implements a focused behavior used by this module.
    // FR: isInfiniteConsumablesKnown déclare ou implémente un comportement précis utilisé par ce module.
    bool isInfiniteConsumablesKnown() const;
    // EN: isIndestructibleEquipmentKnown declares or implements a focused behavior used by this module.
    // FR: isIndestructibleEquipmentKnown déclare ou implémente un comportement précis utilisé par ce module.
    bool isIndestructibleEquipmentKnown() const;
    // EN: isEquipmentProtectionKnown declares or implements a focused behavior used by this module.
    // FR: isEquipmentProtectionKnown déclare ou implémente un comportement précis utilisé par ce module.
    bool isEquipmentProtectionKnown() const;
    // EN: isStorySkipKnown declares or implements a focused behavior used by this module.
    // FR: isStorySkipKnown déclare ou implémente un comportement précis utilisé par ce module.
    bool isStorySkipKnown() const;
    // EN: isSpecialChallengeAccessKnown declares or implements a focused behavior used by this module.
    // FR: isSpecialChallengeAccessKnown déclare ou implémente un comportement précis utilisé par ce module.
    bool isSpecialChallengeAccessKnown() const;
    // EN: isCreatorMessageKnown declares or implements a focused behavior used by this module.
    // FR: isCreatorMessageKnown déclare ou implémente un comportement précis utilisé par ce module.
    bool isCreatorMessageKnown() const;

    // EN: getGoldCheatUseCount declares or implements a focused behavior used by this module.
    // FR: getGoldCheatUseCount déclare ou implémente un comportement précis utilisé par ce module.
    int getGoldCheatUseCount() const;
    // EN: getLevelCheatUseCount declares or implements a focused behavior used by this module.
    // FR: getLevelCheatUseCount déclare ou implémente un comportement précis utilisé par ce module.
    int getLevelCheatUseCount() const;
    // EN: getMaxLevelCheatUseCount declares or implements a focused behavior used by this module.
    // FR: getMaxLevelCheatUseCount déclare ou implémente un comportement précis utilisé par ce module.
    int getMaxLevelCheatUseCount() const;
    // EN: getRefundCheatUseCount declares or implements a focused behavior used by this module.
    // FR: getRefundCheatUseCount déclare ou implémente un comportement précis utilisé par ce module.
    int getRefundCheatUseCount() const;
    // EN: getResetCheatUseCount declares or implements a focused behavior used by this module.
    // FR: getResetCheatUseCount déclare ou implémente un comportement précis utilisé par ce module.
    int getResetCheatUseCount() const;
    // EN: getSwitchClassCheatUseCount declares or implements a focused behavior used by this module.
    // FR: getSwitchClassCheatUseCount déclare ou implémente un comportement précis utilisé par ce module.
    int getSwitchClassCheatUseCount() const;
    // EN: getLethalCheatAttemptCount declares or implements a focused behavior used by this module.
    // FR: getLethalCheatAttemptCount déclare ou implémente un comportement précis utilisé par ce module.
    int getLethalCheatAttemptCount() const;
    // EN: getWorldGazeCombatsRemaining declares or implements a focused behavior used by this module.
    // FR: getWorldGazeCombatsRemaining déclare ou implémente un comportement précis utilisé par ce module.
    int getWorldGazeCombatsRemaining() const;
    // EN: getWorldGazeMaxHpPenalty declares or implements a focused behavior used by this module.
    // FR: getWorldGazeMaxHpPenalty déclare ou implémente un comportement précis utilisé par ce module.
    int getWorldGazeMaxHpPenalty() const;
    // EN: hasWorldGazePenalty declares or implements a focused behavior used by this module.
    // FR: hasWorldGazePenalty déclare ou implémente un comportement précis utilisé par ce module.
    bool hasWorldGazePenalty() const;

    // EN: setLethalCheatPenaltyState declares or implements a focused behavior used by this module.
    // FR: setLethalCheatPenaltyState déclare ou implémente un comportement précis utilisé par ce module.
    void setLethalCheatPenaltyState(int attempts, int remainingCombats, int maxHpPenalty);
    // EN: applyLethalCheatAttemptPenalty declares or implements a focused behavior used by this module.
    // FR: applyLethalCheatAttemptPenalty déclare ou implémente un comportement précis utilisé par ce module.
    void applyLethalCheatAttemptPenalty();

    void setCheatState(
        bool altered,
        bool godMode,
        bool infiniteConsumables,
        bool indestructibleEquipment,
        bool equipmentProtection,
        bool storySkip,
        bool specialChallengeAccess,
        int refundUses
    );
    void setCheatKnowledgeState(
        bool godModeWasKnown,
        bool infiniteConsumablesWasKnown,
        bool indestructibleEquipmentWasKnown,
        bool equipmentProtectionWasKnown,
        bool storySkipWasKnown,
        bool specialChallengeAccessWasKnown,
        bool creatorMessageWasKnown
    );

    // EN: markAsAlteredByCheats declares or implements a focused behavior used by this module.
    // FR: markAsAlteredByCheats déclare ou implémente un comportement précis utilisé par ce module.
    void markAsAlteredByCheats();

    // EN: toggleGodMode declares or implements a focused behavior used by this module.
    // FR: toggleGodMode déclare ou implémente un comportement précis utilisé par ce module.
    bool toggleGodMode();
    // EN: toggleInfiniteConsumables declares or implements a focused behavior used by this module.
    // FR: toggleInfiniteConsumables déclare ou implémente un comportement précis utilisé par ce module.
    bool toggleInfiniteConsumables();
    // EN: toggleIndestructibleEquipment declares or implements a focused behavior used by this module.
    // FR: toggleIndestructibleEquipment déclare ou implémente un comportement précis utilisé par ce module.
    bool toggleIndestructibleEquipment();
    // EN: toggleEquipmentProtection declares or implements a focused behavior used by this module.
    // FR: toggleEquipmentProtection déclare ou implémente un comportement précis utilisé par ce module.
    bool toggleEquipmentProtection();
    // EN: toggleStorySkip declares or implements a focused behavior used by this module.
    // FR: toggleStorySkip déclare ou implémente un comportement précis utilisé par ce module.
    bool toggleStorySkip();
    // EN: unlockSpecialChallengeAccess declares or implements a focused behavior used by this module.
    // FR: unlockSpecialChallengeAccess déclare ou implémente un comportement précis utilisé par ce module.
    void unlockSpecialChallengeAccess();
    // EN: unlockBossRegistryExceptFinal declares or implements a focused behavior used by this module.
    // FR: unlockBossRegistryExceptFinal déclare ou implémente un comportement précis utilisé par ce module.
    void unlockBossRegistryExceptFinal(int maximumBossId, int finalBossId);

    // EN: enableGodMode declares or implements a focused behavior used by this module.
    // FR: enableGodMode déclare ou implémente un comportement précis utilisé par ce module.
    void enableGodMode();
    // EN: enableInfiniteConsumables declares or implements a focused behavior used by this module.
    // FR: enableInfiniteConsumables déclare ou implémente un comportement précis utilisé par ce module.
    void enableInfiniteConsumables();
    // EN: enableIndestructibleEquipment declares or implements a focused behavior used by this module.
    // FR: enableIndestructibleEquipment déclare ou implémente un comportement précis utilisé par ce module.
    void enableIndestructibleEquipment();
    // EN: enableEquipmentProtection declares or implements a focused behavior used by this module.
    // FR: enableEquipmentProtection déclare ou implémente un comportement précis utilisé par ce module.
    void enableEquipmentProtection();
    // EN: enableStorySkip declares or implements a focused behavior used by this module.
    // FR: enableStorySkip déclare ou implémente un comportement précis utilisé par ce module.
    void enableStorySkip();
    bool hasActiveCheatPower() const;
    int clearActiveCheatPowersForFireFlight();
    bool disableGodModeForFireFlight();

    // EN: markCreatorMessageSeen declares or implements a focused behavior used by this module.
    // FR: markCreatorMessageSeen déclare ou implémente un comportement précis utilisé par ce module.
    void markCreatorMessageSeen();
    // EN: recordGoldCheatUse declares or implements a focused behavior used by this module.
    // FR: recordGoldCheatUse déclare ou implémente un comportement précis utilisé par ce module.
    void recordGoldCheatUse();
    // EN: recordLevelCheatUse declares or implements a focused behavior used by this module.
    // FR: recordLevelCheatUse déclare ou implémente un comportement précis utilisé par ce module.
    void recordLevelCheatUse();
    // EN: recordMaxLevelCheatUse declares or implements a focused behavior used by this module.
    // FR: recordMaxLevelCheatUse déclare ou implémente un comportement précis utilisé par ce module.
    void recordMaxLevelCheatUse();
    // EN: recordRefundCheatUse declares or implements a focused behavior used by this module.
    // FR: recordRefundCheatUse déclare ou implémente un comportement précis utilisé par ce module.
    void recordRefundCheatUse();
    // EN: recordResetCheatUse declares or implements a focused behavior used by this module.
    // FR: recordResetCheatUse déclare ou implémente un comportement précis utilisé par ce module.
    void recordResetCheatUse();
    // EN: recordSwitchClassCheatUse declares or implements a focused behavior used by this module.
    // FR: recordSwitchClassCheatUse déclare ou implémente un comportement précis utilisé par ce module.
    void recordSwitchClassCheatUse();
    // EN: consumeRefundUse declares or implements a focused behavior used by this module.
    // FR: consumeRefundUse déclare ou implémente un comportement précis utilisé par ce module.
    bool consumeRefundUse();
    // EN: forceLevelToMaximum declares or implements a focused behavior used by this module.
    // FR: forceLevelToMaximum déclare ou implémente un comportement précis utilisé par ce module.
    void forceLevelToMaximum();
    // EN: gainOneLevelByCheat declares or implements a focused behavior used by this module.
    // FR: gainOneLevelByCheat déclare ou implémente un comportement précis utilisé par ce module.
    void gainOneLevelByCheat();

    // EN: takeDamage declares or implements a focused behavior used by this module.
    // FR: takeDamage déclare ou implémente un comportement précis utilisé par ce module.
    void takeDamage(int damage) override;

    // EN: getInventory declares or implements a focused behavior used by this module.
    // FR: getInventory déclare ou implémente un comportement précis utilisé par ce module.
    Inventory& getInventory();
    const Inventory& getInventory() const;

    // EN: getQuestLog declares or implements a focused behavior used by this module.
    // FR: getQuestLog déclare ou implémente un comportement précis utilisé par ce module.
    QuestLog& getQuestLog();
    const QuestLog& getQuestLog() const;

    // EN: getEquippedWeaponIndex declares or implements a focused behavior used by this module.
    // FR: getEquippedWeaponIndex déclare ou implémente un comportement précis utilisé par ce module.
    int getEquippedWeaponIndex() const;
    // EN: hasEquippedWeapon declares or implements a focused behavior used by this module.
    // FR: hasEquippedWeapon déclare ou implémente un comportement précis utilisé par ce module.
    bool hasEquippedWeapon() const;
    // EN: getEquippedWeapon declares or implements a focused behavior used by this module.
    // FR: getEquippedWeapon déclare ou implémente un comportement précis utilisé par ce module.
    Weapon getEquippedWeapon() const;

    void setNextAmmunitionChoice(const std::string& ammunitionId);
    std::string getNextAmmunitionChoice() const;
    void clearNextAmmunitionChoice();
    void setLastConsumedAmmunition(const std::string& ammunitionId);
    std::string getLastConsumedAmmunition() const;
    void clearLastConsumedAmmunition();
    // EN: equipWeapon declares or implements a focused behavior used by this module.
    // FR: equipWeapon déclare ou implémente un comportement précis utilisé par ce module.
    bool equipWeapon(int index);
    // EN: unequipWeapon declares or implements a focused behavior used by this module.
    // FR: unequipWeapon déclare ou implémente un comportement précis utilisé par ce module.
    void unequipWeapon();

    // EN: getEquippedArmorIndex declares or implements a focused behavior used by this module.
    // FR: getEquippedArmorIndex déclare ou implémente un comportement précis utilisé par ce module.
    int getEquippedArmorIndex() const;
    // EN: hasEquippedArmor declares or implements a focused behavior used by this module.
    // FR: hasEquippedArmor déclare ou implémente un comportement précis utilisé par ce module.
    bool hasEquippedArmor() const;
    // EN: getEquippedArmor declares or implements a focused behavior used by this module.
    // FR: getEquippedArmor déclare ou implémente un comportement précis utilisé par ce module.
    Armor getEquippedArmor() const;
    // EN: equipArmor declares or implements a focused behavior used by this module.
    // FR: equipArmor déclare ou implémente un comportement précis utilisé par ce module.
    bool equipArmor(int index);
    // EN: unequipArmor declares or implements a focused behavior used by this module.
    // FR: unequipArmor déclare ou implémente un comportement précis utilisé par ce module.
    void unequipArmor();

    // EN: initializeStarterInventory declares or implements a focused behavior used by this module.
    // FR: initializeStarterInventory déclare ou implémente un comportement précis utilisé par ce module.
    void initializeStarterInventory();
    void initializeStarterInventory(DifficultyMode difficulty);

    // EN: destroyEquippedWeapon declares or implements a focused behavior used by this module.
    // FR: destroyEquippedWeapon déclare ou implémente un comportement précis utilisé par ce module.
    bool destroyEquippedWeapon();
    // EN: destroyEquippedArmor declares or implements a focused behavior used by this module.
    // FR: destroyEquippedArmor déclare ou implémente un comportement précis utilisé par ce module.
    bool destroyEquippedArmor();

    // EN: gainExperience declares or implements a focused behavior used by this module.
    // FR: gainExperience déclare ou implémente un comportement précis utilisé par ce module.
    void gainExperience(int amount);
    // EN: loseExperience declares or implements a focused behavior used by this module.
    // FR: loseExperience déclare ou implémente un comportement précis utilisé par ce module.
    void loseExperience(int amount);
    // EN: levelUp declares or implements a focused behavior used by this module.
    // FR: levelUp déclare ou implémente un comportement précis utilisé par ce module.
    void levelUp();

    int attack(Random& random, bool& dodged, bool& critical, int damageBonus = 0) override;

    // EN: displayStats declares or implements a focused behavior used by this module.
    // FR: displayStats déclare ou implémente un comportement précis utilisé par ce module.
    void displayStats() const override;
    // EN: displayInventory declares or implements a focused behavior used by this module.
    // FR: displayInventory déclare ou implémente un comportement précis utilisé par ce module.
    void displayInventory() const;
    // EN: displaySimpleEquipment declares or implements a focused behavior used by this module.
    // FR: displaySimpleEquipment déclare ou implémente un comportement précis utilisé par ce module.
    void displaySimpleEquipment() const;
    // EN: displayDetailedEquipment declares or implements a focused behavior used by this module.
    // FR: displayDetailedEquipment déclare ou implémente un comportement précis utilisé par ce module.
    void displayDetailedEquipment() const;
};

#endif
