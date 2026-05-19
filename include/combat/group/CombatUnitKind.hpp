// English: This file belongs to Dinotofu. Code identifiers are written in English; player-facing text can stay in French.
// Français : Ce fichier appartient à Dinotofu. Les identifiants du code sont en anglais ; les textes affichés au joueur peuvent rester en français.
// Description: Identifies what kind of unit occupies a combat slot.

#ifndef INCLUDE_COMBAT_GROUP_COMBATUNITKIND_HPP
#define INCLUDE_COMBAT_GROUP_COMBATUNITKIND_HPP

enum class CombatUnitKind
{
    Empty,
    MainFighter,
    Ally,
    Enemy,
    Summon,
    Boss
};

#endif
