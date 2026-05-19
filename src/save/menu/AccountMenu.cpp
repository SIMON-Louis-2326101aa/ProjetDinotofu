// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.
// Description: Handles local account selection, creation and deletion menus.
// Description : Gère les menus de sélection, création et suppression des comptes locaux.

#include "save/menu/AccountMenu.hpp"

#include "core/Console.hpp"
#include "save/SaveManager.hpp"

#include <iostream>
#include <string>
#include <vector>

std::string AccountMenu::open()
{
    while (true)
    {
        std::vector<AccountSaveSummary> accounts = SaveManager::listAccounts();

        std::cout << "===== COMPTES LOCAUX =====" << std::endl;
        std::cout << "0 : Créer / utiliser un nouveau compte" << std::endl;

        for (int i = 0; i < static_cast<int>(accounts.size()); i++)
        {
            std::cout << (i + 1) << " : " << accounts[i].accountName << std::endl;
        }

        std::cout << "==========================" << std::endl;
        std::cout << std::endl;
        std::cout << "> ";

        int choice = Console::askNumberBetween(
            0,
            static_cast<int>(accounts.size()),
            "Veuillez choisir un compte affiché, ou 0 pour en créer un."
        );

        if (choice == 0)
        {
            std::string accountName;

            std::cout << std::endl;
            std::cout << "Nom du compte local ?" << std::endl;
            std::cout << "Tu peux laisser vide pour utiliser le compte local par défaut." << std::endl;
            std::cout << "> ";

            std::getline(std::cin >> std::ws, accountName);

            if (accountName.empty())
            {
                accountName = "local";
            }

            Console::clear();
            std::cout << "Compte actif : " << accountName << "." << std::endl;
            std::cout << "Les sauvegardes sont rangées dans assets/saves/." << std::endl;
            std::cout << std::endl;

            if (!SaveManager::saveAccountSnapshot(accountName))
            {
                std::cout << "Attention : impossible de préparer la sauvegarde du compte pour le moment." << std::endl;
                std::cout << std::endl;
            }

            Console::waitForEnter();
            Console::clear();
            return accountName;
        }

        AccountSaveSummary selectedAccount = accounts[choice - 1];

        Console::clear();
        std::cout << "Compte sélectionné : " << selectedAccount.accountName << std::endl;
        std::cout << std::endl;
        std::cout << "0 : Retour" << std::endl;
        std::cout << "1 : Se connecter" << std::endl;
        std::cout << "2 : Supprimer ce compte" << std::endl;
        std::cout << std::endl;
        std::cout << "> ";

        int accountAction = Console::askNumberBetween(
            0,
            2,
            "Veuillez choisir 0, 1 ou 2."
        );

        if (accountAction == 0)
        {
            Console::clear();
            continue;
        }

        if (accountAction == 1)
        {
            std::string accountName = selectedAccount.accountName;

            Console::clear();
            std::cout << "Compte actif : " << accountName << "." << std::endl;
            std::cout << "Les sauvegardes sont rangées dans assets/saves/." << std::endl;
            std::cout << std::endl;

            if (!SaveManager::saveAccountSnapshot(accountName))
            {
                std::cout << "Attention : impossible de préparer la sauvegarde du compte pour le moment." << std::endl;
                std::cout << std::endl;
            }

            Console::waitForEnter();
            Console::clear();
            return accountName;
        }

        std::cout << std::endl;
        std::cout << "Supprimer ce compte supprimera aussi tous les personnages liés." << std::endl;
        std::cout << "Cette action est définitive." << std::endl;
        std::cout << "Tape SUPPRIMER pour confirmer." << std::endl;
        std::cout << "> ";

        std::string confirmation;
        std::getline(std::cin >> std::ws, confirmation);

        if (confirmation != "SUPPRIMER")
        {
            Console::clear();
            std::cout << "Suppression annulée." << std::endl;
            std::cout << std::endl;
            Console::waitForEnter();
            Console::clear();
            continue;
        }

        Console::clear();

        if (SaveManager::deleteAccountAndLinkedCharacters(selectedAccount.accountName))
        {
            std::cout << "Compte supprimé : " << selectedAccount.accountName << "." << std::endl;
            std::cout << "Les personnages liés à ce compte ont aussi été supprimés." << std::endl;
        }
        else
        {
            std::cout << "Impossible de supprimer complètement ce compte." << std::endl;
            std::cout << "Vérifie les fichiers dans assets/saves/." << std::endl;
        }

        std::cout << std::endl;
        Console::waitForEnter();
        Console::clear();
    }
}
