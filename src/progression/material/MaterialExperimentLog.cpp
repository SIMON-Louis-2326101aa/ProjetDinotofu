// EN: MaterialExperimentLog.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: MaterialExperimentLog.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file belongs to Dinotofu. Code identifiers are written in English; player-facing text can stay in French.
// Français : Ce fichier appartient à Dinotofu. Les identifiants du code sont en anglais ; les textes affichés au joueur peuvent rester en français.
// Description: Runtime journal for craft/material experimentation.

#include "progression/material/MaterialExperimentLog.hpp"

#include <algorithm>

namespace
{
    std::vector<CraftExperimentRecord> craftRecords;
}

// EN: recordCraft declares or implements a focused behavior used by this module.
// FR: recordCraft déclare ou implémente un comportement précis utilisé par ce module.
void MaterialExperimentLog::recordCraft(const std::string& recipeName, int amount)
{
    if (recipeName.empty() || amount <= 0)
    {
        return;
    }

    for (CraftExperimentRecord& record : craftRecords)
    {
        if (record.recipeName == recipeName)
        {
            record.craftedCount += amount;
            return;
        }
    }

    // EN: craftRecords.push_back declares or implements a focused behavior used by this module.
    // FR: craftRecords.push_back déclare ou implémente un comportement précis utilisé par ce module.
    craftRecords.push_back({recipeName, amount});
}

// EN: getCraftRecords declares or implements a focused behavior used by this module.
// FR: getCraftRecords déclare ou implémente un comportement précis utilisé par ce module.
const std::vector<CraftExperimentRecord>& MaterialExperimentLog::getCraftRecords()
{
    return craftRecords;
}

// EN: getTotalCrafted declares or implements a focused behavior used by this module.
// FR: getTotalCrafted déclare ou implémente un comportement précis utilisé par ce module.
int MaterialExperimentLog::getTotalCrafted()
{
    int total = 0;

    for (const CraftExperimentRecord& record : craftRecords)
    {
        total += record.craftedCount;
    }

    return total;
}
