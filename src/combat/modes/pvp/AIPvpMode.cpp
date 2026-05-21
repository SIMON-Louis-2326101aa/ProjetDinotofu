// EN: AIPvpMode.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: AIPvpMode.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "combat/modes/pvp/AIPvpMode.hpp"

#include "combat/TurnManager.hpp"
#include "combat/ai/CombatAI.hpp"
#include "combat/summon/SummonRules.hpp"
#include "combat/summon/SummonCombatSystem.hpp"
#include "combat/threat/ThreatSystem.hpp"
#include "combat/group/CombatGroupBuilder.hpp"
#include "combat/group/CombatUnitKind.hpp"
#include "combat/role/CombatRoleActionSystem.hpp"

#include "character/RandomCharacterGenerator.hpp"
#include "character/SpecialCharacterCatalog.hpp"
#include "character/SpecialCharacterDialogueCatalog.hpp"
#include "character/SpecialCharacterNativeBonus.hpp"
#include "class_system/ClassCatalog.hpp"
#include "core/Console.hpp"
#include "interface/CombatDisplay.hpp"

#include <iostream>
#include <string>
#include <vector>

namespace
{
    // EN: askClassManually declares or implements a focused behavior used by this module.
    // FR: askClassManually déclare ou implémente un comportement précis utilisé par ce module.
    PlayerClass askClassManually(const std::string& targetName)
    {
        Console::clear();

        std::cout << "Choisis une famille de classe pour " << targetName << "." << std::endl;
        std::cout << "Tu ne choisis pas vraiment l'identité de l'adversaire : seulement son style de combat." << std::endl;
        std::cout << std::endl;

        ClassCatalog::displayClassCategories();

        std::cout << std::endl;
        std::cout << "> ";

        int categoryChoice = Console::askNumberBetween(
            1,
            ClassCatalog::getClassCategoryCount(),
            "Veuillez entrer un chiffre correspondant à une famille affichée."
        );

        Console::clear();

        std::cout << "Famille sélectionnée : "
                  << ClassCatalog::getClassCategoryNameByChoice(categoryChoice)
                  << "."
                  << std::endl;
        std::cout << "Choisis maintenant la classe de l'adversaire." << std::endl;
        std::cout << "Son nom et sa race seront tirés aléatoirement." << std::endl;
        std::cout << std::endl;

        ClassCatalog::displayClassesByCategoryChoice(categoryChoice);

        int maxClassChoice = ClassCatalog::getPlayableClassCountByCategoryChoice(categoryChoice);

        std::cout << "Veuillez entrer uniquement le chiffre correspondant." << std::endl;
        std::cout << "> ";

        int classChoice = Console::askNumberBetween(
            1,
            maxClassChoice,
            "Veuillez entrer un chiffre correspondant à une classe affichée."
        );

        return ClassCatalog::createClassByCategoryChoice(categoryChoice, classChoice);
    }

    // EN: applyMattProUniversalBonus declares or implements a focused behavior used by this module.
    // FR: applyMattProUniversalBonus déclare ou implémente un comportement précis utilisé par ce module.
    void applyMattProUniversalBonus(Player& matt)
    {
        matt.applyFlatStatBonus(
            20,
            2,
            4,
            6
        );
    }

