// EN: InitiativeRules.cpp defines Dexterity-based initiative rules.
// FR: InitiativeRules.cpp définit les règles d'initiative basées sur la Dextérité.

#include "combat/initiative/InitiativeRules.hpp"

#include "combat/summon/Summon.hpp"
#include "entity/Boss.hpp"
#include "entity/Monster.hpp"
#include "entity/Player.hpp"

#include <algorithm>

int InitiativeRules::playerBaseScore(const Player& player)
{
    return std::max(1, player.getAttributes().getDexterity() * 2 + player.getLevel());
}

int InitiativeRules::monsterBaseScore(const Monster& monster)
{
    int score = monster.getLevel() * 2 + std::max(1, monster.getMaxDamage() / 4);
    if (monster.isElite()) score += 3;
    if (monster.isEvolved()) score += 2;
    return std::max(1, score);
}

int InitiativeRules::bossBaseScore(const Boss& boss)
{
    return std::max(1, 12 + boss.getBossId() / 2 + boss.getMaxDamage() / 5);
}

int InitiativeRules::summonBaseScore(const Player& owner, const Summon& summon)
{
    return std::max(1, playerBaseScore(owner) - 3 + summon.getMaxDamage() / 6);
}
