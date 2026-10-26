#include <algorithm>
#include <cassert>

#include "class_system/ClassCatalog.hpp"
#include "entity/Player.hpp"
#include "world/crime/CrimeNetworkSystem.hpp"
#include "world/crime/VaultTheftNetworkCatalog.hpp"

int main()
{
    Player player("Réseau", ClassCatalog::createClassByName("Guerrier"));

    player.getOrCreateCrimeContactState("valebrume_receleur_brocanteur", "Serrin le brocanteur");
    player.getOrCreateCrimeContactState("rocheveille_receleur_peseur", "Dorn le peseur");
    player.getOrCreateCrimeContactState("port_lanterne_complice_quais", "Seline des amarres");
    player.getCrimeContactStates()[1].cooperation = 10;
    player.getCrimeContactStates()[2].pressure = 4;

    const auto related = VaultTheftNetworkCatalog::relatedContacts("valebrume_receleur_brocanteur", false);
    assert(!related.empty());
    assert(std::any_of(related.begin(), related.end(), [](const VaultTheftNetworkContact* c) {
        return c != nullptr && c->id == "rocheveille_receleur_peseur";
    }));


    const CrimeNetworkRelayAssessment relay = CrimeNetworkSystem::assessKnownRelay(
        player,
        "valebrume_receleur_brocanteur",
        "rocheveille_receleur_peseur");
    assert(relay.known);
    assert(relay.sameNetwork);
    assert(relay.available);
    assert(relay.relayLikely);

    const CrimeNetworkWarningResult result = CrimeNetworkSystem::propagateKnownWarning(
        player,
        "valebrume_receleur_brocanteur",
        0,
        4,
        2,
        "Alerte de test");

    assert(result.knownContactsAffected == 1);
    assert(result.intercityContactsAffected == 1);
    const PlayerCrimeContactState* linkedAfter = player.findCrimeContactState("rocheveille_receleur_peseur");
    assert(linkedAfter != nullptr);
    assert(linkedAfter->pressure == 4);
    assert(linkedAfter->cooperation == 8);
    assert(linkedAfter->networkWarnings == 1);
    assert(linkedAfter->lastNetworkWarningDay == player.getWorldDaysElapsed());
    assert(linkedAfter->lastWarningSourceId == "valebrume_receleur_brocanteur");
    assert(linkedAfter->networkId == "marche_gris");
    assert(linkedAfter->homeCityId == "rocheveille");

    const PlayerCrimeContactState* unrelatedAfter = player.findCrimeContactState("port_lanterne_complice_quais");
    assert(unrelatedAfter != nullptr);
    assert(unrelatedAfter->pressure == 4);
    assert(unrelatedAfter->networkWarnings == 0);


    // A second inter-city warning on the same day can be blocked instead of behaving like a perfect telephone.
    const CrimeNetworkWarningResult sameDay = CrimeNetworkSystem::propagateKnownWarning(
        player,
        "valebrume_receleur_brocanteur",
        0,
        4,
        2,
        "Deuxième alerte de test");
    assert(sameDay.intercityContactsAffected == 0);
    assert(sameDay.intercityRelaysBlocked >= 1);

    // Physical unavailability also cuts the known relay.
    player.getCrimeContactStates()[1].detainedUntilDay = player.getWorldDaysElapsed() + 2;
    const CrimeNetworkRelayAssessment detained = CrimeNetworkSystem::assessKnownRelay(
        player,
        "valebrume_receleur_brocanteur",
        "rocheveille_receleur_peseur");
    assert(!detained.available);
    assert(!detained.relayLikely);

    // Unknown contacts are never revealed just because the catalog says the network exists.
    assert(player.findCrimeContactState("lys_givre_receleur_tisseur") == nullptr);
    return 0;
}
