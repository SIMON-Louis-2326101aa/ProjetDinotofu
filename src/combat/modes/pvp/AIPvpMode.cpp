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
#include "interface/menu/common/PagedMenu.hpp"
#include "interface/model/MenuScreen.hpp"
#include "lore/LegendTriggerSystem.hpp"
#include "progression/bestiary/BestiaryRuntimeProgress.hpp"

#include <iostream>
#include <string>
#include <vector>

namespace
{
    bool isCatalogSpecialOpponentName(const std::string& name)
    {
        for (const SpecialCharacter& character : SpecialCharacterCatalog::getAllSpecialCharacters())
        {
            if (character.getName() == name)
            {
                return true;
            }
        }

        return false;
    }

    std::string buildSpecialOpponentBestiaryDescription(const Player& opponent)
    {
        return "Rencontre d'arène IA : " + opponent.getName()
            + " | Race : " + opponent.getRaceText()
            + " | Classe : " + opponent.getType()
            + ". Le registre confirme ce profil seulement après une rencontre réelle, un défi provoqué ou une rumeur fiable.";
    }

    void recordSpecialOpponentEncounterIfNeeded(const Player& opponent)
    {
        if (!isCatalogSpecialOpponentName(opponent.getName()))
        {
            return;
        }

        BestiaryRuntimeProgress::recordEncounter(
            opponent.getName(),
            "Personnages spéciaux",
            buildSpecialOpponentBestiaryDescription(opponent)
        );
    }

    void recordSpecialOpponentVictoryIfNeeded(const Player& opponent)
    {
        if (!isCatalogSpecialOpponentName(opponent.getName()))
        {
            return;
        }

        BestiaryRuntimeProgress::recordKill(
            opponent.getName(),
            "Personnages spéciaux",
            buildSpecialOpponentBestiaryDescription(opponent)
        );
    }

    MenuOptionItemData makeAiClassItemData(
        const std::string& kind,
        const std::string& name,
        const std::string& detail,
        const std::string& status = ""
    )
    {
        MenuOptionItemData itemData;
        itemData.structured = true;
        itemData.kind = kind;
        itemData.section = "Adversaire IA";
        itemData.actionType = "select";
        itemData.name = name;
        itemData.detail = detail;
        itemData.status = status;
        return itemData;
    }

