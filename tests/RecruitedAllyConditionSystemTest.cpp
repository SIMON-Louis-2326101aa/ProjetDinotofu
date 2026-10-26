#include <cassert>

#include "class_system/ClassCatalog.hpp"
#include "combat/ally/RecruitedAllyConditionSystem.hpp"
#include "combat/ally/RecruitedAllyInitiativeSystem.hpp"
#include "entity/Player.hpp"

int main()
{
    Player player("Chef", ClassCatalog::createClassByName("Guerrier"));
    RecruitedAllyCombatSupport ally;
    ally.name = "Aline";
    ally.job = "Éclaireuse";
    ally.trait = "calme loyale agile";
    ally.motivation = "objectif_commun";
    ally.estimatedLevel = 22;
    ally.currentRankIndex = 2;
    ally.weaponQuality = 4;
    ally.maxHp = 100;
    ally.currentHp = 42;
    ally.turnsTaken = 5;
    ally.teamOrigin = AllyTeamOrigin::PersonalRecruit;

    const auto fresh = RecruitedAllyConditionSystem::evaluate(player, ally);
    assert(fresh.fatiguePercent == 0);

    RecruitedAllyConditionSystem::recordCombatStrain(player, ally, false);
    auto strained = RecruitedAllyConditionSystem::evaluate(player, ally);
    assert(strained.fatiguePercent >= 20);
    assert(strained.readinessPercent < fresh.readinessPercent);

    RecruitedAllyCombatSupport freshSupport = ally;
    freshSupport.fatiguePercent = 0;
    freshSupport.moralePercent = 60;
    freshSupport.readinessPercent = 90;
    RecruitedAllyCombatSupport tiredSupport = freshSupport;
    RecruitedAllyConditionSystem::applyToCombatSupport(player, tiredSupport);
    assert(tiredSupport.fatiguePercent == strained.fatiguePercent);
    assert(RecruitedAllyInitiativeSystem::baseInitiativeScore(tiredSupport)
        < RecruitedAllyInitiativeSystem::baseInitiativeScore(freshSupport));

    player.advanceWorldDays(2);
    const auto rested = RecruitedAllyConditionSystem::evaluate(player, ally);
    assert(rested.fatiguePercent < strained.fatiguePercent);

    ally.currentHp = 10;
    ally.turnsTaken = 7;
    RecruitedAllyConditionSystem::recordCombatStrain(player, ally, true);
    const auto exhausted = RecruitedAllyConditionSystem::evaluate(player, ally);
    assert(exhausted.fatiguePercent > rested.fatiguePercent);
    assert(exhausted.readinessPercent < 70);
    assert(!RecruitedAllyConditionSystem::summary(exhausted).empty());

    // Severe injury/KO leaves a slower-recovering light strain, distinct from ordinary fatigue.
    ally.currentHp = 0;
    ally.turnsTaken = 3;
    RecruitedAllyConditionSystem::recordCombatStrain(player, ally, true);
    const auto shaken = RecruitedAllyConditionSystem::evaluate(player, ally);
    assert(shaken.injuryStressPercent > 0);
    assert(shaken.shaken || shaken.injuryStressPercent >= 15);
    player.advanceWorldDays(2);
    const auto twoDaysLater = RecruitedAllyConditionSystem::evaluate(player, ally);
    assert(twoDaysLater.injuryStressPercent < shaken.injuryStressPercent);
    assert(twoDaysLater.injuryStressPercent > 0); // slower than normal fatigue
    assert(twoDaysLater.injuryStateId != "none");
    assert(RecruitedAllyConditionSystem::needsRecoveryCare(player, ally));

    const auto care = RecruitedAllyConditionSystem::provideRecoveryCare(player, ally, 2);
    assert(care.applied);
    assert(care.injuryStressAfter < care.injuryStressBefore);
    assert(care.fatigueAfter <= care.fatigueBefore);
    const auto afterCare = RecruitedAllyConditionSystem::evaluate(player, ally);
    assert(afterCare.injuryStressPercent == care.injuryStressAfter);
    return 0;
}
