// EN: AccountMenu.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: AccountMenu.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.
// Description: Handles local account selection, creation and deletion menus.
// Description : Gère les menus de sélection, création et suppression des comptes locaux.

#include "save/menu/AccountMenu.hpp"

#include "core/Console.hpp"
#include "interface/TerminalInterface.hpp"
#include "interface/menu/common/MessageScreen.hpp"
#include "interface/model/MenuScreen.hpp"
#include "save/SaveManager.hpp"
#include "core/VersionInfo.hpp"
#include "progression/TitleCatalog.hpp"

#include <algorithm>
#include <string>
#include <vector>

namespace
{

    std::string formatAccountPlayTime(long long totalSeconds)
    {
        const long long safe = totalSeconds < 0 ? 0 : totalSeconds;
        const long long hours = safe / 3600;
        const long long minutes = (safe % 3600) / 60;
        if (hours > 0) return std::to_string(hours) + " h " + std::to_string(minutes) + " min";
        return std::to_string(minutes) + " min";
    }
    MenuOptionItemData makeAccountMenuItem(
        const std::string& actionType,
        const std::string& name,
        const std::string& detail,
        const std::string& status = "",
        const std::string& progress = "",
        const std::string& owner = "",
        bool important = false
    )
    {
        MenuOptionItemData data;
        data.structured = true;
        data.kind = "save_account";
        data.section = "Comptes";
        data.actionType = actionType;
        data.name = name;
        data.detail = detail;
        data.status = status;
        data.progress = progress;
        data.owner = owner;
        data.important = important;
        return data;
    }

    MenuOptionItemData makeAccountSummaryItem(const AccountSaveSummary& account)
    {
        const std::string version = account.savedForVersion.empty() ? "inconnue" : account.savedForVersion;
        const std::string activity = account.lastActivityText.empty() ? "Inconnue" : account.lastActivityText;
        const VersionCompatibilityImpact impact = account.savedForVersion.empty()
            ? VersionCompatibilityImpact::None
            : VersionInfo::evaluateCompatibility(account.savedForVersion);
        std::string compatibility = "Compatible";
        if (impact == VersionCompatibilityImpact::PatchUpdate) compatibility = "Patch à appliquer";
        else if (impact == VersionCompatibilityImpact::MidUpdate) compatibility = "Adaptation recommandée";
        else if (impact == VersionCompatibilityImpact::RecreateRecommended) compatibility = "Ancienne structure";

        return makeAccountMenuItem(
            "select",
            account.accountName,
            "Version de sauvegarde : V" + version + " | Dernière activité : " + activity,
            compatibility,
            "Personnages jouables : " + std::to_string(account.playableCharacterCount)
                + " | Titres compte : " + std::to_string(account.unlockedTitleCount)
                + " | Temps total : " + formatAccountPlayTime(account.totalRealPlayTimeSeconds),
            "Jeu actuel : V" + VersionInfo::currentVersion(),
            impact != VersionCompatibilityImpact::None
        );
    }

    void showAccountTitleArchive(const std::string& accountName)
    {
        const std::vector<AccountUnlockedTitleInfo> unlockedDetails = SaveManager::getAccountUnlockedTitleDetails(accountName);
        std::vector<std::string> unlocked;
        unlocked.reserve(unlockedDetails.size());
        for (const AccountUnlockedTitleInfo& info : unlockedDetails) unlocked.push_back(info.name);
        std::vector<std::string> lines;
        lines.push_back("Compte : " + accountName);
        lines.push_back("Titres de compte débloqués et conservés : " + std::to_string(unlocked.size()) + ".");
        lines.push_back("Catalogue des titres : " + TitleCatalog::catalogVersion() + " (dernière mise à jour du catalogue).");
        lines.push_back("Un personnage supprimé ne retire jamais un titre déjà validé sur ce compte.");
        lines.push_back("Un personnage Altéré ne peut plus enrichir la progression normale du compte ; seule la marque spéciale « On ne dira rien... » fait exception.");
        lines.push_back("");
        lines.push_back("TITRES DÉBLOQUÉS");

        if (unlocked.empty())
        {
            lines.push_back("- Aucun titre de compte débloqué pour l'instant.");
        }
        else
        {
            for (const AccountUnlockedTitleInfo& info : unlockedDetails)
            {
                const std::string date = info.unlockedAt.empty() ? "date historique inconnue" : info.unlockedAt;
                const std::string character = info.characterName.empty() ? "personnage historique inconnu" : info.characterName;
                lines.push_back("- [obtenu] " + info.name + " | " + date + " | avec " + character);
            }
        }

        lines.push_back("");
        lines.push_back("TITRES RESTANTS");
        for (const TitleCatalog::TitleDefinition& definition : TitleCatalog::availableTitleDefinitions())
        {
            if (std::find(unlocked.begin(), unlocked.end(), definition.name) != unlocked.end())
            {
                continue;
            }

            if (definition.visibleBeforeUnlock)
            {
                lines.push_back("- [à obtenir] " + definition.name);
                lines.push_back("  " + definition.unlockHint);
            }
            else
            {
                lines.push_back("- ????");
            }
        }

        MessageScreen::show("TITRES DÉBLOQUÉS — COMPTE", "save.accounts.titles", lines);
    }
}

