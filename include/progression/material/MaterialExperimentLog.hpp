// EN: MaterialExperimentLog.hpp briefly defines this Dinotofu module and its responsibilities.
// FR: MaterialExperimentLog.hpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file belongs to Dinotofu. Code identifiers are written in English; player-facing text can stay in French.
// Français : Ce fichier appartient à Dinotofu. Les identifiants du code sont en anglais ; les textes affichés au joueur peuvent rester en français.
// Description: Runtime journal for craft/material experimentation.

#ifndef INCLUDE_PROGRESSION_MATERIAL_MATERIALEXPERIMENTLOG_HPP
#define INCLUDE_PROGRESSION_MATERIAL_MATERIALEXPERIMENTLOG_HPP

#include <string>
#include <vector>

struct CraftExperimentRecord
{
    std::string recipeName;
    int craftedCount;
};

class MaterialExperimentLog
{
public:
    // EN: recordCraft declares or implements a focused behavior used by this module.
    // FR: recordCraft déclare ou implémente un comportement précis utilisé par ce module.
    static void recordCraft(const std::string& recipeName, int amount);
    // EN: getCraftRecords declares or implements a focused behavior used by this module.
    // FR: getCraftRecords déclare ou implémente un comportement précis utilisé par ce module.
    static const std::vector<CraftExperimentRecord>& getCraftRecords();
    // EN: getTotalCrafted declares or implements a focused behavior used by this module.
    // FR: getTotalCrafted déclare ou implémente un comportement précis utilisé par ce module.
    static int getTotalCrafted();
};

#endif
