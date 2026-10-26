#include "quest/IntercityQuestSystem.hpp"

#include "entity/Player.hpp"
#include "economy/Money.hpp"
#include "quest/Quest.hpp"
#include "quest/QuestLog.hpp"
#include "world/City.hpp"
#include "world/CityTravelRules.hpp"

#include <algorithm>
#include <cstdint>
#include <sstream>

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

    std::vector<std::string> splitPipe(const std::string& value)
    {
        std::vector<std::string> result;
        std::stringstream stream(value);
        std::string part;
        while (std::getline(stream, part, '|'))
        {
            if (!part.empty()) result.push_back(part);
        }
        return result;
    }

    std::string joinPipe(const std::vector<std::string>& values)
    {
        std::string out;
        for (const std::string& value : values)
        {
            if (!out.empty()) out += '|';
            out += value;
        }
        return out;
    }

    std::string cityName(const std::string& id)
    {
        const City* city = City::findById(id);
        return city == nullptr ? id : city->getName();
    }

    std::string payloadFor(const Quest& quest, std::uint32_t h)
    {
        if (quest.objectiveType == "intercity_material")
        {
            static const std::vector<std::string> cargo = {
                "caisse scellée de ferrures et pièces d'entretien",
                "lot de matériaux de chantier sous registre de guilde",
                "petite cargaison de composants d'artisanat",
                "caisses de fournitures et matériaux de réparation"
            };
            return cargo[h % cargo.size()];
        }

        static const std::vector<std::string> information = {
            "dossier scellé de renseignements de guilde",
            "rapport signé destiné à un autre comptoir",
            "registre de route à remettre en mains propres",
            "pli officiel contenant des informations non publiques"
        };
        return information[h % information.size()];
    }

    int rewardGoldFor(const Quest& quest, int distanceKm, int stops)
    {
        // This is a one-shot logistics premium, NOT a reimbursement of every trip made while the
        // quest is active. The player still pays normal city travel costs, detours and return trips.
        // Keeping the distance component deliberately small prevents a transport contract from
        // becoming an unlimited-travel subsidy while still paying the actual courier work.
        const bool materialTransport = quest.objectiveType == "intercity_material";
        const int base = materialTransport ? 6 : 4;
        const int distancePart = std::min(8, std::max(1, distanceKm / 18));
        const int relayPart = std::max(0, stops - 1) * 3;
        return std::min(materialTransport ? 22 : 18, base + distancePart + relayPart);
    }

    int rewardExperienceFor(const Quest& quest, int distanceKm, int stops)
    {
        const int base = quest.objectiveType == "intercity_material" ? 8 : 7;
        return base + std::max(1, distanceKm / 12) + std::max(0, stops - 1) * 5;
    }
}

bool IntercityQuestSystem::isIntercityTemplate(const Quest& quest)
{
    return quest.intercityMission
        || quest.objectiveType == "intercity_information"
        || quest.objectiveType == "intercity_material";
}

