// EN: DefensePostureSystem.hpp briefly defines this Dinotofu module and its responsibilities.
// FR: DefensePostureSystem.hpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#ifndef INCLUDE_COMBAT_SYSTEM_DEFENSEPOSTURESYSTEM_HPP
#define INCLUDE_COMBAT_SYSTEM_DEFENSEPOSTURESYSTEM_HPP

#include "core/Random.hpp"
#include "entity/Entity.hpp"

class DefensePostureSystem
{
public:
    // EN: enterDefensePosture declares or implements a focused behavior used by this module.
    // FR: enterDefensePosture déclare ou implémente un comportement précis utilisé par ce module.
    static void enterDefensePosture(Entity& entity);
    // EN: reduceIncomingDamage declares or implements a focused behavior used by this module.
    // FR: reduceIncomingDamage déclare ou implémente un comportement précis utilisé par ce module.
    static int reduceIncomingDamage(Entity& defender, int receivedDamage);
    // EN: tryCounterAfterMiss declares or implements a focused behavior used by this module.
    // FR: tryCounterAfterMiss déclare ou implémente un comportement précis utilisé par ce module.
    static void tryCounterAfterMiss(Entity& defender, Entity& attacker, Random& random);

private:
    // EN: containsClassText declares or implements a focused behavior used by this module.
    // FR: containsClassText déclare ou implémente un comportement précis utilisé par ce module.
    static bool containsClassText(const Entity& entity, const std::string& searched);
    // EN: isMeleeClass declares or implements a focused behavior used by this module.
    // FR: isMeleeClass déclare ou implémente un comportement précis utilisé par ce module.
    static bool isMeleeClass(const Entity& entity);
    // EN: isDistanceClass declares or implements a focused behavior used by this module.
    // FR: isDistanceClass déclare ou implémente un comportement précis utilisé par ce module.
    static bool isDistanceClass(const Entity& entity);
    // EN: isCounterMeleeClass declares or implements a focused behavior used by this module.
    // FR: isCounterMeleeClass déclare ou implémente un comportement précis utilisé par ce module.
    static bool isCounterMeleeClass(const Entity& entity);
    // EN: isCounterDistanceClass declares or implements a focused behavior used by this module.
    // FR: isCounterDistanceClass déclare ou implémente un comportement précis utilisé par ce module.
    static bool isCounterDistanceClass(const Entity& entity);
};

#endif
