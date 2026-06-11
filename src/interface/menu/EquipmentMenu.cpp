// EN: EquipmentMenu.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: EquipmentMenu.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "interface/menu/EquipmentMenu.hpp"

#include "core/Console.hpp"
#include "economy/Money.hpp"

#include "interface/menu/equipment/EquipmentDisplay.hpp"
#include "interface/menu/equipment/EquipmentComparison.hpp"
#include "interface/menu/common/MessageScreen.hpp"
#include "interface/menu/common/PagedMenu.hpp"
#include "interface/TerminalInterface.hpp"
#include "combat/system/CombatClassSystem.hpp"
#include "item/durability/DurabilityRules.hpp"
#include "item/equipment/EquipmentWeightRules.hpp"

#include <iostream>
#include <string>
#include <vector>


namespace
{
    std::string equipmentWeaponDurabilityText(const Weapon& weapon)
    {
        if (weapon.isIndestructible())
        {
            return "indestructible";
        }

        return std::to_string(weapon.getDurability()) + "/" + std::to_string(weapon.getMaxDurability());
    }

    std::string equipmentArmorDurabilityText(const Armor& armor)
    {
        if (armor.isIndestructible())
        {
            return "indestructible";
        }

        return std::to_string(armor.getDurability()) + "/" + std::to_string(armor.getMaxDurability());
    }

    void appendEquipmentWarnings(std::vector<std::string>& lines, const std::vector<std::string>& warnings)
    {
        for (const std::string& warning : warnings)
        {
            if (!warning.empty())
            {
                lines.push_back(warning);
            }
        }
    }

    void showSimpleEquipmentScreen(const Player& player)
    {
        std::vector<std::string> lines;

        if (player.hasEquippedWeapon())
        {
            Weapon weapon = player.getEquippedWeapon();
            lines.push_back("- Arme équipée : " + weapon.getName()
                + " | Durabilité : " + equipmentWeaponDurabilityText(weapon)
                + " | État : " + (weapon.isBroken() ? "Cassée" : "Utilisable")
                + " | Dégâts : +" + std::to_string(weapon.getMinDamageBonus()) + " à +" + std::to_string(weapon.getMaxDamageBonus()));
            lines.push_back("Poids : " + EquipmentWeightRules::getWeightLabel(EquipmentWeightRules::getWeaponWeightClass(weapon))
                + " — " + EquipmentWeightRules::getWeaponTradeoffText(weapon));

            if (!weapon.isBroken() && !player.hasBossEquipmentSeal())
            {
                std::string affinityLabel = CombatClassSystem::getWeaponAffinityLabel(player, weapon.getType(), weapon.getName());
                lines.push_back(affinityLabel.empty()
                    ? "Affinité arme/classe : aucune maîtrise particulière avec cette arme."
                    : "Affinité arme/classe : active, " + affinityLabel + ".");
            }
            else if (weapon.isBroken())
            {
                lines.push_back("Affinité arme/classe : inactive, l'arme est cassée.");
            }
            else
            {
                lines.push_back("Affinité arme/classe : bloquée par le sceau de boss.");
            }
            appendEquipmentWarnings(lines, DurabilityRules::describeWeaponUseWarnings(weapon, player.getRace()));
            appendEquipmentWarnings(lines, DurabilityRules::describeWeaponMaintenanceAdvice(weapon, player.getRace()));
        }
        else
        {
            lines.push_back("- Arme équipée : Aucune");
        }

        if (player.hasEquippedArmor())
        {
            Armor armor = player.getEquippedArmor();
            lines.push_back("- Armure équipée : " + armor.getName()
                + " | Durabilité : " + equipmentArmorDurabilityText(armor)
                + " | État : " + (armor.isBroken() ? "Cassée" : "Utilisable")
                + " | PV max : +" + std::to_string(armor.getMaxHpBonus())
                + " | Réduction : " + std::to_string(armor.getDamageReduction()));
            lines.push_back("Poids : " + EquipmentWeightRules::getWeightLabel(EquipmentWeightRules::getArmorWeightClass(armor))
                + " — " + EquipmentWeightRules::getArmorTradeoffText(armor));
            appendEquipmentWarnings(lines, DurabilityRules::describeArmorFitWarnings(armor, player.getRace()));
            appendEquipmentWarnings(lines, DurabilityRules::describeArmorMaintenanceAdvice(armor, player.getRace()));
        }
        else
        {
            lines.push_back("- Armure équipée : Aucune");
        }

        lines.push_back("Argent séparé : " + player.getInventory().getWalletLine());
        lines.push_back("Argent total : " + player.getInventory().getWalletTotalLine());
        MessageScreen::show("ÉQUIPEMENT SIMPLE", "equipment.simple.summary", lines);
    }

