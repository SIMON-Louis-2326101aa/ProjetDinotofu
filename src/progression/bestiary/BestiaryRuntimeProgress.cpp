// EN: BestiaryRuntimeProgress.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: BestiaryRuntimeProgress.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.
// English: Implements a small in-session bestiary progression layer before full account persistence.
// Français : Implémente une petite couche de progression de bestiaire en session avant la persistance complète par compte.

#include "progression/bestiary/BestiaryRuntimeProgress.hpp"

#include <algorithm>
#include <sstream>

namespace
{
    std::vector<BestiaryRuntimeRecord>& records()
    {
        static std::vector<BestiaryRuntimeRecord> data;
        return data;
    }

    // EN: findRecord declares or implements a focused behavior used by this module.
    // FR: findRecord déclare ou implémente un comportement précis utilisé par ce module.
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

    std::string observationStatusFor(const BestiaryRuntimeRecord& record)
    {
        if (record.informationBought)
        {
            return "Renseignement acheté";
        }

        if (record.kills >= 10 || record.encounters >= 18)
        {
            return "Maîtrise de terrain";
        }

        if (record.kills >= 4 || record.encounters >= 8)
        {
            return "Étudié";
        }

        if (record.kills >= 1 || record.encounters >= 3)
        {
            return "Observé";
        }

        return "Rencontré";
    }

    std::string enrichDescriptionWithProgress(
        const BestiaryRuntimeRecord& record,
        const std::string& baseDescription
    )
    {
        std::ostringstream output;
        output << baseDescription;

        if (record.encounters >= 3 || record.kills >= 1)
        {
            output << " Observation : le comportement commence à être reconnaissable.";
        }

        if (record.encounters >= 8 || record.kills >= 4)
        {
            output << " Analyse : les réactions, faiblesses supposées et habitudes deviennent plus lisibles.";
        }

        if (record.encounters >= 18 || record.kills >= 10)
        {
            output << " Maîtrise : cette entrée est assez connue pour préparer ses rencontres avec beaucoup moins d'incertitude.";
        }

        return output.str();
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


// EN: clear declares or implements a focused behavior used by this module.
// FR: clear déclare ou implémente un comportement précis utilisé par ce module.
void BestiaryRuntimeProgress::clear()
{
    records().clear();
}

// EN: importRecord declares or implements a focused behavior used by this module.
// FR: importRecord déclare ou implémente un comportement précis utilisé par ce module.
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
        record.status = observationStatusFor(record);
        record.description = enrichDescriptionWithProgress(record, description.empty() ? record.description : description);
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
        record.status = observationStatusFor(record);
        record.description = enrichDescriptionWithProgress(record, description.empty() ? record.description : description);
    }
}

