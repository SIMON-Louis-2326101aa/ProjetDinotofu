// EN: TurnManager.hpp briefly defines this Dinotofu module and its responsibilities.
// FR: TurnManager.hpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#ifndef INCLUDE_COMBAT_TURNMANAGER_HPP
#define INCLUDE_COMBAT_TURNMANAGER_HPP

#include "core/Random.hpp"

#include "entity/Entity.hpp"
#include "entity/Player.hpp"
#include "entity/Boss.hpp"
#include "combat/summon/Summon.hpp"

#include <vector>

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

    static bool playHumanTurnWithEnemySummons(
        Entity& attacker,
        Entity& defender,
        std::vector<Summon>& enemySummons,
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

    // EN: checkBossDecryption declares or implements a focused behavior used by this module.
    // FR: checkBossDecryption déclare ou implémente un comportement précis utilisé par ce module.
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
