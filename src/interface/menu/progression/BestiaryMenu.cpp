// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "interface/menu/progression/BestiaryMenu.hpp"

#include "core/Console.hpp"

#include <iostream>

namespace
{
    void displayUnavailableCategory(const std::string& categoryName)
    {
        std::cout << "========== " << categoryName << " ==========" << std::endl;
        std::cout << "Le bestiaire reconnaît cette catégorie, mais les entrées ne sont pas encore reliées à la sauvegarde." << std::endl;
        std::cout << "Plus tard, tu verras d'abord une liste simple, puis une fiche détaillée après sélection." << std::endl;
        std::cout << std::endl;
        std::cout << "Format prévu :" << std::endl;
        std::cout << "- Nom connu ou ???" << std::endl;
        std::cout << "- Description physique si déjà rencontré" << std::endl;
        std::cout << "- Nombre de rencontres" << std::endl;
        std::cout << "- Nombre de fois tué" << std::endl;
        std::cout << "- Informations achetées ou découvertes" << std::endl;
        std::cout << "================================" << std::endl;
        std::cout << std::endl;
    }

    void displayInformationShopPreview()
    {
        std::cout << "========== ACHAT D'INFORMATIONS ==========" << std::endl;
        std::cout << "Plus tard, certains marchands, archives ou érudits pourront vendre des informations." << std::endl;
        std::cout << "Cela concernera surtout les choses communes : races connues, plantes courantes, matériaux simples ou monstres très répandus." << std::endl;
        std::cout << std::endl;
        std::cout << "Les secrets importants, les boss inconnus et les véritables noms devront être découverts autrement." << std::endl;
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
        std::cout << "Il gardera la trace des races, monstres, boss, personnages spéciaux, invocations, matériaux et plantes." << std::endl;
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

        if (choice == 1)
        {
            displayUnavailableCategory("TOUT LE BESTIAIRE");
        }
        else if (choice == 2)
        {
            displayUnavailableCategory("RACES");
        }
        else if (choice == 3)
        {
            displayUnavailableCategory("ENTITÉS HOSTILES / ENNEMIS");
        }
        else if (choice == 4)
        {
            displayUnavailableCategory("ENTITÉS PASSIVES / ALLIÉES");
        }
        else if (choice == 5)
        {
            displayUnavailableCategory("INVOCATIONS");
        }
        else if (choice == 6)
        {
            displayUnavailableCategory("BOSS");
        }
        else if (choice == 7)
        {
            displayUnavailableCategory("PERSONNAGES SPÉCIAUX");
        }
        else if (choice == 8)
        {
            displayUnavailableCategory("MATÉRIAUX ET PLANTES");
        }
        else if (choice == 9)
        {
            displayInformationShopPreview();
        }
    }
}

void BestiaryMenu::displayObjectEntry(const std::string& entryName)
{
    std::cout << "========== BESTIAIRE ==========" << std::endl;
    std::cout << "Recherche : " << entryName << std::endl;
    std::cout << std::endl;
    std::cout << "Entrée non encore renseignée." << std::endl;
    std::cout << "Le bestiaire existe déjà comme objet spécial, mais sa base de données progressive sera branchée plus tard." << std::endl;
    std::cout << std::endl;
    std::cout << "Plus tard, cette action affichera ce que ton personnage sait vraiment sur cette chose." << std::endl;
    std::cout << "Si l'entrée est inconnue, elle pourra apparaître comme ??? ou comme une simple description physique." << std::endl;
    std::cout << "===============================" << std::endl;
    std::cout << std::endl;
}
