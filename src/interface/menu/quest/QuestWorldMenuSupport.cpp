// EN: World/city support extracted from QuestMenu to keep quest navigation modular.
// FR: Support monde/ville extrait de QuestMenu pour garder la navigation de quêtes modulaire.

#include "interface/menu/quest/QuestWorldMenuSupport.hpp"
#include "interface/menu/quest/QuestDeadlineSupport.hpp"
#include "interface/menu/quest/QuestContractorMenu.hpp"
#include "adventure/flavor/ExplorationBiomeFlavor.hpp"
#include "adventure/content/BiomeLivingContentCatalog.hpp"
#include "adventure/content/BiomeAmbientEventSystem.hpp"
#include "interface/menu/training/TrainingGroundMenu.hpp"
#include "interface/menu/LocalReputationRepairMenu.hpp"
#include "core/Console.hpp"
#include "core/Random.hpp"
#include "quest/QuestCatalog.hpp"
#include "item/material/MaterialCatalog.hpp"
#include "entity/MonsterCatalog.hpp"
#include "economy/EconomyBalance.hpp"
#include "economy/shop/ShopTransactionSystem.hpp"
#include "economy/Money.hpp"
#include "interface/menu/InventoryMenu.hpp"
#include "interface/menu/shop/ShopMenu.hpp"
#include "interface/menu/common/PagedMenu.hpp"
#include "interface/menu/common/MessageScreen.hpp"
#include "interface/TerminalInterface.hpp"
#include "interface/model/MenuScreen.hpp"
#include "world/City.hpp"
#include "world/CityTravelRules.hpp"
#include "entity/Player.hpp"

#include <algorithm>
#include <cstddef>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <utility>
#include <vector>

namespace QuestWorldMenuSupport
{
    using QuestDeadlineSupport::expireOverdueQuestDeadlines;
    void openCityVault(Player& player);
    void openInnMenu(Player& player);
    void openCityHubMenu(Player& player);
    void openRouteMicroQuestBoard(Player& player);
    void showExplorationMapPreview(const Player& player);

    std::string currentCityName(const Player& player)
    {
        const City* city = City::findById(player.getCurrentCityId());
        return city == nullptr ? "Ville inconnue" : city->getName();
    }

    int canonicalRecordCount(const Player& player, const std::string& category, const std::string& key)
    {
        for (const PlayerJournalRecord& record : player.getCanonicalJournalRecords())
        {
            if (record.category == category && record.key == key)
            {
                return record.count;
            }
        }
        return 0;
    }

    std::string recentActionKey(const Player& player, const std::string& baseKey)
    {
        return baseKey + ":day" + std::to_string(player.getWorldDaysElapsed()) + ":unit" + std::to_string(player.getWorldDayProgressUnits()) + ":" + std::to_string(player.getCanonicalJournalRecords().size());
    }

    void recordRecentAction(Player& player, const std::string& baseKey, const std::string& label)
    {
        player.recordCanonicalEvent("dernieres_actions", recentActionKey(player, baseKey), label);
    }

    int guildRankPowerForRequests(const std::string& rank)
    {
        if (rank.find("Dieu") != std::string::npos) return 34;
        if (rank.find("Légende") != std::string::npos || rank.find("Legende") != std::string::npos) return 28;
        if (rank.find("Héros mondial") != std::string::npos || rank.find("Heros mondial") != std::string::npos) return 22;
        if (rank.find("SSS") != std::string::npos) return 18;
        if (rank.find("SS") != std::string::npos) return 14;
        if (rank.find("S") != std::string::npos) return 10;
        if (rank.find("A") != std::string::npos) return 7;
        if (rank.find("B") != std::string::npos) return 5;
        if (rank.find("C") != std::string::npos) return 4;
        if (rank.find("D") != std::string::npos) return 3;
        if (rank.find("E") != std::string::npos) return 2;
        return 1;
    }

    std::string guildRankForRequestGate(const Player& player)
    {
        if (!player.hasTitle("Aventurier"))
        {
            return "Non inscrit";
        }

        int completedGuildContracts = 0;
        for (const Quest& quest : player.getQuestLog().getQuests())
        {
            if (quest.guildQuest && quest.turnedIn)
            {
                ++completedGuildContracts;
            }
        }

        struct Threshold
        {
            int requiredContracts;
            int requiredLevel;
            std::string rank;
        };

        const std::vector<Threshold> thresholds = {
            {130, 90, "Dieu"},
            {100, 70, "Légende"},
            {75, 55, "Héros mondial"},
            {55, 42, "SSS"},
            {40, 35, "SS"},
            {28, 24, "S"},
            {20, 18, "A"},
            {14, 12, "B"},
            {9, 8, "C"},
            {5, 5, "D"},
            {2, 2, "E"}
        };

        for (const Threshold& threshold : thresholds)
        {
            if (completedGuildContracts >= threshold.requiredContracts && player.getLevel() >= threshold.requiredLevel)
            {
                return threshold.rank;
            }
        }
        return "F";
    }

    bool guildRequestRankDUnlocked(const Player& player)
    {
        return guildRankPowerForRequests(guildRankForRequestGate(player)) >= guildRankPowerForRequests("D");
    }

    std::string routeDiscoveryLabel(int discoveryIndex, const City& origin, const City& destination)
    {
        switch (discoveryIndex)
        {
            case 0: return "Bivouac discret entre " + origin.getName() + " et " + destination.getName();
            case 1: return "Source claire près de la route " + origin.getName() + " → " + destination.getName();
            case 2: return "Marchand ambulant croisé sur la liaison " + origin.getName() + " / " + destination.getName();
            case 3: return "Raccourci de bornes anciennes vers " + destination.getName();
            default: return "Détail mineur déjà noté sur la route " + origin.getName() + " / " + destination.getName();
        }
    }

    std::vector<std::string> recordRoutePassageAndMaybeDiscovery(Player& player, const City& origin, const City& destination, int distance, int passageAmount)
    {
        std::vector<std::string> lines;
        const std::string routeKey = CityTravelRules::buildNormalizedRouteKey(origin.getId(), destination.getId());
        const std::string routeLabel = origin.getName() + " ↔ " + destination.getName();
        const int safePassageAmount = std::max(1, passageAmount);
        player.recordCanonicalEvent("passages_route", routeKey, routeLabel, safePassageAmount);
        player.recordCanonicalEvent("distance_route", routeKey, routeLabel, std::max(1, distance) * safePassageAmount);

        const int discoveryCount = CityTravelRules::getRouteDiscoveryCount(player, origin.getId(), destination.getId());
        const int discoveryLimit = CityTravelRules::getRouteDiscoveryLimit(distance);
        const int passageCount = canonicalRecordCount(player, "passages_route", routeKey);
        if (discoveryCount >= discoveryLimit)
        {
            lines.push_back("Route connue : limite de découvertes atteinte pour cette liaison (" + std::to_string(discoveryLimit) + "/" + std::to_string(discoveryLimit) + ").");
            return lines;
        }

        const int nextThreshold = 2 + discoveryCount * 3;
        if (passageCount < nextThreshold)
        {
            lines.push_back("Route observée : " + std::to_string(passageCount) + "/" + std::to_string(nextThreshold) + " passage(s) avant une possible nouvelle découverte.");
            return lines;
        }

        const std::string discoveryLabel = routeDiscoveryLabel(discoveryCount, origin, destination);
        const std::string discoveryKey = routeKey + "::" + std::to_string(discoveryCount + 1);
        player.recordCanonicalEvent("decouvertes_route", discoveryKey, discoveryLabel);
        recordRecentAction(player, "route_discovery:" + routeKey, "Découverte de route : " + discoveryLabel);
        lines.push_back("Nouvelle découverte de route : " + discoveryLabel + ".");
        lines.push_back("Progression des découvertes sur cette liaison : " + std::to_string(discoveryCount + 1) + "/" + std::to_string(discoveryLimit) + ".");
        return lines;
    }

    struct TravelRouteOption
    {
        std::string id;
        std::string label;
        std::string detail;
        int extraCopper = 0;
        int extraTimeUnits = 0;
        bool available = true;
        bool nightAllowed = false;
        bool risky = false;
    };

    int clampedTravelTimeWithRoute(int baseTimeUnits, const TravelRouteOption& route)
    {
        return std::max(1, baseTimeUnits + route.extraTimeUnits);
    }

    std::vector<TravelRouteOption> buildTravelRouteOptions(const Player& player, const City& origin, const City& destination, int distance)
    {
        std::vector<TravelRouteOption> routes;
        const bool night = CityTravelRules::isNightTravelClosed(player);
        const int estimatedTicket = EconomyBalance::estimatedTravelCopperCost(distance);
        const int discoveryCount = CityTravelRules::getRouteDiscoveryCount(player, origin.getId(), destination.getId());
        const int discoveryLimit = CityTravelRules::getRouteDiscoveryLimit(distance);

        routes.push_back({
            "controlled",
            "Route contrôlée",
            night ? "Fermée la nuit par les gardes. Pas de marche gratuite pour remplacer l'auberge." : "Route officielle, neutre, contrôlée par les gardes.",
            0,
            0,
            !night,
            false,
            false
        });
        routes.push_back({
            "safe",
            "Route sûre",
            night ? "Fermée la nuit. Plus lente mais plus encadrée quand elle est ouverte." : "Plus lente et un peu plus chère, mais moins propice aux mauvaises surprises.",
            std::max(12, estimatedTicket / 4),
            1,
            !night,
            false,
            false
        });
        routes.push_back({
            "fast",
            "Route rapide",
            night ? "Fermée la nuit : les gardes refusent les départs rapides dans le noir." : "Plus rapide, mais plus risquée et moins confortable.",
            std::max(8, estimatedTicket / 8),
            -1,
            !night,
            false,
            true
        });
        routes.push_back({
            "caravan",
            night ? "Convoi gardé nocturne" : "Convoi marchand",
            night ? "Très cher, mais autorisé : escorte officielle, torches, registre et gardes payés." : "Stable et cher, utile pour voyager sans être seul sur les longues routes.",
            std::max(40, estimatedTicket / 2 + (night ? 90 : 35)),
            night ? 1 : 1,
            true,
            true,
            false
        });
        routes.push_back({
            "shortcut",
            "Raccourci découvert",
            discoveryCount >= discoveryLimit
                ? (night ? "Connu, mais interdit la nuit sans permis : pas de raccourci gratuit dans le noir." : "Débloqué car la liaison est bien connue. Plus court, mais un peu instable.")
                : "Verrouillé : il faut connaître toute la liaison avant de l'utiliser.",
            discoveryCount >= discoveryLimit ? -std::max(3, estimatedTicket / 10) : 0,
            -1,
            discoveryCount >= discoveryLimit && !night,
            false,
            true
        });
        return routes;
    }

