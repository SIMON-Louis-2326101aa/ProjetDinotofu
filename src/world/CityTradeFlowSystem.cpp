#include "world/CityTradeFlowSystem.hpp"

#include "world/City.hpp"
#include "world/CityProfileSystem.hpp"
#include "world/CityTradeIncidentSystem.hpp"

#include <algorithm>
#include <cstdint>
#include <limits>

namespace
{
    int specializationStrength(const CityGameplayProfile& profile, ShopType type)
    {
        for (const CityEconomicSpecialization& specialization : profile.specializations)
        {
            if (specialization.shopType == type)
            {
                return std::clamp(specialization.strength, 0, 3);
            }
        }
        return 0;
    }

    std::uint32_t stableHash(const std::string& text)
    {
        std::uint32_t hash = 2166136261u;
        for (unsigned char c : text)
        {
            hash ^= c;
            hash *= 16777619u;
        }
        return hash;
    }

    int routeRisk(CitySecurityTier tier)
    {
        switch (tier)
        {
            case CitySecurityTier::HighlySecured: return 2;
            case CitySecurityTier::Watched: return 5;
            case CitySecurityTier::NeutralIndependent: return 9;
            case CitySecurityTier::Unstable: return 16;
            case CitySecurityTier::ChaoticAnarchic: return 24;
        }
        return 10;
    }

    bool isCriticalDuringRaid(ShopType type)
    {
        return type == ShopType::Material
            || type == ShopType::Weapon
            || type == ShopType::Armor
            || type == ShopType::Blacksmith
            || type == ShopType::Consumable;
    }

    bool isCriticalDuringReconstruction(ShopType type)
    {
        return type == ShopType::Material
            || type == ShopType::Blacksmith
            || type == ShopType::Plant
            || type == ShopType::Consumable;
    }

    bool isRaidEvent(const std::string& id)
    {
        return id == "raid_defense_preparation" || id == "raid_defense_mobilization";
    }

    int cityDistance(const std::string& a, const std::string& b)
    {
        const City* from = City::findById(a);
        const City* to = City::findById(b);
        if (!from || !to) return 999;
        return City::calculateDistanceBetween(*from, *to);
    }

    const City* strongestSupplier(const std::string& destinationId, ShopType type)
    {
        const City* best = nullptr;
        int bestScore = std::numeric_limits<int>::min();
        for (const City& city : City::getCatalog())
        {
            if (city.getId() == destinationId) continue;
            const CityGameplayProfile profile = CityProfileSystem::profileFor(city.getId());
            const int strength = specializationStrength(profile, type);
            if (strength <= 0) continue;
            const int distance = cityDistance(city.getId(), destinationId);
            const int score = strength * 100 - std::min(distance, 300);
            if (!best || score > bestScore || (score == bestScore && city.getId() < best->getId()))
            {
                best = &city;
                bestScore = score;
            }
        }
        return best;
    }

    const City* bestDestination(const std::string& sourceId, ShopType type)
    {
        const CityGameplayProfile sourceProfile = CityProfileSystem::profileFor(sourceId);
        const int sourceStrength = specializationStrength(sourceProfile, type);
        if (sourceStrength < 2) return nullptr;

        const City* best = nullptr;
        int bestScore = std::numeric_limits<int>::min();
        for (const City& city : City::getCatalog())
        {
            if (city.getId() == sourceId) continue;
            const int destinationStrength = specializationStrength(CityProfileSystem::profileFor(city.getId()), type);
            if (destinationStrength >= sourceStrength) continue;
            const int distance = cityDistance(sourceId, city.getId());
            const int need = (sourceStrength - destinationStrength) * 90;
            const int score = need - std::min(distance, 300);
            if (!best || score > bestScore || (score == bestScore && city.getId() < best->getId()))
            {
                best = &city;
                bestScore = score;
            }
        }
        return best;
    }

    int routePeriod(int distance)
    {
        if (distance >= 80) return 8;
        if (distance >= 55) return 7;
        return 6;
    }

    int routePhase(const std::string& sourceId, const std::string& destinationId, ShopType type, int worldDay, int period)
    {
        const std::string key = sourceId + "|" + destinationId + "|" + std::to_string(static_cast<int>(type));
        const int offset = static_cast<int>(stableHash(key) % static_cast<std::uint32_t>(period));
        const int day = std::max(0, worldDay);
        return (day + offset) % period;
    }

