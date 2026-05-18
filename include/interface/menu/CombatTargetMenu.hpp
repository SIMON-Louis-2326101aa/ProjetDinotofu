// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#ifndef INCLUDE_INTERFACE_MENU_COMBATTARGETMENU_HPP
#define INCLUDE_INTERFACE_MENU_COMBATTARGETMENU_HPP

#include "entity/Player.hpp"
#include "combat/EnemyCombatQueue.hpp"
#include "core/Random.hpp"

class CombatTargetMenu
{
public:
    static bool openForAttack(
        Player& player,
        EnemyCombatQueue& wave,
        Random& random
    );

    static bool ouvrirPourPotionDegats(
        Player& player,
        EnemyCombatQueue& wave,
        Random& random,
        int potionDamageBonus
    );

    static bool openForBoostedAttack(
        Player& player,
        EnemyCombatQueue& wave,
        Random& random,
        int damageBonus
    );

private:
    static int chooseTarget(const EnemyCombatQueue& wave);

    static bool ouvrirMenuCible(
        Player& player,
        EnemyCombatQueue& wave,
        Random& random,
        bool boostedAttack,
        int damageBonus
    );
};

#endif
