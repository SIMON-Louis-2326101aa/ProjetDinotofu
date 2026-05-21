// EN: DamageSystem.hpp briefly defines this Dinotofu module and its responsibilities.
// FR: DamageSystem.hpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#ifndef INCLUDE_COMBAT_SYSTEM_DAMAGESYSTEM_HPP
#define INCLUDE_COMBAT_SYSTEM_DAMAGESYSTEM_HPP

#include "combat/DamageReport.hpp"
#include "entity/Entity.hpp"

class DamageSystem
{
public:
    // EN: calculateReceivedDamage declares or implements a focused behavior used by this module.
    // FR: calculateReceivedDamage déclare ou implémente un comportement précis utilisé par ce module.
    static DamageReport calculateReceivedDamage(Entity& defender, int rawDamage);
    // EN: displayDamageReport declares or implements a focused behavior used by this module.
    // FR: displayDamageReport déclare ou implémente un comportement précis utilisé par ce module.
    static void displayDamageReport(const Entity& defender, const DamageReport& rapport);

    // EN: applyArmorProtection declares or implements a focused behavior used by this module.
    // FR: applyArmorProtection déclare ou implémente un comportement précis utilisé par ce module.
    static int applyArmorProtection(Entity& defender, int rawDamage);
};

#endif