bool IntercityQuestSystem::initializeAtAcceptance(const Player& player, Quest& quest, std::vector<std::string>* lines)
{
    if (!isIntercityTemplate(quest)) return true;
    if (quest.intercityMission && !quest.intercityRouteCityIds.empty()) return true;

    const City* source = City::findById(player.getCurrentCityId());
    if (source == nullptr) return false;

    std::vector<const City*> candidates;
    for (const City& city : City::getCatalog())
    {
        if (city.getId() == source->getId()) continue;
        if (!CityTravelRules::evaluateAccess(player, city).allowed) continue;
        candidates.push_back(&city);
    }
    if (candidates.empty())
    {
        if (lines) lines->push_back("Aucune autre ville actuellement accessible ne permet d'organiser ce contrat inter-ville.");
        return false;
    }

    const std::uint32_t h = stableHash(quest.id + "|" + source->getId() + "|intercity");
    std::sort(candidates.begin(), candidates.end(), [source, h](const City* a, const City* b) {
        const int da = City::calculateDistanceBetween(*source, *a);
        const int db = City::calculateDistanceBetween(*source, *b);
        if (da != db) return da < db;
        return ((stableHash(a->getId()) ^ h) < (stableHash(b->getId()) ^ h));
    });

    int desiredStops = std::clamp(quest.target, 1, 2);
    desiredStops = std::min(desiredStops, static_cast<int>(candidates.size()));

    // Rotate the deterministic candidate list so identical quest templates do not always pick the same city.
    if (!candidates.empty())
    {
        std::rotate(candidates.begin(), candidates.begin() + (h % candidates.size()), candidates.end());
    }

    std::vector<std::string> route;
    route.reserve(desiredStops);
    int totalDistance = 0;
    const City* previous = source;
    for (int i = 0; i < desiredStops; ++i)
    {
        const City* chosen = candidates[static_cast<std::size_t>(i)];
        route.push_back(chosen->getId());
        totalDistance += City::calculateDistanceBetween(*previous, *chosen);
        previous = chosen;
    }

    quest.intercityMission = true;
    quest.intercityKind = quest.objectiveType == "intercity_material" ? "materials" : "information";
    quest.intercitySourceCityId = source->getId();
    quest.intercityRouteCityIds = joinPipe(route);
    quest.intercityPayloadName = payloadFor(quest, h);
    quest.intercityStopIndex = 0;
    quest.intercityTotalDistanceKm = totalDistance;
    quest.intercityTravelRewardGold = rewardGoldFor(quest, totalDistance, desiredStops);
    quest.intercityTravelRewardExperience = rewardExperienceFor(quest, totalDistance, desiredStops);
    quest.progress = 0;
    quest.target = desiredStops;
    quest.location = cityName(route.front());

    if (quest.intercityKind == "materials")
    {
        quest.objective = "Transporter " + quest.intercityPayloadName + " depuis " + source->getName()
            + " jusqu'à " + cityName(route.front()) + (desiredStops > 1 ? ", puis poursuivre les relais indiqués." : ".");
    }
    else
    {
        quest.objective = "Transmettre " + quest.intercityPayloadName + " depuis " + source->getName()
            + " jusqu'à " + cityName(route.front()) + (desiredStops > 1 ? ", puis faire suivre le dossier au relais suivant." : ".");
    }

    quest.rewardGold += quest.intercityTravelRewardGold;
    quest.rewardExperience += quest.intercityTravelRewardExperience;
    if (quest.expiresAtDay >= 0)
    {
        quest.expiresAtDay += std::max(1, totalDistance / 35);
    }

    if (lines)
    {
        lines->push_back("Contrat inter-ville préparé à " + source->getName() + ".");
        lines->push_back("Charge : " + quest.intercityPayloadName + ".");
        lines->push_back("Distance contractuelle totale : environ " + std::to_string(totalDistance) + " km.");
        lines->push_back("Prime logistique unique : " + Money::formatEconomyUnits(quest.intercityTravelRewardGold)
            + ", XP +" + std::to_string(quest.intercityTravelRewardExperience) + ".");
        lines->push_back("Important : ce forfait paie le travail de transport. Les taxes de voyage restent prélevées normalement et aucun détour, retour personnel ou trajet supplémentaire n'est remboursé.");
    }
    return true;
}

