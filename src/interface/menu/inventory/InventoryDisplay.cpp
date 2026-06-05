// EN: InventoryDisplay.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: InventoryDisplay.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "interface/menu/inventory/InventoryDisplay.hpp"

#include "interface/menu/inventory/InventoryUtils.hpp"
#include "core/Console.hpp"
#include "interface/TerminalInterface.hpp"

#include "item/Inventory.hpp"
#include "item/consumable/Consumable.hpp"

#include <algorithm>
#include <iostream>
#include <vector>

namespace
{
    MenuOptionItemData makeInventoryRouteItemData(
        const std::string& kind,
        const std::string& actionType,
        const std::string& name,
        const std::string& detail,
        const std::string& status = "",
        bool important = false
    )
    {
        MenuOptionItemData itemData;
        itemData.structured = true;
        itemData.kind = kind;
        itemData.section = "Inventaire - menu principal";
        itemData.actionType = actionType;
        itemData.name = name;
        itemData.detail = detail;
        itemData.status = status;
        itemData.owner = "Sac du personnage";
        itemData.important = important;
        return itemData;
    }

    MenuOptionItemData makeInventoryWeaponItemData(const Weapon& weapon, const std::string& actionType)
    {
        MenuOptionItemData itemData;
        itemData.structured = true;
        itemData.kind = "weapon";
        itemData.section = "Inventaire - arme sélectionnée";
        itemData.actionType = actionType;
        itemData.name = weapon.getName();
        itemData.detail = "Dégâts : +" + std::to_string(weapon.getMinDamageBonus())
            + " à +" + std::to_string(weapon.getMaxDamageBonus())
            + " | Critique : +" + std::to_string(weapon.getCriticalBonus());
        itemData.status = weapon.isBroken() ? "Cassée" : "Utilisable";
        itemData.progress = "Durabilité : " + InventoryUtils::weaponDurabilityText(weapon);
        itemData.price = std::to_string(weapon.getValue()) + " or";
        itemData.important = weapon.isBroken();
        return itemData;
    }

    MenuOptionItemData makeInventoryArmorItemData(const Armor& armor, const std::string& actionType)
    {
        MenuOptionItemData itemData;
        itemData.structured = true;
        itemData.kind = "armor";
        itemData.section = "Inventaire - armure sélectionnée";
        itemData.actionType = actionType;
        itemData.name = armor.getName();
        itemData.detail = "PV max : +" + std::to_string(armor.getMaxHpBonus())
            + " | Réduction : " + std::to_string(armor.getDamageReduction());
        itemData.status = armor.isBroken() ? "Cassée" : "Utilisable";
        itemData.progress = "Durabilité : " + InventoryUtils::armorDurabilityText(armor);
        itemData.price = std::to_string(armor.getValue()) + " or";
        itemData.important = armor.isBroken();
        return itemData;
    }

    MenuOptionItemData makeInventoryConsumableItemData(const Consumable& consumable, const std::string& actionType, const std::string& status = "", int amount = 1)
    {
        MenuOptionItemData itemData;
        itemData.structured = true;
        itemData.kind = "consumable";
        itemData.section = "Inventaire - consommable sélectionné";
        itemData.actionType = actionType;
        itemData.name = consumable.getName();
        itemData.quantity = std::to_string(std::max(1, amount));
        itemData.detail = InventoryUtils::consumableTypeToText(consumable.getType());
        itemData.status = status;
        itemData.progress = "Puissance : " + consumable.getPowerDisplayText();
        itemData.price = std::to_string(consumable.getValue()) + " or";
        itemData.important = consumable.isHealing();
        return itemData;
    }

    MenuOptionItemData makeInventoryMaterialItemData(const Material& material, const std::string& actionType)
    {
        MenuOptionItemData itemData;
        itemData.structured = true;
        itemData.kind = "material";
        itemData.section = "Inventaire - entrée sélectionnée";
        itemData.actionType = actionType;
        itemData.name = material.getName();
        itemData.quantity = std::to_string(material.getQuantity());
        itemData.detail = material.getCategory();
        itemData.status = material.hasSpecialQuality() ? material.getQualityLabel() : "Qualité normale";
        itemData.price = std::to_string(material.getValue()) + " or/unité";
        itemData.important = material.hasSpecialQuality();
        return itemData;
    }
}

