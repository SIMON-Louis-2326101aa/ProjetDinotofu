#pragma once

#include <string>
#include <vector>

class Monster;
class Player;
class Random;

struct MonsterWeakPointProfile
{
    std::string zoneId = "generic";
    std::string zoneLabel = "zone fragile";

    // A weak point can be useful, dangerous, or both. These values are deliberately
    // small because the tactical action already has its own generic success effects.
    bool revealTargetStats = false;
    bool breakDefensePosture = false;
    int weakeningTurns = 0;
    int weakeningPercent = 0;
    int vulnerabilityTurns = 0;
    int vulnerabilityPercent = 0;
    int nextHitVulnerabilityTurns = 0;
    int nextHitVulnerabilityPercent = 0;

    bool reactiveHazard = false;
    bool selfDestructiveHazard = false;
    int hazardDamage = 0;
    int poisonTurns = 0;
    int poisonDamage = 0;
    int burningTurns = 0;
    int burningDamage = 0;
    int illusionTurns = 0;

    std::string hint;
    std::string successLine;
};

struct MonsterWeakPointHitResult
{
    bool triggered = false;
    bool targetDestroyedByReaction = false;
    int playerDamage = 0;
    std::vector<std::string> lines;
};

namespace MonsterAnatomySystem
{
    MonsterWeakPointProfile weakPointProfile(const Monster& monster);
    MonsterWeakPointHitResult resolveWeakPointReaction(Monster& monster, Player& player, Random& random);
}
