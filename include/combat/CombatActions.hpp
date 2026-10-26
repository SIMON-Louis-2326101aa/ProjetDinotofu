// EN: CombatActions.hpp briefly defines this Dinotofu module and its responsibilities.
// FR: CombatActions.hpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#ifndef INCLUDE_COMBAT_COMBATACTIONS_HPP
#define INCLUDE_COMBAT_COMBATACTIONS_HPP

#include "core/Random.hpp"
#include "entity/Entity.hpp"

#include <string>

class Player;

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

    // Uses the same material/catalyst/incantation rules as arcane class skills.
    // Returns false when the channeling cannot be formed or collapses.
    static bool prepareMagicTechnique(
        Player& player,
        const std::string& techniqueName,
        Random& random,
        int baseManaCost
    );

    // Player-facing variant: material casters can choose/confirm the physical support.
    // The non-interactive overload above remains deterministic for tests/technical paths.
    static bool prepareMagicTechniqueInteractive(
        Player& player,
        const std::string& techniqueName,
        Random& random,
        int baseManaCost
    );

    // Ranged multi-hit techniques must stop when no physical projectile remains.
    static bool hasCompatibleAmmunition(const Player& player);

    static void executeWeaponTechnique(
        Entity& attacker,
        Entity& defender,
        Random& random
    );

    static void executeHeavyAttack(
        Entity& attacker,
        Entity& defender,
        Random& random
    );

    static void executeQuickAttack(
        Entity& attacker,
        Entity& defender,
        Random& random
    );

    static bool executeClassSkill(
        Entity& attacker,
        Entity& defender,
        Random& random
    );

    static bool executeAIClassSkill(
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
