// EN: CharacterMenu.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: CharacterMenu.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.
// Description: Handles playable character selection, loading, deletion and protected special identity validation.
// Description : Gère les menus de sélection, chargement, suppression et validation des identités spéciales protégées.

#include "save/menu/CharacterMenu.hpp"

#include "character/SpecialCharacter.hpp"
#include "character/SpecialCharacterDateRule.hpp"
#include "character/SpecialCharacterNameGuard.hpp"
#include "core/Console.hpp"
#include "core/VersionInfo.hpp"
#include "interface/TerminalInterface.hpp"
#include "interface/menu/common/MessageScreen.hpp"
#include "interface/model/MenuScreen.hpp"
#include "save/SaveManager.hpp"

#include <iostream>
#include <string>
#include <vector>

namespace
{
    // EN: createEmptyResult declares or implements a focused behavior used by this module.
    // FR: createEmptyResult déclare ou implémente un comportement précis utilisé par ce module.
    CharacterMenuResult createEmptyResult()
    {
        CharacterMenuResult result;
        result.characterLoaded = false;
        result.specialIdentityValidated = false;
        result.playerName = "";
        result.difficulty = DifficultyMode::Normal;
        result.forcedRace = CharacterRace::Human;
        return result;
    }

    // EN: askYesNo declares or implements a focused behavior used by this module.
    // FR: askYesNo déclare ou implémente un comportement précis utilisé par ce module.
    bool askYesNo(const std::string& question)
    {
        MenuScreen screen("CONFIRMATION", "save.confirmation.yes_no");
        screen.addLine(question);
        screen.addOption(1, "Oui", "", true, "save.confirmation.yes");
        screen.addOption(2, "Non", "", true, "save.confirmation.no");

        int choice = TerminalInterface::askMenuChoice(
            screen,
            1,
            2,
            "Veuillez choisir 1 ou 2."
        );

        return choice == 1;
    }

    std::string versionImpactShortLabel(VersionCompatibilityImpact impact)
    {
        switch (impact)
        {
            case VersionCompatibilityImpact::RecreateRecommended:
                return "MAJ lourde conseillée";
            case VersionCompatibilityImpact::MidUpdate:
                return "MAJ moyenne";
            case VersionCompatibilityImpact::PatchUpdate:
                return "patch dispo";
            case VersionCompatibilityImpact::None:
            default:
                return "";
        }
    }

    MenuOptionItemData makeCharacterMenuItem(
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
        data.kind = "save_character";
        data.section = "Personnages";
        data.actionType = actionType;
        data.name = name;
        data.detail = detail;
        data.status = status;
        data.progress = progress;
        data.owner = owner;
        data.important = important;
        return data;
    }

    MenuOptionItemData makeCharacterSummaryItem(
        const CharacterSaveSummary& character,
        VersionCompatibilityImpact impact
    )
    {
        const std::string versionLabel = versionImpactShortLabel(impact);
        std::string status;

        if (character.clone)
        {
            status = "CLONE";
        }

        if (!versionLabel.empty())
        {
            if (!status.empty()) status += " · ";
            status += versionLabel;
        }

        if (status.empty())
        {
            status = "Jouable";
        }

        return makeCharacterMenuItem(
            "select",
            character.characterName,
            "Race / classe : " + character.raceName + " / " + character.className,
            status,
            "Niveau " + std::to_string(character.level),
            "Maître : " + character.currentOwnerAccountName,
            impact != VersionCompatibilityImpact::None || character.clone
        );
    }

    void displayVersionCompatibilityWarning(
        const std::string& accountName,
        const CharacterSaveSummary& character
    )
    {
        VersionCompatibilityImpact impact = VersionInfo::evaluateCompatibility(character.lastAdaptedVersion);
        std::string message = VersionInfo::compatibilityMessage(impact);

        if (message.empty())
        {
            return;
        }

        MenuScreen screen("TRACE DE VERSION", "save.characters.version.warning");
        screen.addLine(message);
        screen.addLine("Profil concerné : " + accountName);
        screen.addLine("Personnage concerné : " + character.characterName);
        screen.addLine("Version actuelle : V" + VersionInfo::currentVersion());
        screen.addLine("Dernière adaptation connue : V" + character.lastAdaptedVersion);

        if (impact == VersionCompatibilityImpact::RecreateRecommended)
        {
            screen.addLine("Certaines protections de départ peuvent manquer : équipement, munitions, potions, traces initiales.");
        }

        TerminalInterface::renderMenuScreen(screen, false);
    }


