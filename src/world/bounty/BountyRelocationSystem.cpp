#include "world/bounty/BountyRelocationSystem.hpp"
#include "world/bounty/BountyTradeTransitSystem.hpp"

#include "entity/Player.hpp"
#include "quest/Quest.hpp"
#include "world/City.hpp"
#include "world/CityProfileSystem.hpp"
#include "world/CityTravelRules.hpp"

#include <algorithm>
#include <functional>
#include <sstream>

namespace
{
    bool pipeListContains(const std::string& pipeList, const std::string& value)
    {
        if (value.empty()) return false;
        std::istringstream stream(pipeList);
        std::string token;
        while (std::getline(stream, token, '|'))
        {
            if (token == value) return true;
        }
        return false;
    }

    void appendPipeUnique(std::string& pipeList, const std::string& value)
    {
        if (value.empty() || pipeListContains(pipeList, value)) return;
        if (!pipeList.empty()) pipeList += "|";
        pipeList += value;
    }

    bool cityOwnsBiome(const std::string& cityId, const std::string& biomeName)
    {
        const City* city = City::findById(cityId);
        if (city == nullptr || biomeName.empty()) return false;
        for (const CityBiomeDistance& biome : city->getBiomeDistances())
        {
            if (biome.biomeName == biomeName) return true;
        }
        return false;
    }

    std::string inferCityFromBiome(const std::string& biomeName)
    {
        if (biomeName.empty()) return "";
        std::string found;
        for (const City& city : City::getCatalog())
        {
            if (!cityOwnsBiome(city.getId(), biomeName)) continue;
            if (!found.empty()) return ""; // Ambiguous biome: do not guess.
            found = city.getId();
        }
        return found;
    }

    int refugeBiomeScore(const std::string& name)
    {
        int score = 0;
        auto has = [&](const std::string& needle) {
            return name.find(needle) != std::string::npos;
        };

        if (has("Marché") || has("marché")) score += 22;
        if (has("Quartier") || has("quartier")) score += 20;
        if (has("Canaux") || has("canaux")) score += 16;
        if (has("Route") || has("route")) score += 15;
        if (has("Forêt") || has("forêt")) score += 12;
        if (has("Marais") || has("marais")) score += 12;
        if (has("Ruines") || has("ruines")) score += 11;
        if (has("Mine") || has("mine")) score += 10;
        if (has("Montagne") || has("montagne")) score += 8;
        if (has("Foire") || has("foire")) score += 8;
        if (has("Cimetière") || has("cimetière")) score += 7;
        if (has("Temple") || has("temple")) score += 4;
        return score;
    }

    std::string chooseRefugeBiome(const Quest& quest, const City& city, int salt)
    {
        const auto& biomes = city.getBiomeDistances();
        if (biomes.empty()) return "Ville et alentours";

        int bestScore = -100000;
        std::string best = biomes.front().biomeName;
        for (const CityBiomeDistance& biome : biomes)
        {
            int score = refugeBiomeScore(biome.biomeName);
            if (biome.initiallyKnown) score += 4;
            const std::size_t h = std::hash<std::string>{}(
                quest.id + "|" + city.getId() + "|" + biome.biomeName + "|" + std::to_string(salt));
            score += static_cast<int>(h % 13);
            if (score > bestScore)
            {
                bestScore = score;
                best = biome.biomeName;
            }
        }
        return best;
    }

    int securityDestinationScore(CitySecurityTier tier)
    {
        switch (tier)
        {
            case CitySecurityTier::HighlySecured: return -18;
            case CitySecurityTier::Watched: return -7;
            case CitySecurityTier::NeutralIndependent: return 8;
            case CitySecurityTier::Unstable: return 15;
            case CitySecurityTier::ChaoticAnarchic: return 24;
        }
        return 0;
    }

