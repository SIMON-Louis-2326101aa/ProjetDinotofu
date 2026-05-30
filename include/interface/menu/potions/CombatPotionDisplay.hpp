// EN: CombatPotionDisplay.hpp exposes potion screens through shared UI models.
// FR: CombatPotionDisplay.hpp expose les écrans de potions via les modèles d'interface partagés.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#ifndef INCLUDE_INTERFACE_MENU_POTIONS_COMBATPOTIONDISPLAY_HPP
#define INCLUDE_INTERFACE_MENU_POTIONS_COMBATPOTIONDISPLAY_HPP

#include "entity/Player.hpp"
#include "interface/model/MenuScreen.hpp"
#include "item/consumable/Consumable.hpp"

#include <cstddef>
#include <vector>

class CombatPotionDisplay
{
public:
    static MenuScreen buildMainScreen();
    static MenuScreen buildMainScreen(const Player& player);
    static MenuScreen buildQuickHealingScreen(
        const Player& player,
        const std::vector<int>& indices,
        std::size_t pageIndex = 0,
        std::size_t itemsPerPage = 10
    );
    static MenuScreen buildSelectedHealingPotionScreen(const Consumable& potion);
    static MenuScreen buildSelectedPotionScreen(const Consumable& potion);
    static MenuScreen buildFilteredPotionsScreen(
        const Player& player,
        const std::vector<int>& indices,
        std::size_t pageIndex = 0,
        std::size_t itemsPerPage = 10
    );
    static MenuScreen buildPotionOverviewScreen(
        const Player& player,
        std::size_t pageIndex = 0,
        std::size_t itemsPerPage = 10
    );

    static void displayMainMenu();

    static void displayQuickHealing(
        const Player& player,
        const std::vector<int>& indices
    );

    static void displaySelectedHealingPotion(const Consumable& potion);
    static void displaySelectedPotion(const Consumable& potion);

    static void displayPotions(const Player& player);
    static void showPotionDetails(const Consumable& potion);
    static void showEmptyCategory(const std::string& typeName);
    static void showPotionMissing();

    static void displayFilteredPotions(
        const Player& player,
        const std::vector<int>& indices
    );
};

#endif
