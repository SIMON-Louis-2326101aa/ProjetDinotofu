#include "world/bounty/BountyRegionalConsequenceSystem.hpp"
#include "world/bounty/BountySocialConsequenceSystem.hpp"

#include "entity/Player.hpp"
#include "quest/Quest.hpp"
#include "world/crime/VaultTheftNetworkCatalog.hpp"
#include "world/CityProfileSystem.hpp"
#include "world/rumor/WorldRumorSystem.hpp"

#include <algorithm>

namespace
{
    int localCategoryTotal(const Player& player, const std::string& cityId, const std::string& category)
    {
        int total = 0;
        for (const PlayerJournalRecord& record : player.getCanonicalJournalRecords())
        {
            if (record.locationId == cityId && record.category == category && record.count > 0)
            {
                total += record.count;
            }
        }
        return total;
    }

    std::string resolvedCityId(const Player& player)
    {
        return player.getCurrentCityId().empty() ? std::string("unknown_region") : player.getCurrentCityId();
    }

    std::string targetLabel(const Quest& quest)
    {
        return quest.bountyTargetName.empty() ? quest.title : quest.bountyTargetName;
    }

    int clampTrait(int value)
    {
        return std::clamp(value, 0, 95);
    }
}

BountyRegionalState BountyRegionalConsequenceSystem::stateFor(const Player& player, const std::string& cityId)
{
    BountyRegionalState state;
    if (cityId.empty()) return state;

    state.aliveCaptures = localCategoryTotal(player, cityId, "primes_capturees_vivantes");
    state.lethalResolutions = localCategoryTotal(player, cityId, "primes_resolutions_mortelles");
    state.noisyHunts = localCategoryTotal(player, cityId, "primes_traques_bruyantes");
    state.recordedEscapeAttempts = localCategoryTotal(player, cityId, "primes_evasions_cibles");

    // Every resolved hunt teaches the underworld something. Clean live captures tend to dry up
    // willing support, while lethal/noisy hunts make local criminals close ranks and prepare more.
    state.awarenessPressure = std::min(24,
        state.aliveCaptures * 2
        + state.lethalResolutions * 4
        + state.noisyHunts * 3
        + state.recordedEscapeAttempts * 3);

    state.supportShift = std::clamp(
        -state.aliveCaptures * 2
        + state.lethalResolutions * 2
        + state.noisyHunts,
        -14,
        14);

    state.resourceShift = std::clamp(
        -state.aliveCaptures
        + state.lethalResolutions
        + state.noisyHunts * 2
        + state.recordedEscapeAttempts,
        -8,
        12);

    state.mobilityPressure = std::min(14,
        state.recordedEscapeAttempts * 3
        + state.noisyHunts);

    return state;
}

