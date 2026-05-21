// EN: CombatPotionUtils.hpp briefly defines this Dinotofu module and its responsibilities.
// FR: CombatPotionUtils.hpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#ifndef INCLUDE_INTERFACE_MENU_POTIONS_COMBATPOTIONUTILS_HPP
#define INCLUDE_INTERFACE_MENU_POTIONS_COMBATPOTIONUTILS_HPP

#include "entity/Player.hpp"
#include "item/consumable/ConsumableType.hpp"

#include <string>
#include <vector>

class CombatPotionUtils
{
public:
    static std::vector<int> getPotionIndices(
        const Player& player,
        ConsumableType type
    );

    // EN: typeToText declares or implements a focused behavior used by this module.
    // FR: typeToText déclare ou implémente un comportement précis utilisé par ce module.
    static std::string typeToText(ConsumableType type);
};

#endif
