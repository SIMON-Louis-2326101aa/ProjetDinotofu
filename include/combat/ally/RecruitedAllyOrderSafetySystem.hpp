#pragma once

#include <string>

class Random;

struct RecruitedAllyOrderSafetyContext
{
    int hpPercent = 100;
    int readinessPercent = 100;
    int moralePercent = 50;
    int enemyCount = 1;
    int relationshipScore = 0;
    bool bossContext = false;
    bool aggressiveForcedOrder = false;
};

struct RecruitedAllyOrderSafetyDecision
{
    int dangerScore = 0;
    int refusalChancePercent = 0;
    bool catastrophic = false;
    bool refused = false;
    std::string reason;
};

namespace RecruitedAllyOrderSafetySystem
{
    RecruitedAllyOrderSafetyDecision evaluate(
        const RecruitedAllyOrderSafetyContext& context,
        Random& random
    );
}