    std::string routeEventLabel(int eventIndex, const City& origin, const City& destination)
    {
        switch (eventIndex)
        {
            case 0: return "Patrouille locale notée entre " + origin.getName() + " et " + destination.getName();
            case 1: return "Pont fragile signalé sur la liaison " + origin.getName() + " / " + destination.getName();
            case 2: return "Camp abandonné au bord de la route vers " + destination.getName();
            case 3: return "Convoi bloqué puis dégagé sur la route " + origin.getName() + " → " + destination.getName();
            default: return "Rumeur mineure déjà classée sur la route " + origin.getName() + " / " + destination.getName();
        }
    }

    std::vector<std::string> recordLimitedRouteEventAndRumor(Player& player, const City& origin, const City& destination, int distance, const TravelRouteOption& route)
    {
        std::vector<std::string> lines;
        const std::string routeKey = CityTravelRules::buildNormalizedRouteKey(origin.getId(), destination.getId());
        const int eventCount = CityTravelRules::getRouteEventCount(player, origin.getId(), destination.getId());
        const int eventLimit = CityTravelRules::getRouteEventLimit(distance);
        const int passageCount = canonicalRecordCount(player, "passages_route", routeKey);

        if (eventCount >= eventLimit)
        {
            lines.push_back("Événements de route : limite atteinte pour cette liaison (" + std::to_string(eventLimit) + "/" + std::to_string(eventLimit) + "). Rien de majeur ne se recrée en boucle.");
            return lines;
        }

        const int spacing = route.risky ? 2 : 3;
        const int threshold = 2 + eventCount * spacing;
        if (passageCount < threshold)
        {
            lines.push_back("Rumeur de route : rien de majeur cette fois (" + std::to_string(passageCount) + "/" + std::to_string(threshold) + " passage(s) avant un possible événement important).");
            return lines;
        }

        const std::string label = routeEventLabel(eventCount, origin, destination);
        const std::string key = routeKey + "::" + std::to_string(eventCount + 1);
        player.recordCanonicalEvent("evenements_route", key, label);
        player.recordCanonicalEvent("rumeurs_route", routeKey, "Rumeurs classées : " + origin.getName() + " ↔ " + destination.getName());
        recordRecentAction(player, "route_event:" + routeKey, "Événement de route : " + label);
        lines.push_back("Événement de route limité : " + label + ".");
        lines.push_back("Progression événements de route : " + std::to_string(eventCount + 1) + "/" + std::to_string(eventLimit) + ".");
        return lines;
    }

    TravelRouteOption askTravelRouteChoice(const Player& player, const City& origin, const City& destination, int distance, int baseTaxCopper, int baseTimeUnits)
    {
        const std::vector<TravelRouteOption> routes = buildTravelRouteOptions(player, origin, destination, distance);
        while (true)
        {
            MenuScreen routeScreen("CHOIX DE ROUTE", "quest.city_travel.route_choice");
            routeScreen.addLine("Destination : " + destination.getName() + ".");
            routeScreen.addLine("Taxe de ville de base : " + Money::formatCopper(baseTaxCopper) + ". Les frais de route s'ajoutent selon le trajet choisi.");
            routeScreen.addLine("Temps de base : " + std::to_string(baseTimeUnits) + " segment(s).");
            if (CityTravelRules::isNightTravelClosed(player))
            {
                routeScreen.addLine("Nuit : les routes normales sont fermées. Seul un convoi gardé payant peut partir sans casser l'auberge.");
            }
            routeScreen.addBackOption("Annuler", "quest.city_travel.route_choice.back");

            for (std::size_t i = 0; i < routes.size(); ++i)
            {
                const TravelRouteOption& route = routes[i];
                const int routeTax = std::max(0, baseTaxCopper + route.extraCopper);
                const int routeTime = clampedTravelTimeWithRoute(baseTimeUnits, route);
                std::string detail = route.detail + " | coût total " + Money::formatCopper(routeTax) + " | " + std::to_string(routeTime) + " segment(s).";
                if (!route.available) detail += " [indisponible]";
                routeScreen.addOption(static_cast<int>(i + 1), route.label, detail, route.available, "quest.city_travel.route_choice." + route.id);
            }

            const int choice = TerminalInterface::askMenuChoiceFromOptions(routeScreen, "Choix invalide.");
            Console::clear();
            if (choice == 0)
            {
                return {"", "Annulé", "", 0, 0, false, false, false};
            }
            if (choice >= 1 && choice <= static_cast<int>(routes.size()) && routes[static_cast<std::size_t>(choice - 1)].available)
            {
                return routes[static_cast<std::size_t>(choice - 1)];
            }
        }
    }

    int stableMissionRoll(const std::string& text)
    {
        unsigned int hash = 2166136261u;
        for (char c : text)
        {
            hash ^= static_cast<unsigned char>(c);
            hash *= 16777619u;
        }
        return static_cast<int>(hash % 100);
    }

    std::vector<std::string> maybeCreateRareRouteAdventurerOffer(Player& player, const City& origin, const City& destination, const TravelRouteOption& route)
    {
        std::vector<std::string> lines;
        const std::string routeKey = CityTravelRules::buildNormalizedRouteKey(origin.getId(), destination.getId());
        const std::string rareKey = routeKey + ":" + std::to_string(player.getWorldDaysElapsed()) + ":" + route.id;
        const int chance = route.risky ? 4 : 2;
        const int roll = stableMissionRoll("rare_route_group:" + rareKey + ":" + std::to_string(player.getCanonicalJournalRecords().size()));
        if (roll >= chance)
        {
            return lines;
        }

        const std::vector<std::pair<std::string, std::string>> groups = {
            {"lanternes", "Les Lanternes de Prunigil"},
            {"sables_gris", "Les Sables Gris"},
            {"deux_lames_chariot", "Deux Lames et un Chariot"},
            {"eclats_azur", "Les Éclats d'Azur"},
            {"coureurs_virevent", "Les Coureurs de Virevent"}
        };
        const auto& group = groups[static_cast<std::size_t>(roll % static_cast<int>(groups.size()))];
        const std::string microQuestKey = routeKey + ":" + group.first + ":day" + std::to_string(player.getWorldDaysElapsed());
        player.recordCanonicalEvent("rencontres_rares_groupes_route", routeKey, group.second + " croisés sur " + origin.getName() + " → " + destination.getName());
        player.recordCanonicalEvent("groupes_pnj_decouverts", group.first, group.second + " — rencontre rare sur route");
        player.recordCanonicalEvent("micro_quetes_route_actives", microQuestKey, "Aide ponctuelle proposée par " + group.second);
        recordRecentAction(player, "rare_route_group:" + routeKey, "Groupe croisé sur la route : " + group.second);
        lines.push_back("Événement rare : " + group.second + " te croisent sur la route.");
        lines.push_back("Ils proposent une aide ponctuelle sur leur propre quête. Une micro-quête de route est notée dans le registre, sans spammer le journal principal.");
        lines.push_back("Contact découvert, rumeur ajoutée et micro-quête de route inscrite dans le registre.");
        return lines;
    }

    int askVaultEntryChoice(const std::string& title, const std::string& screenId, const std::vector<std::string>& labels)
    {
        if (labels.empty())
        {
            MessageScreen::show(title, screenId + ".empty", {"Aucun objet disponible dans cette catégorie."}, false);
            return -1;
        }

        MenuScreen screen(title, screenId);
        screen.addLine("Choisis une entrée. 0 annule sans déplacer d'objet.");
        screen.addBackOption("Annuler", screenId + ".back");
        for (std::size_t i = 0; i < labels.size(); ++i)
        {
            screen.addOption(
                static_cast<int>(i + 1),
                labels[i],
                "Déplacer cette entrée.",
                true,
                screenId + ".entry." + std::to_string(i + 1)
            );
        }
        const int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choix invalide.");
        Console::clear();
        if (choice <= 0 || choice > static_cast<int>(labels.size()))
        {
            return -1;
        }
        return choice - 1;
    }

    std::string cityNameFromId(const std::string& cityId)
    {
        const City* city = City::findById(cityId);
        return city == nullptr ? cityId : city->getName();
    }

