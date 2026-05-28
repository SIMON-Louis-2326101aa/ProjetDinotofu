// EN: AIPvpMode.cpp implements the player-versus-AI arena mode.
// FR: AIPvpMode.cpp implémente le mode arène joueur contre IA.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "combat/modes/pvp/AIPvpMode.hpp"

#include "combat/TurnManager.hpp"
#include "combat/ai/CombatAI.hpp"
#include "combat/group/CombatGroupBuilder.hpp"
#include "combat/group/CombatUnitKind.hpp"
#include "combat/role/CombatRoleActionSystem.hpp"
#include "combat/summon/SummonCombatSystem.hpp"
#include "combat/summon/SummonRules.hpp"
#include "combat/threat/ThreatSystem.hpp"

#include "character/RandomCharacterGenerator.hpp"
#include "character/SpecialCharacterCatalog.hpp"
#include "character/SpecialCharacterDialogueCatalog.hpp"
#include "character/SpecialCharacterNativeBonus.hpp"
#include "class_system/ClassCatalog.hpp"
#include "core/Console.hpp"
#include "interface/CombatDisplay.hpp"
#include "interface/TerminalInterface.hpp"
#include "interface/menu/common/MessageScreen.hpp"
#include "interface/model/MenuScreen.hpp"

#include <iostream>
#include <string>
#include <vector>

namespace
{
    PlayerClass askClassManually(const std::string& targetName)
    {
        Console::clear();

        MenuScreen categoryScreen("STYLE DE COMBAT DE " + targetName, "pvp.ai.class.category");
        categoryScreen.addLine("Tu ne choisis pas vraiment l'identité de l'adversaire : seulement son style de combat.");
        categoryScreen.addLine("Le nom, la race et l'équipement restent gérés par l'arène.");

        for (int i = 1; i <= ClassCatalog::getClassCategoryCount(); ++i)
        {
            categoryScreen.addOption(
                i,
                ClassCatalog::getClassCategoryNameByChoice(i),
                std::to_string(ClassCatalog::getPlayableClassCountByCategoryChoice(i)) + " classes disponibles",
                true,
                "pvp.ai.class.category." + std::to_string(i)
            );
        }

        int categoryChoice = TerminalInterface::askMenuChoiceFromOptions(
            categoryScreen,
            "Veuillez choisir une famille affichée."
        );

        Console::clear();

        std::vector<ClassOptionInfo> classOptions = ClassCatalog::getClassOptionsByCategoryChoice(categoryChoice);
        MenuScreen classScreen("CLASSE DE " + targetName, "pvp.ai.class.choice");
        classScreen.addLine("Famille sélectionnée : " + ClassCatalog::getClassCategoryNameByChoice(categoryChoice) + ".");
        classScreen.addLine("Choisis une classe précise pour fixer le rythme du duel.");

        for (std::size_t i = 0; i < classOptions.size(); ++i)
        {
            const ClassOptionInfo& info = classOptions[i];
            classScreen.addOption(
                static_cast<int>(i + 1),
                info.name,
                "Rôle : " + info.role
                    + " | PV " + std::to_string(info.maxHp)
                    + " | Dégâts " + std::to_string(info.minDamage) + "-" + std::to_string(info.maxDamage)
                    + " | Crit " + std::to_string(info.criticalDamage),
                true,
                "pvp.ai.class.choice." + std::to_string(i + 1)
            );
        }

        int classChoice = TerminalInterface::askMenuChoiceFromOptions(
            classScreen,
            "Veuillez choisir une classe affichée."
        );

        return ClassCatalog::createClassByCategoryChoice(categoryChoice, classChoice);
    }

    void applyMattProUniversalBonus(Player& matt)
    {
        matt.applyFlatStatBonus(20, 2, 4, 6);
    }

