// English: This file belongs to Dinotofu. Code identifiers are written in English; player-facing text can stay in French.
// Français : Ce fichier appartient à Dinotofu. Les identifiants du code sont en anglais ; les textes affichés au joueur peuvent rester en français.
// Description: Displays active combat role actions such as provocation, threat reduction, and future ally support skills.

#include "interface/menu/CombatRoleMenu.hpp"

#include "combat/role/CombatRoleActionSystem.hpp"
#include "combat/role/CombatRoleSystem.hpp"
#include "core/Console.hpp"

#include <iostream>

bool CombatRoleMenu::open(Entity& entity)
{
    while (true)
    {
        std::cout << "========== COMPÉTENCES DE RÔLE ==========" << std::endl;
        std::cout << "0 : Retour" << std::endl;
        std::cout << "1 : Voir mon rôle" << std::endl;
        std::cout << "2 : Provocation" << std::endl;
        std::cout << "3 : Réduire ma menace" << std::endl;
        std::cout << "4 : Protection / soin d'allié" << std::endl;
        std::cout << "=========================================" << std::endl;
        std::cout << std::endl;
        std::cout << "> ";

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
            std::cout << "[cette option n'est pas encore accessible pour ce mode]" << std::endl;
            std::cout << "Les soins et protections d'alliés seront branchés avec les vrais slots de groupe." << std::endl;
            std::cout << std::endl;
            continue;
        }
    }
}