    void showCityVaultContentsForCity(const Player& player, const std::string& cityId, bool remoteReadOnly)
    {
        std::vector<std::string> lines;
        lines.push_back("Ville de rattachement : " + cityNameFromId(cityId) + ".");
        lines.push_back("Occupation : " + std::to_string(player.getCityVaultUsedSlotsForCity(cityId)) + "/" + std::to_string(player.getCityVaultCapacityForCity(cityId)) + " emplacements.");
        lines.push_back("Coût par entrée : arme 3, armure 3, consommable 1, pile de matériau 1.");
        if (remoteReadOnly)
        {
            lines.push_back("Consultation distante : lecture seule. Aucun retrait n'est possible depuis une autre ville.");
        }
        lines.push_back("");

        const Inventory& vault = player.getCityVaultForCity(cityId);
        lines.push_back("Armes : " + std::to_string(vault.getWeaponCount()) + ".");
        for (const Weapon& weapon : vault.getWeapons())
        {
            lines.push_back("- " + weapon.getName() + " | durabilité " + std::to_string(weapon.getDurability()) + "/" + std::to_string(weapon.getMaxDurability()) + ".");
        }
        lines.push_back("Armures : " + std::to_string(vault.getArmorCount()) + ".");
        for (const Armor& armor : vault.getArmors())
        {
            lines.push_back("- " + armor.getName() + " | durabilité " + std::to_string(armor.getDurability()) + "/" + std::to_string(armor.getMaxDurability()) + ".");
        }
        lines.push_back("Consommables : " + std::to_string(vault.getConsumableCount()) + ".");
        for (const Consumable& consumable : vault.getConsumables())
        {
            lines.push_back("- " + consumable.getName() + ".");
        }
        lines.push_back("Piles de matériaux : " + std::to_string(vault.getMaterialCount()) + ".");
        for (const Material& material : vault.getMaterials())
        {
            lines.push_back("- " + material.getName() + " x" + std::to_string(material.getQuantity()) + " [" + material.getQualityLabel() + "].");
        }
        if (vault.getWeaponCount() + vault.getArmorCount() + vault.getConsumableCount() + vault.getMaterialCount() == 0)
        {
            lines.push_back("Le coffre est vide.");
        }

        MessageScreen::show(remoteReadOnly ? "COFFRE DISTANT — LECTURE SEULE" : "CONTENU DU COFFRE MUNICIPAL", remoteReadOnly ? "quest.city_vault.remote_contents" : "quest.city_vault.contents", lines, false);
    }

    void showCityVaultContents(const Player& player)
    {
        showCityVaultContentsForCity(player, player.getCurrentCityId(), false);
    }

    void showRemoteCityVaultBrowser(const Player& player)
    {
        MenuScreen screen("COFFRES DISTANTS", "quest.city_vault.remote_browser");
        screen.addLine("Choisis un coffre municipal à consulter. Les retraits restent bloqués hors de la ville concernée.");
        screen.addBackOption("Retour", "quest.city_vault.remote_browser.back");

        int option = 1;
        std::vector<std::string> cityIds;
        for (const City& city : City::getCatalog())
        {
            if (city.getId() == player.getCurrentCityId())
            {
                continue;
            }

            const bool hasVault = player.hasCityVaultInCity(city.getId());
            const std::string detail = hasVault
                ? "Lecture seule : " + std::to_string(player.getCityVaultUsedSlotsForCity(city.getId())) + "/" + std::to_string(player.getCityVaultCapacityForCity(city.getId())) + " emplacements."
                : "Aucun coffre personnel acheté dans cette ville.";
            screen.addOption(option, city.getName(), detail, hasVault, "quest.city_vault.remote." + city.getId());
            cityIds.push_back(city.getId());
            ++option;
        }

        const int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choix invalide.");
        Console::clear();
        if (choice <= 0 || choice > static_cast<int>(cityIds.size()))
        {
            return;
        }

        const std::string cityId = cityIds[choice - 1];
        if (!player.hasCityVaultInCity(cityId))
        {
            MessageScreen::show("COFFRE INEXISTANT", "quest.city_vault.remote_missing", {"Aucun coffre personnel n'a encore été acheté dans cette ville."}, false);
            return;
        }

        showCityVaultContentsForCity(player, cityId, true);
    }

    void showKnownCitiesAndVaultRules(const Player& player)
    {
        std::vector<std::string> lines;
        lines.push_back("Ville actuelle : " + currentCityName(player) + ".");
        lines.push_back("Le coffre personnel est sécurisé et ne fait pas partie de l'inventaire transporté.");
        lines.push_back("Chaque ville possède son propre coffre : achat, niveau, capacité et contenu sont indépendants.");
        lines.push_back("Depuis une autre ville, un coffre déjà acheté peut être consulté à distance, mais aucun retrait n'est permis sans être au bon comptoir.");
        lines.push_back("Chaque coffre municipal reste indépendant : le contenu ne voyage pas automatiquement d'une ville à l'autre.");
        lines.push_back("");
        const City* currentCity = City::findById(player.getCurrentCityId());
        for (const City& city : City::getCatalog())
        {
            const CityAccessReport access = CityTravelRules::evaluateAccess(player, city);
            std::string state = city.getId() == player.getCurrentCityId() ? "ACTUELLE" : (access.allowed ? "ACCESSIBLE" : "FERMÉE");
            std::string vaultState = player.hasCityVaultInCity(city.getId())
                ? "coffre niv. " + std::to_string(player.getCityVaultLevelForCity(city.getId())) + " — " + std::to_string(player.getCityVaultUsedSlotsForCity(city.getId())) + "/" + std::to_string(player.getCityVaultCapacityForCity(city.getId()))
                : "aucun coffre acheté";
            const int distance = currentCity == nullptr ? -1 : City::calculateDistanceBetween(*currentCity, city);
            lines.push_back(city.getName() + " — " + city.getGuildName() + " [" + state + "]");
            lines.push_back("  " + city.getDescription());
            lines.push_back("  Distance depuis ici : " + (distance >= 0 ? std::to_string(distance) + " km." : std::string("inconnue.")));
            lines.push_back("  Entrée : " + city.getAccessRequirementText());
            lines.push_back("  Coffre : " + vaultState + ".");
        }
        MessageScreen::show("RÉSEAU DES VILLES", "quest.city_vault.cities", lines, false);
    }


    void showCanonicalJournalSummary(const Player& player)
    {
        std::vector<std::string> lines;
        lines.push_back("Journal moteur canonique : registre interne sauvegardé. Les compteurs viennent d'événements réels du moteur, pas d'une lecture approximative du texte IG.");
        lines.push_back("Vue filtrée : seules les catégories utiles au joueur restent ici. Les incidents, illégal et sanctions sont dans le registre avancé.");

        struct CategoryView
        {
            std::string id;
            std::string title;
        };

        const std::vector<CategoryView> categories = {
            {"ennemis_tues", "Top ennemis tués"},
            {"ennemis_croises", "Top ennemis croisés"},
            {"boss_tues", "Top boss tués"},
            {"materiaux_ramasses", "Top matériaux ramassés"},
            {"consommables_utilises", "Top consommables utilisés"},
            {"categories_armes_utilisees", "Top catégories d'armes utilisées"},
            {"lieux_visites", "Top lieux visités"},
            {"pnj_servis", "Top PNJ servis"},
            {"types_quetes_completees", "Top types de quêtes complétés"},
            {"voyages", "Top routes empruntées"},
            {"taxes_ville", "Taxes de changement de ville"},
            {"coffres_achetes", "Coffres achetés"},
            {"coffres_ameliores", "Coffres améliorés"},
            {"missions_deleguees_reussies", "Missions PNJ réussies"},
            {"profils_pnj_mandates", "Profils PNJ les plus mandatés"},
            {"quetes_postees_acceptees", "Quêtes publiées acceptées"}
        };

        for (const CategoryView& category : categories)
        {
            const std::vector<PlayerJournalRecord> top = player.getTopCanonicalJournalRecords(category.id, 3);
int total = player.getCanonicalJournalCategoryTotal(category.id);
            lines.push_back("");
            lines.push_back(category.title + " — total catégorie complet : " + std::to_string(total));
            if (top.empty())
            {
                lines.push_back("- Aucun événement enregistré.");
                continue;
            }
            for (std::size_t i = 0; i < top.size(); ++i)
            {
                lines.push_back(std::to_string(i + 1) + ". " + top[i].label + " — " + std::to_string(top[i].count) + " fois | dernier jour " + std::to_string(top[i].lastDay) + ".");
            }
        }

        MessageScreen::show("JOURNAL CANONIQUE — TOP 3", "quest.canonical_journal.summary", lines, false);
    }


    void showAdvancedCanonicalJournalSummary(const Player& player)
    {
        std::vector<std::string> lines;
        lines.push_back("Registre avancé : incidents, illégal, sanctions et détails techniques utiles au debug/lore, mais cachés du Top 3 principal pour ne pas polluer l'écran.");
        lines.push_back("Le total reste toujours celui de la catégorie complète, pas seulement le podium.");

        struct CategoryView
        {
            std::string id;
            std::string title;
        };
        const std::vector<CategoryView> categories = {
            {"demandes_illegales_tentees", "Demandes illégales tentées"},
            {"demandes_illegales_lancees", "Demandes illégales lancées"},
            {"demandes_illegales_vols", "Argent volé par demandes illégales"},
            {"demandes_illegales_reussies", "Demandes illégales réussies"},
            {"demandes_illegales_echouees", "Demandes illégales échouées"},
            {"reputation_souterraine", "Réputation souterraine"},
            {"bannissements_guilde_actifs", "Suspensions de guilde"},
            {"probations_guilde_actives", "Probations de guilde"},
            {"amendes_guilde_payees", "Amendes payées"},
            {"reparations_officielles_guilde", "Réparations officielles"},
            {"incidents_groupes_pnj", "Incidents de groupes"},
            {"tentatives_ramasse_miettes", "Ramasse-miettes après combat"},
            {"aides_rares_groupes_combat", "Aides rares en combat"},
            {"rencontres_rares_groupes_route", "Rencontres rares de route"}
        };
        for (const CategoryView& category : categories)
        {
            const int total = player.getCanonicalJournalCategoryTotal(category.id);
            if (total <= 0) continue;
            lines.push_back("");
            lines.push_back(category.title + " — total catégorie complet : " + std::to_string(total));
            const std::vector<PlayerJournalRecord> top = player.getTopCanonicalJournalRecords(category.id, 3);
            for (std::size_t i = 0; i < top.size(); ++i)
            {
                lines.push_back(std::to_string(i + 1) + ". " + top[i].label + " — " + std::to_string(top[i].count) + " fois | dernier jour " + std::to_string(top[i].lastDay) + ".");
            }
        }
        if (lines.size() <= 2)
        {
            lines.push_back("Aucune catégorie avancée enregistrée pour l'instant.");
        }
        MessageScreen::show("REGISTRE AVANCÉ — INCIDENTS", "quest.canonical_journal.advanced", lines, false);
    }

