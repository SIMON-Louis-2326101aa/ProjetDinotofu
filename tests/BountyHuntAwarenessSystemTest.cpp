#include "world/bounty/BountyHuntAwarenessSystem.hpp"

#include <cassert>
#include <iostream>

int main()
{
    BountyHuntAwarenessContext quiet;
    quiet.targetAwareness = 80;
    quiet.hunterStealth = 90;
    quiet.investigationNoise = 5;
    auto hidden = BountyHuntAwarenessSystem::evaluate(quiet);
    assert(hidden.state == BountyTargetAwarenessState::Unaware);

    BountyHuntAwarenessContext noisy = quiet;
    noisy.hunterStealth = 35;
    noisy.investigationNoise = 70;
    noisy.informantExposure = 50;
    auto detected = BountyHuntAwarenessSystem::evaluate(noisy);
    assert(detected.state != BountyTargetAwarenessState::Unaware);

    BountyHuntAwarenessContext failed = quiet;
    failed.previousFailedPursuits = 2;
    auto confirmed = BountyHuntAwarenessSystem::evaluate(failed);
    assert(confirmed.state == BountyTargetAwarenessState::ConfirmedHunt);

    std::cout << "BountyHuntAwarenessSystemTest OK\n";
    return 0;
}
