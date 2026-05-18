// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#ifndef INCLUDE_COMBAT_AI_COMBATAI_HPP
#define INCLUDE_COMBAT_AI_COMBATAI_HPP

#include "combat/ai/AIAction.hpp"

#include "core/Random.hpp"

#include "entity/Entity.hpp"
#include "entity/Boss.hpp"

class CombatAI
{
public:
    static AIAction chooseAIAction(const Entity& ai, Random& random);
    static AIAction chooseBossAction(const Boss& boss, Random& random);

private:
    static bool canUseHealingPotion(const Entity& entity);
    static bool canUseDamagePotion(const Entity& entity);

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
