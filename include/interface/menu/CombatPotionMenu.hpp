// EN: CombatPotionMenu.hpp briefly defines this Dinotofu module and its responsibilities.
// FR: CombatPotionMenu.hpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#ifndef INCLUDE_INTERFACE_MENU_COMBATPOTIONMENU_HPP
#define INCLUDE_INTERFACE_MENU_COMBATPOTIONMENU_HPP

#include "core/Random.hpp"

#include "entity/Player.hpp"
#include "entity/Entity.hpp"

#include "combat/EnemyCombatQueue.hpp"

#include "item/consumable/ConsumableType.hpp"

#include <vector>

class CombatPotionMenu
{
public:
    // EN: openQuickHealing declares or implements a focused behavior used by this module.
    // FR: openQuickHealing déclare ou implémente un comportement précis utilisé par ce module.
    static bool openQuickHealing(Player& player);

    static bool openAgainstSingleTarget(
        Player& player,
        Entity& target,
        Random& random,
        int potionDamageBonus
    );

    static bool openAgainstWave(
        Player& player,
        EnemyCombatQueue& wave,
        Random& random,
        int potionDamageBonus
    );

private:
    static bool openCategory(
        Player& player,
        ConsumableType type,
        Entity* target,
        EnemyCombatQueue* wave,
        Random& random,
        int potionDamageBonus
    );

    static bool openPotionSelection(
        Player& player,
        const std::vector<int>& indices,
        ConsumableType type,
        Entity* target,
        EnemyCombatQueue* wave,
        Random& random,
        int potionDamageBonus
    );
};

#endif
