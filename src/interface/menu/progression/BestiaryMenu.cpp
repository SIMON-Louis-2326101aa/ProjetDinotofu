// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.
// English: Displays the first usable bestiary menu with prepared categories and detailed entries.
// Français : Affiche le premier bestiaire utilisable avec catégories préparées et fiches détaillées.

#include "interface/menu/progression/BestiaryMenu.hpp"

#include "core/Console.hpp"
#include "progression/bestiary/BestiaryRuntimeProgress.hpp"

#include <iostream>
#include <string>
#include <vector>

namespace
{
    struct BestiaryPreviewEntry
    {
        std::string category;
        std::string name;
        std::string simpleDescription;
        std::string detailedDescription;
        std::string status;
        int encounters;
        int kills;
    };

    const std::vector<BestiaryPreviewEntry>& getEntries()
    {
        static const std::vector<BestiaryPreviewEntry> entries = {
            {"Races", "Humain", "Race commune, adaptable et ambitieuse.", "Les humains sont présents dans beaucoup de villes. Ils servent de base pour les aventuriers, bandits, chevaliers et mages.", "Connu", 0, 0},
            {"Races", "Elfe noir", "Race discrète, souvent liée aux ombres.", "Les elfes noirs peuvent être craints selon les régions. Hazak représente une version très dangereuse de cette race.", "Partiellement connu", 0, 0},
            {"Races", "Kitsune", "Race semi-humaine liée aux illusions et aux flammes.", "Les kitsunes pourront plus tard avoir des bonus de magie, de ruse et d'invocation. Aoi en est un exemple spécial.", "Partiellement connu", 0, 0},
            {"Entités hostiles / ennemis", "Gobelin peureux", "Petit ennemi rapide qui préfère parfois survivre.", "Un gobelin basique. Pas très noble, mais capable de gratter des dégâts et de fuir si le combat devient humiliant.", "Déjà rencontré dans les arènes libres", 1, 0},
            {"Entités hostiles / ennemis", "Loup affamé", "Prédateur opportuniste.", "Le loup teste les proies faibles. Plus tard, il pourra cibler les blessés ou les soigneurs selon son instinct.", "Informations communes", 0, 0},
            {"Entités passives / alliées", "Aventurier random", "Humain ou semi-humain généré par l'arène.", "Un adversaire non spécial, avec nom, race et classe tirés aléatoirement. Il peut devenir allié ou rival plus tard.", "Préparé", 0, 0},
            {"Invocations", "Invocation mineure", "Renfort temporaire appelé par certaines classes.", "Les invocations suivent les mêmes règles globales de ciblage que les monstres, mais peuvent appartenir au joueur ou à l'ennemi.", "Préparé", 0, 0},
            {"Boss", "???", "Nom inconnu.", "Un boss ne révèle son vrai nom que s'il le prononce, à l'entrée, à 50%, avant sa défaite, ou via des écrits crédibles trouvés avant.", "Identité verrouillée", 0, 0},
            {"Personnages spéciaux", "Matt (PRO)", "Combattant silencieux et respectueux.", "Référence directe à Matt de Wii Sports. Il ne parle pas vraiment : il combat proprement, avec respect, et des bonus globaux légers.", "Spécial", 0, 0},
            {"Personnages spéciaux", "Hazak", "Assassin elfe noir, sombre et efficace.", "Hazak prend les gens de haut, cherche la victoire et protège Hestia d'une violence trop traumatisante. Meilleur ami d'Henrique.", "Spécial", 0, 0},
            {"Personnages spéciaux", "Sanctus / Skuro", "Protecteur pouvant vriller en exécuteur sombre.", "Sanctus protège, entrave et s'appuie sur un dieu de lumière. S'il prend trop cher ou inflige trop cher, Skuro peut ressortir.", "Spécial", 0, 0},
            {"Personnages spéciaux", "Aoi", "Kitsune timide, mage de flammes et future invocatrice.", "Aoi protège ses incantations et peut devenir très dangereuse si on lui laisse le temps de préparer sa magie.", "Spécial", 0, 0},
            {"Personnages spéciaux", "Kanadé", "Semi-dragonne rageuse aux sorts zodiacaux.", "Kanadé râle souvent, mais elle avance quand même. Ses futurs sorts seront liés aux treize signes du zodiaque.", "Spécial", 0, 0},
            {"Personnages spéciaux", "Hestia", "Humaine peureuse aux origines divines oubliées.", "Hestia était une divinité avant de perdre ses souvenirs. Elle préfère éviter le combat, mais possède une magie de protection anormalement haute.", "Spécial", 0, 0},
            {"Personnages spéciaux", "Henrique", "Chevalier fonceur capable de revenir une fois.", "Henrique est le meilleur ami de Hazak. Il fonce dans le tas et possède une future capacité de retour à la vie.", "Spécial", 0, 0},
            {"Personnages spéciaux", "Louis", "Artificier naïf qui cherche des amis.", "Louis vise bien, utilise plusieurs projectiles et reste fondamentalement sympathique malgré le chaos de l'arène.", "Spécial", 0, 0},
            {"Personnages spéciaux", "Trexof", "Assassin humain et bêta-testeur principal.", "Trexof possède de légers bonus et fait partie du groupe proche de Mattzelda et Louis.", "Spécial", 0, 0},
            {"Matériaux et plantes", "Fleur bleue de montagne", "Plante calme et rare.", "Clin d'œil à la fleur bleue de Zelda BOTW. Elle servira plus tard aux remèdes, quêtes et secrets liés aux plantes.", "Renseignement de base", 0, 0},
            {"Matériaux et plantes", "Oreille de gobelin", "Composant commun de monstre.", "Matériau simple récupérable sur certains gobelins. Utile pour les premières ventes ou crafts futurs.", "Loot possible", 0, 0}
        };

        return entries;
    }