// EN: displayMainMenu declares or implements a focused behavior used by this module.
// FR: displayMainMenu déclare ou implémente un comportement précis utilisé par ce module.
MenuScreen InventoryDisplay::buildMainScreen()
{
    MenuScreen screen("INVENTAIRE", "inventory.main");
    screen.addOption(
        1,
        "Bestiaire (objet spécial)",
        "Consulter les créatures, boss, matériaux et informations connues.",
        true,
        "inventory.bestiary",
        makeInventoryRouteItemData("bestiary", "inspect", "Bestiaire", "Créatures, boss, matériaux, légendes et connaissances progressives.", "Objet spécial", true)
    );
    screen.addOption(
        2,
        "Voir tout (affichage simple)",
        "Résumé court de l'or, des objets et des ressources.",
        true,
        "inventory.full_simple",
        makeInventoryRouteItemData("inventory_summary", "inspect", "Voir tout", "Résumé court : or, armes, armures, consommables et matériaux sans descriptions longues.", "Affichage simple")
    );
    screen.addOption(
        3,
        "Armes",
        "Voir, inspecter, équiper ou réparer les armes.",
        true,
        "inventory.weapons",
        makeInventoryRouteItemData("weapon", "open", "Armes", "Liste des armes avec inspection, équipement, réparation et lien bestiaire.", "Équipement offensif")
    );
    screen.addOption(
        4,
        "Armures",
        "Voir, inspecter, équiper ou réparer les protections.",
        true,
        "inventory.armors",
        makeInventoryRouteItemData("armor", "open", "Armures", "Liste des protections avec inspection, équipement, réparation et lien bestiaire.", "Équipement défensif")
    );
    screen.addOption(
        5,
        "Consommables",
        "Potions et objets utilisables.",
        true,
        "inventory.consumables",
        makeInventoryRouteItemData("consumable", "use", "Consommables", "Potions et objets utilisables, avec actions qui peuvent parfois consommer le tour.", "Utilisable")
    );
    screen.addOption(
        6,
        "Matériaux / plantes / infos",
        "Ressources, notes, fragments et composants.",
        true,
        "inventory.materials",
        makeInventoryRouteItemData("material", "inspect", "Matériaux / plantes / infos", "Ressources, composants, notes, fragments et connaissances liées aux objets.", "Ressources")
    );
    screen.addOption(
        7,
        "Craft / schémas de fabrication",
        "Fabriquer ou améliorer avec les recettes connues.",
        true,
        "inventory.craft",
        makeInventoryRouteItemData("craft", "create", "Craft / schémas", "Fabriquer, améliorer ou expérimenter avec les recettes connues.", "Fabrication")
    );
    screen.addOption(
        8,
        "Consulter mes quêtes",
        "Ouvrir le journal de quêtes.",
        true,
        "inventory.quests",
        makeInventoryRouteItemData("quest", "quest", "Journal de quêtes", "Voir les quêtes suivies, terminées ou à rendre.", "Progression")
    );
    screen.addBackOption("Retour", "inventory.back");
    return screen;
}

void InventoryDisplay::displayMainMenu()
{
    TerminalInterface::renderMenuScreen(buildMainScreen());
}