    int securityDeparturePressure(CitySecurityTier tier)
    {
        switch (tier)
        {
            case CitySecurityTier::HighlySecured: return 16;
            case CitySecurityTier::Watched: return 8;
            case CitySecurityTier::NeutralIndependent: return 0;
            case CitySecurityTier::Unstable: return -5;
            case CitySecurityTier::ChaoticAnarchic: return -10;
        }
        return 0;
    }

    int specializationEscapeScore(const CityGameplayProfile& profile)
    {
        int score = 0;
        for (const CityEconomicSpecialization& spec : profile.specializations)
        {
            if (spec.shopType == ShopType::BlackMarket) score += 6 * std::clamp(spec.strength, 1, 3);
            if (spec.shopType == ShopType::Transport) score += 3 * std::clamp(spec.strength, 1, 3);
        }
        return score;
    }

    std::string cityName(const std::string& cityId)
    {
        const City* city = City::findById(cityId);
        return city == nullptr ? cityId : city->getName();
    }

    std::string trailLabel(const std::string& kind)
    {
        if (kind == "coach_registry") return "registre de diligence / caravane";
        if (kind == "road_witness") return "témoignage de route";
        if (kind == "suspicious_purchase") return "achat inhabituel repéré chez un marchand";
        if (kind == "criminal_contact") return "contact criminel indirect";
        if (kind == "ferryman_trace") return "trace laissée auprès d'un passeur";
        if (kind == "planted_false_lead") return "piste possiblement préparée pour tromper les chasseurs";
        if (kind == "hidden_in_merchant_convoy") return "passage dissimulé dans un convoi marchand";
        if (kind == "bribed_convoy_driver") return "conducteur ou convoyeur probablement soudoyé";
        if (kind == "stolen_trade_cargo") return "vol ou intrusion dans une cargaison commerciale";
        if (kind == "delayed_convoy_camp") return "passage dans un convoi retardé ou immobilisé";
        if (kind == "merchant_route_witness") return "témoignage lié à une route commerciale";
        return "indice de transit inter-ville";
    }

    void createTransitTrail(
        Quest& quest,
        const std::string& fromCityId,
        const std::string& toCityId,
        const BountyTradeTransitOpportunity* tradeOpportunity)
    {
        if (tradeOpportunity != nullptr && tradeOpportunity->available)
        {
            quest.bountyTransitTrailKind = tradeOpportunity->trailKind;
            quest.bountyTransitTrailFromCityId = fromCityId;
            quest.bountyTransitTrailReliability = std::clamp(tradeOpportunity->trailReliability, 20, 90);
            quest.bountyTransitTrailAvailable = true;
            return;
        }
        const CityGameplayProfile destination = CityProfileSystem::profileFor(toCityId);
        bool transportCity = false;
        bool blackMarketCity = false;
        for (const CityEconomicSpecialization& spec : destination.specializations)
        {
            transportCity = transportCity || spec.shopType == ShopType::Transport;
            blackMarketCity = blackMarketCity || spec.shopType == ShopType::BlackMarket;
        }

        const std::size_t h = std::hash<std::string>{}(
            quest.id + "|trail|" + fromCityId + "|" + toCityId + "|" + std::to_string(quest.bountyRelocationCount));
        const int bucket = static_cast<int>(h % 100);

        if (quest.bountyTargetKnowsHunt && quest.bountyTargetResources >= 58 && bucket < 18)
        {
            quest.bountyTransitTrailKind = "planted_false_lead";
            quest.bountyTransitTrailReliability = 30 + static_cast<int>((h / 100) % 16);
        }
        else if (transportCity && bucket < 45)
        {
            quest.bountyTransitTrailKind = "coach_registry";
            quest.bountyTransitTrailReliability = 72 + static_cast<int>((h / 100) % 15);
        }
        else if (blackMarketCity && bucket < 62)
        {
            quest.bountyTransitTrailKind = "criminal_contact";
            quest.bountyTransitTrailReliability = 48 + static_cast<int>((h / 100) % 20);
        }
        else if (bucket < 42)
        {
            quest.bountyTransitTrailKind = "road_witness";
            quest.bountyTransitTrailReliability = 56 + static_cast<int>((h / 100) % 20);
        }
        else if (bucket < 72)
        {
            quest.bountyTransitTrailKind = "suspicious_purchase";
            quest.bountyTransitTrailReliability = 58 + static_cast<int>((h / 100) % 22);
        }
        else
        {
            quest.bountyTransitTrailKind = "ferryman_trace";
            quest.bountyTransitTrailReliability = 52 + static_cast<int>((h / 100) % 24);
        }

        quest.bountyTransitTrailFromCityId = fromCityId;
        quest.bountyTransitTrailReliability = std::clamp(quest.bountyTransitTrailReliability, 20, 90);
        quest.bountyTransitTrailAvailable = true;
    }
}

