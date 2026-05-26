// EN: MaterialKnowledgeProgress.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: MaterialKnowledgeProgress.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file belongs to Dinotofu. Code identifiers are written in English; player-facing text can stay in French.
// Français : Ce fichier appartient à Dinotofu. Les identifiants du code sont en anglais ; les textes affichés au joueur peuvent rester en français.
// Description: Tracks material discoveries and feeds the bestiary material journal.

#include "progression/material/MaterialKnowledgeProgress.hpp"

#include "progression/bestiary/BestiaryRuntimeProgress.hpp"

#include <algorithm>

namespace
{
    std::vector<MaterialKnowledgeRecord>& records()
    {
        static std::vector<MaterialKnowledgeRecord> data;
        return data;
    }

    // EN: findRecord declares or implements a focused behavior used by this module.
    // FR: findRecord déclare ou implémente un comportement précis utilisé par ce module.
    MaterialKnowledgeRecord* findRecord(const Material& material)
    {
        for (MaterialKnowledgeRecord& record : records())
        {
            if (record.id == material.getId()
                && record.quality == material.getQuality())
            {
                return &record;
            }
        }

        return nullptr;
    }

    std::string describeMaterialDiscovery(const Material& material)
    {
        std::string description = "Découverte réelle : " + material.getName();
        description += " appartient à la catégorie " + material.getCategory() + ".";

        if (!material.getQuality().empty() && material.getQuality() != "normal")
        {
            description += " Qualité observée : " + material.getQuality() + ".";
        }
        else
        {
            description += " Qualité observée : normale.";
        }

        description += " Valeur unitaire actuelle : " + std::to_string(material.getValue()) + " pièce(s).";

        if (material.getQualityCraftWeight() >= 4)
        {
            description += " Ce composant réagit fortement aux crafts à particularité.";
        }
        else if (material.getQualityCraftWeight() <= 1)
        {
            description += " Sa qualité faible ou impure réduira l'efficacité de certains crafts.";
        }

        return description;
    }
}

// EN: clear declares or implements a focused behavior used by this module.
// FR: clear déclare ou implémente un comportement précis utilisé par ce module.
void MaterialKnowledgeProgress::clear()
{
    records().clear();
}

// EN: recordDiscovery declares or implements a focused behavior used by this module.
// FR: recordDiscovery déclare ou implémente un comportement précis utilisé par ce module.
void MaterialKnowledgeProgress::recordDiscovery(const Material& material)
{
    if (material.getId().empty() || material.getQuantity() <= 0)
    {
        return;
    }

    MaterialKnowledgeRecord* existing = findRecord(material);

    if (existing != nullptr)
    {
        existing->discoveredQuantity += material.getQuantity();
        existing->bestQualityWeight = std::max(existing->bestQualityWeight, material.getQualityCraftWeight());
    }
    else
    {
        MaterialKnowledgeRecord record;
        record.id = material.getId();
        record.name = material.getName();
        record.category = material.getCategory();
        record.quality = material.getQuality();
        record.discoveredQuantity = material.getQuantity();
        record.bestQualityWeight = material.getQualityCraftWeight();
        records().push_back(record);
    }

    BestiaryRuntimeProgress::recordEncounter(
        material.getName(),
        "Matériaux et plantes",
        describeMaterialDiscovery(material)
    );
}

std::vector<MaterialKnowledgeRecord> MaterialKnowledgeProgress::getRecords()
{
    std::vector<MaterialKnowledgeRecord> copy = records();

    std::sort(copy.begin(), copy.end(), [](const MaterialKnowledgeRecord& a, const MaterialKnowledgeRecord& b) {
        if (a.category == b.category)
        {
            return a.name < b.name;
        }

        return a.category < b.category;
    });

    return copy;
}