    enum class LegacyCharacterDecision
    {
        Back,
        Recreate,
        HeavyAdaptation,
        ContinueNormally
    };

    LegacyCharacterDecision askLegacyCharacterDecision(
        const std::string& accountName,
        const CharacterSaveSummary& character
    )
    {
        VersionCompatibilityImpact impact = VersionInfo::evaluateCompatibility(character.lastAdaptedVersion);

        if (impact != VersionCompatibilityImpact::RecreateRecommended)
        {
            return LegacyCharacterDecision::ContinueNormally;
        }

        MenuScreen screen("ANCIENNE TRACE DE PERSONNAGE", "save.characters.legacy.decision");
        screen.addLine("Les archives sentent une ancienne version autour de ce personnage.");
        screen.addLine("Son histoire peut continuer, mais certains fils de départ étaient encore absents.");
        screen.addLine("Kit initial, munitions, équipement de secours et protections peuvent devoir être retissés.");
        screen.addLine("Profil concerné : " + accountName);
        screen.addLine("Personnage concerné : " + character.characterName);
        screen.addOption(0, "Retour", "", true, "save.characters.legacy.back");
        screen.addOption(1, "Procéder au rituel d'oubli contrôlé", "Le personnage sera recréé proprement pour cette version.", true, "save.characters.legacy.recreate");
        screen.addOption(2, "Subir une adaptation lourde", "Ajoute les sécurités manquantes sans remplacer tes gains.", true, "save.characters.legacy.heavy_adaptation");

        int choice = TerminalInterface::askMenuChoiceFromOptions(
            screen,
            "Veuillez choisir 0, 1 ou 2."
        );

        if (choice == 1) return LegacyCharacterDecision::Recreate;
        if (choice == 2) return LegacyCharacterDecision::HeavyAdaptation;
        return LegacyCharacterDecision::Back;
    }

    bool validateProtectedName(
        const std::string& proposedName,
        CharacterMenuResult& result
    )
    {
        SpecialCharacter protectedCharacter;

        if (!SpecialCharacterNameGuard::tryGetProtectedCharacter(proposedName, protectedCharacter))
        {
            return true;
        }

        Console::clear();
        SpecialCharacterNameGuard::displayIdentityWarning(protectedCharacter);

        if (protectedCharacter.isPermanentlyNonPlayable())
        {
            Console::waitForEnter();
            Console::clear();
            return false;
        }

        if (!protectedCharacter.canBePlayedWithSpecialDate())
        {
            MessageScreen::show(
                "IDENTITÉ REFUSÉE",
                "save.characters.special_identity.refused_path",
                {"Cette identité refuse d'être incarnée par ce chemin."}
            );
            Console::clear();
            return false;
        }

        if (!askYesNo("Veux-tu vraiment tenter d'usurper cette identité ?"))
        {
            Console::clear();
            MessageScreen::show(
                "IDENTITÉ ABANDONNÉE",
                "save.characters.special_identity.cancelled",
                {"Très bien. L'arène préfère les noms qui appartiennent vraiment à leur porteur."}
            );
            Console::clear();
            return false;
        }

        std::string dateInput = MessageScreen::askText(
            "DATE SPÉCIALE",
            "save.characters.special_identity.date",
            {
                "Pour prouver que c'est bien toi, entre sa date spéciale.",
                "Format attendu : DD/MM/YYYY."
            },
            "DD/MM/YYYY",
            "Date exacte liée à cette identité spéciale.",
            false,
            10,
            10
        );

        if (!SpecialCharacterDateRule::validate(protectedCharacter, dateInput))
        {
            Console::clear();
            SpecialCharacterNameGuard::displayIdentityRefused(protectedCharacter);
            Console::waitForEnter();
            Console::clear();
            return false;
        }

        result.playerName = protectedCharacter.getName();
        result.specialIdentityValidated = true;
        result.forcedRace = protectedCharacter.getRace();

        Console::clear();
        SpecialCharacterNameGuard::displayIdentityAccepted(protectedCharacter);
        MessageScreen::show(
            "IDENTITÉ VALIDÉE",
            "save.characters.special_identity.accepted_details",
            {
                "Race liée à l'identité : " + protectedCharacter.getRaceText() + ".",
                "Tu pourras quand même choisir ta classe.",
                "Si tu choisis sa classe native, le bonus natif pourra s'activer."
            }
        );
        Console::clear();

        return true;
    }
}

