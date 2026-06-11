// EN: EquipmentComparison.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: EquipmentComparison.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "interface/menu/equipment/EquipmentComparison.hpp"

#include "interface/menu/common/MessageScreen.hpp"
#include "interface/menu/equipment/EquipmentDisplay.hpp"

#include <algorithm>
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

    int durabilityPercent(int durability, int maximum)
    {
        if (maximum <= 0) return 0;
        return std::clamp((durability * 100) / maximum, 0, 100);
    }

    std::string comparisonRow(const std::string& stat, int current, int inspected, const std::string& suffix = "")
    {
        return "Comparaison | " + stat + " | " + std::to_string(current) + suffix
            + " | " + std::to_string(inspected) + suffix
            + " | " + signedNumber(inspected - current) + suffix;
    }

    std::string comparisonVerdict(int improvements, int regressions, bool broken)
    {
        if (broken) return "Verdict : inutilisable avant réparation.";
        if (improvements > 0 && regressions == 0) return "Verdict : amélioration directe.";
        if (improvements == 0 && regressions > 0) return "Verdict : moins performante sur les statistiques comparées.";
        if (improvements > 0 && regressions > 0) return "Verdict : compromis — certains bonus montent, d'autres baissent.";
        return "Verdict : statistiques principales équivalentes.";
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
        const Weapon currentWeapon = player.getEquippedWeapon();
        const int currentDurability = durabilityPercent(currentWeapon.getDurability(), currentWeapon.getMaxDurability());
        const int inspectedDurability = durabilityPercent(newWeapon.getDurability(), newWeapon.getMaxDurability());
        const std::vector<int> differences = {
            newWeapon.getMinDamageBonus() - currentWeapon.getMinDamageBonus(),
            newWeapon.getMaxDamageBonus() - currentWeapon.getMaxDamageBonus(),
            newWeapon.getCriticalBonus() - currentWeapon.getCriticalBonus(),
            newWeapon.getEnchantmentCount() - currentWeapon.getEnchantmentCount(),
            inspectedDurability - currentDurability
        };
        const int improvements = static_cast<int>(std::count_if(differences.begin(), differences.end(), [](int value) { return value > 0; }));
        const int regressions = static_cast<int>(std::count_if(differences.begin(), differences.end(), [](int value) { return value < 0; }));

        lines.push_back("Actuel : " + currentWeapon.getName() + " — " + weaponStateText(currentWeapon));
        lines.push_back("Inspecté : " + newWeapon.getName() + " — " + weaponStateText(newWeapon));
        lines.push_back("");
        lines.push_back("Comparaison | Statistique | Actuel | Inspecté | Écart");
        lines.push_back(comparisonRow("Dégâts minimum", currentWeapon.getMinDamageBonus(), newWeapon.getMinDamageBonus()));
        lines.push_back(comparisonRow("Dégâts maximum", currentWeapon.getMaxDamageBonus(), newWeapon.getMaxDamageBonus()));
        lines.push_back(comparisonRow("Critique", currentWeapon.getCriticalBonus(), newWeapon.getCriticalBonus()));
        lines.push_back(comparisonRow("Enchantements", currentWeapon.getEnchantmentCount(), newWeapon.getEnchantmentCount()));
        lines.push_back(comparisonRow("Durabilité", currentDurability, inspectedDurability, "%"));
        lines.push_back("");
        lines.push_back("Effets actuels : " + currentWeapon.getEnchantmentSummaryText() + ".");
        lines.push_back("Effets inspectés : " + newWeapon.getEnchantmentSummaryText() + ".");
        lines.push_back("Les résistances ne sont comparées que lorsqu'elles existent réellement dans les données de l'objet.");
        lines.push_back("");
        lines.push_back(comparisonVerdict(improvements, regressions, newWeapon.isBroken()));
    }
    else
    {
        lines.push_back("Actuel : aucune arme équipée.");
        lines.push_back("Inspecté : " + newWeapon.getName() + " — " + weaponStateText(newWeapon));
        lines.push_back("");
        lines.push_back("Comparaison | Statistique | Actuel | Inspecté | Écart");
        lines.push_back(comparisonRow("Dégâts minimum", 0, newWeapon.getMinDamageBonus()));
        lines.push_back(comparisonRow("Dégâts maximum", 0, newWeapon.getMaxDamageBonus()));
        lines.push_back(comparisonRow("Critique", 0, newWeapon.getCriticalBonus()));
        lines.push_back(comparisonRow("Enchantements", 0, newWeapon.getEnchantmentCount()));
        lines.push_back(comparisonRow("Durabilité", 0, durabilityPercent(newWeapon.getDurability(), newWeapon.getMaxDurability()), "%"));
        lines.push_back("");
        lines.push_back("Effets inspectés : " + newWeapon.getEnchantmentSummaryText() + ".");
        lines.push_back("");
        lines.push_back(newWeapon.isBroken() ? "Verdict : inutilisable avant réparation." : "Verdict : ajoute tous les bonus affichés.");
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
        const Armor currentArmor = player.getEquippedArmor();
        const int currentDurability = durabilityPercent(currentArmor.getDurability(), currentArmor.getMaxDurability());
        const int inspectedDurability = durabilityPercent(newArmor.getDurability(), newArmor.getMaxDurability());
        const std::vector<int> differences = {
            newArmor.getMaxHpBonus() - currentArmor.getMaxHpBonus(),
            newArmor.getDamageReduction() - currentArmor.getDamageReduction(),
            newArmor.getEnchantmentCount() - currentArmor.getEnchantmentCount(),
            inspectedDurability - currentDurability
        };
        const int improvements = static_cast<int>(std::count_if(differences.begin(), differences.end(), [](int value) { return value > 0; }));
        const int regressions = static_cast<int>(std::count_if(differences.begin(), differences.end(), [](int value) { return value < 0; }));

        lines.push_back("Actuel : " + currentArmor.getName() + " — " + armorStateText(currentArmor));
        lines.push_back("Inspecté : " + newArmor.getName() + " — " + armorStateText(newArmor));
        lines.push_back("");
        lines.push_back("Comparaison | Statistique | Actuel | Inspecté | Écart");
        lines.push_back(comparisonRow("PV maximum", currentArmor.getMaxHpBonus(), newArmor.getMaxHpBonus()));
        lines.push_back(comparisonRow("Réduction", currentArmor.getDamageReduction(), newArmor.getDamageReduction()));
        lines.push_back(comparisonRow("Enchantements", currentArmor.getEnchantmentCount(), newArmor.getEnchantmentCount()));
        lines.push_back(comparisonRow("Durabilité", currentDurability, inspectedDurability, "%"));
        lines.push_back("");
        lines.push_back("Effets actuels : " + currentArmor.getEnchantmentSummaryText() + ".");
        lines.push_back("Effets inspectés : " + newArmor.getEnchantmentSummaryText() + ".");
        lines.push_back("Les résistances ne sont comparées que lorsqu'elles existent réellement dans les données de l'objet.");
        lines.push_back("");
        lines.push_back(comparisonVerdict(improvements, regressions, newArmor.isBroken()));
    }
    else
    {
        lines.push_back("Actuel : aucune armure équipée.");
        lines.push_back("Inspecté : " + newArmor.getName() + " — " + armorStateText(newArmor));
        lines.push_back("");
        lines.push_back("Comparaison | Statistique | Actuel | Inspecté | Écart");
        lines.push_back(comparisonRow("PV maximum", 0, newArmor.getMaxHpBonus()));
        lines.push_back(comparisonRow("Réduction", 0, newArmor.getDamageReduction()));
        lines.push_back(comparisonRow("Enchantements", 0, newArmor.getEnchantmentCount()));
        lines.push_back(comparisonRow("Durabilité", 0, durabilityPercent(newArmor.getDurability(), newArmor.getMaxDurability()), "%"));
        lines.push_back("");
        lines.push_back("Effets inspectés : " + newArmor.getEnchantmentSummaryText() + ".");
        lines.push_back("");
        lines.push_back(newArmor.isBroken() ? "Verdict : inutilisable avant réparation." : "Verdict : ajoute toutes les protections affichées.");
    }

    if (newArmor.isBroken())
    {
        lines.push_back("Attention : cette armure est cassée. Ses protections ne s'appliqueront pas avant réparation.");
    }

    MessageScreen::show("COMPARAISON D'ARMURE", "equipment.armor.compare.result", lines);
}

