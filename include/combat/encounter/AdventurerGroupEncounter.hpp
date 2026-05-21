// EN: AdventurerGroupEncounter.hpp briefly defines this Dinotofu module and its responsibilities.
// FR: AdventurerGroupEncounter.hpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file prepares random adventurer group encounters for PvE-like battles.
// Français : Ce fichier prépare les rencontres de groupes d'aventuriers aléatoires pour les combats façon PvE.
// Description: Creates human and semi-human opponent groups using random fighters or special character groups.

#ifndef INCLUDE_COMBAT_ENCOUNTER_ADVENTURERGROUPENCOUNTER_HPP
#define INCLUDE_COMBAT_ENCOUNTER_ADVENTURERGROUPENCOUNTER_HPP

#include "combat/EnemyCombatQueue.hpp"
#include "core/Random.hpp"
#include "entity/Player.hpp"

#include <string>
#include <vector>

class AdventurerGroupEncounter
{
public:
    static EnemyCombatQueue createRandomGroupForPlayer(
        const Player& player,
        Random& random
    );

    // EN: displayGroupEncounterIntroduction declares or implements a focused behavior used by this module.
    // FR: displayGroupEncounterIntroduction déclare ou implémente un comportement précis utilisé par ce module.
    static void displayGroupEncounterIntroduction();

private:
    static EnemyCombatQueue createClassicRandomGroup(
        const Player& player,
        Random& random
    );

    static EnemyCombatQueue createSpecialGroup(
        const Player& player,
        Random& random
    );

    static void addPlayerAsOpponent(
        EnemyCombatQueue& queue,
        const Player& opponent,
        int encounterLevel
    );

    static void addSpecialCharacterAsOpponent(
        EnemyCombatQueue& queue,
        const std::string& characterName,
        int encounterLevel,
        const std::vector<std::string>& groupNames
    );

    static void applyRelationshipBonus(
        Player& opponent,
        const std::string& characterName,
        const std::vector<std::string>& groupNames
    );

    static void announceRelationshipBonus(
        const std::vector<std::string>& groupNames
    );

    // EN: announceSpecialGroup declares or implements a focused behavior used by this module.
    // FR: announceSpecialGroup déclare ou implémente un comportement précis utilisé par ce module.
    static void announceSpecialGroup(const std::vector<std::string>& names);
};

#endif
