#include "entity/Player.hpp"
#include "world/crime/CrimeContactRelationshipSystem.hpp"
#include "world/crime/CrimeNetworkSystem.hpp"

#include <cassert>

int main()
{
    Player player;
    const std::string serrinId = "valebrume_receleur_brocanteur";
    player.getOrCreateCrimeContactState(serrinId, "Serrin le brocanteur");
    player.getOrCreateCrimeContactState("rocheveille_receleur_peseur", "Dorn le peseur");

    auto initial = CrimeContactRelationshipSystem::assess(player, serrinId);
    assert(initial.known);
    assert(initial.available);
    assert(initial.reliability == 50);

    auto good = CrimeContactRelationshipSystem::recordReliableDeal(player, serrinId, 20, "Lot rendu sans entourloupe");
    assert(good.changed);
    assert(good.reliabilityDelta > 0);
    auto afterGood = CrimeContactRelationshipSystem::assess(player, serrinId);
    assert(afterGood.favorBalance > 0);
    assert(afterGood.reliability > 50);

    auto debt = CrimeContactRelationshipSystem::recordPlayerDebt(player, serrinId, 35, "Service de passage discret");
    assert(debt.changed);
    auto afterDebt = CrimeContactRelationshipSystem::assess(player, serrinId);
    assert(afterDebt.favorBalance < afterGood.favorBalance);

    auto broken = CrimeContactRelationshipSystem::recordBrokenDeal(player, serrinId, 5, "Accord dénoncé devant la garde");
    assert(broken.changed);
    assert(broken.ruptureStarted);
    auto afterBroken = CrimeContactRelationshipSystem::assess(player, serrinId);
    assert(afterBroken.ruptured);
    assert(!afterBroken.available);
    assert(afterBroken.rivalry > 0);

    const auto relayWhileBroken = CrimeNetworkSystem::assessKnownRelay(player, serrinId, "rocheveille_receleur_peseur");
    assert(relayWhileBroken.known);
    assert(relayWhileBroken.sameNetwork);
    assert(!relayWhileBroken.available);

    player.advanceWorldDays(30);
    player.getOrCreateCrimeContactState(serrinId, "Serrin le brocanteur"); // applies lifecycle checks.
    auto repaired = CrimeContactRelationshipSystem::recordReliableDeal(player, serrinId, 0, "Retour prudent après rupture");
    assert(repaired.changed);
    auto afterTime = CrimeContactRelationshipSystem::assess(player, serrinId);
    assert(afterTime.available);

    const std::string alternate = CrimeContactRelationshipSystem::chooseKnownAlternateRelay(player, serrinId);
    assert(alternate == "rocheveille_receleur_peseur");
    return 0;
}
