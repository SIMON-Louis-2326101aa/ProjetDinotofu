#include "world/crime/CrimeContactRelationshipSystem.hpp"

#include "entity/Player.hpp"
#include "world/crime/CrimeNetworkSystem.hpp"
#include "world/crime/VaultTheftNetworkCatalog.hpp"

#include <algorithm>

namespace
{
    std::string reliabilityLabel(int value)
    {
        if (value >= 80) return "très fiable";
        if (value >= 60) return "plutôt fiable";
        if (value >= 40) return "incertain";
        if (value >= 20) return "peu fiable";
        return "presque inutilisable";
    }

    std::string debtLabel(int balance)
    {
        if (balance >= 35) return "le contact te doit une faveur importante";
        if (balance >= 10) return "le contact te doit encore quelque chose";
        if (balance <= -35) return "tu lui dois une faveur importante";
        if (balance <= -10) return "tu as encore une dette envers ce contact";
        return "aucune dette notable entre vous";
    }

    std::string relationshipLabel(const PlayerCrimeContactState& state, int day)
    {
        if (state.arrested) return "arrêté durablement";
        if (state.detainedUntilDay > day) return "en détention provisoire";
        if (state.fledUntilDay > day) return "en fuite";
        if (state.ruptureUntilDay > day) return "rupture temporaire";
        if (state.rivalry >= 70) return "rivalité ouverte";
        if (state.rivalry >= 35) return "relation très tendue";
        if (state.cooperation >= 35) return "coopératif";
        if (state.cooperation <= -35) return "hostile";
        return "relation prudente";
    }

    PlayerCrimeContactState* findMutable(Player& player, const std::string& contactId)
    {
        for (PlayerCrimeContactState& state : player.getCrimeContactStates())
            if (state.contactId == contactId) return &state;
        return nullptr;
    }

    void clampState(PlayerCrimeContactState& state)
    {
        state.reliability = std::clamp(state.reliability, 0, 100);
        state.favorBalance = std::clamp(state.favorBalance, -100, 100);
        state.rivalry = std::clamp(state.rivalry, 0, 100);
        state.cooperation = std::clamp(state.cooperation, -100, 100);
        state.pressure = std::clamp(state.pressure, 0, 100);
    }

    CrimeContactInteractionResult missingContact()
    {
        CrimeContactInteractionResult result;
        result.lines.push_back("Aucun contact criminel connu ne correspond à cette identité.");
        return result;
    }
}

CrimeContactRelationshipAssessment CrimeContactRelationshipSystem::assess(const Player& player, const std::string& contactId)
{
    CrimeContactRelationshipAssessment result;
    const PlayerCrimeContactState* state = player.findCrimeContactState(contactId);
    if (state == nullptr) return result;

    const int day = player.getWorldDaysElapsed();
    result.known = true;
    result.ruptured = state->ruptureUntilDay > day;
    result.available = !state->arrested
        && state->detainedUntilDay <= day
        && state->fledUntilDay <= day
        && !result.ruptured;
    result.reliability = std::clamp(state->reliability, 0, 100);
    result.favorBalance = std::clamp(state->favorBalance, -100, 100);
    result.rivalry = std::clamp(state->rivalry, 0, 100);
    result.cooperation = std::clamp(state->cooperation, -100, 100);
    result.pressure = std::clamp(state->pressure, 0, 100);
    result.reliabilityLabel = reliabilityLabel(result.reliability);
    result.debtLabel = debtLabel(result.favorBalance);
    result.relationshipLabel = relationshipLabel(*state, day);
    return result;
}

