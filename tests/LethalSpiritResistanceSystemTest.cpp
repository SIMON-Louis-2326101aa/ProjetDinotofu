#include "combat/death/LethalSpiritResistanceSystem.hpp"
#include "core/Random.hpp"
#include "entity/Player.hpp"

#include <cassert>

namespace
{
    unsigned int seedForSpiritOutcome(bool wantedSurvival)
    {
        for (unsigned int seed = 1; seed < 100000; ++seed)
        {
            Random probe(seed);
            int successes = 0;
            int failures = 0;
            while (successes < LethalSpiritResistanceSystem::RequiredSuccesses
                && failures < LethalSpiritResistanceSystem::RequiredFailures)
            {
                if (probe.rollD20() >= LethalSpiritResistanceSystem::D20SuccessThreshold) ++successes;
                else ++failures;
            }
            const bool survived = successes >= LethalSpiritResistanceSystem::RequiredSuccesses;
            if (survived == wantedSurvival) return seed;
        }
        return 1;
    }
}

int main()
{
    Player living;
    Random irrelevant(1);
    const auto livingResult = LethalSpiritResistanceSystem::tryFirstLethalResistance(
        living, irrelevant, "living");
    assert(!livingResult.available);
    assert(!livingResult.consumed);

    Player survivor;
    survivor.takeDamage(survivor.getMaxHp() * 10);
    assert(survivor.isDead());
    Random successRandom(seedForSpiritOutcome(true));
    const auto success = LethalSpiritResistanceSystem::tryFirstLethalResistance(
        survivor, successRandom, "first_success");
    assert(success.available);
    assert(success.consumed);
    assert(success.survived);
    assert(success.successes == 3);
    assert(success.failures <= 2);
    assert(!survivor.isDead());
    assert(LethalSpiritResistanceSystem::hasAlreadyConsumedFirstResistance(survivor));

    survivor.takeDamage(survivor.getMaxHp() * 10);
    Random secondRandom(seedForSpiritOutcome(true));
    const auto second = LethalSpiritResistanceSystem::tryFirstLethalResistance(
        survivor, secondRandom, "second_attempt");
    assert(!second.available);
    assert(!second.consumed);
    assert(!second.survived);
    assert(survivor.isDead());

    Player failed;
    failed.takeDamage(failed.getMaxHp() * 10);
    Random failureRandom(seedForSpiritOutcome(false));
    const auto failure = LethalSpiritResistanceSystem::tryFirstLethalResistance(
        failed, failureRandom, "first_failure");
    assert(failure.available);
    assert(failure.consumed);
    assert(!failure.survived);
    assert(failure.failures == 3);
    assert(failure.successes <= 2);
    assert(failed.isDead());
    assert(LethalSpiritResistanceSystem::hasAlreadyConsumedFirstResistance(failed));

    Random retryRandom(seedForSpiritOutcome(true));
    const auto retry = LethalSpiritResistanceSystem::tryFirstLethalResistance(
        failed, retryRandom, "retry_after_failure");
    assert(!retry.available);
    assert(!retry.consumed);
    assert(failed.isDead());

    return 0;
}
