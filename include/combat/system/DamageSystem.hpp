// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#ifndef INCLUDE_COMBAT_SYSTEM_DAMAGESYSTEM_HPP
#define INCLUDE_COMBAT_SYSTEM_DAMAGESYSTEM_HPP

#include "combat/DamageReport.hpp"
#include "entity/Entity.hpp"

class DamageSystem
{
public:
    static DamageReport calculerDegatsRecus(Entity& defender, int rawDamage);
    static void displayDamageReport(const Entity& defender, const DamageReport& rapport);

    static int appliquerProtectionArmure(Entity& defender, int rawDamage);
};

#endif