CrimeContactInteractionResult CrimeContactRelationshipSystem::recordReliableDeal(
    Player& player,
    const std::string& contactId,
    int contactFavorOwedToPlayer,
    const std::string& reason)
{
    PlayerCrimeContactState* state = findMutable(player, contactId);
    if (state == nullptr) return missingContact();

    CrimeContactInteractionResult result;
    const int reliabilityBefore = state->reliability;
    const int favorBefore = state->favorBalance;
    const int rivalryBefore = state->rivalry;
    const int day = player.getWorldDaysElapsed();

    if (state->ruptureUntilDay >= 0 && state->ruptureUntilDay <= day)
    {
        state->ruptureUntilDay = -1;
        result.ruptureEnded = true;
    }

    state->reliability += 7;
    state->favorBalance += std::max(0, contactFavorOwedToPlayer);
    state->rivalry -= 5;
    state->cooperation += 5;
    ++state->fulfilledDeals;
    state->lastInteractionDay = day;
    clampState(*state);

    result.changed = true;
    result.reliabilityDelta = state->reliability - reliabilityBefore;
    result.favorDelta = state->favorBalance - favorBefore;
    result.rivalryDelta = state->rivalry - rivalryBefore;
    result.lines.push_back(state->displayName + " retient que cet accord a été respecté.");
    if (contactFavorOwedToPlayer > 0) result.lines.push_back("Une faveur reste maintenant en ta faveur dans cette relation.");

    player.recordCanonicalEvent(
        "crime_contact_reliable_deals",
        contactId,
        (reason.empty() ? std::string("Accord criminel respecté") : reason)
            + " | fiabilite=" + std::to_string(state->reliability)
            + " | solde_faveur=" + std::to_string(state->favorBalance),
        1);
    return result;
}

CrimeContactInteractionResult CrimeContactRelationshipSystem::recordPlayerDebt(
    Player& player,
    const std::string& contactId,
    int amount,
    const std::string& reason)
{
    PlayerCrimeContactState* state = findMutable(player, contactId);
    if (state == nullptr) return missingContact();

    CrimeContactInteractionResult result;
    const int favorBefore = state->favorBalance;
    state->favorBalance -= std::max(0, amount);
    state->cooperation += std::max(0, amount) / 8;
    state->lastInteractionDay = player.getWorldDaysElapsed();
    clampState(*state);

    result.changed = true;
    result.favorDelta = state->favorBalance - favorBefore;
    result.lines.push_back("Le service est accordé, mais " + state->displayName + " n'oubliera pas ce qui lui est dû.");
    player.recordCanonicalEvent(
        "crime_contact_debts",
        contactId,
        (reason.empty() ? std::string("Dette envers un contact criminel") : reason)
            + " | solde_faveur=" + std::to_string(state->favorBalance),
        1);
    return result;
}

CrimeContactInteractionResult CrimeContactRelationshipSystem::settlePlayerDebt(
    Player& player,
    const std::string& contactId,
    int amount,
    const std::string& reason)
{
    PlayerCrimeContactState* state = findMutable(player, contactId);
    if (state == nullptr) return missingContact();

    CrimeContactInteractionResult result;
    const int favorBefore = state->favorBalance;
    if (state->favorBalance < 0)
    {
        state->favorBalance = std::min(0, state->favorBalance + std::max(0, amount));
        state->reliability += 2;
        state->rivalry -= 2;
    }
    state->lastInteractionDay = player.getWorldDaysElapsed();
    clampState(*state);

    result.changed = state->favorBalance != favorBefore;
    result.favorDelta = state->favorBalance - favorBefore;
    result.lines.push_back(result.changed
        ? "Une partie de la dette est réglée proprement."
        : "Aucune dette active ne nécessitait ce règlement.");
    if (result.changed)
    {
        player.recordCanonicalEvent(
            "crime_contact_debts_settled",
            contactId,
            (reason.empty() ? std::string("Dette réglée") : reason)
                + " | solde_faveur=" + std::to_string(state->favorBalance),
            1);
    }
    return result;
}

