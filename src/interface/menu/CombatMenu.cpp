// EN: CombatMenu.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: CombatMenu.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "interface/menu/CombatMenu.hpp"

#include "interface/TerminalInterface.hpp"

#include <iostream>
#include <sstream>

MenuScreen CombatMenu::buildTurnScreen(const Entity& entity)
{
    MenuScreen screen("COMBAT", "combat.turn");
    screen.addSubtitle("Tour de " + entity.getName());

    if (entity.getClassSkillCooldownTurns() > 0)
    {
        std::ostringstream cooldownText;
        cooldownText << "Compétence de classe : récupération "
                     << entity.getClassSkillCooldownTurns()
                     << " tour(s).";
        screen.addLine(cooldownText.str());
    }

    screen.addOption(1, "Attaquer", "Attaque simple, technique d'arme, attaque lourde/rapide ou compétence.", true, "combat.attack");
    screen.addOption(2, "Potion de soin rapide", "Liste seulement les potions de soin utilisables rapidement.", true, "combat.quick_heal");
    screen.addOption(3, "Potions", "Curatif, défensif, buff, offensive ou debuff.", true, "combat.potions");
    screen.addOption(4, "Équipement", "Voir ou changer rapidement l'arme et la tenue.", true, "combat.equipment");
    screen.addOption(5, "Inventaire / bestiaire", "Consulter les objets, matériaux et informations connues.", true, "combat.inventory");
    screen.addOption(6, "Posture de défense", "Renforce la survie jusqu'au prochain tour.", true, "combat.defend");
    screen.addOption(7, "Passer son tour", "Ne rien faire volontairement.", true, "combat.wait");
    screen.addOption(8, "Fuir", "Impossible contre un boss, variable contre les autres ennemis.", true, "combat.flee");
    screen.addOption(0, "Interface / aide rapide", "Rappelle les règles principales du combat.", true, "combat.help");

    return screen;
}

void CombatMenu::displayTurnMenu(const Entity& entity)
{
    TerminalInterface::renderMenuScreen(buildTurnScreen(entity));
}

// EN: displayUnavailableOption declares or implements a focused behavior used by this module.
// FR: displayUnavailableOption déclare ou implémente un comportement précis utilisé par ce module.
void CombatMenu::displayUnavailableOption()
{
    std::cout << "[cette option est inaccessible dans ce combat]" << std::endl;
    std::cout << std::endl;
}
