#pragma once

#include "combat/ally/RecruitedAllyCombatRoster.hpp"
#include "combat/reward/CombatReward.hpp"

#include <string>
#include <vector>

class Boss;
class Player;
class Random;

enum class RecruitedAllyBossTurnPhase
{
    All,
    LeaderOnly,
    NonLeaderOnly
};

struct RecruitedAllyBossPostKoOutcome
{
    bool bossDefeated = false;
    bool playerRecovered = false;
    int simulatedRounds = 0;
    int reserveEntries = 0;
    int alliesFallen = 0;
    std::vector<RecruitedAllyCombatSupport> participants;
};

auto recruitedAllyBossPresenceLines(const std::vector<RecruitedAllyCombatSupport>& allies) -> std::vector<std::string>;
void playRecruitedAlliesAgainstBoss(
    Player& player,
    Boss& boss,
    std::vector<RecruitedAllyCombatSupport>& allies,
    Random& random,
    std::vector<std::string>& lines,
    RecruitedAllyBossTurnPhase phase = RecruitedAllyBossTurnPhase::All
);
bool tryEmployerBossLeaderRotation(
    Player& player,
    std::vector<RecruitedAllyCombatSupport>& allies,
    std::vector<RecruitedAllyCombatSupport>& reserveAllies,
    Random& random,
    std::vector<std::string>& lines
);
bool tryEmployerBossLeaderRecallPlayer(
    Player& player,
    std::vector<RecruitedAllyCombatSupport>& allies,
    std::vector<RecruitedAllyCombatSupport>& reserveAllies,
    Random& random,
    int roundNumber,
    std::vector<std::string>& lines
);
void applyBossPressureToRecruitedAllies(
    Player& player,
    const Boss& boss,
    std::vector<RecruitedAllyCombatSupport>& allies,
    std::vector<RecruitedAllyCombatSupport>& reserveAllies,
    Random& random,
    std::vector<std::string>& lines
);
CombatReward splitBossRewardWithRecruitedAllies(
    Player& player,
    const CombatReward& reward,
    std::vector<RecruitedAllyCombatSupport>& allies,
    std::vector<std::string>& lines
);
void persistBossRecruitedAllyVitals(
    Player& player,
    const std::vector<RecruitedAllyCombatSupport>& allies,
    const std::string& status
);
RecruitedAllyBossPostKoOutcome simulateRecruitedAlliesAfterPlayerDownAgainstBoss(
    Player& player,
    Boss& boss,
    std::vector<RecruitedAllyCombatSupport>& allies,
    std::vector<RecruitedAllyCombatSupport>& reserveAllies,
    Random& random,
    bool allowBossDefeat,
    int roundOffset = 0
);