bool IntercityQuestSystem::prepareSecondPhaseTravel(const Player& player, Quest& quest, std::vector<std::string>* lines)
{
    if (!quest.phaseTwoTravelRequired || !quest.phaseTwoActive || quest.phaseTwoCompleted) return false;
    if (!quest.phaseTwoTravelDestinationCityId.empty()) return true;

    const City* source = City::findById(player.getCurrentCityId());
    if (source == nullptr) return false;

    std::vector<const City*> candidates;
    for (const City& city : City::getCatalog())
    {
        if (city.getId() == source->getId()) continue;
        if (!CityTravelRules::evaluateAccess(player, city).allowed) continue;
        candidates.push_back(&city);
    }
    if (candidates.empty())
    {
        // A dynamically revealed mandatory follow-up must never soft-lock an otherwise valid quest.
        // If the character cannot legally reach another city yet, the same administrative/service
        // complication is handled through a local relay and falls back to the normal phase-2 action.
        quest.phaseTwoTravelRequired = false;
        quest.phaseTwoTravelSourceCityId.clear();
        quest.phaseTwoTravelDestinationCityId.clear();
        quest.phaseTwoTravelDistanceKm = 0;
        if (lines) lines->push_back("Aucune autre ville n'est encore accessible : le contact organise un relais local, sans voyage inter-ville.");
        return true;
    }

    const std::uint32_t h = stableHash(quest.id + "|phase2_travel|" + source->getId());
    std::sort(candidates.begin(), candidates.end(), [source, h](const City* a, const City* b) {
        const int da = City::calculateDistanceBetween(*source, *a);
        const int db = City::calculateDistanceBetween(*source, *b);
        const int sa = da + static_cast<int>((stableHash(a->getId()) ^ h) % 7u);
        const int sb = db + static_cast<int>((stableHash(b->getId()) ^ h) % 7u);
        return sa < sb;
    });

    const City* destination = candidates.front();
    const int distance = City::calculateDistanceBetween(*source, *destination);
    quest.phaseTwoTravelSourceCityId = source->getId();
    quest.phaseTwoTravelDestinationCityId = destination->getId();
    quest.phaseTwoTravelDistanceKm = distance;
    quest.phaseTwoObjective += " Destination : " + destination->getName() + ".";
    const int logisticsSupplement = std::min(10, 3 + std::max(1, distance / 22));
    quest.phaseTwoRewardGold += logisticsSupplement;
    quest.phaseTwoRewardExperience += 2 + std::max(1, distance / 22);
    if (quest.expiresAtDay >= 0) quest.expiresAtDay += std::max(1, distance / 45);

    if (lines)
    {
        lines->push_back("Suite inter-ville préparée : " + source->getName() + " → " + destination->getName() + ".");
        lines->push_back("Distance supplémentaire : environ " + std::to_string(distance) + " km.");
        lines->push_back("Une petite prime logistique unique est ajoutée à la récompense de phase 2 ; les frais réels de déplacement, détours et retour restent à ta charge.");
    }
    return true;
}

