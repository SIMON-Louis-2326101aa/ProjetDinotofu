#pragma once

#include "combat/ally/RecruitedAllyCombatRoster.hpp"

#include <string>

class Random;

enum class RecruitedAllyActionType
{
    Hold,
    SelfHeal,
    HealLeader,
    HealAlly,
    Guard,
    Technique,
    Attack
};

struct RecruitedAllyActionContext
{
    bool hasEnemy = true;
    int enemyCount = 1;
    bool bossContext = false;

    bool leaderPresent = true;
    int leaderHpPercent = 100;
    bool leaderCanBeHealed = true;

    bool woundedAllyAvailable = false;
    int woundedAllyHpPercent = 100;

    bool orderedHeal = false;
    bool orderedGuard = false;
    bool orderedTechnique = false;
    bool hasDirectOrder = false;

    bool techniqueAvailable = true;
    bool techniqueAllowed = true;
    int dangerPercent = 50;
    int relationshipScore = 0;
};

struct RecruitedAllyActionDecision
{
    RecruitedAllyActionType type = RecruitedAllyActionType::Hold;
    bool followsOrder = false;
    std::string reason;
};

namespace RecruitedAllyActionPlanner
{
    bool isSupportProfile(const RecruitedAllyCombatSupport& ally);
    bool isGuardProfile(const RecruitedAllyCombatSupport& ally);
    int naturalSupportChance(const RecruitedAllyCombatSupport& ally);
    int naturalTechniqueChance(const RecruitedAllyCombatSupport& ally, int enemyCount, bool bossContext);
    int orderedTechniqueChance(const RecruitedAllyCombatSupport& ally, bool forced);

    RecruitedAllyActionDecision decide(
        const RecruitedAllyCombatSupport& ally,
        const RecruitedAllyActionContext& context,
        Random& random
    );

    std::string actionTypeLabel(RecruitedAllyActionType type);
}
