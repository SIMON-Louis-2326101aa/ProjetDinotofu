// EN: SpecialCombatEffects.hpp briefly defines this Dinotofu module and its responsibilities.
// FR: SpecialCombatEffects.hpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.
// Description: Centralizes special combat hooks for bosses and special characters.

#ifndef INCLUDE_COMBAT_ACTION_SPECIALCOMBATEFFECTS_HPP
#define INCLUDE_COMBAT_ACTION_SPECIALCOMBATEFFECTS_HPP

#include "core/Random.hpp"
#include "entity/Entity.hpp"

#include <vector>

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

    // EN: registerSpecialGroupContext stores the current special group so individual combat hooks can react to allies.
    // FR: registerSpecialGroupContext mémorise le groupe spécial courant pour que les hooks individuels réagissent aux alliés.
    static void registerSpecialGroupContext(const std::vector<std::string>& names);

private:
    // EN: isName declares or implements a focused behavior used by this module.
    // FR: isName déclare ou implémente un comportement précis utilisé par ce module.
    static bool isName(const Entity& entity, const std::string& expectedName);
    // EN: isUnderHalfHp declares or implements a focused behavior used by this module.
    // FR: isUnderHalfHp déclare ou implémente un comportement précis utilisé par ce module.
    static bool isUnderHalfHp(const Entity& entity);
};

#endif
