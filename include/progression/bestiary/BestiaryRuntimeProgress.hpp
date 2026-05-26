// EN: BestiaryRuntimeProgress.hpp briefly defines this Dinotofu module and its responsibilities.
// FR: BestiaryRuntimeProgress.hpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.
// English: Stores the first runtime bestiary progression used by fights, loot and library information.
// Français : Stocke la première progression de bestiaire en session utilisée par les combats, les loots et la bibliothèque.

#ifndef INCLUDE_PROGRESSION_BESTIARY_BESTIARYRUNTIMEPROGRESS_HPP
#define INCLUDE_PROGRESSION_BESTIARY_BESTIARYRUNTIMEPROGRESS_HPP

#include <string>
#include <vector>

struct BestiaryRuntimeRecord
{
    std::string category;
    std::string name;
    std::string description;
    std::string status;
    std::string habitat;
    std::string weaknesses;
    std::string resistances;
    std::string drops;
    std::string strategy;
    std::string dangerRank;
    int encounters;
    int kills;
    bool informationBought;
};

class BestiaryRuntimeProgress
{
public:
    static void recordEncounter(
        const std::string& name,
        const std::string& category,
        const std::string& description
    );

    static void recordKill(
        const std::string& name,
        const std::string& category,
        const std::string& description
    );

    // EN: unlockCommonInformation declares or implements a focused behavior used by this module.
    // FR: unlockCommonInformation déclare ou implémente un comportement précis utilisé par ce module.
    static void unlockCommonInformation(const std::string& informationId);

    // EN: clear declares or implements a focused behavior used by this module.
    // FR: clear déclare ou implémente un comportement précis utilisé par ce module.
    static void clear();
    // EN: importRecord declares or implements a focused behavior used by this module.
    // FR: importRecord déclare ou implémente un comportement précis utilisé par ce module.
    static void importRecord(const BestiaryRuntimeRecord& record);

    // EN: getEncounterCount declares or implements a focused behavior used by this module.
    // FR: getEncounterCount déclare ou implémente un comportement précis utilisé par ce module.
    static int getEncounterCount(const std::string& name);
    // EN: getKillCount declares or implements a focused behavior used by this module.
    // FR: getKillCount déclare ou implémente un comportement précis utilisé par ce module.
    static int getKillCount(const std::string& name);
    // EN: hasBoughtInformation declares or implements a focused behavior used by this module.
    // FR: hasBoughtInformation déclare ou implémente un comportement précis utilisé par ce module.
    static bool hasBoughtInformation(const std::string& name);
    // EN: getStatusFor declares or implements a focused behavior used by this module.
    // FR: getStatusFor déclare ou implémente un comportement précis utilisé par ce module.
    static std::string getStatusFor(const std::string& name, const std::string& fallbackStatus);

    // EN: getRecords declares or implements a focused behavior used by this module.
    // FR: getRecords déclare ou implémente un comportement précis utilisé par ce module.
    static std::vector<BestiaryRuntimeRecord> getRecords();
};

#endif
