// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "combat/CombatActions.hpp"

#include "combat/action/CombatAttack.hpp"
#include "combat/action/CombatPotion.hpp"

void CombatActions::executeAttack(
    Entity& attacker,
    Entity& defender,
    Random& random
)
{
    CombatAttack::executeAttack(
        attacker,
        defender,
        random
    );
}

void CombatActions::executeBoostedAttack(
    Entity& attacker,
    Entity& defender,
    Random& random,
    int damageBonus
)
{
    CombatAttack::executeBoostedAttack(
        attacker,
        defender,
        random,
        damageBonus
    );
}

bool CombatActions::executeHealingPotion(
    Entity& entity,
    int potionHealAmount
)
{
    return CombatPotion::executeHealingPotion(
        entity,
        potionHealAmount
    );
}

bool CombatActions::executeDamagePotion(
    Entity& attacker,
    Entity& defender,
    Random& random,
    int potionDamageBonus
)
{
    return CombatPotion::executeDamagePotion(
        attacker,
        defender,
        random,
        potionDamageBonus
    );
}