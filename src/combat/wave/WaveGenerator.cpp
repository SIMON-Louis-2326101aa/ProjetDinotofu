// EN: WaveGenerator.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: WaveGenerator.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "combat/wave/WaveGenerator.hpp"

#include "combat/wave/WaveRules.hpp"
#include "entity/MonsterCatalog.hpp"
#include "progression/DifficultyRules.hpp"

#include <algorithm>

namespace
{
    // EN: evolvedMonsterChanceForPlayerLevel declares or implements a focused behavior used by this module.
    // FR: evolvedMonsterChanceForPlayerLevel déclare ou implémente un comportement précis utilisé par ce module.
    int evolvedMonsterChanceForPlayerLevel(int playerLevel)
    {
        if (playerLevel <= 2) return 1;
        if (playerLevel <= 4) return 3;
        if (playerLevel <= 7) return 5;
        if (playerLevel <= 12) return 8;
        if (playerLevel <= 20) return 10;
        return 12;
    }

    int evolvedMonsterChanceCapForDifficulty(DifficultyMode difficulty)
    {
        switch (difficulty)
        {
            case DifficultyMode::Easy: return 6;
            case DifficultyMode::Hard: return 15;
            case DifficultyMode::Nightmare: return 18;
            case DifficultyMode::Lethal: return 22;
            case DifficultyMode::Normal:
            default: return 12;
        }
    }

    bool shouldCreateEvolvedMonster(
        const Player& player,
        int monsterLevel,
        Random& random,
        DifficultyMode difficulty
    )
    {
        int chance = evolvedMonsterChanceForPlayerLevel(player.getLevel())
            + DifficultyRules::getEvolvedMonsterChanceModifier(difficulty);

        if (monsterLevel > player.getLevel())
        {
            chance += 1;
        }

        if (player.getLevel() >= 10)
        {
            chance += 1;
        }

        chance = std::max(0, std::min(chance, evolvedMonsterChanceCapForDifficulty(difficulty)));

        return random.between(1, 100) <= chance;
    }

    // EN: applyPercentage declares or implements a focused behavior used by this module.
    // FR: applyPercentage déclare ou implémente un comportement précis utilisé par ce module.
    int applyPercentage(int value, int percentage)
    {
        int modified = value * percentage / 100;
        return std::max(1, modified);
    }

    // EN: scaleMonsterForDifficulty declares or implements a focused behavior used by this module.
    // FR: scaleMonsterForDifficulty déclare ou implémente un comportement précis utilisé par ce module.
    Monster scaleMonsterForDifficulty(const Monster& monster, DifficultyMode difficulty)
    {
        int hpPercentage = DifficultyRules::getMonsterHealthPercentage(difficulty);
        int damagePercentage = DifficultyRules::getMonsterDamagePercentage(difficulty);

        if (hpPercentage == 100 && damagePercentage == 100)
        {
            return monster;
        }

        return Monster(
            monster.getName(),
            monster.getType(),
            monster.getRace(),
            monster.getLevel(),
            applyPercentage(monster.getMaxHp(), hpPercentage),
            applyPercentage(monster.getMinDamage(), damagePercentage),
            applyPercentage(monster.getMaxDamage(), damagePercentage),
            applyPercentage(monster.getCriticalDamage(), damagePercentage),
            monster.getHealingPotionCount(),
            monster.getDamagePotionCount(),
            monster.isInvocation(),
            monster.isElite(),
            !monster.areStatsVisible(),
            monster.isEvolved()
        );
    }
}

EnemyCombatQueue WaveGenerator::createWaveForPlayer(
    const Player& player,
    Random& random
)
{
    return createWaveForPlayer(player, random, DifficultyMode::Normal);
}

EnemyCombatQueue WaveGenerator::createWaveForPlayer(
    const Player& player,
    Random& random,
    DifficultyMode difficulty
)
{
    EnemyCombatQueue wave;

    int waveSize = WaveRules::getWaveSizeForPlayerLevel(player.getLevel());

    for (int i = 0; i < waveSize; ++i)
    {
        int allowedVariation = 2;
        if (player.getLevel() >= 5)
        {
            allowedVariation = 4;
        }
        if (player.getLevel() >= 10)
        {
            allowedVariation = 7;
        }
        if (player.getLevel() >= 20)
        {
            allowedVariation = 12;
        }
        int levelVariation = random.between(-allowedVariation, allowedVariation);

        int monsterLevel = WaveRules::getMonsterLevelForPlayerLevel(
            player.getLevel(),
            levelVariation
        );

        if (monsterLevel < 1)
        {
            monsterLevel = 1;
        }

        int maximumRandomLevel = player.getLevel() + allowedVariation;
        if (maximumRandomLevel > Player::MAX_LEVEL)
        {
            maximumRandomLevel = Player::MAX_LEVEL;
        }

        if (monsterLevel > maximumRandomLevel)
        {
            monsterLevel = maximumRandomLevel;
        }

        Monster monster = MonsterCatalog::createRandomMonsterForLevel(monsterLevel, random);

        if (shouldCreateEvolvedMonster(player, monsterLevel, random, difficulty))
        {
            monster = MonsterCatalog::createEvolvedVariant(monster, random);
        }

        monster = scaleMonsterForDifficulty(monster, difficulty);

        wave.addWaitingEnemy(monster);
    }

    wave.initializeFrontLine();

    return wave;
}