    bool routeDisrupted(
        const std::string& sourceId,
        const std::string& destinationId,
        ShopType type,
        int worldDay,
        int period,
        int distance)
    {
        const CityGameplayProfile sourceProfile = CityProfileSystem::profileFor(sourceId);
        const CityGameplayProfile destinationProfile = CityProfileSystem::profileFor(destinationId);
        int risk = 3 + distance / 8;
        risk += routeRisk(sourceProfile.security) / 2;
        risk += routeRisk(destinationProfile.security) / 2;
        risk = std::clamp(risk, 5, 34);

        const int cycle = std::max(0, worldDay) / std::max(1, period);
        const std::string key = sourceId + "|" + destinationId + "|" + std::to_string(static_cast<int>(type)) + "|" + std::to_string(cycle);
        return static_cast<int>(stableHash(key) % 100u) < risk;
    }

    std::string cityName(const std::string& cityId)
    {
        if (const City* city = City::findById(cityId)) return city->getName();
        return cityId;
    }

    std::vector<ShopType> visibleTradeTypes()
    {
        return {
            ShopType::Material,
            ShopType::Plant,
            ShopType::Consumable,
            ShopType::Weapon,
            ShopType::Armor,
            ShopType::Blacksmith,
            ShopType::Alchemist,
            ShopType::Enchanter,
            ShopType::Transport,
            ShopType::Lodging,
            ShopType::CityService
        };
    }
}

CityTradeRouteActivity CityTradeFlowSystem::routeBetween(
    const std::string& sourceCityId,
    const std::string& destinationCityId,
    int worldDay,
    ShopType goodsType,
    const std::string& sourceEventId,
    const std::string& destinationEventId)
{
    CityTradeRouteActivity result;
    result.goodsType = goodsType;
    result.sourceCityId = sourceCityId;
    result.sourceCityName = cityName(sourceCityId);
    result.destinationCityId = destinationCityId;
    result.destinationCityName = cityName(destinationCityId);

    if (sourceCityId.empty() || destinationCityId.empty() || sourceCityId == destinationCityId)
    {
        return result;
    }

    const City* source = City::findById(sourceCityId);
    const City* destination = City::findById(destinationCityId);
    if (source == nullptr || destination == nullptr) return result;

    // A route only exists when the source is a plausible supplier for this destination in the
    // same deterministic trade network used by pressureFor(). This prevents other systems from
    // inventing a convenient caravan merely because two cities exist.
    const City* supplier = strongestSupplier(destinationCityId, goodsType);
    const City* favoredDestination = bestDestination(sourceCityId, goodsType);
    const bool supplierMatch = supplier != nullptr && supplier->getId() == sourceCityId;
    const bool destinationMatch = favoredDestination != nullptr && favoredDestination->getId() == destinationCityId;
    if (!supplierMatch && !destinationMatch) return result;

    result.supported = true;
    result.distance = cityDistance(sourceCityId, destinationCityId);
    const int period = routePeriod(result.distance);
    const int phase = routePhase(sourceCityId, destinationCityId, goodsType, worldDay, period);
    const bool disrupted = routeDisrupted(sourceCityId, destinationCityId, goodsType, worldDay, period, result.distance);

    const bool sourceCriticalNeed = (isRaidEvent(sourceEventId) && isCriticalDuringRaid(goodsType))
        || (sourceEventId == "reconstruction" && isCriticalDuringReconstruction(goodsType));
    const bool destinationCriticalNeed = (isRaidEvent(destinationEventId) && isCriticalDuringRaid(goodsType))
        || (destinationEventId == "reconstruction" && isCriticalDuringReconstruction(goodsType));

    result.emergencyPriority = destinationCriticalNeed;
    result.disrupted = disrupted && (phase == 0 || phase == 1 || phase == period - 1);
    result.departing = !sourceCriticalNeed && !disrupted && (phase == period - 1 || phase == 0);
    result.arriving = !sourceCriticalNeed && !disrupted && (phase == 0 || phase == 1);

    if (sourceCriticalNeed)
    {
        result.status = "exportation suspendue : " + result.sourceCityName + " conserve cette ressource pour ses propres besoins";
    }
    else if (result.disrupted)
    {
        result.status = "convoi entre " + result.sourceCityName + " et " + result.destinationCityName + " retardé ou immobilisé";
    }
    else if (result.departing && result.arriving)
    {
        result.status = "convoi commercial actif entre " + result.sourceCityName + " et " + result.destinationCityName;
    }
    else if (result.departing)
    {
        result.status = "convoi en départ de " + result.sourceCityName + " vers " + result.destinationCityName;
    }
    else if (result.arriving)
    {
        result.status = "convoi en arrivée à " + result.destinationCityName + " depuis " + result.sourceCityName;
    }
    else
    {
        result.status = "route commerciale régulière entre " + result.sourceCityName + " et " + result.destinationCityName;
    }

    if (result.emergencyPriority && (result.departing || result.arriving))
    {
        result.status += " avec priorité liée aux besoins locaux";
    }

    const CityTradeIncident incident = CityTradeIncidentSystem::evaluate(
        sourceCityId, destinationCityId, worldDay, goodsType, result.distance,
        result.departing, result.arriving, result.disrupted, result.emergencyPriority);
    if (incident.active)
    {
        result.incidentActive = true;
        result.incidentId = incident.id;
        result.incidentHeadline = incident.headline;
        result.incidentSeverity = incident.severity;
        result.incidentStockModifierPercent = incident.destinationStockModifierPercent;
        result.incidentEscortOpportunity = incident.escortOpportunity;
        result.incidentCreatesBountyLead = incident.createsBountyLead;
        result.incidentSummary = incident.summary;
        result.status += " ; incident : " + incident.headline;
    }

    return result;
}

