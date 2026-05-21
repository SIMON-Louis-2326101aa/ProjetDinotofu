// EN: CombatIntent.hpp briefly defines this Dinotofu module and its responsibilities.
// FR: CombatIntent.hpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file belongs to Dinotofu. Code identifiers are written in English; player-facing text can stay in French.
// Français : Ce fichier appartient à Dinotofu. Les identifiants du code sont en anglais ; les textes affichés au joueur peuvent rester en français.
// Description: Defines the narrative intent of an encounter, especially for future special adventurer groups.
// TODO: Later, this intent should influence rewards, death rules, dialogue choices and whether a fight can become lethal.

#ifndef INCLUDE_COMBAT_ENCOUNTER_COMBATINTENT_HPP
#define INCLUDE_COMBAT_ENCOUNTER_COMBATINTENT_HPP

enum class CombatIntent
{
    FriendlyDuel,
    Training,
    Warning,
    SeriousFight,
    ExperimentalFight,
    DangerousFight,
    DeathMatch,
    StoryLocked
};

#endif
