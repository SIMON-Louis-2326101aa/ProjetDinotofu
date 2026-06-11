// EN: InitiativeSystem.hpp exposes initiative helpers for group fights.
// FR: InitiativeSystem.hpp expose les aides d'initiative pour les combats de groupe.

#ifndef INCLUDE_COMBAT_GROUP_INITIATIVESYSTEM_HPP
#define INCLUDE_COMBAT_GROUP_INITIATIVESYSTEM_HPP

#include "combat/initiative/InitiativeQueue.hpp"

#include <string>
#include <vector>

class Boss;
class EnemyCombatQueue;
class Player;
class Random;
class Summon;

class InitiativeSystem
{
public:
    static InitiativeQueue buildWaveQueue(
        const std::vector<Player*>& party,
        const EnemyCombatQueue& wave,
        const std::vector<std::vector<Summon>>& partySummons,
        Random& random
    );

    static InitiativeQueue buildBossQueue(
        const std::vector<Player*>& party,
        const Boss& boss,
        const std::vector<std::vector<Summon>>& partySummons,
        Random& random
    );

    static std::vector<std::string> buildDisplayLines(const InitiativeQueue& queue);
};

#endif