    int askAiClassChoice(const std::string& targetName, int categoryChoice)
    {
        std::vector<ClassOptionInfo> classOptions = ClassCatalog::getClassOptionsByCategoryChoice(categoryChoice);
        constexpr std::size_t itemsPerPage = 8;
        std::size_t pageIndex = 0;
        const std::size_t totalPages = PagedMenu::pageCount(classOptions.size(), itemsPerPage);

        while (true)
        {
            const std::size_t firstIndex = PagedMenu::firstIndex(pageIndex, itemsPerPage);
            const std::size_t lastIndex = PagedMenu::lastIndexExclusive(classOptions.size(), pageIndex, itemsPerPage);

            MenuScreen classScreen("CLASSE DE " + targetName, "pvp.ai.class.choice");
            classScreen.setPagination(pageIndex, totalPages);
            classScreen.addLine("Famille sélectionnée : " + ClassCatalog::getClassCategoryNameByChoice(categoryChoice) + ".");
            classScreen.addLine("Choisis une classe précise pour fixer le rythme du duel.");
            classScreen.addLine("Page : " + std::to_string(pageIndex + 1) + "/" + std::to_string(totalPages));
            classScreen.addLine("Classes affichées : " + PagedMenu::rangeText(firstIndex, lastIndex, classOptions.size()));

            for (std::size_t index = firstIndex; index < lastIndex; ++index)
            {
                const ClassOptionInfo& info = classOptions[index];
                const int localChoice = static_cast<int>(index - firstIndex + 1);
                MenuOptionItemData itemData = makeAiClassItemData(
                    "class",
                    info.name,
                    info.role,
                    "PV " + std::to_string(info.maxHp)
                        + " · Dégâts " + std::to_string(info.minDamage) + "-" + std::to_string(info.maxDamage)
                        + " · Crit " + std::to_string(info.criticalDamage)
                );
                itemData.progress = info.categoryName;
                itemData.quantity = "Soin x" + std::to_string(info.healingPotionCount);
                itemData.reward = "Dégâts x" + std::to_string(info.damagePotionCount);

                classScreen.addOption(
                    localChoice,
                    info.name,
                    "Rôle : " + info.role
                        + " | PV " + std::to_string(info.maxHp)
                        + " | Dégâts " + std::to_string(info.minDamage) + "-" + std::to_string(info.maxDamage)
                        + " | Crit " + std::to_string(info.criticalDamage),
                    true,
                    "pvp.ai.class.choice." + std::to_string(index + 1),
                    itemData
                );
            }

            if (totalPages > 1 && pageIndex > 0)
            {
                classScreen.addOption(98, "Page précédente", "Voir les classes précédentes.", true, "pvp.ai.class.choice.page.previous");
            }

            if (totalPages > 1 && pageIndex + 1 < totalPages)
            {
                classScreen.addOption(99, "Page suivante", "Voir les classes suivantes.", true, "pvp.ai.class.choice.page.next");
            }

            int classChoice = TerminalInterface::askMenuChoiceFromOptions(
                classScreen,
                "Veuillez choisir une classe visible sur la page."
            );

            if (classChoice == 98 && pageIndex > 0)
            {
                --pageIndex;
                Console::clear();
                continue;
            }

            if (classChoice == 99 && pageIndex + 1 < totalPages)
            {
                ++pageIndex;
                Console::clear();
                continue;
            }

            const std::size_t selectedIndex = firstIndex + static_cast<std::size_t>(classChoice - 1);
            if (selectedIndex >= classOptions.size() || selectedIndex >= lastIndex)
            {
                Console::clear();
                MessageScreen::show(
                    "CHOIX REFUSÉ",
                    "pvp.ai.class.choice.invalid_page",
                    {"Cette classe n'est pas visible sur la page actuelle."},
                    false
                );
                continue;
            }

            return static_cast<int>(selectedIndex + 1);
        }
    }

    PlayerClass askClassManually(const std::string& targetName)
    {
        Console::clear();

        MenuScreen categoryScreen("STYLE DE COMBAT DE " + targetName, "pvp.ai.class.category");
        categoryScreen.addLine("Tu ne choisis pas vraiment l'identité de l'adversaire : seulement son style de combat.");
        categoryScreen.addLine("Le nom, la race et l'équipement restent gérés par l'arène.");

        for (int i = 1; i <= ClassCatalog::getClassCategoryCount(); ++i)
        {
            const std::string categoryName = ClassCatalog::getClassCategoryNameByChoice(i);
            const std::string classCountText = std::to_string(ClassCatalog::getPlayableClassCountByCategoryChoice(i)) + " classes disponibles";
            categoryScreen.addOption(
                i,
                categoryName,
                classCountText,
                true,
                "pvp.ai.class.category." + std::to_string(i),
                makeAiClassItemData("class_category", categoryName, classCountText)
            );
        }

        int categoryChoice = TerminalInterface::askMenuChoiceFromOptions(
            categoryScreen,
            "Veuillez choisir une famille affichée."
        );

        Console::clear();

        int classChoice = askAiClassChoice(targetName, categoryChoice);

        return ClassCatalog::createClassByCategoryChoice(categoryChoice, classChoice);
    }

    void applyMattProUniversalBonus(Player& matt)
    {
        matt.applyFlatStatBonus(20, 2, 4, 6);
    }