    std::vector<BestiaryPreviewEntry> filterEntries(const std::string& category)
    {
        std::vector<BestiaryPreviewEntry> filtered;

        for (BestiaryPreviewEntry entry : getEntries())
        {
            entry.encounters += BestiaryRuntimeProgress::getEncounterCount(entry.name);
            entry.kills += BestiaryRuntimeProgress::getKillCount(entry.name);
            entry.status = BestiaryRuntimeProgress::getStatusFor(entry.name, entry.status);

            if (category == "Tout" || entry.category == category)
            {
                filtered.push_back(entry);
            }
        }

        for (const BestiaryRuntimeRecord& record : BestiaryRuntimeProgress::getRecords())
        {
            bool alreadyListed = false;

            for (const BestiaryPreviewEntry& entry : filtered)
            {
                if (entry.name == record.name)
                {
                    alreadyListed = true;
                    break;
                }
            }

            if (!alreadyListed && (category == "Tout" || record.category == category))
            {
                filtered.push_back({
                    record.category,
                    record.name,
                    record.description,
                    record.description,
                    record.status,
                    record.encounters,
                    record.kills
                });
            }
        }

        return filtered;
    }

    void displayEntryDetail(const BestiaryPreviewEntry& entry)
    {
        std::cout << "========== FICHE BESTIAIRE ==========" << std::endl;
        std::cout << "Catégorie : " << entry.category << std::endl;
        std::cout << "Nom : " << entry.name << std::endl;
        std::cout << "Statut : " << entry.status << std::endl;
        std::cout << "Rencontres : " << entry.encounters << std::endl;
        std::cout << "Tués par le joueur : " << entry.kills << std::endl;
        std::cout << std::endl;
        std::cout << entry.detailedDescription << std::endl;
        std::cout << "====================================" << std::endl;
        std::cout << std::endl;
    }

    void displayEntryList(const std::string& category)
    {
        std::vector<BestiaryPreviewEntry> entries = filterEntries(category);

        if (entries.empty())
        {
            std::cout << "Aucune entrée préparée pour cette catégorie." << std::endl;
            std::cout << std::endl;
            return;
        }

        while (true)
        {
            std::cout << "========== " << category << " ==========" << std::endl;
            std::cout << "0 : Retour" << std::endl;

            for (int i = 0; i < static_cast<int>(entries.size()); ++i)
            {
                std::cout << i + 1 << " : " << entries[i].name
                          << " | Statut : " << entries[i].status
                          << " | Tués : " << entries[i].kills
                          << std::endl;
            }

            std::cout << "================================" << std::endl;
            std::cout << std::endl;
            std::cout << "> ";

            int choice = Console::askNumberBetween(
                0,
                static_cast<int>(entries.size()),
                "Choix invalide."
            );

            Console::clear();

            if (choice == 0)
            {
                return;
            }

            displayEntryDetail(entries[choice - 1]);
            Console::waitForEnter();
            Console::clear();
        }
    }

