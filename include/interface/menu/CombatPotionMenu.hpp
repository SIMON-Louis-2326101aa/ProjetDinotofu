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
    static bool ouvrirCategorie(
        Player& player,
        ConsumableType type,
        Entity* target,
        EnemyCombatQueue* wave,
        Random& random,
        int potionDamageBonus
    );

    static bool ouvrirSelectionPotion(
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
