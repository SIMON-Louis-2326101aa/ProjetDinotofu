#include "world/CityTradeIncidentMissionSystem.hpp"

#include "entity/Player.hpp"
#include "quest/QuestLog.hpp"
#include "world/City.hpp"
#include "world/CityTradeFlowSystem.hpp"

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

    std::vector<ShopType> incidentGoods()
    {
        return {ShopType::Material, ShopType::Plant, ShopType::Consumable, ShopType::Weapon,
                ShopType::Armor, ShopType::MonsterMaterial, ShopType::Transport,
                ShopType::Blacksmith, ShopType::Alchemist};
    }

    std::string rankForSeverity(int severity)
    {
        if (severity >= 3) return "D";
        if (severity == 2) return "E";
        return "F";
    }

    int rewardXpForSeverity(int severity)
    {
        return severity >= 3 ? 95 : (severity == 2 ? 58 : 32);
    }

    int rewardGoldForSeverity(int severity)
    {
        return severity >= 3 ? 75 : (severity == 2 ? 42 : 22);
    }

    bool sameIncidentAlreadyKnown(const Player& player, const std::string& incidentId)
    {
        for (const Quest& quest : player.getQuestLog().getQuests())
        {
            if (quest.tradeIncidentMission && quest.tradeIncidentId == incidentId && !quest.failed) return true;
        }
        return false;
    }

    Quest makeBounty(const Quest& incidentQuest)
    {
        Quest bounty;
        bounty.id = "trade_incident_bounty:" + incidentQuest.tradeIncidentId;
        bounty.rank = incidentQuest.tradeIncidentSeverity >= 3 ? "D" : "E";
        bounty.title = "Prime : responsables du convoi " + incidentQuest.tradeIncidentId;
        bounty.origin = "Guilde / enquête commerciale";
        bounty.client = "Maître de guilde";
        bounty.location = incidentQuest.tradeIncidentDestinationCityId;
        bounty.objective = "Identifier puis intercepter les responsables liés à l'incident commercial confirmé.";
        bounty.objectiveType = "bounty";
        bounty.targetFamily = "Réseau de voleurs de cargaison";
        bounty.rewardExperience = incidentQuest.tradeIncidentSeverity >= 3 ? 150 : 95;
        bounty.rewardGold = incidentQuest.tradeIncidentSeverity >= 3 ? 130 : 78;
        bounty.target = 1;
        bounty.guildQuest = true;
        bounty.accepted = false;
        bounty.bountyMission = true;
        bounty.bountyTargetName = "Responsables du convoi";
        bounty.bountyTargetTemperament = "mobile et opportuniste";
        bounty.bountyStage = "investigation";
        bounty.bountyDangerRating = std::clamp(incidentQuest.tradeIncidentSeverity + 1, 2, 5);
        bounty.bountyNotorietyRating = std::clamp(incidentQuest.tradeIncidentSeverity, 1, 4);
        bounty.bountyTargetResources = 35 + incidentQuest.tradeIncidentSeverity * 10;
        bounty.bountyTargetMobility = 55 + incidentQuest.tradeIncidentSeverity * 8;
        bounty.bountyCurrentCityId = incidentQuest.tradeIncidentDestinationCityId;
        bounty.bountyCurrentBiome = "Route commerciale";
        bounty.bountyTransitTrailKind = "trade_incident_witnesses";
        bounty.bountyTransitTrailFromCityId = incidentQuest.tradeIncidentSourceCityId;
        bounty.bountyTransitTrailReliability = 62 + incidentQuest.tradeIncidentSeverity * 8;
        bounty.bountyTransitTrailAvailable = true;
        return bounty;
    }
}

std::vector<CityTradeIncidentMissionOffer> CityTradeIncidentMissionSystem::offersFor(const Player& player)
{
    std::vector<CityTradeIncidentMissionOffer> offers;
    const std::string currentCityId = player.getCurrentCityId();
    const int day = player.getWorldDaysElapsed();
    if (currentCityId.empty()) return offers;

    for (const City& other : City::getCatalog())
    {
        if (other.getId() == currentCityId) continue;
        for (const ShopType goods : incidentGoods())
        {
            for (int direction = 0; direction < 2; ++direction)
            {
                const std::string source = direction == 0 ? other.getId() : currentCityId;
                const std::string destination = direction == 0 ? currentCityId : other.getId();
                const CityTradeRouteActivity route = CityTradeFlowSystem::routeBetween(source, destination, day, goods);
                if (!route.supported || !route.incidentActive) continue;
                if (sameIncidentAlreadyKnown(player, route.incidentId)) continue;

                CityTradeIncidentMissionOffer offer;
                offer.valid = true;
                offer.incidentId = route.incidentId;
                offer.incidentKind = route.incidentHeadline;
                offer.headline = route.incidentHeadline;
                offer.summary = route.incidentSummary;
                offer.sourceCityId = route.sourceCityId;
                offer.sourceCityName = route.sourceCityName;
                offer.destinationCityId = route.destinationCityId;
                offer.destinationCityName = route.destinationCityName;
                offer.goodsType = goods;
                offer.severity = route.incidentSeverity;
                offer.createsBountyLead = route.incidentCreatesBountyLead;
                offer.escortOpportunity = route.incidentEscortOpportunity;
                offers.push_back(offer);
                if (offers.size() >= 3) return offers;
            }
        }
    }
    return offers;
}

