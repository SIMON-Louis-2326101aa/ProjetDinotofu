// EN: BestiaryRuntimeProgress.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: BestiaryRuntimeProgress.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.
// English: Implements a small in-session bestiary progression layer before full account persistence.
// Français : Implémente une petite couche de progression de bestiaire en session avant la persistance complète par compte.

#include "progression/bestiary/BestiaryRuntimeProgress.hpp"

#include <algorithm>
#include <cctype>
#include <sstream>

namespace
{
    std::vector<BestiaryRuntimeRecord>& records()
    {
        static std::vector<BestiaryRuntimeRecord> data;
        return data;
    }

    std::string lowerCopy(const std::string& value)
    {
        std::string output = value;

        std::transform(output.begin(), output.end(), output.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });

        return output;
    }

    void fillIfEmpty(std::string& target, const std::string& value)
    {
        if (target.empty())
        {
            target = value;
        }
    }

    void applyAutomaticHints(BestiaryRuntimeRecord& record)
    {
        const std::string probe = lowerCopy(record.name + " " + record.category + " " + record.description);

        fillIfEmpty(record.dangerRank, "Variable");
        fillIfEmpty(record.habitat, "Habitat encore mal confirmé.");
        fillIfEmpty(record.weaknesses, "Faiblesses non confirmées.");
        fillIfEmpty(record.resistances, "Résistances non confirmées.");
        fillIfEmpty(record.drops, "Butin à confirmer par observation ou dépeçage.");
        fillIfEmpty(record.strategy, "Observer deux ou trois tours avant de conclure.");

        if (probe.find("slime") != std::string::npos || probe.find("gel") != std::string::npos)
        {
            record.dangerRank = "Faible à instable selon couleur";
            record.habitat = "Mares gélatineuses, grottes humides, ruines chargées de résidus.";
            record.weaknesses = "Souvent sensible aux impacts nets, au froid ou au feu selon couleur.";
            record.resistances = "Peu sensible au saignement ; poison souvent peu fiable sur gelées simples.";
            record.drops = "Résidus de slime, gel coloré, noyau instable pour les variantes rares.";
            record.strategy = "Identifier la couleur avant de gaspiller un élément : chaque gelée ne réagit pas pareil.";
        }
        else if (probe.find("gobelin") != std::string::npos || probe.find("hobgobelin") != std::string::npos)
        {
            record.dangerRank = "Faible seul, dangereux en bande";
            record.habitat = "Routes, caches de pillards, campements de fortune, grottes proches des villes.";
            record.weaknesses = "Panique, dégâts de zone, focus rapide du chef ou du shaman.";
            record.resistances = "Aucune résistance naturelle majeure, mais les chefs compensent par équipement.";
            record.drops = "Oreilles de gobelin, insignes, pièces volées, bouts de cuir ou métal médiocre.";
            record.strategy = "Couper le soutien d'abord : shaman, frondeur ou chef de sacs changent tout le rythme.";
        }
        else if (probe.find("loup") != std::string::npos || probe.find("bête") != std::string::npos || probe.find("sanglier") != std::string::npos)
        {
            record.dangerRank = "Faible à moyen";
            record.habitat = "Plaines, forêts, routes isolées, lisières proches de proies faciles.";
            record.weaknesses = "Entraves, feu maîtrisé, contre-attaques après charge.";
            record.resistances = "Aucune grande résistance magique ; l'instinct réduit parfois les pièges grossiers.";
            record.drops = "Peau, crocs, viande, fourrure ou cuir robuste selon qualité de la mise à mort.";
            record.strategy = "Éviter l'isolement : les prédateurs punissent surtout une cible blessée ou seule.";
        }
        else if (probe.find("mort") != std::string::npos || probe.find("squelette") != std::string::npos || probe.find("osseux") != std::string::npos || probe.find("ombre") != std::string::npos)
        {
            record.dangerRank = "Moyen, parfois trompeur";
            record.habitat = "Cimetières, ruines, lieux froids ou chargés de mana mort.";
            record.weaknesses = "Lumière, feu, coups lourds contre les os, purification selon origine.";
            record.resistances = "Poison et saignement souvent faibles ; peur rarement efficace.";
            record.drops = "Os fissurés, fil d'ombre, poussière funéraire, fragments de sceau.";
            record.strategy = "Ne pas compter sur le poison : chercher rupture, lumière ou contrôle du terrain.";
        }
        else if (probe.find("plante") != std::string::npos || probe.find("racine") != std::string::npos || probe.find("ronce") != std::string::npos)
        {
            record.dangerRank = "Moyen si la zone est serrée";
            record.habitat = "Forêts anciennes, marais, ruines recouvertes, sols riches en mana vert.";
            record.weaknesses = "Feu, coupe nette, gel sur certaines fibres humides.";
            record.resistances = "Poison souvent peu utile ; entraves naturelles difficiles à retourner contre elles.";
            record.drops = "Fibres végétales, sève, graines rares, ronces occultes si magie sombre.";
            record.strategy = "Briser l'entrave avant de chercher le burst : une plante gagne quand tu ne bouges plus.";
        }
        else if (probe.find("bandit") != std::string::npos || probe.find("alchimiste") != std::string::npos || probe.find("mage renégat") != std::string::npos || probe.find("chasseur de primes") != std::string::npos)
        {
            record.dangerRank = "Variable selon équipement";
            record.habitat = "Routes, camps, ruelles, guildes douteuses et contrats mal tournés.";
            record.weaknesses = "Pression sur les soutiens, interruption des fioles, désarmement ou contrôle de distance.";
            record.resistances = "Pas de résistance naturelle fiable ; potions et armures changent tout.";
            record.drops = "Insignes abîmés, cuir, fioles, poussière arcanique ou pièces volées.";
            record.strategy = "Lire le rôle avant la cible : apothicaire, mage et chef ne doivent pas être laissés libres.";
        }
        else if (probe.find("orc") != std::string::npos || probe.find("berserker") != std::string::npos || probe.find("colosse") != std::string::npos)
        {
            record.dangerRank = "Moyen à lourd";
            record.habitat = "Campements robustes, zones de chasse, arènes et passages disputés.";
            record.weaknesses = "Entrave, allonge, brise-garde et contrôle du rythme.";
            record.resistances = "Bonne résistance physique ; fuite plus difficile au contact.";
            record.drops = "Cuir épais, métal cabossé, trophées et matériaux de réparation.";
            record.strategy = "Ne pas échanger coup pour coup sans garde : le rythme lent cache souvent un gros impact.";
        }
        else if (probe.find("araignée") != std::string::npos || probe.find("insect") != std::string::npos || probe.find("venimeuse") != std::string::npos)
        {
            record.dangerRank = "Faible à dangereux si poison accumulé";
            record.habitat = "Grottes, forêts humides, marais et coins non entretenus.";
            record.weaknesses = "Feu contrôlé, attaques de zone, antidote ou purification.";
            record.resistances = "Bon instinct d'esquive ; poison moins fiable sur variantes déjà toxiques.";
            record.drops = "Venin, soie, chitine légère, mandibules.";
            record.strategy = "Traiter le poison tôt ou finir vite avant que les dégâts de durée décident du combat.";
        }
        else if (probe.find("dragon") != std::string::npos || probe.find("draconide") != std::string::npos || probe.find("kobold") != std::string::npos)
        {
            record.dangerRank = "Élevé quand l'âge augmente";
            record.habitat = "Nids rocheux, montagnes, anciennes ruines et territoires gardés.";
            record.weaknesses = "Faille de résistance, contrôle du souffle, élément opposé selon lignée.";
            record.resistances = "Écailles, souffle et résistance naturelle supérieure à la moyenne.";
            record.drops = "Écailles, braises, griffes, fragments draconiques.";
            record.strategy = "Préparer le combat et éviter de gâcher les matériaux par une mise à mort trop brutale.";
        }
        else if (probe.find("construction") != std::string::npos || probe.find("armure") != std::string::npos || probe.find("sentinelle") != std::string::npos || probe.find("gardien ancien") != std::string::npos)
        {
            record.dangerRank = "Lourd et défensif";
            record.habitat = "Ruines, portes scellées, salles de garde et anciens ateliers.";
            record.weaknesses = "Choc ciblé, marteau, fissures, corrosion ou brise-garde.";
            record.resistances = "Poison et saignement quasi inutiles ; très bonne résistance aux petits coups.";
            record.drops = "Fragments de métal, plaques, noyaux, anciennes pièces de mécanisme.";
            record.strategy = "Chercher les fissures plutôt que de multiplier les entailles faibles.";
        }
        else if (probe.find("élémentaire") != std::string::npos || probe.find("méphaïte") != std::string::npos || probe.find("feu") != std::string::npos || probe.find("givre") != std::string::npos || probe.find("choc") != std::string::npos || probe.find("anomalie") != std::string::npos)
        {
            record.dangerRank = "Variable, élément dépendant";
            record.habitat = "Zones extrêmes, failles magiques, lieux touchés par un élément dominant.";
            record.weaknesses = "Élément opposé ou rupture de catalyse selon la forme.";
            record.resistances = "Très résistant à son propre élément ; immunités possibles sur formes pures.";
            record.drops = "Poussière arcanique, éclat élémentaire, noyau chargé.";
            record.strategy = "Lire l'élément avant d'attaquer : frapper feu contre feu est souvent une erreur.";
        }
        else if (probe.find("boss") != std::string::npos || record.category == "Boss")
        {
            record.dangerRank = "Boss";
            record.habitat = "Arène, lieu scellé ou territoire marqué par sa légende.";
            record.weaknesses = "Failles spécifiques à découvrir par phases, dialogues ou registre.";
            record.resistances = "Résistances élevées ; les ultimes changent souvent les règles à 50% PV.";
            record.drops = "Fragments, invitations, matériaux uniques ou traces de boss selon révélation.";
            record.strategy = "Lire l'estimation de puissance et garder des ressources : la fuite ordinaire ne résout rien face à un boss.";
        }
        else if (record.category == "Matériaux et plantes")
        {
            record.dangerRank = "Ressource";
            record.habitat = "Dépend de la zone, de la famille de monstre ou de la qualité de récolte.";
            record.weaknesses = "Peut être abîmé par feu, explosions, armes lourdes ou récolte brutale.";
            record.resistances = "Une haute qualité garde mieux ses propriétés en craft.";
            record.drops = "Quantité et qualité dépendent de l'action, du monstre et parfois de la chance.";
            record.strategy = "Chercher la récolte propre si l'objet doit servir au craft plutôt qu'à la vente.";
        }
        else if (record.category == "Habitats / zones")
        {
            record.dangerRank = "Zone";
            record.habitat = "Lieu d'exploration avec familles dominantes, ressources et variantes rares.";
            record.weaknesses = "La préparation dépend du biome : antidotes, anti-givre, arme lourde, feu ou voile élémentaire.";
            record.resistances = "Une zone ne résiste pas, mais elle favorise certains dangers et pièges.";
            record.drops = "Ressources locales, coffres, traces, contrats et matériaux liés aux familles présentes.";
            record.strategy = "Préparer consommables et équipement selon les familles visibles avant d'entrer.";
        }
        else if (record.category == "Objets rares")
        {
            record.dangerRank = "Objet";
            record.habitat = "Bibliothèque, marché noir, coffre rare, boss ou récompense spéciale.";
            record.weaknesses = "Usage limité, instabilité ou prérequis de maîtrise.";
            record.resistances = "Un objet rare garde sa valeur mais peut exiger le bon contexte.";
            record.drops = "Obtention par achat, troc, boss, exploration ou secret.";
            record.strategy = "Ne pas consommer un objet rare sans savoir s'il est apprenable, jetable ou unique.";
        }
        else if (record.category == "Légendes / contes")
        {
            record.dangerRank = "Lore optionnel";
            record.habitat = "Bibliothèque, PNJ conteur, salle ancienne, rumeur ou déclencheur rare.";
            record.weaknesses = "Aucune faiblesse : ce contenu ne doit pas bloquer la progression.";
            record.resistances = "Doit rester rare, lisible et non répétitif.";
            record.drops = "Fragments de lore, rumeurs, pistes ou ambiance.";
            record.strategy = "Lire quand l'envie est là ; ignorer sans perdre la jouabilité principale.";
        }
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

            applyAutomaticHints(*existingRecord);

            return *existingRecord;
        }

        BestiaryRuntimeRecord record;
        record.category = category.empty() ? "Entités hostiles / ennemis" : category;
        record.name = name;
        record.description = description.empty()
            ? "Entrée découverte pendant cette session. Les détails complets arriveront avec la sauvegarde avancée du bestiaire."
            : description;
        record.status = "Rencontré";
        record.habitat = "";
        record.weaknesses = "";
        record.resistances = "";
        record.drops = "";
        record.strategy = "";
        record.dangerRank = "";
        record.encounters = 0;
        record.kills = 0;
        record.informationBought = false;

        applyAutomaticHints(record);

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

    void appendIfMissing(std::ostringstream& output, const std::string& baseDescription, const std::string& sentence)
    {
        if (baseDescription.find(sentence) == std::string::npos)
        {
            output << " " << sentence;
        }
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
            appendIfMissing(output, baseDescription, "Observation : le comportement commence à être reconnaissable.");
        }

        if (record.encounters >= 8 || record.kills >= 4)
        {
            appendIfMissing(output, baseDescription, "Analyse : les réactions, faiblesses supposées et habitudes deviennent plus lisibles.");
        }

        if (record.encounters >= 18 || record.kills >= 10)
        {
            appendIfMissing(output, baseDescription, "Maîtrise : cette entrée est assez connue pour préparer ses rencontres avec beaucoup moins d'incertitude.");
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
    record.habitat = importedRecord.habitat.empty() ? record.habitat : importedRecord.habitat;
    record.weaknesses = importedRecord.weaknesses.empty() ? record.weaknesses : importedRecord.weaknesses;
    record.resistances = importedRecord.resistances.empty() ? record.resistances : importedRecord.resistances;
    record.drops = importedRecord.drops.empty() ? record.drops : importedRecord.drops;
    record.strategy = importedRecord.strategy.empty() ? record.strategy : importedRecord.strategy;
    record.dangerRank = importedRecord.dangerRank.empty() ? record.dangerRank : importedRecord.dangerRank;
    record.encounters = importedRecord.encounters;
    record.kills = importedRecord.kills;
    record.informationBought = importedRecord.informationBought;

    applyAutomaticHints(record);
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
        applyAutomaticHints(record);
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
        applyAutomaticHints(record);
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
            "Notes achetées : les pillards aiment les objets brillants. Ils surveillent les corps tombés et les sacs mal fermés."
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
            "Guide acheté : plante basique souvent utilisée dans des remèdes simples et des recettes de soin."
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
            "Note achetée : une classe à distance ne tire pas avec une épée. Les tirs spéciaux vérifient l'arme équipée. Une bonne arme pour une bonne classe donne maintenant un très léger bonus de maîtrise ; une arme incohérente ne reçoit pas ce coup de pouce."
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
        unlockEntry(
            "Plaine sauvage",
            "Habitats / zones",
            "Carnet lu : zone ouverte de début, avec loups, sangliers, rats, petits gobelins, slimes faibles, plantes sèches et quelques coffres visibles. Les récompenses doivent rester modestes sauf événement ou risque réel."
        );
        unlockEntry(
            "Route commerciale",
            "Habitats / zones",
            "Carnet lu : route de passage où les humanoïdes, bandits, gobelins organisés, pilleurs et clients de guilde sont plus crédibles. Les soins ennemis y viennent plutôt de profils préparés."
        );
        unlockEntry(
            "Mares gélatineuses",
            "Habitats / zones",
            "Carnet lu : terrain naturel des slimes colorés. Le danger vient moins du nom slime que de la couleur, de la rareté et des réactions aux éléments."
        );
        unlockEntry(
            "Cimetière oublié",
            "Habitats / zones",
            "Carnet lu : morts-vivants, esprits, goules, oracles et ombres rares. Poison et saignement sont moins fiables ; lumière, feu, purification et coups lourds montent en valeur."
        );
        unlockEntry(
            "Ruines effondrées",
            "Habitats / zones",
            "Carnet lu : ruines instables avec constructions, armures mortes, anomalies, squelettes arcanisés et poussière arcanique. Les réflexes simples y mentent souvent."
        );
        return;
    }

    if (informationId == "basic_magic_manual")
    {
        unlockEntry(
            "Magie basique",
            "Races",
            "Manuel acheté : la magie de base écoute l'intelligence, la sagesse, le mana et parfois le sang de celui qui l'appelle."
        );
        return;
    }

    if (informationId == "magic_learning_notes")
    {
        unlockEntry(
            "Supports magiques",
            "Objets rares",
            "Notes achetées : un parchemin est un sort consommable, utilisable aussi par un mage, mais perdu après usage. Un grimoire sert à apprendre durablement un sort, avec classe compatible, niveau, catalyseur et limites. Certains sorts n'existent qu'en grimoire, d'autres seulement en parchemin."
        );
        unlockEntry(
            "Catalyseurs de mage",
            "Objets rares",
            "Notes achetées : bâton, baguette, pierre ou catalyseur stable améliorent la réussite. Un catalyseur brisé, épuisé ou incohérent réduit la stabilité et peut faire perdre le tour magique."
        );
        unlockEntry(
            "Parchemins jetables",
            "Objets rares",
            "Notes achetées : les non-mages peuvent déclencher certains parchemins, mais pas apprendre toute la magie. Les mages peuvent aussi utiliser ces consommables pour économiser leur souffle ou lancer un effet qu'ils n'ont pas étudié."
        );
        unlockEntry(
            "Purification mineure",
            "Objets rares",
            "Notes achetées : certains parchemins n'ont pas de grimoire équivalent stable. La purification mineure arrache des altérations courtes, mais le tracé brûle avec le papier."
        );
        unlockEntry(
            "Suture de mana",
            "Objets rares",
            "Notes achetées : certains grimoires n'ont pas de parchemin courant. La suture de mana soigne lentement sur plusieurs tours, demande un niveau suffisant et devient risquée avec un mauvais catalyseur."
        );
        unlockEntry(
            "Ronces occultes",
            "Objets rares",
            "Notes achetées : ce sort se transmet surtout par grimoire, pas par parchemin courant. Il combine entrave, poison et fatigue magique, avec un vrai catalyseur recommandé."
        );
        unlockEntry(
            "Venin rampant",
            "Objets rares",
            "Notes achetées : ce parchemin est jetable et ne correspond pas à un sort durable fiable. Le tracé empoisonne puis disparaît avec le papier."
        );
        return;
    }

    if (informationId == "elemental_weakness_notes")
    {
        unlockEntry(
            "Résistances et faiblesses",
            "Races",
            "Notes achetées : les effets persistants, éléments, races, classes, armures et matières doivent rester cohérents. Métal et choc, feu et plantes, froid et mouvements, poison et organique n'ont pas tous la même valeur selon la cible."
        );
        unlockEntry(
            "Équipement et éléments",
            "Objets rares",
            "Notes achetées : une armure ou arme adaptée peut réduire un danger, mais une matière incompatible peut aggraver l'usure ou la réaction. Une arme trop durement utilisée contre une matière opposée s'abîme plus vite."
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
            "Notes achetées : trio de potes. Mattzelda encaisse, Louis improvise, Trexof analyse chaque échange comme une partie à résoudre."
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
            "Manuel acheté : les invocations occupent des places précises. Les plus anciennes pages parlent de mana, d'évolution, de sacrifices et d'ombres liées à Hazak."
        );
        unlockEntry(
            "Ombres de Hazak",
            "Invocations",
            "Indice acheté : certaines ombres gardent une partie des gestes qu'elles possédaient de leur vivant. Hazak semble comprendre ce langage mieux que les autres."
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
            "Indice acheté : Fitoria peut laisser une plume lumineuse, Zelef du sang démoniaque, et Atlas une plaque brisée. Ces matériaux sont rares et réagissent aux reliques anciennes."
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
            "Notes lues : les boss vaincus peuvent laisser un fragment unique. Même incomplets, ces fragments servent déjà de base à des crafts rares."
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
            "Note achetée : les maîtres d'armes racontent que les gestes naissent de la répétition, du niveau, du craft et des erreurs qui ne tuent pas."
        );
        return;
    }

    if (informationId == "necromancy_warning")
    {
        unlockEntry(
            "Nécromancie instable",
            "Invocations",
            "Avertissement lu : relever une ombre n'est jamais gratuit. Les os, le mana, les morts récents et l'état mental du nécromancien laissent toujours une dette."
        );
        return;
    }

    if (informationId == "legend_child_tales")
    {
        unlockEntry(
            "Légendes de bibliothèque",
            "Légendes / contes",
            "Rayonnage ouvert : la bibliothèque commence à classer les légendes, les contes pour enfant et les rumeurs anciennes sans forcer le joueur à lire tout le lore."
        );
        unlockEntry(
            "Conte des Bras cassés",
            "Légendes / contes",
            "Conte acheté : cinq héros chaotiques libèrent des villages, sauvent des habitants, contestent parfois les règles et repartent sous les applaudissements... ou les soupirs des responsables locaux."
        );
        unlockEntry(
            "Conte pour enfant : les cinq héros trop bruyants",
            "Légendes / contes",
            "Archive ouverte : version adoucie des Bras cassés, lisible depuis le registre des légendes sans révéler les salles de boss."
        );
        unlockEntry(
            "Origine du nom des Bras cassés",
            "Légendes / contes",
            "Rumeur achetée : Hazak aurait perdu un bras face à un orc avant de réussir à le régénérer après deux semaines. Le surnom vient aussi de leurs catastrophes répétées qui finissent malgré tout en réussite."
        );
        return;
    }


    if (informationId == "legend_trigger_notes")
    {
        unlockEntry(
            "Rumeur du conteur prudent",
            "Légendes / contes",
            "Note achetée : les conteurs sérieux ne racontent pas une légende à chaque salle. Ils attendent un lieu, un PNJ ou un moment qui mérite vraiment de laisser une trace."
        );
        unlockEntry(
            "Note : légendes sans obligation",
            "Légendes / contes",
            "Note achetée : les légendes doivent enrichir le monde sans bloquer le jeu. Tu peux les ignorer, puis revenir les lire dans le registre quand tu veux."
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
