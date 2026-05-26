// EN: CombatPotionDisplay.cpp centralizes potion screens for terminal and future GUI rendering.
// FR: CombatPotionDisplay.cpp centralise les écrans de potions pour le terminal et la future IG.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "interface/menu/potions/CombatPotionDisplay.hpp"

#include "interface/TerminalInterface.hpp"
#include "interface/menu/potions/CombatPotionUtils.hpp"

#include "item/Inventory.hpp"
#include "item/consumable/Consumable.hpp"

#include <iostream>
#include <string>

MenuScreen CombatPotionDisplay::buildMainScreen()
{
    MenuScreen screen("POTIONS", "potions.main");
    screen.addBackOption("Retour", "potions.back");
    screen.addOption(1, "Voir les potions", "Liste toutes les potions de l'inventaire.", true, "potions.list");
    screen.addOption(2, "Utiliser une potion curative", "Soin et récupération.", true, "potions.healing");
    screen.addOption(3, "Utiliser une potion défensive", "Protection, réduction ou résistance.", true, "potions.defensive");
    screen.addOption(4, "Utiliser une potion offensive", "Dégâts ou effets lancés sur l'ennemi.", true, "potions.offensive");
    screen.addOption(5, "Utiliser une potion de buff", "Renforcer temporairement le personnage.", true, "potions.buff");
    screen.addOption(6, "Utiliser une potion de debuff", "Affaiblir ou gêner la cible.", true, "potions.debuff");
    screen.addOption(7, "Utiliser une potion spéciale", "Effets rares, instables ou situationnels.", true, "potions.special");
    return screen;
}

MenuScreen CombatPotionDisplay::buildQuickHealingScreen(
    const Player& player,
    const std::vector<int>& indices
)
{
    MenuScreen screen("POTION DE SOIN RAPIDE", "potions.quick_heal");

    for (int i = 0; i < static_cast<int>(indices.size()); ++i)
    {
        int inventoryIndex = indices[i];
        Consumable potion = player.getInventory().getConsumable(inventoryIndex);

        screen.addOption(
            i + 1,
            potion.getName(),
            "Soin : " + std::to_string(potion.getPower()) + " PV",
            true,
            "potions.quick_heal.use"
        );
    }

    screen.addBackOption("Retour", "potions.quick_heal.back");
    return screen;
}

MenuScreen CombatPotionDisplay::buildSelectedHealingPotionScreen(const Consumable& potion)
{
    MenuScreen screen("POTION SÉLECTIONNÉE", "potions.healing.selected");
    screen.addLine("Potion : " + potion.getName());
    screen.addLine("Description : " + potion.getDescription());
    screen.addLine("Soin : " + std::to_string(potion.getPower()) + " PV");
    screen.addBackOption("Retour", "potions.healing.back");
    screen.addOption(1, "Inspecter", "Lire les détails de la potion.", true, "potions.healing.inspect");
    screen.addOption(2, "Utiliser", "Consommer cette potion maintenant.", true, "potions.healing.use");
    return screen;
}

MenuScreen CombatPotionDisplay::buildSelectedPotionScreen(const Consumable& potion)
{
    MenuScreen screen("POTION SÉLECTIONNÉE", "potions.selected");
    screen.addLine("Potion : " + potion.getName());
    screen.addLine("Type : " + CombatPotionUtils::typeToText(potion.getType()));
    screen.addLine("Puissance : " + std::to_string(potion.getPower()));
    screen.addBackOption("Retour", "potions.selected.back");
    screen.addOption(1, "Inspecter", "Lire les détails de la potion.", true, "potions.selected.inspect");
    screen.addOption(2, "Utiliser", "Consommer ou lancer cette potion selon son type.", true, "potions.selected.use");
    return screen;
}

MenuScreen CombatPotionDisplay::buildFilteredPotionsScreen(
    const Player& player,
    const std::vector<int>& indices
)
{
    MenuScreen screen("LISTE DES POTIONS", "potions.filtered");

    for (int i = 0; i < static_cast<int>(indices.size()); ++i)
    {
        int inventoryIndex = indices[i];
        Consumable potion = player.getInventory().getConsumable(inventoryIndex);
        screen.addOption(
            i + 1,
            potion.getName(),
            "Puissance : " + std::to_string(potion.getPower()),
            true,
            "potions.filtered.select"
        );
    }

    screen.addFooterLine("Choisis une potion.");
    screen.addFooterLine("Entre son numéro dans la liste, ou 0 pour revenir.");
    screen.addBackOption("Retour", "potions.filtered.back");
    return screen;
}

void CombatPotionDisplay::displayMainMenu()
{
    TerminalInterface::renderMenuScreen(buildMainScreen());
}

void CombatPotionDisplay::displayQuickHealing(
    const Player& player,
    const std::vector<int>& indices
)
{
    TerminalInterface::renderMenuScreen(buildQuickHealingScreen(player, indices));
}

void CombatPotionDisplay::displaySelectedHealingPotion(const Consumable& potion)
{
    TerminalInterface::renderMenuScreen(buildSelectedHealingPotionScreen(potion));
}

void CombatPotionDisplay::displaySelectedPotion(const Consumable& potion)
{
    TerminalInterface::renderMenuScreen(buildSelectedPotionScreen(potion));
}

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
    TerminalInterface::renderMenuScreen(buildFilteredPotionsScreen(player, indices));
}
