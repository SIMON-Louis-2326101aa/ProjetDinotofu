#include "world/crime/CrimeNetworkSystem.hpp"

#include "entity/Player.hpp"
#include "world/crime/VaultTheftNetworkCatalog.hpp"

#include <algorithm>

namespace
{
    const PlayerCrimeContactState* findState(const Player& player, const std::string& id)
    {
        return player.findCrimeContactState(id);
    }
}

CrimeNetworkRelayAssessment CrimeNetworkSystem::assessKnownRelay(
    const Player& player,
    const std::string& sourceContactId,
    const std::string& targetContactId)
{
    CrimeNetworkRelayAssessment result;
    const VaultTheftNetworkContact* source = VaultTheftNetworkCatalog::findById(sourceContactId);
    const VaultTheftNetworkContact* target = VaultTheftNetworkCatalog::findById(targetContactId);
    const PlayerCrimeContactState* sourceState = findState(player, sourceContactId);
    const PlayerCrimeContactState* targetState = findState(player, targetContactId);

    result.known = source != nullptr && target != nullptr && sourceState != nullptr && targetState != nullptr;
    if (!result.known)
    {
        result.reason = "un des contacts ou son lien n'est pas réellement connu";
        return result;
    }

    result.sameNetwork = !source->networkId.empty() && source->networkId == target->networkId;
    if (!result.sameNetwork)
    {
        result.reason = "aucun réseau commun crédible n'est connu";
        return result;
    }

    const int day = player.getWorldDaysElapsed();
    const bool sourceUnavailable = sourceState->arrested
        || sourceState->detainedUntilDay > day
        || sourceState->fledUntilDay > day
        || sourceState->ruptureUntilDay > day;
    const bool targetUnavailable = targetState->arrested
        || targetState->detainedUntilDay > day
        || targetState->fledUntilDay > day
        || targetState->ruptureUntilDay > day;
    if (sourceUnavailable || targetUnavailable)
    {
        result.reason = "le relais est indisponible (fuite, détention, arrestation ou rupture temporaire)";
        return result;
    }

    result.available = true;
    int readiness = 68;
    readiness += std::min(12, std::max(0, targetState->networkWarnings) * 2);
    readiness += std::max(0, -targetState->cooperation) / 5;
    readiness -= std::max(0, targetState->cooperation) / 3;
    readiness -= std::max(0, targetState->pressure - 35) / 3;
    readiness += (std::clamp(targetState->reliability, 0, 100) - 50) / 4;
    readiness -= std::clamp(targetState->rivalry, 0, 100) / 5;
    readiness += std::max(0, targetState->favorBalance) / 8;
    readiness -= std::max(0, -targetState->favorBalance) / 12;
    if (targetState->lastNetworkWarningDay == day) readiness -= 28;
    if (sourceState->lastNetworkWarningDay == day) readiness -= 10;
    result.relayReadiness = std::clamp(readiness, 0, 100);
    result.relayLikely = result.relayReadiness >= 50;
    result.reason = result.relayLikely
        ? "le réseau connu possède encore un relais crédible entre ces deux contacts"
        : "le relais connu est trop perturbé, compromis ou déjà saturé aujourd'hui";
    return result;
}

CrimeNetworkWarningResult CrimeNetworkSystem::propagateKnownWarning(
    Player& player,
    const std::string& sourceContactId,
    int localPressureGain,
    int intercityPressureGain,
    int cooperationLoss,
    const std::string& reason)
{
    CrimeNetworkWarningResult result;
    const VaultTheftNetworkContact* source = VaultTheftNetworkCatalog::findById(sourceContactId);
    if (source == nullptr || source->networkId.empty()) return result;

    const int day = player.getWorldDaysElapsed();
    const PlayerCrimeContactState* sourceState = player.findCrimeContactState(sourceContactId);
    if (sourceState != nullptr && (sourceState->arrested || sourceState->detainedUntilDay > day || sourceState->fledUntilDay > day))
    {
        return result;
    }

    for (PlayerCrimeContactState& state : player.getCrimeContactStates())
    {
        if (state.contactId.empty() || state.contactId == sourceContactId || state.arrested) continue;
        const VaultTheftNetworkContact* candidate = VaultTheftNetworkCatalog::findById(state.contactId);
        if (candidate == nullptr || candidate->networkId != source->networkId) continue;

        const bool local = candidate->cityId == source->cityId;
        const int gain = std::max(0, local ? localPressureGain : intercityPressureGain);
        if ((local && localPressureGain <= 0) || (!local && intercityPressureGain <= 0)) continue;

        if (!local)
        {
            const CrimeNetworkRelayAssessment relay = assessKnownRelay(player, sourceContactId, state.contactId);
            if (!relay.relayLikely)
            {
                ++result.intercityRelaysBlocked;
                result.blockedContactIds.push_back(state.contactId);
                continue;
            }
        }

        state.pressure = std::clamp(state.pressure + gain, 0, 100);
        state.cooperation = std::clamp(state.cooperation - std::max(0, cooperationLoss), -100, 100);
        state.reliability = std::clamp(state.reliability - (local ? 1 : 2), 0, 100);
        state.rivalry = std::clamp(state.rivalry + (local ? 1 : 2), 0, 100);
        state.networkId = source->networkId;
        state.homeCityId = candidate->cityId;
        ++state.networkWarnings;
        state.lastNetworkWarningDay = day;
        state.lastWarningSourceId = sourceContactId;

        ++result.knownContactsAffected;
        if (local) ++result.localContactsAffected;
        else ++result.intercityContactsAffected;
        result.affectedContactIds.push_back(state.contactId);
    }

    if (result.knownContactsAffected > 0 || result.intercityRelaysBlocked > 0)
    {
        player.recordCanonicalEvent(
            "crime_network_warnings",
            source->networkId + ":day_" + std::to_string(day),
            (reason.empty() ? std::string("Avertissement circulant dans un réseau criminel connu") : reason)
                + " | source=" + sourceContactId
                + " | local=" + std::to_string(result.localContactsAffected)
                + " | interville=" + std::to_string(result.intercityContactsAffected)
                + " | relais_bloques=" + std::to_string(result.intercityRelaysBlocked),
            std::max(1, result.knownContactsAffected));
    }
    return result;
}