// EN: displaySimpleFullInventory declares or implements a focused behavior used by this module.
// FR: displaySimpleFullInventory déclare ou implémente un comportement précis utilisé par ce module.
MenuScreen InventoryDisplay::buildSimpleFullInventoryScreen(const Player& player)
{
    const Inventory& inventory = player.getInventory();

    MenuScreen screen("INVENTAIRE - RÉSUMÉ SIMPLE", "inventory.full_simple.summary");
    screen.setContinueInput("Valide pour revenir à l'inventaire.");
    screen.addSubtitle("Affichage volontairement court : noms, quantités et états importants.");
    screen.addLine("Or : " + std::to_string(inventory.getGold()) + " pièces");
    screen.addLine("Objet spécial : Bestiaire");
    screen.addLine("Armes : " + std::to_string(inventory.getWeaponCount()));

    for (int i = 0; i < inventory.getWeaponCount(); ++i)
    {
        Weapon weapon = inventory.getWeapon(i);
        std::string line = "[" + std::to_string(i) + "] " + weapon.getName()
            + " | Durabilité : " + InventoryUtils::weaponDurabilityText(weapon);

        if (weapon.isBroken())
        {
            line += " | Cassée";
        }

        screen.addLine(line);
    }

    screen.addLine("");
    screen.addLine("Armures : " + std::to_string(inventory.getArmorCount()));

    for (int i = 0; i < inventory.getArmorCount(); ++i)
    {
        Armor armor = inventory.getArmor(i);
        std::string line = "[" + std::to_string(i) + "] " + armor.getName()
            + " | Durabilité : " + InventoryUtils::armorDurabilityText(armor);

        if (armor.isBroken())
        {
            line += " | Cassée";
        }

        screen.addLine(line);
    }

    screen.addLine("");
    screen.addLine("Consommables : " + std::to_string(inventory.getConsumableCount()));

    std::vector<ConsumableGroup> groups = InventoryUtils::groupConsumables(player);

    for (int i = 0; i < static_cast<int>(groups.size()); ++i)
    {
        const ConsumableGroup& group = groups[i];
        screen.addLine(
            "[" + std::to_string(i) + "] " + InventoryUtils::stackLabel(group.name, group.amount)
            + " | " + InventoryUtils::consumableTypeToText(group.type)
            + " | Puissance : " + std::to_string(group.power)
        );
    }

    screen.addLine("");
    screen.addLine("Matériaux / plantes / infos : " + std::to_string(inventory.getMaterialCount()));

    for (int i = 0; i < static_cast<int>(inventory.getMaterials().size()); ++i)
    {
        Material material = inventory.getMaterial(i);
        std::string line = "[" + std::to_string(i) + "] " + material.getName()
            + " x" + std::to_string(material.getQuantity())
            + " | " + material.getCategory();

        if (material.hasSpecialQuality())
        {
            line += " | " + material.getQualityLabel();
        }

        screen.addLine(line);
    }

    screen.addFooterLine("Pour agir sur un objet, reviens au menu et choisis sa catégorie.");
    return screen;
}

// EN: displaySimpleFullInventory declares or implements a focused behavior used by this module.
// FR: displaySimpleFullInventory déclare ou implémente un comportement précis utilisé par ce module.
void InventoryDisplay::displaySimpleFullInventory(const Player& player)
{
    TerminalInterface::renderMenuScreen(buildSimpleFullInventoryScreen(player), false);
    Console::waitForEnter();
    Console::clear();
}

MenuScreen InventoryDisplay::buildSelectedWeaponScreen(const Weapon& weapon)
{
    MenuScreen screen("ARME SÉLECTIONNÉE", "inventory.weapon.selected");
    screen.addLine("Arme : " + weapon.getName());
    screen.addBackOption("Retour", "inventory.weapon.back");
    screen.addOption(1, "Inspecter", "Voir la description et les détails de l'arme.", true, "inventory.weapon.inspect", makeInventoryWeaponItemData(weapon, "inspect"));
    screen.addOption(2, "Équiper", "Équiper cette arme maintenant.", true, "inventory.weapon.equip", makeInventoryWeaponItemData(weapon, "equip"));
    screen.addOption(3, "Voir dans le bestiaire", "Consulter les informations connues liées à cette entrée.", true, "inventory.weapon.bestiary", makeInventoryWeaponItemData(weapon, "inspect"));
    screen.addOption(4, "Réparer", "Utiliser un kit ou des matériaux compatibles.", true, "inventory.weapon.repair", makeInventoryWeaponItemData(weapon, "repair"));
    return screen;
}

void InventoryDisplay::displaySelectedWeapon(const Weapon& weapon)
{
    TerminalInterface::renderMenuScreen(buildSelectedWeaponScreen(weapon));
}

MenuScreen InventoryDisplay::buildSelectedArmorScreen(const Armor& armor)
{
    MenuScreen screen("ARMURE SÉLECTIONNÉE", "inventory.armor.selected");
    screen.addLine("Armure : " + armor.getName());
    screen.addBackOption("Retour", "inventory.armor.back");
    screen.addOption(1, "Inspecter", "Voir la description et les détails de l'armure.", true, "inventory.armor.inspect", makeInventoryArmorItemData(armor, "inspect"));
    screen.addOption(2, "Équiper", "Équiper cette protection maintenant.", true, "inventory.armor.equip", makeInventoryArmorItemData(armor, "equip"));
    screen.addOption(3, "Voir dans le bestiaire", "Consulter les informations connues liées à cette entrée.", true, "inventory.armor.bestiary", makeInventoryArmorItemData(armor, "inspect"));
    screen.addOption(4, "Réparer", "Utiliser un kit ou des matériaux compatibles.", true, "inventory.armor.repair", makeInventoryArmorItemData(armor, "repair"));
    return screen;
}

