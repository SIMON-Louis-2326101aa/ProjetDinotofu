// EN: CombatPotionDisplay.hpp briefly defines this Dinotofu module and its responsibilities.
// FR: CombatPotionDisplay.hpp résume brièvement ce module de Dinotofu et ses responsabilités.
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
    // EN: displayMainMenu declares or implements a focused behavior used by this module.
    // FR: displayMainMenu déclare ou implémente un comportement précis utilisé par ce module.
    static void displayMainMenu();

    static void displayQuickHealing(
        const Player& player,
        const std::vector<int>& indices
    );

    // EN: displaySelectedHealingPotion declares or implements a focused behavior used by this module.
    // FR: displaySelectedHealingPotion déclare ou implémente un comportement précis utilisé par ce module.
    static void displaySelectedHealingPotion(const Consumable& potion);
    // EN: displaySelectedPotion declares or implements a focused behavior used by this module.
    // FR: displaySelectedPotion déclare ou implémente un comportement précis utilisé par ce module.
    static void displaySelectedPotion(const Consumable& potion);

    // EN: displayPotions declares or implements a focused behavior used by this module.
    // FR: displayPotions déclare ou implémente un comportement précis utilisé par ce module.
    static void displayPotions(const Player& player);

    static void displayFilteredPotions(
        const Player& player,
        const std::vector<int>& indices
    );
};

#endif
