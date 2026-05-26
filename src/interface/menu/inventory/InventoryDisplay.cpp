// EN: InventoryDisplay.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: InventoryDisplay.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "interface/menu/inventory/InventoryDisplay.hpp"

#include "interface/menu/inventory/InventoryUtils.hpp"
#include "interface/TerminalInterface.hpp"

#include "item/Inventory.hpp"
#include "item/consumable/Consumable.hpp"

#include <iostream>
#include <vector>

// EN: displayMainMenu declares or implements a focused behavior used by this module.
// FR: displayMainMenu déclare ou implémente un comportement précis utilisé par ce module.
MenuScreen InventoryDisplay::buildMainScreen()
{
    MenuScreen screen("INVENTAIRE", "inventory.main");
    screen.addOption(1, "Bestiaire (objet spécial)", "Consulter les créatures, boss, matériaux et informations connues.", true, "inventory.bestiary");
    screen.addOption(2, "Voir tout (affichage simple)", "Résumé court de l'or, des objets et des ressources.", true, "inventory.full_simple");
    screen.addOption(3, "Armes", "Voir, inspecter, équiper ou réparer les armes.", true, "inventory.weapons");
    screen.addOption(4, "Armures", "Voir, inspecter, équiper ou réparer les protections.", true, "inventory.armors");
    screen.addOption(5, "Consommables", "Potions et objets utilisables.", true, "inventory.consumables");
    screen.addOption(6, "Matériaux / plantes / infos", "Ressources, notes, fragments et composants.", true, "inventory.materials");
    screen.addOption(7, "Craft / schémas de fabrication", "Fabriquer ou améliorer avec les recettes connues.", true, "inventory.craft");
    screen.addOption(8, "Consulter mes quêtes", "Ouvrir le journal de quêtes.", true, "inventory.quests");
    screen.addBackOption("Retour", "inventory.back");
    return screen;
}

void InventoryDisplay::displayMainMenu()
{
    TerminalInterface::renderMenuScreen(buildMainScreen());
}

// EN: displaySimpleFullInventory declares or implements a focused behavior used by this module.
// FR: displaySimpleFullInventory déclare ou implémente un comportement précis utilisé par ce module.
void InventoryDisplay::displaySimpleFullInventory(const Player& player)
{
    const Inventory& inventory = player.getInventory();

    std::cout << "================ INVENTAIRE ================" << std::endl;
    std::cout << "Or : " << inventory.getGold() << " pièces" << std::endl;
    std::cout << "Objet spécial : Bestiaire" << std::endl;
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

    std::vector<ConsumableGroup> groups = InventoryUtils::groupConsumables(player);

    for (int i = 0; i < static_cast<int>(groups.size()); ++i)
    {
        const ConsumableGroup& group = groups[i];

        std::cout << "[" << i << "] " << group.name
                  << " x" << group.amount
                  << " | " << InventoryUtils::consumableTypeToText(group.type)
                  << " | Puissance : " << group.power
                  << std::endl;
    }

    std::cout << std::endl;
    std::cout << "Matériaux / plantes / infos : " << inventory.getMaterialCount() << std::endl;

    for (int i = 0; i < static_cast<int>(inventory.getMaterials().size()); ++i)
    {
        Material material = inventory.getMaterial(i);
        std::cout << "[" << i << "] " << material.getName()
                  << " x" << material.getQuantity()
                  << " | " << material.getCategory()
                  << std::endl;
    }

    std::cout << "============================================" << std::endl;
    std::cout << std::endl;
}

MenuScreen InventoryDisplay::buildSelectedWeaponScreen(const Weapon& weapon)
{
    MenuScreen screen("ARME SÉLECTIONNÉE", "inventory.weapon.selected");
    screen.addLine("Arme : " + weapon.getName());
    screen.addBackOption("Retour", "inventory.weapon.back");
    screen.addOption(1, "Inspecter", "Voir la description et les détails de l'arme.", true, "inventory.weapon.inspect");
    screen.addOption(2, "Équiper", "Équiper cette arme maintenant.", true, "inventory.weapon.equip");
    screen.addOption(3, "Voir dans le bestiaire", "Consulter les informations connues liées à cette entrée.", true, "inventory.weapon.bestiary");
    screen.addOption(4, "Réparer", "Utiliser un kit ou des matériaux compatibles.", true, "inventory.weapon.repair");
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
    screen.addOption(1, "Inspecter", "Voir la description et les détails de l'armure.", true, "inventory.armor.inspect");
    screen.addOption(2, "Équiper", "Équiper cette protection maintenant.", true, "inventory.armor.equip");
    screen.addOption(3, "Voir dans le bestiaire", "Consulter les informations connues liées à cette entrée.", true, "inventory.armor.bestiary");
    screen.addOption(4, "Réparer", "Utiliser un kit ou des matériaux compatibles.", true, "inventory.armor.repair");
    return screen;
}

void InventoryDisplay::displaySelectedArmor(const Armor& armor)
{
    TerminalInterface::renderMenuScreen(buildSelectedArmorScreen(armor));
}

MenuScreen InventoryDisplay::buildSelectedConsumableScreen(const Consumable& consumable)
{
    MenuScreen screen("CONSOMMABLE SÉLECTIONNÉ", "inventory.consumable.selected");
    screen.addLine("Consommable : " + consumable.getName());
    screen.addLine("Type : " + InventoryUtils::consumableTypeToText(consumable.getType()));
    screen.addLine("Puissance : " + std::to_string(consumable.getPower()));
    screen.addBackOption("Retour", "inventory.consumable.back");
    screen.addOption(1, "Inspecter", "Lire la description complète.", true, "inventory.consumable.inspect");

    if (consumable.getType() == ConsumableType::Healing)
    {
        screen.addOption(2, "Utiliser", "Boire cette potion de soin hors combat.", true, "inventory.consumable.use");
    }
    else
    {
        screen.addOption(2, "Utiliser depuis le menu Potions en combat", "Ce type demande une situation de combat.", true, "inventory.consumable.use_locked");
    }

    screen.addOption(3, "Voir dans le bestiaire", "Consulter les informations connues liées à cette entrée.", true, "inventory.consumable.bestiary");
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
    screen.addOption(1, "Inspecter", "Lire la description complète.", true, "inventory.material.inspect");
    screen.addOption(2, "Voir dans le bestiaire", "Consulter la connaissance liée à cette ressource.", true, "inventory.material.bestiary");
    screen.addOption(3, "Voir les usages connus", "Recettes, réparations ou pistes déjà découvertes.", true, "inventory.material.uses");
    screen.addOption(4, "Lire / utiliser", "Exploiter cette entrée si elle contient une note ou un usage direct.", true, "inventory.material.use");
    return screen;
}

void InventoryDisplay::displaySelectedMaterial(const Material& material)
{
    TerminalInterface::renderMenuScreen(buildSelectedMaterialScreen(material));
}

MenuScreen InventoryDisplay::buildUnavailableMaterialsScreen()
{
    MenuScreen screen("MATÉRIAUX", "inventory.materials.empty");
    screen.addLine("Aucun matériau exploitable dans cette section.");
    screen.addLine("Quand tu en possèdes, ils peuvent servir à réparer, améliorer ou fabriquer de l'équipement.");
    return screen;
}

void InventoryDisplay::displayUnavailableMaterials()
{
    TerminalInterface::renderMenuScreen(buildUnavailableMaterialsScreen(), false);
}
