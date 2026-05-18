// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "interface/menu/CombatTargetMenu.hpp"

#include "combat/TurnManager.hpp"
#include "combat/CombatActions.hpp"
#include "combat/action/CombatAttack.hpp"
#include "combat/system/ObservationSystem.hpp"

#include "core/Console.hpp"

#include <iostream>

bool CombatTargetMenu::openForAttack(
    Player& player,
    EnemyCombatQueue& wave,
    Random& random
)
{
    return ouvrirMenuCible(
        player,
        wave,
        random,
        false,
        0
    );
}

bool CombatTargetMenu::ouvrirPourPotionDegats(
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
    return ouvrirMenuCible(
        player,
        wave,
        random,
        true,
        damageBonus
    );
}

int CombatTargetMenu::chooseTarget(const EnemyCombatQueue& wave)
{
    wave.displayActiveEnemies();

    std::cout << "Choisis une cible." << std::endl;
    std::cout << "Entre le numéro de l'ennemi à sélectionner." << std::endl;
    std::cout << "> ";

    return Console::askNumberBetween(
        0,
        wave.getActiveEnemyCount() - 1,
        "Cible invalide. Choisis un ennemi actif."
    );
}

bool CombatTargetMenu::ouvrirMenuCible(
    Player& player,
    EnemyCombatQueue& wave,
    Random& random,
    bool boostedAttack,
    int damageBonus
)
{
    while (wave.hasEnemiesLeft())
    {
        int indexCible = chooseTarget(wave);

        Console::clear();

        if (!wave.isActiveIndexValid(indexCible))
        {
            std::cout << "Cette cible n'est plus disponible." << std::endl;
            std::cout << std::endl;
            return false;
        }

        Monster& target = wave.getActiveEnemy(indexCible);

        bool resterSurCetteCible = true;

        while (resterSurCetteCible && !target.isDead())
        {
            std::cout << "========== CIBLE SÉLECTIONNÉE ==========" << std::endl;
            std::cout << "Cible : " << target.getName() << std::endl;
            std::cout << "Race : " << target.getRaceTexte() << std::endl;

            if (target.estInvocation())
            {
                std::cout << "Statut : Invocation" << std::endl;
            }
            else if (target.estElite())
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

                    wave.removeDeadAndReplace();

                    return true;
                }

                TurnManager::executeAttack(player, target, random);

                wave.removeDeadAndReplace();

                return true;
            }

            if (choice == 2)
            {
                ObservationSystem::displayTerminalStats(target);
            }

            if (choice == 3)
            {
                resterSurCetteCible = false;
            }
        }
    }

    return false;
}