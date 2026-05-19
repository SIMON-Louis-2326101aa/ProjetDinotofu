// English: This file belongs to Dinotofu. Code identifiers are written in English; player-facing text can stay in French.
// Français : Ce fichier appartient à Dinotofu. Les identifiants du code sont en anglais ; les textes affichés au joueur peuvent rester en français.
// Description: Infers a simple combat role from an entity class/type for future targeting and AI rules.

#ifndef INCLUDE_COMBAT_ROLE_COMBATROLESYSTEM_HPP
#define INCLUDE_COMBAT_ROLE_COMBATROLESYSTEM_HPP

#include "combat/role/CombatRole.hpp"
#include "entity/Entity.hpp"

class CombatRoleSystem
{
public:
    static CombatRole getRole(const Entity& entity);
    static bool isTank(const Entity& entity);
    static bool isHealer(const Entity& entity);
    static bool isSummoner(const Entity& entity);
    static bool isAssassin(const Entity& entity);
};

#endif
