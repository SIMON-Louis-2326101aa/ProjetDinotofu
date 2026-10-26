#include "world/crime/CrimeLocalOpportunitySystem.hpp"

#include "entity/Player.hpp"
#include "quest/Quest.hpp"

#include <algorithm>

namespace
{
    constexpr const char* Category = "crime_local_opportunity";

    bool splitSubject(const std::string& subject, std::string& cityId, std::string& kind, std::string& sourceId)
    {
        const std::size_t p1 = subject.find('|');
        if (p1 == std::string::npos || p1 == 0) return false;
        const std::size_t p2 = subject.find('|', p1 + 1);
        if (p2 == std::string::npos || p2 == p1 + 1) return false;
        cityId = subject.substr(0, p1);
        kind = subject.substr(p1 + 1, p2 - p1 - 1);
        sourceId = subject.substr(p2 + 1);
        return !cityId.empty() && !kind.empty();
    }

    std::string makeSubject(const std::string& cityId, const std::string& kind, const std::string& sourceId)
    {
        return cityId + "|" + kind + "|" + (sourceId.empty() ? std::string("local") : sourceId);
    }

    bool sameOpenKindExists(const Player& player, const std::string& cityId, const std::string& kind)
    {
        for (const PlayerHistoricalEvent& event : player.getHistoricalEvents())
        {
            if (event.category != Category || event.resolved) continue;
            std::string eventCity;
            std::string eventKind;
            std::string eventSource;
            if (!splitSubject(event.subjectId, eventCity, eventKind, eventSource)) continue;
            if (eventCity == cityId && eventKind == kind) return true;
        }
        return false;
    }

    const PlayerHistoricalEvent* findPendingEvent(const Player& player, const std::string& eventId)
    {
        for (const PlayerHistoricalEvent& event : player.getHistoricalEvents())
        {
            if (event.id == eventId && event.category == Category && !event.resolved) return &event;
        }
        return nullptr;
    }

    CrimeLocalOpportunityEntry buildEntry(const PlayerHistoricalEvent& event)
    {
        CrimeLocalOpportunityEntry entry;
        entry.eventId = event.id;
        entry.eventDay = event.day;
        if (!splitSubject(event.subjectId, entry.cityId, entry.kind, entry.sourceId)) return entry;

        if (entry.kind == "merchant_check")
        {
            entry.title = "Registres sous tension";
            entry.client = "Maître de guilde";
            entry.objective = "Comparer les signalements de marchands avec les mouvements suspects récents et isoler les incohérences sans accuser au hasard.";
            entry.lines.push_back("Plusieurs commerçants signalent des retards, des intermédiaires inhabituels ou des changements de prix difficiles à expliquer.");
            entry.lines.push_back("La guilde veut un contrôle court des registres, pas une accusation automatique contre le premier marchand louche.");
        }
        else if (entry.kind == "route_watch")
        {
            entry.title = "Relais sous surveillance";
            entry.client = "Maître de guilde";
            entry.objective = "Recouper les passages signalés autour des relais locaux et remettre un rapport exploitable sur les routes réellement perturbées.";
            entry.lines.push_back("Les relais de la ville commencent à modifier leurs habitudes à cause d'une tension criminelle visible.");
            entry.lines.push_back("Le travail demandé consiste à vérifier les mouvements connus, pas à inventer un nouveau réseau secret.");
        }
        else
        {
            entry.title = "Après le remous";
            entry.client = "Maître de guilde";
            entry.objective = "Recueillir des témoignages locaux sur les conséquences du dernier remous criminel et distinguer ce qui est observé de ce qui relève encore de la rumeur.";
            entry.lines.push_back("Une perturbation assez forte a laissé plusieurs versions contradictoires dans la ville.");
            entry.lines.push_back("La guilde cherche surtout à séparer les faits, les témoignages et les rumeurs avant qu'ils ne se mélangent complètement.");
        }
        return entry;
    }
}

