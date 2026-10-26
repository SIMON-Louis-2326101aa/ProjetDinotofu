#pragma once

#include "world/bounty/BountyPursuitSystem.hpp"

#include <string>
#include <vector>

struct BountyTargetBehaviorContext
{
    std::string temperament;
    int resources = 50;
    int awareness = 50;
    // Being wanted is not the same as knowing a hunter is actively following you.
    bool knowsItIsHunted = false;
    bool suspectsSurveillance = false;
    int preparationWindow = 1;
    int localSupport = 0;
    int mobility = 50;
    int previousEscapeAttempts = 0;
    bool knowsTerrain = false;
    bool injured = false;
};

struct BountyTargetBehaviorDecision
{
    BountyPursuitPreparation preparation;
    std::vector<BountyTargetCountermeasure> selectedActions;
    int availableActions = 0;
    int counterplayScore = 0;
    std::string summary;
};

namespace BountyTargetBehaviorSystem
{
    // Chooses only from facts available to the target itself: its temperament, resources,
    // awareness, local support and time. No inventory/build omniscience about the hunter.
    BountyTargetBehaviorDecision chooseCountermeasures(
        const BountyTargetBehaviorContext& context,
        const BountyPursuitPreparation& existing = {}
    );

    std::string actionLabel(BountyTargetCountermeasure action);
}