    void showRecentActions(const Player& player)
    {
        std::vector<PlayerJournalRecord> actions;
        for (const PlayerJournalRecord& record : player.getCanonicalJournalRecords())
        {
            if (record.category == "dernieres_actions")
            {
                actions.push_back(record);
            }
        }
        std::sort(actions.begin(), actions.end(), [](const PlayerJournalRecord& a, const PlayerJournalRecord& b) {
            if (a.lastDay != b.lastDay) return a.lastDay > b.lastDay;
            return a.key > b.key;
        });

        std::vector<std::string> lines;
        lines.push_back("Nom joueur : Dernières actions.");
        lines.push_back("Historique court des actions récentes enregistrées pour ce personnage.");
        if (actions.empty())
        {
            lines.push_back("Aucune action récente enregistrée pour l'instant.");
        }
        else
        {
            const std::size_t limit = std::min<std::size_t>(10, actions.size());
            for (std::size_t i = 0; i < limit; ++i)
            {
                lines.push_back(std::to_string(i + 1) + ". " + actions[i].label + " — jour " + std::to_string(actions[i].lastDay) + ".");
            }
        }
        MessageScreen::show("REGISTRE — DERNIÈRES ACTIONS", "quest.canonical_journal.recent_actions", lines, false);
    }

    std::vector<PlayerJournalRecord> getActiveRouteMicroQuests(const Player& player)
    {
        std::vector<PlayerJournalRecord> active;
        for (const PlayerJournalRecord& record : player.getCanonicalJournalRecords())
        {
            if (record.category != "micro_quetes_route_actives") continue;
            bool resolved = false;
            for (const PlayerJournalRecord& resolvedRecord : player.getCanonicalJournalRecords())
            {
                if (resolvedRecord.category == "micro_quetes_route_resolues" && resolvedRecord.key == record.key)
                {
                    resolved = true;
                    break;
                }
            }
            if (!resolved) active.push_back(record);
        }
        return active;
    }

    void openRouteMicroQuestBoard(Player& player)
    {
        while (true)
        {
            const std::vector<PlayerJournalRecord> active = getActiveRouteMicroQuests(player);
            MenuScreen screen("MICRO-QUÊTES DE ROUTE", "quest.route_micro_quests");
            screen.addLine("Rencontres rares de groupes sur la route. Elles restent légères pour rendre le monde vivant sans spammer le joueur.");
            screen.addLine("Actives : " + std::to_string(active.size()) + ".");
            screen.addBackOption("Retour", "quest.route_micro_quests.back");
            for (std::size_t i = 0; i < active.size(); ++i)
            {
                const PlayerJournalRecord& quest = active[i];
                screen.addOption(static_cast<int>(i + 1), quest.label, "Résoudre l'aide ponctuelle. Récompense modeste : rumeur, note de route ou petite somme.", true, "quest.route_micro_quests.resolve");
            }
            const int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choix invalide.");
            Console::clear();
            if (choice == 0) return;
            if (choice < 1 || choice > static_cast<int>(active.size())) continue;

            const PlayerJournalRecord quest = active[static_cast<std::size_t>(choice - 1)];
            const int roll = stableMissionRoll("micro_route:" + quest.key + ":" + std::to_string(player.getWorldDaysElapsed()) + ":" + std::to_string(player.getCanonicalJournalRecords().size()));
            std::vector<std::string> lines;
            lines.push_back("Micro-quête : " + quest.label + ".");
            if (roll < 62)
            {
                const int copper = 35 + player.getLevel() * 4 + roll % 18;
                player.getInventory().earnCopper(copper);
                player.getInventory().addMaterial(MaterialCatalog::createRouteScoutNote(1));
                player.recordCanonicalEvent("micro_quetes_route_reussies", quest.key, quest.label);
                player.recordCanonicalEvent("rumeurs_route_confirmees", player.getCurrentCityId(), "Rumeur confirmée via micro-quête de route");
                lines.push_back("Résultat : réussite. Le groupe repart avec son objectif réglé.");
                lines.push_back("Récompense : " + Money::formatCopper(copper) + " et une note d'éclaireur de route.");
            }
            else if (roll < 88)
            {
                player.getInventory().addMaterial(MaterialCatalog::createRouteScoutNote(1));
                player.recordCanonicalEvent("micro_quetes_route_partielles", quest.key, quest.label);
                lines.push_back("Résultat : partiel. Pas de vraie récompense en argent, mais une note de route utile.");
            }
            else
            {
                const int damage = std::min(std::max(0, player.getHp() - 1), std::max(1, player.getMaxHp() / 12));
                if (damage > 0) player.takeDamage(damage);
                player.recordCanonicalEvent("micro_quetes_route_echouees", quest.key, quest.label);
                lines.push_back("Résultat : échec léger. Personne ne meurt, mais tu perds du temps et quelques PV.");
                lines.push_back("Dégâts subis : " + std::to_string(damage) + ".");
            }
            player.recordCanonicalEvent("micro_quetes_route_resolues", quest.key, quest.label);
            recordRecentAction(player, "micro_route_resolved", "Micro-quête de route résolue : " + quest.label);
            MessageScreen::show("MICRO-QUÊTE RÉSOLUE", "quest.route_micro_quests.done", lines, false);
        }
    }

    void showCityHubOverview(const Player& player)
    {
        MessageScreen::show(
            "VILLE ACTUELLE — HUB",
            "quest.city_hub.overview",
            CityTravelRules::buildCityHubLines(player),
            false
        );
    }


    void showExplorationMapPreview(const Player& player)
    {
        const std::vector<std::string> lines = CityTravelRules::buildExplorationMapLines(player);
        MessageScreen::show("CARTE D'EXPLORATION", "quest.city_travel.exploration_map", lines, false);
    }

    void openGroupedShopShortcut(Player& player)
    {
        ShopMenu::open(player);
    }

    void openCityHubMenu(Player& player)
    {
        const City* city = City::findById(player.getCurrentCityId());
        if (city == nullptr)
        {
            MessageScreen::show("VILLE INCONNUE", "quest.city_hub.unknown", {"Le hub de ville ne peut pas être ouvert sans ville actuelle valide."}, false);
            return;
        }

        while (true)
        {
            const std::vector<CityBuildingPreview> buildings = CityTravelRules::getBuildingsForCity(player, *city);
            MenuScreen screen("VILLE — " + city->getName(), "quest.city_hub.menu");
            screen.addLine("Services de la ville : choisis un bâtiment ou un comptoir.");
            screen.addLine("Chaque lieu garde ses propres services, contacts et conditions d'accès.");
            screen.addBackOption("Retour", "quest.city_hub.menu.back");
            screen.addOption(90, "Résumé de la ville", "Identité locale, ressources, stocks, bâtiments et conditions.", true, "quest.city_hub.summary");
            screen.addOption(91, "Boutiques et comptoirs", "Accès unique : catégories rapides ou liste complète sans doublon.", true, "quest.city_hub.shops.shortcut");
            for (std::size_t i = 0; i < buildings.size(); ++i)
            {
                const CityBuildingPreview& building = buildings[i];
                std::string detail = building.category + " — " + building.contact + " | " + building.detail + " | Ambiance : " + building.pixelArtHint + ".";
                screen.addOption(static_cast<int>(i + 1), building.name, detail, building.unlocked, "quest.city_hub.building." + building.id);
            }

            const int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choix invalide.");
            Console::clear();
            if (choice == 0) return;
            if (choice == 90)
            {
                showCityHubOverview(player);
                continue;
            }
            if (choice == 91)
            {
                openGroupedShopShortcut(player);
                continue;
            }
            if (choice < 1 || choice > static_cast<int>(buildings.size())) continue;

            const CityBuildingPreview building = buildings[static_cast<std::size_t>(choice - 1)];
            if (!building.unlocked)
            {
                MessageScreen::show("BÂTIMENT VERROUILLÉ", "quest.city_hub.building.locked", {building.name + " n'est pas encore accessible.", building.detail}, false);
                continue;
            }

            if (building.id == "guild")
            {
                QuestContractorMenu::openGuildTribunalMenu(player);
            }
            else if (building.id == "vault")
            {
                openCityVault(player);
            }
            else if (building.id == "inn")
            {
                openInnMenu(player);
            }
            else if (building.id == "delegated_office")
            {
                QuestContractorMenu::openDelegatedMissionBoard(player);
            }
            else if (building.id == "gate" || building.id == "mine_lift" || building.id == "frost_gate")
            {
                showExplorationMapPreview(player);
            }
            else if (building.id == "arena")
            {
                MessageScreen::show("ARÈNE DE VILLE", "quest.city_hub.arena", {
                    "Cette arène accueille les combats uniques et les entraînements de la ville.",
                    "Les règles de combat restent identiques à celles des autres affrontements.",
                    "Le maître d'arène tient le registre des affrontements disponibles."
                }, false);
            }
            else if (building.id == "market" || building.id == "harbor" || building.id == "underbridge")
            {
                std::vector<std::string> lines = CityTravelRules::buildLocalCityDifferentiationLines(player);
                lines.insert(lines.begin(), "Commerce local : " + building.name + ".");
                lines.push_back("Ce comptoir permet achat, vente, discussion et services locaux.");
                lines.push_back("Les stocks, prix et conditions sont consignés directement au comptoir.");
                MessageScreen::show("COMMERCE LOCAL", "quest.city_hub.market", lines, false);
                if (building.id == "harbor")
                {
                    ShopMenu::openShopOfType(player, ShopType::Transport);
                }
                else if (building.id == "underbridge")
                {
                    ShopMenu::openShopOfType(player, ShopType::BlackMarket);
                }
                else
                {
                    ShopMenu::open(player);
                }
            }
            else if (building.id == "archives")
            {
                MessageScreen::show("ARCHIVES LOCALES", "quest.city_hub.archives", {
                    "Archives de " + city->getName() + ".",
                    "Rôle : conserver les rumeurs, biomes et connaissances régionales sans tout révéler gratuitement.",
                    "Le comptoir propose livres, cartes, renseignements et autres services de bibliothèque.",
                    "Cartes murales, livres et légendes donnent à la salle son identité d'archives."
                }, false);
                ShopMenu::openShopOfType(player, ShopType::Library);
            }
            else if (building.id == "forge_heavy" || building.id == "bram_forge")
            {
                MessageScreen::show("FORGE LOCALE", "quest.city_hub.forge", {
                    building.name + " — " + building.category + ".",
                    "Contact : " + building.contact + ".",
                    building.detail,
                    "Le comptoir propose achats, ventes, réparations et discussion avec le forgeron."
                }, false);
                ShopMenu::openShopOfType(player, ShopType::Blacksmith);
            }
            else if (building.id == "sanctuary")
            {
                MessageScreen::show("SANCTUAIRE LOCAL", "quest.city_hub.sanctuary", {
                    building.name + " — " + building.category + ".",
                    "Contact : " + building.contact + ".",
                    building.detail,
                    "Le sanctuaire propose soins, bénédictions encadrées et services religieux."
                }, false);
                ShopMenu::openShopOfType(player, ShopType::Church);
            }
            else
            {
                MessageScreen::show("BÂTIMENT LOCAL", "quest.city_hub.building.info", {
                    building.name + " — " + building.category + ".",
                    "Contact : " + building.contact + ".",
                    building.detail,
                    "Ambiance : " + building.pixelArtHint + "."
                }, false);
            }
        }
    }

