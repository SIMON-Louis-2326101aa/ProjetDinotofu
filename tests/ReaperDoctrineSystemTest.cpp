#include <cassert>

#include "class_system/ClassCatalog.hpp"
#include "entity/Player.hpp"
#include "world/reapers/ReaperDoctrineSystem.hpp"
#include "world/rumor/WorldRumorSystem.hpp"

int main()
{
    const auto& legends = ReaperDoctrineSystem::legendCatalog();
    assert(legends.size() >= 11);
    assert(ReaperDoctrineSystem::findLegend("reaper13_not_every_bounty") != nullptr);
    assert(ReaperDoctrineSystem::findLegend("reaper13_fallen_agent_or_clone") != nullptr);
    assert(ReaperDoctrineSystem::findLegend("reapers_shadow_bounty_hunters") != nullptr);

    Player player("Lecteur", ClassCatalog::createClassByName("Guerrier"));
    ReaperDoctrineSystem::hearLegend(player, "reaper13_not_every_bounty", "tavern");
    auto rumor = WorldRumorSystem::statusFor(player, "reaper13_not_every_bounty");
    assert(rumor.known);
    assert(!rumor.confirmed);
    assert(rumor.subjectId == "reapers13");

    const auto weakCase = ReaperDoctrineSystem::assessHunt(30, 90, 8, true, true, false);
    assert(!weakCase.accepted); // no sentence without evidence

    const auto mistake = ReaperDoctrineSystem::assessHunt(90, 20, 0, false, false, true);
    assert(!mistake.accepted);

    const auto dangerous = ReaperDoctrineSystem::assessHunt(88, 85, 5, true, true, false);
    assert(dangerous.accepted);
    assert(dangerous.priorityTarget);
    assert(dangerous.lethalForceJustified);

    const auto surrender = ReaperDoctrineSystem::assessHunt(90, 82, 4, true, false, true);
    assert(surrender.accepted);
    assert(surrender.executionExpected);
    assert(surrender.lethalForceJustified); // une reddition ne gomme pas automatiquement un dossier déjà jugé exécutable

    // Core identity: powerful shadow bounty hunters close escape routes by preparation, not magic.
    const auto pursuit = ReaperDoctrineSystem::planPursuit(72, 2, false, true, false);
    assert(pursuit.hiddenApproach);
    assert(pursuit.containmentPercent >= 58);
    assert(pursuit.extraEscapeOpenings >= 1);
    assert(!pursuit.capturePreferred);
    assert(BountyPursuitSystem::adjustedEscapeChance(70, pursuit) < 70);

    const auto murderer = ReaperDoctrineSystem::assessHunt(92, 40, 0, false, false, true, true);
    assert(murderer.accepted);
    assert(murderer.executionExpected);
    assert(murderer.lethalForceJustified);

    const auto cleanRecord = ReaperDoctrineSystem::assessExecutionRecord(3, 0);
    assert(cleanRecord.respectedRecord);
    assert(cleanRecord.meritScore > 0);
    const auto stainedRecord = ReaperDoctrineSystem::assessExecutionRecord(6, 1);
    assert(!stainedRecord.respectedRecord);
    return 0;
}
