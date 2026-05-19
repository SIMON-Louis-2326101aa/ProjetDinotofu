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
    static void markAllyHealingAction(Entity& healer, const Entity& healedAlly);
    static void markSelfHealingAction(const Entity& healer);

    static void tryActivatePassiveProvocation(Entity& entity, Random& random);

    static bool shouldForceTargetMainEntity(
        const Entity& mainTarget,
        const std::string& attackerName
    );

    static void notifyForcedTarget(
        const Entity& target,
        const std::string& attackerName
    );

    static void consumeForcedTargetIfNeeded(Entity& target);

    static bool isNaturalProvoker(const Entity& entity);
};

#endif
