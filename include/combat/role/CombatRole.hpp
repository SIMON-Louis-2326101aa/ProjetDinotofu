// EN: CombatRole.hpp briefly defines this Dinotofu module and its responsibilities.
// FR: CombatRole.hpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file belongs to Dinotofu. Code identifiers are written in English; player-facing text can stay in French.
// Français : Ce fichier appartient à Dinotofu. Les identifiants du code sont en anglais ; les textes affichés au joueur peuvent rester en français.
// Description: Defines broad combat roles used by targeting, threat, and future AI decisions.

#ifndef INCLUDE_COMBAT_ROLE_COMBATROLE_HPP
#define INCLUDE_COMBAT_ROLE_COMBATROLE_HPP

#include <string>

enum class CombatRole
{
    Tank,
    Healer,
    DamageDealer,
    Assassin,
    Summoner,
    Support,
    Hybrid,
    Unknown
};

std::string combatRoleToText(CombatRole role);

#endif
