#include <cassert>

#include "class_system/ClassCatalog.hpp"
#include "core/Random.hpp"
#include "entity/Player.hpp"
#include "world/reapers/ReaperWorldSystem.hpp"

int main()
{
    Player natural("Observateur", ClassCatalog::createClassByName("Guerrier"));
    assert(!ReaperWorldSystem::hasCredibleKnowledge(natural));

    // Find a deterministic seed that triggers the rare natural encounter without weakening its chance.
    bool naturalTriggered = false;
    for (unsigned int seed = 1; seed < 500 && !naturalTriggered; ++seed)
    {
        Player probe = natural;
        Random random(seed);
        const auto result = ReaperWorldSystem::tryExplorationEncounter(probe, random, "Route commerciale");
        if (!result.occurred) continue;
        naturalTriggered = true;
        assert(result.firstCredibleKnowledge);
        assert(!result.recruitmentOffered);
        assert(ReaperWorldSystem::hasCredibleKnowledge(probe));
        assert(ReaperWorldSystem::observationScore(probe) >= 1);
    }
    assert(naturalTriggered);

    // Optional-faction guard: no natural REAPERS-13 sighting in an ordinary safe biome,
    // regardless of RNG. This does not apply to a deliberately transmitted Protocol 13 signal.
    for (unsigned int seed = 1; seed <= 500; ++seed)
    {
        Player safeProbe("Promeneur", ClassCatalog::createClassByName("Guerrier"));
        Random random(seed);
        const auto result = ReaperWorldSystem::tryExplorationEncounter(safeProbe, random, "Prairie paisible");
        assert(!result.occurred);
    }

    // Once a natural trace was seen, the faction cannot crowd ordinary exploration again for 12 days.
    Player cooldownBase;
    bool cooldownSeedFound = false;
    for (unsigned int seed = 1; seed < 500 && !cooldownSeedFound; ++seed)
    {
        Player probe("Témoin cooldown", ClassCatalog::createClassByName("Guerrier"));
        Random random(seed);
        const auto first = ReaperWorldSystem::tryExplorationEncounter(probe, random, "Route dangereuse");
        if (!first.occurred) continue;
        cooldownBase = probe;
        cooldownSeedFound = true;
    }
    assert(cooldownSeedFound);

    for (unsigned int seed = 1; seed <= 500; ++seed)
    {
        Player probe = cooldownBase;
        Random random(seed);
        assert(!ReaperWorldSystem::tryExplorationEncounter(probe, random, "Route dangereuse").occurred);
    }

    Player dayEleven = cooldownBase;
    dayEleven.advanceWorldDays(11);
    for (unsigned int seed = 1; seed <= 500; ++seed)
    {
        Player probe = dayEleven;
        Random random(seed);
        assert(!ReaperWorldSystem::tryExplorationEncounter(probe, random, "Route dangereuse").occurred);
    }

    Player dayTwelve = cooldownBase;
    dayTwelve.advanceWorldDays(12);
    bool canReappearAfterCooldown = false;
    for (unsigned int seed = 1; seed < 500 && !canReappearAfterCooldown; ++seed)
    {
        Player probe = dayTwelve;
        Random random(seed);
        const auto result = ReaperWorldSystem::tryExplorationEncounter(probe, random, "Route dangereuse");
        if (!result.occurred) continue;
        canReappearAfterCooldown = true;
        assert(!result.recruitmentOffered);
    }
    assert(canReappearAfterCooldown);

    Player signaled("Signaleur", ClassCatalog::createClassByName("Guerrier"));
    signaled.recordCanonicalEvent("reapers13_knowledge", "npc_rumor", "Rumeur crédible", 1);
    signaled.recordCanonicalEvent("protocoles_secrets", "reapers13_protocol_13_signal", "Balise transmise", 1);
    assert(ReaperWorldSystem::hasPendingProtocol13Signal(signaled));

    bool responseTriggered = false;
    for (unsigned int seed = 1; seed < 100 && !responseTriggered; ++seed)
    {
        Player probe = signaled;
        Random random(seed);
        const auto result = ReaperWorldSystem::tryExplorationEncounter(probe, random, "Forêt ancienne");
        if (!result.occurred) continue;
        responseTriggered = true;
        assert(result.respondedToProtocol13);
        assert(!result.recruitmentOffered);
        assert(ReaperWorldSystem::hasReceivedProtocol13Response(probe));
        assert(!ReaperWorldSystem::hasPendingProtocol13Signal(probe));
    }
    assert(responseTriggered);
    return 0;
}
