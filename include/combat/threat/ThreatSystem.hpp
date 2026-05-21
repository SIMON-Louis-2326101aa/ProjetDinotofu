// EN: ThreatSystem.hpp briefly defines this Dinotofu module and its responsibilities.
// FR: ThreatSystem.hpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file belongs to Dinotofu. Code identifiers are written in English; player-facing text can stay in French.
// Français : Ce fichier appartient à Dinotofu. Les identifiants du code sont en anglais ; les textes affichés au joueur peuvent rester en français.
// Description: Handles simple threat rules such as ally healers becoming priority targets and tanks using provocation.

#ifndef INCLUDE_COMBAT_THREAT_THREATSYSTEM_HPP
#define INCLUDE_COMBAT_THREAT_THREATSYSTEM_HPP

#include "core/Random.hpp"
#include "entity/Entity.hpp"

#include <string>

class ThreatSystem
{
public:
    // EN: markAllyHealingAction declares or implements a focused behavior used by this module.
    // FR: markAllyHealingAction déclare ou implémente un comportement précis utilisé par ce module.
    static void markAllyHealingAction(Entity& healer, const Entity& healedAlly);
    // EN: markSelfHealingAction declares or implements a focused behavior used by this module.
    // FR: markSelfHealingAction déclare ou implémente un comportement précis utilisé par ce module.
    static void markSelfHealingAction(const Entity& healer);

    // EN: tryActivatePassiveProvocation declares or implements a focused behavior used by this module.
    // FR: tryActivatePassiveProvocation déclare ou implémente un comportement précis utilisé par ce module.
    static void tryActivatePassiveProvocation(Entity& entity, Random& random);

    static bool shouldForceTargetMainEntity(
        const Entity& mainTarget,
        const std::string& attackerName
    );

    static void notifyForcedTarget(
        const Entity& target,
        const std::string& attackerName
    );

    // EN: consumeForcedTargetIfNeeded declares or implements a focused behavior used by this module.
    // FR: consumeForcedTargetIfNeeded déclare ou implémente un comportement précis utilisé par ce module.
    static void consumeForcedTargetIfNeeded(Entity& target);

    // EN: isNaturalProvoker declares or implements a focused behavior used by this module.
    // FR: isNaturalProvoker déclare ou implémente un comportement précis utilisé par ce module.
    static bool isNaturalProvoker(const Entity& entity);
};

#endif