    void showDetailedEquipmentScreen(const Player& player)
    {
        std::vector<std::string> lines;

        if (player.hasEquippedWeapon())
        {
            Weapon weapon = player.getEquippedWeapon();
            lines.push_back("- Arme équipée : " + weapon.getName()
                + " | Durabilité : " + equipmentWeaponDurabilityText(weapon)
                + " | État : " + (weapon.isBroken() ? "Cassée" : player.hasBossEquipmentSeal() ? "Sceau de boss" : "Utilisable"));
            lines.push_back("Description : " + weapon.getDescription());
            lines.push_back("Bonus dégâts : +" + std::to_string(weapon.getMinDamageBonus()) + " à +" + std::to_string(weapon.getMaxDamageBonus()));
            lines.push_back("Bonus critique : +" + std::to_string(weapon.getCriticalBonus()));
            lines.push_back("Poids : " + EquipmentWeightRules::getWeightLabel(EquipmentWeightRules::getWeaponWeightClass(weapon)));
            lines.push_back("Contrepartie : " + EquipmentWeightRules::getWeaponTradeoffText(weapon));

            if (weapon.isBroken())
            {
                lines.push_back("État : cassée, ses bonus ne s'appliquent plus.");
                lines.push_back("Affinité arme/classe : inactive, l'arme est cassée.");
            }
            else if (player.hasBossEquipmentSeal())
            {
                lines.push_back("État : utilisable, mais le sceau de boss bloque ses bonus.");
                lines.push_back("Affinité arme/classe : bloquée par le sceau de boss.");
            }
            else
            {
                lines.push_back("État : Utilisable");
                std::string affinityLabel = CombatClassSystem::getWeaponAffinityLabel(player, weapon.getType(), weapon.getName());
                lines.push_back(affinityLabel.empty()
                    ? "Affinité arme/classe : aucune maîtrise particulière avec cette arme."
                    : "Affinité arme/classe : active, " + affinityLabel + ".");
            }
            appendEquipmentWarnings(lines, DurabilityRules::describeWeaponUseWarnings(weapon, player.getRace()));
            appendEquipmentWarnings(lines, DurabilityRules::describeWeaponMaintenanceAdvice(weapon, player.getRace()));
        }
        else
        {
            lines.push_back("- Arme équipée : Aucune");
        }

        lines.push_back("");
        if (player.hasEquippedArmor())
        {
            Armor armor = player.getEquippedArmor();
            lines.push_back("- Armure équipée : " + armor.getName()
                + " | Durabilité : " + equipmentArmorDurabilityText(armor)
                + " | État : " + (armor.isBroken() ? "Cassée" : "Utilisable"));
            lines.push_back("Description : " + armor.getDescription());
            lines.push_back("Bonus PV max : +" + std::to_string(armor.getMaxHpBonus()));
            lines.push_back("Réduction dégâts : " + std::to_string(armor.getDamageReduction()));
            lines.push_back("Poids : " + EquipmentWeightRules::getWeightLabel(EquipmentWeightRules::getArmorWeightClass(armor)));
            lines.push_back("Contrepartie : " + EquipmentWeightRules::getArmorTradeoffText(armor));
            lines.push_back(armor.isBroken() ? "État : cassée, ses bonus ne s'appliquent plus." : "État : Utilisable");
            appendEquipmentWarnings(lines, DurabilityRules::describeArmorFitWarnings(armor, player.getRace()));
            appendEquipmentWarnings(lines, DurabilityRules::describeArmorMaintenanceAdvice(armor, player.getRace()));
        }
        else
        {
            lines.push_back("- Armure équipée : Aucune");
        }

        lines.push_back("");
        lines.push_back("Argent séparé : " + player.getInventory().getWalletLine());
        lines.push_back("Argent total : " + player.getInventory().getWalletTotalLine());
        MessageScreen::show("ÉQUIPEMENT DÉTAILLÉ", "equipment.details.summary", lines);
    }

