// EN: EquipmentComparison.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: EquipmentComparison.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "interface/menu/equipment/EquipmentComparison.hpp"

#include "interface/menu/common/MessageScreen.hpp"
#include "interface/menu/equipment/EquipmentDisplay.hpp"

#include <string>
#include <vector>

namespace
{
    std::string signedNumber(int value)
    {
        return value >= 0 ? "+" + std::to_string(value) : std::to_string(value);
    }

    std::string weaponStateText(const Weapon& weapon)
    {
        return weapon.isBroken() ? "Cassée" : "Utilisable";
    }

    std::string armorStateText(const Armor& armor)
    {
        return armor.isBroken() ? "Cassée" : "Utilisable";
    }
}

void EquipmentComparison::displayWeaponComparison(
    const Player& player,
    const Weapon& newWeapon
)
{
    std::vector<std::string> lines;

    if (player.hasEquippedWeapon())
    {
        Weapon currentWeapon = player.getEquippedWeapon();

        lines.push_back("Arme actuelle : " + currentWeapon.getName());
        lines.push_back("- Dégâts : +" + std::to_string(currentWeapon.getMinDamageBonus()) + " à +" + std::to_string(currentWeapon.getMaxDamageBonus()));
        lines.push_back("- Critique : +" + std::to_string(currentWeapon.getCriticalBonus()));
        lines.push_back("- Durabilité : " + EquipmentDisplay::weaponDurabilityText(currentWeapon));
        lines.push_back("- État : " + weaponStateText(currentWeapon));
        lines.push_back("");
        lines.push_back("Nouvelle arme : " + newWeapon.getName());
        lines.push_back("- Dégâts : +" + std::to_string(newWeapon.getMinDamageBonus()) + " à +" + std::to_string(newWeapon.getMaxDamageBonus()));
        lines.push_back("- Critique : +" + std::to_string(newWeapon.getCriticalBonus()));
        lines.push_back("- Durabilité : " + EquipmentDisplay::weaponDurabilityText(newWeapon));
        lines.push_back("- État : " + weaponStateText(newWeapon));
        lines.push_back("");
        lines.push_back("Différences si équipée :");
        lines.push_back("- Dégâts min : " + signedNumber(newWeapon.getMinDamageBonus() - currentWeapon.getMinDamageBonus()));
        lines.push_back("- Dégâts max : " + signedNumber(newWeapon.getMaxDamageBonus() - currentWeapon.getMaxDamageBonus()));
        lines.push_back("- Critique : " + signedNumber(newWeapon.getCriticalBonus() - currentWeapon.getCriticalBonus()));
    }
    else
    {
        lines.push_back("Arme actuelle : aucune");
        lines.push_back("Nouvelle arme : " + newWeapon.getName());
        lines.push_back("- Dégâts : +" + std::to_string(newWeapon.getMinDamageBonus()) + " à +" + std::to_string(newWeapon.getMaxDamageBonus()));
        lines.push_back("- Critique : +" + std::to_string(newWeapon.getCriticalBonus()));
        lines.push_back("- Durabilité : " + EquipmentDisplay::weaponDurabilityText(newWeapon));
        lines.push_back("- État : " + weaponStateText(newWeapon));
        lines.push_back("Équiper cette arme ajouterait ses bonus si elle n'est pas cassée.");
    }

    if (newWeapon.isBroken())
    {
        lines.push_back("Attention : cette arme est cassée. Ses bonus ne s'appliqueront pas avant réparation.");
    }

    MessageScreen::show("COMPARAISON D'ARME", "equipment.weapon.compare.result", lines);
}

void EquipmentComparison::displayArmorComparison(
    const Player& player,
    const Armor& newArmor
)
{
    std::vector<std::string> lines;

    if (player.hasEquippedArmor())
    {
        Armor currentArmor = player.getEquippedArmor();

        lines.push_back("Armure actuelle : " + currentArmor.getName());
        lines.push_back("- PV max : +" + std::to_string(currentArmor.getMaxHpBonus()));
        lines.push_back("- Réduction : " + std::to_string(currentArmor.getDamageReduction()));
        lines.push_back("- Durabilité : " + EquipmentDisplay::armorDurabilityText(currentArmor));
        lines.push_back("- État : " + armorStateText(currentArmor));
        lines.push_back("");
        lines.push_back("Nouvelle armure : " + newArmor.getName());
        lines.push_back("- PV max : +" + std::to_string(newArmor.getMaxHpBonus()));
        lines.push_back("- Réduction : " + std::to_string(newArmor.getDamageReduction()));
        lines.push_back("- Durabilité : " + EquipmentDisplay::armorDurabilityText(newArmor));
        lines.push_back("- État : " + armorStateText(newArmor));
        lines.push_back("");
        lines.push_back("Différences si équipée :");
        lines.push_back("- PV max : " + signedNumber(newArmor.getMaxHpBonus() - currentArmor.getMaxHpBonus()));
        lines.push_back("- Réduction : " + signedNumber(newArmor.getDamageReduction() - currentArmor.getDamageReduction()));
    }
    else
    {
        lines.push_back("Armure actuelle : aucune");
        lines.push_back("Nouvelle armure : " + newArmor.getName());
        lines.push_back("- PV max : +" + std::to_string(newArmor.getMaxHpBonus()));
        lines.push_back("- Réduction : " + std::to_string(newArmor.getDamageReduction()));
        lines.push_back("- Durabilité : " + EquipmentDisplay::armorDurabilityText(newArmor));
        lines.push_back("- État : " + armorStateText(newArmor));
        lines.push_back("Équiper cette armure modifierait les PV maximum et la réduction si elle n'est pas cassée.");
    }

    if (newArmor.isBroken())
    {
        lines.push_back("Attention : cette armure est cassée. Ses protections ne s'appliqueront pas avant réparation.");
    }

    MessageScreen::show("COMPARAISON D'ARMURE", "equipment.armor.compare.result", lines);
}
