#include "quest/QuestLocationAccessSystem.hpp"

#include "entity/Player.hpp"
#include "progression/bestiary/BestiaryRuntimeProgress.hpp"
#include "quest/Quest.hpp"
#include "world/City.hpp"
#include "world/CityTravelRules.hpp"

#include <algorithm>
#include <cctype>
#include <string>
#include <vector>

namespace
{
    std::string lowerAscii(std::string value)
    {
        std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });
        return value;
    }

    bool containsText(const std::string& value, const std::string& needle)
    {
        if (needle.empty()) return false;
        return lowerAscii(value).find(lowerAscii(needle)) != std::string::npos;
    }

    bool containsAny(const std::string& value, const std::vector<std::string>& needles)
    {
        for (const std::string& needle : needles)
        {
            if (containsText(value, needle)) return true;
        }
        return false;
    }

    bool flexibleSearchQuest(const Quest& quest)
    {
        return quest.objectiveType == "combat"
            || quest.objectiveType == "bestiaire"
            || quest.objectiveType == "exploration"
            || quest.objectiveType == "livraison"
            || quest.objectiveType == "material"
            || quest.objectiveType == "récolte / vérification";
    }

    bool explicitlyRequiresAnnouncedPlace(const Quest& quest, const std::string& place)
    {
        if (place.empty()) return false;
        if (quest.intercityMission || quest.objectiveType == "intercity_information" || quest.objectiveType == "intercity_material") return true;
        if (quest.objectiveType == "service" || quest.objectiveType == "dialogue" || quest.objectiveType == "story_bundle") return true;

        const bool placeNamedInObjective = containsText(quest.objective, place);
        if (!placeNamedInObjective) return false;

        return containsAny(quest.objective, {
            "dans ", "au ", "à la ", "a la ", "à l'", "a l'", "au sein", "à l'intérieur", "a l'interieur",
            "sur place", "dans cette zone", "dans ce bâtiment", "dans ce batiment", "jusqu'à", "jusqu'a",
            "remettre", "livrer", "signer", "rendre au", "rendre à", "rendre a"
        });
    }

    bool biomeDiscovered(const std::string& biomeName)
    {
        return BestiaryRuntimeProgress::getEncounterCount(biomeName) > 0
            || BestiaryRuntimeProgress::getEncounterCount("Observation - " + biomeName) > 0;
    }

    struct MatchedPlace
    {
        const City* city = nullptr;
        std::string placeName;
        bool biome = false;
        bool building = false;
        bool initiallyKnown = true;
    };

    MatchedPlace findMatchedPlace(const Player& player, const Quest& quest)
    {
        const std::string location = quest.location;
        if (location.empty()) return {};

        // City names are the strongest exact match.
        for (const City& city : City::getCatalog())
        {
            if (containsText(location, city.getName()) || containsText(location, city.getId()))
            {
                return {&city, city.getName(), false, false, true};
            }
        }

        // Then real city biomes.
        for (const City& city : City::getCatalog())
        {
            for (const CityBiomeDistance& biome : city.getBiomeDistances())
            {
                if (containsText(location, biome.biomeName))
                {
                    return {&city, biome.biomeName, true, false, biome.initiallyKnown};
                }
            }
        }

        // Finally named buildings/services. This is intentionally conservative: generic labels
        // such as "place du village" stay local rather than being falsely bound to another city.
        for (const City& city : City::getCatalog())
        {
            const std::vector<CityBuildingPreview> buildings = CityTravelRules::getBuildingsForCity(player, city);
            for (const CityBuildingPreview& building : buildings)
            {
                if ((!building.name.empty() && containsText(location, building.name))
                    || (!building.id.empty() && containsText(location, building.id)))
                {
                    return {&city, building.name.empty() ? location : building.name, false, true, true};
                }
            }
        }

        return {};
    }
}

QuestLocationAccessReport QuestLocationAccessSystem::evaluate(const Player& player, const Quest& quest)
{
    QuestLocationAccessReport report;
    report.announcedLocation = quest.location;

    const MatchedPlace place = findMatchedPlace(player, quest);
    if (place.city == nullptr)
    {
        // Generic local areas and abstract actions are deliberately not guessed as inaccessible.
        report.lines.push_back("Accès au lieu : aucune restriction de ville certaine n'est détectée sur cette fiche.");
        return report;
    }

    report.announcedLocation = place.placeName;
    report.exactLocationRequired = explicitlyRequiresAnnouncedPlace(quest, place.placeName);

    const CityAccessReport cityAccess = CityTravelRules::evaluateAccess(player, *place.city);
    const bool sameCity = player.getCurrentCityId() == place.city->getId();
    const bool cityReachable = sameCity || cityAccess.allowed;

    bool known = true;
    if (place.biome)
    {
        known = place.initiallyKnown || biomeDiscovered(place.placeName);
    }
    report.locationKnown = known;

    if (cityReachable && known)
    {
        report.state = QuestLocationAccessState::Accessible;
        report.lines.push_back("Accès au lieu : accessible avec la progression actuelle.");
        return report;
    }

    const bool canPlausiblySearchElsewhere = flexibleSearchQuest(quest) && !report.exactLocationRequired;
    if (canPlausiblySearchElsewhere)
    {
        report.state = QuestLocationAccessState::ReducedChance;
        if (!known)
        {
            report.reason = "Le lieu conseillé est encore inconnu, mais la cible ou la ressource peut exceptionnellement être trouvée ailleurs.";
        }
        else
        {
            report.reason = "Le lieu conseillé est actuellement inaccessible, mais la cible ou la ressource existe parfois dans d'autres zones.";
        }
        report.lines.push_back((known ? "Lieu conseillé actuellement inaccessible." : "Lieu conseillé actuellement inconnu.")
            + std::string(" Chance réduite de trouvaille si tu acceptes de chercher dans d'autres zones compatibles."));
        report.lines.push_back("Diane conseille de ne prendre ce contrat que si tu acceptes une recherche moins fiable que dans son habitat principal.");
        return report;
    }

    report.state = QuestLocationAccessState::Blocked;
    if (!known)
    {
        report.reason = "Cette quête exige précisément un lieu encore inconnu.";
        report.lines.push_back("Lieu requis actuellement inconnu : la quête demande précisément cette zone, pas seulement une cible pouvant apparaître ailleurs.");
    }
    else
    {
        report.reason = "Cette quête exige précisément un lieu actuellement inaccessible.";
        report.lines.push_back("Lieu requis actuellement inaccessible : l'objectif doit être réalisé précisément à cet endroit.");
    }
    report.lines.push_back("Diane refuse de valider ce contrat pour l'instant afin d'éviter de te donner une mission techniquement impossible ou inutilement frustrante.");
    return report;
}

std::string QuestLocationAccessSystem::statusLabel(const QuestLocationAccessReport& report)
{
    switch (report.state)
    {
        case QuestLocationAccessState::Accessible: return "Accessible";
        case QuestLocationAccessState::ReducedChance: return "Chance réduite de trouvaille";
        case QuestLocationAccessState::Blocked:
            return report.locationKnown ? "Actuellement inaccessible" : "Actuellement inconnu";
    }
    return "Accessible";
}

std::string QuestLocationAccessSystem::decorateLocation(const std::string& locationText, const QuestLocationAccessReport& report)
{
    if (report.state == QuestLocationAccessState::Accessible) return locationText;
    return locationText + " [" + statusLabel(report) + "]";
}
