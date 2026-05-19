// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#ifndef INCLUDE_COMBAT_TURN_HUMANCOMBATTURN_HPP
#define INCLUDE_COMBAT_TURN_HUMANCOMBATTURN_HPP

#include "core/Random.hpp"

#include "entity/Entity.hpp"
#include "combat/summon/Summon.hpp"

#include <vector>

class HumanCombatTurn
{
public:
    static bool play(
        Entity& attacker,
        Entity& defender,
        Random& random,
        int potionHealAmount,
        int potionDamageBonus
    );

    static bool playWithEnemySummons(
        Entity& attacker,
        Entity& defender,
        std::vector<Summon>& enemySummons,
        Random& random,
        int potionHealAmount,
        int potionDamageBonus
    );

private:
    static bool openObservationInterface(
        Entity& interfacePlayer,
        Entity& target
    );

    static bool chooseAndExecuteAttack(
        Entity& attacker,
        Entity& defender,
        std::vector<Summon>& enemySummons,
        Random& random
    );

    static bool inspectCombatTarget(
        Entity& interfacePlayer,
        Entity& target,
        const std::vector<Summon>& enemySummons
    );

    static bool handleEscape(
        Entity& attacker,
        Entity& defender,
        Random& random
    );
};

#endif
