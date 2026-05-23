// EN: CombatRewardSystem.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: CombatRewardSystem.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "combat/reward/CombatRewardSystem.hpp"

#include "progression/DifficultyRules.hpp"

#include <algorithm>
#include <iostream>
#include <functional>
#include <string>

namespace
{
    // EN: givesNormalGoldReward declares or implements a focused behavior used by this module.
    // FR: givesNormalGoldReward déclare ou implémente un comportement précis utilisé par ce module.
    bool givesNormalGoldReward(Race race)
    {
        switch (race)
        {
            case Race::Humain:
            case Race::SemiHumain:
            case Race::Elfe:
            case Race::ElfeNoir:
            case Race::Nain:
            case Race::Gnome:
            case Race::Halfelin:
            case Race::Tieffelin:
            case Race::Aasimar:
            case Race::Kitsune:
            case Race::Fee:
            case Race::SemiDragon:
            case Race::Gobelin:
            case Race::Hobgobelin:
            case Race::Orc:
            case Race::Demon:
            case Race::Ange:
                return true;

            default:
                return false;
        }
    }


    int calculateRareScavengedGold(const Monster& monster, Random* random)
    {
        if (givesNormalGoldReward(monster.getRace()))
        {
            return 0;
        }

        int chancePercent = 4;
        if (monster.isElite()) chancePercent += 3;
        if (monster.isEvolved()) chancePercent += 2;
        if (monster.getLevel() >= 8) chancePercent += 1;

        bool foundCoins = false;
        if (random != nullptr)
        {
            foundCoins = random->between(1, 100) <= chancePercent;
        }
        else
        {
            std::size_t signature = std::hash<std::string>{}(monster.getName())
                ^ (static_cast<std::size_t>(monster.getLevel()) << 4)
                ^ (static_cast<std::size_t>(monster.getMaxHp()) << 1);
            foundCoins = static_cast<int>(signature % 100) < chancePercent;
        }

        if (!foundCoins)
        {
            return 0;
        }

        int gold = 1 + monster.getLevel() / 4;
        if (monster.isElite()) gold += 1;
        if (monster.isEvolved()) gold += 1;
        return std::max(1, std::min(8, gold));
    }

    int difficultySurvivalPercent(DifficultyMode difficulty)
    {
        switch (difficulty)
        {
            case DifficultyMode::Easy:
                return 80;
            case DifficultyMode::Hard:
                return 120;
            case DifficultyMode::Nightmare:
                return 145;
            case DifficultyMode::Lethal:
                return 160;
            case DifficultyMode::Normal:
            default:
                return 100;
        }
    }

    CombatReward calculateCombatEffortReward(
        const Player& player,
        int initialPlayerHp,
        int turnCount,
        DifficultyMode difficulty,
        int baseExperienceDivider,
        int baseGoldDivider
    )
    {
        int damageReceived = std::max(0, initialPlayerHp - player.getHp());
        int enduranceExperience = damageReceived / baseExperienceDivider;
        int durationExperience = std::max(0, turnCount - 1) * std::max(1, player.getLevel() / 2 + 1);

        int experience = (enduranceExperience + durationExperience)
            * difficultySurvivalPercent(difficulty)
            / 100;

        int gold = 0;
        if (damageReceived > 0 && baseGoldDivider > 0)
        {
            gold = std::min(35, damageReceived / baseGoldDivider);
        }

        return CombatReward(experience, gold);
    }
}

// EN: calculateMonsterReward declares or implements a focused behavior used by this module.
// FR: calculateMonsterReward déclare ou implémente un comportement précis utilisé par ce module.
CombatReward CombatRewardSystem::calculateMonsterReward(const Monster& monster, Random* random)
{
    int monsterLevel = monster.getLevel();

    int experience = 18 + monsterLevel * 10;
    int gold = givesNormalGoldReward(monster.getRace())
        ? 5 + monsterLevel * 3
        : calculateRareScavengedGold(monster, random);

    if (monster.isElite())
    {
        experience += 12 + monsterLevel * 13;
        gold += givesNormalGoldReward(monster.getRace())
            ? 10 + monsterLevel * 5
            : 0;
    }

    if (monster.isEvolved())
    {
        experience += 20 + monsterLevel * 8;
        gold += givesNormalGoldReward(monster.getRace())
            ? 8 + monsterLevel * 4
            : 0;
    }

    return CombatReward(experience, gold);
}