CityTradePressure CityTradeFlowSystem::pressureFor(
    const std::string& cityId,
    int worldDay,
    ShopType goodsType,
    const std::string& localEventId)
{
    CityTradePressure result;
    result.goodsType = goodsType;
    result.destinationCityId = cityId;
    result.destinationCityName = cityName(cityId);

    const CityGameplayProfile localProfile = CityProfileSystem::profileFor(cityId);
    const int localStrength = specializationStrength(localProfile, goodsType);

    const City* supplier = strongestSupplier(cityId, goodsType);
    if (supplier)
    {
        const CityGameplayProfile supplierProfile = CityProfileSystem::profileFor(supplier->getId());
        const int supplierStrength = specializationStrength(supplierProfile, goodsType);
        const int distance = cityDistance(supplier->getId(), cityId);
        const int period = routePeriod(distance);
        const int phase = routePhase(supplier->getId(), cityId, goodsType, worldDay, period);
        const bool disruption = routeDisrupted(supplier->getId(), cityId, goodsType, worldDay, period, distance);

        const bool importUseful = supplierStrength > localStrength || localStrength <= 1
            || localEventId == "merchant_fair" || localEventId == "merchant_fair_preparation"
            || localEventId == "reconstruction" || isRaidEvent(localEventId);

        if (importUseful)
        {
            result.sourceCityId = supplier->getId();
            result.sourceCityName = supplier->getName();
            result.distance = distance;
            result.disrupted = disruption && (phase == 0 || phase == 1);

            if (phase == 0 || phase == 1)
            {
                result.incomingConvoy = true;
                if (result.disrupted)
                {
                    // A city dependent on imports really feels a delayed convoy. A specialized city
                    // can absorb the same delay from its own production and does not get the extra penalty.
                    result.importStockModifierPercent = localStrength == 0 ? -5 : 0;
                    result.status = "convoi attendu depuis " + supplier->getName() + " retardé sur la route";
                }
                else
                {
                    int bonus = phase == 0 ? (7 + supplierStrength * 2) : (4 + supplierStrength);
                    if (localEventId == "merchant_fair" || localEventId == "merchant_fair_preparation") bonus += 4;
                    if (isRaidEvent(localEventId) && isCriticalDuringRaid(goodsType))
                    {
                        bonus += 3;
                        result.emergencyPriority = true;
                    }
                    if (localEventId == "reconstruction" && isCriticalDuringReconstruction(goodsType))
                    {
                        bonus += 3;
                        result.emergencyPriority = true;
                    }
                    result.importStockModifierPercent = std::clamp(bonus, 0, 18);
                    result.status = "arrivage depuis " + supplier->getName();
                    if (result.emergencyPriority) result.status += " prioritaire pour les besoins locaux";
                }
            }
        }
    }

    const City* destination = bestDestination(cityId, goodsType);
    if (destination)
    {
        const int distance = cityDistance(cityId, destination->getId());
        const int period = routePeriod(distance);
        const int phase = routePhase(cityId, destination->getId(), goodsType, worldDay, period);
        const bool criticalLocalNeed = (isRaidEvent(localEventId) && isCriticalDuringRaid(goodsType))
            || (localEventId == "reconstruction" && isCriticalDuringReconstruction(goodsType));
        const bool disrupted = routeDisrupted(cityId, destination->getId(), goodsType, worldDay, period, distance);

        // Goods leave public stocks the day before / day of departure. Emergency local demand suspends exports.
        if (!criticalLocalNeed && (phase == period - 1 || phase == 0) && !disrupted)
        {
            result.outgoingConvoy = true;
            const int pressure = 2 + localStrength * 2;
            result.exportStockModifierPercent = -std::clamp(pressure, 0, 8);
            if (result.status.empty())
            {
                result.status = "expédition vers " + destination->getName();
            }
            else
            {
                result.status += " ; expédition simultanée vers " + destination->getName();
            }
        }
    }

    // A rare route incident is evaluated from the exact same deterministic route. Incoming
    // incidents can reduce what actually reaches the destination; outbound incidents do not
    // remove stock twice because the exported goods already left the public shelves.
    if (supplier)
    {
        const CityTradeRouteActivity route = routeBetween(
            supplier->getId(), cityId, worldDay, goodsType, "", localEventId);
        if (route.incidentActive && (route.arriving || route.disrupted))
        {
            result.incidentActive = true;
            result.incidentId = route.incidentId;
            result.incidentHeadline = route.incidentHeadline;
            result.incidentSeverity = route.incidentSeverity;
            result.incidentStockModifierPercent = route.incidentStockModifierPercent;
            result.incidentEscortOpportunity = route.incidentEscortOpportunity;
            result.incidentCreatesBountyLead = route.incidentCreatesBountyLead;
            result.importStockModifierPercent += route.incidentStockModifierPercent;
            if (!route.incidentHeadline.empty())
            {
                if (!result.status.empty()) result.status += " ; ";
                result.status += "incident : " + route.incidentHeadline;
            }
        }
    }

    if (destination && !result.incidentActive)
    {
        const CityTradeRouteActivity route = routeBetween(
            cityId, destination->getId(), worldDay, goodsType, localEventId, "");
        if (route.incidentActive && (route.departing || route.disrupted))
        {
            result.incidentActive = true;
            result.incidentId = route.incidentId;
            result.incidentHeadline = route.incidentHeadline;
            result.incidentSeverity = route.incidentSeverity;
            result.incidentStockModifierPercent = 0;
            result.incidentEscortOpportunity = route.incidentEscortOpportunity;
            result.incidentCreatesBountyLead = route.incidentCreatesBountyLead;
            if (!route.incidentHeadline.empty())
            {
                if (!result.status.empty()) result.status += " ; ";
                result.status += "incident sur le départ : " + route.incidentHeadline;
            }
        }
    }

    result.netStockModifierPercent = std::clamp(
        result.importStockModifierPercent + result.exportStockModifierPercent,
        -18,
        20);
    return result;
}

