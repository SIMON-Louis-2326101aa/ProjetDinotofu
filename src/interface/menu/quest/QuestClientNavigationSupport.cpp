// EN: QuestClientNavigationSupport.cpp centralizes client quest status/navigation rules.
// FR: QuestClientNavigationSupport.cpp centralise les règles d'état/navigation des quêtes PNJ.
#include "interface/menu/quest/QuestClientNavigationSupport.hpp"
#include "entity/Player.hpp"
#include "quest/Quest.hpp"

#include <algorithm>
#include <set>

namespace QuestClientNavigationSupport
{
    bool isMaterialDeliveryQuest(const Quest& quest)
    {
        return !quest.requiredMaterialId.empty() && quest.requiredMaterialQuantity > 0;
    }

    bool canCompleteMaterialDelivery(const Player& player, const Quest& quest)
    {
        return isMaterialDeliveryQuest(quest)
            && player.getInventory().countMaterialQualityPointsById(quest.requiredMaterialId) >= quest.requiredMaterialQuantity * 2;
    }

    bool hasRequiredQuestMaterial(const Player& player, const Quest& quest)
    {
        if (quest.requiredMaterialId.empty() || quest.requiredMaterialQuantity <= 0)
        {
            return true;
        }
        return player.getInventory().countMaterialQualityPointsById(quest.requiredMaterialId) >= quest.requiredMaterialQuantity * 2;
    }

    bool isReadyToTurnIn(const Player& player, const Quest& quest)
    {
        if (quest.failed || quest.turnedIn)
        {
            return false;
        }
        if (quest.objectiveType == "service")
        {
            return quest.completed && hasRequiredQuestMaterial(player, quest);
        }
        return quest.completed || canCompleteMaterialDelivery(player, quest);
    }

    namespace
    {
        std::string recommendationPrefix()
        {
            return "Client supplémentaire recommandé : ";
        }

        std::string extractRecommendedClientNameInternal(const Quest& quest)
        {
            const std::string prefix = recommendationPrefix();
            return quest.rewardNote.rfind(prefix, 0) == 0 ? quest.rewardNote.substr(prefix.size()) : "";
        }
    }


    std::string extractRecommendedClientName(const Quest& quest)
    {
        return extractRecommendedClientNameInternal(quest);
    }
    std::vector<std::string> defaultRecommendedClientNames()
    {
        return {
            "Mirette la couturière",
            "Noro le palefrenier",
            "Éliane du vieux pont",
            "Caldor le porteur de caisses",
            "Bruma la réparatrice de selles"
        };
    }

    std::vector<std::string> collectRecommendedClients(const Player& player)
    {
        std::vector<std::string> clients;
        std::set<std::string> seen;
        for (const Quest& quest : player.getQuestLog().getQuests())
        {
            if (!quest.turnedIn || quest.rewardMaterialId != "client_recommendation") continue;
            const std::string clientName = extractRecommendedClientName(quest);
            if (!clientName.empty() && player.getQuestLog().hasRecommendedClientCapacity(clientName) && seen.insert(clientName).second)
            {
                clients.push_back(clientName);
            }
        }

        const int looseRecommendations = player.getInventory().countMaterialById("client_recommendation");
        if (looseRecommendations <= 0) return clients;

        const std::vector<std::string> fallbackClients = defaultRecommendedClientNames();
        const int desiredVisibleCount = std::max(static_cast<int>(clients.size()), looseRecommendations);
        for (const std::string& clientName : fallbackClients)
        {
            if (static_cast<int>(clients.size()) >= desiredVisibleCount) break;
            if (player.getQuestLog().hasRecommendedClientCapacity(clientName) && seen.insert(clientName).second)
            {
                clients.push_back(clientName);
            }
        }
        return clients;
    }

    bool isRecommendedClientName(const std::string& clientName)
    {
        const auto names = defaultRecommendedClientNames();
        return std::find(names.begin(), names.end(), clientName) != names.end();
    }

    ClientQuestCounts countQuestsForClient(const Player& player, const std::string& clientName)
    {
        ClientQuestCounts counts;
        for (const Quest& quest : player.getQuestLog().getQuests())
        {
            if (quest.client != clientName) continue;
            ++counts.total;
            if (quest.turnedIn || quest.failed)
            {
                ++counts.turnedIn;
                continue;
            }
            ++counts.active;
            if (isReadyToTurnIn(player, quest)) ++counts.ready;
        }
        return counts;
    }

    std::string clientQuestStatusText(const ClientQuestCounts& counts)
    {
        if (counts.ready > 0)
        {
            return "À rendre : " + std::to_string(counts.ready)
                + " | En cours : " + std::to_string(counts.active)
                + " | Rendues : " + std::to_string(counts.turnedIn);
        }
        if (counts.active > 0)
        {
            return "En cours : " + std::to_string(counts.active)
                + " | Rendues : " + std::to_string(counts.turnedIn);
        }
        if (counts.turnedIn > 0) return "Aucune demande active | Rendues : " + std::to_string(counts.turnedIn);
        return "Aucune demande enregistrée";
    }

    std::string clientQuestHintText(const ClientQuestCounts& counts)
    {
        if (counts.ready > 0) return "Une demande peut être rendue ici.";
        if (counts.active > 0) return "Des demandes sont encore en cours.";
        return "Aucune demande active avec ce contact.";
    }

    MenuOptionItemData makeClientQuestNavigationItemData(
        const std::string& clientName,
        const std::string& section,
        const std::string& detail,
        const ClientQuestCounts& counts
    )
    {
        MenuOptionItemData itemData;
        itemData.structured = true;
        itemData.kind = "npc";
        itemData.section = section;
        itemData.actionType = counts.ready > 0 ? "turn_in" : "talk";
        itemData.name = clientName;
        itemData.detail = detail;
        itemData.status = clientQuestStatusText(counts);
        itemData.progress = counts.active > 0 ? "Demandes actives " + std::to_string(counts.active) : "Aucune demande active";
        itemData.owner = clientName;
        itemData.important = counts.ready > 0;
        return itemData;
    }
}
