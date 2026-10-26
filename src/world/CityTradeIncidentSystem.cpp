#include "world/CityTradeIncidentSystem.hpp"

#include <algorithm>
#include <cstdint>

namespace
{
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

    bool supportsPhysicalIncident(ShopType type)
    {
        return type == ShopType::Material
            || type == ShopType::Plant
            || type == ShopType::Consumable
            || type == ShopType::Weapon
            || type == ShopType::Armor
            || type == ShopType::MonsterMaterial
            || type == ShopType::Transport
            || type == ShopType::Blacksmith
            || type == ShopType::Alchemist;
    }

    int severityFrom(std::uint32_t h, int distance, bool emergencyPriority)
    {
        int severity = 1 + static_cast<int>((h / 1000u) % 3u);
        if (distance >= 80 && severity < 3) ++severity;
        if (emergencyPriority && severity > 1) --severity; // priority convoys receive more guards.
        return std::clamp(severity, 1, 3);
    }
}

std::string CityTradeIncidentSystem::kindId(CityTradeIncidentKind kind)
{
    switch (kind)
    {
        case CityTradeIncidentKind::EscortRequest: return "escort_request";
        case CityTradeIncidentKind::CargoTheft: return "cargo_theft";
        case CityTradeIncidentKind::MissingConvoy: return "missing_convoy";
        case CityTradeIncidentKind::DamagedArrival: return "damaged_arrival";
        case CityTradeIncidentKind::RoadObstruction: return "road_obstruction";
        default: return "none";
    }
}

CityTradeIncident CityTradeIncidentSystem::evaluate(
    const std::string& sourceCityId,
    const std::string& destinationCityId,
    int worldDay,
    ShopType goodsType,
    int distance,
    bool departing,
    bool arriving,
    bool disrupted,
    bool emergencyPriority)
{
    CityTradeIncident result;
    if (sourceCityId.empty() || destinationCityId.empty() || sourceCityId == destinationCityId) return result;
    if (!supportsPhysicalIncident(goodsType)) return result;
    if (!departing && !arriving && !disrupted) return result;

    const int safeDay = std::max(0, worldDay);
    // Incidents are tied to a route window/cycle, not rerolled by repeatedly opening a menu.
    const int cycleBucket = safeDay / 6;
    const std::string key = sourceCityId + "|" + destinationCityId + "|" + std::to_string(static_cast<int>(goodsType))
        + "|cycle=" + std::to_string(cycleBucket);
    const std::uint32_t h = stableHash(key);

    // Normal active traffic is quiet most of the time. An already disrupted convoy has more ways
    // to turn into a visible incident, but even then an ordinary delay can remain just a delay.
    const int chance = disrupted ? 44 : (emergencyPriority ? 9 : 13);
    if (static_cast<int>(h % 100u) >= chance) return result;

    result.active = true;
    result.severity = severityFrom(h, distance, emergencyPriority);
    const int selector = static_cast<int>((h / 100u) % 100u);

    if (disrupted && selector < 32)
    {
        result.kind = CityTradeIncidentKind::MissingConvoy;
        result.headline = "convoi introuvable";
        result.summary = "Le convoi n'a pas rejoint le relais attendu. Les registres parlent d'un retard, mais personne ne confirme encore où se trouvent les conducteurs et la cargaison.";
        result.destinationStockModifierPercent = -(5 + result.severity * 3);
        result.createsBountyLead = result.severity >= 2;
    }
    else if ((disrupted && selector < 60) || (!disrupted && selector < 18))
    {
        result.kind = CityTradeIncidentKind::CargoTheft;
        result.headline = "cargaison partiellement volée";
        result.summary = "Une partie de la cargaison manque à l'appel. Les traces sont assez nettes pour que les marchands parlent déjà d'un vol plutôt que d'une simple erreur de registre.";
        result.destinationStockModifierPercent = -(4 + result.severity * 3);
        result.createsBountyLead = true;
    }
    else if (arriving && selector < 76)
    {
        result.kind = CityTradeIncidentKind::DamagedArrival;
        result.headline = "arrivée endommagée";
        result.summary = "Le convoi atteint la ville, mais plusieurs caisses, roues ou protections ont souffert pendant le trajet. Une partie des marchandises reste exploitable, une autre part au tri ou en réparation.";
        result.destinationStockModifierPercent = -(2 + result.severity * 2);
    }
    else if (disrupted || selector < 89)
    {
        result.kind = CityTradeIncidentKind::RoadObstruction;
        result.headline = "route temporairement obstruée";
        result.summary = "Un passage dégradé, un pont encombré ou un incident de terrain ralentit la colonne. Ce n'est pas forcément criminel, mais la livraison perd du temps et de la capacité utile.";
        result.destinationStockModifierPercent = -(1 + result.severity * 2);
    }
    else
    {
        result.kind = CityTradeIncidentKind::EscortRequest;
        result.headline = "renfort d'escorte recherché";
        result.summary = "Les convoyeurs préfèrent payer quelques bras de plus avant le départ plutôt que transformer un trajet normal en pari inutile. La cargaison n'est pas encore perdue.";
        result.destinationStockModifierPercent = 0;
        result.escortOpportunity = true;
    }

    // Priority convoys are more heavily guarded: incidents may still happen, but the economic
    // damage is softened. Escort requests remain useful instead of becoming a hidden stock penalty.
    if (emergencyPriority && result.destinationStockModifierPercent < 0)
    {
        result.destinationStockModifierPercent += std::min(-result.destinationStockModifierPercent, 2);
    }

    result.id = kindId(result.kind) + "|" + sourceCityId + "|" + destinationCityId + "|"
        + std::to_string(static_cast<int>(goodsType)) + "|" + std::to_string(cycleBucket);
    return result;
}