bool CrimeLocalOpportunitySystem::queue(
    Player& player,
    const std::string& cityId,
    const std::string& kind,
    const std::string& sourceId,
    const std::string& reason)
{
    if (cityId.empty() || kind.empty() || sameOpenKindExists(player, cityId, kind)) return false;

    // Avoid creating a new service every day from the same kind of disturbance.
    for (const PlayerHistoricalEvent& event : player.getHistoricalEvents())
    {
        if (event.category != Category) continue;
        std::string eventCity;
        std::string eventKind;
        std::string eventSource;
        if (!splitSubject(event.subjectId, eventCity, eventKind, eventSource)) continue;
        if (eventCity == cityId && eventKind == kind && player.getWorldDaysElapsed() - event.day < 4) return false;
    }

    player.recordHistoricalEvent(
        Category,
        makeSubject(cityId, kind, sourceId),
        reason.empty() ? "Une perturbation locale ouvre un petit suivi de guilde." : reason,
        false);
    player.recordCanonicalEvent(
        "crime_local_opportunities_queued",
        cityId + ":" + kind,
        "Opportunité locale mémorisée : " + kind,
        1);
    return true;
}

std::vector<CrimeLocalOpportunityEntry> CrimeLocalOpportunitySystem::pending(const Player& player, const std::string& cityId)
{
    std::vector<CrimeLocalOpportunityEntry> result;
    for (const PlayerHistoricalEvent& event : player.getHistoricalEvents())
    {
        if (event.category != Category || event.resolved) continue;
        CrimeLocalOpportunityEntry entry = buildEntry(event);
        if (entry.cityId.empty()) continue;
        if (!cityId.empty() && entry.cityId != cityId) continue;
        result.push_back(std::move(entry));
    }
    std::stable_sort(result.begin(), result.end(), [](const CrimeLocalOpportunityEntry& a, const CrimeLocalOpportunityEntry& b) {
        if (a.eventDay != b.eventDay) return a.eventDay < b.eventDay;
        return a.title < b.title;
    });
    return result;
}

int CrimeLocalOpportunitySystem::pendingCount(const Player& player, const std::string& cityId)
{
    return static_cast<int>(pending(player, cityId).size());
}

CrimeLocalOpportunityAcceptResult CrimeLocalOpportunitySystem::accept(Player& player, const std::string& eventId)
{
    CrimeLocalOpportunityAcceptResult result;
    const PlayerHistoricalEvent* event = findPendingEvent(player, eventId);
    if (event == nullptr)
    {
        result.lines.push_back("Cette opportunité locale n'est plus disponible.");
        return result;
    }

    const CrimeLocalOpportunityEntry entry = buildEntry(*event);
    if (entry.cityId.empty())
    {
        result.lines.push_back("Le suivi local est incomplet et ne peut pas devenir un contrat propre.");
        return result;
    }

    Quest quest;
    quest.id = "crime_local_" + entry.kind + "_" + entry.cityId + "_day" + std::to_string(entry.eventDay);
    quest.rank = entry.kind == "merchant_check" ? "E" : "D";
    quest.title = entry.title;
    quest.origin = "Conséquence locale";
    quest.client = entry.client;
    quest.location = entry.cityId + " / guilde locale";
    quest.objective = entry.objective;
    quest.objectiveType = "service";
    quest.targetFamily = "Criminalité locale / vérification";
    quest.rewardExperience = entry.kind == "witness_followup" ? 34 : 26;
    quest.rewardGold = entry.kind == "route_watch" ? 10 : 8;
    quest.rewardNote = "Petite mission née d'une conséquence locale réellement observée ; récompense volontairement modeste.";
    quest.target = entry.kind == "witness_followup" ? 2 : 1;
    quest.guildQuest = true;
    quest.availableFromDay = player.getWorldDaysElapsed();
    quest.expiresAtDay = player.getWorldDaysElapsed() + 5;
    quest.accepted = true;

    if (!player.getQuestLog().addQuest(quest))
    {
        result.lines.push_back("La guilde garde cette opportunité de côté : le journal refuse le contrat pour le moment.");
        return result;
    }

    player.resolveHistoricalEvent(eventId);
    player.recordCanonicalEvent(
        "crime_local_opportunities_accepted",
        entry.cityId + ":" + entry.kind,
        "Opportunité locale acceptée : " + entry.title,
        1);
    player.recordHistoricalEvent(
        "crime_local_opportunity_accepted",
        quest.id,
        "La guilde transforme une conséquence locale en petit contrat : " + entry.title + ".",
        true);

    result.accepted = true;
    result.questId = quest.id;
    result.lines = entry.lines;
    result.lines.push_back("Contrat ajouté : " + quest.title + ".");
    result.lines.push_back("Récompense modeste : ce travail sert surtout à stabiliser et documenter la situation locale.");
    return result;
}