std::vector<std::string> CityTradeFlowSystem::informationLines(
    const std::string& cityId,
    int worldDay,
    const std::string& localEventId)
{
    struct RankedLine
    {
        int importance = 0;
        std::string text;
    };

    std::vector<RankedLine> ranked;
    for (ShopType type : visibleTradeTypes())
    {
        const CityTradePressure pressure = pressureFor(cityId, worldDay, type, localEventId);
        if (!pressure.incomingConvoy && !pressure.outgoingConvoy && !pressure.disrupted) continue;

        std::string text = std::string("- ") + shopTypeToText(type) + " : " + pressure.status + ".";
        if (pressure.netStockModifierPercent > 0)
        {
            text += " Disponibilité publique renforcée d'environ +" + std::to_string(pressure.netStockModifierPercent) + "%.";
        }
        else if (pressure.netStockModifierPercent < 0)
        {
            text += " Disponibilité publique sous pression d'environ " + std::to_string(pressure.netStockModifierPercent) + "%.";
        }
        else if (pressure.disrupted)
        {
            text += " Les stocks spécialisés locaux amortissent pour l'instant le retard.";
        }
        if (pressure.incidentEscortOpportunity)
        {
            text += " Les convoyeurs cherchent ponctuellement du renfort pour sécuriser le trajet.";
        }
        if (pressure.incidentCreatesBountyLead)
        {
            text += " Les gardes et marchands conservent des témoignages exploitables si une enquête ou une prime suit l'affaire.";
        }

        int importance = std::abs(pressure.netStockModifierPercent) + (pressure.disrupted ? 8 : 0) + (pressure.emergencyPriority ? 4 : 0) + (pressure.incidentActive ? 12 + pressure.incidentSeverity * 2 : 0);
        ranked.push_back({importance, text});
    }

    std::sort(ranked.begin(), ranked.end(), [](const RankedLine& a, const RankedLine& b) {
        if (a.importance != b.importance) return a.importance > b.importance;
        return a.text < b.text;
    });

    std::vector<std::string> lines;
    const std::size_t count = std::min<std::size_t>(4, ranked.size());
    for (std::size_t i = 0; i < count; ++i) lines.push_back(ranked[i].text);
    return lines;
}
