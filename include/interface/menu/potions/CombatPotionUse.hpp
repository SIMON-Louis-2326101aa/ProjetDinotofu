// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#ifndef INCLUDE_INTERFACE_MENU_POTIONS_COMBATPOTIONUSE_HPP
#define INCLUDE_INTERFACE_MENU_POTIONS_COMBATPOTIONUSE_HPP

#include "core/Random.hpp"

#include "entity/Player.hpp"
#include "entity/Entity.hpp"

#include "combat/EnemyCombatQueue.hpp"

#include "item/consumable/Consumable.hpp"
#include "item/consumable/ConsumableType.hpp"

class CombatPotionUse
{
public:
    static bool useHealingPotion(
        Player& player,
        int consumableIndex,
        const Consumable& potion
    );

    static bool useSelectedPotion(
        Player& player,
        int consumableIndex,
        ConsumableType type,
        Entity* target,
        EnemyCombatQueue* wave,
        Random& random,
        int potionDamageBonus
    );
};

#endif
