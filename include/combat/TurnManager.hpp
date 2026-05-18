// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#ifndef INCLUDE_COMBAT_TURNMANAGER_HPP
#define INCLUDE_COMBAT_TURNMANAGER_HPP

#include "core/Random.hpp"

#include "entity/Entity.hpp"
#include "entity/Player.hpp"
#include "entity/Boss.hpp"

class TurnManager
{
public:
    static bool playHumanTurn(
        Entity& attacker,
        Entity& defender,
        Random& random,
        int potionHealAmount,
        int potionDamageBonus
    );

    static bool playAITurn(
        Entity& ai,
        Entity& defender,
        Random& random,
        int potionHealAmount,
        int potionDamageBonus
    );

    static bool playBossTurn(
        Boss& boss,
        Entity& player,
        Random& random
    );

    static void checkBossDecryption(Boss& boss);

    static void executeAttack(
        Entity& attacker,
        Entity& defender,
        Random& random
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
