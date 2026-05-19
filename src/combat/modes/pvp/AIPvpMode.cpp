// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "combat/modes/pvp/AIPvpMode.hpp"

#include "combat/TurnManager.hpp"
#include "combat/summon/SummonRules.hpp"
#include "combat/summon/SummonCombatSystem.hpp"
#include "combat/threat/ThreatSystem.hpp"
#include "combat/group/CombatGroupBuilder.hpp"
#include "combat/group/CombatUnitKind.hpp"
#include "combat/role/CombatRoleActionSystem.hpp"

#include "character/RandomCharacterGenerator.hpp"
#include "character/SpecialCharacterDialogueCatalog.hpp"
#include "class_system/ClassCatalog.hpp"
#include "core/Console.hpp"
#include "interface/CombatDisplay.hpp"

#include <iostream>
#include <string>
#include <vector>

namespace
{
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

    void applyMattProUniversalBonus(Player& matt)
    {
        matt.applyFlatStatBonus(
            20,
            2,
            4,
            6
        );
    }

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
    std::cout << "> ";

    int opponentChoice = Console::askNumberBetween(
        1,
        3,
        "Veuillez entrer un chiffre valide entre 1 et 3."
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
    else
    {
        ai = createClassicOpponentFromChosenClass(random);
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
                && random.between(1, 100) <= 35)
            {
                int summonIndex = SummonCombatSystem::chooseRandomTargetableSummonIndex(
                    playerSummons,
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
}
