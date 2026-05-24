// EN: CombatPotionDisplay.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: CombatPotionDisplay.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "interface/menu/potions/CombatPotionDisplay.hpp"

#include "interface/menu/potions/CombatPotionUtils.hpp"

#include "item/Inventory.hpp"
#include "item/consumable/Consumable.hpp"

#include <iostream>

// EN: displayMainMenu declares or implements a focused behavior used by this module.
// FR: displayMainMenu déclare ou implémente un comportement précis utilisé par ce module.
void CombatPotionDisplay::displayMainMenu()
{
    std::cout << "========== POTIONS ==========" << std::endl;
    std::cout << "0 : Retour" << std::endl;
    std::cout << "1 : Voir les potions" << std::endl;
    std::cout << "2 : Utiliser une potion curative" << std::endl;
    std::cout << "3 : Utiliser une potion défensive" << std::endl;
    std::cout << "4 : Utiliser une potion offensive" << std::endl;
    std::cout << "5 : Utiliser une potion de buff" << std::endl;
    std::cout << "6 : Utiliser une potion de debuff" << std::endl;
    std::cout << "7 : Utiliser une potion spéciale" << std::endl;
    std::cout << "============================" << std::endl;
    std::cout << std::endl;
    std::cout << "> ";
}

void CombatPotionDisplay::displayQuickHealing(
    const Player& player,
    const std::vector<int>& indices
)
{
    std::cout << "========== POTION DE SOIN RAPIDE ==========" << std::endl;

    for (int i = 0; i < static_cast<int>(indices.size()); ++i)
    {
        int inventoryIndex = indices[i];
        Consumable potion = player.getInventory().getConsumable(inventoryIndex);

        std::cout << i + 1
                  << " : "
                  << potion.getName()
                  << " | Soin : "
                  << potion.getPower()
                  << " PV"
                  << std::endl;
    }

    std::cout << "===========================================" << std::endl;
    std::cout << "0 : Retour" << std::endl;
    std::cout << std::endl;
    std::cout << "> ";
}

// EN: displaySelectedHealingPotion declares or implements a focused behavior used by this module.
// FR: displaySelectedHealingPotion déclare ou implémente un comportement précis utilisé par ce module.
void CombatPotionDisplay::displaySelectedHealingPotion(const Consumable& potion)
{
    std::cout << "========== POTION SÉLECTIONNÉE ==========" << std::endl;
    std::cout << "Potion : " << potion.getName() << std::endl;
    std::cout << "Description : " << potion.getDescription() << std::endl;
    std::cout << "Soin : " << potion.getPower() << " PV" << std::endl;
    std::cout << "=========================================" << std::endl;
    std::cout << "0 : Retour" << std::endl;
    std::cout << "1 : Inspecter" << std::endl;
    std::cout << "2 : Utiliser" << std::endl;
    std::cout << std::endl;
    std::cout << "> ";
}

// EN: displaySelectedPotion declares or implements a focused behavior used by this module.
// FR: displaySelectedPotion déclare ou implémente un comportement précis utilisé par ce module.
void CombatPotionDisplay::displaySelectedPotion(const Consumable& potion)
{
    std::cout << "========== POTION SÉLECTIONNÉE ==========" << std::endl;
    std::cout << "Potion : " << potion.getName() << std::endl;
    std::cout << "Type : " << CombatPotionUtils::typeToText(potion.getType()) << std::endl;
    std::cout << "Puissance : " << potion.getPower() << std::endl;
    std::cout << "=========================================" << std::endl;
    std::cout << "0 : Retour" << std::endl;
    std::cout << "1 : Inspecter" << std::endl;
    std::cout << "2 : Utiliser" << std::endl;
    std::cout << std::endl;
    std::cout << "> ";
}

// EN: displayPotions declares or implements a focused behavior used by this module.
// FR: displayPotions déclare ou implémente un comportement précis utilisé par ce module.
void CombatPotionDisplay::displayPotions(const Player& player)
{
    const std::vector<Consumable>& consumables =
        player.getInventory().getConsumables();

    std::cout << "========== POTIONS DISPONIBLES ==========" << std::endl;

    if (consumables.empty())
    {
        std::cout << "Aucune potion dans l'inventaire." << std::endl;
        std::cout << "=========================================" << std::endl;
        std::cout << std::endl;
        return;
    }

    for (int i = 0; i < static_cast<int>(consumables.size()); ++i)
    {
        const Consumable& potion = consumables[i];

        std::cout << "[" << i << "] "
                  << potion.getName()
                  << " | "
                  << CombatPotionUtils::typeToText(potion.getType())
                  << " | Puissance : "
                  << potion.getPower()
                  << std::endl;
    }

    std::cout << "=========================================" << std::endl;
    std::cout << std::endl;
}

void CombatPotionDisplay::displayFilteredPotions(
    const Player& player,
    const std::vector<int>& indices
)
{
    std::cout << "========== LISTE DES POTIONS ==========" << std::endl;

    for (int i = 0; i < static_cast<int>(indices.size()); ++i)
    {
        int inventoryIndex = indices[i];
        Consumable potion = player.getInventory().getConsumable(inventoryIndex);

        std::cout << i + 1
                  << " : "
                  << potion.getName()
                  << " | Puissance : "
                  << potion.getPower()
                  << std::endl;
    }

    std::cout << "=======================================" << std::endl;
    std::cout << "Choisis une potion." << std::endl;
    std::cout << "Entre son numéro dans la liste, ou 0 pour revenir." << std::endl;
    std::cout << "> ";
}
