#include "entity/Player.hpp"
#include "world/crime/CrimeLocalImpactSystem.hpp"
#include "world/crime/VaultTheftNetworkCatalog.hpp"
#include "world/rumor/WorldRumorSystem.hpp"

#include <cassert>
#include <iostream>

int main()
{
    Player player;
    player.setCurrentCityId("valebrume");

    const std::string serrinId = "valebrume_receleur_brocanteur";
    PlayerCrimeContactState& serrin = player.getOrCreateCrimeContactState(serrinId, "Serrin le brocanteur");
    serrin.homeCityId = "valebrume";
    serrin.networkId = "marche_gris";
    serrin.pressure = 70;
    serrin.rivalry = 45;
    serrin.arrested = true;

    // Do not retain a reference into the contact vector across another insertion.
    // getOrCreateCrimeContactState may grow the vector and invalidate earlier references.
    PlayerCrimeContactState& vela = player.getOrCreateCrimeContactState("valebrume_complice_courriere", "Vela la courrière");
    vela.homeCityId = "valebrume";
    vela.networkId = "courriers_nocturnes";
    vela.pressure = 55;
    vela.rivalry = 60;

    const CrimeLocalImpactRegistration registration = CrimeLocalImpactSystem::registerDisruption(
        player, "valebrume", serrinId, 5, "Une arrestation durable recompose les relais connus");
    assert(registration.recorded);
    assert(registration.rumorRecorded);
    assert(registration.opportunityQueued);

    const CrimeLocalImpactSnapshot impact = CrimeLocalImpactSystem::assess(player, "valebrume");
    assert(impact.observable);
    assert(impact.pressureScore >= 45);
    assert(impact.arrestedContacts == 1);
    assert(impact.knownNetworks >= 2);
    assert(impact.serviceQualityDelta < 0);
    assert(impact.tradeSellModifierPercent > 0);

    CityEconomicPulse pulse;
    CrimeLocalImpactSystem::applyEconomicPressure(player, "valebrume", pulse);
    assert(pulse.serviceQualityDelta < 0);
    assert(pulse.generalSellModifierPercent > 0);

    const WorldRumorStatus rumor = WorldRumorSystem::statusFor(player, "crime_local_shift:valebrume");
    assert(rumor.known);
    assert(!rumor.confirmed);

    std::cout << "CrimeLocalImpactSystemTest PASS\n";
    return 0;
}