std::vector<std::string> IntercityQuestSystem::onCityArrival(Player& player, const std::string& fromCityId, const std::string& toCityId)
{
    std::vector<std::string> lines;
    std::vector<std::string> completedIds;

    for (Quest& quest : player.getQuestLog().getQuests())
    {
        if (!quest.phaseTwoTravelRequired || !quest.phaseTwoActive || quest.phaseTwoCompleted || quest.failed || quest.turnedIn) continue;
        if (quest.phaseTwoTravelDestinationCityId.empty() || quest.phaseTwoTravelDestinationCityId != toCityId) continue;

        quest.phaseTwoProgress = std::max(1, quest.phaseTwoTarget);
        quest.phaseTwoActive = false;
        quest.phaseTwoCompleted = true;
        quest.completed = true;
        lines.push_back("Phase 2 inter-ville : remise effectuée à " + cityName(toCityId) + ". La quête peut maintenant être finalisée.");
        player.recordCanonicalEvent("suites_quete_interville", quest.id + "::" + toCityId,
            cityName(fromCityId) + " → " + cityName(toCityId) + " | " + quest.phaseTwoTitle, 1);
        player.recordCanonicalEvent(
            "suites_quete_terminees",
            quest.phaseTwoMandatory ? "mandatory" : "optional",
            quest.phaseTwoTitle.empty() ? quest.title : quest.phaseTwoTitle,
            1
        );
    }

    for (Quest& quest : player.getQuestLog().getQuests())
    {
        if (!quest.intercityMission || !quest.accepted || quest.turnedIn || quest.failed || quest.completed) continue;
        const std::vector<std::string> route = splitPipe(quest.intercityRouteCityIds);
        if (quest.intercityStopIndex < 0 || quest.intercityStopIndex >= static_cast<int>(route.size())) continue;
        if (route[static_cast<std::size_t>(quest.intercityStopIndex)] != toCityId) continue;

        ++quest.intercityStopIndex;
        quest.progress = std::min(quest.target, quest.progress + 1);
        const bool finalStop = quest.intercityStopIndex >= static_cast<int>(route.size());
        lines.push_back("Contrat inter-ville : étape validée à " + cityName(toCityId) + " ("
            + std::to_string(quest.progress) + "/" + std::to_string(std::max(1, quest.target)) + ").");
        player.recordCanonicalEvent("missions_interville", quest.id + "::" + toCityId,
            cityName(fromCityId) + " → " + cityName(toCityId) + " | " + quest.intercityPayloadName, 1);

        if (finalStop)
        {
            completedIds.push_back(quest.id);
            player.recordCanonicalEvent(
                "contrats_interville_termines",
                quest.intercityKind == "materials" ? "materials" : "information",
                quest.title,
                1
            );
            if (route.size() > 1)
            {
                player.recordCanonicalEvent(
                    "contrats_interville_relais_multiples",
                    quest.id,
                    "Contrat inter-ville terminé avec plusieurs relais",
                    1
                );
            }
            lines.push_back(quest.intercityKind == "materials"
                ? "La cargaison contractuelle est remise au bon comptoir. Retourne à la guilde pour le paiement."
                : "L'information est remise en mains propres. Retourne à la guilde pour le paiement.");
        }
        else
        {
            quest.location = cityName(route[static_cast<std::size_t>(quest.intercityStopIndex)]);
            lines.push_back("Relais suivant : " + quest.location + ".");
        }
    }

    for (const std::string& id : completedIds)
    {
        player.getQuestLog().completeQuest(id);
    }
    return lines;
}

std::vector<std::string> IntercityQuestSystem::journalLines(const Quest& quest)
{
    std::vector<std::string> lines;
    if (!quest.intercityMission) return lines;

    const std::vector<std::string> route = splitPipe(quest.intercityRouteCityIds);
    lines.push_back("--- Contrat inter-ville ---");
    lines.push_back("Nature : " + std::string(quest.intercityKind == "materials" ? "transport rémunéré de matériaux." : "transmission rémunérée d'informations."));
    if (!quest.intercityPayloadName.empty()) lines.push_back("Charge : " + quest.intercityPayloadName + ".");
    if (!quest.intercitySourceCityId.empty()) lines.push_back("Départ contractuel : " + cityName(quest.intercitySourceCityId) + ".");
    lines.push_back("Distance totale prévue : environ " + std::to_string(std::max(0, quest.intercityTotalDistanceKm)) + " km.");
    if (quest.intercityStopIndex < static_cast<int>(route.size()))
    {
        lines.push_back("Prochaine remise : " + cityName(route[static_cast<std::size_t>(quest.intercityStopIndex)]) + ".");
    }
    else
    {
        lines.push_back("Toutes les remises inter-ville sont terminées.");
    }
    lines.push_back("Étapes : " + std::to_string(std::max(0, quest.intercityStopIndex)) + "/" + std::to_string(route.size()) + ".");
    lines.push_back("Prime logistique : forfait unique de " + Money::formatEconomyUnits(std::max(0, quest.intercityTravelRewardGold))
        + ". Les voyages restent facturés normalement ; les détours et trajets personnels ne sont jamais remboursés.");
    return lines;
}

std::string IntercityQuestSystem::nextDestinationName(const Quest& quest)
{
    const std::vector<std::string> route = splitPipe(quest.intercityRouteCityIds);
    if (quest.intercityStopIndex < 0 || quest.intercityStopIndex >= static_cast<int>(route.size())) return "";
    return cityName(route[static_cast<std::size_t>(quest.intercityStopIndex)]);
}
