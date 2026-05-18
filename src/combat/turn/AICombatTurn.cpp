// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "combat/turn/AICombatTurn.hpp"

#include "combat/CombatActions.hpp"
#include "combat/ai/CombatAI.hpp"
#include "combat/ai/AIAction.hpp"

#include "core/Console.hpp"

#include <iostream>

bool AICombatTurn::play(
    Entity& ai,
    Entity& defender,
    Random& random,
    int potionHealAmount,
    int potionDamageBonus
)
{
    std::cout << "Tour de " << ai.getName() << std::endl;
    std::cout << std::endl;

    Console::pauseSeconds(1);

    AIAction action = CombatAI::chooseAIAction(ai, random);

    if (action == AIAction::Attack)
    {
        CombatActions::executeAttack(ai, defender, random);
        return true;
    }

    if (action == AIAction::HealingPotion)
    {
        bool actionSucceeded = CombatActions::executeHealingPotion(ai, potionHealAmount);

        if (!actionSucceeded)
        {
            CombatActions::executeAttack(ai, defender, random);
        }

        return true;
    }

    if (action == AIAction::DamagePotion)
    {
        bool actionSucceeded = CombatActions::executeDamagePotion(
            ai,
            defender,
            random,
            potionDamageBonus
        );

        if (!actionSucceeded)
        {
            CombatActions::executeAttack(ai, defender, random);
        }

        return true;
    }

    std::cout << ai.getName() << " hésite, fixe le vide, et passe son tour." << std::endl;
    std::cout << std::endl;

    return true;
}