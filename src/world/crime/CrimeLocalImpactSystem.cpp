#include "world/crime/CrimeLocalImpactSystem.hpp"

#include "entity/Player.hpp"
#include "world/crime/CrimeLocalOpportunitySystem.hpp"
#include "world/crime/CrimeNetworkCompetitionSystem.hpp"
#include "world/crime/VaultTheftNetworkCatalog.hpp"
#include "world/rumor/WorldRumorSystem.hpp"

#include <algorithm>

namespace
{
    std::string cityForContact(const PlayerCrimeContactState& state)
    {
        if (!state.homeCityId.empty()) return state.homeCityId;
        const VaultTheftNetworkContact* definition = VaultTheftNetworkCatalog::findById(state.contactId);
        return definition != nullptr ? definition->cityId : std::string{};
    }

    int recentDisruptionWeight(const Player& player, const std::string& cityId)
    {
        int weight = 0;
        const int day = player.getWorldDaysElapsed();
        for (const PlayerJournalRecord& record : player.getCanonicalJournalRecords())
        {
            if (record.category != "crime_local_disruptions" || record.key != cityId) continue;
            const int age = std::max(0, day - record.lastDay);
            if (age > 5) continue;
            const int freshness = std::max(1, 6 - age);
            weight += std::min(30, record.count * freshness / 2);
        }
        return std::clamp(weight, 0, 35);
    }

    int recentStabilizationWeight(const Player& player, const std::string& cityId)
    {
        int weight = 0;
        const int day = player.getWorldDaysElapsed();
        for (const PlayerJournalRecord& record : player.getCanonicalJournalRecords())
        {
            if (record.category != "crime_local_stabilizations" || record.key != cityId) continue;
            const int age = std::max(0, day - record.lastDay);
            if (age > 5) continue;
            const int freshness = std::max(1, 6 - age);
            weight += std::min(24, record.count * freshness);
        }
        return std::clamp(weight, 0, 30);
    }

    int effectivePressure(const PlayerCrimeContactState& state, int day)
    {
        if (state.lastInteractionDay < 0) return std::clamp(state.pressure, 0, 100);
        const int quietDays = std::max(0, day - state.lastInteractionDay - 1);
        return std::clamp(state.pressure - quietDays * 3, 0, 100);
    }

    std::string opportunityKindFor(int severity, int pressure)
    {
        if (severity >= 5 || pressure >= 70) return "witness_followup";
        if (severity >= 4 || pressure >= 50) return "route_watch";
        return "merchant_check";
    }
}

CrimeLocalImpactSnapshot CrimeLocalImpactSystem::assess(const Player& player, const std::string& cityId)
{
    CrimeLocalImpactSnapshot result;
    if (cityId.empty()) return result;

    int pressureSum = 0;
    int rivalrySum = 0;
    for (const PlayerCrimeContactState& state : player.getCrimeContactStates())
    {
        if (cityForContact(state) != cityId) continue;
        ++result.knownContacts;
        pressureSum += effectivePressure(state, player.getWorldDaysElapsed());
        rivalrySum += std::clamp(state.rivalry, 0, 100);
        if (state.arrested) ++result.arrestedContacts;
        else if (state.detainedUntilDay >= player.getWorldDaysElapsed()
            || state.fledUntilDay >= player.getWorldDaysElapsed()
            || state.ruptureUntilDay >= player.getWorldDaysElapsed())
        {
            ++result.unavailableContacts;
        }
    }

    const CrimeNetworkCompetitionAssessment competition = CrimeNetworkCompetitionSystem::assessKnownCity(player, cityId);
    result.knownNetworks = competition.knownNetworks;
    result.competitionTension = competition.tension;
    result.recentDisruptionWeight = recentDisruptionWeight(player, cityId);
    result.recentStabilizationWeight = recentStabilizationWeight(player, cityId);

    const int divisor = std::max(1, result.knownContacts);
    const int averagePressure = pressureSum / divisor;
    const int averageRivalry = rivalrySum / divisor;
    result.pressureScore = std::clamp(
        std::max(0, result.recentDisruptionWeight - result.recentStabilizationWeight)
            + averagePressure / 4
            + averageRivalry / 5
            + result.arrestedContacts * 12
            + result.unavailableContacts * 5
            + (competition.observable ? competition.tension / 4 : 0),
        0,
        100);

    int newestInteractionDay = -1;
    for (const PlayerCrimeContactState& state : player.getCrimeContactStates())
    {
        if (cityForContact(state) != cityId) continue;
        newestInteractionDay = std::max(newestInteractionDay, state.lastInteractionDay);
    }
    if (newestInteractionDay >= 0)
        result.quietRecoveryDays = std::max(0, player.getWorldDaysElapsed() - newestInteractionDay - 1);

    result.observable = result.knownContacts > 0 && result.pressureScore >= 15;
    if (!result.observable)
    {
        result.levelId = "quiet";
        result.headline = "aucune perturbation criminelle locale clairement observable";
        return result;
    }

    if (result.pressureScore >= 70)
    {
        result.levelId = "severe";
        result.headline = "forte recomposition des réseaux connus";
        result.serviceQualityDelta = -5;
        result.tradeSellModifierPercent = 3;
        result.transportRiskPercent = 18;
    }
    else if (result.pressureScore >= 45)
    {
        result.levelId = "disrupted";
        result.headline = "tension criminelle locale visible";
        result.serviceQualityDelta = -3;
        result.tradeSellModifierPercent = 2;
        result.transportRiskPercent = 10;
    }
    else
    {
        result.levelId = "watchful";
        result.headline = "méfiance accrue autour de plusieurs relais connus";
        result.serviceQualityDelta = -1;
        result.tradeSellModifierPercent = 1;
        result.transportRiskPercent = 4;
    }

    result.lines.push_back("Criminalité locale : " + result.headline + " (pression " + std::to_string(result.pressureScore) + "/100)." );
    result.lines.push_back("Contacts connus ici : " + std::to_string(result.knownContacts)
        + " | arrêtés durablement : " + std::to_string(result.arrestedContacts)
        + " | temporairement indisponibles : " + std::to_string(result.unavailableContacts) + ".");
    if (competition.observable)
        result.lines.push_back("Concurrence connue : " + std::to_string(result.knownNetworks)
            + " réseaux | tension observée " + std::to_string(result.competitionTension) + "/100.");
    result.lines.push_back("Effet local mesuré : qualité de certains services " + std::to_string(result.serviceQualityDelta)
        + " | demande commerciale jusqu'à +" + std::to_string(result.tradeSellModifierPercent)
        + "% | risque logistique indicatif +" + std::to_string(result.transportRiskPercent) + "%.");
    if (result.quietRecoveryDays > 0)
        result.lines.push_back("Récupération naturelle : " + std::to_string(result.quietRecoveryDays)
            + " jour(s) relativement calmes réduisent déjà la pression visible, sans effacer les rivalités historiques.");
    if (result.recentStabilizationWeight > 0)
        result.lines.push_back("Stabilisation active : les suivis locaux déjà terminés réduisent une partie du poids des perturbations récentes.");
    return result;
}