void BountyRelocationSystem::ensureLocalPosition(Quest& quest, const std::string& fallbackCityId)
{
    if (!quest.bountyMission) return;

    if (quest.bountyCurrentCityId.empty())
    {
        quest.bountyCurrentCityId = inferCityFromBiome(quest.location);
        if (quest.bountyCurrentCityId.empty())
        {
            quest.bountyCurrentCityId = fallbackCityId.empty() ? "valebrume" : fallbackCityId;
        }
    }

    const City* city = City::findById(quest.bountyCurrentCityId);
    if (city == nullptr)
    {
        quest.bountyCurrentCityId = fallbackCityId.empty() ? "valebrume" : fallbackCityId;
        city = City::findById(quest.bountyCurrentCityId);
    }

    if (quest.bountyCurrentBiome.empty())
    {
        if (city != nullptr && cityOwnsBiome(city->getId(), quest.location))
        {
            quest.bountyCurrentBiome = quest.location;
        }
        else if (city != nullptr)
        {
            quest.bountyCurrentBiome = chooseRefugeBiome(quest, *city, quest.bountyRelocationCount);
        }
        else
        {
            quest.bountyCurrentBiome = quest.location.empty() ? "Ville et alentours" : quest.location;
        }
    }

    if (!quest.bountyCurrentBiome.empty()) quest.location = quest.bountyCurrentBiome;
    appendPipeUnique(quest.bountyVisitedCityIds, quest.bountyCurrentCityId);
}

