#include "world/bounty/BountyHuntAwarenessSystem.hpp"

#include <algorithm>

BountyHuntAwarenessResult BountyHuntAwarenessSystem::evaluate(const BountyHuntAwarenessContext& context)
{
    BountyHuntAwarenessResult result;
    const int awareness = std::clamp(context.targetAwareness, 0, 100);
    const int stealth = std::clamp(context.hunterStealth, 0, 100);
    const int noise = std::clamp(context.investigationNoise, 0, 100);
    const int informants = std::clamp(context.informantExposure, 0, 100);
    const int failed = std::clamp(context.previousFailedPursuits, 0, 6);

    if (context.directWarning || context.sawHunterAct || failed >= 2)
    {
        result.state = BountyTargetAwarenessState::ConfirmedHunt;
        result.detectionScore = 100;
        result.summary = "La cible sait désormais qu'une traque active est en cours.";
        return result;
    }

    result.detectionScore = std::clamp(
        awareness * 40 / 100
        + noise * 45 / 100
        + informants * 35 / 100
        + failed * 22
        - stealth * 32 / 100,
        0, 100);

    if (result.detectionScore >= 68)
    {
        result.state = BountyTargetAwarenessState::ConfirmedHunt;
        result.summary = "Les signes sont trop nombreux : la cible comprend qu'elle est personnellement traquée.";
    }
    else if (result.detectionScore >= 38)
    {
        result.state = BountyTargetAwarenessState::Suspicious;
        result.summary = "La cible remarque quelque chose d'anormal, sans encore savoir qui la cherche ni pourquoi.";
    }
    else
    {
        result.state = BountyTargetAwarenessState::Unaware;
        result.summary = "La cible ne semble pas avoir compris qu'une chasse est en cours et conserve sa routine.";
    }

    return result;
}

std::string BountyHuntAwarenessSystem::stateLabel(BountyTargetAwarenessState state)
{
    switch (state)
    {
        case BountyTargetAwarenessState::Unaware: return "ne sait pas qu'elle est traquée";
        case BountyTargetAwarenessState::Suspicious: return "soupçonne une surveillance";
        case BountyTargetAwarenessState::ConfirmedHunt: return "sait qu'elle est traquée";
    }
    return "état inconnu";
}