CrimeLocalImpactRegistration CrimeLocalImpactSystem::registerDisruption(
    Player& player,
    const std::string& cityId,
    const std::string& sourceId,
    int severity,
    const std::string& reason)
{
    CrimeLocalImpactRegistration result;
    if (cityId.empty()) return result;
    const int safeSeverity = std::clamp(severity, 1, 5);
    const std::string cleanReason = reason.empty()
        ? std::string("Une perturbation criminelle modifie temporairement les habitudes locales")
        : reason;

    player.recordCanonicalEvent(
        "crime_local_disruptions",
        cityId,
        "source=" + (sourceId.empty() ? std::string("unknown") : sourceId)
            + " | severity=" + std::to_string(safeSeverity)
            + " | reason=" + cleanReason,
        safeSeverity);
    player.recordHistoricalEvent(
        "crime_local_disruption",
        cityId,
        cleanReason + ". La conséquence reste locale et temporaire tant que rien ne justifie davantage.",
        true);
    result.recorded = true;

    const CrimeLocalImpactSnapshot snapshot = assess(player, cityId);
    WorldRumorSystem::recordRumor(
        player,
        "crime_local_shift:" + cityId,
        "city:" + cityId,
        safeSeverity >= 4
            ? "Des habitants parlent d'un changement visible dans les relais et habitudes des réseaux criminels connus."
            : "Plusieurs commerçants et habitués disent que certains relais deviennent plus prudents depuis les derniers événements.",
        WorldRumorSourceType::Witness,
        40 + safeSeverity * 5,
        1);
    result.rumorRecorded = true;

    if (safeSeverity >= 3 || snapshot.pressureScore >= 35)
    {
        result.opportunityKind = opportunityKindFor(safeSeverity, snapshot.pressureScore);
        result.opportunityQueued = CrimeLocalOpportunitySystem::queue(
            player,
            cityId,
            result.opportunityKind,
            sourceId,
            "La guilde conserve un petit suivi après cette perturbation : " + cleanReason + ".");
    }

    result.lines.push_back("La perturbation est enregistrée comme conséquence locale, pas comme crise permanente de la ville.");
    if (result.rumorRecorded)
        result.lines.push_back("Une rumeur locale sourcée commence à circuler ; elle reste distincte des faits confirmés.");
    if (result.opportunityQueued)
        result.lines.push_back("La guilde garde une petite opportunité de suivi disponible au comptoir.");
    return result;
}

void CrimeLocalImpactSystem::applyEconomicPressure(
    const Player& player,
    const std::string& cityId,
    CityEconomicPulse& pulse)
{
    const CrimeLocalImpactSnapshot impact = assess(player, cityId);
    if (!impact.observable) return;

    pulse.generalSellModifierPercent = std::clamp(
        pulse.generalSellModifierPercent + impact.tradeSellModifierPercent,
        -10,
        10);
    pulse.serviceQualityDelta = std::clamp(
        pulse.serviceQualityDelta + impact.serviceQualityDelta,
        -20,
        12);

    if (impact.pressureScore >= 45)
    {
        CityMarketDemand transportDemand;
        transportDemand.goodsType = ShopType::Transport;
        transportDemand.label = "petite logistique, scellés, cordages et matériel de relais perturbés";
        transportDemand.sellBonusPercent = impact.pressureScore >= 70 ? 5 : 3;
        pulse.demands.push_back(transportDemand);
    }
}

std::vector<std::string> CrimeLocalImpactSystem::informationLines(const Player& player, const std::string& cityId)
{
    const CrimeLocalImpactSnapshot impact = assess(player, cityId);
    if (!impact.observable)
        return {"Criminalité locale : aucun changement suffisamment documenté pour modifier les services ou les commerces."};
    return impact.lines;
}