Quest CityTradeIncidentMissionSystem::buildQuest(const Player& player, const CityTradeIncidentMissionOffer& offer)
{
    Quest quest;
    if (!offer.valid) return quest;
    quest.id = "trade_incident_mission:" + offer.incidentId;
    quest.rank = rankForSeverity(offer.severity);
    quest.title = offer.escortOpportunity ? "Renfort de convoi : " + offer.headline : "Incident commercial : " + offer.headline;
    quest.origin = "Bureau d'information locale";
    quest.client = "Bureau de ville";
    quest.location = offer.sourceCityName + " -> " + offer.destinationCityName;
    quest.objective = offer.summary;
    quest.objectiveType = "service";
    quest.targetFamily = "Route commerciale";
    quest.rewardExperience = rewardXpForSeverity(offer.severity);
    quest.rewardGold = rewardGoldForSeverity(offer.severity);
    quest.target = 2; // preparation + route intervention, tracked by the dedicated stage.
    quest.progress = 0;
    quest.accepted = true;
    quest.availableFromDay = player.getWorldDaysElapsed();
    quest.expiresAtDay = player.getWorldDaysElapsed() + (offer.escortOpportunity ? 1 : 3);
    quest.tradeIncidentMission = true;
    quest.tradeIncidentId = offer.incidentId;
    quest.tradeIncidentKind = offer.incidentKind;
    quest.tradeIncidentSourceCityId = offer.sourceCityId;
    quest.tradeIncidentDestinationCityId = offer.destinationCityId;
    quest.tradeIncidentGoodsType = static_cast<int>(offer.goodsType);
    quest.tradeIncidentSeverity = offer.severity;
    quest.tradeIncidentStage = "preparation";
    quest.tradeIncidentCreatesBountyLead = offer.createsBountyLead;
    return quest;
}

bool CityTradeIncidentMissionSystem::prepare(Player& player, Quest& quest)
{
    if (!isActiveIncidentQuest(quest) || quest.tradeIncidentStage != "preparation") return false;
    player.advanceWorldDayUnits(1);
    quest.tradeIncidentStage = "action";
    quest.progress = std::max(quest.progress, 1);
    return true;
}

CityTradeIncidentMissionResult CityTradeIncidentMissionSystem::resolve(Player& player, Quest& quest, bool cautiousApproach)
{
    CityTradeIncidentMissionResult result;
    if (!isActiveIncidentQuest(quest) || quest.tradeIncidentStage != "action")
    {
        result.lines.push_back("L'intervention n'est pas prête : les préparatifs locaux doivent être terminés d'abord.");
        return result;
    }

    result.attempted = true;
    result.timeUnits = cautiousApproach ? 2 : 1;
    player.advanceWorldDayUnits(result.timeUnits);
    ++quest.tradeIncidentAttempts;

    int chance = 69 + std::min(12, player.getLevel() / 10);
    chance -= std::max(0, quest.tradeIncidentSeverity - 1) * 7;
    chance += cautiousApproach ? 14 : -4;
    result.chancePercent = std::clamp(chance, 35, 92);

    const std::uint32_t h = stableHash(quest.id + "|attempt=" + std::to_string(quest.tradeIncidentAttempts)
        + "|day=" + std::to_string(player.getWorldDaysElapsed()) + (cautiousApproach ? "|careful" : "|quick"));
    result.success = static_cast<int>(h % 100u) < result.chancePercent;
    result.lines.push_back(cautiousApproach
        ? "Tu prends le temps de recouper les traces, les conducteurs et l'état de la route avant d'intervenir."
        : "Tu interviens au plus vite pour gagner du temps, avec davantage de risque de rater un détail.");
    result.lines.push_back("Chance de réussite : " + std::to_string(result.chancePercent) + "%.");

    if (!result.success)
    {
        result.lines.push_back("L'intervention échoue : le temps passe réellement et aucune récompense n'est accordée.");
        return result;
    }

    quest.tradeIncidentStage = "resolved";
    quest.progress = quest.target;
    quest.completed = true;
    result.completed = true;
    result.lines.push_back("L'incident commercial est suffisamment résolu pour être rendu au Bureau de ville.");
    result.bountyPublished = publishBountyIfEligible(player, quest);
    if (result.bountyPublished)
    {
        result.lines.push_back("Les éléments recueillis suffisent pour publier une prime distincte au tableau de guilde.");
    }
    return result;
}

bool CityTradeIncidentMissionSystem::publishBountyIfEligible(Player& player, Quest& quest)
{
    if (!quest.tradeIncidentMission || !quest.completed || quest.tradeIncidentBountyPublished) return false;
    if (!quest.tradeIncidentCreatesBountyLead || quest.tradeIncidentSeverity < 2) return false;

    std::vector<Quest>& board = player.getQuestLog().getGuildBoardOffers();
    const std::string bountyId = "trade_incident_bounty:" + quest.tradeIncidentId;
    for (const Quest& existing : board)
    {
        if (existing.id == bountyId)
        {
            quest.tradeIncidentBountyPublished = true;
            return false;
        }
    }

    const Quest bounty = makeBounty(quest);
    const int targetSize = std::max(1, player.getQuestLog().getGuildBoardTargetSize());
    if (static_cast<int>(board.size()) >= targetSize)
    {
        auto replace = std::find_if(board.begin(), board.end(), [](const Quest& q) {
            return !q.bountyMission && !q.guildChallenge;
        });
        if (replace == board.end()) return false;
        *replace = bounty;
    }
    else
    {
        board.push_back(bounty);
    }
    quest.tradeIncidentBountyPublished = true;
    player.recordCanonicalEvent(
        "incidents_commerciaux_primes_publiees",
        quest.tradeIncidentId,
        "Une enquête d'incident commercial débouche sur une prime distincte",
        1
    );
    return true;
}

bool CityTradeIncidentMissionSystem::isActiveIncidentQuest(const Quest& quest)
{
    return quest.tradeIncidentMission && quest.accepted && !quest.completed && !quest.turnedIn && !quest.failed;
}