    void openInnMenu(Player& player)
    {
        while (true)
        {
            const int commonBedCost = EconomyBalance::innCommonBedCostCopper(player.getCurrentCityId(), player.getLevel());
            const int roomCost = EconomyBalance::innSafeRoomCostCopper(player.getCurrentCityId(), player.getLevel());
            const int mealCost = EconomyBalance::innWarmMealCostCopper(player.getCurrentCityId(), player.getLevel());
            MenuScreen screen("AUBERGE — " + currentCityName(player), "quest.city_hub.inn");
            screen.addLine("Repos réel : l'auberge existe pour éviter que les routes deviennent un lit gratuit.");
            screen.addLine("PV : " + std::to_string(player.getHp()) + "/" + std::to_string(player.getMaxHp()) + ".");
            screen.addLine("Argent : " + player.getInventory().getWalletLine() + ".");
            screen.addLine("Fatigue de route estimée : " + std::to_string(std::max(0, player.getCanonicalJournalCategoryTotal("distance_route") / 60 - player.getCanonicalJournalCategoryTotal("repos_auberge") * 2 - player.getCanonicalJournalCategoryTotal("repas_auberge"))) + " cran(s) narratif(s).");
            screen.addBackOption("Retour", "quest.city_hub.inn.back");
            screen.addOption(1, "Lit commun — " + Money::formatCopper(commonBedCost), "Avance jusqu'au lendemain, mais une nuit simple ne dépasse pas 50% PV.", true, "quest.city_hub.inn.common_bed");
            screen.addOption(2, "Chambre sûre — " + Money::formatCopper(roomCost), "Lit cher et chambre fermée : repos plus profond, plafonné à 90% PV.", true, "quest.city_hub.inn.room");
            screen.addOption(3, "Repas chaud — " + Money::formatCopper(mealCost), "Petit soin et baisse narrative de fatigue sans dormir. Jamais gratuit : le prix est affiché avant validation.", true, "quest.city_hub.inn.meal");
            screen.addOption(4, "Comptoir de l'auberge", "Acheter, vendre, discuter avec l'aubergiste et utiliser les services détaillés de l'auberge.", true, "quest.city_hub.inn.shop");
            screen.addOption(5, "Fond de l'écurie — gratuit", "Un coin de paille offert aux voyageurs sans le sou. Avance au lendemain, récupération très limitée (25% PV max).", true, "quest.city_hub.inn.stable_floor");
            const int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choix invalide.");
            Console::clear();
            if (choice == 0) return;

            if (choice == 1)
            {
                if (!player.getInventory().spendCopper(commonBedCost))
                {
                    MessageScreen::show("ARGENT INSUFFISANT", "quest.city_hub.inn.no_money", {"Coût : " + Money::formatCopper(commonBedCost) + ".", "Argent actuel : " + player.getInventory().getWalletLine() + "."}, false);
                    continue;
                }
                player.advanceWorldDays(1);
                const int targetHp = std::max(1, player.getMaxHp() * 50 / 100);
                if (player.getHp() < targetHp)
                {
                    player.heal(targetHp - player.getHp());
                }
                player.recordCanonicalEvent("repos_auberge", player.getCurrentCityId(), "Lit commun à " + currentCityName(player));
                if (stableMissionRoll("inn_common:" + player.getCurrentCityId() + ":" + std::to_string(player.getWorldDaysElapsed())) < 9)
                {
                    player.recordCanonicalEvent("evenements_nocturnes_auberge", player.getCurrentCityId(), "Bruit, voisin bizarre ou rumeur pendant la nuit d'auberge");
                }
                recordRecentAction(player, "inn_common_bed", "Repos en lit commun à " + currentCityName(player));
                MessageScreen::show("REPOS À L'AUBERGE", "quest.city_hub.inn.common_bed.done", {"Tu dors dans un lit commun. Ce n'est pas luxueux, et une simple nuit ne répare pas tout.", "Plafond de récupération : 50% des PV maximum.", "Prix payé : " + Money::formatCopper(commonBedCost) + ".", "PV actuels : " + std::to_string(player.getHp()) + "/" + std::to_string(player.getMaxHp()) + ".", player.formatWorldDateTimeLine()}, false);
                return;
            }
            if (choice == 2)
            {
                if (!player.getInventory().spendCopper(roomCost))
                {
                    MessageScreen::show("ARGENT INSUFFISANT", "quest.city_hub.inn.no_money", {"Coût : " + Money::formatCopper(roomCost) + ".", "Argent actuel : " + player.getInventory().getWalletLine() + "."}, false);
                    continue;
                }
                player.advanceWorldDays(1);
                const int targetHp = std::max(1, player.getMaxHp() * 90 / 100);
                if (player.getHp() < targetHp)
                {
                    player.heal(targetHp - player.getHp());
                }
                player.recordCanonicalEvent("repos_auberge", player.getCurrentCityId(), "Chambre sûre à " + currentCityName(player));
                player.recordCanonicalEvent("nuits_securisees", player.getCurrentCityId(), "Chambre sûre à " + currentCityName(player));
                player.recordCanonicalEvent("fatigue_route_reduite", player.getCurrentCityId(), "Chambre sûre : fatigue de route calmée");
                recordRecentAction(player, "inn_safe_room", "Chambre sûre à " + currentCityName(player));
                MessageScreen::show("CHAMBRE SÛRE", "quest.city_hub.inn.room.done", {"Tu prends une vraie chambre. Les portes ferment, le lit tient debout, et personne ne fouille ton sac dans le couloir.", "Lit cher : récupération possible jusqu'à 90% des PV, pas une guérison parfaite gratuite.", "Prix payé : " + Money::formatCopper(roomCost) + ".", "PV actuels : " + std::to_string(player.getHp()) + "/" + std::to_string(player.getMaxHp()) + ".", player.formatWorldDateTimeLine()}, false);
                return;
            }
            if (choice == 3)
            {
                if (!player.getInventory().spendCopper(mealCost))
                {
                    MessageScreen::show("ARGENT INSUFFISANT", "quest.city_hub.inn.no_money", {"Coût : " + Money::formatCopper(mealCost) + ".", "Argent actuel : " + player.getInventory().getWalletLine() + "."}, false);
                    continue;
                }
                player.advanceWorldDayUnits(1);
                player.heal(std::max(2, player.getMaxHp() / 6));
                player.recordCanonicalEvent("repas_auberge", player.getCurrentCityId(), "Repas chaud à " + currentCityName(player));
                player.recordCanonicalEvent("fatigue_route_reduite", player.getCurrentCityId(), "Repas chaud : petite récupération sans dormir");
                recordRecentAction(player, "inn_meal", "Repas chaud à " + currentCityName(player));
                MessageScreen::show("REPAS CHAUD", "quest.city_hub.inn.meal.done", {"Tu prends un repas chaud. Ce n'est pas un sommeil complet, mais ça évite de traiter la fatigue comme une ligne invisible.", "Prix payé : " + Money::formatCopper(mealCost) + ".", "PV actuels : " + std::to_string(player.getHp()) + "/" + std::to_string(player.getMaxHp()) + ".", player.formatWorldDateTimeLine()}, false);
                continue;
            }
            if (choice == 4)
            {
                ShopMenu::openShopOfType(player, ShopType::Lodging);
                continue;
            }
            if (choice == 5)
            {
                player.advanceWorldDays(1);
                const int targetHp = std::max(1, player.getMaxHp() * 25 / 100);
                if (player.getHp() < targetHp)
                {
                    player.heal(targetHp - player.getHp());
                }
                player.recordCanonicalEvent("repos_auberge", player.getCurrentCityId(), "Nuit gratuite dans l'écurie à " + currentCityName(player));
                recordRecentAction(player, "inn_stable_floor", "Nuit gratuite dans l'écurie à " + currentCityName(player));
                MessageScreen::show(
                    "PAILLE DE L'ÉCURIE",
                    "quest.city_hub.inn.stable_floor.done",
                    {
                        "L'aubergiste te laisse un coin sec au fond de l'écurie. Ce n'est pas confortable, mais personne n'est obligé de rester bloqué dehors faute d'argent.",
                        "Prix payé : gratuit.",
                        "Récupération maximale : 25% des PV. Les lits payants restent nettement meilleurs.",
                        "PV actuels : " + std::to_string(player.getHp()) + "/" + std::to_string(player.getMaxHp()) + ".",
                        player.formatWorldDateTimeLine()
                    },
                    false
                );
                return;
            }
        }
    }

