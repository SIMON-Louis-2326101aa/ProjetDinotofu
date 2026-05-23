// EN: CombatRewardSystem.hpp briefly defines this Dinotofu module and its responsibilities.
// FR: CombatRewardSystem.hpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#ifndef INCLUDE_COMBAT_REWARD_COMBATREWARDSYSTEM_HPP
#define INCLUDE_COMBAT_REWARD_COMBATREWARDSYSTEM_HPP

#include "combat/EnemyCombatQueue.hpp"
#include "combat/reward/CombatReward.hpp"

#include "entity/Boss.hpp"
#include "entity/Monster.hpp"
#include "entity/Player.hpp"
#include "progression/DifficultyMode.hpp"
#include "core/Random.hpp"

#include <string>

class CombatRewardSystem
{
public:
    static constexpr int PLAYER_PVE_ESCAPE_DEFEATED_REWARD_PERCENTAGE = 50;
    static constexpr int PLAYER_PVE_ESCAPE_DAMAGED_ALIVE_REWARD_PERCENTAGE = 25;
    static constexpr int MONSTER_ESCAPE_REWARD_PERCENTAGE = 75;

    // EN: calculateMonsterReward declares or implements a focused behavior used by this module.
    // FR: calculateMonsterReward déclare ou implémente un comportement précis utilisé par ce module.
    static CombatReward calculateMonsterReward(const Monster& monster, Random* random = nullptr);
    static CombatReward calculateBossReward(
        const Boss& boss,
        DifficultyMode difficulty,
        int damageReceived,
        int turnCount
    );

    // EN: calculateDefeatedEnemiesReward declares or implements a focused behavior used by this module.
    // FR: calculateDefeatedEnemiesReward déclare ou implémente un comportement précis utilisé par ce module.
    static CombatReward calculateDefeatedEnemiesReward(const EnemyCombatQueue& wave);
    // EN: calculateEscapedEnemiesReward declares or implements a focused behavior used by this module.
    // FR: calculateEscapedEnemiesReward déclare ou implémente un comportement précis utilisé par ce module.
    static CombatReward calculateEscapedEnemiesReward(const EnemyCombatQueue& wave);
    // EN: calculateDamagedAliveEnemiesReward declares or implements a focused behavior used by this module.
    // FR: calculateDamagedAliveEnemiesReward déclare ou implémente un comportement précis utilisé par ce module.
    static CombatReward calculateDamagedAliveEnemiesReward(const EnemyCombatQueue& wave);
    static CombatReward calculateDamagedAliveEnemiesReward(
        const EnemyCombatQueue& wave,
        DifficultyMode difficulty
    );
    // EN: calculateWaveReward declares or implements a focused behavior used by this module.
    // FR: calculateWaveReward déclare ou implémente un comportement précis utilisé par ce module.
    static CombatReward calculateWaveReward(const EnemyCombatQueue& wave);
    static CombatReward calculateWaveReward(
        const EnemyCombatQueue& wave,
        DifficultyMode difficulty
    );
    static CombatReward calculateWaveReward(
        const EnemyCombatQueue& wave,
        DifficultyMode difficulty,
        const Player& player,
        int initialPlayerHp,
        int turnCount
    );
    static CombatReward calculateWaveReward(
        const EnemyCombatQueue& wave,
        DifficultyMode difficulty,
        const Player& player,
        int initialPlayerHp,
        int turnCount,
        Random& random
    );

    // EN: calculatePlayerEscapeReward declares or implements a focused behavior used by this module.
    // FR: calculatePlayerEscapeReward déclare ou implémente un comportement précis utilisé par ce module.
    static CombatReward calculatePlayerEscapeReward(const EnemyCombatQueue& wave);
    static CombatReward calculatePlayerEscapeReward(
        const EnemyCombatQueue& wave,
        DifficultyMode difficulty
    );

    static void giveRewardToPlayer(
        Player& player,
        const CombatReward& reward
    );

    static void displayReward(
        const CombatReward& reward
    );

    static void displayPartialReward(
        const CombatReward& reward,
        const std::string& reason
    );
};

#endif
