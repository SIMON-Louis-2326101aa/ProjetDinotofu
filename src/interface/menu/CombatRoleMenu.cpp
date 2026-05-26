// EN: CombatRoleMenu.cpp centralizes combat role screens for terminal and future GUI rendering.
// FR: CombatRoleMenu.cpp centralise les écrans de rôle de combat pour le terminal et la future IG.
// English: This file belongs to Dinotofu. Code identifiers are written in English; player-facing text can stay in French.
// Français : Ce fichier appartient à Dinotofu. Les identifiants du code sont en anglais ; les textes affichés au joueur peuvent rester en français.
// Description: Displays active combat role actions such as provocation, threat reduction, and future ally support skills.

#include "interface/menu/CombatRoleMenu.hpp"

#include "combat/role/CombatRoleActionSystem.hpp"
#include "combat/role/CombatRoleSystem.hpp"
#include "core/Console.hpp"
#include "interface/TerminalInterface.hpp"

#include <iostream>

MenuScreen CombatRoleMenu::buildScreen(const Entity& entity)
{
    MenuScreen screen("COMPÉTENCES DE RÔLE", "combat.role");
    screen.addSubtitle("Rôle actif de " + entity.getName());
    screen.addBackOption("Retour", "combat.role.back");
    screen.addOption(1, "Voir mon rôle", "Résumé de la menace, du style de combat et du rôle actuel.", true, "combat.role.identity");
    screen.addOption(2, "Provocation", "Attirer l'attention et activer une posture défensive.", true, "combat.role.provoke");
    screen.addOption(3, "Réduire ma menace", "Plus efficace pour les profils d'assassin.", true, "combat.role.reduce_threat");
    screen.addOption(4, "Protection / soin d'allié", "Nécessite un allié stable à portée.", true, "combat.role.ally_support");
    return screen;
}

bool CombatRoleMenu::open(Entity& entity)
{
    while (true)
    {
        TerminalInterface::renderMenuScreen(buildScreen(entity));

        int choice = Console::askNumberBetween(
            0,
            4,
            "Choix invalide. Entre un chiffre entre 0 et 4."
        );

        Console::clear();

        if (choice == 0)
        {
            return false;
        }

        if (choice == 1)
        {
            CombatRoleActionSystem::displayRoleIdentity(entity);
            continue;
        }

        if (choice == 2)
        {
            if (CombatRoleActionSystem::activateManualProvocation(entity, 2))
            {
                return true;
            }

            continue;
        }

        if (choice == 3)
        {
            if (!CombatRoleSystem::isAssassin(entity))
            {
                std::cout << entity.getName()
                          << " tente de se faire oublier, mais ce style demande une vraie approche d'assassin."
                          << std::endl;
                std::cout << std::endl;
                continue;
            }

            entity.clearHealingThreat();
            entity.clearProvocation();

            std::cout << entity.getName()
                      << " efface sa présence et réduit sa menace immédiate."
                      << std::endl;
            std::cout << "Les ennemis auront plus de mal à le garder comme priorité." << std::endl;
            std::cout << std::endl;

            return true;
        }

        if (choice == 4)
        {
            std::cout << "[cette option est inaccessible dans ce combat]" << std::endl;
            std::cout << "Aucun allié stable ne se trouve dans une position où tu peux le protéger ainsi." << std::endl;
            std::cout << std::endl;
            continue;
        }
    }
}
