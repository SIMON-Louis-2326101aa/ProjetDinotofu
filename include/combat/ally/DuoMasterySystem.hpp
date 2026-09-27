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

class DuoMasterySystem
{
public:
    static std::string pairKey(const std::string& firstName, const std::string& secondName);
    static int experience(const Player& player, const std::string& pairKey);
    static int masteryTier(int experience);
    static std::string masteryLabel(int tier);
    static DuoCombatRole roleForJob(const std::string& job);
    static DuoTechniquePlan techniqueForJobs(const std::string& firstJob, const std::string& secondJob);
    static int coordinationChance(int maturityTotal, int rankTotal, int experience, bool bondsOath);
};

#endif
