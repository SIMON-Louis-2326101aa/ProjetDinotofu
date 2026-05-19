// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "interface/menu/inventory/InventorySelection.hpp"

#include "combat/threat/ThreatSystem.hpp"

#include "core/Console.hpp"

#include "interface/menu/inventory/InventoryDisplay.hpp"
#include "interface/menu/inventory/InventoryUtils.hpp"
#include "interface/menu/progression/BestiaryMenu.hpp"

#include "item/Inventory.hpp"
#include "item/weapon/Weapon.hpp"
#include "item/armor/Armor.hpp"
#include "item/consumable/Consumable.hpp"
#include "item/consumable/ConsumableType.hpp"

#include <iostream>
#include <vector>

bool InventorySelection::openWeapons(Player& player)
{
    if (player.getInventory().getWeaponCount() <= 0)
    {
        std::cout << "Tu n'as aucune arme dans ton inventaire." << std::endl;
        std::cout << std::endl;
        return false;
    }

    player.getInventory().displayWeaponList();

    std::cout << "Sélectionne une arme, ou entre -1 pour revenir." << std::endl;
    std::cout << "> ";

    int index = Console::askNumberBetween(
        -1,
        player.getInventory().getWeaponCount() - 1,
        "Choix invalide. Entre un numéro d'arme valide, ou -1 pour revenir."
    );

    Console::clear();

    if (index == -1)
    {
        return false;
    }

    if (!player.getInventory().hasWeapon(index))
    {
        std::cout << "Cette arme n'existe pas dans ton inventaire." << std::endl;
        std::cout << std::endl;
        return false;
    }

    Weapon weapon = player.getInventory().getWeapon(index);
    InventoryDisplay::displaySelectedWeapon(weapon);

    int action = Console::askNumberBetween(
        0,
        3,
        "Choix invalide. Entre 0, 1, 2 ou 3."
    );

    Console::clear();

    if (action == 1)
    {
        player.getInventory().inspectWeapon(index);
        return false;
    }

    if (action == 3)
    {
        BestiaryMenu::displayObjectEntry(weapon.getName());
        return false;
    }

    if (action == 2)
    {
        if (player.equipWeapon(index))
        {
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
        }
        else
        {
            std::cout << "Impossible d'équiper cette arme." << std::endl;
            std::cout << std::endl;
        }
    }

    return false;
}

bool InventorySelection::openArmors(Player& player)
{
    if (player.getInventory().getArmorCount() <= 0)
    {
        std::cout << "Tu n'as aucune armure dans ton inventaire." << std::endl;
        std::cout << std::endl;
        return false;
    }

    player.getInventory().displayArmorList();

    std::cout << "Sélectionne une armure, ou entre -1 pour revenir." << std::endl;
    std::cout << "> ";

    int index = Console::askNumberBetween(
        -1,
        player.getInventory().getArmorCount() - 1,
        "Choix invalide. Entre un numéro d'armure valide, ou -1 pour revenir."
    );

    Console::clear();

    if (index == -1)
    {
        return false;
    }

    if (!player.getInventory().hasArmor(index))
    {
        std::cout << "Cette armure n'existe pas dans ton inventaire." << std::endl;
        std::cout << std::endl;
        return false;
    }

    Armor armor = player.getInventory().getArmor(index);
    InventoryDisplay::displaySelectedArmor(armor);

    int action = Console::askNumberBetween(
        0,
        3,
        "Choix invalide. Entre 0, 1, 2 ou 3."
    );

    Console::clear();

    if (action == 1)
    {
        player.getInventory().inspectArmor(index);
        return false;
    }

    if (action == 3)
    {
        BestiaryMenu::displayObjectEntry(armor.getName());
        return false;
    }

    if (action == 2)
    {
        if (player.equipArmor(index))
        {
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
        }
        else
        {
            std::cout << "Impossible d'équiper cette armure." << std::endl;
            std::cout << std::endl;
        }
    }

    return false;
}

bool InventorySelection::openConsumables(Player& player)
{
    if (player.getInventory().getConsumableCount() <= 0)
    {
        std::cout << "Tu n'as aucun consommable dans ton inventaire." << std::endl;
        std::cout << std::endl;
        return false;
    }

    std::vector<ConsumableGroup> groups = InventoryUtils::groupConsumables(player);

    std::cout << "============ CONSOMMABLES ============" << std::endl;

    for (int i = 0; i < static_cast<int>(groups.size()); ++i)
    {
        const ConsumableGroup& group = groups[i];

        std::cout << i + 1
                  << " : "
                  << group.name
                  << " x"
                  << group.amount
                  << " | "
                  << InventoryUtils::consumableTypeToText(group.type)
                  << " | Puissance : "
                  << group.power
                  << std::endl;
    }

    std::cout << "=======================================" << std::endl;
    std::cout << "Sélectionne un consommable, ou entre 0 pour revenir." << std::endl;
    std::cout << "> ";

    int choice = Console::askNumberBetween(
        0,
        static_cast<int>(groups.size()),
        "Choix invalide. Sélectionne un consommable affiché, ou 0 pour revenir."
    );

    Console::clear();

    if (choice == 0)
    {
        return false;
    }

    int index = groups[choice - 1].firstIndex;

    if (!player.getInventory().hasConsumable(index))
    {
        std::cout << "Ce consommable n'existe plus dans ton inventaire." << std::endl;
        std::cout << std::endl;
        return false;
    }

    Consumable consumable = player.getInventory().getConsumable(index);

    InventoryDisplay::displaySelectedConsumable(consumable);

    int action = Console::askNumberBetween(
        0,
        3,
        "Choix invalide. Entre 0, 1, 2 ou 3."
    );

    Console::clear();

    if (action == 1)
    {
        player.getInventory().inspectConsumable(index);
        return false;
    }

    if (action == 3)
    {
        BestiaryMenu::displayObjectEntry(consumable.getName());
        return false;
    }

    if (action == 2)
    {
        if (consumable.getType() != ConsumableType::Healing)
        {
            std::cout << "Ce consommable demande une cible ou un effet spécial." << std::endl;
            std::cout << "Utilise plutôt l'option Potions du menu de combat." << std::endl;
            std::cout << std::endl;
            return false;
        }

        player.heal(consumable.getPower());
        ThreatSystem::markSelfHealingAction(player);
        player.getInventory().removeConsumable(index);

        std::cout << player.getName() << " utilise : " << consumable.getName() << "." << std::endl;
        std::cout << "Ses blessures se referment, et il récupère "
                  << consumable.getPower()
                  << " PV."
                  << std::endl;
        std::cout << player.getName() << " possède maintenant "
                  << player.getHp()
                  << "/"
                  << player.getMaxHp()
                  << " PV."
                  << std::endl;
        std::cout << std::endl;

        return true;
    }

    return false;
}

bool InventorySelection::openMaterials(Player& player)
{
    (void)player;

    InventoryDisplay::displayUnavailableMaterials();

    return false;
}