BountyRegionalAdaptationResult BountyRegionalConsequenceSystem::adaptTargetToRegion(Player& player, Quest& quest)
{
    BountyRegionalAdaptationResult result;
    if (!quest.bountyMission) return result;

    const std::string cityId = quest.bountyCurrentCityId.empty() ? resolvedCityId(player) : quest.bountyCurrentCityId;

    // Compatibility with saves created before bountyRegionalProfileCityId existed: if the old
    // boolean says the regional profile was already applied, consider the target adapted to its
    // current/legacy city instead of applying the same modifiers a second time on first reload.
    if (quest.bountyRegionalProfileApplied && quest.bountyRegionalProfileCityId.empty())
    {
        quest.bountyRegionalProfileCityId = cityId;
        return result;
    }
    if (quest.bountyRegionalProfileApplied && quest.bountyRegionalProfileCityId == cityId) return result;

    // A mobile target can change city. Regional history is applied once when entering the new
    // region, while the structural city modifier is applied as a delta from the previous city so
    // security bonuses do not stack forever after several relocations.
    const std::string previousCityId = quest.bountyRegionalProfileCityId;
    const CityBountyEnvironment previousEnvironment = previousCityId.empty()
        ? CityBountyEnvironment{}
        : CityProfileSystem::bountyEnvironmentFor(previousCityId);

    quest.bountyRegionalProfileApplied = true;
    quest.bountyRegionalProfileCityId = cityId;
    result.applied = true;

    const BountyRegionalState state = stateFor(player, cityId);
    const CityGameplayProfile cityProfile = CityProfileSystem::profileFor(cityId);
    const CityBountyEnvironment cityEnvironment = CityProfileSystem::bountyEnvironmentFor(cityId);

    const int awarenessBefore = quest.bountyTargetAwareness;
    const int supportBefore = quest.bountyTargetLocalSupport;
    const int resourcesBefore = quest.bountyTargetResources;
    const int mobilityBefore = quest.bountyTargetMobility;

    quest.bountyTargetAwareness = clampTrait(quest.bountyTargetAwareness + state.awarenessPressure
        + cityEnvironment.awarenessDelta - previousEnvironment.awarenessDelta);
    quest.bountyTargetLocalSupport = clampTrait(quest.bountyTargetLocalSupport + state.supportShift
        + cityEnvironment.localSupportDelta - previousEnvironment.localSupportDelta);
    quest.bountyTargetResources = clampTrait(quest.bountyTargetResources + state.resourceShift
        + cityEnvironment.resourcesDelta - previousEnvironment.resourcesDelta);
    quest.bountyTargetMobility = clampTrait(quest.bountyTargetMobility + state.mobilityPressure
        + cityEnvironment.mobilityDelta - previousEnvironment.mobilityDelta);

    result.awarenessDelta = quest.bountyTargetAwareness - awarenessBefore;
    result.localSupportDelta = quest.bountyTargetLocalSupport - supportBefore;
    result.resourcesDelta = quest.bountyTargetResources - resourcesBefore;
    result.mobilityDelta = quest.bountyTargetMobility - mobilityBefore;

    result.lines.push_back("Profil local : zone " + CityProfileSystem::securityLabel(cityProfile.security)
        + ". " + CityProfileSystem::securityDescription(cityProfile.security) + ".");

    const bool hasRegionalHistory = state.aliveCaptures > 0
        || state.lethalResolutions > 0
        || state.noisyHunts > 0
        || state.recordedEscapeAttempts > 0;
    if (hasRegionalHistory)
    {
        result.lines.push_back("La région se souvient des chasses précédentes : les prochaines cibles et leurs soutiens adaptent réellement leurs habitudes.");
    }
    if (state.aliveCaptures > 0 && state.supportShift < 0)
    {
        result.lines.push_back("Les captures vivantes précédentes ont refroidi une partie des relais locaux : moins de gens veulent risquer leur peau pour une cible recherchée.");
    }
    if (state.lethalResolutions > 0 || state.noisyHunts > 0)
    {
        result.lines.push_back("Les résolutions violentes ou trop visibles ont aussi rendu le milieu local plus méfiant et mieux préparé.");
    }
    if (state.recordedEscapeAttempts > 0)
    {
        result.lines.push_back("Les anciennes évasions ont circulé comme leçons pratiques : les fugitifs préparent davantage leurs routes de fuite.");
    }
    return result;
}

