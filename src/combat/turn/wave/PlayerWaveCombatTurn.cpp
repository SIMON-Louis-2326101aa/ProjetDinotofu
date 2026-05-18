// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "combat/turn/wave/PlayerWaveCombatTurn.hpp"

#include "combat/system/EscapeSystem.hpp"

#include "core/Console.hpp"

#include "interface/menu/CombatMenu.hpp"
#include "interface/menu/CombatTargetMenu.hpp"
#include "interface/menu/EquipmentMenu.hpp"
#include "interface/menu/InventoryMenu.hpp"
#include "interface/menu/CombatPotionMenu.hpp"

#include <iostream>

bool PlayerWaveCombatTurn::play(
    Player& player,
    EnemyCombatQueue& wave,
    Random& random,
    bool& escapeSucceeded
)
{
    CombatMenu::displayTurnMenu(player);

    int choice = Console::askNumberBetween(
        0,
        7,
        "Choix invalide. Entre un chiffre entre 0 et 7."
    );

    Console::clear();

    if (choice == 0)
    {
        return openWaveInterface(player, wave);
    }

    if (choice == 1)
    {
        return CombatTargetMenu::openForAttack(
            player,
            wave,
            random
        );
    }

    if (choice == 2)
    {
        return CombatPotionMenu::openQuickHealing(player);
    }

    if (choice == 3)
    {
        return CombatPotionMenu::openAgainstWave(
            player,
            wave,
            random,
            PVE_POTION_DAMAGE_BONUS
        );
    }

    if (choice == 4)
    {
        EquipmentMenu::open(player);
        return false;
    }

    if (choice == 5)
    {
        return InventoryMenu::open(player);
    }

    if (choice == 6)
    {
        std::cout << player.getName() << " choisit de ne rien faire ce tour-ci." << std::endl;
        std::cout << "Parfois, survivre commence par attendre le bon moment." << std::endl;
        std::cout << std::endl;

        return true;
    }

    if (choice == 7)
    {
        escapeSucceeded = EscapeSystem::playerAttemptsEscape(player, random);
        return true;
    }

    return false;
}

bool PlayerWaveCombatTurn::openWaveInterface(
    Player& player,
    EnemyCombatQueue& wave
)
{
    std::cout << "========== INTERFACE ==========" << std::endl;
    std::cout << "0 : Retour" << std::endl;
    std::cout << "1 : Voir mes statistiques" << std::endl;
    std::cout << "2 : Inspecter les adversaires" << std::endl;
    std::cout << "3 : Voir l'état du combat" << std::endl;
    std::cout << "===============================" << std::endl;
    std::cout << std::endl;
    std::cout << "> ";

    int interfaceChoice = Console::askNumberBetween(
        0,
        3,
        "Choix invalide. Entre un chiffre entre 0 et 3."
    );

    Console::clear();

    if (interfaceChoice == 0)
    {
        return false;
    }

    if (interfaceChoice == 1)
    {
        player.displayStats();
        return false;
    }

    if (interfaceChoice == 2)
    {
        wave.displayActiveEnemies();
        wave.displayQueueSummary();
        return false;
    }

    if (interfaceChoice == 3)
    {
        wave.displayQueueSummary();
        return false;
    }

    return false;
}