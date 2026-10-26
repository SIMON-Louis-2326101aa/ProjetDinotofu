#include "combat/group/EmployerTeamDisciplineSystem.hpp"
#include "combat/group/EmployerTeamRosterSystem.hpp"
#include "combat/group/TeamEmploymentSystem.hpp"
#include "class_system/ClassCatalog.hpp"
#include "entity/Player.hpp"

#include <cassert>

int main()
{
    Player player("DisciplineTester", ClassCatalog::createClassByName("Guerrier"));
    assert(TeamEmploymentSystem::startExternalContract(
        player, "discipline_contract", "Capitaine Rhea", "watch", "Veilleurs", "membre", 100, 7, -1));

    auto state = EmployerTeamDisciplineSystem::stateFor(player);
    assert(state.professionalTrust == 70);
    assert(state.warnings == 0);
    assert(!EmployerTeamDisciplineSystem::playerStartsInReserve(player));

    EmployerTeamDisciplineSystem::recordMissionCompleted(player, "mission_ok");
    state = EmployerTeamDisciplineSystem::stateFor(player);
    assert(state.professionalTrust == 79);
    assert(state.completedMissions == 1);

    EmployerTeamDisciplineSystem::recordMissionFailed(player, "mission_fail_1", "ordre_non_accompli");
    state = EmployerTeamDisciplineSystem::stateFor(player);
    assert(state.professionalTrust == 62);
    assert(state.warnings == 1);
    assert(!EmployerTeamDisciplineSystem::playerStartsInReserve(player));

    EmployerTeamDisciplineSystem::recordMissionFailed(player, "mission_fail_2", "ordre_non_accompli");
    state = EmployerTeamDisciplineSystem::stateFor(player);
    assert(state.professionalTrust == 45);
    assert(state.warnings == 2);
    assert(EmployerTeamDisciplineSystem::playerStartsInReserve(player));
    assert(EmployerTeamDisciplineSystem::reserveBattlesRemaining(player) >= 1);

    const EmployerTeamFormation formation = EmployerTeamRosterSystem::formationFor(player, true);
    assert(formation.playerInReserve);
    assert(formation.activeMembers.size() == 3);
    const auto order = EmployerTeamRosterSystem::openingTurnOrder(player, true);
    assert(!order.empty());
    assert(order.front() == "Capitaine Rhea");
    for (const std::string& actor : order) assert(actor != player.getName());

    assert(EmployerTeamDisciplineSystem::consumeReserveBattle(player));

    EmployerTeamOrderOutcome justified;
    justified.obeyed = false;
    justified.justifiedDisobedience = true;
    justified.preventedSeriousHarm = true;
    justified.reason = "sauvetage_du_soigneur";
    EmployerTeamDisciplineSystem::recordOrderOutcome(player, justified);
    state = EmployerTeamDisciplineSystem::stateFor(player);
    assert(state.professionalTrust == 48);

    EmployerTeamOrderOutcome bad;
    bad.obeyed = false;
    bad.reason = "ordre_ignore_sans_raison";
    EmployerTeamDisciplineSystem::recordOrderOutcome(player, bad);
    EmployerTeamDisciplineSystem::recordOrderOutcome(player, bad);
    state = EmployerTeamDisciplineSystem::stateFor(player);
    assert(state.unjustifiedDisobedience == 2);
    assert(state.warnings >= 3);

    // Une accumulation supplémentaire doit finir par rendre le licenciement disciplinaire crédible.
    EmployerTeamDisciplineSystem::recordMissionFailed(player, "mission_fail_3", "ordre_non_accompli");
    EmployerTeamDisciplineSystem::recordMissionFailed(player, "mission_fail_4", "ordre_non_accompli");
    assert(EmployerTeamDisciplineSystem::dismissalRecommended(player));
    const int retention = EmployerTeamDisciplineSystem::recommendedSalaryRetentionPercent(player);
    assert(retention >= 50 && retention <= 75);

    player.advanceWorldDays(7);
    const long long before = player.getInventory().getEconomyUnits();
    assert(TeamEmploymentSystem::dismissForDiscipline(player, "fautes_repetees", 75));
    assert(!TeamEmploymentSystem::hasActiveContract(player));
    // 100 dus, 75 % retenus -> 25 versés ; l'argent déjà payé n'est jamais repris.
    assert(player.getInventory().getEconomyUnits() == before + 25);

    // Les ordres employeur sont maintenant évalués depuis des faits observables,
    // sans transformer une action ambiguë en faute automatique.
    Player observed("ObservedOrderTester", ClassCatalog::createClassByName("Guerrier"));
    assert(TeamEmploymentSystem::startExternalContract(
        observed, "observed_order_contract", "Capitaine Rhea", "watch", "Veilleurs", "membre", 100, 7, -1));

    EmployerObservedOrderActionContext hold;
    hold.orderKind = EmployerTacticalOrderKind::HoldLine;
    hold.orderCode = "hold_line";
    hold.actionKind = "defense";
    hold.leaderPresent = true;
    hold.witnessCount = 2;
    auto assessment = EmployerTeamDisciplineSystem::assessObservedOrderAction(hold);
    assert(assessment.verdict == EmployerOrderObservationVerdict::Obeyed);

    EmployerObservedOrderActionContext focus = hold;
    focus.orderKind = EmployerTacticalOrderKind::FocusFire;
    focus.orderCode = "focus_fire";
    focus.actionKind = "basic_attack";
    focus.orderedTargetName = "Gobelin éclaireur";
    focus.orderedTargetId = "enemy_1";
    focus.orderedTargetStillActive = true;
    focus.actionTargetName = "Gobelin éclaireur";
    focus.actionTargetId = "enemy_1";
    assessment = EmployerTeamDisciplineSystem::assessObservedOrderAction(focus);
    assert(assessment.verdict == EmployerOrderObservationVerdict::Obeyed);

    EmployerObservedOrderActionContext wrongFocus = focus;
    wrongFocus.actionTargetName = "Gobelin archer";
    wrongFocus.actionTargetId = "enemy_2";
    assessment = EmployerTeamDisciplineSystem::assessObservedOrderAction(wrongFocus);
    assert(assessment.verdict == EmployerOrderObservationVerdict::UnjustifiedDisobedience);


    EmployerObservedOrderActionContext duplicateNameWrong = focus;
    duplicateNameWrong.actionTargetName = "Gobelin éclaireur";
    duplicateNameWrong.actionTargetId = "enemy_3";
    assessment = EmployerTeamDisciplineSystem::assessObservedOrderAction(duplicateNameWrong);
    assert(assessment.verdict == EmployerOrderObservationVerdict::UnjustifiedDisobedience);

    EmployerObservedOrderActionContext forcedFocus = wrongFocus;
    forcedFocus.targetForcedByCombatRules = true;
    assessment = EmployerTeamDisciplineSystem::assessObservedOrderAction(forcedFocus);
    assert(assessment.verdict == EmployerOrderObservationVerdict::JustifiedDisobedience);

    EmployerObservedOrderActionContext expiredFocus = focus;
    expiredFocus.orderedTargetStillActive = false;
    assessment = EmployerTeamDisciplineSystem::assessObservedOrderAction(expiredFocus);
    assert(assessment.verdict == EmployerOrderObservationVerdict::NotApplicable);

    EmployerObservedOrderActionContext unknownFocus = focus;
    unknownFocus.actionTargetName.clear();
    unknownFocus.actionTargetId.clear();
    assessment = EmployerTeamDisciplineSystem::assessObservedOrderAction(unknownFocus);
    assert(assessment.verdict == EmployerOrderObservationVerdict::Inconclusive);

    EmployerUnauthorizedSwitchContext urgentSwitch;
    urgentSwitch.actorName = "Mira";
    urgentSwitch.leaderPresent = true;
    urgentSwitch.witnessCount = 2;
    urgentSwitch.outgoingHpPercent = 9;
    urgentSwitch.incomingHpPercent = 86;
    urgentSwitch.fearPercent = 88;
    urgentSwitch.survivalUrgent = true;
    urgentSwitch.tacticalOpportunity = true;
    assessment = EmployerTeamDisciplineSystem::assessUnauthorizedIndividualSwitch(urgentSwitch);
    assert(assessment.verdict == EmployerOrderObservationVerdict::JustifiedDisobedience);
    assert(assessment.preventedSeriousHarm);

    EmployerUnauthorizedSwitchContext tacticalSwitch = urgentSwitch;
    tacticalSwitch.outgoingHpPercent = 38;
    tacticalSwitch.fearPercent = 35;
    tacticalSwitch.survivalUrgent = false;
    assessment = EmployerTeamDisciplineSystem::assessUnauthorizedIndividualSwitch(tacticalSwitch);
    assert(assessment.verdict == EmployerOrderObservationVerdict::JustifiedDisobedience);

    EmployerUnauthorizedSwitchContext badSwitch = tacticalSwitch;
    badSwitch.outgoingHpPercent = 82;
    badSwitch.incomingHpPercent = 88;
    badSwitch.tacticalOpportunity = false;
    assessment = EmployerTeamDisciplineSystem::assessUnauthorizedIndividualSwitch(badSwitch);
    assert(assessment.verdict == EmployerOrderObservationVerdict::UnjustifiedDisobedience);

    EmployerUnauthorizedSwitchContext unseenSwitch = urgentSwitch;
    unseenSwitch.leaderPresent = false;
    unseenSwitch.witnessCount = 0;
    assessment = EmployerTeamDisciplineSystem::assessUnauthorizedIndividualSwitch(unseenSwitch);
    assert(assessment.verdict == EmployerOrderObservationVerdict::Unobserved);

    EmployerObservedOrderActionContext unseen = hold;
    unseen.leaderPresent = false;
    unseen.witnessCount = 0;
    assessment = EmployerTeamDisciplineSystem::assessObservedOrderAction(unseen);
    assert(assessment.verdict == EmployerOrderObservationVerdict::Unobserved);

    EmployerOrderCombatReview obeyedReview;
    EmployerTeamDisciplineSystem::appendObservedOrderAction(
        observed, obeyedReview, hold, "discipline_test_obeyed", 1);
    auto reviewOutcome = EmployerTeamDisciplineSystem::finalizeOrderReview(
        observed, obeyedReview, "hold_line", "discipline_test_obeyed");
    assert(reviewOutcome.verdict == EmployerOrderObservationVerdict::Obeyed);
    assert(reviewOutcome.disciplineRecorded);
    assert(EmployerTeamDisciplineSystem::stateFor(observed).professionalTrust == 72);

    EmployerObservedOrderActionContext badEscape = hold;
    badEscape.actionKind = "escape";
    badEscape.playerHpPercent = 80;
    badEscape.escapeSucceeded = true;
    EmployerOrderCombatReview badEscapeReview;
    EmployerTeamDisciplineSystem::appendObservedOrderAction(
        observed, badEscapeReview, badEscape, "discipline_test_bad_escape", 1);
    reviewOutcome = EmployerTeamDisciplineSystem::finalizeOrderReview(
        observed, badEscapeReview, "hold_line", "discipline_test_bad_escape");
    assert(reviewOutcome.verdict == EmployerOrderObservationVerdict::UnjustifiedDisobedience);
    assert(EmployerTeamDisciplineSystem::stateFor(observed).professionalTrust == 61);

    EmployerObservedOrderActionContext survivalEscape = badEscape;
    survivalEscape.playerHpPercent = 12;
    survivalEscape.immediateSurvivalThreat = true;
    EmployerOrderCombatReview survivalReview;
    EmployerTeamDisciplineSystem::appendObservedOrderAction(
        observed, survivalReview, survivalEscape, "discipline_test_survival_escape", 1);
    reviewOutcome = EmployerTeamDisciplineSystem::finalizeOrderReview(
        observed, survivalReview, "hold_line", "discipline_test_survival_escape");
    assert(reviewOutcome.verdict == EmployerOrderObservationVerdict::JustifiedDisobedience);
    assert(EmployerTeamDisciplineSystem::stateFor(observed).professionalTrust == 64);

    EmployerOrderCombatReview unseenReview;
    EmployerTeamDisciplineSystem::appendObservedOrderAction(
        observed, unseenReview, unseen, "discipline_test_unseen", 1);
    reviewOutcome = EmployerTeamDisciplineSystem::finalizeOrderReview(
        observed, unseenReview, "hold_line", "discipline_test_unseen");
    assert(reviewOutcome.verdict == EmployerOrderObservationVerdict::Unobserved);
    assert(!reviewOutcome.disciplineRecorded);
    assert(EmployerTeamDisciplineSystem::stateFor(observed).professionalTrust == 64);

    return 0;
}
