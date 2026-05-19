// English: This file belongs to Dinotofu. Code identifiers are written in English; player-facing text can stay in French.
// Français : Ce fichier appartient à Dinotofu. Les identifiants du code sont en anglais ; les textes affichés au joueur peuvent rester en français.
// Description: Applies first active role behaviours such as automatic tank provocation and future healer/support hooks.

#ifndef INCLUDE_COMBAT_ROLE_COMBATROLEACTIONSYSTEM_HPP
#define INCLUDE_COMBAT_ROLE_COMBATROLEACTIONSYSTEM_HPP

#include "core/Random.hpp"
#include "entity/Entity.hpp"

class CombatRoleActionSystem
{
public:
    static void tryActivateAutomaticRoleReaction(
        Entity& entity,
        Random& random
    );

    static bool activateManualProvocation(
        Entity& entity,
        int turns
    );

    static void displayRoleIdentity(const Entity& entity);
};

#endif
