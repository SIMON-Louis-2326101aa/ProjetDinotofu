#pragma once

#include <string>
#include <vector>

class Player;

enum class AllyTeamOrigin
{
    PersonalRecruit,
    EmployerTeam
};

struct RecruitedAllyCombatSupport
{
    std::string name;
    std::string race;
    std::string job;
    std::string trait;
    std::string motivation;
    int recruitedLevel = 1;
    int estimatedLevel = 1;
    int originRankIndex = 0;
    int currentRankIndex = 0;
    int requestedShare = 10;
    int normalizedShare = 0;
    int damageDealt = 0;
    int healingDone = 0;
    int turnsTaken = 0;
    int maxHp = 1;
    int currentHp = 1;
    int healingPotionCharges = 0;
    int weaponQuality = 1;
    int armorQuality = 1;
    int supportKitQuality = 0;
    // V3.51.62: lightweight persistent condition derived from journal-backed combat strain.
    int fatiguePercent = 0;
    int moralePercent = 50;
    int readinessPercent = 100;
    int activeSkillCooldown = 0;
    int supportActions = 0;
    int finishBlows = 0;
    bool protectedThisRound = false;
    // Transient combat-only marker: entering from reserve never grants an immediate free action.
    bool enteredFromReserveThisPhase = false;
    // Transient post-KO knowledge. These fields are never persisted in character saves: they only
    // describe what this participant could actually witness while the player was unconscious.
    int postKoFirstObservedRound = 0;
    int postKoLastObservedRound = 0;
    int postKoObservedDefeats = 0;
    int postKoObservedRoundsFromReserve = 0;
    bool postKoObservedFromReserve = false;
    bool postKoObservedFinalOutcome = false;
    bool postKoAvailableForDebrief = true;
    AllyTeamOrigin teamOrigin = AllyTeamOrigin::PersonalRecruit;
    std::string persistentMemberId;
    bool teamLeader = false;
    // Temporary municipal/event support participates physically but never takes loot or persists as a recruit.
    bool temporaryNoReward = false;
};

namespace RecruitedAllyCombatRoster
{
    inline constexpr int MaxSoloTeamMembers = 6;
    inline constexpr int MaxRecruitedTeamMembers = MaxSoloTeamMembers - 1;
    inline constexpr int MaxActiveRecruitsWithPlayer = 2;
    inline constexpr int MaxActiveRecruitsWithoutPlayer = 3;

    std::vector<RecruitedAllyCombatSupport> collectActive(const Player& player);
    std::vector<RecruitedAllyCombatSupport> collectReserve(const Player& player);
    int normalizeShares(std::vector<RecruitedAllyCombatSupport>& allies);
    bool jobContains(const RecruitedAllyCombatSupport& ally, const std::string& token);
    int maturityScore(const RecruitedAllyCombatSupport& ally);
    int maturityTier(const RecruitedAllyCombatSupport& ally);
    std::string maturityName(const RecruitedAllyCombatSupport& ally);
    int relationshipScore(const Player& player, const std::string& recruitName);

    // A reserve entry caused before the ally phase must never create an immediate extra action.
    // The marker is transient and consumed exactly once when that ally would otherwise act.
    void markReserveEntryDelay(RecruitedAllyCombatSupport& ally);
    bool consumeReserveEntryDelay(RecruitedAllyCombatSupport& ally);

    // Post-KO observation is separate from active participation: a nearby reserve member may
    // witness the fight from the sideline/stands without ever taking a combat turn.
    void recordPostKoObservation(RecruitedAllyCombatSupport& ally, int round, bool fromReserve);
    void recordPostKoObservedDefeats(RecruitedAllyCombatSupport& ally, int defeatedCount);
    void recordPostKoFinalOutcome(RecruitedAllyCombatSupport& ally);
    bool hasPostKoObservation(const RecruitedAllyCombatSupport& ally);
}
