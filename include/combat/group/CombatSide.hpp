// EN: CombatSide.hpp briefly defines this Dinotofu module and its responsibilities.
// FR: CombatSide.hpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file belongs to Dinotofu. Code identifiers are written in English; player-facing text can stay in French.
// Français : Ce fichier appartient à Dinotofu. Les identifiants du code sont en anglais ; les textes affichés au joueur peuvent rester en français.
// Description: Defines the two broad sides used by future group combat slots.

#ifndef INCLUDE_COMBAT_GROUP_COMBATSIDE_HPP
#define INCLUDE_COMBAT_GROUP_COMBATSIDE_HPP

enum class CombatSide
{
    PlayerSide,
    EnemySide,
    Neutral
};

#endif
