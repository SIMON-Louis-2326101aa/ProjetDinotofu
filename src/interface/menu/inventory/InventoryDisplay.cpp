// EN: InventoryDisplay.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: InventoryDisplay.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "interface/menu/inventory/InventoryDisplay.hpp"

#include "interface/menu/inventory/InventoryUtils.hpp"
#include "interface/menu/common/MenuFrame.hpp"

#include "item/Inventory.hpp"
#include "item/consumable/Consumable.hpp"

#include <iostream>
#include <vector>

// EN: displayMainMenu declares or implements a focused behavior used by this module.
// FR: displayMainMenu déclare ou implémente un comportement précis utilisé par ce module.
void InventoryDisplay::displayMainMenu()
{
    MenuFrame::title("INVENTAIRE");
    MenuFrame::option(1, "Bestiaire (objet spécial)");
    MenuFrame::option(2, "Voir tout (affichage simple)");
    MenuFrame::option(3, "Armes");
    MenuFrame::option(4, "Armures");
    MenuFrame::option(5, "Consommables");
    MenuFrame::option(6, "Matériaux / plantes / infos");
    MenuFrame::option(7, "Craft / schémas de fabrication");
    MenuFrame::option(8, "Consulter mes quêtes");
    MenuFrame::backOption("Retour");
    MenuFrame::end();
    MenuFrame::prompt();
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

// EN: displaySelectedWeapon declares or implements a focused behavior used by this module.
// FR: displaySelectedWeapon déclare ou implémente un comportement précis utilisé par ce module.
void InventoryDisplay::displaySelectedWeapon(const Weapon& weapon)
{
    std::cout << "========== ARME SÉLECTIONNÉE ==========" << std::endl;
    std::cout << "Arme : " << weapon.getName() << std::endl;
    std::cout << "=======================================" << std::endl;
    std::cout << "0 : Retour" << std::endl;
    std::cout << "1 : Inspecter" << std::endl;
    std::cout << "2 : Équiper" << std::endl;
    std::cout << "3 : Voir dans le bestiaire" << std::endl;
    std::cout << "4 : Réparer" << std::endl;
    std::cout << std::endl;
    std::cout << "> ";
}

// EN: displaySelectedArmor declares or implements a focused behavior used by this module.
// FR: displaySelectedArmor déclare ou implémente un comportement précis utilisé par ce module.
void InventoryDisplay::displaySelectedArmor(const Armor& armor)
{
    std::cout << "========== ARMURE SÉLECTIONNÉE ==========" << std::endl;
    std::cout << "Armure : " << armor.getName() << std::endl;
    std::cout << "=========================================" << std::endl;
    std::cout << "0 : Retour" << std::endl;
    std::cout << "1 : Inspecter" << std::endl;
    std::cout << "2 : Équiper" << std::endl;
    std::cout << "3 : Voir dans le bestiaire" << std::endl;
    std::cout << "4 : Réparer" << std::endl;
    std::cout << std::endl;
    std::cout << "> ";
}

// EN: displaySelectedConsumable declares or implements a focused behavior used by this module.
// FR: displaySelectedConsumable déclare ou implémente un comportement précis utilisé par ce module.
void InventoryDisplay::displaySelectedConsumable(const Consumable& consumable)
{
    std::cout << "========== CONSOMMABLE SÉLECTIONNÉ ==========" << std::endl;
    std::cout << "Consommable : " << consumable.getName() << std::endl;
    std::cout << "Type : " << InventoryUtils::consumableTypeToText(consumable.getType()) << std::endl;
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

    std::cout << "3 : Voir dans le bestiaire" << std::endl;
    std::cout << std::endl;
    std::cout << "> ";
}


// EN: displaySelectedMaterial declares or implements a focused behavior used by this module.
// FR: displaySelectedMaterial déclare ou implémente un comportement précis utilisé par ce module.
void InventoryDisplay::displaySelectedMaterial(const Material& material)
{
    std::cout << "========== ENTRÉE SÉLECTIONNÉE ==========" << std::endl;
    std::cout << "Nom : " << material.getName() << std::endl;
    std::cout << "Catégorie : " << material.getCategory() << std::endl;
    std::cout << "Quantité : " << material.getQuantity() << std::endl;
    std::cout << "=========================================" << std::endl;
    std::cout << "0 : Retour" << std::endl;
    std::cout << "1 : Inspecter" << std::endl;
    std::cout << "2 : Voir dans le bestiaire" << std::endl;
    std::cout << "3 : Voir l'utilité prévue" << std::endl;
    std::cout << "4 : Lire / utiliser" << std::endl;
    std::cout << std::endl;
    std::cout << "> ";
}

// EN: displayUnavailableMaterials declares or implements a focused behavior used by this module.
// FR: displayUnavailableMaterials déclare ou implémente un comportement précis utilisé par ce module.
void InventoryDisplay::displayUnavailableMaterials()
{
    std::cout << "========== MATÉRIAUX ==========" << std::endl;
    std::cout << "Les matériaux ne sont pas encore disponibles." << std::endl;
    std::cout << "Plus tard, ils serviront à réparer, améliorer et fabriquer de l'équipement." << std::endl;
    std::cout << "===============================" << std::endl;
    std::cout << std::endl;
}