    // EN: createMattOpponent declares or implements a focused behavior used by this module.
    // FR: createMattOpponent déclare ou implémente un comportement précis utilisé par ce module.
    Player createMattOpponent(Random& random)
    {
        std::cout << "Matt (PRO) est entré dans l'arène." << std::endl;
        std::cout << "Il n'a pas vraiment de spécialité : il est juste meilleur que prévu, quelle que soit sa classe." << std::endl;
        std::cout << std::endl;

        Console::pauseSeconds(1);

        std::cout << "Choisis comment Matt obtiendra sa classe :" << std::endl;
        std::cout << std::endl;
        std::cout << "1 : Classe aléatoire" << std::endl;
        std::cout << "2 : Choisir sa classe toi-même" << std::endl;
        std::cout << std::endl;
        std::cout << "> ";

        int classChoiceType = Console::askNumberBetween(
            1,
            2,
            "Veuillez entrer un chiffre valide : 1 ou 2."
        );

        PlayerClass mattClass;

        if (classChoiceType == 1)
        {
            int aiClassChoice = random.between(1, ClassCatalog::getPlayableClassCount());
            mattClass = ClassCatalog::createBaseClass(aiClassChoice);
        }
        else
        {
            std::cout << "Pas très fair-play, mais l'arène accepte ce genre de petit caprice." << std::endl;
            Console::pauseSeconds(1);

            mattClass = askClassManually("Matt (PRO)");
        }

        Player ai("Matt (PRO)", mattClass);
        ai.setRace(CharacterRace::Human);
        ai.initializeStarterInventory();
        applyMattProUniversalBonus(ai);

        return ai;
    }

    // EN: createChosenSpecialOpponent declares or implements a focused behavior used by this module.
    // FR: createChosenSpecialOpponent déclare ou implémente un comportement précis utilisé par ce module.
    Player createChosenSpecialOpponent()
    {
        Console::clear();

        std::vector<SpecialCharacter> characters = SpecialCharacterCatalog::getAllSpecialCharacters();

        std::cout << "========== PERSONNAGES SPÉCIAUX ==========" << std::endl;
        std::cout << "Le code a ouvert une porte que l'arène garde normalement rare." << std::endl;
        std::cout << "Tu peux provoquer un personnage spécial précis, y compris Matt (PRO)." << std::endl;
        std::cout << std::endl;

        for (std::size_t index = 0; index < characters.size(); ++index)
        {
            const SpecialCharacter& character = characters[index];
            std::cout << index + 1 << " : " << character.getName()
                      << " | Race : " << character.getRaceText()
                      << " | Classe native : " << character.getNativeClass()
                      << std::endl;
            std::cout << "    " << character.getCombatStyle() << std::endl;
        }

        std::cout << std::endl;
        std::cout << "> ";

        int choice = Console::askNumberBetween(
            1,
            static_cast<int>(characters.size()),
            "Veuillez choisir un personnage spécial affiché."
        );

        const SpecialCharacter& selected = characters[static_cast<std::size_t>(choice - 1)];

        Player opponent(
            selected.getName(),
            ClassCatalog::createClassByName(selected.getNativeClass())
        );

        opponent.setRace(selected.getRace());
        opponent.initializeStarterInventory();

        SpecialCharacterNativeBonus::applyForSpecialCharacter(
            opponent,
            selected
        );

        Console::clear();
        std::cout << selected.getName() << " a été appelé directement par le registre altéré." << std::endl;
        std::cout << "Ce n'est plus une rencontre rare : c'est un défi provoqué." << std::endl;
        std::cout << std::endl;

        return opponent;
    }

    // EN: createClassicOpponentFromChosenClass declares or implements a focused behavior used by this module.
    // FR: createClassicOpponentFromChosenClass déclare ou implémente un comportement précis utilisé par ce module.
    Player createClassicOpponentFromChosenClass(Random& random)
    {
        Console::clear();

        std::cout << "Création d'un adversaire classique par style de combat." << std::endl;
        std::cout << "Cette option ne peut pas faire apparaître de personnage spécial." << std::endl;
        std::cout << "Tu choisis seulement la classe. Le nom et la race seront tirés par l'arène." << std::endl;
        std::cout << std::endl;

        Console::pauseSeconds(1);

        PlayerClass opponentClass = askClassManually("l'adversaire");

        Player opponent = RandomCharacterGenerator::generateClassicOpponentWithClass(
            opponentClass,
            random
        );

        return opponent;
    }
}

