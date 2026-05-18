// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "interface/menu/equipment/EquipmentDisplay.hpp"

#include <iostream>
#include <string>

void EquipmentDisplay::displayMainMenu()
{
    std::cout << "========== ÉQUIPEMENT ==========" << std::endl;
    std::cout << "0 : Retour" << std::endl;
    std::cout << "1 : Voir équipement simple" << std::endl;
    std::cout << "2 : Voir équipement détaillé" << std::endl;
    std::cout << "3 : Changer arme rapide" << std::endl;
    std::cout << "4 : Changer tenue rapide" << std::endl;
    std::cout << "================================" << std::endl;
    std::cout << std::endl;
    std::cout << "> ";
}

void EquipmentDisplay::displayWeaponList(const Player& player)
{
    std::cout << "========== CHANGER ARME ==========" << std::endl;

    for (int i = 0; i < player.getInventory().getWeaponCount(); ++i)
    {
        Weapon weapon = player.getInventory().getWeapon(i);
        displayWeaponSummary(weapon, i);
    }

    std::cout << "==================================" << std::endl;
    std::cout << "Choisis l'arme à équiper." << std::endl;
    std::cout << "Entre son numéro, ou -1 pour revenir." << std::endl;
    std::cout << "> ";
}

void EquipmentDisplay::displayArmorList(const Player& player)
{
    std::cout << "========== CHANGER TENUE ==========" << std::endl;

    for (int i = 0; i < player.getInventory().getArmorCount(); ++i)
    {
        Armor armor = player.getInventory().getArmor(i);
        displayArmorSummary(armor, i);
    }

    std::cout << "===================================" << std::endl;
    std::cout << "Choisis l'armure à équiper." << std::endl;
    std::cout << "Entre son numéro, ou -1 pour revenir." << std::endl;
    std::cout << "> ";
}

void EquipmentDisplay::displaySelectedWeapon(const Weapon& weapon)
{
    std::cout << "========== ARME SÉLECTIONNÉE ==========" << std::endl;
    std::cout << "Arme : " << weapon.getName() << std::endl;
    std::cout << "Durabilité : " << weaponDurabilityText(weapon) << std::endl;
    std::cout << "=======================================" << std::endl;
    std::cout << "0 : Retour" << std::endl;
    std::cout << "1 : Inspecter" << std::endl;
    std::cout << "2 : Comparer" << std::endl;
    std::cout << "3 : Équiper" << std::endl;
    std::cout << std::endl;
    std::cout << "> ";
}

void EquipmentDisplay::displaySelectedArmor(const Armor& armor)
{
    std::cout << "========== ARMURE SÉLECTIONNÉE ==========" << std::endl;
    std::cout << "Armure : " << armor.getName() << std::endl;
    std::cout << "Durabilité : " << armorDurabilityText(armor) << std::endl;
    std::cout << "=========================================" << std::endl;
    std::cout << "0 : Retour" << std::endl;
    std::cout << "1 : Inspecter" << std::endl;
    std::cout << "2 : Comparer" << std::endl;
    std::cout << "3 : Équiper" << std::endl;
    std::cout << std::endl;
    std::cout << "> ";
}

void EquipmentDisplay::displayWeaponSummary(const Weapon& weapon, int index)
{
    std::cout << index << " : " << weapon.getName()
              << " | Dégâts : +" << weapon.getMinDamageBonus()
              << " à +" << weapon.getMaxDamageBonus()
              << " | Critique : +" << weapon.getCriticalBonus()
              << " | Durabilité : " << weaponDurabilityText(weapon);

    if (weapon.isBroken())
    {
        std::cout << " | Cassée";
    }

    std::cout << std::endl;
}

void EquipmentDisplay::displayArmorSummary(const Armor& armor, int index)
{
    std::cout << index << " : " << armor.getName()
              << " | PV max : +" << armor.getMaxHpBonus()
              << " | Réduction : " << armor.getDamageReduction()
              << " | Durabilité : " << armorDurabilityText(armor);

    if (armor.isBroken())
    {
        std::cout << " | Cassée";
    }

    std::cout << std::endl;
}

std::string EquipmentDisplay::weaponDurabilityText(const Weapon& weapon)
{
    if (weapon.isIndestructible())
    {
        return "indestructible";
    }

    return std::to_string(weapon.getDurability()) + "/" + std::to_string(weapon.getMaxDurability());
}

std::string EquipmentDisplay::armorDurabilityText(const Armor& armor)
{
    if (armor.isIndestructible())
    {
        return "indestructible";
    }

    return std::to_string(armor.getDurability()) + "/" + std::to_string(armor.getMaxDurability());
}