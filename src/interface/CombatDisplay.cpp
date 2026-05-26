// EN: CombatDisplay.cpp centralizes combat presentation for terminal and future GUI.
// FR: CombatDisplay.cpp centralise la présentation du combat pour le terminal et la future IG.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "interface/CombatDisplay.hpp"

#include "character/SpecialCharacterDialogueCatalog.hpp"
#include "interface/menu/common/MessageScreen.hpp"
#include "interface/model/CombatStateSnapshot.hpp"

#include <string>
#include <vector>

void CombatDisplay::displayCombatResult(
    const Entity& combattant1,
    const Entity& combattant2
)
{
    std::vector<std::string> lines;

    if (combattant1.isDead() && combattant2.isDead())
    {
        lines.push_back("Les deux combattants s'effondrent en même temps.");
        lines.push_back("L'arène reste silencieuse... aucun vainqueur clair.");
    }
    else if (combattant1.isDead())
    {
        lines.push_back(combattant1.getName() + " tombe au sol.");
        lines.push_back(combattant2.getName() + " remporte le duel.");
    }
    else if (combattant2.isDead())
    {
        lines.push_back(combattant2.getName() + " tombe au sol.");
        lines.push_back(combattant1.getName() + " remporte le duel.");
    }
    else
    {
        lines.push_back("Le combat s'arrête sans vainqueur.");
    }

    MessageScreen::show("RÉSULTAT DU COMBAT", "combat.result", lines, false);

    if (combattant1.isDead() && !combattant2.isDead())
    {
        SpecialCharacterDialogueCatalog::displayVictoryDialogue(combattant2.getName());
        SpecialCharacterDialogueCatalog::displayDefeatDialogue(combattant1.getName());
    }
    else if (combattant2.isDead() && !combattant1.isDead())
    {
        SpecialCharacterDialogueCatalog::displayVictoryDialogue(combattant1.getName());
        SpecialCharacterDialogueCatalog::displayDefeatDialogue(combattant2.getName());
    }
}

GuiCombatStateSnapshot CombatDisplay::buildDuelSnapshot(
    const Entity& playerSideEntity,
    const Entity& enemySideEntity,
    const std::string& title,
    const std::string& phase,
    int turnNumber
)
{
    return CombatStateSnapshot::fromDuel(playerSideEntity, enemySideEntity, title, phase, turnNumber);
}

GuiCombatStateSnapshot CombatDisplay::buildGroupSnapshot(
    const CombatGroup& playerGroup,
    const CombatGroup& enemyGroup,
    const std::string& title,
    const std::string& phase,
    int turnNumber
)
{
    return CombatStateSnapshot::fromGroups(playerGroup, enemyGroup, title, phase, turnNumber);
}

void CombatDisplay::displayCombatState(const GuiCombatStateSnapshot& snapshot, bool waitAndClear)
{
    MessageScreen::show(snapshot.title, snapshot.screenId, CombatStateSnapshot::toDisplayLines(snapshot), waitAndClear);
}
