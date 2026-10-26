#include "world/bounty/BountyTradeTransitSystem.hpp"

#include "quest/Quest.hpp"
#include "world/CityEconomyDynamicsSystem.hpp"
#include "world/CityTradeFlowSystem.hpp"

#include <algorithm>
#include <array>
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

    int goodsPriority(ShopType type)
    {
        switch (type)
        {
            case ShopType::Transport: return 8;
            case ShopType::Material: return 6;
            case ShopType::Consumable: return 5;
            case ShopType::Weapon: return 4;
            case ShopType::Armor: return 4;
            case ShopType::Plant: return 3;
            case ShopType::Blacksmith: return 2;
            default: return 0;
        }
    }
}

BountyTradeTransitOpportunity BountyTradeTransitSystem::assess(
    const Quest& quest,
    const std::string& sourceCityId,
    const std::string& destinationCityId,
    int worldDay)
{
    BountyTradeTransitOpportunity best;
    if (sourceCityId.empty() || destinationCityId.empty() || sourceCityId == destinationCityId)
    {
        return best;
    }

    const CityEconomicPulse sourcePulse = CityEconomyDynamicsSystem::stateFor(sourceCityId, worldDay, 0, 0);
    const CityEconomicPulse destinationPulse = CityEconomyDynamicsSystem::stateFor(destinationCityId, worldDay, 0, 0);

    static const std::array<ShopType, 7> candidateTypes = {
        ShopType::Transport,
        ShopType::Material,
        ShopType::Consumable,
        ShopType::Weapon,
        ShopType::Armor,
        ShopType::Plant,
        ShopType::Blacksmith
    };

    int bestScore = -1;
    CityTradeRouteActivity bestRoute;
    for (ShopType type : candidateTypes)
    {
        const CityTradeRouteActivity route = CityTradeFlowSystem::routeBetween(
            sourceCityId,
            destinationCityId,
            worldDay,
            type,
            sourcePulse.eventId,
            destinationPulse.eventId);

        if (!route.supported || (!route.departing && !route.arriving && !route.disrupted)) continue;

        int score = goodsPriority(type);
        if (route.departing || route.arriving) score += 22;
        if (route.disrupted) score += 17;
        if (route.emergencyPriority) score += 2;
        if (route.incidentActive) score += 5 + route.incidentSeverity;
        if (route.incidentEscortOpportunity) score -= 4; // more guards make discreet travel less attractive.
        if (route.incidentCreatesBountyLead) score += 2; // tempting chaos, but it will leave evidence.

        if (score > bestScore)
        {
            bestScore = score;
            bestRoute = route;
        }
    }

    if (bestScore < 0) return best;

    best.available = true;
    best.activeConvoy = bestRoute.departing || bestRoute.arriving;
    best.disruptedConvoy = bestRoute.disrupted;
    best.goodsType = bestRoute.goodsType;
    best.goodsLabel = shopTypeToText(bestRoute.goodsType);

    const std::uint32_t h = stableHash(
        quest.id + "|trade_escape|" + sourceCityId + "|" + destinationCityId + "|"
        + std::to_string(std::max(0, worldDay)) + "|" + std::to_string(static_cast<int>(bestRoute.goodsType)));
    const int bucket = static_cast<int>(h % 100u);
    const int reliabilityVariation = static_cast<int>((h / 100u) % 9u);

    if (bestRoute.incidentActive && bestRoute.incidentHeadline == "cargaison partiellement volée"
        && quest.bountyTargetResources <= 48 && quest.bountyDangerRating >= 3)
    {
        best.destinationScoreBonus = 9;
        best.relocationChanceBonus = 5;
        best.trailKind = "stolen_trade_cargo";
        best.trailReliability = 84 + reliabilityVariation;
        best.summary = "La cible semble avoir profité d'un incident commercial déjà réel sur cette route. Une cargaison manque et les témoins savent qu'il s'est passé quelque chose : la fuite gagne en mobilité, mais la piste laissée est particulièrement visible.";
        best.trailReliability = std::clamp(best.trailReliability, 25, 92);
        return best;
    }

    if (bestRoute.incidentEscortOpportunity)
    {
        best.destinationScoreBonus = 3;
        best.relocationChanceBonus = 1;
        best.trailKind = "merchant_route_witness";
        best.trailReliability = 78 + reliabilityVariation;
        best.summary = "Le convoi cherche justement des gardes supplémentaires. La cible peut encore profiter du trafic, mais la présence de nouveaux convoyeurs rend le passage beaucoup plus facile à remarquer.";
        best.trailReliability = std::clamp(best.trailReliability, 25, 90);
        return best;
    }

    if (bestRoute.disrupted)
    {
        best.destinationScoreBonus = 7;
        best.relocationChanceBonus = 4;
        best.trailKind = "delayed_convoy_camp";
        best.trailReliability = 62 + reliabilityVariation;
        best.summary = "La cible profite d'un convoi retardé : les voyageurs, bêtes de somme et cargaisons immobilisées créent assez de désordre pour masquer une partie de son passage.";
        return best;
    }

    if (quest.bountyTargetResources >= 62 && quest.bountyTargetKnowsHunt && bucket < 34)
    {
        best.destinationScoreBonus = 12;
        best.relocationChanceBonus = 9;
        best.trailKind = "bribed_convoy_driver";
        best.trailReliability = 44 + reliabilityVariation;
        best.summary = "La cible semble avoir payé un conducteur ou un convoyeur pour voyager sous une identité discrète. Le convoi aide sa fuite, mais quelqu'un a forcément vu ou encaissé quelque chose.";
    }
    else if (quest.bountyTargetResources <= 38 && quest.bountyDangerRating >= 3 && bucket < 28)
    {
        best.destinationScoreBonus = 8;
        best.relocationChanceBonus = 5;
        best.trailKind = "stolen_trade_cargo";
        best.trailReliability = 80 + reliabilityVariation;
        best.summary = "Faute de moyens propres, la cible semble avoir volé des provisions ou une place dans une cargaison commerciale. La fuite gagne en mobilité, mais le vol laisse une trace assez voyante chez les marchands.";
    }
    else if (quest.bountyTargetMobility >= 62)
    {
        best.destinationScoreBonus = 10;
        best.relocationChanceBonus = 7;
        best.trailKind = "hidden_in_merchant_convoy";
        best.trailReliability = 68 + reliabilityVariation;
        best.summary = "La cible exploite un convoi commercial réel entre les deux villes pour se fondre parmi les voyageurs, gardes et marchandises pendant le trajet.";
    }
    else
    {
        best.destinationScoreBonus = 6;
        best.relocationChanceBonus = 3;
        best.trailKind = "merchant_route_witness";
        best.trailReliability = 72 + reliabilityVariation;
        best.summary = "Le trafic marchand offre une route praticable à la cible, mais plusieurs marchands ou conducteurs peuvent ensuite se souvenir de son passage.";
    }

    best.trailReliability = std::clamp(best.trailReliability, 25, 90);
    return best;
}
