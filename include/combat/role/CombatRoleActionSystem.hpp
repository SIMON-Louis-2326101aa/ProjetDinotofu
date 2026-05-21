// EN: CombatRoleActionSystem.hpp briefly defines this Dinotofu module and its responsibilities.
// FR: CombatRoleActionSystem.hpp résume brièvement ce module de Dinotofu et ses responsabilités.
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

    static bool tryActivateAllyProtection(
        Entity& protector,
        Entity& endangeredAlly,
        Random& random
    );

    static bool tryActivateSupportRecovery(
        Entity& support,
        Entity& ally,
        Random& random
    );

    // EN: displayRoleIdentity declares or implements a focused behavior used by this module.
    // FR: displayRoleIdentity déclare ou implémente un comportement précis utilisé par ce module.
    static void displayRoleIdentity(const Entity& entity);
};

#endif
