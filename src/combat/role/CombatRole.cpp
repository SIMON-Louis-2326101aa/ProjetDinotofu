// English: This file belongs to Dinotofu. Code identifiers are written in English; player-facing text can stay in French.
// Français : Ce fichier appartient à Dinotofu. Les identifiants du code sont en anglais ; les textes affichés au joueur peuvent rester en français.
// Description: Defines broad combat roles used by targeting, threat, and future AI decisions.

#include "combat/role/CombatRole.hpp"

std::string combatRoleToText(CombatRole role)
{
    switch (role)
    {
        case CombatRole::Tank:
            return "Tank";

        case CombatRole::Healer:
            return "Soigneur";

        case CombatRole::DamageDealer:
            return "DPS";

        case CombatRole::Assassin:
            return "Assassin";

        case CombatRole::Summoner:
            return "Invocateur";

        case CombatRole::Support:
            return "Support";

        case CombatRole::Hybrid:
            return "Hybride";

        case CombatRole::Unknown:
        default:
            return "Inconnu";
    }
}
