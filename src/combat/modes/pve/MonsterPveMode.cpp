// EN: MonsterPveMode.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: MonsterPveMode.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "combat/modes/pve/MonsterPveMode.hpp"

#include "combat/EnemyCombatQueue.hpp"
#include "combat/encounter/AdventurerGroupEncounter.hpp"
#include "combat/summon/SummonCombatSystem.hpp"
#include "combat/system/WaveCombatSystem.hpp"
#include "combat/reward/CombatReward.hpp"
#include "combat/reward/CombatRewardSystem.hpp"
#include "combat/loot/LootGenerator.hpp"
#include "combat/group/CombatGroupBuilder.hpp"
#include "combat/group/CombatUnitKind.hpp"
#include "combat/role/CombatRoleActionSystem.hpp"

#include "combat/turn/wave/PlayerWaveCombatTurn.hpp"
#include "combat/turn/wave/MonsterWaveCombatTurn.hpp"

#include "progression/DifficultyRules.hpp"
#include "progression/death/DeathPenaltyResult.hpp"
#include "progression/death/DeathPenaltySystem.hpp"
#include "progression/bestiary/BestiaryRuntimeProgress.hpp"
#include "core/Console.hpp"
#include "character/SpecialCharacterDialogueCatalog.hpp"
#include "character/relationship/SpecialCharacterGroupDialogueCatalog.hpp"

#include <iostream>
#include <string>
#include <vector>

namespace
{
    std::string buildMonsterBestiaryDescription(const Monster& monster)
    {
        std::string description = monster.getName()
            + " | Race : "
            + monster.getRaceText()
            + " | Niveau : "
            + std::to_string(monster.getLevel())
            + ".";

        if (monster.isElite())
        {
            description += " Cette entité est considérée comme élite.";
        }

        if (monster.isEvolved())
        {
            description += " Des signes d'évolution anormale sont visibles : masse renforcée, instincts plus nets, énergie plus dense.";
        }

        if (!monster.areStatsVisible())
        {
            description += " Certaines statistiques restent troubles pour le moment.";
        }

        return description;
    }

    // EN: recordWaveEncountersInBestiary declares or implements a focused behavior used by this module.
    // FR: recordWaveEncountersInBestiary déclare ou implémente un comportement précis utilisé par ce module.
    void recordWaveEncountersInBestiary(const EnemyCombatQueue& wave)
    {
        for (int i = 0; i < wave.getActiveEnemyCount(); ++i)
        {
            const Monster& monster = wave.getActiveEnemy(i);
            BestiaryRuntimeProgress::recordEncounter(
                monster.getName(),
                "Entités hostiles / ennemis",
                buildMonsterBestiaryDescription(monster)
            );
        }

        for (int i = 0; i < wave.getWaitingEnemyCount(); ++i)
        {
            const Monster& monster = wave.getWaitingEnemy(i);
            BestiaryRuntimeProgress::recordEncounter(
                monster.getName(),
                "Entités hostiles / ennemis",
                buildMonsterBestiaryDescription(monster)
            );
        }
    }


    void collectSpecialNamesFromWavePart(
        const EnemyCombatQueue& wave,
        std::vector<std::string>& names,
        int count,
        bool defeated
    )
    {
        for (int i = 0; i < count; ++i)
        {
            const Monster& monster = defeated ? wave.getDefeatedEnemy(i) : wave.getActiveEnemy(i);

            if (SpecialCharacterDialogueCatalog::hasDialogueFor(monster.getName()))
            {
                names.push_back(monster.getName());
            }
        }
    }

    std::vector<std::string> collectDefeatedSpecialNames(const EnemyCombatQueue& wave)
    {
        std::vector<std::string> names;
        collectSpecialNamesFromWavePart(wave, names, wave.getDefeatedEnemyCount(), true);
        return names;
    }

    std::vector<std::string> collectSurvivingSpecialNames(const EnemyCombatQueue& wave)
    {
        std::vector<std::string> names;

        for (int i = 0; i < wave.getActiveEnemyCount(); ++i)
        {
            const Monster& monster = wave.getActiveEnemy(i);
            if (SpecialCharacterDialogueCatalog::hasDialogueFor(monster.getName())) names.push_back(monster.getName());
        }

        for (int i = 0; i < wave.getWaitingEnemyCount(); ++i)
        {
            const Monster& monster = wave.getWaitingEnemy(i);
            if (SpecialCharacterDialogueCatalog::hasDialogueFor(monster.getName())) names.push_back(monster.getName());
        }

        return names;
    }

