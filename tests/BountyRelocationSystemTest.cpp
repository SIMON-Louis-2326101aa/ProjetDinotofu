#include "class_system/ClassCatalog.hpp"
#include "entity/Player.hpp"
#include "item/material/MaterialCatalog.hpp"
#include "quest/Quest.hpp"
#include "world/bounty/BountyRelocationSystem.hpp"

#include <cassert>
#include <iostream>
#include <string>

namespace
{
    Quest mobileBounty()
    {
        Quest quest;
        quest.id = "bounty_relocation_test";
        quest.title = "Prime mobile de test";
        quest.accepted = true;
        quest.bountyMission = true;
        quest.bountyTargetName = "Vela la Passeuse";
        quest.bountyTargetMobility = 82;
        quest.bountyTargetResources = 72;
        quest.bountyTargetLocalSupport = 55;
        quest.bountyTargetKnowsHunt = true;
        quest.bountyTargetEscapeAttempts = 1;
        quest.bountyDaysAtLarge = 110;
        quest.bountyDangerRating = 4;
        quest.bountyClues = 4;
        quest.bountyExitRecon = 2;
        quest.bountyInformantNetwork = 2;
        quest.bountyAmbushPreparation = 2;
        quest.bountyShadowingPreparation = 2;
        quest.bountyRouteVariation = 2;
        quest.bountyRefugeConcealment = 2;
        quest.location = "Route commerciale";
        return quest;
    }
}

int main()
{
    Player player("Pisteur", ClassCatalog::createClassByName("Guerrier"));
    player.setLoadedProgress(20, 0, 1);
    player.setCurrentCityId("valebrume");
    player.getInventory().addMaterial(MaterialCatalog::createById("guild_card", 1));
    player.getInventory().addMaterial(MaterialCatalog::createById("city_service_stamp", 1));

    // Legacy/default bounty location is inferred before fallback: Route commerciale belongs to Valebrume.
    Quest quest = mobileBounty();
    BountyRelocationSystem::ensureLocalPosition(quest, "port_lanterne");
    assert(quest.bountyCurrentCityId == "valebrume");
    assert(quest.bountyCurrentBiome == "Route commerciale");
    assert(quest.location == "Route commerciale");
    assert(quest.bountyVisitedCityIds.find("valebrume") != std::string::npos);
    assert(BountyRelocationSystem::isTargetCurrentlyInCity(quest, "valebrume"));
    assert(!BountyRelocationSystem::isTargetCurrentlyInCity(quest, "rocheveille"));

    // A highly mobile established fugitive can really change city after a failed interception.
    const BountyRelocationResult moved = BountyRelocationSystem::tryRelocateAfterEscape(player, quest, 1);
    assert(moved.evaluated);
    assert(moved.relocationChancePercent > 0);
    assert(moved.moved);
    assert(moved.fromCityId == "valebrume");
    assert(!moved.toCityId.empty());
    assert(moved.toCityId != "valebrume");
    assert(!moved.toBiome.empty());
    assert(quest.bountyCurrentCityId == moved.toCityId);
    assert(quest.bountyCurrentBiome == moved.toBiome);
    assert(quest.location == moved.toBiome);
    assert(quest.bountyRelocationCount == 1);
    assert(quest.bountyClues <= 1);
    assert(quest.bountyExitRecon == 0);
    assert(quest.bountyAmbushPreparation == 0);
    assert(quest.bountyShadowingPreparation == 0);
    assert(quest.bountyVisitedCityIds.find(moved.toCityId) != std::string::npos);
    assert(BountyRelocationSystem::locationSummary(quest).find(moved.toBiome) != std::string::npos);
    assert(quest.bountyTransitTrailAvailable);
    assert(!quest.bountyTransitTrailKind.empty());
    assert(quest.bountyTransitTrailReliability > 0);
    assert(!BountyRelocationSystem::transitTrailSummary(quest).empty());

    const BountyTransitTrailUseResult trailUse = BountyRelocationSystem::useTransitTrail(quest, 20, 1);
    assert(trailUse.evaluated);
    assert(trailUse.useful);
    assert(trailUse.cluesGained >= 1);
    assert(!quest.bountyTransitTrailAvailable);

    // The target does not ping-pong immediately back into an already visited city.
    player.setCurrentCityId(moved.toCityId);
    quest.bountyTargetEscapeAttempts++;
    quest.bountyClues = 4;
    const BountyRelocationResult secondMove = BountyRelocationSystem::tryRelocateAfterEscape(player, quest, 1);
    if (secondMove.moved)
    {
        assert(secondMove.toCityId != "valebrume");
        assert(secondMove.toCityId != moved.toCityId);
        assert(quest.bountyRelocationCount == 2);
    }

    // Low-level players do not receive an impossible relocation behind inaccessible city gates.
    Player novice("Novice", ClassCatalog::createClassByName("Guerrier"));
    novice.setCurrentCityId("valebrume");
    Quest gated = mobileBounty();
    BountyRelocationSystem::ensureLocalPosition(gated, "valebrume");
    const BountyRelocationResult blocked = BountyRelocationSystem::tryRelocateAfterEscape(novice, gated, 1);
    assert(blocked.evaluated);
    assert(!blocked.moved);
    assert(gated.bountyCurrentCityId == "valebrume");

    // Weak/short-lived targets can still escape an interception without automatically changing region.
    Quest local;
    local.id = "bounty_local_escape";
    local.title = "Petite prime locale";
    local.accepted = true;
    local.bountyMission = true;
    local.bountyTargetEscapeAttempts = 1;
    local.bountyTargetMobility = 35;
    local.bountyTargetResources = 25;
    local.bountyDaysAtLarge = 12;
    local.bountyDangerRating = 1;
    local.location = "Route commerciale";
    BountyRelocationSystem::ensureLocalPosition(local, "valebrume");
    const BountyRelocationResult stays = BountyRelocationSystem::tryRelocateAfterEscape(player, local, 1);
    assert(stays.evaluated);
    assert(!stays.moved);
    assert(local.bountyCurrentCityId == "valebrume");

    Quest falseTrail;
    falseTrail.bountyTransitTrailKind = "planted_false_lead";
    falseTrail.bountyTransitTrailFromCityId = "valebrume";
    falseTrail.bountyTransitTrailReliability = 35;
    falseTrail.bountyTransitTrailAvailable = true;
    const BountyTransitTrailUseResult badTrail = BountyRelocationSystem::useTransitTrail(falseTrail, 0, 100);
    assert(badTrail.evaluated);
    assert(!badTrail.useful);
    assert(badTrail.noiseAdded > 0);
    assert(!falseTrail.bountyTransitTrailAvailable);

    std::cout << "BountyRelocationSystemTest OK\n";
    return 0;
}
