// EN: EquipmentDisplay.cpp centralizes equipment screens for terminal and future GUI rendering.
// FR: EquipmentDisplay.cpp centralise les écrans d'équipement pour le terminal et la future IG.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "interface/menu/equipment/EquipmentDisplay.hpp"

#include "interface/TerminalInterface.hpp"
#include "interface/menu/common/MessageScreen.hpp"
#include "interface/menu/common/PagedMenu.hpp"

#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>

namespace
{
    MenuOptionItemData makeWeaponItemData(const Weapon& weapon, const std::string& actionType)
    {
        MenuOptionItemData itemData;
        itemData.structured = true;
        itemData.kind = "weapon";
        itemData.section = "Équipement - armes";
        itemData.actionType = actionType;
        itemData.name = weapon.getName();
        itemData.detail = "Dégâts : +" + std::to_string(weapon.getMinDamageBonus())
            + " à +" + std::to_string(weapon.getMaxDamageBonus())
            + " | Critique : +" + std::to_string(weapon.getCriticalBonus());
        itemData.status = weapon.isBroken() ? "Cassée" : "Utilisable";
        itemData.progress = "Durabilité : " + EquipmentDisplay::weaponDurabilityText(weapon);
        itemData.price = std::to_string(weapon.getValue()) + " or";
        itemData.important = weapon.isBroken();
        return itemData;
    }

    MenuOptionItemData makeArmorItemData(const Armor& armor, const std::string& actionType)
    {
        MenuOptionItemData itemData;
        itemData.structured = true;
        itemData.kind = "armor";
        itemData.section = "Équipement - armures";
        itemData.actionType = actionType;
        itemData.name = armor.getName();
        itemData.detail = "PV max : +" + std::to_string(armor.getMaxHpBonus())
            + " | Réduction : " + std::to_string(armor.getDamageReduction());
        itemData.status = armor.isBroken() ? "Cassée" : "Utilisable";
        itemData.progress = "Durabilité : " + EquipmentDisplay::armorDurabilityText(armor);
        itemData.price = std::to_string(armor.getValue()) + " or";
        itemData.important = armor.isBroken();
        return itemData;
    }
}

MenuScreen EquipmentDisplay::buildMainScreen()
{
    MenuScreen screen("ÉQUIPEMENT", "equipment.main");
    screen.addBackOption("Retour", "equipment.back");
    screen.addOption(1, "Voir équipement simple", "Affichage court de l'arme et de la tenue actuelles.", true, "equipment.simple");
    screen.addOption(2, "Voir équipement détaillé", "Statistiques, bonus et durabilité complète.", true, "equipment.details");
    screen.addOption(3, "Changer arme rapide", "Choisir une arme dans l'inventaire.", true, "equipment.weapon.change");
    screen.addOption(4, "Changer tenue rapide", "Choisir une armure ou tenue dans l'inventaire.", true, "equipment.armor.change");
    return screen;
}

MenuScreen EquipmentDisplay::buildWeaponListScreen(const Player& player)
{
    const std::size_t totalItems = static_cast<std::size_t>(std::max(0, player.getInventory().getWeaponCount()));
    const std::size_t itemsPerPage = totalItems == 0 ? 1 : totalItems;
    return buildWeaponListScreen(player, 0, itemsPerPage);
}

MenuScreen EquipmentDisplay::buildWeaponListScreen(const Player& player, std::size_t pageIndex, std::size_t itemsPerPage)
{
    MenuScreen screen("CHANGER ARME", "equipment.weapon.list");
    const std::size_t totalItems = static_cast<std::size_t>(std::max(0, player.getInventory().getWeaponCount()));
    const std::size_t totalPages = PagedMenu::pageCount(totalItems, itemsPerPage);

    if (pageIndex >= totalPages)
    {
        pageIndex = totalPages - 1;
    }

    const std::size_t first = PagedMenu::firstIndex(pageIndex, itemsPerPage);
    const std::size_t last = PagedMenu::lastIndexExclusive(totalItems, pageIndex, itemsPerPage);

    screen.addSubtitle(PagedMenu::pageInfoText(pageIndex, totalPages, totalItems));
    screen.addLine("Affichage : " + PagedMenu::rangeText(first, last, totalItems));

    for (std::size_t i = first; i < last; ++i)
    {
        Weapon weapon = player.getInventory().getWeapon(static_cast<int>(i));
        screen.addOption(
            static_cast<int>(i - first + 1),
            weapon.getName(),
            "Choisir cette arme puis inspecter, comparer ou équiper.",
            true,
            "equipment.weapon.select." + std::to_string(i),
            makeWeaponItemData(weapon, "select")
        );
    }

    PagedMenu::addNavigationOptions(screen, pageIndex, totalPages);
    screen.addFooterLine("Choisis une arme affichée. Le numéro visible correspond à l'entrée de cette page.");
    return screen;
}

MenuScreen EquipmentDisplay::buildArmorListScreen(const Player& player)
{
    const std::size_t totalItems = static_cast<std::size_t>(std::max(0, player.getInventory().getArmorCount()));
    const std::size_t itemsPerPage = totalItems == 0 ? 1 : totalItems;
    return buildArmorListScreen(player, 0, itemsPerPage);
}

