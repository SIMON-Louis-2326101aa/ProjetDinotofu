// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#ifndef INCLUDE_INTERFACE_COMBATDISPLAY_HPP
#define INCLUDE_INTERFACE_COMBATDISPLAY_HPP

#include "entity/Entity.hpp"

class CombatDisplay
{
public:
    static void displayCombatResult(
        const Entity& combattant1,
        const Entity& combattant2
    );
};

#endif