    Player createMattOpponent(Random& random)
    {
        MessageScreen::show(
            "MATT (PRO)",
            "pvp.ai.matt.intro",
            {
                "Matt (PRO) est entré dans l'arène.",
                "Il n'a pas vraiment de spécialité : il est juste meilleur que prévu, quelle que soit sa classe."
            },
            false
        );

        Console::pauseSeconds(1);

        MenuScreen screen("CLASSE DE MATT (PRO)", "pvp.ai.matt.class_mode");
        screen.addLine("Choisis comment Matt obtiendra sa classe.");
        screen.addOption(1, "Classe aléatoire", "L'arène décide, comme prévu.", true, "pvp.ai.matt.random");
        screen.addOption(2, "Choisir sa classe toi-même", "Pas très fair-play, mais accepté.", true, "pvp.ai.matt.manual");

        int classChoiceType = TerminalInterface::askMenuChoiceFromOptions(
            screen,
            "Veuillez choisir 1 ou 2."
        );

        PlayerClass mattClass;

        if (classChoiceType == 1)
        {
            int aiClassChoice = random.between(1, ClassCatalog::getPlayableClassCount());
            mattClass = ClassCatalog::createBaseClass(aiClassChoice);
        }
        else
        {
            MessageScreen::show(
                "CAPRICE ACCEPTÉ",
                "pvp.ai.matt.manual.warning",
                {"L'arène accepte que tu règles toi-même le style de Matt."},
                false
            );
            Console::pauseSeconds(1);
            mattClass = askClassManually("Matt (PRO)");
        }

        Player ai("Matt (PRO)", mattClass);
        ai.setRace(CharacterRace::Human);
        ai.initializeStarterInventory();
        applyMattProUniversalBonus(ai);

        return ai;
    }

    Player createChosenSpecialOpponent()
    {
        Console::clear();

        std::vector<SpecialCharacter> characters = SpecialCharacterCatalog::getAllSpecialCharacters();
        MenuScreen screen("PERSONNAGES SPÉCIAUX", "pvp.ai.special.choice");
        screen.addLine("Le code a ouvert une porte que l'arène garde normalement rare.");
        screen.addLine("Tu peux provoquer un personnage spécial précis, y compris Matt (PRO).");

        for (std::size_t index = 0; index < characters.size(); ++index)
        {
            const SpecialCharacter& character = characters[index];
            screen.addOption(
                static_cast<int>(index + 1),
                character.getName(),
                "Race : " + character.getRaceText()
                    + " | Classe native : " + character.getNativeClass()
                    + " | " + character.getCombatStyle(),
                true,
                "pvp.ai.special." + std::to_string(index + 1)
            );
        }

        int choice = TerminalInterface::askMenuChoiceFromOptions(
            screen,
            "Veuillez choisir un personnage spécial affiché."
        );

        const SpecialCharacter& selected = characters[static_cast<std::size_t>(choice - 1)];

        Player opponent(selected.getName(), ClassCatalog::createClassByName(selected.getNativeClass()));
        opponent.setRace(selected.getRace());
        opponent.initializeStarterInventory();

        SpecialCharacterNativeBonus::applyForSpecialCharacter(opponent, selected);

        Console::clear();
        MessageScreen::show(
            "DÉFI PROVOQUÉ",
            "pvp.ai.special.called",
            {
                selected.getName() + " a été appelé directement par le registre altéré.",
                "Ce n'est plus une rencontre rare : c'est un défi provoqué."
            },
            false
        );

        return opponent;
    }

    Player createClassicOpponentFromChosenClass(Random& random)
    {
        Console::clear();

        MessageScreen::show(
            "ADVERSAIRE CLASSIQUE",
            "pvp.ai.classic.manual_intro",
            {
                "Création d'un adversaire classique par style de combat.",
                "Cette option ne peut pas faire apparaître de personnage spécial.",
                "Tu choisis seulement la classe. Le nom et la race seront tirés par l'arène."
            },
            false
        );

        Console::pauseSeconds(1);

        PlayerClass opponentClass = askClassManually("l'adversaire");
        return RandomCharacterGenerator::generateClassicOpponentWithClass(opponentClass, random);
    }
}

