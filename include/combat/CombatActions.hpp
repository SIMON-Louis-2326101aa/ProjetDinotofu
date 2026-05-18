// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#ifndef INCLUDE_COMBAT_COMBATACTIONS_HPP
#define INCLUDE_COMBAT_COMBATACTIONS_HPP

#include "core/Random.hpp"
#include "entity/Entity.hpp"

class CombatActions
{
public:
    static void executeAttack(
        Entity& attacker,
        Entity& defender,
        Random& random
    );

    static void executeBoostedAttack(
        Entity& attacker,
        Entity& defender,
        Random& random,
        int damageBonus
    );

    static bool executeHealingPotion(
        Entity& entity,
        int potionHealAmount
    );

    static bool executeDamagePotion(
        Entity& attacker,
        Entity& defender,
        Random& random,
        int potionDamageBonus
    );
};

#endif
