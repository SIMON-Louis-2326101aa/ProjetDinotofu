// EN: EquipmentComparison.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: EquipmentComparison.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "interface/menu/equipment/EquipmentComparison.hpp"

#include "interface/menu/equipment/EquipmentDisplay.hpp"

#include <iostream>

void EquipmentComparison::displayWeaponComparison(
    const Player& player,
    const Weapon& newWeapon
)
{
    std::cout << "=========== COMPARAISON D'ARME ===========" << std::endl;

    if (player.hasEquippedWeapon())
    {
        Weapon currentWeapon = player.getEquippedWeapon();

        std::cout << "Arme actuelle : " << currentWeapon.getName() << std::endl;
        std::cout << "Dégâts bonus : +"
                  << currentWeapon.getMinDamageBonus()
                  << " à +"
                  << currentWeapon.getMaxDamageBonus()
                  << std::endl;
        std::cout << "Critique bonus : +" << currentWeapon.getCriticalBonus() << std::endl;
        std::cout << "Durabilité : "
                  << EquipmentDisplay::weaponDurabilityText(currentWeapon)
                  << std::endl;

        if (currentWeapon.isBroken())
        {
            std::cout << "État : Cassée" << std::endl;
        }
        else
        {
            std::cout << "État : Utilisable" << std::endl;
        }

        std::cout << std::endl;
    }
    else
    {
        std::cout << "Arme actuelle : Aucune" << std::endl;
        std::cout << std::endl;
    }

    std::cout << "Nouvelle arme : " << newWeapon.getName() << std::endl;
    std::cout << "Dégâts bonus : +"
              << newWeapon.getMinDamageBonus()
              << " à +"
              << newWeapon.getMaxDamageBonus()
              << std::endl;
    std::cout << "Critique bonus : +" << newWeapon.getCriticalBonus() << std::endl;
    std::cout << "Durabilité : "
              << EquipmentDisplay::weaponDurabilityText(newWeapon)
              << std::endl;

    if (newWeapon.isBroken())
    {
        std::cout << "État : Cassée" << std::endl;
    }
    else
    {
        std::cout << "État : Utilisable" << std::endl;
    }

    if (player.hasEquippedWeapon())
    {
        Weapon currentWeapon = player.getEquippedWeapon();

        std::cout << std::endl;
        std::cout << "Différence dégâts min : "
                  << newWeapon.getMinDamageBonus() - currentWeapon.getMinDamageBonus()
                  << std::endl;
        std::cout << "Différence dégâts max : "
                  << newWeapon.getMaxDamageBonus() - currentWeapon.getMaxDamageBonus()
                  << std::endl;
        std::cout << "Différence critique : "
                  << newWeapon.getCriticalBonus() - currentWeapon.getCriticalBonus()
                  << std::endl;
    }

    std::cout << "===========================================" << std::endl;
    std::cout << std::endl;
}

void EquipmentComparison::displayArmorComparison(
    const Player& player,
    const Armor& newArmor
)
{
    std::cout << "========== COMPARAISON D'ARMURE ==========" << std::endl;

    if (player.hasEquippedArmor())
    {
        Armor currentArmor = player.getEquippedArmor();

        std::cout << "Armure actuelle : " << currentArmor.getName() << std::endl;
        std::cout << "Bonus PV max : +" << currentArmor.getMaxHpBonus() << std::endl;
        std::cout << "Réduction dégâts : " << currentArmor.getDamageReduction() << std::endl;
        std::cout << "Durabilité : "
                  << EquipmentDisplay::armorDurabilityText(currentArmor)
                  << std::endl;

        if (currentArmor.isBroken())
        {
            std::cout << "État : Cassée" << std::endl;
        }
        else
        {
            std::cout << "État : Utilisable" << std::endl;
        }

        std::cout << std::endl;
    }
    else
    {
        std::cout << "Armure actuelle : Aucune" << std::endl;
        std::cout << std::endl;
    }

    std::cout << "Nouvelle armure : " << newArmor.getName() << std::endl;
    std::cout << "Bonus PV max : +" << newArmor.getMaxHpBonus() << std::endl;
    std::cout << "Réduction dégâts : " << newArmor.getDamageReduction() << std::endl;
    std::cout << "Durabilité : "
              << EquipmentDisplay::armorDurabilityText(newArmor)
              << std::endl;

    if (newArmor.isBroken())
    {
        std::cout << "État : Cassée" << std::endl;
    }
    else
    {
        std::cout << "État : Utilisable" << std::endl;
    }

    if (player.hasEquippedArmor())
    {
        Armor currentArmor = player.getEquippedArmor();

        std::cout << std::endl;
        std::cout << "Différence PV max : "
                  << newArmor.getMaxHpBonus() - currentArmor.getMaxHpBonus()
                  << std::endl;
        std::cout << "Différence réduction : "
                  << newArmor.getDamageReduction() - currentArmor.getDamageReduction()
                  << std::endl;
    }

    std::cout << "===========================================" << std::endl;
    std::cout << std::endl;
}