BountyRegionalConsequenceResult BountyRegionalConsequenceSystem::applyResolution(
    Player& player,
    Quest& quest,
    BountyResolutionKind resolution)
{
    BountyRegionalConsequenceResult result;
    if (!quest.bountyMission || quest.bountyRegionalConsequencesApplied) return result;

    quest.bountyRegionalConsequencesApplied = true;
    quest.bountyResolution = resolutionId(resolution);
    result.applied = true;

    const std::string cityId = resolvedCityId(player);
    const std::string target = targetLabel(quest);
    result.noisyHunt = quest.bountyInvestigationNoise >= 35 || quest.bountyInformantExposure >= 25;

    switch (resolution)
    {
        case BountyResolutionKind::CapturedBySurrender:
            player.recordCanonicalEvent("primes_capturees_vivantes", quest.id, target + " capturé(e) vivant(e) après reddition", 1);
            result.localReputationGain = 3;
            break;
        case BountyResolutionKind::CapturedAfterCombat:
            player.recordCanonicalEvent("primes_capturees_vivantes", quest.id, target + " capturé(e) vivant(e) après confrontation", 1);
            result.localReputationGain = 2;
            break;
        case BountyResolutionKind::KilledInCombat:
            player.recordCanonicalEvent("primes_resolutions_mortelles", quest.id, target + " tué(e) pendant la confrontation", 1);
            result.localReputationGain = 1;
            break;
    }

    if (result.noisyHunt)
    {
        player.recordCanonicalEvent(
            "primes_traques_bruyantes",
            quest.id,
            "Traque très visible autour de " + target,
            1);
    }
    if (quest.bountyTargetEscapeAttempts > 0)
    {
        player.recordCanonicalEvent(
            "primes_evasions_cibles",
            quest.id,
            target + " avait déjà réussi à échapper à la chasse",
            std::min(4, quest.bountyTargetEscapeAttempts));
    }
    if (quest.bountyRelocationCount > 0
        && resolution != BountyResolutionKind::KilledInCombat)
    {
        player.recordCanonicalEvent(
            "primes_capture_apres_relocalisation",
            quest.id,
            target + " capturé(e) après un véritable changement de ville",
            1
        );
    }

    if (result.localReputationGain > 0)
    {
        player.recordCanonicalEvent(
            "reputation_locale_positive",
            "prime:" + quest.id,
            "Résolution d'une prime ciblée : " + target,
            result.localReputationGain);
    }

    const int basePressure = resolution == BountyResolutionKind::KilledInCombat
        ? 12
        : (resolution == BountyResolutionKind::CapturedAfterCombat ? 9 : 7);
    const int cooperationLoss = resolution == BountyResolutionKind::KilledInCombat
        ? 6
        : (resolution == BountyResolutionKind::CapturedAfterCombat ? 3 : 2);
    const int noisyExtra = result.noisyHunt ? 4 : 0;

    for (PlayerCrimeContactState& contactState : player.getCrimeContactStates())
    {
        if (contactState.arrested) continue;

        std::string homeCityId = contactState.homeCityId;
        if (homeCityId.empty())
        {
            const VaultTheftNetworkContact* knownContact = VaultTheftNetworkCatalog::findById(contactState.contactId);
            if (knownContact != nullptr)
            {
                homeCityId = knownContact->cityId;
                contactState.homeCityId = knownContact->cityId;
                if (contactState.networkId.empty()) contactState.networkId = knownContact->networkId;
            }
        }
        if (homeCityId != cityId) continue;

        contactState.pressure = std::clamp(contactState.pressure + basePressure + noisyExtra, 0, 100);
        contactState.cooperation = std::clamp(contactState.cooperation - cooperationLoss - (result.noisyHunt ? 2 : 0), -100, 100);
        ++result.knownCrimeContactsAffected;
    }

    if (result.knownCrimeContactsAffected > 0)
    {
        player.recordCanonicalEvent(
            "reactions_reseaux_primes",
            quest.id,
            "Des contacts criminels déjà connus deviennent plus prudents après la résolution de la prime sur " + target,
            result.knownCrimeContactsAffected);
    }

    const std::string rumorId = "bounty_resolution:" + cityId + ":" + quest.id;
    std::string rumorText;
    switch (resolution)
    {
        case BountyResolutionKind::CapturedBySurrender:
            rumorText = target + " a été capturé(e) vivant(e) après avoir fini par se rendre.";
            break;
        case BountyResolutionKind::CapturedAfterCombat:
            rumorText = target + " a été maîtrisé(e) puis capturé(e) vivant(e) après une confrontation.";
            break;
        case BountyResolutionKind::KilledInCombat:
            rumorText = target + " est mort(e) pendant la confrontation liée à sa prime.";
            break;
    }
    WorldRumorSystem::recordRumor(
        player,
        rumorId,
        quest.id,
        rumorText,
        WorldRumorSourceType::DirectObservation,
        80,
        4);

    result.lines.push_back("Conséquence régionale : " + resolutionLabel(resolution) + ".");
    if (result.localReputationGain > 0)
    {
        result.lines.push_back("Réputation locale : +" + std::to_string(result.localReputationGain) + " pour avoir réellement conclu le contrat.");
    }
    if (result.noisyHunt)
    {
        result.lines.push_back("La traque a été assez visible pour laisser une trace : les prochaines cibles locales pourront en tirer des leçons.");
    }
    if (result.knownCrimeContactsAffected > 0)
    {
        result.lines.push_back(std::to_string(result.knownCrimeContactsAffected)
            + " contact(s) criminel(s) déjà connu(s) dans cette région deviennent plus tendus et moins coopératifs.");
    }

    const BountySocialConsequenceResult social = BountySocialConsequenceSystem::applyResolution(player, quest, resolution);
    result.lines.insert(result.lines.end(), social.lines.begin(), social.lines.end());
    result.lines.push_back("Une rumeur locale persistante enregistre maintenant l'issue réelle de cette prime.");
    return result;
}

std::string BountyRegionalConsequenceSystem::resolutionId(BountyResolutionKind resolution)
{
    switch (resolution)
    {
        case BountyResolutionKind::CapturedBySurrender: return "captured_surrender";
        case BountyResolutionKind::CapturedAfterCombat: return "captured_combat";
        case BountyResolutionKind::KilledInCombat: return "killed_combat";
    }
    return "unknown";
}

std::string BountyRegionalConsequenceSystem::resolutionLabel(BountyResolutionKind resolution)
{
    switch (resolution)
    {
        case BountyResolutionKind::CapturedBySurrender: return "cible livrée vivante après reddition";
        case BountyResolutionKind::CapturedAfterCombat: return "cible capturée vivante après confrontation";
        case BountyResolutionKind::KilledInCombat: return "cible morte pendant la confrontation";
    }
    return "issue inconnue";
}