    int travelTimeUnitsForDistance(int distanceKm)
    {
        return EconomyBalance::travelTimeUnitsForDistance(distanceKm);
    }

    void openCityTravelMenu(Player& player)
    {
        while (true)
        {
            const City* origin = City::findById(player.getCurrentCityId());
            MenuScreen screen("RELAIS DES ROUTES", "quest.city_travel");
            screen.addLine("Ville actuelle : " + currentCityName(player) + ".");
            screen.addLine("Chaque ville possède ses propres distances, biomes proches, coffres, stocks et conditions d'entrée.");
            screen.addLine("Les villes fermées restent visibles, mais les gardes peuvent refuser l'entrée.");
            screen.addBackOption("Retour aux lieux", "quest.city_travel.back");

            std::vector<std::string> destinationIds;
            int option = 1;
            for (const City& city : City::getCatalog())
            {
                if (city.getId() == player.getCurrentCityId())
                {
                    continue;
                }

                const int distance = origin == nullptr ? -1 : City::calculateDistanceBetween(*origin, city);
                const CityAccessReport access = CityTravelRules::evaluateAccess(player, city);
                std::string detail = "Distance : " + (distance >= 0 ? std::to_string(distance) + " km" : std::string("inconnue"));
                detail += access.allowed ? " | Entrée possible." : " | Entrée fermée pour l'instant.";
                detail += " " + city.getAccessRequirementText();

                MenuOptionItemData itemData;
                itemData.structured = true;
                itemData.kind = "city_destination";
                itemData.section = "Villes";
                itemData.actionType = access.allowed ? "travel" : "inspect";
                itemData.name = city.getName();
                itemData.detail = city.getDescription();
                itemData.status = access.allowed ? "Accessible" : "Fermée";
                itemData.stock = distance >= 0 ? std::to_string(distance) + " km" : "Distance inconnue";
                itemData.progress = "Niveau requis " + std::to_string(city.getMinimumLevel()) + " | " + std::to_string(EconomyBalance::travelTimeUnitsForDistance(distance)) + " segment(s)";
                itemData.reward = "Taxe changement de ville " + std::to_string(CityTravelRules::getTravelTaxCopper(player, city, distance)) + " cuivre";
                itemData.owner = city.getGuildName();
                itemData.important = !access.allowed;

                screen.addOption(option, city.getName(), detail, true, "quest.city_travel.destination." + city.getId(), itemData);
                destinationIds.push_back(city.getId());
                ++option;
            }

            screen.addOption(89, "Voir la ville actuelle", "Bâtiments locaux, services et conditions d'accès.", true, "quest.city_travel.city_hub");
            screen.addOption(90, "Voir la carte d'exploration", "Distances vers les biomes et zones encore inconnues.", true, "quest.city_travel.map_preview");
            screen.addOption(92, "Voir le registre des événements", "Événements enregistrés par lieu et catégorie.", true, "quest.city_travel.canonical_journal");
            screen.addOption(93, "Voir les dernières actions", "Historique court des actions récentes du personnage.", true, "quest.city_travel.recent_actions");
            screen.addOption(94, "Bureau des missions déléguées", "Payer des aventuriers/PNJ pour une mission avec coût, jours et taux de réussite.", true, "quest.city_travel.delegated_missions");
            screen.addOption(95, "Carte schématique", "Voir une carte simple des routes, villes et biomes connus.", true, "quest.city_travel.schematic_map");
            screen.addOption(96, "Registre avancé / incidents", "Infractions, sanctions, aides rares et événements secondaires.", true, "quest.city_travel.canonical_journal_advanced");
            screen.addOption(97, "Auberge locale", "Lit, chambre sûre ou repas chaud. Repos réel pour ne pas remplacer l'auberge par la route.", true, "quest.city_travel.inn");
            screen.addOption(98, "Micro-quêtes de route", "Résoudre les aides rares proposées par des groupes croisés pendant un trajet.", true, "quest.city_travel.route_micro_quests");

            const int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choix invalide.");
            Console::clear();

            if (choice == 0)
            {
                return;
            }
            if (choice == 89)
            {
                openCityHubMenu(player);
                continue;
            }
            if (choice == 90)
            {
                showExplorationMapPreview(player);
                continue;
            }
            if (choice == 92)
            {
                showCanonicalJournalSummary(player);
                continue;
            }
            if (choice == 93)
            {
                showRecentActions(player);
                continue;
            }
            if (choice == 94)
            {
                QuestContractorMenu::openDelegatedMissionBoard(player);
                continue;
            }
            if (choice == 95)
            {
                MessageScreen::show("CARTE SCHÉMATIQUE", "quest.city_travel.schematic_map", CityTravelRules::buildSchematicMapLines(player), false);
                continue;
            }
            if (choice == 96)
            {
                showAdvancedCanonicalJournalSummary(player);
                continue;
            }
            if (choice == 97)
            {
                openInnMenu(player);
                continue;
            }
            if (choice == 98)
            {
                openRouteMicroQuestBoard(player);
                continue;
            }
            if (choice < 1 || choice > static_cast<int>(destinationIds.size()))
            {
                continue;
            }

            const City* destination = City::findById(destinationIds[choice - 1]);
            if (destination == nullptr)
            {
                MessageScreen::show("DESTINATION INCONNUE", "quest.city_travel.unknown", {"Cette destination n'existe pas dans le réseau connu."}, false);
                continue;
            }

            const CityAccessReport access = CityTravelRules::evaluateAccess(player, *destination);
            std::vector<std::string> previewLines = CityTravelRules::buildTravelPreviewLines(player, *destination);
            if (!access.allowed)
            {
                MessageScreen::show("VILLE FERMÉE", "quest.city_travel.locked", previewLines, false);
                continue;
            }

            if (origin == nullptr)
            {
                MessageScreen::show("VILLE ACTUELLE INCONNUE", "quest.city_travel.origin_unknown", {"Le relais ne peut pas calculer une vraie route sans ville de départ."}, false);
                continue;
            }

            const int distance = City::calculateDistanceBetween(*origin, *destination);
            const int baseTravelTaxCopper = CityTravelRules::getTravelTaxCopper(player, *destination, distance);
            const int normalTravelTaxCopper = EconomyBalance::cityChangeTaxCopper(destination->getId(), distance);
            const int baseTimeUnits = travelTimeUnitsForDistance(distance);
            TravelRouteOption selectedRoute = askTravelRouteChoice(player, *origin, *destination, distance, baseTravelTaxCopper, baseTimeUnits);
            if (!selectedRoute.available || selectedRoute.id.empty())
            {
                continue;
            }

            const int travelTaxCopper = std::max(0, baseTravelTaxCopper + selectedRoute.extraCopper);
            const int timeUnits = clampedTravelTimeWithRoute(baseTimeUnits, selectedRoute);

            MenuScreen confirm("VOYAGE VERS " + destination->getName(), "quest.city_travel.confirm");
            confirm.addLine("Le trajet sera validé maintenant dans la sauvegarde du personnage.");
            confirm.addLine("Route choisie : " + selectedRoute.label + ".");
            confirm.addLine("Coût total annoncé : " + Money::formatCopper(travelTaxCopper) + ".");
            confirm.addLine("Temps annoncé : " + std::to_string(timeUnits) + " segment(s).");
            for (const std::string& line : previewLines)
            {
                confirm.addLine(line);
            }
            confirm.addBackOption("Annuler", "quest.city_travel.confirm.back");
            confirm.addOption(1, "Partir", "Changer de ville actuelle et avancer le temps selon la distance et la route choisie.", true, "quest.city_travel.confirm.go");

            const int confirmChoice = TerminalInterface::askMenuChoiceFromOptions(confirm, "Choix invalide.");
            Console::clear();
            if (confirmChoice != 1)
            {
                continue;
            }

            const int dayBefore = player.getWorldDaysElapsed();
            const int unitBefore = player.getWorldDayProgressUnits();
            if (!player.getInventory().spendCopper(travelTaxCopper))
            {
                const bool deniedBeforeNightDeparture = CityTravelRules::isNightTravelClosed(player) && selectedRoute.nightAllowed;
                if (!deniedBeforeNightDeparture)
                {
                    player.advanceWorldDayUnits(timeUnits * 2);
                }
                std::vector<std::string> failedLines = {
                    "Coût annoncé pour " + selectedRoute.label + " : " + Money::formatCopper(travelTaxCopper) + ".",
                    "Argent actuel : " + player.getInventory().getWalletLine() + "."
                };
                if (deniedBeforeNightDeparture)
                {
                    failedLines.push_back("Convoi nocturne refusé au guichet avant départ : pas assez d'argent pour l'escorte.");
                    failedLines.push_back("Aucun temps gratuit n'est avancé ici, pour éviter d'utiliser le convoi raté comme lit d'auberge gratuit.");
                    failedLines.push_back("Les routes normales restent fermées par les gardes pendant la nuit.");
                    player.recordCanonicalEvent("voyages_nocturnes_refuses", origin->getId() + "->" + destination->getId(), origin->getName() + " → " + destination->getName());
                }
                else
                {
                    failedLines.push_back("Le trajet est quand même compté : aller jusqu'aux portes, refus/contrôle administratif, puis retour.");
                    failedLines.push_back("Aucune taxe n'est prélevée, et aucune deuxième taxe n'est ajoutée pour le retour.");
                    failedLines.push_back("Temps écoulé : +" + std::to_string(timeUnits * 2) + " segment(s) (aller-retour).");
                    failedLines.push_back(player.formatWorldTimeChange(dayBefore, unitBefore));
                    failedLines.push_back("Rappel : cette taxe existe uniquement lors d'un vrai changement de ville, pas à chaque exploration.");
                    player.recordCanonicalEvent("voyages_rates", origin->getId() + "->" + destination->getId(), origin->getName() + " → " + destination->getName() + " puis retour");
                    std::vector<std::string> discoveryLines = recordRoutePassageAndMaybeDiscovery(player, *origin, *destination, distance, 2);
                    failedLines.insert(failedLines.end(), discoveryLines.begin(), discoveryLines.end());
                }
                recordRecentAction(player, "travel_failed_tax", "Voyage refusé faute de paiement vers " + destination->getName());
                expireOverdueQuestDeadlines(player, "quest.city_travel.tax.failed", true);
                MessageScreen::show(
                    deniedBeforeNightDeparture ? "CONVOI REFUSÉ" : "TAXE IMPOSSIBLE — ALLER-RETOUR",
                    "quest.city_travel.tax.failed",
                    failedLines,
                    false
                );
                continue;
            }
            player.recordCanonicalEvent("taxes_ville", destination->getId(), "Taxe d'entrée vers " + destination->getName(), travelTaxCopper);
            if (player.hasCityVaultInCity(destination->getId()))
            {
                player.recordCanonicalEvent("reductions_taxe_coffre", destination->getId(), "Réduction coffre municipal à " + destination->getName(), std::max(1, normalTravelTaxCopper - travelTaxCopper));
            }
            player.advanceWorldDayUnits(timeUnits);
            player.setCurrentCityId(destination->getId());
            player.recordCanonicalEvent("lieux_visites", destination->getId(), destination->getName());
            player.recordCanonicalEvent("voyages", origin->getId() + "->" + destination->getId(), origin->getName() + " → " + destination->getName());
            player.recordCanonicalEvent("routes_choisies", selectedRoute.id, selectedRoute.label);
            if (selectedRoute.risky)
            {
                player.recordCanonicalEvent("routes_risquees", selectedRoute.id, selectedRoute.label);
            }
            recordRecentAction(player, "travel_success", "Voyage vers " + destination->getName() + " via " + selectedRoute.label);
            std::vector<std::string> discoveryLines = recordRoutePassageAndMaybeDiscovery(player, *origin, *destination, distance, 1);
            std::vector<std::string> routeEventLines = recordLimitedRouteEventAndRumor(player, *origin, *destination, distance, selectedRoute);
            std::vector<std::string> rareGroupLines = maybeCreateRareRouteAdventurerOffer(player, *origin, *destination, selectedRoute);
            expireOverdueQuestDeadlines(player, "quest.city_travel.done", true);

            std::vector<std::string> resultLines = {
                "Tu arrives à " + destination->getName() + ".",
                "Route utilisée : " + selectedRoute.label + ".",
                "Distance parcourue : " + std::to_string(distance) + " km environ.",
                "Taxe/frais de changement de ville payés : " + Money::formatCopper(travelTaxCopper) + ".",
                "Temps écoulé : +" + std::to_string(timeUnits) + " segment(s).",
                player.formatWorldTimeChange(dayBefore, unitBefore),
                "Le trajet se termine devant les portes de " + destination->getName() + "."
            };
            if (player.hasCityVaultInCity(destination->getId()))
            {
                resultLines.push_back("Réduction appliquée : coffre municipal possédé dans cette ville, taxe divisée par deux.");
            }
            resultLines.insert(resultLines.end(), discoveryLines.begin(), discoveryLines.end());
            resultLines.insert(resultLines.end(), routeEventLines.begin(), routeEventLines.end());
            resultLines.insert(resultLines.end(), rareGroupLines.begin(), rareGroupLines.end());
            if (!player.isRegisteredAtCurrentCityGuild())
            {
                resultLines.push_back("Guilde locale : tu peux demander une mise à niveau d'inscription ici, sans refaire l'inscription complète.");
            }
            MessageScreen::show("VOYAGE TERMINÉ", "quest.city_travel.done", resultLines, false);
        }
    }

