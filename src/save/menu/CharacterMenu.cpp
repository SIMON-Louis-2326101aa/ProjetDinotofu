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
        std::cout << question << std::endl;
        std::cout << "1 : Oui" << std::endl;
        std::cout << "2 : Non" << std::endl;
        std::cout << "> ";

        int choice = Console::askNumberBetween(
            1,
            2,
            "Veuillez choisir 1 ou 2."
        );

        return choice == 1;
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

        std::cout << "==============================================" << std::endl;
        std::cout << message << std::endl;
        std::cout << "Profil concerné : " << accountName << std::endl;
        std::cout << "Personnage concerné : " << character.characterName << std::endl;
        std::cout << "Version actuelle : V" << VersionInfo::currentVersion() << std::endl;
        std::cout << "Dernière adaptation connue : V" << character.lastAdaptedVersion << std::endl;

        if (impact == VersionCompatibilityImpact::RecreateRecommended)
        {
            std::cout << "Certaines nouveautés de départ peuvent ne pas être présentes : équipement, munitions, potions, équilibrage initial." << std::endl;
        }

        std::cout << "==============================================" << std::endl;
        std::cout << std::endl;
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

        std::cout << "Les archives sentent une ancienne version autour de ce personnage." << std::endl;
        std::cout << "Son histoire peut continuer, mais certains fils de départ n'existaient pas encore" << std::endl;
        std::cout << "dans cette version du monde : kit initial, munitions, équipement de secours, protections." << std::endl;
        std::cout << std::endl;
        std::cout << "Profil concerné : " << accountName << std::endl;
        std::cout << "Personnage concerné : " << character.characterName << std::endl;
        std::cout << std::endl;
        std::cout << "Que veux-tu faire ?" << std::endl;
        std::cout << "0 : Retour" << std::endl;
        std::cout << "1 : Procéder au rituel d'oubli contrôlé" << std::endl;
        std::cout << "    Le personnage sera recréé proprement pour cette version." << std::endl;
        std::cout << "2 : Subir une adaptation lourde" << std::endl;
        std::cout << "    Le jeu ajoute seulement les sécurités manquantes sans remplacer tes gains." << std::endl;
        std::cout << std::endl;
        std::cout << "> ";

        int choice = Console::askNumberBetween(
            0,
            2,
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
            std::cout << "Cette identité n'est pas prévue pour être incarnée." << std::endl;
            std::cout << std::endl;
            Console::waitForEnter();
            Console::clear();
            return false;
        }

        if (!askYesNo("Veux-tu vraiment tenter d'usurper cette identité ?"))
        {
            Console::clear();
            std::cout << "Très bien. L'arène préfère les noms qui appartiennent vraiment à leur porteur." << std::endl;
            std::cout << std::endl;
            Console::waitForEnter();
            Console::clear();
            return false;
        }

        std::cout << std::endl;
        std::cout << "Pour prouver que c'est bien toi, entre sa date spéciale au format DD/MM/YYYY." << std::endl;
        std::cout << "> ";

        std::string dateInput;
        std::getline(std::cin >> std::ws, dateInput);

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
        std::cout << "Race liée à l'identité : " << protectedCharacter.getRaceText() << "." << std::endl;
        std::cout << "Tu pourras quand même choisir ta classe." << std::endl;
        std::cout << "Si tu choisis sa classe native, le bonus natif pourra s'activer." << std::endl;
        std::cout << std::endl;
        Console::waitForEnter();
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

        std::cout << "===== PERSONNAGES =====" << std::endl;
        std::cout << "0 : Créer un nouveau personnage" << std::endl;

        for (int i = 0; i < static_cast<int>(characters.size()); i++)
        {
            std::cout << (i + 1) << " : "
                      << characters[i].characterName
                      << " | " << characters[i].raceName
                      << " / " << characters[i].className
                      << " | Niveau " << characters[i].level
                      << (characters[i].clone ? " | CLONE" : "")
                      << std::endl;
        }

        std::cout << "=======================" << std::endl;
        std::cout << std::endl;
        std::cout << "> ";

        int choice = Console::askNumberBetween(
            0,
            static_cast<int>(characters.size()),
            "Veuillez choisir un personnage affiché, ou 0 pour en créer un."
        );

        if (choice == 0)
        {
            CharacterMenuResult result = createEmptyResult();

            while (true)
            {
                std::cout << "Quel est ton nom ?" << std::endl;
                std::cout << "> ";

                std::getline(std::cin >> std::ws, result.playerName);

                while (result.playerName.empty())
                {
                    std::cout << "Un nom vide ? Même les gobelins ont plus de présence que ça." << std::endl;
                    std::cout << "Entre un vrai nom." << std::endl;
                    std::cout << "> ";

                    std::getline(std::cin >> std::ws, result.playerName);
                }

                if (validateProtectedName(result.playerName, result))
                {
                    break;
                }
            }

            Console::clear();

            std::cout << "Très bien, " << result.playerName << "." << std::endl;
            std::cout << "L'arène se souviendra peut-être de ce nom..." << std::endl;
            std::cout << std::endl;

            Console::waitForEnter();
            Console::clear();
            return result;
        }

        CharacterSaveSummary selectedCharacter = characters[choice - 1];

        Console::clear();
        std::cout << "Personnage sélectionné : " << selectedCharacter.characterName << std::endl;
        std::cout << "Race / classe : " << selectedCharacter.raceName << " / " << selectedCharacter.className << std::endl;
        std::cout << "Niveau : " << selectedCharacter.level << std::endl;
        std::cout << "Créateur : " << selectedCharacter.creatorAccountName << std::endl;
        std::cout << "Joueur / maître actuel : " << selectedCharacter.currentOwnerAccountName << std::endl;
        std::cout << "Créé le : " << selectedCharacter.createdAt
                  << " | V" << selectedCharacter.createdForVersion << std::endl;
        std::cout << "Dernière adaptation faite pour la V"
                  << selectedCharacter.lastAdaptedVersion << std::endl;
        if (selectedCharacter.clone)
        {
            std::cout << "Statut : CLONE — JcJ amical uniquement." << std::endl;
        }
        std::cout << std::endl;

        if (selectedCharacter.accountName != accountName
            || selectedCharacter.currentOwnerAccountName != accountName)
        {
            std::cout << "Une marque étrangère traverse cette sauvegarde." << std::endl;
            std::cout << "Ce personnage n'appartient pas à ce compte, même si son fichier a été déplacé ici." << std::endl;
            std::cout << "Un personnage n'a qu'un seul maître." << std::endl;
            std::cout << "Maître inscrit : " << selectedCharacter.currentOwnerAccountName << std::endl;
            std::cout << "Compte actif : " << accountName << std::endl;
            std::cout << std::endl;
            std::cout << "Chargement refusé. Utilise le transfert irréversible prévu par le jeu si le personnage doit changer de joueur." << std::endl;
            std::cout << std::endl;
            Console::waitForEnter();
            Console::clear();
            continue;
        }

        displayVersionCompatibilityWarning(accountName, selectedCharacter);
        std::cout << "0 : Retour" << std::endl;
        std::cout << "1 : Incarner" << std::endl;
        std::cout << "2 : Donner ce personnage à un autre compte" << std::endl;
        std::cout << "    Action irréversible : le maître actuel changera vraiment." << std::endl;
        std::cout << "3 : Extraire / transférer ce personnage en dossier portable" << std::endl;
        std::cout << "4 : Extraire un clone du personnage" << std::endl;
        std::cout << "5 : Supprimer ce personnage" << std::endl;
        std::cout << std::endl;
        std::cout << "> ";

        int characterAction = Console::askNumberBetween(
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
                std::cout << "Le nom est conservé, mais le fil sera retissé depuis le début." << std::endl;
                std::cout << "Choisis à nouveau la difficulté, la race et la classe pour recréer proprement ce personnage." << std::endl;
                std::cout << std::endl;
                Console::waitForEnter();
                Console::clear();
                return result;
            }

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

            if (result.characterLoaded)
            {
                result.playerName = player.getName();

                Console::clear();

                std::cout << "Personnage incarné : " << result.playerName << "." << std::endl;
                std::cout << "Race : " << player.getRaceText() << std::endl;
                std::cout << "Classe : " << player.getType() << std::endl;
                std::cout << std::endl;

                if (!adaptationChanges.empty())
                {
                    std::cout << "Adaptation lourde appliquée :" << std::endl;
                    for (const std::string& change : adaptationChanges)
                    {
                        std::cout << "- " << change << std::endl;
                    }
                    std::cout << std::endl;
                }

                player.displayStats();
                player.displaySimpleEquipment();

                Console::waitForEnter();
                Console::clear();
                return result;
            }

            Console::clear();
            std::cout << "Impossible de charger ce personnage. Il faudra en choisir un autre ou en créer un nouveau." << std::endl;
            std::cout << std::endl;
            Console::waitForEnter();
            Console::clear();
            continue;
        }

        if (characterAction == 2)
        {
            Console::clear();

            std::cout << "Transfert de maîtrise irréversible." << std::endl;
            std::cout << selectedCharacter.characterName << " gardera son créateur inscrit : "
                      << selectedCharacter.creatorAccountName << "." << std::endl;
            std::cout << "Mais son joueur / maître actuel deviendra le compte que tu vas indiquer." << std::endl;
            std::cout << "Après ça, ce compte ne pourra plus l'incarner." << std::endl;
            std::cout << std::endl;
            std::cout << "Nom exact du compte destinataire :" << std::endl;
            std::cout << "> ";

            std::string targetAccount;
            std::getline(std::cin >> std::ws, targetAccount);

            if (targetAccount.empty() || targetAccount == accountName)
            {
                std::cout << "Transfert annulé : le destinataire est vide ou identique au compte actuel." << std::endl;
                std::cout << std::endl;
                Console::waitForEnter();
                Console::clear();
                continue;
            }

            std::cout << std::endl;
            std::cout << "Tape TRANSFERER pour confirmer." << std::endl;
            std::cout << "> ";

            std::string confirmation;
            std::getline(std::cin >> std::ws, confirmation);

            if (confirmation != "TRANSFERER")
            {
                std::cout << "Transfert annulé." << std::endl;
                std::cout << std::endl;
                Console::waitForEnter();
                Console::clear();
                continue;
            }

            if (SaveManager::transferCharacterOwnership(selectedCharacter, targetAccount))
            {
                std::cout << "Le fil de maîtrise a été déplacé." << std::endl;
                std::cout << "Créateur : " << selectedCharacter.creatorAccountName << std::endl;
                std::cout << "Nouveau joueur / maître : " << targetAccount << std::endl;
            }
            else
            {
                std::cout << "Transfert impossible. Vérifie que le destinataire n'a pas déjà un personnage du même nom." << std::endl;
            }

            std::cout << std::endl;
            Console::waitForEnter();
            Console::clear();
            continue;
        }

        if (characterAction == 3)
        {
            Console::clear();
            std::string exportedPath;

            std::cout << selectedCharacter.characterName << " part en voyage." << std::endl;
            std::cout << "L'extraction transfère le personnage : il quitte ce compte local." << std::endl;
            std::cout << "Le compte reste présent, mais ce personnage ne sera plus jouable ici tant qu'il n'est pas réimporté." << std::endl;
            std::cout << std::endl;

            if (SaveManager::exportCharacterPackage(selectedCharacter, exportedPath))
            {
                std::cout << "Personnage extrait avec succès." << std::endl;
                std::cout << "Dossier portable : " << exportedPath << std::endl;
            }
            else
            {
                std::cout << "Extraction impossible pour ce personnage." << std::endl;
            }

            std::cout << std::endl;
            Console::waitForEnter();
            Console::clear();
            continue;
        }

        if (characterAction == 4)
        {
            Console::clear();
            std::string exportedPath;

            std::cout << "Création d'un clone portable de " << selectedCharacter.characterName << "." << std::endl;
            std::cout << "Le personnage original reste ici." << std::endl;
            std::cout << "Le clone sera marqué CLONE et limité aux combats JcJ amicaux." << std::endl;
            std::cout << std::endl;

            if (SaveManager::exportCharacterClonePackage(selectedCharacter, exportedPath))
            {
                std::cout << "Clone extrait avec succès." << std::endl;
                std::cout << "Dossier portable : " << exportedPath << std::endl;
            }
            else
            {
                std::cout << "Extraction du clone impossible." << std::endl;
            }

            std::cout << std::endl;
            Console::waitForEnter();
            Console::clear();
            continue;
        }

        std::cout << std::endl;
        std::cout << "Supprimer ce personnage est définitif." << std::endl;
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

        if (SaveManager::deletePlayableCharacter(selectedCharacter))
        {
            std::cout << "Personnage supprimé : " << selectedCharacter.characterName << "." << std::endl;
        }
        else
        {
            std::cout << "Impossible de supprimer ce personnage." << std::endl;
            std::cout << "Vérifie les fichiers dans assets/saves/characters/playable/." << std::endl;
        }

        std::cout << std::endl;
        Console::waitForEnter();
        Console::clear();
    }
}
