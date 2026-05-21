// EN: CombatAI.hpp briefly defines this Dinotofu module and its responsibilities.
// FR: CombatAI.hpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#ifndef INCLUDE_COMBAT_AI_COMBATAI_HPP
#define INCLUDE_COMBAT_AI_COMBATAI_HPP

#include "combat/ai/AIAction.hpp"

#include "core/Random.hpp"

#include "entity/Entity.hpp"
#include "entity/Boss.hpp"

#include <string>

class CombatAI
{
public:
    // EN: chooseAIAction declares or implements a focused behavior used by this module.
    // FR: chooseAIAction déclare ou implémente un comportement précis utilisé par ce module.
    static AIAction chooseAIAction(const Entity& ai, Random& random);
    // EN: chooseSpecialCharacterAction declares or implements a focused behavior used by this module.
    // FR: chooseSpecialCharacterAction déclare ou implémente un comportement précis utilisé par ce module.
    static AIAction chooseSpecialCharacterAction(const Entity& ai, Random& random);
    // EN: chooseBossAction declares or implements a focused behavior used by this module.
    // FR: chooseBossAction déclare ou implémente un comportement précis utilisé par ce module.
    static AIAction chooseBossAction(const Boss& boss, Random& random);

    // EN: getSummonTargetPriorityChance declares or implements a focused behavior used by this module.
    // FR: getSummonTargetPriorityChance déclare ou implémente un comportement précis utilisé par ce module.
    static int getSummonTargetPriorityChance(const Entity& attacker);

private:
    // EN: canUseHealingPotion declares or implements a focused behavior used by this module.
    // FR: canUseHealingPotion déclare ou implémente un comportement précis utilisé par ce module.
    static bool canUseHealingPotion(const Entity& entity);
    // EN: canUseDamagePotion declares or implements a focused behavior used by this module.
    // FR: canUseDamagePotion déclare ou implémente un comportement précis utilisé par ce module.
    static bool canUseDamagePotion(const Entity& entity);

    // EN: calculateHpPercentage declares or implements a focused behavior used by this module.
    // FR: calculateHpPercentage déclare ou implémente un comportement précis utilisé par ce module.
    static int calculateHpPercentage(const Entity& entity);

    static AIAction chooseDangerAIAction(
        bool healingAvailable,
        bool damageAvailable,
        int roll
    );

    static AIAction chooseMidHealthAIAction(
        bool healingAvailable,
        bool damageAvailable,
        int roll
    );

    static AIAction chooseStableAIAction(
        bool damageAvailable,
        int roll
    );

    // EN: normalizeName declares or implements a focused behavior used by this module.
    // FR: normalizeName déclare ou implémente un comportement précis utilisé par ce module.
    static std::string normalizeName(const std::string& name);

    static AIAction chooseDangerBossAction(
        bool healingAvailable,
        bool damageAvailable,
        bool ultimateAvailable,
        int roll
    );

    static AIAction chooseBossActionWithUltimateAvailable(
        bool damageAvailable,
        int roll
    );

    static AIAction chooseStableBossAction(
        bool damageAvailable,
        int roll
    );
};

#endif
