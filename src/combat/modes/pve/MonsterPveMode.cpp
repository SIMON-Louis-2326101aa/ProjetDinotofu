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

        if (!monster.areStatsVisible())
        {
            description += " Certaines statistiques restent troubles pour le moment.";
        }

        return description;
    }

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
        wave = WaveCombatSystem::createWaveForPlayer(player, random);
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
            && !player.isDead()
            && wave.hasEnemiesLeft()
            && !escapeSucceeded)
        {
            playerTurnFinished = PlayerWaveCombatTurn::play(
                player,
                wave,
                random,
                escapeSucceeded
            );

            if (!playerTurnFinished && !escapeSucceeded)
            {
                std::cout << "Ton tour n'est pas encore consommé." << std::endl;
                std::cout << std::endl;
            }
        }

        if (!player.isDead()
            && wave.hasEnemiesLeft()
            && !escapeSucceeded
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
}
