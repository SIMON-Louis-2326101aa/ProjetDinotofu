// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "interface/menu/inventory/InventoryDisplay.hpp"

#include "interface/menu/inventory/InventoryUtils.hpp"

#include "item/Inventory.hpp"
#include "item/consumable/Consumable.hpp"

#include <iostream>
#include <vector>

void InventoryDisplay::displayMainMenu()
{
    std::cout << "================ INVENTAIRE ================" << std::endl;
    std::cout << "0 : Retour" << std::endl;
    std::cout << "1 : Voir tout" << std::endl;
    std::cout << "2 : Voir les armes" << std::endl;
    std::cout << "3 : Voir les armures" << std::endl;
    std::cout << "4 : Voir les consommables" << std::endl;
    std::cout << "5 : Voir les matériaux" << std::endl;
    std::cout << "============================================" << std::endl;
    std::cout << std::endl;
    std::cout << "> ";
}

void InventoryDisplay::displaySimpleFullInventory(const Player& player)
{
    const Inventory& inventory = player.getInventory();

    std::cout << "================ INVENTAIRE ================" << std::endl;
    std::cout << "Or : " << inventory.getGold() << " pièces" << std::endl;
    std::cout << std::endl;

    std::cout << "Armes : " << inventory.getWeaponCount() << std::endl;

    for (int i = 0; i < inventory.getWeaponCount(); ++i)
    {
        Weapon weapon = inventory.getWeapon(i);

        std::cout << "[" << i << "] " << weapon.getName()
                  << " | Durabilité : " << InventoryUtils::weaponDurabilityText(weapon);

        if (weapon.isBroken())
        {
            std::cout << " | Cassée";
        }

        std::cout << std::endl;
    }

    std::cout << std::endl;
    std::cout << "Armures : " << inventory.getArmorCount() << std::endl;

    for (int i = 0; i < inventory.getArmorCount(); ++i)
    {
        Armor armor = inventory.getArmor(i);

        std::cout << "[" << i << "] " << armor.getName()
                  << " | Durabilité : " << InventoryUtils::armorDurabilityText(armor);

        if (armor.isBroken())
        {
            std::cout << " | Cassée";
        }

        std::cout << std::endl;
    }

    std::cout << std::endl;
    std::cout << "Consommables : " << inventory.getConsumableCount() << std::endl;

    std::vector<GroupeConsommable> groups = InventoryUtils::grouperConsommables(player);

    for (int i = 0; i < static_cast<int>(groups.size()); ++i)
    {
        const GroupeConsommable& group = groups[i];

        std::cout << "[" << i << "] " << group.name
                  << " x" << group.amount
                  << " | " << InventoryUtils::typeConsommableVersTexte(group.type)
                  << " | Puissance : " << group.power
                  << std::endl;
    }

    std::cout << std::endl;
    std::cout << "Matériaux : pas encore disponibles" << std::endl;
    std::cout << "============================================" << std::endl;
    std::cout << std::endl;
}

void InventoryDisplay::displaySelectedWeapon(const Weapon& weapon)
{
    std::cout << "========== ARME SÉLECTIONNÉE ==========" << std::endl;
    std::cout << "Arme : " << weapon.getName() << std::endl;
    std::cout << "=======================================" << std::endl;
    std::cout << "0 : Retour" << std::endl;
    std::cout << "1 : Inspecter" << std::endl;
    std::cout << "2 : Équiper" << std::endl;
    std::cout << std::endl;
    std::cout << "> ";
}

void InventoryDisplay::displaySelectedArmor(const Armor& armor)
{
    std::cout << "========== ARMURE SÉLECTIONNÉE ==========" << std::endl;
    std::cout << "Armure : " << armor.getName() << std::endl;
    std::cout << "=========================================" << std::endl;
    std::cout << "0 : Retour" << std::endl;
    std::cout << "1 : Inspecter" << std::endl;
    std::cout << "2 : Équiper" << std::endl;
    std::cout << std::endl;
    std::cout << "> ";
}

void InventoryDisplay::displaySelectedConsumable(const Consumable& consumable)
{
    std::cout << "========== CONSOMMABLE SÉLECTIONNÉ ==========" << std::endl;
    std::cout << "Consommable : " << consumable.getName() << std::endl;
    std::cout << "Type : " << InventoryUtils::typeConsommableVersTexte(consumable.getType()) << std::endl;
    std::cout << "Puissance : " << consumable.getPower() << std::endl;
    std::cout << "=============================================" << std::endl;
    std::cout << "0 : Retour" << std::endl;
    std::cout << "1 : Inspecter" << std::endl;

    if (consumable.getType() == ConsumableType::Healing)
    {
        std::cout << "2 : Utiliser" << std::endl;
    }
    else
    {
        std::cout << "2 : Utiliser (à faire depuis le menu Potions en combat)" << std::endl;
    }

    std::cout << std::endl;
    std::cout << "> ";
}

void InventoryDisplay::displayUnavailableMaterials()
{
    std::cout << "========== MATÉRIAUX ==========" << std::endl;
    std::cout << "Les matériaux ne sont pas encore disponibles." << std::endl;
    std::cout << "Plus tard, ils serviront à réparer, améliorer et fabriquer de l'équipement." << std::endl;
    std::cout << "===============================" << std::endl;
    std::cout << std::endl;
}