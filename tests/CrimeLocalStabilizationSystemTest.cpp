#include "entity/Player.hpp"
#include "quest/Quest.hpp"
#include "world/crime/CrimeLocalImpactSystem.hpp"
#include "world/crime/CrimeLocalStabilizationSystem.hpp"

#include <cassert>
#include <iostream>

int main()
{
    Player player;
    player.setCurrentCityId("valebrume");

    PlayerCrimeContactState& serrin = player.getOrCreateCrimeContactState(
        "valebrume_receleur_brocanteur", "Serrin le brocanteur");
    serrin.homeCityId = "valebrume";
    serrin.networkId = "marche_gris";
    serrin.pressure = 80;
    serrin.rivalry = 45;
    serrin.lastInteractionDay = 0;

    PlayerCrimeContactState& vela = player.getOrCreateCrimeContactState(
        "valebrume_complice_courriere", "Vela la courrière");
    vela.homeCityId = "valebrume";
    vela.networkId = "courriers_nocturnes";
    vela.pressure = 70;
    vela.rivalry = 55;
    vela.lastInteractionDay = 0;

    const CrimeLocalImpactRegistration disruption = CrimeLocalImpactSystem::registerDisruption(
        player,
        "valebrume",
        "valebrume_receleur_brocanteur",
        5,
        "Une grosse perturbation oblige plusieurs relais connus à changer leurs habitudes");
    assert(disruption.recorded);

    const CrimeLocalImpactSnapshot immediate = CrimeLocalImpactSystem::assess(player, "valebrume");
    assert(immediate.observable);
    assert(immediate.pressureScore >= 45);

    player.advanceWorldDays(3);
    const CrimeLocalImpactSnapshot afterQuietDays = CrimeLocalImpactSystem::assess(player, "valebrume");
    assert(afterQuietDays.quietRecoveryDays >= 2);
    assert(afterQuietDays.pressureScore < immediate.pressureScore);

    Quest followUp;
    followUp.id = "crime_local_route_watch_valebrume_day0";
    followUp.title = "Relais sous surveillance";
    followUp.origin = "Conséquence locale";
    followUp.location = "valebrume / guilde locale";
    followUp.completed = true;
    followUp.turnedIn = true;

    const int pressureBefore = player.findCrimeContactState("valebrume_receleur_brocanteur")->pressure;
    const CrimeLocalStabilizationResult stabilization = CrimeLocalStabilizationSystem::applyQuestTurnIn(player, followUp);
    assert(stabilization.applied);
    assert(stabilization.cityId == "valebrume");
    assert(stabilization.strength == 2);
    assert(stabilization.contactsCalmed == 2);
    assert(player.findCrimeContactState("valebrume_receleur_brocanteur")->pressure < pressureBefore);

    const CrimeLocalImpactSnapshot afterFollowUp = CrimeLocalImpactSystem::assess(player, "valebrume");
    assert(afterFollowUp.recentStabilizationWeight > 0);
    assert(afterFollowUp.pressureScore < afterQuietDays.pressureScore);

    const CrimeLocalStabilizationResult duplicate = CrimeLocalStabilizationSystem::applyQuestTurnIn(player, followUp);
    assert(!duplicate.applied);

    std::cout << "CrimeLocalStabilizationSystemTest PASS\n";
    return 0;
}
