#ifndef INCLUDE_COMBAT_ALLY_DUOMASTERYSYSTEM_HPP
#define INCLUDE_COMBAT_ALLY_DUOMASTERYSYSTEM_HPP

#include <string>

class Player;

enum class DuoCombatRole
{
    Tank,
    Support,
    Ranged,
    Assault,
    Other
};

struct DuoTechniquePlan
{
    std::string name = "Assaut synchronisé";
    int powerPercent = 100;
    int vulnerabilityPercent = 0;
    int weakening = 0;
    int playerHealPercent = 0;
    int playerGuardPercent = 0;
};

struct DuoBondProfile
{
    int successfulCombos = 0;
    int partialCombos = 0;
    int failedCombos = 0;
    int sharedCombatDays = 0;
    int coordinatedOrderDays = 0;
    int relationshipScore = 0;
    int bondScore = 0;
    int bondTier = 0;
};

class DuoMasterySystem
{
public:
    static std::string pairKey(const std::string& firstName, const std::string& secondName);
    static int experience(const Player& player, const std::string& pairKey);
    static int masteryTier(int experience);
    static std::string masteryLabel(int tier);
    static DuoBondProfile bondProfile(const Player& player, const std::string& firstName, const std::string& secondName);
    static std::string bondLabel(int tier);
    static bool recordSharedCombatDay(Player& player, const std::string& firstName, const std::string& secondName, const std::string& contextId);
    static bool recordCoordinatedOrderDay(Player& player, const std::string& firstName, const std::string& secondName, const std::string& contextId);
    static DuoCombatRole roleForJob(const std::string& job);
    static DuoTechniquePlan techniqueForJobs(const std::string& firstJob, const std::string& secondJob);
    static int coordinationChance(int maturityTotal, int rankTotal, int experience, int relationshipScore, bool bondsOath);

    // Combat-local pacing rule: after any real duo attempt, two enemy responses are required
    // to return to ready (the immediate response, then one full intervening round).
    static int combinedTechniqueRecoveryAfterAttempt();
    static int advanceCombinedTechniqueRecovery(int remainingEnemyResponses);
    static bool combinedTechniqueReady(int remainingEnemyResponses);
};

#endif
