#include "world/crime/CrimeNetworkCompetitionSystem.hpp"

#include "entity/Player.hpp"
#include "world/crime/VaultTheftNetworkCatalog.hpp"

#include <algorithm>
#include <set>

namespace
{
    int effectivePressure(const PlayerCrimeContactState& state, int day)
    {
        if (state.lastInteractionDay < 0) return std::clamp(state.pressure, 0, 100);
        const int quietDays = std::max(0, day - state.lastInteractionDay - 1);
        return std::clamp(state.pressure - quietDays * 3, 0, 100);
    }
}

CrimeNetworkCompetitionAssessment CrimeNetworkCompetitionSystem::assessKnownCity(
    const Player& player,
    const std::string& cityId)
{
    CrimeNetworkCompetitionAssessment result;
    if (cityId.empty()) return result;

    std::set<std::string> networks;
    int rivalrySum = 0;
    int pressureSum = 0;
    for (const PlayerCrimeContactState& state : player.getCrimeContactStates())
    {
        const VaultTheftNetworkContact* definition = VaultTheftNetworkCatalog::findById(state.contactId);
        const std::string contactCity = !state.homeCityId.empty()
            ? state.homeCityId
            : (definition != nullptr ? definition->cityId : std::string{});
        const std::string networkId = !state.networkId.empty()
            ? state.networkId
            : (definition != nullptr ? definition->networkId : std::string{});
        if (contactCity != cityId || networkId.empty()) continue;

        ++result.knownContacts;
        networks.insert(networkId);
        rivalrySum += std::clamp(state.rivalry, 0, 100);
        pressureSum += effectivePressure(state, player.getWorldDaysElapsed());
    }

    result.networkIds.assign(networks.begin(), networks.end());
    result.knownNetworks = static_cast<int>(result.networkIds.size());
    result.observable = result.knownNetworks >= 2;
    if (!result.observable) return result;

    const int divisor = std::max(1, result.knownContacts);
    result.tension = std::clamp(20 + rivalrySum / divisor / 2 + pressureSum / divisor / 3, 0, 100);
    result.lines.push_back(std::to_string(result.knownNetworks)
        + " réseaux différents sont déjà connus dans cette ville : leur coexistence n'est pas neutre.");
    if (result.tension >= 70)
        result.lines.push_back("La tension connue est forte : les contacts surveillent davantage les mouvements des réseaux concurrents.");
    else if (result.tension >= 40)
        result.lines.push_back("La concurrence est visible mais reste encore contenue.");
    else
        result.lines.push_back("La concurrence existe surtout comme méfiance et opportunisme, sans conflit ouvert pour le moment.");
    return result;
}

CrimeNetworkDisruptionResult CrimeNetworkCompetitionSystem::applyKnownDisruption(
    Player& player,
    const std::string& sourceContactId,
    int severity,
    const std::string& reason)
{
    CrimeNetworkDisruptionResult result;
    const VaultTheftNetworkContact* source = VaultTheftNetworkCatalog::findById(sourceContactId);
    if (source == nullptr || source->networkId.empty() || source->cityId.empty()) return result;

    const int safeSeverity = std::clamp(severity, 1, 5);
    for (PlayerCrimeContactState& state : player.getCrimeContactStates())
    {
        if (state.contactId == sourceContactId || state.arrested) continue;
        const VaultTheftNetworkContact* candidate = VaultTheftNetworkCatalog::findById(state.contactId);
        if (candidate == nullptr || candidate->cityId != source->cityId || candidate->networkId.empty()) continue;
        if (candidate->networkId == source->networkId) continue;

        state.homeCityId = candidate->cityId;
        state.networkId = candidate->networkId;
        state.rivalry = std::clamp(state.rivalry + safeSeverity * 5, 0, 100);
        state.pressure = std::clamp(state.pressure + safeSeverity * 2, 0, 100);
        if (safeSeverity >= 4 && state.cooperation >= 0)
        {
            state.cooperation = std::clamp(state.cooperation + 3, -100, 100);
            ++result.opportunityContacts;
        }
        state.lastInteractionDay = std::max(state.lastInteractionDay, player.getWorldDaysElapsed());
        ++result.rivalContactsAffected;
        result.affectedContactIds.push_back(state.contactId);
    }

    if (result.rivalContactsAffected <= 0) return result;
    result.applied = true;
    player.recordCanonicalEvent(
        "crime_network_competition_events",
        source->cityId + ":" + source->networkId,
        (reason.empty() ? std::string("Perturbation visible d'un réseau criminel connu") : reason)
            + " | rivaux_connus=" + std::to_string(result.rivalContactsAffected),
        1);
    player.recordHistoricalEvent(
        "crime_network_competition",
        source->cityId + ":" + source->networkId,
        "Une perturbation du réseau " + source->networkId + " est remarquée par "
            + std::to_string(result.rivalContactsAffected) + " contact(s) concurrent(s) déjà connus.",
        true);

    result.lines.push_back(std::to_string(result.rivalContactsAffected)
        + " contact(s) concurrent(s) déjà connus remarquent la faiblesse momentanée du réseau touché.");
    if (result.opportunityContacts > 0)
        result.lines.push_back("Certains rivaux deviennent momentanément plus ouverts : ils voient une occasion de gagner du terrain, pas une amitié soudaine.");
    return result;
}
