#include "combat/ally/RecruitedAllyAftermathSystem.hpp"
#include "core/Random.hpp"
#include "entity/Player.hpp"

#include <algorithm>
#include <cassert>
#include <string>
#include <vector>

namespace
{
    std::string recruitLabel(const std::string& name, const std::string& job, const std::string& trait, const std::string& motivation)
    {
        return name + " | race=Humain | profil=" + job + " | trait=" + trait
            + " | motivation=" + motivation
            + " | niveau_recrutement=20 | rang_origine_index=2 | part_base=18 | remise_fidelite=2";
    }

    bool rosterContains(const std::vector<RecruitedAllyCombatSupport>& allies, const std::string& name)
    {
        return std::any_of(allies.begin(), allies.end(), [&](const auto& ally) { return ally.name == name; });
    }
}

int main()
{
    Player player;
    player.recordCanonicalEvent("pnj_recrutables_retenus", "Aline", recruitLabel("Aline", "Soigneur", "calme loyal", "protéger le groupe"), 1);
    player.recordCanonicalEvent("recrues_equipees_toggle", "Aline", "Aline équipé", 1);

    auto active = RecruitedAllyCombatRoster::collectActive(player);
    assert(active.size() == 1);
    const auto& aline = active.front();
    assert(RecruitedAllyAftermathSystem::reportChancePercent(player, aline) >= 80);
    assert(RecruitedAllyAftermathSystem::theftChancePercent(player, aline) == 0);

    player.recordCanonicalEvent("participation_recrues", "Aline", "combats vécus", 10);
    player.recordCanonicalEvent("sauvetages_relation_recrues", "Aline", "sauvetage du chef", 1);
    active = RecruitedAllyCombatRoster::collectActive(player);
    assert(RecruitedAllyAftermathSystem::reportChancePercent(player, active.front()) >= 85);

    const long long moneyBefore = player.getInventory().getEconomyUnits();
    RecruitedAllyAftermathDecision honest;
    honest.memberName = "Aline";
    const int delivered = RecruitedAllyAftermathSystem::deliverPlayerEconomyShare(player, 37, honest, "test_honest");
    assert(delivered == 37);
    assert(player.getInventory().getEconomyUnits() == moneyBefore + 37);

    player.recordCanonicalEvent("pnj_recrutables_retenus", "Rook", recruitLabel("Rook", "Roublard", "rancunier opportuniste", "profit rapide"), 1);
    player.recordCanonicalEvent("recrues_equipees_toggle", "Rook", "Rook équipé", 1);
    player.recordCanonicalEvent("salaires_recrues_impayes", "Rook", "salaire impayé", 2);
    active = RecruitedAllyCombatRoster::collectActive(player);
    auto it = std::find_if(active.begin(), active.end(), [](const auto& ally) { return ally.name == "Rook"; });
    assert(it != active.end());
    assert(RecruitedAllyAftermathSystem::theftChancePercent(player, *it) > 0);

    RecruitedAllyAftermathDecision betrayal;
    betrayal.memberName = "Rook";
    betrayal.stealsPlayerShare = true;
    const long long beforeBetrayal = player.getInventory().getEconomyUnits();
    assert(RecruitedAllyAftermathSystem::deliverPlayerEconomyShare(player, 50, betrayal, "test_betrayal") == 0);
    assert(player.getInventory().getEconomyUnits() == beforeBetrayal);
    active = RecruitedAllyCombatRoster::collectActive(player);
    const auto reserve = RecruitedAllyCombatRoster::collectReserve(player);
    assert(!rosterContains(active, "Rook"));
    assert(!rosterContains(reserve, "Rook"));

    Random random(42u);
    const RecruitedAllyAftermathDecision decision = RecruitedAllyAftermathSystem::decide(player, active, random, "test_report");
    assert(decision.memberName == "Aline");
    assert(decision.reportChancePercent >= 80);
    assert(decision.theftChancePercent == 0);

    // Un reporter entré tard ne reçoit pas magiquement les rounds précédents.
    RecruitedAllyCombatSupport lateReporter = active.front();
    lateReporter.name = "Late";
    lateReporter.postKoFirstObservedRound = 3;
    lateReporter.postKoLastObservedRound = 5;
    lateReporter.postKoObservedDefeats = 2;
    lateReporter.postKoObservedFinalOutcome = true;
    lateReporter.postKoAvailableForDebrief = true;

    RecruitedAllyCombatSupport earlyWitness = active.front();
    earlyWitness.name = "Witness";
    earlyWitness.postKoFirstObservedRound = 1;
    earlyWitness.postKoLastObservedRound = 5;
    earlyWitness.postKoObservedDefeats = 4;
    earlyWitness.postKoObservedFinalOutcome = true;
    earlyWitness.postKoAvailableForDebrief = true;

    RecruitedAllyAftermathDecision lateDecision;
    lateDecision.memberName = "Late";
    const auto sourced = RecruitedAllyAftermathSystem::buildKnowledgeReport(
        lateDecision, {lateReporter, earlyWitness}, 5, true, 4, 0);
    assert(sourced.directFirstRound == 3);
    assert(sourced.directLastRound == 5);
    assert(sourced.directlyObservedDefeats == 2);
    assert(sourced.usesSecondHandSource);
    assert(sourced.secondHandSourceName == "Witness");
    assert(!sourced.hasUnknownGap);

    earlyWitness.postKoAvailableForDebrief = false;
    const auto unsourced = RecruitedAllyAftermathSystem::buildKnowledgeReport(
        lateDecision, {lateReporter, earlyWitness}, 5, true, 4, 0);
    assert(!unsourced.usesSecondHandSource);
    assert(unsourced.hasUnknownGap);

    RecruitedAllyCombatSupport fullWitness = lateReporter;
    fullWitness.name = "Full";
    fullWitness.postKoFirstObservedRound = 1;
    fullWitness.postKoLastObservedRound = 5;
    fullWitness.postKoObservedDefeats = 4;
    RecruitedAllyAftermathDecision fullDecision;
    fullDecision.memberName = "Full";
    const auto complete = RecruitedAllyAftermathSystem::buildKnowledgeReport(
        fullDecision, {fullWitness}, 5, true, 4, 0);
    assert(complete.directFirstRound == 1);
    assert(complete.knowsFinalOutcomeDirectly);
    assert(!complete.hasUnknownGap);

    // Un membre resté entièrement en file d'attente peut quand même être un témoin direct si la
    // réserve est physiquement proche et visible (gradins/bord d'arène). Cela ne lui donne aucun tour.
    RecruitedAllyCombatSupport reserveSpectator = active.front();
    reserveSpectator.name = "Spectator";
    reserveSpectator.turnsTaken = 0;
    reserveSpectator.postKoFirstObservedRound = 1;
    reserveSpectator.postKoLastObservedRound = 5;
    reserveSpectator.postKoObservedDefeats = 4;
    reserveSpectator.postKoObservedRoundsFromReserve = 5;
    reserveSpectator.postKoObservedFromReserve = true;
    reserveSpectator.postKoObservedFinalOutcome = true;
    reserveSpectator.postKoAvailableForDebrief = true;
    RecruitedAllyAftermathDecision spectatorDecision;
    spectatorDecision.memberName = "Spectator";
    const auto spectatorReport = RecruitedAllyAftermathSystem::buildKnowledgeReport(
        spectatorDecision, {reserveSpectator}, 5, true, 4, 0);
    assert(spectatorReport.reporterNeverEnteredCombat);
    assert(spectatorReport.observedFromReserve);
    assert(spectatorReport.directReserveObservationRounds == 5);
    assert(spectatorReport.directFirstRound == 1);
    assert(spectatorReport.directLastRound == 5);
    assert(spectatorReport.knowsFinalOutcomeDirectly);
    assert(!spectatorReport.hasUnknownGap);

    // Le loot post-KO reste physique : transporter le joueur inconscient consomme une partie
    // de la capacité du groupe, et seuls les survivants encore capables comptent.
    RecruitedAllyCombatSupport carrierA = active.front();
    carrierA.name = "CarrierA";
    carrierA.race = "Humain";
    carrierA.job = "Soigneur";
    carrierA.trait = "calme";
    carrierA.maxHp = 100;
    carrierA.currentHp = 100;
    RecruitedAllyCombatSupport carrierB = carrierA;
    carrierB.name = "CarrierB";
    RecruitedAllyCombatSupport deadCarrier = carrierA;
    deadCarrier.name = "DeadCarrier";
    deadCarrier.currentHp = 0;

    const auto oneCarrier = RecruitedAllyAftermathSystem::buildLootTransportPlan(
        {carrierA}, 5, true);
    assert(oneCarrier.capableSurvivors == 1);
    assert(oneCarrier.playerTransportConsumesCapacity);
    assert(oneCarrier.totalCarryUnits == 3);
    assert(oneCarrier.playerTransportUnitCost == 2);
    assert(oneCarrier.availableLootCarryUnits == 1);
    assert(oneCarrier.portableBundleCapacity == 1);
    assert(oneCarrier.recoverableEnemyBundles == 1);
    assert(oneCarrier.abandonedEnemyBundles == 4);
    assert(oneCarrier.heavyItemCapacity == 0);

    const auto twoCarriers = RecruitedAllyAftermathSystem::buildLootTransportPlan(
        {carrierA, carrierB, deadCarrier}, 5, true);
    assert(twoCarriers.capableSurvivors == 2);
    assert(twoCarriers.totalCarryUnits == 6);
    assert(twoCarriers.playerTransportUnitCost == 3);
    assert(twoCarriers.availableLootCarryUnits == 3);
    assert(twoCarriers.portableBundleCapacity == 3);
    assert(twoCarriers.recoverableEnemyBundles == 3);
    assert(twoCarriers.abandonedEnemyBundles == 2);
    assert(twoCarriers.heavyItemCapacity == 1);

    const auto twoFreeCarriers = RecruitedAllyAftermathSystem::buildLootTransportPlan(
        {carrierA, carrierB}, 5, false);
    assert(!twoFreeCarriers.playerTransportConsumesCapacity);
    assert(twoFreeCarriers.availableLootCarryUnits == 6);
    assert(twoFreeCarriers.portableBundleCapacity == 6);
    assert(twoFreeCarriers.recoverableEnemyBundles == 5);
    assert(twoFreeCarriers.abandonedEnemyBundles == 0);
    assert(twoFreeCarriers.heavyItemCapacity == 3);

    RecruitedAllyCombatSupport strongCarrier = carrierA;
    strongCarrier.name = "Strong";
    strongCarrier.race = "Orc";
    strongCarrier.job = "Guerrier tank";
    assert(RecruitedAllyAftermathSystem::carryingCapacityUnits(strongCarrier) >= 4);

    RecruitedAllyCombatSupport injuredSmallCarrier = carrierA;
    injuredSmallCarrier.name = "SmallInjured";
    injuredSmallCarrier.race = "Fée";
    injuredSmallCarrier.job = "Mage";
    injuredSmallCarrier.currentHp = 15;
    assert(RecruitedAllyAftermathSystem::carryingCapacityUnits(injuredSmallCarrier) == 1);
    assert(RecruitedAllyAftermathSystem::carryingCapacityUnits(strongCarrier)
        > RecruitedAllyAftermathSystem::carryingCapacityUnits(injuredSmallCarrier));

    // V3.51.59: when exact objects compete for limited post-KO capacity, protected
    // quest/personal/unique property must beat ordinary loot even when ordinary loot is lighter.
    const auto recoverySelection = RecruitedAllyAftermathSystem::selectPostKoRecoveryCandidates(
        {
            {"ordinary_light", 1, 0, false},
            {"personal_weapon", 2, 95, true},
            {"quest_object", 1, 100, true},
            {"unique_relic", 1, 90, true}
        },
        3);
    assert(recoverySelection.usedCarryUnits == 3);
    assert(recoverySelection.remainingCarryUnits == 0);
    assert(recoverySelection.recoveredIds.size() == 2);
    assert(recoverySelection.recoveredIds[0] == "quest_object");
    assert(recoverySelection.recoveredIds[1] == "personal_weapon");
    assert(std::find(recoverySelection.deferredIds.begin(), recoverySelection.deferredIds.end(), "ordinary_light") != recoverySelection.deferredIds.end());
    assert(std::find(recoverySelection.deferredIds.begin(), recoverySelection.deferredIds.end(), "unique_relic") != recoverySelection.deferredIds.end());


    return 0;
}