    void showAiPvpConclusion(
        const Player& player,
        const Player& opponent,
        bool playerWon,
        bool playerWasRevived,
        int initialHp
    )
    {
        MenuScreen screen("BILAN DUEL IA", "pvp.ai.result.conclusion");
        screen.addSubtitle("Résumé de sortie d'arène");
        screen.addLine("Résultat : " + std::string(playerWon ? "victoire du joueur" : "défaite du joueur"));
        screen.addLine("Mode : arène IA non létale");
        screen.addLine("Joueur : " + player.getName());
        screen.addLine("Adversaire : " + opponent.getName());
        screen.addLine("- " + player.getName() + " | rôle : joueur | PV " + std::to_string(player.getHp()) + "/" + std::to_string(player.getMaxHp()));
        screen.addLine("- " + opponent.getName() + " | rôle : adversaire IA | PV " + std::to_string(opponent.getHp()) + "/" + std::to_string(opponent.getMaxHp()));
        screen.addLine("PV joueur avant duel : " + std::to_string(initialHp));
        screen.addLine("PV joueur après bilan : " + std::to_string(player.getHp()) + "/" + std::to_string(player.getMaxHp()));
        screen.addLine("Statistiques : JcJ et combat mises à jour");
        screen.addLine("Mort définitive : non");
        screen.addLine("Restauration : " + std::string(playerWasRevived ? "réveil automatique appliqué" : "non nécessaire"));
        screen.addLine("Dommages permanents : aucun dans cette arène IA");
        screen.setDisplayOnlyInput("Résumé affiché sans saisie directe.");
        TerminalInterface::renderMenuScreen(screen, false);
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
        screen.addOption(
            1,
            "Classe aléatoire",
            "L'arène décide, comme prévu.",
            true,
            "pvp.ai.matt.random",
            makeAiClassItemData("combat_route", "Classe aléatoire", "L'arène fixe seule le style de Matt (PRO).", "Défi spécial")
        );
        screen.addOption(
            2,
            "Choisir sa classe toi-même",
            "Pas très fair-play, mais accepté.",
            true,
            "pvp.ai.matt.manual",
            makeAiClassItemData("combat_route", "Choisir sa classe", "Le joueur force la classe de Matt (PRO).", "Donnée altérée")
        );

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

    MenuOptionItemData makeSpecialOpponentItemData(const SpecialCharacter& character)
    {
        MenuOptionItemData itemData;
        itemData.structured = true;
        itemData.kind = "special_character";
        itemData.section = "Personnages spéciaux";
        itemData.actionType = "select";
        itemData.name = character.getName();
        itemData.detail = character.getCombatStyle();
        itemData.status = "Défi provoqué";
        itemData.owner = "Arène IA";
        itemData.progress = "Classe native : " + character.getNativeClass();
        itemData.reward = "Race : " + character.getRaceText();
        itemData.important = character.getName() == "Matt (PRO)";
        return itemData;
    }

    Player createChosenSpecialOpponent()
    {
        Console::clear();

        std::vector<SpecialCharacter> characters = SpecialCharacterCatalog::getAllSpecialCharacters();
        constexpr std::size_t itemsPerPage = 8;
        std::size_t pageIndex = 0;
        const std::size_t totalPages = PagedMenu::pageCount(characters.size(), itemsPerPage);

        while (true)
        {
            const std::size_t firstIndex = PagedMenu::firstIndex(pageIndex, itemsPerPage);
            const std::size_t lastIndex = PagedMenu::lastIndexExclusive(characters.size(), pageIndex, itemsPerPage);

            MenuScreen screen("PERSONNAGES SPÉCIAUX", "pvp.ai.special.choice");
            screen.setPagination(pageIndex, totalPages);
            screen.addLine("Le code a ouvert une porte que l'arène garde normalement rare.");
            screen.addLine("Tu peux provoquer un personnage spécial précis, y compris Matt (PRO).");
            screen.addLine("Page : " + std::to_string(pageIndex + 1) + "/" + std::to_string(totalPages));
            screen.addLine("Entrées affichées : " + PagedMenu::rangeText(firstIndex, lastIndex, characters.size()));

            for (std::size_t index = firstIndex; index < lastIndex; ++index)
            {
                const SpecialCharacter& character = characters[index];
                const int localChoice = static_cast<int>(index - firstIndex + 1);
                screen.addOption(
                    localChoice,
                    character.getName(),
                    "Race : " + character.getRaceText()
                        + " | Classe native : " + character.getNativeClass()
                        + " | " + character.getCombatStyle(),
                    true,
                    "pvp.ai.special." + std::to_string(index + 1),
                    makeSpecialOpponentItemData(character)
                );
            }

            if (totalPages > 1 && pageIndex > 0)
            {
                screen.addOption(98, "Page précédente", "Voir les personnages spéciaux précédents.", true, "pvp.ai.special.page.previous");
            }

            if (totalPages > 1 && pageIndex + 1 < totalPages)
            {
                screen.addOption(99, "Page suivante", "Voir les personnages spéciaux suivants.", true, "pvp.ai.special.page.next");
            }

            int choice = TerminalInterface::askMenuChoiceFromOptions(
                screen,
                "Veuillez choisir un personnage spécial affiché."
            );

            if (choice == 98 && pageIndex > 0)
            {
                --pageIndex;
                Console::clear();
                continue;
            }

            if (choice == 99 && pageIndex + 1 < totalPages)
            {
                ++pageIndex;
                Console::clear();
                continue;
            }

            const std::size_t selectedIndex = firstIndex + static_cast<std::size_t>(choice - 1);
            if (selectedIndex >= characters.size() || selectedIndex >= lastIndex)
            {
                Console::clear();
                MessageScreen::show(
                    "CHOIX REFUSÉ",
                    "pvp.ai.special.choice.invalid_page",
                    {"Ce personnage n'est pas visible sur la page actuelle."},
                    false
                );
                continue;
            }

            const SpecialCharacter& selected = characters[selectedIndex];

            Player opponent(selected.getName(), ClassCatalog::createClassByName(selected.getNativeClass()));
            opponent.setRace(selected.getRace());
            opponent.initializeStarterInventory();

            SpecialCharacterNativeBonus::applyForSpecialCharacter(opponent, selected);

            Console::clear();
            MessageScreen::show(
                "DÉFI PROVOQUÉ",
                "pvp.ai.special.called",
                {
                    "Personnage : " + selected.getName(),
                    "Race : " + selected.getRaceText(),
                    "Classe native : " + selected.getNativeClass(),
                    selected.getName() + " a été appelé directement par le registre altéré.",
                    "Ce n'est plus une rencontre rare : c'est un défi provoqué."
                },
                false
            );

            return opponent;
        }
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
        "pvp.ai.opponent.matt",
        makeAiClassItemData("combat_route", "Matt (PRO)", "Adversaire spécial de référence, meilleur globalement.", "Défi spécial")
    );
    opponentScreen.addOption(
        2,
        "Tirage d'arène aléatoire",
        "Nom, race et classe aléatoires. Personnage spécial possible : "
            + std::to_string(RandomCharacterGenerator::SPECIAL_ARENA_SPAWN_PERCENTAGE) + "%.",
        true,
        "pvp.ai.opponent.random",
        makeAiClassItemData(
            "combat_route",
            "Tirage d'arène aléatoire",
            "Nom, race et classe tirés par l'arène.",
            "Spécial possible : " + std::to_string(RandomCharacterGenerator::SPECIAL_ARENA_SPAWN_PERCENTAGE) + "%"
        )
    );
    opponentScreen.addOption(
        3,
        "Choisir une classe d'adversaire",
        "Tu choisis le style de combat. Aucun personnage spécial ne peut apparaître avec cette option.",
        true,
        "pvp.ai.opponent.manual_class",
        makeAiClassItemData("combat_route", "Classe imposée", "Le joueur choisit seulement le style de combat adverse.", "Aucun spécial")
    );

    if (player1.hasSpecialChallengeAccess())
    {
        opponentScreen.addOption(
            4,
            "Spéciaux",
            "Liste tous les personnages spéciaux et permet d'en provoquer un directement. Donnée altérée.",
            true,
            "pvp.ai.opponent.special",
            makeAiClassItemData("combat_route", "Spéciaux", "Provoquer directement un personnage spécial du registre.", "Donnée altérée")
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

    recordSpecialOpponentEncounterIfNeeded(ai);
    LegendTriggerSystem::maybeDisplaySpecialOpponentLegend(ai.getName(), random);

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

    const int playerInitialHp = player1.getHp();

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

    const bool playerWon = !player1.isDead() && ai.isDead();
    bool playerWasRevived = false;

    if (playerWon)
    {
        player1.recordPvpVictory();
        player1.recordVictory();
        recordSpecialOpponentVictoryIfNeeded(ai);
    }
    else
    {
        player1.recordPvpDefeat();
        player1.recordDefeat();

        if (player1.isDead())
        {
            player1.reviveWithHealthPercentage(35);
            playerWasRevived = true;
        }
    }

    showAiPvpConclusion(
        player1,
        ai,
        playerWon,
        playerWasRevived,
        playerInitialHp
    );
}
