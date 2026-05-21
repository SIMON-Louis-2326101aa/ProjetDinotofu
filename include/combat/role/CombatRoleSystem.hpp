// EN: CombatRoleSystem.hpp briefly defines this Dinotofu module and its responsibilities.
// FR: CombatRoleSystem.hpp résume brièvement ce module de Dinotofu et ses responsabilités.
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
    // EN: getRole declares or implements a focused behavior used by this module.
    // FR: getRole déclare ou implémente un comportement précis utilisé par ce module.
    static CombatRole getRole(const Entity& entity);
    // EN: isTank declares or implements a focused behavior used by this module.
    // FR: isTank déclare ou implémente un comportement précis utilisé par ce module.
    static bool isTank(const Entity& entity);
    // EN: isHealer declares or implements a focused behavior used by this module.
    // FR: isHealer déclare ou implémente un comportement précis utilisé par ce module.
    static bool isHealer(const Entity& entity);
    // EN: isSummoner declares or implements a focused behavior used by this module.
    // FR: isSummoner déclare ou implémente un comportement précis utilisé par ce module.
    static bool isSummoner(const Entity& entity);
    // EN: isAssassin declares or implements a focused behavior used by this module.
    // FR: isAssassin déclare ou implémente un comportement précis utilisé par ce module.
    static bool isAssassin(const Entity& entity);
    // EN: isSupport declares or implements a focused behavior used by this module.
    // FR: isSupport déclare ou implémente un comportement précis utilisé par ce module.
    static bool isSupport(const Entity& entity);
    // EN: isHybrid declares or implements a focused behavior used by this module.
    // FR: isHybrid déclare ou implémente un comportement précis utilisé par ce module.
    static bool isHybrid(const Entity& entity);
};

#endif
