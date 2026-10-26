#include "world/bounty/BountyQuestSystem.hpp"

#include <cassert>
#include <iostream>

int main()
{
    Quest q;
    q.id = "bounty_test";
    q.title = "Prime ciblée de test";
    q.accepted = true;
    q.bountyMission = true;
    q.rewardGold = 100;
    BountyQuestSystem::initializeBounty(q, 12);
    assert(!q.bountyTargetName.empty());
    assert(!q.bountyTargetKnowsHunt);
    assert(q.bountyRewardProfileApplied);
    assert(q.bountyBaseRewardGold == 100);
    assert(q.bountyDangerRating >= 1 && q.bountyDangerRating <= 5);
    assert(q.bountyNotorietyRating >= 1 && q.bountyNotorietyRating <= 5);
    assert(q.bountyDaysAtLarge > 0);
    assert(q.rewardGold >= q.bountyBaseRewardGold);
    assert(BountyQuestSystem::contractValueSummary(q).find("En fuite") != std::string::npos);

    // Defensive re-initialization must never reroll an existing target.
    const std::string originalName = q.bountyTargetName;
    const std::string originalTemperament = q.bountyTargetTemperament;
    const int originalResources = q.bountyTargetResources;
    const int originalMobility = q.bountyTargetMobility;
    const int originalReward = q.rewardGold;
    const int originalDaysAtLarge = q.bountyDaysAtLarge;
    BountyQuestSystem::initializeBounty(q, 1);
    assert(q.bountyTargetName == originalName);
    assert(q.bountyTargetTemperament == originalTemperament);
    assert(q.bountyTargetResources == originalResources);
    assert(q.bountyTargetMobility == originalMobility);
    assert(q.rewardGold == originalReward);
    assert(q.bountyDaysAtLarge == originalDaysAtLarge);

    // Quiet work may build a real trail while the target remains unaware.
    q.bountyTargetAwareness = 45;
    auto quiet = BountyQuestSystem::investigate(q, BountyInvestigationAction::ObserveQuietly, 90, 10);
    assert(quiet.changed);
    assert(q.bountyClues >= 1);
    assert(!q.bountyTargetKnowsHunt);

    BountyQuestSystem::investigate(q, BountyInvestigationAction::CheckExits, 90, 10);
    BountyQuestSystem::investigate(q, BountyInvestigationAction::ObserveQuietly, 90, 10);
    assert(q.bountyClues >= 3);

    BountyPursuitHunterProfile hunter;
    hunter.trackingSkill = 80;
    hunter.routeControl = 80;
    hunter.stealthSkill = 85;
    hunter.coordination = 70;
    auto intercepted = BountyQuestSystem::attemptPursuit(q, hunter, 100);
    assert(intercepted.intercepted);
    assert(q.bountyStage == "confrontation");

    Quest escape = q;
    escape.completed = false;
    escape.bountyStage = "preparation";
    escape.bountyClues = 4;
    escape.bountyTargetKnowsHunt = false;
    escape.bountyTargetSuspectsHunt = false;
    BountyPursuitHunterProfile weakHunter;
    weakHunter.trackingSkill = 20;
    weakHunter.routeControl = 20;
    weakHunter.stealthSkill = 20;
    weakHunter.coordination = 20;
    auto fled = BountyQuestSystem::attemptPursuit(escape, weakHunter, 1);
    assert(fled.escaped);
    assert(escape.bountyTargetKnowsHunt);
    assert(escape.bountyStage == "investigation");

    std::cout << "BountyQuestSystemTest OK\n";
    return 0;
}
