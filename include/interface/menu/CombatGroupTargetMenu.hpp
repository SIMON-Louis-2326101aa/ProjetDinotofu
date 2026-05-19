// English: This file belongs to Dinotofu. Code identifiers are written in English; player-facing text can stay in French.
// Français : Ce fichier appartient à Dinotofu. Les identifiants du code sont en anglais ; les textes affichés au joueur peuvent rester en français.
// Description: Lets the player choose targets through CombatGroup slots during the gradual migration to real group fights.

#ifndef INCLUDE_INTERFACE_MENU_COMBATGROUPTARGETMENU_HPP
#define INCLUDE_INTERFACE_MENU_COMBATGROUPTARGETMENU_HPP

#include "combat/summon/Summon.hpp"
#include "core/Random.hpp"
#include "entity/Entity.hpp"

#include <vector>

class CombatGroupTargetMenu
{
public:
    static bool openSingleEnemyAttack(
        Entity& attacker,
        Entity& mainDefender,
        std::vector<Summon>& defenderSummons,
        Random& random
    );
};

#endif
