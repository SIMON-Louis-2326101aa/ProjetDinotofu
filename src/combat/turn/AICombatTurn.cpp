// EN: AICombatTurn.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: AICombatTurn.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "combat/turn/AICombatTurn.hpp"

#include "combat/CombatActions.hpp"
#include "combat/ai/CombatAI.hpp"
#include "combat/ai/AIAction.hpp"
#include "character/SpecialCharacterDialogueCatalog.hpp"

#include "core/Console.hpp"
#include "interface/menu/common/MessageScreen.hpp"

#include <string>
#include <vector>

bool AICombatTurn::play(
    Entity& ai,
    Entity& defender,
    Random& random,
    int potionHealAmount,
    int potionDamageBonus
)
{
    MessageScreen::show(
        "TOUR ADVERSAIRE",
        "combat.ai.turn.start",
        {"Tour de " + ai.getName() + "."},
        false
    );

    Console::pauseSeconds(1);

    AIAction action = CombatAI::chooseAIAction(ai, random);

    if (SpecialCharacterDialogueCatalog::hasDialogueFor(ai.getName())
        && ai.getHp() * 100 / ai.getMaxHp() <= 45)
    {
        SpecialCharacterDialogueCatalog::displayLowHealthDialogue(ai.getName());
    }

    if (action == AIAction::Attack)
    {
        SpecialCharacterDialogueCatalog::displayCombatActionDialogue(ai.getName(), "attack");
        CombatActions::executeAttack(ai, defender, random);
        return true;
    }

    if (action == AIAction::ClassSkill)
    {
        SpecialCharacterDialogueCatalog::displayCombatActionDialogue(ai.getName(), "attack");
        bool actionSucceeded = CombatActions::executeAIClassSkill(ai, defender, random);

        if (!actionSucceeded)
        {
            CombatActions::executeAttack(ai, defender, random);
        }

        return true;
    }

    if (action == AIAction::HealingPotion)
    {
        SpecialCharacterDialogueCatalog::displayCombatActionDialogue(ai.getName(), "healing");
        bool actionSucceeded = CombatActions::executeHealingPotion(ai, potionHealAmount);

        if (!actionSucceeded)
        {
            CombatActions::executeAttack(ai, defender, random);
        }

        return true;
    }

    if (action == AIAction::DamagePotion)
    {
        SpecialCharacterDialogueCatalog::displayCombatActionDialogue(ai.getName(), "damage");
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

    MessageScreen::show(
        "TOUR ADVERSAIRE",
        "combat.ai.turn.skip",
        {ai.getName() + " hésite, fixe le vide, et passe son tour."},
        false
    );

    return true;
}