    // EN: displaySpecialDefeatDialogues declares or implements a focused behavior used by this module.
    // FR: displaySpecialDefeatDialogues déclare ou implémente un comportement précis utilisé par ce module.
    void displaySpecialDefeatDialogues(const EnemyCombatQueue& wave)
    {
        std::vector<std::string> names = collectDefeatedSpecialNames(wave);
        SpecialCharacterGroupDialogueCatalog::displayDefeatDialogue(names);

        for (const std::string& name : names)
        {
            SpecialCharacterDialogueCatalog::displayDefeatDialogue(name);
        }
    }

    // EN: displaySpecialVictoryDialogues declares or implements a focused behavior used by this module.
    // FR: displaySpecialVictoryDialogues déclare ou implémente un comportement précis utilisé par ce module.
    void displaySpecialVictoryDialogues(const EnemyCombatQueue& wave)
    {
        std::vector<std::string> names = collectSurvivingSpecialNames(wave);
        SpecialCharacterGroupDialogueCatalog::displayVictoryDialogue(names);

        for (const std::string& name : names)
        {
            SpecialCharacterDialogueCatalog::displayVictoryDialogue(name);
        }
    }


    // EN: countDefeatedEvolvedMonsters declares or implements a focused behavior used by this module.
    // FR: countDefeatedEvolvedMonsters déclare ou implémente un comportement précis utilisé par ce module.
    int countDefeatedEvolvedMonsters(const EnemyCombatQueue& wave)
    {
        int total = 0;

        for (int i = 0; i < wave.getDefeatedEnemyCount(); ++i)
        {
            if (wave.getDefeatedEnemy(i).isEvolved())
            {
                ++total;
            }
        }

        return total;
    }

    // EN: recordWaveKillsInBestiary declares or implements a focused behavior used by this module.
    // FR: recordWaveKillsInBestiary déclare ou implémente un comportement précis utilisé par ce module.
    void recordWaveKillsInBestiary(const EnemyCombatQueue& wave)
    {
        for (int i = 0; i < wave.getDefeatedEnemyCount(); ++i)
        {
            const Monster& monster = wave.getDefeatedEnemy(i);
            BestiaryRuntimeProgress::recordKill(
                monster.getName(),
                "Entités hostiles / ennemis",
                buildMonsterBestiaryDescription(monster)
            );
        }
    }
}