MenuScreen EquipmentDisplay::buildArmorListScreen(const Player& player, std::size_t pageIndex, std::size_t itemsPerPage)
{
    MenuScreen screen("CHANGER TENUE", "equipment.armor.list");
    const std::size_t totalItems = static_cast<std::size_t>(std::max(0, player.getInventory().getArmorCount()));
    const std::size_t totalPages = PagedMenu::pageCount(totalItems, itemsPerPage);

    if (pageIndex >= totalPages)
    {
        pageIndex = totalPages - 1;
    }

    const std::size_t first = PagedMenu::firstIndex(pageIndex, itemsPerPage);
    const std::size_t last = PagedMenu::lastIndexExclusive(totalItems, pageIndex, itemsPerPage);

    screen.addSubtitle(PagedMenu::pageInfoText(pageIndex, totalPages, totalItems));
    screen.addLine("Affichage : " + PagedMenu::rangeText(first, last, totalItems));

    for (std::size_t i = first; i < last; ++i)
    {
        Armor armor = player.getInventory().getArmor(static_cast<int>(i));
        screen.addOption(
            static_cast<int>(i - first + 1),
            armor.getName(),
            "Choisir cette protection puis inspecter, comparer ou équiper.",
            true,
            "equipment.armor.select." + std::to_string(i),
            makeArmorItemData(armor, "select")
        );
    }

    PagedMenu::addNavigationOptions(screen, pageIndex, totalPages);
    screen.addFooterLine("Choisis une armure affichée. Le numéro visible correspond à l'entrée de cette page.");
    return screen;
}

MenuScreen EquipmentDisplay::buildSelectedWeaponScreen(const Weapon& weapon)
{
    MenuScreen screen("ARME SÉLECTIONNÉE", "equipment.weapon.selected");
    screen.addLine("Arme : " + weapon.getName());
    screen.addLine("Durabilité : " + weaponDurabilityText(weapon));
    screen.addBackOption("Retour", "equipment.weapon.back");
    screen.addOption(1, "Inspecter", "Voir la description et les détails de l'arme.", true, "equipment.weapon.inspect", makeWeaponItemData(weapon, "inspect"));
    screen.addOption(2, "Comparer", "Comparer avec l'arme actuellement équipée.", true, "equipment.weapon.compare", makeWeaponItemData(weapon, "compare"));
    screen.addOption(3, "Équiper", "Remplacer l'arme actuelle par cette arme.", true, "equipment.weapon.equip", makeWeaponItemData(weapon, "equip"));
    return screen;
}

MenuScreen EquipmentDisplay::buildSelectedArmorScreen(const Armor& armor)
{
    MenuScreen screen("ARMURE SÉLECTIONNÉE", "equipment.armor.selected");
    screen.addLine("Armure : " + armor.getName());
    screen.addLine("Durabilité : " + armorDurabilityText(armor));
    screen.addBackOption("Retour", "equipment.armor.back");
    screen.addOption(1, "Inspecter", "Voir la description et les détails de l'armure.", true, "equipment.armor.inspect", makeArmorItemData(armor, "inspect"));
    screen.addOption(2, "Comparer", "Comparer avec l'armure actuellement équipée.", true, "equipment.armor.compare", makeArmorItemData(armor, "compare"));
    screen.addOption(3, "Équiper", "Remplacer l'armure actuelle par cette armure.", true, "equipment.armor.equip", makeArmorItemData(armor, "equip"));
    return screen;
}

void EquipmentDisplay::displayMainMenu()
{
    TerminalInterface::renderMenuScreen(buildMainScreen());
}

void EquipmentDisplay::displayWeaponList(const Player& player)
{
    TerminalInterface::renderMenuScreen(buildWeaponListScreen(player));
}

void EquipmentDisplay::displayArmorList(const Player& player)
{
    TerminalInterface::renderMenuScreen(buildArmorListScreen(player));
}

void EquipmentDisplay::displaySelectedWeapon(const Weapon& weapon)
{
    TerminalInterface::renderMenuScreen(buildSelectedWeaponScreen(weapon));
}

void EquipmentDisplay::displaySelectedArmor(const Armor& armor)
{
    TerminalInterface::renderMenuScreen(buildSelectedArmorScreen(armor));
}

std::string EquipmentDisplay::weaponSummaryText(const Weapon& weapon, int index)
{
    std::ostringstream output;
    output << index << " : " << weapon.getName()
           << " | Dégâts : +" << weapon.getMinDamageBonus()
           << " à +" << weapon.getMaxDamageBonus()
           << " | Critique : +" << weapon.getCriticalBonus()
           << " | Durabilité : " << weaponDurabilityText(weapon);

    if (weapon.isBroken())
    {
        output << " | Cassée";
    }

    return output.str();
}

std::string EquipmentDisplay::armorSummaryText(const Armor& armor, int index)
{
    std::ostringstream output;
    output << index << " : " << armor.getName()
           << " | PV max : +" << armor.getMaxHpBonus()
           << " | Réduction : " << armor.getDamageReduction()
           << " | Durabilité : " << armorDurabilityText(armor);

    if (armor.isBroken())
    {
        output << " | Cassée";
    }

    return output.str();
}

void EquipmentDisplay::displayWeaponSummary(const Weapon& weapon, int index)
{
    MessageScreen::show(
        "ARME",
        "equipment.weapon.summary",
        {weaponSummaryText(weapon, index)},
        false
    );
}

void EquipmentDisplay::displayArmorSummary(const Armor& armor, int index)
{
    MessageScreen::show(
        "ARMURE",
        "equipment.armor.summary",
        {armorSummaryText(armor, index)},
        false
    );
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