// EN: run declares or implements a focused behavior used by this module.
// FR: run déclare ou implémente un comportement précis utilisé par ce module.
void AIPvpMode::run(Player& player1, Random& random)
{
    std::cout << "Préparation de l'IA..." << std::endl;
    std::cout << std::endl;

    Console::pauseSeconds(1);

    std::cout << "Choisis le type d'adversaire IA :" << std::endl;
    std::cout << std::endl;
    std::cout << "1 : Matt (PRO)" << std::endl;
    std::cout << "    L'adversaire référence. Il est meilleur globalement, peu importe sa classe." << std::endl;
    std::cout << std::endl;
    std::cout << "2 : Tirage d'arène aléatoire" << std::endl;
    std::cout << "    Nom, race et classe aléatoires. Peut appeler un personnage spécial." << std::endl;
    std::cout << "    Chance actuelle de personnage spécial : "
              << RandomCharacterGenerator::SPECIAL_ARENA_SPAWN_PERCENTAGE
              << "%" << std::endl;
    std::cout << std::endl;
    std::cout << "3 : Choisir une classe d'adversaire" << std::endl;
    std::cout << "    Tu choisis le style de combat, mais le nom et la race restent aléatoires." << std::endl;
    std::cout << "    Aucun personnage spécial ne peut apparaître avec cette option." << std::endl;
    std::cout << std::endl;

    int maxOpponentChoice = 3;

    if (player1.hasSpecialChallengeAccess())
    {
        maxOpponentChoice = 4;
        std::cout << "4 : Spéciaux" << std::endl;
        std::cout << "    Liste tous les personnages spéciaux et permet d'en provoquer un directement." << std::endl;
        std::cout << "    Cette option vient d'une donnée altérée." << std::endl;
        std::cout << std::endl;
    }

    std::cout << "> ";

    int opponentChoice = Console::askNumberBetween(
        1,
        maxOpponentChoice,
        "Veuillez entrer un chiffre valide affiché."
    );

    Player ai;

    if (opponentChoice == 1)
    {
        ai = createMattOpponent(random);
    }
    else if (opponentChoice == 2)
    {
        ai = RandomCharacterGenerator::generateArenaOpponent(random);
    }
    else if (opponentChoice == 3)
    {
        ai = createClassicOpponentFromChosenClass(random);
    }
    else
    {
        ai = createChosenSpecialOpponent();
    }

    Console::clear();

    std::cout << ai.getName() << " entre dans l'arène." << std::endl;
    std::cout << "Race : " << ai.getRaceText() << "." << std::endl;
    std::cout << "Classe : " << ai.getType() << "." << std::endl;
    std::cout << "Ses statistiques ont été gravées dans l'arène." << std::endl;

    if (SpecialCharacterDialogueCatalog::hasDialogueFor(ai.getName()))
    {
        std::cout << std::endl;
        SpecialCharacterDialogueCatalog::displayEntranceDialogue(ai.getName());
    }

    if (SummonRules::classCanSummon(ai.getType()))
    {
        std::cout << std::endl;
        std::cout << "Note : " << SummonRules::getSummonWarningText(ai.getType()) << std::endl;
        std::cout << "Un 1v1 peut vite devenir un PvE miniature quand quelqu'un appelle des renforts." << std::endl;
    }

    std::cout << std::endl;

    Console::pauseSeconds(2);

    std::vector<Summon> playerSummons = SummonCombatSystem::createInitialSummonsFor(player1);
    std::vector<Summon> aiSummons = SummonCombatSystem::createInitialSummonsFor(ai);

    SummonCombatSystem::displaySummonArrival(player1, playerSummons);

    CombatGroup playerGroupPreview = CombatGroupBuilder::buildSideFromEntityAndSummons(
        player1,
        playerSummons,
        CombatSide::PlayerSide,
        CombatUnitKind::MainFighter
    );

    CombatGroupBuilder::displayGroup(
        playerGroupPreview,
        "GROUPE DU JOUEUR"
    );

    CombatRoleActionSystem::displayRoleIdentity(player1);

    SummonControlMode playerSummonControlMode =
        SummonCombatSystem::askPlayerSummonControlMode(player1, playerSummons);

    SummonCombatSystem::displaySummonArrival(ai, aiSummons);

    CombatGroup enemyGroupPreview = CombatGroupBuilder::buildSideFromEntityAndSummons(
        ai,
        aiSummons,
        CombatSide::EnemySide,
        CombatUnitKind::Enemy
    );

    CombatGroupBuilder::displayGroup(
        enemyGroupPreview,
        "GROUPE ADVERSE"
    );

    CombatRoleActionSystem::displayRoleIdentity(ai);

    int turn = random.chooseFirstTurn();

    std::cout << "Prépare-toi..." << std::endl;
    Console::pauseSeconds(2);
    std::cout << "Le combat contre " << ai.getName() << " commence maintenant." << std::endl;
    std::cout << std::endl;

    while (!player1.isDead() && !ai.isDead())
    {
        bool turnFinished = false;

        if (turn == 1)
        {
            turnFinished = TurnManager::playHumanTurnWithEnemySummons(
                player1,
                ai,
                aiSummons,
                random,
                POTION_HEAL_AMOUNT,
                POTION_DAMAGE_BONUS
            );

            if (turnFinished)
            {
                if (!ai.isDead() && SummonCombatSystem::hasActiveSummons(playerSummons))
                {
                    SummonCombatSystem::playPlayerSummonTurnsAgainstEntity(
                        playerSummons,
                        ai,
                        random,
                        playerSummonControlMode
                    );
                }

                turn = 2;
            }
        }
        else
        {
            CombatRoleActionSystem::tryActivateAutomaticRoleReaction(player1, random);

            if (ThreatSystem::shouldForceTargetMainEntity(player1, ai.getName()))
            {
                ThreatSystem::notifyForcedTarget(player1, ai.getName());
            }
            else if (SummonCombatSystem::hasTargetableSummons(playerSummons)
                && random.between(1, 100) <= CombatAI::getSummonTargetPriorityChance(ai))
            {
                int summonIndex = SummonCombatSystem::chooseStrategicTargetableSummonIndex(
                    playerSummons,
                    ai,
                    random
                );

                if (summonIndex >= 0)
                {
                    std::cout << ai.getName()
                              << " choisit de briser une invocation avant de viser l'invocateur."
                              << std::endl;
                    std::cout << std::endl;

                    SummonCombatSystem::entityAttacksSummon(
                        ai,
                        playerSummons[summonIndex],
                        random
                    );

                    SummonCombatSystem::removeInactiveSummons(playerSummons);
                    turnFinished = true;
                }
            }

            if (!turnFinished)
            {
                turnFinished = TurnManager::playAITurn(
                    ai,
                    player1,
                    random,
                    POTION_HEAL_AMOUNT,
                    POTION_DAMAGE_BONUS
                );

                if (turnFinished)
                {
                    ThreatSystem::consumeForcedTargetIfNeeded(player1);
                }
            }

            if (turnFinished)
            {
                if (!player1.isDead() && SummonCombatSystem::hasActiveSummons(aiSummons))
                {
                    SummonCombatSystem::playSummonTurnsAgainstEntity(
                        aiSummons,
                        player1,
                        random
                    );
                }

                turn = 1;
            }
        }
    }

    CombatDisplay::displayCombatResult(player1, ai);

    if (SpecialCharacterDialogueCatalog::hasDialogueFor(ai.getName()))
    {
        if (ai.isDead())
        {
            SpecialCharacterDialogueCatalog::displayDefeatDialogue(ai.getName());
        }
        else if (player1.isDead())
        {
            SpecialCharacterDialogueCatalog::displayVictoryDialogue(ai.getName());
        }
    }
}