// EN: open declares or implements a focused behavior used by this module.
// FR: open déclare ou implémente un comportement précis utilisé par ce module.
CharacterMenuResult CharacterMenu::open(const std::string& accountName, Player& player)
{
    while (true)
    {
        std::vector<CharacterSaveSummary> characters = SaveManager::listPlayableCharacters(accountName);

        MenuScreen characterListScreen("PERSONNAGES", "save.characters.list");
        characterListScreen.addOption(
            0,
            "Créer un nouveau personnage",
            "Prépare une nouvelle fiche personnage pour cette version.",
            true,
            "save.characters.create",
            makeCharacterMenuItem("create", "Nouveau personnage", "Créer une nouvelle fiche personnage.", "Disponible")
        );

        for (int i = 0; i < static_cast<int>(characters.size()); i++)
        {
            VersionCompatibilityImpact listImpact = VersionInfo::evaluateCompatibility(characters[i].lastAdaptedVersion);
            std::string versionLabel = versionImpactShortLabel(listImpact);

            std::string label = characters[i].characterName
                + " | " + characters[i].raceName
                + " / " + characters[i].className
                + " | Niveau " + std::to_string(characters[i].level)
                + (characters[i].clone ? " | CLONE" : "");

            if (!versionLabel.empty())
            {
                label += " | " + versionLabel;
            }

            characterListScreen.addOption(
                i + 1,
                label,
                "Sélectionner ce personnage et afficher ses actions.",
                true,
                "save.characters.select." + std::to_string(i + 1),
                makeCharacterSummaryItem(characters[i], listImpact)
            );
        }

        int choice = TerminalInterface::askMenuChoice(
            characterListScreen,
            0,
            static_cast<int>(characters.size()),
            "Veuillez choisir un personnage affiché, ou 0 pour en créer un."
        );

        if (choice == 0)
        {
            CharacterMenuResult result = createEmptyResult();

            while (true)
            {
                result.playerName = MessageScreen::askText(
                    "NOM DU PERSONNAGE",
                    "save.characters.create.name",
                    {"Quel est ton nom ?"}
                );

                while (result.playerName.empty())
                {
                    result.playerName = MessageScreen::askText(
                        "NOM DU PERSONNAGE",
                        "save.characters.create.name.empty",
                        {
                            "Un nom vide ? Même les gobelins ont plus de présence que ça.",
                            "Entre un vrai nom."
                        }
                    );
                }

                if (validateProtectedName(result.playerName, result))
                {
                    break;
                }
            }

            Console::clear();

            MessageScreen::show(
                "NOM GRAVÉ",
                "save.characters.create.name.accepted",
                {
                    "Très bien, " + result.playerName + ".",
                    "L'arène se souviendra peut-être de ce nom..."
                }
            );
            return result;
        }

        CharacterSaveSummary selectedCharacter = characters[choice - 1];

        Console::clear();
        MenuScreen selectedCharacterScreen("PERSONNAGE SÉLECTIONNÉ", "save.characters.selected.summary");
        selectedCharacterScreen.addLine("Personnage : " + selectedCharacter.characterName);
        selectedCharacterScreen.addLine("Race / classe : " + selectedCharacter.raceName + " / " + selectedCharacter.className);
        selectedCharacterScreen.addLine("Niveau : " + std::to_string(selectedCharacter.level));
        selectedCharacterScreen.addLine("Créateur : " + selectedCharacter.creatorAccountName);
        selectedCharacterScreen.addLine("Joueur / maître actuel : " + selectedCharacter.currentOwnerAccountName);
        selectedCharacterScreen.addLine("Créé le : " + selectedCharacter.createdAt + " | V" + selectedCharacter.createdForVersion);
        selectedCharacterScreen.addLine("Dernière adaptation faite pour la V" + selectedCharacter.lastAdaptedVersion);
        if (selectedCharacter.clone)
        {
            selectedCharacterScreen.addLine("Statut : CLONE — JcJ amical uniquement.");
        }
        TerminalInterface::renderMenuScreen(selectedCharacterScreen, false);

        if (selectedCharacter.accountName != accountName
            || selectedCharacter.currentOwnerAccountName != accountName)
        {
            MessageScreen::show(
                "MAÎTRE REFUSÉ",
                "save.characters.owner.refused",
                {
                    "Une marque étrangère traverse cette sauvegarde.",
                    "Ce personnage n'appartient pas à ce compte, même si son fichier a été déplacé ici.",
                    "Un personnage n'a qu'un seul maître.",
                    "Maître inscrit : " + selectedCharacter.currentOwnerAccountName,
                    "Compte actif : " + accountName,
                    "Chargement refusé. Utilise le rituel de transfert irréversible si ce personnage doit changer de maître."
                }
            );
            continue;
        }

        displayVersionCompatibilityWarning(accountName, selectedCharacter);

        MenuScreen characterActionScreen("ACTIONS DU PERSONNAGE", "save.characters.actions");
        characterActionScreen.addLine("Personnage : " + selectedCharacter.characterName);
        characterActionScreen.addLine("Action irréversible disponible : le maître actuel peut changer vraiment.");
        characterActionScreen.addOption(0, "Retour", "", true, "save.characters.actions.back");
        characterActionScreen.addOption(
            1,
            "Incarner",
            "Charge ce personnage et reprend la session avec lui.",
            true,
            "save.characters.actions.play",
            makeCharacterMenuItem("play", "Incarner", "Personnage : " + selectedCharacter.characterName, "Action normale")
        );
        characterActionScreen.addOption(
            2,
            "Donner ce personnage à un autre compte",
            "Change définitivement le maître actuel du personnage.",
            true,
            "save.characters.actions.transfer_owner",
            makeCharacterMenuItem("transfer", "Donner à un autre compte", "Personnage : " + selectedCharacter.characterName, "Irréversible", "", "Maître actuel : " + selectedCharacter.currentOwnerAccountName, true)
        );
        characterActionScreen.addOption(
            3,
            "Extraire / transférer ce personnage en dossier portable",
            "Le personnage quitte ce compte local dans un dossier portable.",
            true,
            "save.characters.actions.export",
            makeCharacterMenuItem("export", "Extraire / transférer", "Personnage : " + selectedCharacter.characterName, "Transfert")
        );
        characterActionScreen.addOption(
            4,
            "Extraire un clone du personnage",
            "Garde l'original ici et crée un clone portable limité au JcJ amical.",
            true,
            "save.characters.actions.clone",
            makeCharacterMenuItem("clone", "Extraire un clone", "Personnage : " + selectedCharacter.characterName, "Clone")
        );
        characterActionScreen.addOption(
            5,
            "Supprimer ce personnage",
            "Action définitive, confirmation obligatoire.",
            true,
            "save.characters.actions.delete",
            makeCharacterMenuItem("delete", "Supprimer", "Personnage : " + selectedCharacter.characterName, "Irréversible", "", "Maître actuel : " + selectedCharacter.currentOwnerAccountName, true)
        );

        int characterAction = TerminalInterface::askMenuChoice(
            characterActionScreen,
            0,
            5,
            "Veuillez choisir 0, 1, 2, 3, 4 ou 5."
        );

        if (characterAction == 0)
        {
            Console::clear();
            continue;
        }

        if (characterAction == 1)
        {
            LegacyCharacterDecision legacyDecision = askLegacyCharacterDecision(accountName, selectedCharacter);

            if (legacyDecision == LegacyCharacterDecision::Back)
            {
                Console::clear();
                continue;
            }

            if (legacyDecision == LegacyCharacterDecision::Recreate)
            {
                CharacterMenuResult result = createEmptyResult();
                result.playerName = selectedCharacter.characterName;
                Console::clear();
                MessageScreen::show(
                    "RITUEL D'OUBLI CONTRÔLÉ",
                    "save.characters.legacy.recreate.notice",
                    {
                        "Le nom est conservé, mais le fil sera retissé depuis le début.",
                        "Choisis à nouveau la difficulté, la race et la classe pour recréer proprement ce personnage."
                    }
                );
                return result;
            }

            VersionCompatibilityImpact selectedImpact = VersionInfo::evaluateCompatibility(selectedCharacter.lastAdaptedVersion);

            CharacterMenuResult result = createEmptyResult();
            result.characterLoaded = SaveManager::loadPlayerSnapshot(
                selectedCharacter,
                player,
                result.difficulty
            );

            std::vector<std::string> adaptationChanges;

            if (result.characterLoaded && legacyDecision == LegacyCharacterDecision::HeavyAdaptation)
            {
                adaptationChanges = player.applyHeavyVersionAdaptation(result.difficulty);
                if (!SaveManager::savePlayerSnapshot(player, accountName, result.difficulty))
                {
                    adaptationChanges.push_back("Attention : adaptation appliquée en mémoire, mais sauvegarde immédiate impossible.");
                }
            }
            else if (result.characterLoaded
                && (selectedImpact == VersionCompatibilityImpact::PatchUpdate
                    || selectedImpact == VersionCompatibilityImpact::MidUpdate))
            {
                player.markAdaptedToCurrentVersion();

                if (SaveManager::savePlayerSnapshot(player, accountName, result.difficulty))
                {
                    adaptationChanges.push_back("Marque de version mise à jour vers V" + VersionInfo::currentVersion() + ".");
                }
                else
                {
                    adaptationChanges.push_back("Attention : nouvelle version détectée, mais sauvegarde de la marque de version impossible.");
                }
            }

            if (result.characterLoaded)
            {
                result.playerName = player.getName();

                Console::clear();

                std::vector<std::string> summaryLines = {
                    "Personnage incarné : " + result.playerName + ".",
                    "Race : " + player.getRaceText(),
                    "Classe : " + player.getType(),
                    "PV : " + std::to_string(player.getHp()) + "/" + std::to_string(player.getMaxHp()),
                    "Or : " + std::to_string(player.getInventory().getGold()) + " pièces"
                };

                if (!adaptationChanges.empty())
                {
                    summaryLines.push_back(legacyDecision == LegacyCharacterDecision::HeavyAdaptation
                        ? "Adaptation lourde appliquée :"
                        : "Mise à jour de version appliquée :");

                    for (const std::string& change : adaptationChanges)
                    {
                        summaryLines.push_back("- " + change);
                    }
                }

                summaryLines.push_back("Arme équipée : " + (player.hasEquippedWeapon() ? player.getEquippedWeapon().getName() : std::string("aucune")));
                summaryLines.push_back("Armure équipée : " + (player.hasEquippedArmor() ? player.getEquippedArmor().getName() : std::string("aucune")));
                summaryLines.push_back("Dégâts : " + std::to_string(player.getMinDamage()) + "-" + std::to_string(player.getMaxDamage())
                    + " | Critique : " + std::to_string(player.getCriticalDamage()));

                MessageScreen::show(
                    "PERSONNAGE INCARNÉ",
                    "save.characters.loaded.summary",
                    summaryLines
                );

                return result;
            }

            Console::clear();
            MessageScreen::show(
                "CHARGEMENT IMPOSSIBLE",
                "save.characters.load.failed",
                {"Impossible de charger ce personnage. Il faudra en choisir un autre ou en créer un nouveau."}
            );
            Console::clear();
            continue;
        }

        if (characterAction == 2)
        {
            Console::clear();

            std::string targetAccount = MessageScreen::askText(
                "TRANSFERT DE MAÎTRISE",
                "save.characters.transfer.target",
                {
                    "Transfert de maîtrise irréversible.",
                    selectedCharacter.characterName + " gardera son créateur inscrit : " + selectedCharacter.creatorAccountName + ".",
                    "Mais son joueur / maître actuel deviendra le compte que tu vas indiquer.",
                    "Après ça, ce compte ne pourra plus l'incarner.",
                    "Nom exact du compte destinataire :"
                }
            );

            if (targetAccount.empty() || targetAccount == accountName)
            {
                MessageScreen::show(
                    "TRANSFERT ANNULÉ",
                    "save.characters.transfer.invalid_target",
                    {"Transfert annulé : le destinataire est vide ou identique au compte actuel."}
                );
                continue;
            }

            if (!MessageScreen::askKeywordConfirmation(
                    "CONFIRMATION DE TRANSFERT",
                    "save.characters.transfer.confirm",
                    {
                        "Personnage : " + selectedCharacter.characterName,
                        "Créateur conservé : " + selectedCharacter.creatorAccountName,
                        "Nouveau joueur / maître : " + targetAccount
                    },
                    "TRANSFERER"
                ))
            {
                MessageScreen::show(
                    "TRANSFERT ANNULÉ",
                    "save.characters.transfer.cancelled",
                    {"Transfert annulé."}
                );
                continue;
            }

            if (SaveManager::transferCharacterOwnership(selectedCharacter, targetAccount))
            {
                MessageScreen::show(
                    "MAÎTRISE TRANSFÉRÉE",
                    "save.characters.transfer.success",
                    {
                        "Le fil de maîtrise a été déplacé.",
                        "Créateur : " + selectedCharacter.creatorAccountName,
                        "Nouveau joueur / maître : " + targetAccount
                    }
                );
            }
            else
            {
                MessageScreen::show(
                    "TRANSFERT IMPOSSIBLE",
                    "save.characters.transfer.failure",
                    {"Transfert impossible. Vérifie que le destinataire n'a pas déjà un personnage du même nom."}
                );
            }

            continue;
        }

        if (characterAction == 3)
        {
            Console::clear();
            std::string exportedPath;

            MessageScreen::show(
                "EXTRACTION DU PERSONNAGE",
                "save.characters.export.warning",
                {
                    selectedCharacter.characterName + " part en voyage.",
                    "L'extraction transfère le personnage : il quitte ce compte local.",
                    "Le compte reste présent, mais ce personnage ne sera plus jouable ici tant qu'il n'est pas réimporté."
                },
                false
            );

            if (SaveManager::exportCharacterPackage(selectedCharacter, exportedPath))
            {
                MessageScreen::show(
                    "PERSONNAGE EXTRAIT",
                    "save.characters.export.success",
                    {
                        "Personnage extrait avec succès.",
                        "Dossier portable : " + exportedPath
                    }
                );
            }
            else
            {
                MessageScreen::show(
                    "EXTRACTION IMPOSSIBLE",
                    "save.characters.export.failure",
                    {"Extraction impossible pour ce personnage."}
                );
            }

            continue;
        }

        if (characterAction == 4)
        {
            Console::clear();
            std::string exportedPath;

            MessageScreen::show(
                "CLONE PORTABLE",
                "save.characters.clone.warning",
                {
                    "Création d'un clone portable de " + selectedCharacter.characterName + ".",
                    "Le personnage original reste ici.",
                    "Le clone sera marqué CLONE et limité aux combats JcJ amicaux."
                },
                false
            );

            if (SaveManager::exportCharacterClonePackage(selectedCharacter, exportedPath))
            {
                MessageScreen::show(
                    "CLONE EXTRAIT",
                    "save.characters.clone.success",
                    {
                        "Clone extrait avec succès.",
                        "Dossier portable : " + exportedPath
                    }
                );
            }
            else
            {
                MessageScreen::show(
                    "EXTRACTION IMPOSSIBLE",
                    "save.characters.clone.failure",
                    {"Extraction du clone impossible."}
                );
            }

            continue;
        }

        if (!MessageScreen::askKeywordConfirmation(
                "SUPPRESSION DE PERSONNAGE",
                "save.characters.delete.confirm",
                {"Supprimer ce personnage est définitif."},
                "SUPPRIMER"
            ))
        {
            Console::clear();
            MessageScreen::show(
                "SUPPRESSION ANNULÉE",
                "save.characters.delete.cancelled",
                {"Suppression annulée."}
            );
            continue;
        }

        Console::clear();

        if (SaveManager::deletePlayableCharacter(selectedCharacter))
        {
            MessageScreen::show(
                "PERSONNAGE SUPPRIMÉ",
                "save.characters.delete.success",
                {"Personnage supprimé : " + selectedCharacter.characterName + "."}
            );
        }
        else
        {
            MessageScreen::show(
                "SUPPRESSION IMPOSSIBLE",
                "save.characters.delete.failure",
                {
                    "Impossible de supprimer ce personnage.",
                    "Vérifie les fichiers dans assets/saves/characters/playable/."
                }
            );
        }
    }
}