BountyRelocationResult BountyRelocationSystem::tryRelocateAfterEscape(
    const Player& player,
    Quest& quest,
    int roll1To100)
{
    BountyRelocationResult result;
    if (!quest.bountyMission || quest.completed || quest.turnedIn || quest.failed) return result;
    if (quest.bountyTargetEscapeAttempts <= 0) return result;

    ensureLocalPosition(quest, player.getCurrentCityId());
    result.evaluated = true;
    result.fromCityId = quest.bountyCurrentCityId;

    const CityGameplayProfile originProfile = CityProfileSystem::profileFor(result.fromCityId);
    const bool mobileEnough = quest.bountyTargetMobility >= 58;
    const bool establishedFugitive = quest.bountyDaysAtLarge >= 35;
    const bool dangerousEnough = quest.bountyDangerRating >= 3;
    if (!mobileEnough && !establishedFugitive && !dangerousEnough)
    {
        result.lines.push_back("La cible fuit l'interception mais ne paraît pas capable d'abandonner toute la région : la piste reste locale.");
        return result;
    }

    int chance = 16
        + quest.bountyTargetMobility / 3
        + quest.bountyTargetResources / 8
        + quest.bountyTargetEscapeAttempts * 7
        + securityDeparturePressure(originProfile.security);
    if (quest.bountyTargetKnowsHunt) chance += 10;
    if (quest.bountyTargetInjured) chance -= 14;
    if (quest.bountyDaysAtLarge >= 90) chance += 6;

    const City* origin = City::findById(result.fromCityId);
    const City* bestDestination = nullptr;
    BountyTradeTransitOpportunity bestTradeOpportunity;
    int bestScore = -100000;

    for (const City& candidate : City::getCatalog())
    {
        if (candidate.getId() == result.fromCityId) continue;
        if (pipeListContains(quest.bountyVisitedCityIds, candidate.getId())) continue;

        // A live bounty must never become impossible merely because its target crossed an
        // unavailable level/boss/administrative gate.
        if (!CityTravelRules::evaluateAccess(player, candidate).allowed) continue;

        const CityGameplayProfile profile = CityProfileSystem::profileFor(candidate.getId());
        int score = securityDestinationScore(profile.security) + specializationEscapeScore(profile);
        if (origin != nullptr)
        {
            score -= std::max(0, City::calculateDistanceBetween(*origin, candidate)) / 7;
        }
        if (quest.bountyTargetResources >= 60 && profile.wealth >= CityWealthTier::Rich) score += 4;
        if (quest.bountyTargetLocalSupport >= 45 && profile.security == CitySecurityTier::NeutralIndependent) score += 4;

        const BountyTradeTransitOpportunity tradeOpportunity = BountyTradeTransitSystem::assess(
            quest,
            result.fromCityId,
            candidate.getId(),
            player.getWorldDaysElapsed());
        score += tradeOpportunity.destinationScoreBonus;

        const std::size_t h = std::hash<std::string>{}(
            quest.id + "|relocation|" + std::to_string(quest.bountyRelocationCount) + "|" + candidate.getId());
        score += static_cast<int>(h % 15);
        if (score > bestScore)
        {
            bestScore = score;
            bestDestination = &candidate;
            bestTradeOpportunity = tradeOpportunity;
        }
    }

    // If every unvisited city is inaccessible, keep the contract local rather than cheating a gate.
    if (bestDestination == nullptr)
    {
        result.relocationChancePercent = std::clamp(chance, 18, 88);
        result.lines.push_back("La cible tente de quitter la région, mais les routes exploitables sont trop risquées ou fermées : elle se replie sur un autre refuge local.");
        return result;
    }

    chance += bestTradeOpportunity.relocationChanceBonus;
    result.relocationChancePercent = std::clamp(chance, 18, 92);
    result.usedTradeRoute = bestTradeOpportunity.available;
    result.tradeTransitKind = bestTradeOpportunity.trailKind;

    if (std::clamp(roll1To100, 1, 100) > result.relocationChancePercent)
    {
        result.lines.push_back("La cible change de cache et brouille sa piste, mais les indices restent dans la même région.");
        if (bestTradeOpportunity.available)
        {
            result.lines.push_back("Une route commerciale aurait pu faciliter une fuite plus longue, mais la cible n'arrive pas à l'exploiter avant que la pression locale ne remonte.");
        }
        return result;
    }

    quest.bountyCurrentCityId = bestDestination->getId();
    ++quest.bountyRelocationCount;
    quest.bountyCurrentBiome = chooseRefugeBiome(quest, *bestDestination, quest.bountyRelocationCount);
    quest.location = quest.bountyCurrentBiome;
    appendPipeUnique(quest.bountyVisitedCityIds, quest.bountyCurrentCityId);

    // A city change destroys exact local preparation. The player keeps only a broad lead and must
    // rebuild a real trail instead of chaining an interception immediately after fast travel.
    quest.bountyClues = std::min(1, quest.bountyClues);
    quest.bountyExitRecon = 0;
    quest.bountyAmbushPreparation = 0;
    quest.bountyShadowingPreparation = 0;
    quest.bountyInformantNetwork = std::max(0, quest.bountyInformantNetwork - 1);
    quest.bountyRouteVariation = std::max(0, quest.bountyRouteVariation - 1);
    quest.bountyRefugeConcealment = std::max(0, quest.bountyRefugeConcealment - 1);
    quest.bountyStage = "investigation";
    createTransitTrail(
        quest,
        result.fromCityId,
        quest.bountyCurrentCityId,
        bestTradeOpportunity.available ? &bestTradeOpportunity : nullptr);

    result.moved = true;
    result.toCityId = quest.bountyCurrentCityId;
    result.toBiome = quest.bountyCurrentBiome;
    result.lines.push_back("La fuite devient inter-ville : la cible abandonne " + cityName(result.fromCityId)
        + " avant que les recherches locales ne se referment sur elle.");
    if (bestTradeOpportunity.available)
    {
        result.lines.push_back(bestTradeOpportunity.summary);
    }
    result.lines.push_back("Une piste de relais et de témoins la replace vers " + cityName(result.toCityId)
        + ", probablement autour de : " + result.toBiome + ".");
    result.lines.push_back("Tes préparatifs exacts de l'ancienne ville expirent. Il reste seulement une piste générale : il faudra enquêter de nouveau sur place.");
    result.lines.push_back("Trace inter-ville : " + trailLabel(quest.bountyTransitTrailKind)
        + " (fiabilité estimée " + std::to_string(quest.bountyTransitTrailReliability) + "/100). Elle pourra aider une enquête locale, mais pas fournir une interception gratuite.");
    return result;
}