    void openVaultMaterialTransferMenu(Player& player)
    {
        if (!player.hasCityVault())
        {
            MessageScreen::show("TRANSPORT IMPOSSIBLE", "quest.city_vault.transfer.no_current", {"Tu dois posséder le coffre de la ville actuelle pour envoyer une pile."}, false);
            return;
        }
        if (player.getCityVault().getMaterialCount() <= 0)
        {
            MessageScreen::show("AUCUN MATÉRIAU", "quest.city_vault.transfer.no_material", {"Le coffre actuel ne contient aucune pile de matériaux à transporter."}, false);
            return;
        }

        std::vector<std::pair<int, const City*>> destinationChoices;
        MenuScreen cityScreen("TRANSPORT DE COFFRE", "quest.city_vault.transfer.city");
        cityScreen.addLine("Transport encadré : les piles de matériaux peuvent être expédiées avec un coût et un coffre de destination.");
        cityScreen.addLine("Le retrait reste impossible à distance : tu envoies depuis le coffre actuel vers un autre coffre possédé.");
        cityScreen.addBackOption("Retour", "quest.city_vault.transfer.city.back");
        int option = 1;
        for (const City& city : City::getCatalog())
        {
            if (city.getId() == player.getCurrentCityId()) continue;
            const bool hasVault = player.hasCityVaultInCity(city.getId());
            const int currentOption = option++;
            if (hasVault) destinationChoices.push_back({currentOption, &city});
            const int distance = CityTravelRules::getDistanceBetweenCities(player.getCurrentCityId(), city.getId());
            cityScreen.addOption(currentOption, city.getName(), hasVault ? "Coffre possédé | distance " + std::to_string(distance) + " km." : "Aucun coffre possédé ici : transport impossible.", hasVault, "quest.city_vault.transfer.city." + city.getId());
        }
        const int cityChoice = TerminalInterface::askMenuChoiceFromOptions(cityScreen, "Choix invalide.");
        Console::clear();
        const City* destination = nullptr;
        for (const auto& destinationChoice : destinationChoices)
        {
            if (destinationChoice.first == cityChoice)
            {
                destination = destinationChoice.second;
                break;
            }
        }
        if (destination == nullptr) return;

        std::vector<std::string> labels;
        for (const Material& material : player.getCityVault().getMaterials())
        {
            const int distance = CityTravelRules::getDistanceBetweenCities(player.getCurrentCityId(), destination->getId());
            const int cost = EconomyBalance::cityVaultMaterialTransferCostCopper(player.getCurrentCityId(), destination->getId(), distance, material.getQuantity());
            labels.push_back(material.getName() + " x" + std::to_string(material.getQuantity()) + " [" + material.getQualityLabel() + "] — coût " + Money::formatCopper(cost));
        }
        const int materialIndex = askVaultEntryChoice("CHOISIR UNE PILE À TRANSPORTER", "quest.city_vault.transfer.material", labels);
        if (materialIndex < 0) return;
        const Material selected = player.getCityVault().getMaterial(materialIndex);
        const int distance = CityTravelRules::getDistanceBetweenCities(player.getCurrentCityId(), destination->getId());
        const int cost = EconomyBalance::cityVaultMaterialTransferCostCopper(player.getCurrentCityId(), destination->getId(), distance, selected.getQuantity());

        MenuScreen confirm("CONFIRMER LE TRANSPORT", "quest.city_vault.transfer.confirm");
        confirm.addLine("Pile : " + selected.getName() + " x" + std::to_string(selected.getQuantity()) + ".");
        confirm.addLine("Destination : " + destination->getName() + ".");
        confirm.addLine("Coût : " + Money::formatCopper(cost) + ".");
        confirm.addLine("Règle de comptoir : le transport concerne les matériaux déposés ici, pas les retraits à distance.");
        confirm.addBackOption("Annuler", "quest.city_vault.transfer.confirm.back");
        confirm.addOption(1, "Envoyer la pile", "Déplace réellement la pile vers le coffre municipal de destination si place disponible.", true, "quest.city_vault.transfer.confirm.send");
        const int confirmChoice = TerminalInterface::askMenuChoiceFromOptions(confirm, "Choix invalide.");
        Console::clear();
        if (confirmChoice != 1) return;

        const bool moved = player.transferMaterialBetweenCityVaults(destination->getId(), materialIndex, selected.getQuantity(), cost);
        if (moved)
        {
            recordRecentAction(player, "vault_material_transfer", "Transport de coffre : " + selected.getName() + " vers " + destination->getName());
        }
        MessageScreen::show(
            moved ? "TRANSPORT LANCÉ" : "TRANSPORT REFUSÉ",
            moved ? "quest.city_vault.transfer.done" : "quest.city_vault.transfer.failed",
            moved
                ? std::vector<std::string>{"Pile déplacée vers " + destination->getName() + ".", "Coût payé : " + Money::formatCopper(cost) + ".", "Le retrait devra se faire dans la ville de destination."}
                : std::vector<std::string>{"Aucune pile n'a bougé.", "Cause possible : argent insuffisant, coffre de destination plein, coffre manquant ou pile invalide."},
            false
        );
    }

