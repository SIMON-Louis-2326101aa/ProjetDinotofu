// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "combat/turn/HumanCombatTurn.hpp"

#include "combat/CombatActions.hpp"
#include "combat/system/EscapeSystem.hpp"
#include "combat/system/ObservationSystem.hpp"

#include "core/Console.hpp"

#include "entity/Player.hpp"
#include "entity/Boss.hpp"

#include "interface/menu/CombatMenu.hpp"
#include "interface/menu/EquipmentMenu.hpp"
#include "interface/menu/InventoryMenu.hpp"
#include "interface/menu/CombatPotionMenu.hpp"

#include <iostream>

bool HumanCombatTurn::jouer(
    Entity& attacker,
    Entity& defender,
    Random& random,
    int potionHealAmount,
    int potionDamageBonus
)
{
    (void)potionHealAmount;

    CombatMenu::displayTurnMenu(attacker);

    int option = Console::askNumberBetween(
        0,
        7,
        "Choix invalide. Entre un chiffre entre 0 et 7."
    );

    Console::clear();

    if (option == 0)
    {
        return openObservationInterface(attacker, defender);
    }

    if (option == 1)
    {
        CombatActions::executeAttack(attacker, defender, random);
        return true;
    }

    if (option == 2)
    {
        Player* player = dynamic_cast<Player*>(&attacker);

        if (player == nullptr)
        {
            CombatMenu::displayUnavailableOption();
            return false;
        }

        return CombatPotionMenu::openQuickHealing(*player);
    }

    if (option == 3)
    {
        Player* player = dynamic_cast<Player*>(&attacker);

        if (player == nullptr)
        {
            CombatMenu::displayUnavailableOption();
            return false;
        }

        return CombatPotionMenu::openAgainstSingleTarget(
            *player,
            defender,
            random,
            potionDamageBonus
        );
    }

    if (option == 4)
    {
        Player* player = dynamic_cast<Player*>(&attacker);

        if (player == nullptr)
        {
            CombatMenu::displayUnavailableOption();
            return false;
        }

        EquipmentMenu::open(*player);
        return false;
    }

    if (option == 5)
    {
        Player* player = dynamic_cast<Player*>(&attacker);

        if (player == nullptr)
        {
            CombatMenu::displayUnavailableOption();
            return false;
        }

        return InventoryMenu::open(*player);
    }

    if (option == 6)
    {
        std::cout << attacker.getName() << " baisse sa garde et passe son tour." << std::endl;
        std::cout << "Parfois, attendre le bon moment est déjà une décision." << std::endl;
        std::cout << std::endl;

        return true;
    }

    if (option == 7)
    {
        return gererFuite(attacker, defender, random);
    }

    return false;
}

bool HumanCombatTurn::openObservationInterface(
    Entity& joueurInterface,
    Entity& target
)
{
    std::cout << "========== INTERFACE ==========" << std::endl;
    std::cout << "0 : Retour" << std::endl;
    std::cout << "1 : Voir mes statistiques" << std::endl;
    std::cout << "2 : Inspecter l'adversaire" << std::endl;
    std::cout << "===============================" << std::endl;
    std::cout << std::endl;
    std::cout << "> ";

    int choice = Console::askNumberBetween(
        0,
        2,
        "Choix invalide. Entre 0, 1 ou 2."
    );

    Console::clear();

    if (choice == 0)
    {
        return false;
    }

    if (choice == 1)
    {
        joueurInterface.displayStats();
        return false;
    }

    if (choice == 2)
    {
        ObservationSystem::displayTerminalStats(target);
        return false;
    }

    return false;
}

bool HumanCombatTurn::gererFuite(
    Entity& attacker,
    Entity& defender,
    Random& random
)
{
    Boss* bossCible = dynamic_cast<Boss*>(&defender);

    if (bossCible != nullptr)
    {
        Player* player = dynamic_cast<Player*>(&attacker);

        if (player != nullptr)
        {
            EscapeSystem::playerAttemptsBossEscape(*player, *bossCible);
        }
        else
        {
            std::cout << "[la fuite est impossible durant ce combat]" << std::endl;
            std::cout << std::endl;
        }

        return true;
    }

    Player* player = dynamic_cast<Player*>(&attacker);

    if (player == nullptr)
    {
        CombatMenu::displayUnavailableOption();
        return false;
    }

    EscapeSystem::playerAttemptsDuelEscape(*player, defender, random);

    return true;
}