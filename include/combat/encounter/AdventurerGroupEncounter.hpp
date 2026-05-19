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
        int encounterLevel
    );

    static void announceSpecialGroup(const std::vector<std::string>& names);
};

#endif
