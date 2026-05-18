// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "combat/reward/CombatRewardSystem.hpp"

#include "progression/DifficultyRules.hpp"

#include <iostream>

CombatReward CombatRewardSystem::calculateMonsterReward(const Monster& monster)
{
    int monsterLevel = monster.getLevel();

    int experience = 12 + monsterLevel * 8;
    int gold = 3 + monsterLevel * 2;

    if (monster.isElite())
    {
        experience += monsterLevel * 10;
        gold += monsterLevel * 4;
    }

    return CombatReward(experience, gold);
}

CombatReward CombatRewardSystem::calculateDefeatedEnemiesReward(const EnemyCombatQueue& wave)
{
    CombatReward totalReward;

    for (int i = 0; i < wave.getDefeatedEnemyCount(); ++i)
    {
        totalReward.addReward(
            calculateMonsterReward(wave.getDefeatedEnemy(i))
        );
    }

    return totalReward;
}

CombatReward CombatRewardSystem::calculateEscapedEnemiesReward(const EnemyCombatQueue& wave)
{
    CombatReward totalReward;

    for (int i = 0; i < wave.getEscapedEnemyCount(); ++i)
    {
        CombatReward enemyReward = calculateMonsterReward(wave.getEscapedEnemy(i));

        totalReward.addReward(
            enemyReward.getPercentage(MONSTER_ESCAPE_REWARD_PERCENTAGE)
        );
    }

    return totalReward;
}

CombatReward CombatRewardSystem::calculateDamagedAliveEnemiesReward(const EnemyCombatQueue& wave)
{
    CombatReward totalReward;

    for (int i = 0; i < wave.getDamagedAliveEnemyCount(); ++i)
    {
        CombatReward enemyReward = calculateMonsterReward(wave.getDamagedAliveEnemy(i));

        totalReward.addReward(
            enemyReward.getPercentage(PLAYER_PVE_ESCAPE_DAMAGED_ALIVE_REWARD_PERCENTAGE)
        );
    }

    return totalReward;
}

CombatReward CombatRewardSystem::calculateDamagedAliveEnemiesReward(
    const EnemyCombatQueue& wave,
    DifficultyMode difficulty
)
{
    CombatReward totalReward;

    for (int i = 0; i < wave.getDamagedAliveEnemyCount(); ++i)
    {
        CombatReward enemyReward = calculateMonsterReward(wave.getDamagedAliveEnemy(i));

        totalReward.addReward(
            enemyReward.getPercentage(
                DifficultyRules::getPlayerPveEscapeDamagedAliveRewardPercentage(difficulty)
            )
        );
    }

    return totalReward;
}

CombatReward CombatRewardSystem::calculateWaveReward(const EnemyCombatQueue& wave)
{
    CombatReward totalReward;

    totalReward.addReward(
        calculateDefeatedEnemiesReward(wave)
    );

    totalReward.addReward(
        calculateEscapedEnemiesReward(wave)
    );

    return totalReward;
}

CombatReward CombatRewardSystem::calculatePlayerEscapeReward(const EnemyCombatQueue& wave)
{
    CombatReward totalReward;

    CombatReward defeatedReward = calculateDefeatedEnemiesReward(wave);

    totalReward.addReward(
        defeatedReward.getPercentage(PLAYER_PVE_ESCAPE_DEFEATED_REWARD_PERCENTAGE)
    );

    totalReward.addReward(
        calculateDamagedAliveEnemiesReward(wave)
    );

    return totalReward;
}

CombatReward CombatRewardSystem::calculatePlayerEscapeReward(
    const EnemyCombatQueue& wave,
    DifficultyMode difficulty
)
{
    CombatReward totalReward;

    CombatReward defeatedReward = calculateDefeatedEnemiesReward(wave);

    totalReward.addReward(
        defeatedReward.getPercentage(
            DifficultyRules::getPlayerPveEscapeDefeatedRewardPercentage(difficulty)
        )
    );

    totalReward.addReward(
        calculateDamagedAliveEnemiesReward(wave, difficulty)
    );

    return totalReward;
}

void CombatRewardSystem::giveRewardToPlayer(
    Player& player,
    const CombatReward& reward
)
{
    if (reward.getExperience() > 0)
    {
        player.gainExperience(reward.getExperience());
    }

    if (reward.getGold() > 0)
    {
        player.getInventory().earnGold(reward.getGold());
    }
}

void CombatRewardSystem::displayReward(
    const CombatReward& reward
)
{
    std::cout << "========== RÉCOMPENSES ==========" << std::endl;

    if (reward.getExperience() <= 0 && reward.getGold() <= 0)
    {
        std::cout << "Aucune récompense récupérée." << std::endl;
    }
    else
    {
        std::cout << "Expérience gagnée : " << reward.getExperience() << std::endl;
        std::cout << "Or gagné : " << reward.getGold() << " pièces" << std::endl;
    }

    std::cout << "=================================" << std::endl;
    std::cout << std::endl;
}

void CombatRewardSystem::displayPartialReward(
    const CombatReward& reward,
    const std::string& reason
)
{
    std::cout << "====== RÉCOMPENSES PARTIELLES ======" << std::endl;
    std::cout << reason << std::endl;
    std::cout << std::endl;

    if (reward.getExperience() <= 0 && reward.getGold() <= 0)
    {
        std::cout << "Aucune récompense récupérée." << std::endl;
    }
    else
    {
        std::cout << "Expérience récupérée : " << reward.getExperience() << std::endl;
        std::cout << "Or récupéré : " << reward.getGold() << " pièces" << std::endl;
    }

    std::cout << "====================================" << std::endl;
    std::cout << std::endl;
}