CrimeContactInteractionResult CrimeContactRelationshipSystem::recordBrokenDeal(
    Player& player,
    const std::string& contactId,
    int severity,
    const std::string& reason)
{
    PlayerCrimeContactState* state = findMutable(player, contactId);
    if (state == nullptr) return missingContact();

    CrimeContactInteractionResult result;
    const int safeSeverity = std::clamp(severity, 1, 5);
    const int reliabilityBefore = state->reliability;
    const int favorBefore = state->favorBalance;
    const int rivalryBefore = state->rivalry;
    const int day = player.getWorldDaysElapsed();

    state->reliability -= 8 + safeSeverity * 5;
    state->favorBalance -= safeSeverity * 4;
    state->rivalry += 8 + safeSeverity * 7;
    state->cooperation -= 5 + safeSeverity * 4;
    state->pressure += safeSeverity * 3;
    ++state->brokenDeals;
    state->lastInteractionDay = day;
    clampState(*state);

    if (state->rivalry >= 65 || state->reliability <= 20 || safeSeverity >= 5)
    {
        const int duration = 2 + safeSeverity * 2 + state->brokenDeals;
        const int until = day + std::min(18, duration);
        if (until > state->ruptureUntilDay)
        {
            state->ruptureUntilDay = until;
            result.ruptureStarted = true;
            result.ruptureUntilDay = until;
        }
    }

    result.changed = true;
    result.reliabilityDelta = state->reliability - reliabilityBefore;
    result.favorDelta = state->favorBalance - favorBefore;
    result.rivalryDelta = state->rivalry - rivalryBefore;
    result.lines.push_back(state->displayName + " considère l'accord comme rompu.");
    if (result.ruptureStarted)
        result.lines.push_back("Le contact coupe temporairement les échanges directs jusqu'au jour " + std::to_string(state->ruptureUntilDay) + ".");

    player.recordCanonicalEvent(
        "crime_contact_broken_deals",
        contactId,
        (reason.empty() ? std::string("Accord criminel rompu") : reason)
            + " | severite=" + std::to_string(safeSeverity)
            + " | fiabilite=" + std::to_string(state->reliability)
            + " | rivalite=" + std::to_string(state->rivalry)
            + " | rupture_jusqua=" + std::to_string(state->ruptureUntilDay),
        1);
    return result;
}

std::string CrimeContactRelationshipSystem::chooseKnownAlternateRelay(const Player& player, const std::string& contactId)
{
    const VaultTheftNetworkContact* source = VaultTheftNetworkCatalog::findById(contactId);
    if (source == nullptr || source->networkId.empty()) return {};

    std::string bestId;
    int bestScore = -100000;
    for (const PlayerCrimeContactState& state : player.getCrimeContactStates())
    {
        if (state.contactId == contactId) continue;
        const VaultTheftNetworkContact* candidate = VaultTheftNetworkCatalog::findById(state.contactId);
        if (candidate == nullptr || candidate->networkId != source->networkId) continue;

        const CrimeNetworkRelayAssessment relay = CrimeNetworkSystem::assessKnownRelay(player, contactId, state.contactId);
        if (!relay.relayLikely) continue;
        const CrimeContactRelationshipAssessment relation = assess(player, state.contactId);
        if (!relation.available) continue;

        const int score = relay.relayReadiness
            + relation.reliability
            + relation.cooperation / 2
            - relation.rivalry
            + std::max(0, relation.favorBalance) / 2;
        if (score > bestScore)
        {
            bestScore = score;
            bestId = state.contactId;
        }
    }
    return bestId;
}
std::string CrimeContactRelationshipSystem::chooseKnownReplacementContact(const Player& player, const std::string& contactId)
{
    const VaultTheftNetworkContact* source = VaultTheftNetworkCatalog::findById(contactId);
    if (source == nullptr || source->networkId.empty()) return {};

    std::string bestId;
    int bestScore = -100000;
    const int day = player.getWorldDaysElapsed();
    for (const PlayerCrimeContactState& state : player.getCrimeContactStates())
    {
        if (state.contactId == contactId) continue;
        const VaultTheftNetworkContact* candidate = VaultTheftNetworkCatalog::findById(state.contactId);
        if (candidate == nullptr || candidate->networkId != source->networkId) continue;
        if (state.arrested || state.detainedUntilDay > day || state.fledUntilDay > day || state.ruptureUntilDay > day) continue;

        const CrimeContactRelationshipAssessment relation = assess(player, state.contactId);
        if (!relation.known || !relation.available) continue;
        const int score = relation.reliability
            + relation.cooperation / 2
            - relation.rivalry
            - relation.pressure / 3
            + std::max(0, relation.favorBalance) / 2;
        if (score > bestScore)
        {
            bestScore = score;
            bestId = state.contactId;
        }
    }
    return bestId;
}
