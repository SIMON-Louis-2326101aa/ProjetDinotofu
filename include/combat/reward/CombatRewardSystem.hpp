// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#ifndef INCLUDE_COMBAT_REWARD_COMBATREWARDSYSTEM_HPP
#define INCLUDE_COMBAT_REWARD_COMBATREWARDSYSTEM_HPP

#include "combat/EnemyCombatQueue.hpp"
#include "combat/reward/CombatReward.hpp"

#include "entity/Monster.hpp"
#include "entity/Player.hpp"

#include <string>

class CombatRewardSystem
{
public:
    static constexpr int PLAYER_PVE_ESCAPE_DEFEATED_REWARD_PERCENTAGE = 50;
    static constexpr int PLAYER_PVE_ESCAPE_DAMAGED_ALIVE_REWARD_PERCENTAGE = 25;
    static constexpr int MONSTER_ESCAPE_REWARD_PERCENTAGE = 75;

    static CombatReward calculateMonsterReward(const Monster& monster);

    static CombatReward calculateDefeatedEnemiesReward(const EnemyCombatQueue& wave);
    static CombatReward calculateEscapedEnemiesReward(const EnemyCombatQueue& wave);
    static CombatReward calculateDamagedAliveEnemiesReward(const EnemyCombatQueue& wave);
    static CombatReward calculateWaveReward(const EnemyCombatQueue& wave);

    static CombatReward calculatePlayerEscapeReward(const EnemyCombatQueue& wave);

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
