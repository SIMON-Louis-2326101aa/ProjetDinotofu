// EN: EquipmentMenu.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: EquipmentMenu.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "interface/menu/EquipmentMenu.hpp"

#include "core/Console.hpp"

#include "interface/menu/equipment/EquipmentDisplay.hpp"
#include "interface/menu/equipment/EquipmentComparison.hpp"

#include <iostream>

// EN: open declares or implements a focused behavior used by this module.
// FR: open déclare ou implémente un comportement précis utilisé par ce module.
bool EquipmentMenu::open(Player& player)
{
    while (true)
    {
        EquipmentDisplay::displayMainMenu();

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
            player.displaySimpleEquipment();
            continue;
        }

        if (choice == 2)
        {
            player.displayDetailedEquipment();
            continue;
        }

        if (choice == 3)
        {
            equipWeaponFromInventory(player);
            continue;
        }

        if (choice == 4)
        {
            equipArmorFromInventory(player);
            continue;
        }
    }
}

// EN: equipWeaponFromInventory declares or implements a focused behavior used by this module.
// FR: equipWeaponFromInventory déclare ou implémente un comportement précis utilisé par ce module.
bool EquipmentMenu::equipWeaponFromInventory(Player& player)
{
    if (player.getInventory().getWeaponCount() <= 0)
    {
        std::cout << player.getName() << " n'a aucune arme à équiper." << std::endl;
        std::cout << std::endl;
        return false;
    }

    EquipmentDisplay::displayWeaponList(player);

    int choice = Console::askNumberBetween(
        -1,
        player.getInventory().getWeaponCount() - 1,
        "Choix invalide. Entre un numéro d'arme valide, ou -1 pour revenir."
    );

    Console::clear();

    if (choice == -1)
    {
        return false;
    }

    if (!player.getInventory().hasWeapon(choice))
    {
        std::cout << "Cette arme n'existe pas dans ton inventaire." << std::endl;
        std::cout << std::endl;
        return false;
    }

    Weapon newWeapon = player.getInventory().getWeapon(choice);

    EquipmentDisplay::displaySelectedWeapon(newWeapon);

    int action = Console::askNumberBetween(
        0,
        3,
        "Choix invalide. Entre un chiffre entre 0 et 3."
    );

    Console::clear();

    if (action == 0)
    {
        return false;
    }

    if (action == 1)
    {
        player.getInventory().inspectWeapon(choice);
        return false;
    }

    if (action == 2)
    {
        EquipmentComparison::displayWeaponComparison(player, newWeapon);
        return false;
    }

    if (!player.equipWeapon(choice))
    {
        std::cout << "Impossible d'équiper cette arme." << std::endl;
        std::cout << std::endl;
        return false;
    }

    Weapon equippedWeapon = player.getEquippedWeapon();

    std::cout << player.getName() << " équipe : " << equippedWeapon.getName() << "." << std::endl;

    if (equippedWeapon.isBroken())
    {
        std::cout << "Attention : cette arme est cassée, elle ne donnera aucun bonus." << std::endl;
    }
    else
    {
        std::cout << "La prise en main est bonne. Cette arme est prête au combat." << std::endl;
    }

    std::cout << std::endl;

    return false;
}

// EN: equipArmorFromInventory declares or implements a focused behavior used by this module.
// FR: equipArmorFromInventory déclare ou implémente un comportement précis utilisé par ce module.
bool EquipmentMenu::equipArmorFromInventory(Player& player)
{
    if (player.getInventory().getArmorCount() <= 0)
    {
        std::cout << player.getName() << " n'a aucune armure à équiper." << std::endl;
        std::cout << std::endl;
        return false;
    }

    EquipmentDisplay::displayArmorList(player);

    int choice = Console::askNumberBetween(
        -1,
        player.getInventory().getArmorCount() - 1,
        "Choix invalide. Entre un numéro d'armure valide, ou -1 pour revenir."
    );

    Console::clear();

    if (choice == -1)
    {
        return false;
    }

    if (!player.getInventory().hasArmor(choice))
    {
        std::cout << "Cette armure n'existe pas dans ton inventaire." << std::endl;
        std::cout << std::endl;
        return false;
    }

    Armor newArmor = player.getInventory().getArmor(choice);

    EquipmentDisplay::displaySelectedArmor(newArmor);

    int action = Console::askNumberBetween(
        0,
        3,
        "Choix invalide. Entre un chiffre entre 0 et 3."
    );

    Console::clear();

    if (action == 0)
    {
        return false;
    }

    if (action == 1)
    {
        player.getInventory().inspectArmor(choice);
        return false;
    }

    if (action == 2)
    {
        EquipmentComparison::displayArmorComparison(player, newArmor);
        return false;
    }

    if (!player.equipArmor(choice))
    {
        std::cout << "Impossible d'équiper cette armure." << std::endl;
        std::cout << std::endl;
        return false;
    }

    Armor equippedArmor = player.getEquippedArmor();

    std::cout << player.getName() << " équipe : " << equippedArmor.getName() << "." << std::endl;

    if (equippedArmor.isBroken())
    {
        std::cout << "Attention : cette armure est cassée, elle ne donnera aucun bonus." << std::endl;
    }
    else
    {
        std::cout << "Ses protections sont maintenant actives." << std::endl;
    }

    std::cout << player.getName() << " possède maintenant "
              << player.getHp()
              << "/"
              << player.getMaxHp()
              << " PV."
              << std::endl;

    std::cout << std::endl;

    return false;
}