void AIPvpMode::run(Player& player1, Random& random)
{
    MessageScreen::show(
        "PRÉPARATION DE L'IA",
        "pvp.ai.intro",
        {"L'arène grave une silhouette adverse dans ses données."},
        false
    );

    Console::pauseSeconds(1);

    MenuScreen opponentScreen("TYPE D'ADVERSAIRE IA", "pvp.ai.opponent_type");
    opponentScreen.addOption(
        1,
        "Matt (PRO)",
        "L'adversaire référence. Il est meilleur globalement, peu importe sa classe.",
        true,
        "pvp.ai.opponent.matt"
    );
    opponentScreen.addOption(
        2,
        "Tirage d'arène aléatoire",
        "Nom, race et classe aléatoires. Personnage spécial possible : "
            + std::to_string(RandomCharacterGenerator::SPECIAL_ARENA_SPAWN_PERCENTAGE) + "%.",
        true,
        "pvp.ai.opponent.random"
    );
    opponentScreen.addOption(
        3,
        "Choisir une classe d'adversaire",
        "Tu choisis le style de combat. Aucun personnage spécial ne peut apparaître avec cette option.",
        true,
        "pvp.ai.opponent.manual_class"
    );

    if (player1.hasSpecialChallengeAccess())
    {
        opponentScreen.addOption(
            4,
            "Spéciaux",
            "Liste tous les personnages spéciaux et permet d'en provoquer un directement. Donnée altérée.",
            true,
            "pvp.ai.opponent.special"
        );
    }

    int opponentChoice = TerminalInterface::askMenuChoiceFromOptions(
        opponentScreen,
        "Veuillez choisir une option affichée."
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

    MessageScreen::show(
        "ADVERSAIRE GRAVÉ",
        "pvp.ai.opponent.ready",
        {
            ai.getName() + " entre dans l'arène.",
            "Race : " + ai.getRaceText() + ".",
            "Classe : " + ai.getType() + ".",
            "Ses statistiques ont été gravées dans l'arène."
        },
        false
    );

    MessageScreen::show(
        "ÉQUIPEMENT ADVERSAIRE",
        "pvp.ai.opponent.equipment_intro",
        {"Aperçu de son équipement :"},
        false
    );
    ai.displaySimpleEquipment();

    if (SpecialCharacterDialogueCatalog::hasDialogueFor(ai.getName()))
    {
        SpecialCharacterDialogueCatalog::displayEntranceDialogue(ai.getName());
    }

    if (SummonRules::classCanSummon(ai.getType()))
    {
        MessageScreen::show(
            "RISQUE D'INVOCATION",
            "pvp.ai.summon.warning",
            {
                SummonRules::getSummonWarningText(ai.getType()),
                "Un duel peut basculer en combat de groupe dès qu'un invocateur appelle ses renforts."
            },
            false
        );
    }

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

    CombatGroupBuilder::displayGroup(playerGroupPreview, "GROUPE DU JOUEUR");
    CombatRoleActionSystem::displayRoleIdentity(player1);

    SummonControlMode playerSummonControlMode = SummonCombatSystem::askPlayerSummonControlMode(player1, playerSummons);

    SummonCombatSystem::displaySummonArrival(ai, aiSummons);

    CombatGroup enemyGroupPreview = CombatGroupBuilder::buildSideFromEntityAndSummons(
        ai,
        aiSummons,
        CombatSide::EnemySide,
        CombatUnitKind::Enemy
    );

    CombatGroupBuilder::displayGroup(enemyGroupPreview, "GROUPE ADVERSE");
    CombatRoleActionSystem::displayRoleIdentity(ai);

    CombatDisplay::displayCombatState(
        CombatDisplay::buildGroupSnapshot(
            playerGroupPreview,
            enemyGroupPreview,
            "ÉTAT DE COMBAT",
            "Pré-combat : groupes actifs et invocations visibles"
        ),
        false
    );

    int turn = random.chooseFirstTurn();

    MessageScreen::show(
        "DÉBUT DU COMBAT",
        "pvp.ai.fight.start",
        {"Le combat contre " + ai.getName() + " commence maintenant."},
        false
    );

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
                    MessageScreen::show(
                        "CIBLAGE D'INVOCATION",
                        "pvp.ai.summon.targeted",
                        {ai.getName() + " choisit de briser une invocation avant de viser l'invocateur."},
                        false
                    );

                    SummonCombatSystem::entityAttacksSummon(ai, playerSummons[summonIndex], random);
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
                    SummonCombatSystem::playSummonTurnsAgainstEntity(aiSummons, player1, random);
                }

                turn = 1;
            }
        }
    }

    CombatDisplay::displayCombatResult(player1, ai);
}
