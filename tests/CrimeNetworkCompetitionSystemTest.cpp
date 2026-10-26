#include "entity/Player.hpp"
#include "world/crime/CrimeContactDialogueSystem.hpp"
#include "world/crime/CrimeContactRelationshipSystem.hpp"
#include "world/crime/CrimeNetworkCompetitionSystem.hpp"

#include <cassert>
#include <string>

namespace
{
    bool contains(const std::vector<std::string>& lines, const std::string& needle)
    {
        for (const std::string& line : lines)
            if (line.find(needle) != std::string::npos) return true;
        return false;
    }
}

int main()
{
    Player player;
    player.setCurrentCityId("valebrume");
    player.getOrCreateCrimeContactState("valebrume_receleur_brocanteur", "Serrin le brocanteur");
    player.getOrCreateCrimeContactState("valebrume_complice_courriere", "Vela la courrière");
    player.getOrCreateCrimeContactState("rocheveille_receleur_peseur", "Dorn le peseur");
    PlayerCrimeContactState* serrin = nullptr;
    PlayerCrimeContactState* vela = nullptr;
    for (PlayerCrimeContactState& state : player.getCrimeContactStates())
    {
        if (state.contactId == "valebrume_receleur_brocanteur") serrin = &state;
        if (state.contactId == "valebrume_complice_courriere") vela = &state;
    }
    assert(serrin != nullptr && vela != nullptr);
    serrin->homeCityId = "valebrume";
    serrin->networkId = "marche_gris";
    vela->homeCityId = "valebrume";
    vela->networkId = "courriers_nocturnes";

    const CrimeNetworkCompetitionAssessment before = CrimeNetworkCompetitionSystem::assessKnownCity(player, "valebrume");
    assert(before.observable);
    assert(before.knownNetworks == 2);

    const int rivalryBefore = vela->rivalry;
    const CrimeNetworkDisruptionResult disruption = CrimeNetworkCompetitionSystem::applyKnownDisruption(
        player, serrin->contactId, 5, "Arrestation durable de test");
    assert(disruption.applied);
    assert(disruption.rivalContactsAffected == 1);
    assert(vela->rivalry > rivalryBefore);
    assert(player.getCanonicalJournalCategoryTotal("crime_network_competition_events") == 1);

    // An unavailable contact can still point the player toward another contact already known in the same network.
    serrin->arrested = true;
    const std::string replacement = CrimeContactRelationshipSystem::chooseKnownReplacementContact(player, serrin->contactId);
    assert(replacement == "rocheveille_receleur_peseur");
    const CrimeContactDialogueSnapshot dialogue = CrimeContactDialogueSystem::build(player, serrin->contactId);
    assert(contains(dialogue.lines, "relais déjà connu"));

    // Unknown rivals are never materialized by the competition system.
    assert(player.findCrimeContactState("port_lanterne_complice_quais") == nullptr);
    return 0;
}