CombatReward CombatRewardSystem::calculateBossReward(
    const Boss& boss,
    DifficultyMode difficulty,
    int damageReceived,
    int turnCount
)
{
    int bossId = std::max(1, boss.getBossId());
    int bossPower = boss.getMaxHp() / 4 + boss.getMaxDamage() * 6 + boss.getCriticalDamage() * 3;

    int experience = 180 + bossId * 18 + bossPower;
    int gold = 80 + bossId * 7 + boss.getMaxHp() / 12;

    if (turnCount >= 8)
    {
        experience += (turnCount - 7) * 12;
    }

    if (damageReceived > 0)
    {
        experience += damageReceived / 2;
    }

    CombatReward reward(experience, gold);
    return reward.getModified(
        DifficultyRules::getVictoryExperienceRewardPercentage(difficulty),
        DifficultyRules::getVictoryGoldRewardPercentage(difficulty)
    );
}

// EN: calculateDefeatedEnemiesReward declares or implements a focused behavior used by this module.
// FR: calculateDefeatedEnemiesReward déclare ou implémente un comportement précis utilisé par ce module.
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

namespace
{
    CombatReward calculateDefeatedEnemiesRewardWithRandom(const EnemyCombatQueue& wave, Random& random)
    {
        CombatReward totalReward;

        for (int i = 0; i < wave.getDefeatedEnemyCount(); ++i)
        {
            totalReward.addReward(
                CombatRewardSystem::calculateMonsterReward(wave.getDefeatedEnemy(i), &random)
            );
        }

        return totalReward;
    }

    CombatReward calculateWaveRewardWithRandom(const EnemyCombatQueue& wave, Random& random)
    {
        CombatReward totalReward;
        totalReward.addReward(calculateDefeatedEnemiesRewardWithRandom(wave, random));
        totalReward.addReward(CombatRewardSystem::calculateEscapedEnemiesReward(wave));
        return totalReward;
    }
}


// EN: calculateEscapedEnemiesReward declares or implements a focused behavior used by this module.
// FR: calculateEscapedEnemiesReward déclare ou implémente un comportement précis utilisé par ce module.
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

// EN: calculateDamagedAliveEnemiesReward declares or implements a focused behavior used by this module.
// FR: calculateDamagedAliveEnemiesReward déclare ou implémente un comportement précis utilisé par ce module.
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

// EN: calculateWaveReward declares or implements a focused behavior used by this module.
// FR: calculateWaveReward déclare ou implémente un comportement précis utilisé par ce module.
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


CombatReward CombatRewardSystem::calculateWaveReward(
    const EnemyCombatQueue& wave,
    DifficultyMode difficulty
)
{
    CombatReward baseReward = calculateWaveReward(wave);

    return baseReward.getModified(
        DifficultyRules::getVictoryExperienceRewardPercentage(difficulty),
        DifficultyRules::getVictoryGoldRewardPercentage(difficulty)
    );
}


CombatReward CombatRewardSystem::calculateWaveReward(
    const EnemyCombatQueue& wave,
    DifficultyMode difficulty,
    const Player& player,
    int initialPlayerHp,
    int turnCount
)
{
    CombatReward totalReward = calculateWaveReward(wave, difficulty);
    totalReward.addReward(
        calculateCombatEffortReward(
            player,
            initialPlayerHp,
            turnCount,
            difficulty,
            5,
            18
        )
    );

    return totalReward;
}

CombatReward CombatRewardSystem::calculateWaveReward(
    const EnemyCombatQueue& wave,
    DifficultyMode difficulty,
    const Player& player,
    int initialPlayerHp,
    int turnCount,
    Random& random
)
{
    CombatReward totalReward = calculateWaveRewardWithRandom(wave, random).getModified(
        DifficultyRules::getVictoryExperienceRewardPercentage(difficulty),
        DifficultyRules::getVictoryGoldRewardPercentage(difficulty)
    );

    totalReward.addReward(
        calculateCombatEffortReward(
            player,
            initialPlayerHp,
            turnCount,
            difficulty,
            5,
            18
        )
    );

    return totalReward;
}

// EN: calculatePlayerEscapeReward declares or implements a focused behavior used by this module.
// FR: calculatePlayerEscapeReward déclare ou implémente un comportement précis utilisé par ce module.
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
