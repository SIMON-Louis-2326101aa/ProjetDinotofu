// EN: PlayerWaveCombatTurn.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: PlayerWaveCombatTurn.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "combat/turn/wave/PlayerWaveCombatTurn.hpp"

#include "combat/system/EscapeSystem.hpp"
#include "combat/system/DefensePostureSystem.hpp"
#include "combat/role/CombatRoleActionSystem.hpp"

#include "core/Console.hpp"

#include "interface/menu/CombatMenu.hpp"
#include "interface/menu/CombatTargetMenu.hpp"
#include "interface/menu/EquipmentMenu.hpp"
#include "interface/menu/InventoryMenu.hpp"
#include "interface/menu/CombatPotionMenu.hpp"
#include "interface/menu/CombatRoleMenu.hpp"
#include "interface/menu/progression/BestiaryMenu.hpp"
#include "interface/menu/progression/StatisticsMenu.hpp"

#include <iostream>

bool PlayerWaveCombatTurn::play(
    Player& player,
    EnemyCombatQueue& wave,
    Random& random,
    bool& escapeSucceeded,
    DifficultyMode difficulty
)
{
    for (int i = 0; i < wave.getActiveEnemyCount(); ++i)
    {
        CombatRoleActionSystem::tryActivateAutomaticRoleReaction(
            wave.getActiveEnemy(i),
            random
        );
    }

    CombatMenu::displayTurnMenu(player);

    int choice = Console::askNumberBetween(
        0,
        8,
        "Choix invalide. Entre un chiffre entre 0 et 8."
    );

    Console::clear();

    if (choice == 0)
    {
        return openWaveInterface(player, wave, difficulty);
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
        DefensePostureSystem::enterDefensePosture(player);
        return true;
    }

    if (choice == 7)
    {
        std::cout << player.getName() << " choisit de ne rien faire ce tour-ci." << std::endl;
        std::cout << "Parfois, survivre commence par attendre le bon moment." << std::endl;
        std::cout << std::endl;

        return true;
    }

    if (choice == 8)
    {
        escapeSucceeded = EscapeSystem::playerAttemptsEscape(
            player,
            random,
            difficulty,
            wave.getTotalRemainingEnemyCount()
        );
        return true;
    }

    return false;
}

bool PlayerWaveCombatTurn::openWaveInterface(
    Player& player,
    EnemyCombatQueue& wave,
    DifficultyMode difficulty
)
{
    std::cout << "========== INTERFACE ==========" << std::endl;
    std::cout << "0 : Retour" << std::endl;
    std::cout << "1 : Voir l'état du combat" << std::endl;
    std::cout << "2 : Voir mes statistiques" << std::endl;
    std::cout << "3 : Résumé équipement" << std::endl;
    std::cout << "4 : Compétences de rôle" << std::endl;
    std::cout << "5 : Observer / analyser les adversaires" << std::endl;
    std::cout << "6 : Voir un adversaire dans le bestiaire" << std::endl;
    std::cout << "7 : Ordres aux alliés" << std::endl;
    std::cout << "8 : Contrôle des invocations" << std::endl;
    std::cout << "===============================" << std::endl;
    std::cout << std::endl;
    std::cout << "> ";

    int interfaceChoice = Console::askNumberBetween(
        0,
        8,
        "Choix invalide. Entre un chiffre entre 0 et 8."
    );

    Console::clear();

    if (interfaceChoice == 0)
    {
        return false;
    }

    if (interfaceChoice == 1)
    {
        wave.displayActiveEnemies();
        wave.displayQueueSummary();
        return false;
    }

    if (interfaceChoice == 2)
    {
        StatisticsMenu::open(player, difficulty);
        return false;
    }

    if (interfaceChoice == 3)
    {
        player.displaySimpleEquipment();
        return false;
    }

    if (interfaceChoice == 4)
    {
        return CombatRoleMenu::open(player);
    }

    if (interfaceChoice == 5)
    {
        wave.displayActiveEnemies();
        wave.displayQueueSummary();
        return false;
    }

    if (interfaceChoice == 6)
    {
        if (!wave.hasActiveEnemies())
        {
            std::cout << "Aucun adversaire actif à consulter dans le bestiaire." << std::endl;
            std::cout << std::endl;
            return false;
        }

        wave.displayActiveEnemies();
        std::cout << "Choisis l'adversaire à rechercher dans le bestiaire." << std::endl;
        std::cout << "0 : Retour" << std::endl;
        std::cout << "> ";

        int targetChoice = Console::askNumberBetween(
            0,
            wave.getActiveEnemyCount(),
            "Choix invalide."
        );

        Console::clear();

        if (targetChoice == 0)
        {
            return false;
        }

        BestiaryMenu::displayObjectEntry(
            wave.getActiveEnemy(targetChoice - 1).getName()
        );

        return false;
    }

    if (interfaceChoice == 7)
    {
        std::cout << "Les ordres aux alliés seront disponibles quand les alliés permanents seront branchés." << std::endl;
        std::cout << std::endl;
        return false;
    }

    if (interfaceChoice == 8)
    {
        std::cout << "Le contrôle des invocations se choisit déjà au début du combat si tu possèdes des invocations." << std::endl;
        std::cout << "Plus tard, cette option permettra de changer les ordres pendant le combat." << std::endl;
        std::cout << std::endl;
        return false;
    }

    return false;
}