    void showEquipmentWeaponInspection(const Player& player, const Weapon& weapon)
    {
        std::vector<std::string> lines = {
            "Nom : " + weapon.getName(),
            "Description : " + weapon.getDescription(),
            "Bonus dégâts : +" + std::to_string(weapon.getMinDamageBonus()) + " à +" + std::to_string(weapon.getMaxDamageBonus()),
            "Bonus critique : +" + std::to_string(weapon.getCriticalBonus()),
            "Poids : " + EquipmentWeightRules::getWeightLabel(EquipmentWeightRules::getWeaponWeightClass(weapon)),
            "Contrepartie : " + EquipmentWeightRules::getWeaponTradeoffText(weapon),
            "Durabilité : " + equipmentWeaponDurabilityText(weapon),
            weapon.isBroken() ? "État : Cassée" : "État : Utilisable",
            "Valeur estimée : " + Money::formatGoldWithRaw(weapon.getValue())
        };
        appendEquipmentWarnings(lines, DurabilityRules::describeWeaponUseWarnings(weapon, player.getRace()));
        appendEquipmentWarnings(lines, DurabilityRules::describeWeaponMaintenanceAdvice(weapon, player.getRace()));
        MessageScreen::show("INSPECTION - ARME", "equipment.weapon.inspect.details", lines);
    }

    void showEquipmentArmorInspection(const Player& player, const Armor& armor)
    {
        std::vector<std::string> lines = {
            "Nom : " + armor.getName(),
            "Description : " + armor.getDescription(),
            "Bonus PV max : +" + std::to_string(armor.getMaxHpBonus()),
            "Réduction dégâts : " + std::to_string(armor.getDamageReduction()),
            "Poids : " + EquipmentWeightRules::getWeightLabel(EquipmentWeightRules::getArmorWeightClass(armor)),
            "Contrepartie : " + EquipmentWeightRules::getArmorTradeoffText(armor),
            "Durabilité : " + equipmentArmorDurabilityText(armor),
            armor.isBroken() ? "État : Cassée" : "État : Utilisable",
            "Valeur estimée : " + Money::formatGoldWithRaw(armor.getValue())
        };
        appendEquipmentWarnings(lines, DurabilityRules::describeArmorFitWarnings(armor, player.getRace()));
        appendEquipmentWarnings(lines, DurabilityRules::describeArmorMaintenanceAdvice(armor, player.getRace()));
        MessageScreen::show("INSPECTION - ARMURE", "equipment.armor.inspect.details", lines);
    }

    void showEquipmentWeaponEquipResult(const Player& player, const Weapon& weapon)
    {
        MessageScreen::show(
            "ARME ÉQUIPÉE",
            "equipment.weapon.equip.result",
            {
                player.getName() + " équipe : " + weapon.getName() + ".",
                "Durabilité : " + equipmentWeaponDurabilityText(weapon),
                "Poids : " + EquipmentWeightRules::getWeightLabel(EquipmentWeightRules::getWeaponWeightClass(weapon)) + " — " + EquipmentWeightRules::getWeaponTradeoffText(weapon),
                weapon.isBroken() ? "Attention : cette arme est cassée, elle ne donnera aucun bonus." : "La prise en main est bonne. Cette arme est prête au combat.",
                DurabilityRules::weaponWearStateText(weapon)
            }
        );
    }