std::string AccountMenu::open()
{
    while (true)
    {
        std::vector<AccountSaveSummary> accounts = SaveManager::listAccounts();

        MenuScreen accountListScreen("COMPTES LOCAUX", "save.accounts.list");
        accountListScreen.addSubtitle("Sauvegardes locales");
        accountListScreen.addLine("Comptes trouvés : " + std::to_string(accounts.size()) + ".");
        accountListScreen.addLine("Chaque carte indique ses personnages, sa version de sauvegarde et sa dernière activité connue.");
        accountListScreen.addLine("Choisis un compte existant, crée un compte, ou importe un dossier portable.");
        accountListScreen.addOption(
            0,
            "Créer / utiliser un nouveau compte",
            "Prépare un compte local ou reprend le compte local par défaut.",
            true,
            "save.accounts.create_or_use",
            makeAccountMenuItem("create", "Nouveau compte", "Créer ou utiliser un compte local.", "Disponible", "Compte actif immédiat")
        );
        accountListScreen.addOption(
            50,
            "Importer un compte extrait",
            "Récupère un dossier portable exporté depuis une autre installation.",
            true,
            "save.accounts.import",
            makeAccountMenuItem("import", "Importer", "Importer un compte portable.", "Dossier requis", "Dossier exporté", "assets/saves/import_accounts/", true)
        );

        for (int i = 0; i < static_cast<int>(accounts.size()); i++)
        {
            accountListScreen.addOption(
                i + 1,
                accounts[i].accountName + " | Temps total " + formatAccountPlayTime(accounts[i].totalRealPlayTimeSeconds),
                "Compte local disponible. Le temps reste cumulé même si des personnages sont morts ou supprimés.",
                true,
                "save.accounts.select." + std::to_string(i + 1),
                makeAccountSummaryItem(accounts[i])
            );
        }

        int choice = TerminalInterface::askMenuChoiceFromOptions(
            accountListScreen,
            "Veuillez choisir un compte affiché, 0 pour créer, ou 50 pour importer."
        );

        if (choice == 50)
        {
            std::string packagePath = MessageScreen::askText(
                "IMPORT DE COMPTE",
                "save.accounts.import.path",
                {
                    "Chemin du dossier de compte extrait ?",
                    "Exemple : assets/saves/import_accounts/mon-compte_dinotofu_account"
                }
            );

            std::string importedAccountName;
            Console::clear();

            if (SaveManager::importAccountPackage(packagePath, importedAccountName))
            {
                MessageScreen::show(
                    "COMPTE IMPORTÉ",
                    "save.accounts.import.success",
                    {
                        "Compte importé : " + importedAccountName + ".",
                        "Tu peux maintenant le sélectionner dans la liste des comptes."
                    }
                );
            }
            else
            {
                MessageScreen::show(
                    "IMPORT IMPOSSIBLE",
                    "save.accounts.import.failure",
                    {
                        "Import impossible.",
                        "Vérifie que le dossier contient bien accounts/ et les fichiers exportés."
                    }
                );
            }

            continue;
        }

        if (choice == 0)
        {
            std::string accountName = MessageScreen::askText(
                "NOUVEAU COMPTE",
                "save.accounts.create.name",
                {
                    "Nom du compte local ?",
                    "Tu peux laisser vide pour utiliser le compte local par défaut."
                }
            );

            if (accountName.empty())
            {
                accountName = "local";
            }

            Console::clear();

            std::vector<std::string> lines = {
                "Compte actif : " + accountName + ".",
                "Les sauvegardes sont rangées dans assets/saves/."
            };

            if (!SaveManager::saveAccountSnapshot(accountName))
            {
                lines.push_back("Attention : impossible de préparer la sauvegarde du compte pour le moment.");
            }

            MessageScreen::show("COMPTE ACTIF", "save.accounts.create.active", lines);
            return accountName;
        }

        AccountSaveSummary selectedAccount = accounts[choice - 1];

        Console::clear();

        MenuScreen accountActionScreen("COMPTE SÉLECTIONNÉ", "save.accounts.actions");
        accountActionScreen.addSubtitle("Compte local sélectionné");
        accountActionScreen.addLine("Compte : " + selectedAccount.accountName);
        accountActionScreen.addLine("Dossier : " + selectedAccount.path);
        accountActionScreen.addLine("Personnages jouables liés : " + std::to_string(selectedAccount.playableCharacterCount) + ".");
        accountActionScreen.addLine("Titres légitimes conservés : " + std::to_string(selectedAccount.unlockedTitleCount) + ".");
        accountActionScreen.addLine("Temps total historique du compte : " + formatAccountPlayTime(selectedAccount.totalRealPlayTimeSeconds) + ".");
        accountActionScreen.addLine("Version de sauvegarde : V" + (selectedAccount.savedForVersion.empty() ? std::string("inconnue") : selectedAccount.savedForVersion));
        accountActionScreen.addLine("Dernière activité connue : " + (selectedAccount.lastActivityText.empty() ? std::string("Inconnue") : selectedAccount.lastActivityText));
        accountActionScreen.addOption(0, "Retour", "", true, "save.accounts.actions.back");
        accountActionScreen.addOption(
            1,
            "Se connecter",
            "Utiliser ce compte pour choisir ou créer un personnage.",
            true,
            "save.accounts.actions.login",
            makeAccountMenuItem("login", "Se connecter", "Compte : " + selectedAccount.accountName, "Action normale", "Ouvre la liste des personnages")
        );
        accountActionScreen.addOption(
            2,
            "Extraire / transférer ce compte",
            "Crée un dossier portable pour déplacer les données du compte.",
            true,
            "save.accounts.actions.export",
            makeAccountMenuItem("export", "Extraire / transférer", "Compte : " + selectedAccount.accountName, "Transfert", "Dossier portable", "Dossier source : " + selectedAccount.path, true)
        );
        accountActionScreen.addOption(
            3,
            "Supprimer ce compte",
            "Action définitive, confirmation obligatoire.",
            true,
            "save.accounts.actions.delete",
            makeAccountMenuItem("delete", "Supprimer", "Compte : " + selectedAccount.accountName, "Irréversible", "Supprime aussi les personnages liés", "Confirmation : SUPPRIMER", true)
        );
        accountActionScreen.addOption(
            4,
            "Titres débloqués",
            "Consulter l'historique permanent des titres légitimement obtenus sur ce compte.",
            true,
            "save.accounts.actions.titles",
            makeAccountMenuItem("inspect", "Titres débloqués", "Collection permanente du compte.", "Progression légitime", std::to_string(selectedAccount.unlockedTitleCount) + " titre(s)")
        );

        int accountAction = TerminalInterface::askMenuChoice(
            accountActionScreen,
            0,
            4,
            "Veuillez choisir une option de 0 à 4."
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

            std::vector<std::string> lines = {
                "Compte actif : " + accountName + ".",
                "Les sauvegardes sont rangées dans assets/saves/."
            };

            if (!SaveManager::saveAccountSnapshot(accountName))
            {
                lines.push_back("Attention : impossible de préparer la sauvegarde du compte pour le moment.");
            }

            MessageScreen::show("COMPTE ACTIF", "save.accounts.login.active", lines);
            return accountName;
        }

        if (accountAction == 2)
        {
            std::string exportedPath;

            Console::clear();

            MessageScreen::show(
                "EXTRACTION DE COMPTE",
                "save.accounts.export.warning",
                {
                    "Attention : l'extraction fonctionne comme un transfert.",
                    "Les personnages et données liées partiront en voyage dans le dossier portable.",
                    "Le compte local restera visible, mais sans les personnages transférés."
                },
                false
            );

            if (SaveManager::exportAccountPackage(selectedAccount.accountName, exportedPath))
            {
                MessageScreen::show(
                    "COMPTE EXTRAIT",
                    "save.accounts.export.success",
                    {
                        "Compte extrait avec succès.",
                        "Dossier portable : " + exportedPath,
                        "Tu peux copier ce dossier sur clé USB, puis l'importer sur une autre installation."
                    }
                );
            }
            else
            {
                MessageScreen::show(
                    "EXTRACTION IMPOSSIBLE",
                    "save.accounts.export.failure",
                    {
                        "Extraction impossible.",
                        "Vérifie que le compte existe et que assets/saves/ est accessible."
                    }
                );
            }

            continue;
        }

        if (accountAction == 4)
        {
            Console::clear();
            showAccountTitleArchive(selectedAccount.accountName);
            continue;
        }

        if (!MessageScreen::askKeywordConfirmation(
                "SUPPRESSION DE COMPTE",
                "save.accounts.delete.confirm",
                {
                    "Supprimer ce compte supprimera aussi tous les personnages liés.",
                    "Cette action est définitive."
                },
                "SUPPRIMER"
            ))
        {
            Console::clear();
            MessageScreen::show(
                "SUPPRESSION ANNULÉE",
                "save.accounts.delete.cancelled",
                {"Suppression annulée."}
            );
            continue;
        }

        Console::clear();

        if (SaveManager::deleteAccountAndLinkedCharacters(selectedAccount.accountName))
        {
            MessageScreen::show(
                "COMPTE SUPPRIMÉ",
                "save.accounts.delete.success",
                {
                    "Compte supprimé : " + selectedAccount.accountName + ".",
                    "Les personnages liés à ce compte ont aussi été supprimés."
                }
            );
        }
        else
        {
            MessageScreen::show(
                "SUPPRESSION INCOMPLÈTE",
                "save.accounts.delete.failure",
                {
                    "Impossible de supprimer complètement ce compte.",
                    "Vérifie les fichiers dans assets/saves/."
                }
            );
        }
    }
}
