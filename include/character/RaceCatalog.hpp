// English: This file belongs to Dinotofu. Code identifiers are written in English; player-facing text can stay in French.
// Français : Ce fichier appartient à Dinotofu. Les identifiants du code sont en anglais ; les textes affichés au joueur peuvent rester en français.
// Description: Playable race catalog, racial starting bonuses and future trade modifiers.

#ifndef INCLUDE_CHARACTER_RACECATALOG_HPP
#define INCLUDE_CHARACTER_RACECATALOG_HPP

#include "character/CharacterRace.hpp"

#include <string>
#include <vector>

struct RaceStartingBonus
{
    int maxHpBonus;
    int minDamageBonus;
    int maxDamageBonus;
    int criticalDamageBonus;
};

class RaceCatalog
{
public:
    static int getPlayableRaceCount();
    static std::vector<CharacterRace> getPlayableRaces();

    static CharacterRace getPlayableRaceByChoice(int choice);
    static void displayPlayableRaces();

    static RaceStartingBonus getStartingBonus(CharacterRace race);
    static std::string getShortDescription(CharacterRace race);
    static std::string getGameplayIdentity(CharacterRace race);

    static int getMerchantPurchasePricePercentage(CharacterRace race);
    static int getMerchantSalePricePercentage(CharacterRace race);
};

#endif