    void showEquipmentArmorEquipResult(const Player& player, const Armor& armor)
    {
        MessageScreen::show(
            "ARMURE ÉQUIPÉE",
            "equipment.armor.equip.result",
            {
                player.getName() + " équipe : " + armor.getName() + ".",
                "Durabilité : " + equipmentArmorDurabilityText(armor),
                "Poids : " + EquipmentWeightRules::getWeightLabel(EquipmentWeightRules::getArmorWeightClass(armor)) + " — " + EquipmentWeightRules::getArmorTradeoffText(armor),
                armor.isBroken() ? "Attention : cette armure est cassée, elle ne donnera aucun bonus." : "Ses protections sont maintenant actives.",
                DurabilityRules::armorWearStateText(armor),
                player.getName() + " possède maintenant " + std::to_string(player.getHp()) + "/" + std::to_string(player.getMaxHp()) + " PV."
            }
        );
    }

    void showEquipmentNotice(const std::string& title, const std::string& screenId, const std::vector<std::string>& lines)
    {
        MessageScreen::show(title, screenId, lines);
    }
}

// EN: open declares or implements a focused behavior used by this module.
// FR: open déclare ou implémente un comportement précis utilisé par ce module.
bool EquipmentMenu::open(Player& player)
{
    while (true)
    {
        MenuScreen screen = EquipmentDisplay::buildMainScreen();
        int choice = TerminalInterface::askMenuChoiceFromOptions(
            screen,
            "Choix invalide. Choisis une option visible de l'équipement."
        );

        Console::clear();

        if (choice == 0)
        {
            return false;
        }

        if (choice == 1)
        {
            showSimpleEquipmentScreen(player);
            continue;
        }

        if (choice == 2)
        {
            showDetailedEquipmentScreen(player);
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
        showEquipmentNotice("CHANGER ARME", "equipment.weapon.empty", {player.getName() + " n'a aucune arme à équiper.", "Les armes récupérées apparaîtront ici."});
        return false;
    }

    constexpr std::size_t itemsPerPage = 10;
    std::size_t pageIndex = 0;

    while (true)
    {
        const std::size_t totalItems = static_cast<std::size_t>(player.getInventory().getWeaponCount());
        const std::size_t totalPages = PagedMenu::pageCount(totalItems, itemsPerPage);
        if (pageIndex >= totalPages)
        {
            pageIndex = totalPages - 1;
        }

        const std::size_t first = PagedMenu::firstIndex(pageIndex, itemsPerPage);
        const std::size_t last = PagedMenu::lastIndexExclusive(totalItems, pageIndex, itemsPerPage);

        MenuScreen weaponListScreen = EquipmentDisplay::buildWeaponListScreen(player, pageIndex, itemsPerPage);
        int choice = TerminalInterface::askMenuChoiceFromOptions(
            weaponListScreen,
            "Choix invalide. Choisis une arme affichée, une page ou 0 pour revenir."
        );

        Console::clear();

        if (choice == 0)
        {
            return false;
        }

        if (choice == 98 && pageIndex > 0)
        {
            --pageIndex;
            continue;
        }

        if (choice == 99 && pageIndex + 1 < totalPages)
        {
            ++pageIndex;
            continue;
        }

        const int visibleCount = static_cast<int>(last - first);
        if (choice < 1 || choice > visibleCount)
        {
            showEquipmentNotice("ARME INTROUVABLE", "equipment.weapon.invalid_choice", {"Ce numéro ne correspond à aucune arme affichée.", "Change de page ou choisis une entrée visible."});
            continue;
        }

        const int weaponIndex = static_cast<int>(first) + choice - 1;

        if (!player.getInventory().hasWeapon(weaponIndex))
        {
            showEquipmentNotice("ARME INTROUVABLE", "equipment.weapon.missing", {"Cette arme n'existe pas dans ton inventaire.", "Le registre se mettra à jour au prochain affichage."});
            continue;
        }

        Weapon newWeapon = player.getInventory().getWeapon(weaponIndex);

        MenuScreen selectedWeaponScreen = EquipmentDisplay::buildSelectedWeaponScreen(newWeapon);
        int action = TerminalInterface::askMenuChoiceFromOptions(
            selectedWeaponScreen,
            "Choix invalide. Choisis une action visible pour cette arme."
        );

        Console::clear();

        if (action == 0)
        {
            continue;
        }

        if (action == 1)
        {
            showEquipmentWeaponInspection(player, newWeapon);
            continue;
        }

        if (action == 2)
        {
            EquipmentComparison::displayWeaponComparison(player, newWeapon);
            continue;
        }

        if (!player.equipWeapon(weaponIndex))
        {
            showEquipmentNotice("ARME NON ÉQUIPÉE", "equipment.weapon.equip.failed", {"Impossible d'équiper cette arme.", "Elle a peut-être été déplacée, retirée ou rendue indisponible."});
            continue;
        }

        showEquipmentWeaponEquipResult(player, player.getEquippedWeapon());
        return false;
    }
}

// EN: equipArmorFromInventory declares or implements a focused behavior used by this module.
// FR: equipArmorFromInventory déclare ou implémente un comportement précis utilisé par ce module.
bool EquipmentMenu::equipArmorFromInventory(Player& player)
{
    if (player.getInventory().getArmorCount() <= 0)
    {
        showEquipmentNotice("CHANGER TENUE", "equipment.armor.empty", {player.getName() + " n'a aucune armure à équiper.", "Les protections récupérées apparaîtront ici."});
        return false;
    }

    constexpr std::size_t itemsPerPage = 10;
    std::size_t pageIndex = 0;

    while (true)
    {
        const std::size_t totalItems = static_cast<std::size_t>(player.getInventory().getArmorCount());
        const std::size_t totalPages = PagedMenu::pageCount(totalItems, itemsPerPage);
        if (pageIndex >= totalPages)
        {
            pageIndex = totalPages - 1;
        }

        const std::size_t first = PagedMenu::firstIndex(pageIndex, itemsPerPage);
        const std::size_t last = PagedMenu::lastIndexExclusive(totalItems, pageIndex, itemsPerPage);

        MenuScreen armorListScreen = EquipmentDisplay::buildArmorListScreen(player, pageIndex, itemsPerPage);
        int choice = TerminalInterface::askMenuChoiceFromOptions(
            armorListScreen,
            "Choix invalide. Choisis une armure affichée, une page ou 0 pour revenir."
        );

        Console::clear();

        if (choice == 0)
        {
            return false;
        }

        if (choice == 98 && pageIndex > 0)
        {
            --pageIndex;
            continue;
        }

        if (choice == 99 && pageIndex + 1 < totalPages)
        {
            ++pageIndex;
            continue;
        }

        const int visibleCount = static_cast<int>(last - first);
        if (choice < 1 || choice > visibleCount)
        {
            showEquipmentNotice("ARMURE INTROUVABLE", "equipment.armor.invalid_choice", {"Ce numéro ne correspond à aucune armure affichée.", "Change de page ou choisis une entrée visible."});
            continue;
        }

        const int armorIndex = static_cast<int>(first) + choice - 1;

        if (!player.getInventory().hasArmor(armorIndex))
        {
            showEquipmentNotice("ARMURE INTROUVABLE", "equipment.armor.missing", {"Cette armure n'existe pas dans ton inventaire.", "Le registre se mettra à jour au prochain affichage."});
            continue;
        }

        Armor newArmor = player.getInventory().getArmor(armorIndex);

        MenuScreen selectedArmorScreen = EquipmentDisplay::buildSelectedArmorScreen(newArmor);
        int action = TerminalInterface::askMenuChoiceFromOptions(
            selectedArmorScreen,
            "Choix invalide. Choisis une action visible pour cette armure."
        );

        Console::clear();

        if (action == 0)
        {
            continue;
        }

        if (action == 1)
        {
            showEquipmentArmorInspection(player, newArmor);
            continue;
        }

        if (action == 2)
        {
            EquipmentComparison::displayArmorComparison(player, newArmor);
            continue;
        }

        if (!player.equipArmor(armorIndex))
        {
            showEquipmentNotice("ARMURE NON ÉQUIPÉE", "equipment.armor.equip.failed", {"Impossible d'équiper cette armure.", "Elle a peut-être été déplacée, retirée ou rendue indisponible."});
            continue;
        }

        showEquipmentArmorEquipResult(player, player.getEquippedArmor());
        return false;
    }
}
