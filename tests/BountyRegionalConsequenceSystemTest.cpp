#include "class_system/ClassCatalog.hpp"
#include "entity/Player.hpp"
#include "quest/Quest.hpp"
#include "world/bounty/BountyRegionalConsequenceSystem.hpp"
#include "world/rumor/WorldRumorSystem.hpp"

#include <cassert>
#include <iostream>

namespace
{
    Quest makeBounty(const std::string& id, const std::string& target)
    {
        Quest quest;
        quest.id = id;
        quest.title = "Prime de test";
        quest.accepted = true;
        quest.bountyMission = true;
        quest.bountyTargetName = target;
        quest.bountyTargetAwareness = 40;
        quest.bountyTargetLocalSupport = 40;
        quest.bountyTargetResources = 40;
        quest.bountyTargetMobility = 40;
        return quest;
    }
}

int main()
{
    Player player("Traqueur", ClassCatalog::createClassByName("Guerrier"));
    player.setCurrentCityId("valebrume");

    PlayerCrimeContactState& knownContact = player.getOrCreateCrimeContactState(
        "valebrume_receleur_brocanteur",
        "Serrin le brocanteur");
    knownContact.pressure = 5;
    knownContact.cooperation = 12;

    Quest first = makeBounty("bounty_region_1", "Marek Sans-Lanterne");
    first.bountyInvestigationNoise = 12;
    first.bountyInformantExposure = 4;

    const BountyRegionalConsequenceResult captured = BountyRegionalConsequenceSystem::applyResolution(
        player,
        first,
        BountyResolutionKind::CapturedBySurrender);
    assert(captured.applied);
    assert(first.bountyRegionalConsequencesApplied);
    assert(first.bountyResolution == "captured_surrender");
    assert(captured.localReputationGain == 3);
    assert(captured.knownCrimeContactsAffected == 1);
    assert(!captured.noisyHunt);

    const PlayerCrimeContactState* contactAfter = player.findCrimeContactState("valebrume_receleur_brocanteur");
    assert(contactAfter != nullptr);
    assert(contactAfter->pressure > 5);
    assert(contactAfter->cooperation < 12);

    BountyRegionalState state = BountyRegionalConsequenceSystem::stateFor(player, "valebrume");
    assert(state.aliveCaptures == 1);
    assert(state.lethalResolutions == 0);
    assert(state.noisyHunts == 0);
    assert(state.awarenessPressure > 0);
    assert(state.supportShift < 0);

    const WorldRumorStatus rumor = WorldRumorSystem::statusFor(
        player,
        "bounty_resolution:valebrume:bounty_region_1");
    assert(rumor.known);
    assert(rumor.subjectId == "bounty_region_1");
    assert(rumor.evidenceLevel >= 4);

    // Resolution is idempotent: re-entering a screen after reload must not double the consequences.
    const BountyRegionalConsequenceResult duplicate = BountyRegionalConsequenceSystem::applyResolution(
        player,
        first,
        BountyResolutionKind::KilledInCombat);
    assert(!duplicate.applied);
    state = BountyRegionalConsequenceSystem::stateFor(player, "valebrume");
    assert(state.aliveCaptures == 1);
    assert(state.lethalResolutions == 0);

    Quest adapted = makeBounty("bounty_region_2", "Vela la Passeuse");
    const int initialAwareness = adapted.bountyTargetAwareness;
    const int initialSupport = adapted.bountyTargetLocalSupport;
    const BountyRegionalAdaptationResult adaptation = BountyRegionalConsequenceSystem::adaptTargetToRegion(player, adapted);
    assert(adaptation.applied);
    assert(adapted.bountyRegionalProfileApplied);
    assert(adapted.bountyTargetAwareness > initialAwareness);
    assert(adapted.bountyTargetLocalSupport < initialSupport);

    const int onceAwareness = adapted.bountyTargetAwareness;
    const int onceSupport = adapted.bountyTargetLocalSupport;
    const BountyRegionalAdaptationResult noSecondAdaptation = BountyRegionalConsequenceSystem::adaptTargetToRegion(player, adapted);
    assert(!noSecondAdaptation.applied);
    assert(adapted.bountyTargetAwareness == onceAwareness);
    assert(adapted.bountyTargetLocalSupport == onceSupport);

    // Relocation to another city re-evaluates the new environment exactly once instead of stacking
    // the old city's security modifier forever.
    adapted.bountyCurrentCityId = "port_lanterne";
    player.setCurrentCityId("port_lanterne");
    const BountyRegionalAdaptationResult movedRegion = BountyRegionalConsequenceSystem::adaptTargetToRegion(player, adapted);
    assert(movedRegion.applied);
    assert(adapted.bountyRegionalProfileCityId == "port_lanterne");
    const int movedAwareness = adapted.bountyTargetAwareness;
    const BountyRegionalAdaptationResult movedRegionAgain = BountyRegionalConsequenceSystem::adaptTargetToRegion(player, adapted);
    assert(!movedRegionAgain.applied);
    assert(adapted.bountyTargetAwareness == movedAwareness);
    player.setCurrentCityId("valebrume");

    // Legacy V3.51.74 save: the old boolean can be true while the new city-id field is absent.
    // Reloading such a contract must bind the existing adaptation to the current city without
    // applying its security/economy modifiers twice.
    Quest legacyAdapted = makeBounty("bounty_region_legacy", "Orsen l'Ancien");
    legacyAdapted.bountyRegionalProfileApplied = true;
    legacyAdapted.bountyCurrentCityId = "valebrume";
    const int legacyAwareness = legacyAdapted.bountyTargetAwareness;
    const int legacySupport = legacyAdapted.bountyTargetLocalSupport;
    const BountyRegionalAdaptationResult legacyCompatibility = BountyRegionalConsequenceSystem::adaptTargetToRegion(player, legacyAdapted);
    assert(!legacyCompatibility.applied);
    assert(legacyAdapted.bountyRegionalProfileCityId == "valebrume");
    assert(legacyAdapted.bountyTargetAwareness == legacyAwareness);
    assert(legacyAdapted.bountyTargetLocalSupport == legacySupport);

    Quest lethal = makeBounty("bounty_region_3", "Sira la Grise");
    lethal.bountyInvestigationNoise = 52;
    lethal.bountyInformantExposure = 31;
    lethal.bountyTargetEscapeAttempts = 2;
    const BountyRegionalConsequenceResult killed = BountyRegionalConsequenceSystem::applyResolution(
        player,
        lethal,
        BountyResolutionKind::KilledInCombat);
    assert(killed.applied);
    assert(killed.noisyHunt);

    state = BountyRegionalConsequenceSystem::stateFor(player, "valebrume");
    assert(state.aliveCaptures == 1);
    assert(state.lethalResolutions == 1);
    assert(state.noisyHunts == 1);
    assert(state.recordedEscapeAttempts == 2);
    assert(state.mobilityPressure > 0);

    Quest learned = makeBounty("bounty_region_4", "Neris le Fuyard");
    const int baseMobility = learned.bountyTargetMobility;
    BountyRegionalConsequenceSystem::adaptTargetToRegion(player, learned);
    assert(learned.bountyTargetMobility > baseMobility);
    assert(learned.bountyTargetAwareness > 40);

    std::cout << "BountyRegionalConsequenceSystemTest OK\n";
    return 0;
}
