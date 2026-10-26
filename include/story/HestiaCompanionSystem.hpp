#pragma once

#include <string>
#include <vector>

class Player;
class Random;
struct RecruitedAllyCombatSupport;

struct HestiaCompanionProfile
{
    bool available = false;
    bool activeInParty = false;
    std::string className = "Indéfinie";
    std::string weaponStyle = "Aucun";
    std::string armorStyle = "Aucune";
    int practiceSessions = 0;
    int combatParticipations = 0;
    int generalAptitude = -4;
    int magicAptitude = -4;
    int divineLightAptitude = -4;
    int mishapChancePercent = 45;
    int awakeningStage = 0;
};

struct HestiaCombatMishap
{
    bool occurred = false;
    bool mayHitLeader = false;
    bool mayHitAlly = false;
    int maxFriendlyDamagePercent = 0;
    std::string line;
};

struct HestiaAwakeningCombatResult
{
    bool triggered = false;
    int stage = 0;
    int bonusDamage = 0;
    bool returnsToHeavens = false;
    std::vector<std::string> lines;
};

namespace HestiaCompanionSystem
{
    HestiaCompanionProfile profile(const Player& player);
    bool isActive(const Player& player);

    std::vector<std::string> classChoices();
    std::vector<std::string> weaponStyleChoices();
    std::vector<std::string> armorStyleChoices();

    bool chooseClass(Player& player, const std::string& className);
    bool chooseWeaponStyle(Player& player, const std::string& weaponStyle);
    bool chooseArmorStyle(Player& player, const std::string& armorStyle);
    bool practice(Player& player);
    void recordCombatParticipation(Player& player);

    bool looksMagicOriented(const HestiaCompanionProfile& profile);
    RecruitedAllyCombatSupport toCombatSupport(const Player& player);

    HestiaCombatMishap rollMishap(const Player& player, Random& random, bool magicalAction);
    HestiaAwakeningCombatResult tryAwakening(Player& player, Random& random, bool magicalAction);
    std::vector<std::string> statusLines(const Player& player);
}