void InventoryDisplay::displaySelectedArmor(const Armor& armor)
{
    TerminalInterface::renderMenuScreen(buildSelectedArmorScreen(armor));
}

MenuScreen InventoryDisplay::buildSelectedConsumableScreen(const Consumable& consumable, int amount)
{
    amount = std::max(1, amount);

    MenuScreen screen("CONSOMMABLE SÉLECTIONNÉ", "inventory.consumable.selected");
    screen.addLine("Consommable : " + InventoryUtils::stackLabel(consumable.getName(), amount));
    screen.addLine("Quantité dans la pile : " + std::to_string(amount));
    screen.addLine("Type : " + InventoryUtils::consumableTypeToText(consumable.getType()));
    screen.addLine("Puissance : " + consumable.getPowerDisplayText());
    screen.addBackOption("Retour", "inventory.consumable.back");
    screen.addOption(1, "Inspecter", "Lire la description complète.", true, "inventory.consumable.inspect", makeInventoryConsumableItemData(consumable, "inspect", "", amount));

    if (consumable.getType() == ConsumableType::Healing)
    {
        screen.addOption(2, "Utiliser", "Boire une potion de cette pile hors combat.", true, "inventory.consumable.use", makeInventoryConsumableItemData(consumable, "use", "Utilisable hors combat", amount));
    }
    else
    {
        screen.addOption(2, "Utiliser depuis le menu Potions en combat", "Ce type demande une situation de combat.", false, "inventory.consumable.use_locked", makeInventoryConsumableItemData(consumable, "use", "Combat requis", amount));
    }

    screen.addOption(3, "Voir dans le bestiaire", "Consulter les informations connues liées à cette entrée.", true, "inventory.consumable.bestiary", makeInventoryConsumableItemData(consumable, "inspect", "", amount));
    return screen;
}

void InventoryDisplay::displaySelectedConsumable(const Consumable& consumable)
{
    TerminalInterface::renderMenuScreen(buildSelectedConsumableScreen(consumable));
}

MenuScreen InventoryDisplay::buildSelectedMaterialScreen(const Material& material)
{
    MenuScreen screen("ENTRÉE SÉLECTIONNÉE", "inventory.material.selected");
    screen.addLine("Nom : " + material.getName());
    screen.addLine("Catégorie : " + material.getCategory());
    screen.addLine("Quantité : " + std::to_string(material.getQuantity()));
    screen.addBackOption("Retour", "inventory.material.back");
    screen.addOption(1, "Inspecter", "Lire la description complète.", true, "inventory.material.inspect", makeInventoryMaterialItemData(material, "inspect"));
    screen.addOption(2, "Voir dans le bestiaire", "Consulter la connaissance liée à cette ressource.", true, "inventory.material.bestiary", makeInventoryMaterialItemData(material, "inspect"));
    screen.addOption(3, "Voir les usages connus", "Recettes, réparations ou pistes déjà découvertes.", true, "inventory.material.uses", makeInventoryMaterialItemData(material, "inspect"));
    screen.addOption(4, "Lire / utiliser", "Exploiter cette entrée si elle contient une note ou un usage direct.", true, "inventory.material.use", makeInventoryMaterialItemData(material, "use"));
    return screen;
}

void InventoryDisplay::displaySelectedMaterial(const Material& material)
{
    TerminalInterface::renderMenuScreen(buildSelectedMaterialScreen(material));
}

MenuScreen InventoryDisplay::buildUnavailableMaterialsScreen()
{
    MenuScreen screen("MATÉRIAUX", "inventory.materials.empty");
    screen.setDisplayOnlyInput("Section vide affichée sans saisie directe.");
    screen.addLine("Aucun matériau exploitable dans cette section.");
    screen.addLine("Quand tu en possèdes, ils peuvent servir à réparer, améliorer ou fabriquer de l'équipement.");
    return screen;
}

void InventoryDisplay::displayUnavailableMaterials()
{
    TerminalInterface::renderMenuScreen(buildUnavailableMaterialsScreen(), false);
}
