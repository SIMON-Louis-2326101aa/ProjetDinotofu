// EN: RaceCatalog.hpp briefly defines this Dinotofu module and its responsibilities.
// FR: RaceCatalog.hpp résume brièvement ce module de Dinotofu et ses responsabilités.
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
    // EN: getPlayableRaceCount declares or implements a focused behavior used by this module.
    // FR: getPlayableRaceCount déclare ou implémente un comportement précis utilisé par ce module.
    static int getPlayableRaceCount();
    // EN: getPlayableRaces declares or implements a focused behavior used by this module.
    // FR: getPlayableRaces déclare ou implémente un comportement précis utilisé par ce module.
    static std::vector<CharacterRace> getPlayableRaces();

    // EN: getPlayableRaceByChoice declares or implements a focused behavior used by this module.
    // FR: getPlayableRaceByChoice déclare ou implémente un comportement précis utilisé par ce module.
    static CharacterRace getPlayableRaceByChoice(int choice);
    // EN: displayPlayableRaces declares or implements a focused behavior used by this module.
    // FR: displayPlayableRaces déclare ou implémente un comportement précis utilisé par ce module.
    static std::vector<std::string> getPlayableRaceDisplayLines();
    static void displayPlayableRaces();

    // EN: getStartingBonus declares or implements a focused behavior used by this module.
    // FR: getStartingBonus déclare ou implémente un comportement précis utilisé par ce module.
    static RaceStartingBonus getStartingBonus(CharacterRace race);
    // EN: getShortDescription declares or implements a focused behavior used by this module.
    // FR: getShortDescription déclare ou implémente un comportement précis utilisé par ce module.
    static std::string getShortDescription(CharacterRace race);
    // EN: getGameplayIdentity declares or implements a focused behavior used by this module.
    // FR: getGameplayIdentity déclare ou implémente un comportement précis utilisé par ce module.
    static std::string getGameplayIdentity(CharacterRace race);
    static bool hasInnateNightVision(CharacterRace race);
    static int getEnvironmentalTemperatureScore(CharacterRace race, const std::string& hazard);
    static bool hasFireWeakness(CharacterRace race);
    static std::string getElementalAffinityLine(CharacterRace race);
    static std::string getInnatePassiveLine(CharacterRace race);
    static bool isSemiHumanFamily(CharacterRace race);
    static std::string getRaceFamilyLine(CharacterRace race);
    static int getMaximumAge(CharacterRace race);
    static std::string getAgeBand(CharacterRace race, int age);

    // EN: getMerchantPurchasePricePercentage declares or implements a focused behavior used by this module.
    // FR: getMerchantPurchasePricePercentage déclare ou implémente un comportement précis utilisé par ce module.
    static int getMerchantPurchasePricePercentage(CharacterRace race);
    // EN: getMerchantSalePricePercentage declares or implements a focused behavior used by this module.
    // FR: getMerchantSalePricePercentage déclare ou implémente un comportement précis utilisé par ce module.
    static int getMerchantSalePricePercentage(CharacterRace race);
};

#endif
