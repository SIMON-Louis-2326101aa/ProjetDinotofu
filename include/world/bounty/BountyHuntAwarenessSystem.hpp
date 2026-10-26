#pragma once

#include <string>

enum class BountyTargetAwarenessState
{
    Unaware,
    Suspicious,
    ConfirmedHunt
};

struct BountyHuntAwarenessContext
{
    int targetAwareness = 50;
    int hunterStealth = 50;
    int investigationNoise = 0;
    int informantExposure = 0;
    int previousFailedPursuits = 0;
    bool directWarning = false;
    bool sawHunterAct = false;
};

struct BountyHuntAwarenessResult
{
    BountyTargetAwarenessState state = BountyTargetAwarenessState::Unaware;
    int detectionScore = 0;
    std::string summary;
};

namespace BountyHuntAwarenessSystem
{
    BountyHuntAwarenessResult evaluate(const BountyHuntAwarenessContext& context);
    std::string stateLabel(BountyTargetAwarenessState state);
}