    void openCityVault(Player& player)
    {
        while (true)
        {
            MenuScreen screen("COFFRE MUNICIPAL — " + currentCityName(player), "quest.city_vault");
            screen.addLine("Stockage personnel sécurisé : son contenu n'est pas accessible depuis l'inventaire normal.");
            screen.addLine("Une mort ou un vol d'inventaire n'atteint pas les objets déjà déposés ici.");
            screen.addLine("Argent transporté : " + player.getInventory().getWalletLine() + ".");
            screen.addBackOption("Retour aux lieux", "quest.city_vault.back");

            if (!player.hasCityVault())
            {
                screen.addLine("Statut : aucun coffre acheté.");
                screen.addLine("Premier coffre : 12 emplacements.");
                screen.addOption(1, "Acheter le coffre personnel", "Coût : " + Money::formatEconomyUnits(player.getCityVaultPurchaseCost()) + ".", true, "quest.city_vault.purchase");
                screen.addOption(2, "Voir les villes et les règles distantes", "Consulter le réseau municipal sans inventer un voyage encore verrouillé.", true, "quest.city_vault.cities");
                screen.addOption(3, "Consulter un autre coffre", "Lecture seule si un coffre existe ailleurs.", true, "quest.city_vault.remote_browser");

                const int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choix invalide.");
                Console::clear();
                if (choice == 0) return;
                if (choice == 1)
                {
                    if (player.purchaseCityVault())
                    {
                        MessageScreen::show("COFFRE ACHETÉ", "quest.city_vault.purchase.success", {"Niveau 1 débloqué : 12 emplacements sécurisés.", "Les améliorations coûteront progressivement plus cher."}, false);
                    }
                    else
                    {
                        MessageScreen::show("ACHAT IMPOSSIBLE", "quest.city_vault.purchase.failed", {"Fonds insuffisants ou coffre déjà possédé.", "Aucune pièce n'a été dépensée."}, false);
                    }
                }
                else if (choice == 2)
                {
                    showKnownCitiesAndVaultRules(player);
                }
                else if (choice == 3)
                {
                    showRemoteCityVaultBrowser(player);
                }
                continue;
            }

            screen.addLine("Niveau : " + std::to_string(player.getCityVaultLevel()) + "/5.");
            screen.addLine("Occupation : " + std::to_string(player.getCityVaultUsedSlots()) + "/" + std::to_string(player.getCityVaultCapacity()) + " emplacements.");
            screen.addLine("Coût par entrée : arme 3, armure 3, consommable 1, pile de matériau 1.");
            const std::string upgradeHint = player.canUpgradeCityVault()
                ? "Coût : " + Money::formatEconomyUnits(player.getCityVaultUpgradeCost()) + ". Ajoute 8 emplacements."
                : "Niveau maximal atteint.";
            screen.addOption(1, "Améliorer le coffre", upgradeHint, player.canUpgradeCityVault(), "quest.city_vault.upgrade");
            screen.addOption(2, "Consulter le contenu", "Vue complète en lecture seule.", true, "quest.city_vault.contents");
            screen.addOption(3, "Déposer une arme", "Impossible pour l'arme actuellement équipée. Coût : 3 emplacements.", player.getInventory().getWeaponCount() > 0, "quest.city_vault.deposit.weapon");
            screen.addOption(4, "Déposer une armure", "Impossible pour l'armure équipée et la tenue simple. Coût : 3 emplacements.", player.getInventory().getArmorCount() > 0, "quest.city_vault.deposit.armor");
            screen.addOption(5, "Déposer un consommable", "Coût : 1 emplacement.", player.getInventory().getConsumableCount() > 0, "quest.city_vault.deposit.consumable");
            screen.addOption(6, "Déposer une pile de matériau", "Une pile compatible déjà présente n'utilise pas de nouvel emplacement.", player.getInventory().getMaterialCount() > 0, "quest.city_vault.deposit.material");
            screen.addOption(7, "Retirer une arme", "Retourne l'objet dans l'inventaire transporté.", player.getCityVault().getWeaponCount() > 0, "quest.city_vault.withdraw.weapon");
            screen.addOption(8, "Retirer une armure", "Retourne l'objet dans l'inventaire transporté.", player.getCityVault().getArmorCount() > 0, "quest.city_vault.withdraw.armor");
            screen.addOption(9, "Retirer un consommable", "Retourne l'objet dans l'inventaire transporté.", player.getCityVault().getConsumableCount() > 0, "quest.city_vault.withdraw.consumable");
            screen.addOption(10, "Retirer une pile de matériau", "Retourne toute la pile dans l'inventaire transporté.", player.getCityVault().getMaterialCount() > 0, "quest.city_vault.withdraw.material");
            screen.addOption(11, "Voir les villes et les règles distantes", "Réseau municipal et spécialités prévues.", true, "quest.city_vault.cities");
            screen.addOption(12, "Consulter un autre coffre", "Lecture seule : aucun retrait à distance.", true, "quest.city_vault.remote_browser");
            screen.addOption(13, "Transporter une pile vers un autre coffre", "Déplace réellement une pile de matériau vers un autre coffre possédé, avec coût de transport.", player.getCityVault().getMaterialCount() > 0, "quest.city_vault.transfer.material");

            const int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choix invalide.");
            Console::clear();
            if (choice == 0) return;
            if (choice == 1)
            {
                if (player.upgradeCityVault())
                {
                    MessageScreen::show("COFFRE AMÉLIORÉ", "quest.city_vault.upgrade.success", {"Niveau actuel : " + std::to_string(player.getCityVaultLevel()) + ".", "Capacité : " + std::to_string(player.getCityVaultCapacity()) + " emplacements."}, false);
                }
                else
                {
                    MessageScreen::show("AMÉLIORATION IMPOSSIBLE", "quest.city_vault.upgrade.failed", {"Niveau maximal ou or insuffisant.", "Aucune pièce n'a été dépensée."}, false);
                }
                continue;
            }
            if (choice == 2) { showCityVaultContents(player); continue; }
            if (choice == 11) { showKnownCitiesAndVaultRules(player); continue; }
            if (choice == 12) { showRemoteCityVaultBrowser(player); continue; }
            if (choice == 13) { openVaultMaterialTransferMenu(player); continue; }

            std::vector<std::string> labels;
            int index = -1;
            bool moved = false;
            if (choice == 3)
            {
                for (std::size_t i = 0; i < player.getInventory().getWeapons().size(); ++i)
                {
                    const Weapon& weapon = player.getInventory().getWeapons()[i];
                    std::string label = weapon.getName() + " | durabilité " + std::to_string(weapon.getDurability()) + "/" + std::to_string(weapon.getMaxDurability());
                    if (static_cast<int>(i) == player.getEquippedWeaponIndex()) label += " [ÉQUIPÉE — PROTÉGÉE]";
                    labels.push_back(label);
                }
                index = askVaultEntryChoice("DÉPOSER UNE ARME", "quest.city_vault.deposit.weapon.list", labels);
                if (index >= 0) moved = player.depositWeaponInCityVault(index);
            }
            else if (choice == 4)
            {
                for (std::size_t i = 0; i < player.getInventory().getArmors().size(); ++i)
                {
                    const Armor& armor = player.getInventory().getArmors()[i];
                    std::string label = armor.getName() + " | durabilité " + std::to_string(armor.getDurability()) + "/" + std::to_string(armor.getMaxDurability());
                    if (static_cast<int>(i) == player.getEquippedArmorIndex()) label += " [ÉQUIPÉE — PROTÉGÉE]";
                    if (armor.getName() == "Tenue simple") label += " [OBJET DE BASE — PROTÉGÉ]";
                    labels.push_back(label);
                }
                index = askVaultEntryChoice("DÉPOSER UNE ARMURE", "quest.city_vault.deposit.armor.list", labels);
                if (index >= 0) moved = player.depositArmorInCityVault(index);
            }
            else if (choice == 5)
            {
                for (const Consumable& consumable : player.getInventory().getConsumables()) labels.push_back(consumable.getName());
                index = askVaultEntryChoice("DÉPOSER UN CONSOMMABLE", "quest.city_vault.deposit.consumable.list", labels);
                if (index >= 0) moved = player.depositConsumableInCityVault(index);
            }
            else if (choice == 6)
            {
                for (const Material& material : player.getInventory().getMaterials()) labels.push_back(material.getName() + " x" + std::to_string(material.getQuantity()) + " [" + material.getQualityLabel() + "]");
                index = askVaultEntryChoice("DÉPOSER UNE PILE", "quest.city_vault.deposit.material.list", labels);
                if (index >= 0) moved = player.depositMaterialInCityVault(index);
            }
            else if (choice == 7)
            {
                for (const Weapon& weapon : player.getCityVault().getWeapons()) labels.push_back(weapon.getName());
                index = askVaultEntryChoice("RETIRER UNE ARME", "quest.city_vault.withdraw.weapon.list", labels);
                if (index >= 0) moved = player.withdrawWeaponFromCityVault(index);
            }
            else if (choice == 8)
            {
                for (const Armor& armor : player.getCityVault().getArmors()) labels.push_back(armor.getName());
                index = askVaultEntryChoice("RETIRER UNE ARMURE", "quest.city_vault.withdraw.armor.list", labels);
                if (index >= 0) moved = player.withdrawArmorFromCityVault(index);
            }
            else if (choice == 9)
            {
                for (const Consumable& consumable : player.getCityVault().getConsumables()) labels.push_back(consumable.getName());
                index = askVaultEntryChoice("RETIRER UN CONSOMMABLE", "quest.city_vault.withdraw.consumable.list", labels);
                if (index >= 0) moved = player.withdrawConsumableFromCityVault(index);
            }
            else if (choice == 10)
            {
                for (const Material& material : player.getCityVault().getMaterials()) labels.push_back(material.getName() + " x" + std::to_string(material.getQuantity()) + " [" + material.getQualityLabel() + "]");
                index = askVaultEntryChoice("RETIRER UNE PILE", "quest.city_vault.withdraw.material.list", labels);
                if (index >= 0) moved = player.withdrawMaterialFromCityVault(index);
            }

            if (index >= 0)
            {
                MessageScreen::show(
                    moved ? "TRANSFERT VALIDÉ" : "TRANSFERT REFUSÉ",
                    moved ? "quest.city_vault.transfer.success" : "quest.city_vault.transfer.failed",
                    moved
                        ? std::vector<std::string>{"L'objet a été déplacé sans duplication.", "Occupation actuelle : " + std::to_string(player.getCityVaultUsedSlots()) + "/" + std::to_string(player.getCityVaultCapacity()) + "."}
                        : std::vector<std::string>{"L'objet n'a pas bougé.", "Cause possible : objet équipé/protégé, coffre plein ou entrée invalide."},
                    false
                );
            }
        }
    }

} // namespace QuestWorldMenuSupport
