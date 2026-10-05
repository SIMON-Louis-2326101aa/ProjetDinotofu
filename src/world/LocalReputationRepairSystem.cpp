#include "world/LocalReputationRepairSystem.hpp"
#include "world/LocalReputationSystem.hpp"
#include "entity/Player.hpp"
#include "economy/Money.hpp"

#include <algorithm>

namespace
{
std::string serviceKey(const Player& player, const std::string& cityId)
{
    return cityId + ":day:" + std::to_string(player.getWorldDaysElapsed());
}

int localJournalCount(const Player& player, const std::string& category, const std::string& key, const std::string& cityId)
{
    int total = 0;
    for (const PlayerJournalRecord& record : player.getCanonicalJournalRecords())
    {
        if (record.category == category && record.key == key && record.locationId == cityId)
        {
            total += std::max(0, record.count);
        }
    }
    return total;
}
}

int LocalReputationRepairSystem::fineCostEconomyUnits(int reputationScore)
{
    if (reputationScore >= 0) return 0;
    const int debt = std::min(80, -reputationScore);
    return std::max(15, 8 + debt * 3);
}

int LocalReputationRepairSystem::fineReputationGain(int reputationScore)
{
    if (reputationScore >= 0) return 0;
    const int debt = -reputationScore;
    return std::min(debt, std::max(6, debt / 2));
}

bool LocalReputationRepairSystem::canPayFine(const Player& player, const std::string& cityId)
{
    return !cityId.empty()
        && cityId == player.getCurrentCityId()
        && LocalReputationSystem::score(player, cityId) < 0
        && player.getInventory().getEconomyUnits() >= fineCostEconomyUnits(LocalReputationSystem::score(player, cityId));
}

bool LocalReputationRepairSystem::canPerformCommunityService(const Player& player, const std::string& cityId)
{
    if (cityId.empty() || cityId != player.getCurrentCityId() || LocalReputationSystem::score(player, cityId) >= 0) return false;
    return localJournalCount(player, "rehabilitations_locales", serviceKey(player, cityId), cityId) == 0;
}

std::vector<std::string> LocalReputationRepairSystem::payFine(Player& player, const std::string& cityId)
{
    const int before = LocalReputationSystem::score(player, cityId);
    if (before >= 0)
    {
        return {"Aucune dette de réputation locale ne justifie une amende de réparation ici."};
    }
    if (cityId != player.getCurrentCityId())
    {
        return {"La médiation doit être réglée dans la ville concernée ; la réputation ne se répare pas à distance par magie."};
    }

    const int cost = fineCostEconomyUnits(before);
    const int gain = fineReputationGain(before);
    if (!player.getInventory().spendEconomyUnits(cost))
    {
        return {"Fonds insuffisants : l'amende proposée est de " + Money::formatEconomyUnits(cost) + "."};
    }

    player.recordCanonicalEvent("reputation_locale_positive", "amende_reparatrice", "Amende réparatrice réglée", gain);
    player.recordCanonicalEvent("rehabilitations_locales", "amende_reparatrice", "Réparation locale par amende", 1);
    player.recordHistoricalEvent("local_reputation_repair", cityId, "Amende réparatrice réglée à " + cityId, true);
    const int after = LocalReputationSystem::score(player, cityId);
    return {
        "Amende réglée : -" + Money::formatEconomyUnits(cost) + ".",
        "Réparation reconnue : +" + std::to_string(gain) + " de réputation locale.",
        "Réputation : " + std::to_string(before) + " -> " + std::to_string(after) + ".",
        after < 0
            ? "La ville reste méfiante : payer ne supprime pas d'un coup toutes les conséquences de tes actes."
            : "La dette sociale immédiate est suffisamment réparée pour revenir au moins à une situation neutre."
    };
}

std::vector<std::string> LocalReputationRepairSystem::performCommunityService(Player& player, const std::string& cityId)
{
    const int before = LocalReputationSystem::score(player, cityId);
    if (before >= 0)
    {
        return {"Le service de réhabilitation est réservé aux personnages qui ont réellement quelque chose à réparer dans cette ville."};
    }
    if (cityId != player.getCurrentCityId())
    {
        return {"Tu dois être présent dans la ville concernée pour effectuer un service communautaire."};
    }
    const std::string key = serviceKey(player, cityId);
    if (localJournalCount(player, "rehabilitations_locales", key, cityId) > 0)
    {
        return {"Tu as déjà effectué un service de réparation aujourd'hui ici. La ville attend des actes dans la durée, pas une boucle infinie."};
    }

    player.advanceWorldDayUnits(2);
    player.recordCanonicalEvent("services_locaux_reussis", "service_rehabilitation", "Service communautaire de réparation", 1);
    player.recordCanonicalEvent("rehabilitations_locales", key, "Service de réparation du jour", 1);
    player.recordHistoricalEvent("local_reputation_repair", cityId, "Service communautaire accompli à " + cityId, true);
    const int after = LocalReputationSystem::score(player, cityId);
    return {
        "Tu consacres deux segments de journée à un travail visible : manutention, réparation légère, registre ou nettoyage d'un espace public.",
        "Le service vaut +3 de réputation locale parce qu'il est réellement accompli et prend du temps.",
        "Réputation : " + std::to_string(before) + " -> " + std::to_string(after) + ".",
        "Limite : un service de réhabilitation par jour et par ville. Une mauvaise réputation ne se gomme pas en spammant un menu."
    };
}
