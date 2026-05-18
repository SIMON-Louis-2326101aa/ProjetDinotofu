// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#ifndef INCLUDE_COMBAT_ACTION_SPECIALCOMBATEFFECTS_HPP
#define INCLUDE_COMBAT_ACTION_SPECIALCOMBATEFFECTS_HPP

#include "entity/Entity.hpp"

class SpecialCombatEffects
{
public:
    static bool atlasBlocksAttack(
        Entity& attacker,
        Entity& defender,
        int damage
    );

    static void applyDemonLifestealIfNeeded(
        Entity& attacker,
        int damageDealt
    );
};

#endif
