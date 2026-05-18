// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#ifndef INCLUDE_COMBAT_TURN_HUMANCOMBATTURN_HPP
#define INCLUDE_COMBAT_TURN_HUMANCOMBATTURN_HPP

#include "core/Random.hpp"

#include "entity/Entity.hpp"

class HumanCombatTurn
{
public:
    static bool jouer(
        Entity& attacker,
        Entity& defender,
        Random& random,
        int potionHealAmount,
        int potionDamageBonus
    );

private:
    static bool openObservationInterface(
        Entity& joueurInterface,
        Entity& target
    );

    static bool gererFuite(
        Entity& attacker,
        Entity& defender,
        Random& random
    );
};

#endif
