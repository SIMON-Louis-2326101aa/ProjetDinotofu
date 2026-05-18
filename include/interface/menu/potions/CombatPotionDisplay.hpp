// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#ifndef INCLUDE_INTERFACE_MENU_POTIONS_COMBATPOTIONDISPLAY_HPP
#define INCLUDE_INTERFACE_MENU_POTIONS_COMBATPOTIONDISPLAY_HPP

#include "entity/Player.hpp"
#include "item/consumable/Consumable.hpp"

#include <vector>

class CombatPotionDisplay
{
public:
    static void displayMainMenu();

    static void displayQuickHealing(
        const Player& player,
        const std::vector<int>& indices
    );

    static void displaySelectedHealingPotion(const Consumable& potion);
    static void displaySelectedPotion(const Consumable& potion);

    static void displayPotions(const Player& player);

    static void displayFilteredPotions(
        const Player& player,
        const std::vector<int>& indices
    );
};

#endif