// EN: unlockCommonInformation declares or implements a focused behavior used by this module.
// FR: unlockCommonInformation déclare ou implémente un comportement précis utilisé par ce module.
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



    if (informationId == "class_identity_manual")
    {
        unlockEntry(
            "Différences de classes",
            "Races",
            "Manuel acheté : l'Assassin cherche saignement et précision, le Colosse encaisse et s'ancre, le Mage applique des statuts, le Rôdeur dépend vraiment d'une arme à distance, le Lancier profite de la portée, le Berserker devient dangereux blessé, et l'Artificier/Alchimiste transforme ses outils en effets instables."
        );
        unlockEntry(
            "Armes et classes",
            "Matériaux et plantes",
            "Note achetée : une classe à distance ne tire pas avec une épée. Les tirs spéciaux vérifient l'arme équipée. Les armes à distance gardent une défense d'urgence au petit couteau si les munitions compatibles manquent."
        );
        return;
    }

    if (informationId == "biome_field_notes")
    {
        unlockEntry(
            "Évolution des zones",
            "Divinités / lore",
            "Carnet lu : si le personnage dépasse trop le niveau maximum d'une zone, la zone attire des menaces plus fortes. L'entrée d'exploration signale les biomes déjà réajustés, et les récompenses suivent mieux le danger."
        );
        unlockEntry(
            "Biomes et présences locales",
            "Entités hostiles / ennemis",
            "Carnet lu : les monstres peuvent exister partout, mais chaque biome favorise certaines familles. Les slimes dominent les mares gélatineuses, le marais et les ruines deviennent plus dangereux, et les variantes rares apparaissent surtout là où le lieu leur donne une raison d'exister."
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
        return;
    }

    if (informationId == "special_adventurer_notes")
    {
        unlockEntry(
            "Groupes spéciaux",
            "Entités passives / alliées",
            "Notes achetées : certains groupes ne cherchent pas forcément la mort du joueur. Hazak protège Hestia, Fail respecte un contrat avec Hazak, et Skuro rend souvent le combat plus dangereux."
        );
        unlockEntry(
            "Mattzelda / Louis / Trexof",
            "Entités passives / alliées",
            "Notes achetées : trio de potes. Mattzelda encaisse, Louis improvise, Trexof analyse le combat comme un test d'équilibrage."
        );
        unlockEntry(
            "Aoi / Kanadé / Sanctus",
            "Entités passives / alliées",
            "Notes achetées : groupe proche. Sanctus protège, Aoi stabilise ses flammes, Kanadé déteste perdre et lance une magie très variable."
        );
        return;
    }

    if (informationId == "summoning_notes")
    {
        unlockEntry(
            "Invocations",
            "Invocations",
            "Manuel acheté : les invocations utilisent déjà des slots. Les futures versions ajouteront mana, évolution, sacrifices et ombres de Hazak."
        );
        unlockEntry(
            "Ombres de Hazak",
            "Invocations",
            "Indice acheté : Hazak pourra plus tard relever l'ombre d'un ennemi vaincu avec une partie de ses compétences d'origine."
        );
        return;
    }

    if (informationId == "boss_identity_scrap")
    {
        unlockEntry(
            "Identité des boss",
            "Boss",
            "Fragment acheté : un boss reste officiellement en ??? tant qu'il n'a pas révélé son nom par une entrée, une phrase à 50%, ou une phrase avant sa défaite."
        );
        unlockEntry(
            "Fragments de boss",
            "Matériaux et plantes",
            "Indice acheté : Fitoria peut laisser une plume lumineuse, Zelef du sang démoniaque, et Atlas une plaque brisée. Ces matériaux sont rares et liés aux futures reliques."
        );
        return;
    }

    if (informationId == "potion_recipe_page")
    {
        unlockEntry(
            "Recettes de potions simples",
            "Matériaux et plantes",
            "Page lue : 2 Feuilles amères + 1 Résidu de slime donnent une Potion de soin. 1 Fleur bleue + 1 Poussière arcanique donnent une Potion de soin renforcée. Les crocs, os et poussières alimentent les potions de rage."
        );
        return;
    }

    if (informationId == "repair_recipe_page")
    {
        unlockEntry(
            "Réparation de fortune",
            "Matériaux et plantes",
            "Page lue : 2 Fragments de métal rouillé, 1 Morceau de cuir abîmé et 1 Résidu de slime peuvent être assemblés en Kit de réparation faible. Le kit autorise la réparation autonome à +25% environ, coûte des matériaux, et perd 1 durabilité après chaque réparation."
        );
        unlockEntry(
            "Évolution des kits de réparation",
            "Matériaux et plantes",
            "Les kits progressent ainsi : faible +25%, moyen +50%, gros kit +75%, kit complet du bricoleur +95%. Les armes demandent surtout du métal ; les armures demandent surtout du cuir et parfois de la peau robuste. Chaque réparation consomme aussi 1 durabilité du kit. Le Forgeron ne répare pas magiquement sans forge, mais il a une chance de ne pas user le kit grâce à son savoir-faire."
        );
        return;
    }

    if (informationId == "advanced_monster_notes")
    {
        unlockEntry(
            "Loots de monstres avancés",
            "Entités hostiles / ennemis",
            "Notes lues : les humanoïdes peuvent laisser des insignes, les morts-vivants des os fissurés, les slimes des résidus et certains mages de la poussière arcanique."
        );
        unlockEntry(
            "Matériaux rares de race",
            "Matériaux et plantes",
            "Notes lues : certaines races ou profils spéciaux peuvent laisser des composants rares : peau robuste, fil d'ombre, braise kitsune, écaille draconique ou noyau instable."
        );
        unlockEntry(
            "Fragments de boss",
            "Matériaux et plantes",
            "Notes lues : les boss vaincus peuvent laisser un fragment unique. Ces fragments ne servent pas encore à une relique complète, mais sont déjà utilisables comme base de craft rare."
        );
        unlockEntry(
            "Variantes de slimes",
            "Entités hostiles / ennemis",
            "Notes lues : les mares gélatineuses peuvent contenir presque toutes les couleurs connues. Vert = classique, rouge = brûlure, violet/noir = poison, bleu/blanc = froid, jaune = choc, ambré = colle/ralentit, rose = rebonds imprévisibles, chromatique/prisme/miroir = réactions instables. Certaines couleurs apparaissent aussi ailleurs si le lieu les attire."
        );
        unlockEntry(
            "Monstres soigneurs",
            "Entités hostiles / ennemis",
            "Notes lues : toutes les créatures ne savent pas soigner. Un loup ou un slime classique n'utilise pas une potion. Les soigneurs crédibles sont des profils précis : shaman, chamane, oracle, mage ou créature spéciale avec une vraie raison lore."
        );
        unlockEntry(
            "Affinités élémentaires faibles",
            "Entités hostiles / ennemis",
            "Notes lues : certains monstres peuvent déclencher brûlure, poison, givre ou choc. L'électricité devient plus dangereuse contre les armes et armures métalliques. Les potions anti-statut existent maintenant : antidote, baume anti-brûlure, anti-givre et isolante."
        );
        return;
    }

    if (informationId == "slime_color_codex")
    {
        unlockEntry(
            "Codex des slimes colorés",
            "Entités hostiles / ennemis",
            "Codex acheté : vert = base stable, bleu/blanc = ralentissement froid, rouge = brûlure, violet/noir/putride = poison, jaune/orage = choc, ambré = colle, rose = rebond, doré = attiré par ce qui brille, chromatique/prisme = réaction instable. Les mares gélatineuses restent leur vrai terrain de diversité."
        );
        unlockEntry(
            "Slime doré et objets brillants",
            "Entités hostiles / ennemis",
            "Note achetée : les slimes dorés ne sont pas forcément intelligents, mais ils peuvent avaler des pièces ou objets brillants. C'est une justification rare, pas une source fiable d'or."
        );
        return;
    }

    if (informationId == "monster_family_evolution_notes")
    {
        unlockEntry(
            "Évolutions gobelines",
            "Entités hostiles / ennemis",
            "Dossier acheté : gobelin ramasseur, frondeur, rapiéceur, shaman, grand shaman et hobgobelin n'ont pas le même rôle. Les shamans peuvent soigner car ils ont une raison lore ; le gobelin basique ne sait pas devenir médecin par miracle."
        );
        unlockEntry(
            "Familles de monstres",
            "Entités hostiles / ennemis",
            "Dossier acheté : les loups doivent rester prédateurs, les morts-vivants tenaces ou froids, les plantes entravantes, les insectes toxiques/rapides, les constructions défensives et les anomalies imprévisibles."
        );
        return;
    }

    if (informationId == "weapon_training_notes")
    {
        unlockEntry(
            "Techniques débloquables",
            "Races",
            "Manuel acheté : les techniques doivent venir de la classe, de l'arme équipée, du niveau et de l'expérimentation. À partir des premiers niveaux, certaines classes déclenchent déjà des passifs plus visibles : saignement, garde, soutien sacré, instabilité magique ou rage blessée."
        );
        unlockEntry(
            "Apprentissage par usage",
            "Matériaux et plantes",
            "Note achetée : plus tard, utiliser souvent une arme ou crafter des munitions pourra apprendre des recettes/gestes. Pour l'instant, les paliers de niveau préparent cette logique sans bloquer le jeu."
        );
        return;
    }

    if (informationId == "necromancy_warning")
    {
        unlockEntry(
            "Nécromancie instable",
            "Invocations",
            "Avertissement lu : relever une ombre ne devra pas être gratuit. Les os, le mana, les morts récents et la stabilité mentale du personnage pourront compter plus tard."
        );
        return;
    }

}

// EN: getEncounterCount declares or implements a focused behavior used by this module.
// FR: getEncounterCount déclare ou implémente un comportement précis utilisé par ce module.
int BestiaryRuntimeProgress::getEncounterCount(const std::string& name)
{
    BestiaryRuntimeRecord* record = findRecord(name);
    return record == nullptr ? 0 : record->encounters;
}

// EN: getKillCount declares or implements a focused behavior used by this module.
// FR: getKillCount déclare ou implémente un comportement précis utilisé par ce module.
int BestiaryRuntimeProgress::getKillCount(const std::string& name)
{
    BestiaryRuntimeRecord* record = findRecord(name);
    return record == nullptr ? 0 : record->kills;
}

// EN: hasBoughtInformation declares or implements a focused behavior used by this module.
// FR: hasBoughtInformation déclare ou implémente un comportement précis utilisé par ce module.
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
