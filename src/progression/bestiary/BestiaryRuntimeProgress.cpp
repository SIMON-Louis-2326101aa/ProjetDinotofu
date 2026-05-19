// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.
// English: Implements a small in-session bestiary progression layer before full account persistence.
// Français : Implémente une petite couche de progression de bestiaire en session avant la persistance complète par compte.

#include "progression/bestiary/BestiaryRuntimeProgress.hpp"

#include <algorithm>

namespace
{
    std::vector<BestiaryRuntimeRecord>& records()
    {
        static std::vector<BestiaryRuntimeRecord> data;
        return data;
    }

    BestiaryRuntimeRecord* findRecord(const std::string& name)
    {
        for (BestiaryRuntimeRecord& record : records())
        {
            if (record.name == name)
            {
                return &record;
            }
        }

        return nullptr;
    }

    BestiaryRuntimeRecord& ensureRecord(
        const std::string& name,
        const std::string& category,
        const std::string& description
    )
    {
        BestiaryRuntimeRecord* existingRecord = findRecord(name);

        if (existingRecord != nullptr)
        {
            if (!category.empty())
            {
                existingRecord->category = category;
            }

            if (!description.empty())
            {
                existingRecord->description = description;
            }

            return *existingRecord;
        }

        BestiaryRuntimeRecord record;
        record.category = category.empty() ? "Entités hostiles / ennemis" : category;
        record.name = name;
        record.description = description.empty()
            ? "Entrée découverte pendant cette session. Les détails complets arriveront avec la sauvegarde avancée du bestiaire."
            : description;
        record.status = "Rencontré";
        record.encounters = 0;
        record.kills = 0;
        record.informationBought = false;

        records().push_back(record);
        return records().back();
    }

    void unlockEntry(
        const std::string& name,
        const std::string& category,
        const std::string& description
    )
    {
        BestiaryRuntimeRecord& record = ensureRecord(name, category, description);
        record.informationBought = true;
        record.status = "Renseignement acheté";

        if (record.encounters <= 0)
        {
            record.encounters = 0;
        }
    }
}


void BestiaryRuntimeProgress::clear()
{
    records().clear();
}

void BestiaryRuntimeProgress::importRecord(const BestiaryRuntimeRecord& importedRecord)
{
    if (importedRecord.name.empty())
    {
        return;
    }

    BestiaryRuntimeRecord& record = ensureRecord(
        importedRecord.name,
        importedRecord.category,
        importedRecord.description
    );

    record.status = importedRecord.status.empty() ? record.status : importedRecord.status;
    record.encounters = importedRecord.encounters;
    record.kills = importedRecord.kills;
    record.informationBought = importedRecord.informationBought;
}

void BestiaryRuntimeProgress::recordEncounter(
    const std::string& name,
    const std::string& category,
    const std::string& description
)
{
    if (name.empty())
    {
        return;
    }

    BestiaryRuntimeRecord& record = ensureRecord(name, category, description);
    record.encounters++;

    if (!record.informationBought)
    {
        record.status = "Rencontré";
    }
}

void BestiaryRuntimeProgress::recordKill(
    const std::string& name,
    const std::string& category,
    const std::string& description
)
{
    if (name.empty())
    {
        return;
    }

    BestiaryRuntimeRecord& record = ensureRecord(name, category, description);
    record.kills++;

    if (!record.informationBought)
    {
        record.status = "Combattu";
    }
}

void BestiaryRuntimeProgress::unlockCommonInformation(const std::string& informationId)
{
    if (informationId == "common_goblin_notes")
    {
        unlockEntry(
            "Gobelin peureux",
            "Entités hostiles / ennemis",
            "Notes achetées : les gobelins sont petits, opportunistes et plus dangereux en groupe qu'en duel. Certains fuient si le combat tourne mal."
        );
        unlockEntry(
            "Gobelin brutal",
            "Entités hostiles / ennemis",
            "Notes achetées : un gobelin brutal tape plus fort, mais garde l'instinct de survie lâche de son espèce."
        );
        unlockEntry(
            "Gobelin pillard",
            "Entités hostiles / ennemis",
            "Notes achetées : les pillards aiment les objets brillants. Ils sont de futurs candidats logiques au vol après une mort non définitive."
        );
        return;
    }

    if (informationId == "common_wolf_notes")
    {
        unlockEntry(
            "Loup affamé",
            "Entités hostiles / ennemis",
            "Notes achetées : le loup cible naturellement les proies fragiles ou blessées. Plus tard, son instinct pourra interagir avec la menace."
        );
        unlockEntry(
            "Sanglier sauvage",
            "Entités hostiles / ennemis",
            "Notes achetées : le sanglier encaisse mieux qu'il n'en a l'air et punit les joueurs trop confiants."
        );
        return;
    }

    if (informationId == "basic_plant_manual")
    {
        unlockEntry(
            "Fleur bleue de montagne",
            "Matériaux et plantes",
            "Guide acheté : plante rare et calme, inspirée de la fleur bleue de Zelda BOTW. Elle servira aux remèdes, quêtes et secrets botaniques."
        );
        unlockEntry(
            "Feuille amère de soin",
            "Matériaux et plantes",
            "Guide acheté : plante basique pouvant entrer dans de futures recettes de soin."
        );
        return;
    }

    if (informationId == "basic_magic_manual")
    {
        unlockEntry(
            "Magie basique",
            "Races",
            "Manuel acheté : la magie de base dépendra plus tard de l'intelligence, de la sagesse, du mana et parfois de la race."
        );
    }
}

int BestiaryRuntimeProgress::getEncounterCount(const std::string& name)
{
    BestiaryRuntimeRecord* record = findRecord(name);
    return record == nullptr ? 0 : record->encounters;
}

int BestiaryRuntimeProgress::getKillCount(const std::string& name)
{
    BestiaryRuntimeRecord* record = findRecord(name);
    return record == nullptr ? 0 : record->kills;
}

bool BestiaryRuntimeProgress::hasBoughtInformation(const std::string& name)
{
    BestiaryRuntimeRecord* record = findRecord(name);
    return record != nullptr && record->informationBought;
}

std::string BestiaryRuntimeProgress::getStatusFor(
    const std::string& name,
    const std::string& fallbackStatus
)
{
    BestiaryRuntimeRecord* record = findRecord(name);

    if (record == nullptr)
    {
        return fallbackStatus;
    }

    return record->status;
}

std::vector<BestiaryRuntimeRecord> BestiaryRuntimeProgress::getRecords()
{
    std::vector<BestiaryRuntimeRecord> copy = records();

    std::sort(copy.begin(), copy.end(), [](const BestiaryRuntimeRecord& a, const BestiaryRuntimeRecord& b) {
        if (a.category == b.category)
        {
            return a.name < b.name;
        }

        return a.category < b.category;
    });

    return copy;
}
