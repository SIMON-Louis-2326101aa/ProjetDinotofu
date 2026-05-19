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

    static void unlockCommonInformation(const std::string& informationId);

    static void clear();
    static void importRecord(const BestiaryRuntimeRecord& record);

    static int getEncounterCount(const std::string& name);
    static int getKillCount(const std::string& name);
    static bool hasBoughtInformation(const std::string& name);
    static std::string getStatusFor(const std::string& name, const std::string& fallbackStatus);

    static std::vector<BestiaryRuntimeRecord> getRecords();
};

#endif