void MonsterPveMode::run(
    Player& player,
    Random& random,
    DifficultyMode difficulty
)
{
    Console::clear();

    std::cout << "Choisis le type de rencontre PvE." << std::endl;
    std::cout << std::endl;
    std::cout << "1 : Vague de monstres" << std::endl;
    std::cout << "    Une file d'ennemis classiques, avec maximum trois ennemis actifs." << std::endl;
    std::cout << std::endl;
    std::cout << "2 : Groupe d'aventuriers aléatoire" << std::endl;
    std::cout << "    Humains, semi-humains ou groupe spécial. Normalement pas un combat à mort, sauf cas dangereux." << std::endl;
    std::cout << std::endl;
    std::cout << "> ";

    int encounterChoice = Console::askNumberBetween(
        1,
        2,
        "Veuillez entrer 1 ou 2."
    );

    EnemyCombatQueue wave;

    if (encounterChoice == 2)
    {
        AdventurerGroupEncounter::displayGroupEncounterIntroduction();
        wave = AdventurerGroupEncounter::createRandomGroupForPlayer(player, random);
    }
    else
    {
        WaveCombatSystem::displayWaveIntroduction();
        wave = WaveCombatSystem::createWaveForPlayer(player, random, difficulty);
    }

    WaveCombatSystem::displayFrontLineArrival(wave);
    recordWaveEncountersInBestiary(wave);

    CombatGroup enemyFrontPreview = CombatGroupBuilder::buildSideFromWave(
        wave,
        CombatSide::EnemySide
    );

    CombatGroupBuilder::displayGroup(
        enemyFrontPreview,
        "LIGNE ENNEMIE ACTIVE"
    );

    std::vector<Summon> playerSummons = SummonCombatSystem::createInitialSummonsFor(player);
    SummonCombatSystem::displaySummonArrival(player, playerSummons);

    CombatGroup playerGroupPreview = CombatGroupBuilder::buildSideFromEntityAndSummons(
        player,
        playerSummons,
        CombatSide::PlayerSide,
        CombatUnitKind::MainFighter
    );

    CombatGroupBuilder::displayGroup(
        playerGroupPreview,
        "GROUPE DU JOUEUR"
    );

    CombatRoleActionSystem::displayRoleIdentity(player);

    SummonControlMode playerSummonControlMode =
        SummonCombatSystem::askPlayerSummonControlMode(player, playerSummons);

    bool escapeSucceeded = false;

    while (!player.isDead() && wave.hasEnemiesLeft() && !escapeSucceeded)
    {
        bool playerTurnFinished = false;

        while (!playerTurnFinished
            // EN: !player.isDead declares or implements a focused behavior used by this module.
            // FR: !player.isDead déclare ou implémente un comportement précis utilisé par ce module.
            && !player.isDead()
            // EN: wave.hasEnemiesLeft declares or implements a focused behavior used by this module.
            // FR: wave.hasEnemiesLeft déclare ou implémente un comportement précis utilisé par ce module.
            && wave.hasEnemiesLeft()
            && !escapeSucceeded)
        {
            playerTurnFinished = PlayerWaveCombatTurn::play(
                player,
                wave,
                random,
                escapeSucceeded,
                difficulty
            );

            if (!playerTurnFinished && !escapeSucceeded)
            {
                std::cout << "Ton tour n'est pas encore consommé." << std::endl;
                std::cout << std::endl;
            }
        }

        if (!player.isDead()
            // EN: wave.hasEnemiesLeft declares or implements a focused behavior used by this module.
            // FR: wave.hasEnemiesLeft déclare ou implémente un comportement précis utilisé par ce module.
            && wave.hasEnemiesLeft()
            && !escapeSucceeded
            // EN: hasActiveSummons declares or implements a focused behavior used by this module.
            // FR: hasActiveSummons déclare ou implémente un comportement précis utilisé par ce module.
            && SummonCombatSystem::hasActiveSummons(playerSummons))
        {
            SummonCombatSystem::playPlayerSummonTurnsAgainstWave(
                playerSummons,
                wave,
                random,
                playerSummonControlMode
            );
        }

        if (!player.isDead() && wave.hasEnemiesLeft() && !escapeSucceeded)
        {
            MonsterWaveCombatTurn::playMonsterTurns(
                player,
                wave,
                playerSummons,
                random
            );
        }
    }

    if (escapeSucceeded)
    {
        std::cout << "Tu as quitté le combat." << std::endl;
        std::cout << "Tu ne récupéreras qu'une partie des récompenses liées à ce qui s'est réellement passé." << std::endl;
        std::cout << std::endl;

        CombatReward reward = CombatRewardSystem::calculatePlayerEscapeReward(
            wave,
            difficulty
        );

        CombatRewardSystem::displayPartialReward(
            reward,
            "Fuite réussie : les récompenses sont calculées selon la difficulté, les ennemis vaincus, et les ennemis encore en vie déjà blessés."
        );

        CombatRewardSystem::giveRewardToPlayer(player, reward);
        player.recordEscape();
        player.recordEnemyKills(wave.getDefeatedEnemyCount());
        recordWaveKillsInBestiary(wave);

        return;
    }

    if (player.isDead())
    {
        std::cout << player.getName() << " tombe face à la vague ennemie." << std::endl;
        std::cout << "L'arène se referme dans un silence brutal." << std::endl;
        std::cout << std::endl;

        player.recordDefeat();
        player.recordDeath();

        if (DifficultyRules::isPermanentDeath(difficulty))
        {
            DeathPenaltySystem::displayLethalDeathCorruption();
            return;
        }

        DeathPenaltyResult deathPenalty = DeathPenaltySystem::applyNonLethalDeathPenalty(
            player,
            difficulty,
            random
        );

        DeathPenaltySystem::displayNonLethalDeathPenalty(deathPenalty);

        displaySpecialVictoryDialogues(wave);

        player.reviveWithHealthPercentage(
            DifficultyRules::getNonLethalRespawnHealthPercentage(difficulty)
        );

        std::cout << player.getName()
                  << " revient à lui avec "
                  << player.getHp()
                  << "/"
                  << player.getMaxHp()
                  << " PV."
                  << std::endl;
        std::cout << "Tu as survécu, mais la mort a laissé sa trace." << std::endl;
        std::cout << std::endl;

        return;
    }

    displaySpecialDefeatDialogues(wave);

    std::cout << "Tous les monstres de la vague ont été vaincus." << std::endl;
    std::cout << player.getName() << " reste debout au milieu des corps et de la poussière." << std::endl;
    std::cout << std::endl;

    CombatReward reward = CombatRewardSystem::calculateWaveReward(wave, difficulty);

    CombatRewardSystem::displayReward(reward);
    CombatRewardSystem::giveRewardToPlayer(player, reward);
    player.recordVictory();
    player.recordEnemyKills(wave.getDefeatedEnemyCount());
    recordWaveKillsInBestiary(wave);
    LootGenerator::giveDefeatedWaveLoot(player, wave, random, difficulty);

    int evolvedKilled = countDefeatedEvolvedMonsters(wave);
    if (evolvedKilled > 0)
    {
        int updated = player.getQuestLog().progressCombatQuestsByFamily(evolvedKilled, "Créature évoluée");
        if (updated > 0)
        {
            std::cout << "Le bestiaire et les quêtes liées aux créatures évoluées progressent." << std::endl;
            std::cout << std::endl;
        }
    }
}