bool BountyRelocationSystem::isTargetCurrentlyInCity(const Quest& quest, const std::string& cityId)
{
    return quest.bountyCurrentCityId.empty() || quest.bountyCurrentCityId == cityId;
}

std::string BountyRelocationSystem::locationSummary(const Quest& quest)
{
    if (quest.bountyCurrentCityId.empty()) return quest.location;
    const std::string name = cityName(quest.bountyCurrentCityId);
    if (quest.bountyCurrentBiome.empty()) return name;
    return name + " — " + quest.bountyCurrentBiome;
}

std::string BountyRelocationSystem::transitTrailSummary(const Quest& quest)
{
    if (!quest.bountyTransitTrailAvailable || quest.bountyTransitTrailKind.empty()) return "";
    return trailLabel(quest.bountyTransitTrailKind) + " depuis " + cityName(quest.bountyTransitTrailFromCityId)
        + " | fiabilité estimée " + std::to_string(std::clamp(quest.bountyTransitTrailReliability, 0, 100)) + "/100";
}

BountyTransitTrailUseResult BountyRelocationSystem::useTransitTrail(Quest& quest, int investigatorBias, int roll1To100)
{
    BountyTransitTrailUseResult result;
    if (!quest.bountyTransitTrailAvailable || quest.bountyTransitTrailKind.empty()) return result;

    result.evaluated = true;
    quest.bountyTransitTrailAvailable = false; // one-shot lead: once checked, it becomes old information.

    int chance = std::clamp(quest.bountyTransitTrailReliability + investigatorBias, 18, 95);
    if (quest.bountyTransitTrailKind == "planted_false_lead") chance = std::clamp(chance - 18, 12, 62);
    const int roll = std::clamp(roll1To100, 1, 100);

    if (roll <= chance)
    {
        result.useful = true;
        result.cluesGained = (quest.bountyTransitTrailReliability >= 82 && roll <= chance / 2) ? 2 : 1;
        result.lines.push_back("La trace inter-ville tient assez bien : elle recoupe des horaires, témoins ou achats locaux et raccourcit l'enquête.");
    }
    else
    {
        result.noiseAdded = quest.bountyTransitTrailKind == "planted_false_lead" ? 9 : 3;
        result.lines.push_back(quest.bountyTransitTrailKind == "planted_false_lead"
            ? "La piste était largement préparée pour faire perdre du temps : tu l'écartes, mais quelques questions ont déjà circulé."
            : "La trace s'arrête sur des informations trop vieilles ou contradictoires. Elle est désormais épuisée.");
    }
    return result;
}