    void displayInformationShopPreview()
    {
        std::cout << "========== ACHAT D'INFORMATIONS ==========" << std::endl;
        std::cout << "La bibliothèque peut maintenant débloquer des renseignements communs dans cette session." << std::endl;
        std::cout << "Les secrets, les boss et les noms importants devront toujours se mériter." << std::endl;
        std::cout << std::endl;
        std::cout << "Un nom de boss ne sera pas donné gratuitement si personne ne l'a prononcé." << std::endl;
        std::cout << "==========================================" << std::endl;
        std::cout << std::endl;
    }
}

void BestiaryMenu::open()
{
    while (true)
    {
        std::cout << "================ BESTIAIRE ================" << std::endl;
        std::cout << "Objet spécial de base : Bestiaire" << std::endl;
        std::cout << "Version actuelle : catalogue jouable + progression en session." << std::endl;
        std::cout << "===========================================" << std::endl;
        std::cout << "0 : Retour" << std::endl;
        std::cout << "1 : Tout voir" << std::endl;
        std::cout << "2 : Races" << std::endl;
        std::cout << "3 : Entités hostiles / ennemis" << std::endl;
        std::cout << "4 : Entités passives / alliées" << std::endl;
        std::cout << "5 : Invocations" << std::endl;
        std::cout << "6 : Boss" << std::endl;
        std::cout << "7 : Personnages spéciaux" << std::endl;
        std::cout << "8 : Matériaux et plantes" << std::endl;
        std::cout << "9 : Acheter des informations communes" << std::endl;
        std::cout << "===========================================" << std::endl;
        std::cout << std::endl;
        std::cout << "> ";

        int choice = Console::askNumberBetween(
            0,
            9,
            "Choix invalide. Entre un chiffre entre 0 et 9."
        );

        Console::clear();

        if (choice == 0)
        {
            return;
        }

        if (choice == 1) displayEntryList("Tout");
        else if (choice == 2) displayEntryList("Races");
        else if (choice == 3) displayEntryList("Entités hostiles / ennemis");
        else if (choice == 4) displayEntryList("Entités passives / alliées");
        else if (choice == 5) displayEntryList("Invocations");
        else if (choice == 6) displayEntryList("Boss");
        else if (choice == 7) displayEntryList("Personnages spéciaux");
        else if (choice == 8) displayEntryList("Matériaux et plantes");
        else if (choice == 9) displayInformationShopPreview();
    }
}

void BestiaryMenu::displayObjectEntry(const std::string& entryName)
{
    for (BestiaryPreviewEntry entry : getEntries())
    {
        if (entry.name == entryName)
        {
            entry.encounters += BestiaryRuntimeProgress::getEncounterCount(entry.name);
            entry.kills += BestiaryRuntimeProgress::getKillCount(entry.name);
            entry.status = BestiaryRuntimeProgress::getStatusFor(entry.name, entry.status);
            displayEntryDetail(entry);
            return;
        }
    }

    for (const BestiaryRuntimeRecord& record : BestiaryRuntimeProgress::getRecords())
    {
        if (record.name == entryName)
        {
            displayEntryDetail({
                record.category,
                record.name,
                record.description,
                record.description,
                record.status,
                record.encounters,
                record.kills
            });
            return;
        }
    }

    std::cout << "========== BESTIAIRE ==========" << std::endl;
    std::cout << "Recherche : " << entryName << std::endl;
    std::cout << std::endl;
    std::cout << "Entrée inconnue ou pas encore découverte." << std::endl;
    std::cout << "Si ton personnage découvre cette chose plus tard, le bestiaire pourra afficher son nom, sa description, ses rencontres et ses tués." << std::endl;
    std::cout << "===============================" << std::endl;
    std::cout << std::endl;
}
