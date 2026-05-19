// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.
// Description: Centralizes special combat hooks for bosses and special characters.

#ifndef INCLUDE_COMBAT_ACTION_SPECIALCOMBATEFFECTS_HPP
#define INCLUDE_COMBAT_ACTION_SPECIALCOMBATEFFECTS_HPP

#include "core/Random.hpp"
#include "entity/Entity.hpp"

class SpecialCombatEffects
{
public:
    static bool atlasBlocksAttack(
        Entity& attacker,
        Entity& defender,
        int damage
    );

    static void applyDemonLifestealIfNeeded(
        Entity& attacker,
        int damageDealt
    );

    static bool specialCharacterDodgesBeforeDamage(
        Entity& defender,
        Random& random
    );

    static bool specialCharacterMissesBeforeDamage(
        Entity& attacker,
        Random& random
    );

    static void applySpecialCharacterAttackBonus(
        Entity& attacker,
        Random& random,
        int& rawDamage,
        bool& critical
    );

    static void applySpecialCharacterAfterDamage(
        Entity& attacker,
        Entity& defender,
        Random& random,
        int receivedDamage
    );

    static void applySpecialCharacterAfterReceivingDamage(
        Entity& defender,
        int receivedDamage,
        Random& random
    );

private:
    static bool isName(const Entity& entity, const std::string& expectedName);
    static bool isUnderHalfHp(const Entity& entity);
};

#endif
