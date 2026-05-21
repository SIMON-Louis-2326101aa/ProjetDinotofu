// EN: CombatTargetMenu.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: CombatTargetMenu.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "interface/menu/CombatTargetMenu.hpp"

#include "combat/TurnManager.hpp"
#include "combat/CombatActions.hpp"
#include "combat/action/CombatAttack.hpp"
#include "combat/system/ObservationSystem.hpp"
#include "combat/threat/ThreatSystem.hpp"

#include "core/Console.hpp"

#include <iostream>

namespace
{
    // EN: findForcedTargetIndex declares or implements a focused behavior used by this module.
    // FR: findForcedTargetIndex déclare ou implémente un comportement précis utilisé par ce module.
    int findForcedTargetIndex(const EnemyCombatQueue& wave)
    {
        for (int i = 0; i < wave.getActiveEnemyCount(); ++i)
        {
            const Monster& enemy = wave.getActiveEnemy(i);

            if (enemy.isProvoking() || enemy.hasHealingThreat())
            {
                return i;
            }
        }

        return -1;
    }
}

bool CombatTargetMenu::openForAttack(
    Player& player,
    EnemyCombatQueue& wave,
    Random& random
)
{
    return openTargetMenu(
        player,
        wave,
        random,
        false,
        0
    );
}

bool CombatTargetMenu::openForDamagePotion(
    Player& player,
    EnemyCombatQueue& wave,
    Random& random,
    int potionDamageBonus
)
{
    return openForBoostedAttack(
        player,
        wave,
        random,
        potionDamageBonus
    );
}

bool CombatTargetMenu::openForBoostedAttack(
    Player& player,
    EnemyCombatQueue& wave,
    Random& random,
    int damageBonus
)
{
    return openTargetMenu(
        player,
        wave,
        random,
        true,
        damageBonus
    );
}

// EN: chooseTarget declares or implements a focused behavior used by this module.
// FR: chooseTarget déclare ou implémente un comportement précis utilisé par ce module.
int CombatTargetMenu::chooseTarget(const EnemyCombatQueue& wave)
{
    wave.displayActiveEnemies();

    std::cout << "Choisis une cible." << std::endl;
    std::cout << "Entre le numéro de l'ennemi à sélectionner, ou 0 pour revenir." << std::endl;
    std::cout << "> ";

    int choice = Console::askNumberBetween(
        0,
        wave.getActiveEnemyCount(),
        "Cible invalide. Choisis un ennemi actif, ou 0 pour revenir."
    );

    if (choice == 0)
    {
        return -1;
    }

    return choice - 1;
}

bool CombatTargetMenu::openTargetMenu(
    Player& player,
    EnemyCombatQueue& wave,
    Random& random,
    bool boostedAttack,
    int damageBonus
)
{
    while (wave.hasEnemiesLeft())
    {
        int targetIndex = chooseTarget(wave);

        Console::clear();

        if (targetIndex == -1)
        {
            return false;
        }

        if (!wave.isActiveIndexValid(targetIndex))
        {
            std::cout << "Cette cible n'est plus disponible." << std::endl;
            std::cout << std::endl;
            return false;
        }

        int forcedTargetIndex = findForcedTargetIndex(wave);

        if (forcedTargetIndex >= 0 && forcedTargetIndex != targetIndex)
        {
            const Monster& forcedTarget = wave.getActiveEnemy(forcedTargetIndex);

            if (forcedTarget.isProvoking())
            {
                std::cout << forcedTarget.getName()
                          << " bloque la ligne. Sa provocation t'empêche d'ignorer sa présence."
                          << std::endl;
            }
            else
            {
                std::cout << forcedTarget.getName()
                          << " vient de soigner un allié. Ton attention se fixe sur le soigneur."
                          << std::endl;
            }

            std::cout << std::endl;
            return false;
        }

        Monster& target = wave.getActiveEnemy(targetIndex);

        bool stayOnThisTarget = true;

        while (stayOnThisTarget && !target.isDead())
        {
            std::cout << "========== CIBLE SÉLECTIONNÉE ==========" << std::endl;
            std::cout << "Cible : " << target.getName() << std::endl;
            std::cout << "Race : " << target.getRaceText() << std::endl;

            if (target.isInvocation())
            {
                std::cout << "Statut : Invocation" << std::endl;
            }
            else if (target.isElite())
            {
                std::cout << "Statut : Élite" << std::endl;
            }
            else
            {
                std::cout << "Statut : Ennemi standard" << std::endl;
            }

            std::cout << "========================================" << std::endl;
            std::cout << "0 : Retour au menu principal" << std::endl;

            if (boostedAttack)
            {
                std::cout << "1 : Utiliser la potion de rage sur cette cible" << std::endl;
            }
            else
            {
                std::cout << "1 : Attaquer cette cible" << std::endl;
            }

            std::cout << "2 : Inspecter cette cible" << std::endl;
            std::cout << "3 : Choisir une autre cible" << std::endl;
            std::cout << std::endl;
            std::cout << "> ";

            int choice = Console::askNumberBetween(
                0,
                3,
                "Choix invalide. Entre un chiffre entre 0 et 3."
            );

            Console::clear();

            if (choice == 0)
            {
                return false;
            }

            if (choice == 1)
            {
                if (boostedAttack)
                {
                    CombatAttack::executeBoostedAttack(
                        player,
                        target,
                        random,
                        damageBonus
                    );

                    ThreatSystem::consumeForcedTargetIfNeeded(target);
                    wave.removeDeadAndReplace();

                    return true;
                }

                TurnManager::executeAttack(player, target, random);
                ThreatSystem::consumeForcedTargetIfNeeded(target);

                wave.removeDeadAndReplace();

                return true;
            }

            if (choice == 2)
            {
                ObservationSystem::displayTerminalStats(target);
            }

            if (choice == 3)
            {
                stayOnThisTarget = false;
            }
        }
    }

    return false;
}
