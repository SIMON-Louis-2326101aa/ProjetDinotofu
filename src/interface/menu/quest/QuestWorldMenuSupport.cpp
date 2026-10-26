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
#include "quest/IntercityQuestSystem.hpp"
#include "item/material/MaterialCatalog.hpp"
#include "item/inventory/CarryLoadSystem.hpp"
#include "entity/MonsterCatalog.hpp"
#include "combat/modes/pve/MonsterPveMode.hpp"
#include "progression/DifficultyMode.hpp"
#include "progression/DeathRuleMode.hpp"
#include "economy/EconomyBalance.hpp"
#include "economy/GuildFinanceSystem.hpp"
#include "economy/shop/ShopTransactionSystem.hpp"
#include "economy/Money.hpp"
#include "interface/menu/InventoryMenu.hpp"
#include "interface/menu/shop/ShopMenu.hpp"
#include "interface/menu/shop/ShopCityServiceMenu.hpp"
#include "interface/menu/common/PagedMenu.hpp"
#include "interface/menu/common/MessageScreen.hpp"
#include "interface/TerminalInterface.hpp"
#include "interface/model/MenuScreen.hpp"
#include "world/City.hpp"
#include "world/CityTravelRules.hpp"
#include "world/CityProfileSystem.hpp"
#include "world/LodgingRestSystem.hpp"
#include "world/crime/CrimeContactRelationshipSystem.hpp"
#include "world/crime/CrimeContactDialogueSystem.hpp"
#include "world/crime/CrimeJusticeSystem.hpp"
#include "world/crime/CrimeNetworkCompetitionSystem.hpp"
#include "world/home/PlayerHomeSystem.hpp"
#include "story/HestiaArcSystem.hpp"
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
    void openCityVault(Player& player, DifficultyMode difficulty, DeathRuleMode deathRule);
    void openInnMenu(Player& player);
    void openPlayerHomeMenu(Player& player);
    void openMorphologyProgramMenu(Player& player);
    void openCityHubMenu(Player& player, DifficultyMode difficulty, DeathRuleMode deathRule);
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
            routeScreen.addLine("Temps de base : " + std::to_string(baseTimeUnits) + " moment(s) de la journée.");
            if (CityTravelRules::isNightTravelClosed(player))
            {
                routeScreen.addLine("Nuit : les routes normales sont fermées. Seul un convoi gardé payant peut partir sans casser l'auberge.");
            }
            routeScreen.addBackOption("Annuler", "quest.city_travel.route_choice.back");

            for (std::size_t i = 0; i < routes.size(); ++i)
            {
                const TravelRouteOption& route = routes[i];
                const int routeTax = std::max(0, baseTaxCopper + route.extraCopper);
                const int routeTime = CarryLoadSystem::adjustedActionTimeUnits(player, clampedTravelTimeWithRoute(baseTimeUnits, route));
                std::string detail = route.detail + " | coût total " + Money::formatCopper(routeTax) + " | " + std::to_string(routeTime) + " moment(s) de la journée.";
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
        lines.push_back("Occupation : " + std::to_string(player.getCityVaultUsedSlotsForCity(cityId)) + "/" + std::to_string(player.getCityVaultCapacityForCity(cityId)) + " unités d'encombrement.");
        lines.push_back("Rangement optimisé : -" + std::to_string((player.hasCityVaultPlaceOptimizationForCity(cityId) ? 10 : 0) + (player.hasCityVaultPreparedStorageForCity(cityId) ? 15 : 0)) + " % de la valeur de base utilisée.");
        if (remoteReadOnly)
        {
            lines.push_back("Consultation distante : lecture seule. Aucun retrait n'est possible depuis une autre ville.");
        }
        lines.push_back("");

        const Inventory& vault = player.getCityVaultForCity(cityId);
        lines.push_back("Monnaie physique : " + vault.getWalletLine() + " — encombrement " + std::to_string(CarryLoadSystem::coinLoad(vault)) + ".");
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
        if (vault.getWeaponCount() + vault.getArmorCount() + vault.getConsumableCount() + vault.getMaterialCount() == 0 && vault.getTotalCopper() <= 0)
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
                ? "Lecture seule : " + std::to_string(player.getCityVaultUsedSlotsForCity(city.getId())) + "/" + std::to_string(player.getCityVaultCapacityForCity(city.getId())) + " unités."
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
            lines.push_back("  " + CityProfileSystem::profileSummary(city.getId()));
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
                lines.push_back(std::to_string(i + 1) + ". " + top[i].label + " — " + std::to_string(top[i].count) + " fois | dernier jour " + std::to_string(top[i].lastDay + 1) + ".");
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
                lines.push_back(std::to_string(i + 1) + ". " + top[i].label + " — " + std::to_string(top[i].count) + " fois | dernier jour " + std::to_string(top[i].lastDay + 1) + ".");
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
                lines.push_back(std::to_string(i + 1) + ". " + actions[i].label + " — jour " + std::to_string(actions[i].lastDay + 1) + ".");
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

    void openCityHubMenu(Player& player, DifficultyMode difficulty, DeathRuleMode deathRule)
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
                openCityVault(player, difficulty, deathRule);
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

    void openMorphologyProgramMenu(Player& player)
    {
        constexpr int sessionCostCopper = 35;
        constexpr int requiredSessions = 8;
        constexpr int requiredDays = 14;

        while (true)
        {
            const std::string current = player.getBodyMorphology();
            MenuScreen screen("PROGRAMME MORPHOLOGIQUE", "quest.city_hub.inn.morphology");
            screen.addLine("Morphologie actuelle : " + current + ".");
            screen.addLine("Le changement est volontaire, progressif et bidirectionnel : aucun repas ou entraînement isolé ne transforme automatiquement le personnage.");
            screen.addLine("Un palier demande " + std::to_string(requiredSessions) + " séances encadrées ET au moins " + std::to_string(requiredDays) + " jours depuis le début.");
            screen.addLine("Chaque séance combine entraînement adapté + alimentation suivie, coûte " + Money::formatCopper(sessionCostCopper) + " et consomme 2 moments de journée.");
            screen.addBackOption("Retour à l'auberge", "quest.city_hub.inn.morphology.back");

            if (!player.hasActiveMorphologyProgram())
            {
                screen.addLine("Programme actif : aucun.");
                if (current == "Standard")
                {
                    screen.addOption(1, "Viser Fine", "Travail de mobilité, endurance légère et alimentation contrôlée. Passage d'un seul palier.", true, "quest.city_hub.inn.morphology.start.fine");
                    screen.addOption(2, "Viser Robuste", "Renforcement progressif, récupération et alimentation adaptée. Passage d'un seul palier.", true, "quest.city_hub.inn.morphology.start.robust");
                }
                else
                {
                    screen.addOption(1, "Revenir vers Standard", "Un changement extrême ne saute jamais directement Fine <-> Robuste.", true, "quest.city_hub.inn.morphology.start.standard");
                }

                const int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choix invalide.");
                Console::clear();
                if (choice == 0) return;
                std::string target;
                if (current == "Standard") target = choice == 1 ? "Fine" : "Robuste";
                else target = "Standard";
                if (player.startMorphologyProgram(target))
                {
                    MessageScreen::show("PROGRAMME COMMENCÉ", "quest.city_hub.inn.morphology.started", {
                        "Objectif : " + current + " → " + target + ".",
                        "Aucun effet immédiat : la morphologie actuelle reste " + current + " tant que le programme n'est pas réellement terminé.",
                        "Les séances sont limitées à une par journée du monde."
                    }, false);
                }
                continue;
            }

            const int elapsedDays = std::max(0, player.getWorldDaysElapsed() - player.getMorphologyProgramStartedDay());
            screen.addLine("Programme actif : " + current + " → " + player.getMorphologyProgramTarget() + ".");
            screen.addLine("Séances : " + std::to_string(player.getMorphologyProgramSessions()) + "/" + std::to_string(requiredSessions) + ".");
            screen.addLine("Temps écoulé : " + std::to_string(elapsedDays) + "/" + std::to_string(requiredDays) + " jour(s) minimum.");
            screen.addOption(1, "Faire la séance du jour — " + Money::formatCopper(sessionCostCopper),
                player.canPerformMorphologySession() ? "Entraînement + alimentation suivie, puis 2 moments de journée s'écoulent." : "Une séance a déjà été faite aujourd'hui.",
                player.canPerformMorphologySession(), "quest.city_hub.inn.morphology.session");
            screen.addOption(2, "Annuler le programme", "Aucun changement corporel n'est appliqué ; la progression de ce programme est abandonnée.", true, "quest.city_hub.inn.morphology.cancel");

            const int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choix invalide.");
            Console::clear();
            if (choice == 0) return;
            if (choice == 2)
            {
                player.cancelMorphologyProgram();
                MessageScreen::show("PROGRAMME ANNULÉ", "quest.city_hub.inn.morphology.cancelled", {"La morphologie actuelle ne change pas."}, false);
                continue;
            }
            if (choice == 1)
            {
                if (!player.getInventory().spendCopper(sessionCostCopper))
                {
                    MessageScreen::show("FONDS INSUFFISANTS", "quest.city_hub.inn.morphology.no_money", {"Coût de la séance : " + Money::formatCopper(sessionCostCopper) + ".", "Aucune séance n'est enregistrée."}, false);
                    continue;
                }
                if (!player.performMorphologySession())
                {
                    player.getInventory().earnCopper(sessionCostCopper);
                    continue;
                }
                const int beforeDay = player.getWorldDaysElapsed();
                const int beforeUnit = player.getWorldDayProgressUnits();
                player.advanceWorldDayUnits(2);
                const bool completed = player.tryCompleteMorphologyProgram();
                std::vector<std::string> lines = {
                    "Séance validée : entraînement adapté + alimentation suivie.",
                    "Progression : " + std::to_string(completed ? requiredSessions : player.getMorphologyProgramSessions()) + "/" + std::to_string(requiredSessions) + " séance(s).",
                    player.formatWorldTimeChange(beforeDay, beforeUnit)
                };
                if (completed) lines.push_back("Transformation terminée : morphologie actuelle = " + player.getBodyMorphology() + ".");
                else lines.push_back("Aucun changement brutal : le corps conserve encore sa morphologie actuelle pendant le programme.");
                MessageScreen::show(completed ? "PROGRAMME TERMINÉ" : "SÉANCE TERMINÉE", completed ? "quest.city_hub.inn.morphology.complete" : "quest.city_hub.inn.morphology.session.done", lines, false);
            }
        }
    }

    void openPlayerHomeMenu(Player& player)
    {
        while (true)
        {
            const std::string cityId = player.getCurrentCityId();
            const std::string cityName = currentCityName(player);
            const PlayerHomeState& home = player.getHomeState();

            if (!home.owned)
            {
                const PlayerHomePurchasePlan rentPlan = PlayerHomeSystem::planPurchase(player, cityId, false);
                const PlayerHomePurchasePlan buyPlan = PlayerHomeSystem::planPurchase(player, cityId, true);
                MenuScreen purchase("LOGEMENT PRINCIPAL — " + cityName, "quest.city_hub.home.purchase");
                purchase.addLine("Tu ne peux posséder qu'une seule maison principale par personnage.");
                purchase.addLine("La formule avec loyer est moins chère à l'entrée. Un loyer impayé suspend seulement les services gratuits : aucune dette automatique, aucune expulsion et aucun effet sur Hestia ou un conjoint.");
                purchase.addLine("Argent : " + player.getInventory().getWalletLine() + ".");
                purchase.addBackOption("Retour", "quest.city_hub.home.purchase.back");
                purchase.addOption(1,
                    "Prendre le logement avec loyer — " + Money::formatCopper(rentPlan.upfrontCost),
                    "Loyer : " + Money::formatCopper(rentPlan.recurringRent) + " tous les 7 jours. Dormir chez soi devient gratuit tant que les services sont actifs.",
                    rentPlan.valid,
                    "quest.city_hub.home.purchase.rent");
                purchase.addOption(2,
                    "Acheter définitivement — " + Money::formatCopper(buyPlan.upfrontCost),
                    "Beaucoup plus cher, mais aucun loyer récurrent.",
                    buyPlan.valid,
                    "quest.city_hub.home.purchase.outright");
                const int buyChoice = TerminalInterface::askMenuChoiceFromOptions(purchase, "Choix invalide.");
                Console::clear();
                if (buyChoice == 0) return;

                const bool outright = buyChoice == 2;
                const PlayerHomePurchasePlan chosen = outright ? buyPlan : rentPlan;
                if (!chosen.valid || !PlayerHomeSystem::registerPurchase(player, cityId, outright, player.getWorldDaysElapsed()))
                {
                    MessageScreen::show(
                        "ACHAT IMPOSSIBLE",
                        "quest.city_hub.home.purchase.failed",
                        {"Coût : " + Money::formatCopper(chosen.upfrontCost) + ".", "Argent actuel : " + player.getInventory().getWalletLine() + ".", "Aucun crédit ni dette n'est créé pour acheter une maison."},
                        false);
                    continue;
                }

                MessageScreen::show(
                    "MAISON PRINCIPALE",
                    "quest.city_hub.home.purchase.done",
                    {
                        "Le logement de " + cityName + " devient ta seule maison principale.",
                        outright ? "Achat définitif : aucun loyer." : "Premier loyer prévu au jour " + std::to_string(player.getHomeState().nextRentDueDay) + ".",
                        "Dormir chez toi ne coûte rien lorsque les services du logement sont actifs."
                    },
                    false);
                continue;
            }

            if (home.cityId != cityId)
            {
                MessageScreen::show(
                    "MAISON PRINCIPALE",
                    "quest.city_hub.home.other_city",
                    {"Ta maison se trouve à " + home.cityId + ".", "Tu dois être dans cette ville pour y entrer ou utiliser ses services."},
                    false);
                return;
            }

            const bool rentDue = PlayerHomeSystem::isRentDue(player, player.getWorldDaysElapsed());
            const bool active = PlayerHomeSystem::servicesActive(player, player.getWorldDaysElapsed());
            MenuScreen screen("CHEZ TOI — " + cityName, "quest.city_hub.home");
            screen.addLine(home.fullyPurchased ? "Maison achetée définitivement." : "Maison avec loyer.");
            if (!home.fullyPurchased)
            {
                screen.addLine("Loyer : " + Money::formatCopper(home.rentAmount) + " | prochaine échéance : jour " + std::to_string(home.nextRentDueDay) + ".");
                if (rentDue) screen.addLine("Services suspendus tant que le loyer actuel n'est pas payé. Aucun arriéré ne s'accumule.");
            }
            for (const std::string& line : PlayerHomeSystem::describeResidents(player)) screen.addLine(line);
            screen.addBackOption("Retour", "quest.city_hub.home.back");
            screen.addOption(1, "Dormir chez soi — gratuit", active ? "Une nuit complète, récupération jusqu'à 90% PV et mana complet." : "Indisponible tant que le loyer courant n'est pas payé.", active, "quest.city_hub.home.sleep");
            if (rentDue)
                screen.addOption(2, "Payer le loyer — " + Money::formatCopper(home.rentAmount), "Réactive immédiatement les services pour 7 jours. Aucun arriéré supplémentaire.", true, "quest.city_hub.home.rent");

            const PlayerHestiaStoryState& h = player.getHestiaStoryState();
            if (h.rescued && h.materialized)
            {
                if (h.residence != "player_home")
                    screen.addOption(3, "Proposer à Hestia de rester ici", "Elle peut vivre ici avec toi et un éventuel conjoint, sans entretien obligatoire ni pénalité d'absence.", true, "quest.city_hub.home.hestia.move_in");
                else
                {
                    screen.addOption(3, "Passer voir Hestia", "Moment facultatif : aucune relation ne baisse si tu ne le fais pas.", true, "quest.city_hub.home.hestia.visit");
                    screen.addOption(4, "Demander à Hestia de voyager avec toi", "Elle redevient disponible dans l'équipe active.", true, "quest.city_hub.home.hestia.party");
                    screen.addOption(5, "Confier Hestia à Diane", "Elle sera en sécurité à la guilde. Cela ne bloque rien et ne crée aucune obligation de visite.", true, "quest.city_hub.home.hestia.diane");
                }
            }

            const int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choix invalide.");
            Console::clear();
            if (choice == 0) return;

            if (choice == 1 && active)
            {
                player.advanceWorldDays(1);
                const int targetHp = std::max(1, player.getMaxHp() * 90 / 100);
                if (player.getHp() < targetHp) player.heal(targetHp - player.getHp());
                player.restoreManaToFull();
                player.recordCanonicalEvent("repos_maison", cityId, "Nuit gratuite dans la maison principale", 1);
                MessageScreen::show("NUIT CHEZ TOI", "quest.city_hub.home.sleep.done", {"Tu dors dans ton propre logement.", "Coût : gratuit.", "PV : " + std::to_string(player.getHp()) + "/" + std::to_string(player.getMaxHp()) + ".", "Mana restauré.", player.formatWorldDateTimeLine()}, false);
                continue;
            }
            if (choice == 2 && rentDue)
            {
                if (!PlayerHomeSystem::payRent(player, player.getWorldDaysElapsed()))
                {
                    MessageScreen::show("LOYER NON PAYÉ", "quest.city_hub.home.rent.failed", {"Montant : " + Money::formatCopper(home.rentAmount) + ".", "Argent actuel : " + player.getInventory().getWalletLine() + ".", "Aucune dette n'est créée : les services restent simplement suspendus."}, false);
                }
                else
                {
                    MessageScreen::show("LOYER PAYÉ", "quest.city_hub.home.rent.done", {"Services réactivés.", "Nouvelle échéance : jour " + std::to_string(player.getHomeState().nextRentDueDay) + "."}, false);
                }
                continue;
            }
            if (choice == 3 && h.rescued && h.materialized)
            {
                if (h.residence != "player_home")
                {
                    HestiaArcSystem::moveResidence(player, "player_home", true);
                    MessageScreen::show("HESTIA À LA MAISON", "quest.city_hub.home.hestia.move_in.done", {"Hestia peut rester ici autant qu'elle le souhaite.", "Tu n'as aucune jauge d'entretien à remplir et aucune absence ne fait baisser votre relation."}, false);
                }
                else
                {
                    Random random;
                    std::vector<std::string> lines = HestiaArcSystem::visit(player, player.getWorldDaysElapsed(), random);
                    HestiaSmallFind find = HestiaArcSystem::maybeBringSmallFind(player, player.getWorldDaysElapsed(), random);
                    lines.insert(lines.end(), find.lines.begin(), find.lines.end());
                    lines.push_back("Cette visite est entièrement facultative : ne pas revenir ne provoque aucune pénalité.");
                    MessageScreen::show("HESTIA", "quest.city_hub.home.hestia.visit.done", lines, false);
                }
                continue;
            }
            if (choice == 4 && h.rescued && h.materialized && h.residence == "player_home")
            {
                HestiaArcSystem::moveResidence(player, "active_party", true);
                continue;
            }
            if (choice == 5 && h.rescued && h.materialized && h.residence == "player_home")
            {
                HestiaArcSystem::moveResidence(player, "guild_diane", true);
                continue;
            }
        }
    }

    void openInnMenu(Player& player)
    {
        while (true)
        {
            const int roomCost = EconomyBalance::innSafeRoomCostCopper(player.getCurrentCityId(), player.getLevel());
            const int mealVoucherCount = player.getInventory().countMaterialById("warm_meal_voucher");
            const int bedVoucherCount = player.getInventory().countMaterialById("lodging_bed_token");

            MenuScreen screen("AUBERGE — " + currentCityName(player), "quest.city_hub.inn");
            screen.addLine("Repos réel : l'auberge existe pour éviter que les routes deviennent un lit gratuit.");
            screen.addLine("PV : " + std::to_string(player.getHp()) + "/" + std::to_string(player.getMaxHp()) + ".");
            screen.addLine("Argent : " + player.getInventory().getWalletLine() + ".");
            screen.addLine("Bons : repas chaud x" + std::to_string(mealVoucherCount) + " | lit commun x" + std::to_string(bedVoucherCount) + ".");
            screen.addLine("Fatigue de route estimée : " + std::to_string(std::max(0, player.getCanonicalJournalCategoryTotal("distance_route") / 60 - player.getCanonicalJournalCategoryTotal("repos_auberge") * 2 - player.getCanonicalJournalCategoryTotal("repas_auberge"))) + " cran(s) narratif(s).");
            screen.addBackOption("Retour", "quest.city_hub.inn.back");
            screen.addOption(1, "Utiliser l'auberge", "Repas chaud, lit commun, bons prépayés, abonnements et services d'écurie. Les bons présents sont utilisés avant un paiement direct.", true, "quest.city_hub.inn.services");
            screen.addOption(2, "Chambre sûre — " + Money::formatCopper(roomCost), "Service premium sans bon générique : chambre fermée, nuit complète, récupération jusqu'à 90% PV.", true, "quest.city_hub.inn.room");
            screen.addOption(3, "Comptoir de l'auberge", "Acheter ou vendre, discuter avec l'aubergiste et acheter des carnets de bons sans les consommer immédiatement.", true, "quest.city_hub.inn.shop");
            const StableFloorAccess stableFloor = LodgingRestSystem::stableFloorAccess(player, player.getCurrentCityId());
            std::string stableFloorHint = "Un coin de paille offert aux voyageurs sans le sou. Avance au lendemain, récupération très limitée (25% PV max).";
            if (stableFloor.banned)
            {
                stableFloorHint = "Accès refusé encore " + std::to_string(stableFloor.banDaysRemaining) + " jour(s) après abus du couchage gratuit.";
            }
            else if (stableFloor.evictedForOveruse)
            {
                stableFloorHint = "Tu as déjà trop utilisé ce dépannage récemment : une nouvelle demande entraînera une interdiction temporaire.";
            }
            else if (stableFloor.recentFreeNights > 0)
            {
                stableFloorHint += " Utilisations récentes : " + std::to_string(stableFloor.recentFreeNights) + "/3 tolérées avant expulsion.";
            }
            screen.addOption(4, "Fond de l'écurie — gratuit", stableFloorHint, !stableFloor.banned, "quest.city_hub.inn.stable_floor");
            screen.addOption(5, "Programme morphologique", "Changer Fine / Standard / Robuste uniquement par un programme volontaire, long et suivi.", true, "quest.city_hub.inn.morphology");
            screen.addOption(6, player.getHomeState().owned ? "Maison principale" : "Chercher un logement principal", player.getHomeState().owned ? "Accéder à ton logement s'il se trouve dans cette ville." : "Acheter une seule maison principale : formule avec loyer ou achat définitif.", true, "quest.city_hub.inn.home");

            const int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choix invalide.");
            Console::clear();
            if (choice == 0) return;

            if (choice == 1)
            {
                ShopCityServiceMenu::openLodgingServiceMenu(player);
                continue;
            }
            if (choice == 2)
            {
                GuildFinancePaymentResult payment = GuildFinanceSystem::payAffiliatedService(
                    player,
                    roomCost,
                    "affiliated_inn_safe_room:" + player.getCurrentCityId(),
                    "Chambre sûre à " + currentCityName(player),
                    false
                );

                if (!payment.success)
                {
                    const GuildFinanceOverview finance = GuildFinanceSystem::overview(player);
                    const long long wallet = player.getInventory().getTotalCopper();
                    const long long creditNeeded = std::max(0LL, static_cast<long long>(roomCost) - wallet);
                    if (finance.member && creditNeeded > 0 && finance.overdraftRemainingCopper >= creditNeeded)
                    {
                        MenuScreen creditScreen("CRÉDIT DE GUILDE", "quest.city_hub.inn.room.credit");
                        creditScreen.addLine("Coût de la chambre : " + Money::formatCopper(roomCost) + ".");
                        creditScreen.addLine("Bourse disponible : " + Money::formatCopper(wallet) + ".");
                        creditScreen.addLine("Crédit nécessaire : " + Money::formatCopper(creditNeeded) + ".");
                        creditScreen.addLine("Découvert utilisé : " + Money::formatCopper(finance.overdraftUsedCopper) + ".");
                        creditScreen.addLine("Marge restante avant paiement : " + Money::formatCopper(finance.overdraftRemainingCopper) + ".");
                        creditScreen.addBackOption("Annuler", "quest.city_hub.inn.room.credit.cancel");
                        creditScreen.addOption(1, "Utiliser le découvert", "La bourse est prélevée d'abord ; la guilde règle uniquement le manque au partenaire.", true, "quest.city_hub.inn.room.credit.confirm");
                        const int creditChoice = TerminalInterface::askMenuChoiceFromOptions(creditScreen, "Choisis si tu veux utiliser le crédit de guilde.");
                        Console::clear();
                        if (creditChoice == 1)
                        {
                            payment = GuildFinanceSystem::payAffiliatedService(
                                player,
                                roomCost,
                                "affiliated_inn_safe_room:" + player.getCurrentCityId(),
                                "Chambre sûre à " + currentCityName(player),
                                true
                            );
                        }
                    }

                    if (!payment.success)
                    {
                        const GuildFinanceOverview afterFailure = GuildFinanceSystem::overview(player);
                        MessageScreen::show(
                            "ARGENT INSUFFISANT",
                            "quest.city_hub.inn.no_money",
                            {
                                "Coût : " + Money::formatCopper(roomCost) + ".",
                                "Argent actuel : " + player.getInventory().getWalletLine() + ".",
                                "Découvert utilisé : " + Money::formatCopper(afterFailure.overdraftUsedCopper) + ".",
                                "Marge de découvert : " + Money::formatCopper(afterFailure.overdraftRemainingCopper) + ".",
                                payment.failureReason.empty() ? "Aucun paiement n'a été effectué." : payment.failureReason
                            },
                            false
                        );
                        continue;
                    }
                }
                player.advanceWorldDays(1);
                const int targetHp = std::max(1, player.getMaxHp() * 90 / 100);
                if (player.getHp() < targetHp)
                {
                    player.heal(targetHp - player.getHp());
                }
                player.restoreManaToFull();
                player.recordCanonicalEvent("repos_auberge", player.getCurrentCityId(), "Chambre sûre à " + currentCityName(player));
                player.recordCanonicalEvent("nuits_securisees", player.getCurrentCityId(), "Chambre sûre à " + currentCityName(player));
                player.recordCanonicalEvent("fatigue_route_reduite", player.getCurrentCityId(), "Chambre sûre : fatigue de route calmée");
                recordRecentAction(player, "inn_safe_room", "Chambre sûre à " + currentCityName(player));
                std::vector<std::string> roomLines = {
                    "Tu prends une vraie chambre. Les portes ferment, le lit tient debout, et personne ne fouille ton sac dans le couloir.",
                    "Lit cher : récupération possible jusqu'à 90% des PV, pas une guérison parfaite gratuite.",
                    "Prix payé : " + Money::formatCopper(roomCost) + ".",
                    "PV actuels : " + std::to_string(player.getHp()) + "/" + std::to_string(player.getMaxHp()) + ".",
                    "Mana : " + std::to_string(player.getMana()) + "/" + std::to_string(player.getMaxMana()) + "."
                };
                if (payment.overdraftDrawnCopper > 0)
                {
                    roomLines.push_back("Paiement : " + Money::formatCopper(payment.walletPaidCopper) + " depuis la bourse + " + Money::formatCopper(payment.overdraftDrawnCopper) + " de crédit réseau.");
                    roomLines.push_back("Découvert utilisé après paiement : " + Money::formatCopper(GuildFinanceSystem::overdraftUsedCopper(player)) + ".");
                }
                roomLines.push_back(player.formatWorldDateTimeLine());
                MessageScreen::show("CHAMBRE SÛRE", "quest.city_hub.inn.room.done", roomLines, false);
                return;
            }
            if (choice == 3)
            {
                ShopMenu::openShopOfType(player, ShopType::Lodging);
                continue;
            }
            if (choice == 5)
            {
                openMorphologyProgramMenu(player);
                continue;
            }
            if (choice == 6)
            {
                openPlayerHomeMenu(player);
                continue;
            }
            if (choice == 4)
            {
                const StableFloorAccess access = LodgingRestSystem::requestStableFloorRest(player, player.getCurrentCityId());
                if (!access.allowed)
                {
                    std::vector<std::string> lines;
                    if (access.evictedForOveruse)
                    {
                        lines.push_back("L'aubergiste te reconnaît avant même que tu montres la paille du doigt.");
                        lines.push_back("Ce coin gratuit sert à dépanner les voyageurs sans le sou, pas à remplacer indéfiniment une chambre ou un logement.");
                        lines.push_back("Tu es délogé avant de t'installer et interdit de couchage gratuit ici pendant 3 jours.");
                    }
                    else
                    {
                        lines.push_back("L'écurie refuse encore de te laisser reprendre le même coin gratuitement.");
                        lines.push_back("Interdiction restante : " + std::to_string(access.banDaysRemaining) + " jour(s).");
                    }
                    lines.push_back("Les chambres payantes, ton propre logement et les autres solutions de repos restent disponibles normalement.");
                    MessageScreen::show("ÉCURIE — ACCÈS REFUSÉ", "quest.city_hub.inn.stable_floor.denied", lines, false);
                    continue;
                }

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
                        "Utilisations gratuites récentes dans cette ville : " + std::to_string(access.recentFreeNights) + "/3 tolérées dans la fenêtre anti-abus.",
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

    void openCityTravelMenu(Player& player, DifficultyMode difficulty, DeathRuleMode deathRule)
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
                itemData.progress = "Niveau requis " + std::to_string(city.getMinimumLevel()) + " | " + std::to_string(EconomyBalance::travelTimeUnitsForDistance(distance)) + " moment(s) de la journée";
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
                openCityHubMenu(player, difficulty, deathRule);
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

            const CarryLoadSummary departureLoad = CarryLoadSystem::evaluate(player);
            if (departureLoad.movementBlocked)
            {
                MessageScreen::show("TROP CHARGÉ POUR PARTIR", "quest.city_travel.carry_blocked", {
                    "Encombrement : " + std::to_string(departureLoad.used) + "/" + std::to_string(departureLoad.capacity) + " — " + std::to_string(departureLoad.percent) + " %.",
                    "À 200 % ou plus, le personnage ne peut pas commencer un voyage.",
                    "Allège le sac : vendre, déposer, donner, consommer ou abandonner des objets reste possible."
                }, false);
                continue;
            }

            const int travelTaxCopper = std::max(0, baseTravelTaxCopper + selectedRoute.extraCopper);
            const int rawTimeUnits = clampedTravelTimeWithRoute(baseTimeUnits, selectedRoute);
            const int timeUnits = CarryLoadSystem::adjustedActionTimeUnits(player, rawTimeUnits);

            MenuScreen confirm("VOYAGE VERS " + destination->getName(), "quest.city_travel.confirm");
            confirm.addLine("Le trajet sera validé maintenant dans la sauvegarde du personnage.");
            confirm.addLine("Route choisie : " + selectedRoute.label + ".");
            confirm.addLine("Coût total annoncé : " + Money::formatCopper(travelTaxCopper) + ".");
            confirm.addLine("Temps annoncé : " + std::to_string(timeUnits) + " moment(s) de la journée.");
            if (timeUnits > rawTimeUnits) confirm.addLine("Surcharge : " + std::to_string(rawTimeUnits) + " → " + std::to_string(timeUnits) + " moment(s) (+" + std::to_string(departureLoad.explorationTimeModifierPercent) + " %)." );
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
                    failedLines.push_back("Temps écoulé : +" + std::to_string(timeUnits * 2) + " moment(s) de la journée (aller-retour).");
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
            std::vector<std::string> intercityQuestLines = IntercityQuestSystem::onCityArrival(player, origin->getId(), destination->getId());
            expireOverdueQuestDeadlines(player, "quest.city_travel.done", true);

            std::vector<std::string> resultLines = {
                "Tu arrives à " + destination->getName() + ".",
                "Route utilisée : " + selectedRoute.label + ".",
                "Distance parcourue : " + std::to_string(distance) + " km environ.",
                "Taxe/frais de changement de ville payés : " + Money::formatCopper(travelTaxCopper) + ".",
                "Temps écoulé : +" + std::to_string(timeUnits) + " moment(s) de la journée.",
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
            resultLines.insert(resultLines.end(), intercityQuestLines.begin(), intercityQuestLines.end());
            if (!player.isRegisteredAtCurrentCityGuild())
            {
                resultLines.push_back("Guilde locale : tu peux demander une mise à niveau d'inscription ici, sans refaire l'inscription complète.");
            }
            MessageScreen::show("VOYAGE TERMINÉ", "quest.city_travel.done", resultLines, false);
        }
    }

    int previewVaultUsedForCity(const Player& player, const std::string& cityId, const Inventory& inventory)
    {
        const int raw = CarryLoadSystem::inventoryLoad(inventory);
        int reduction = 0;
        if (player.hasCityVaultPlaceOptimizationForCity(cityId)) reduction += 10;
        if (player.hasCityVaultPreparedStorageForCity(cityId)) reduction += 15;
        return std::max(0, (raw * (100 - reduction) + 99) / 100);
    }

    std::string vaultCityName(const std::string& cityId)
    {
        const City* city = City::findById(cityId);
        return city == nullptr ? cityId : city->getName();
    }

    bool runCityVaultDefense(Player& player, DifficultyMode difficulty, DeathRuleMode deathRule)
    {
        if (!player.hasCityVaultRaidPendingForCity(player.getCurrentCityId())) return false;

        Random random;
        const bool monsterRaid = random.between(1, 100) <= 8;
        const int enemyCount = player.getLevel() >= 45 ? 3 : (player.getLevel() >= 12 ? 2 : 1);
        std::vector<Monster> enemies;
        for (int i = 0; i < enemyCount; ++i)
        {
            Monster scaled = MonsterCatalog::createRandomMonsterForLevel(std::max(1, player.getLevel() + random.between(-2, 2)), random);
            if (monsterRaid)
            {
                enemies.push_back(scaled);
            }
            else
            {
                const std::string name = i == 0 ? "Pillard du coffre" : (i == 1 ? "Voleur de réserve" : "Brigand de soutien");
                enemies.emplace_back(
                    name,
                    "Humain voleur organisé",
                    Race::Humain,
                    scaled.getLevel(),
                    scaled.getMaxHp(),
                    scaled.getMinDamage(),
                    scaled.getMaxDamage(),
                    scaled.getCriticalDamage(),
                    scaled.getHealingPotionCount(),
                    scaled.getDamagePotionCount(),
                    false,
                    i == enemyCount - 1 && enemyCount >= 3
                );
            }
        }

        const bool guardMayJoin = random.between(1, 100) <= 35;
        std::vector<std::string> intro = {
            monsterRaid
                ? "Exceptionnellement, la menace n'est pas une bande de voleurs classique : des créatures ont profité de l'agitation autour du coffre."
                : "Les assaillants sont principalement des voleurs et brigands venus pour le stockage, pas pour conquérir la ville.",
            "La garde locale sécurise les rues ; ton combat se concentre sur ceux qui ont réellement atteint ton coffre."
        };
        if (guardMayJoin) intro.push_back("Un garde municipal est disponible : s'il reste une vraie place dans ton équipe, il rejoindra temporairement la défense sans demander de part du butin.");
        MessageScreen::show("DÉFENSE DU COFFRE", "quest.city_vault.raid.intro", intro, false);

        const bool victory = MonsterPveMode::runExplorationWave(
            player, random, difficulty, deathRule, enemies,
            "Défense du coffre municipal de " + currentCityName(player), false, guardMayJoin, 40, 40
        );
        player.resolveCurrentCityVaultRaid(victory);
        if (victory)
        {
            player.recordCanonicalEvent("services_locaux_reussis", "defense_coffre:" + player.getCurrentCityId(), "Défense du coffre municipal", 1);
            MessageScreen::show("COFFRE SAUVÉ", "quest.city_vault.raid.victory", {"La tentative de pillage est stoppée.", "Les autorités reprennent les assaillants et la majorité de leurs biens : seuls environ 40 % du butin ordinaire sont légalement récupérables.", "Tout bien qui t'appartenait déjà et qui est identifié comme volé est repris en priorité et ne compte pas dans cette limite.", "Le garde éventuel repart ensuite à son poste : il ne devient pas une recrue permanente et ne prend aucun équipement."}, false);
        }
        else
        {
            MessageScreen::show("DÉFENSE ÉCHOUÉE", "quest.city_vault.raid.defeat", {"Les assaillants réussissent à repartir avec une petite part de la monnaie physique accessible.", "Le système ne détruit pas silencieusement les objets uniques, de quête ou équipements irremplaçables."}, false);
        }
        return victory;
    }

    void openVaultMaterialTransferMenu(Player& player)
    {
        if (!player.hasCityVault())
        {
            MessageScreen::show("TRANSPORT IMPOSSIBLE", "quest.city_vault.transfer.no_current", {"Tu dois posséder le coffre de la ville actuelle pour utiliser le réseau logistique."}, false);
            return;
        }

        auto hasCargo = [](const Inventory& vault)
        {
            return vault.getWeaponCount() > 0 || vault.getArmorCount() > 0 || vault.getConsumableCount() > 0
                || vault.getMaterialCount() > 0 || vault.getTotalCopper() > 0;
        };

        MenuScreen mode("TRANSPORT INTER-VILLE", "quest.city_vault.transfer.mode");
        mode.addLine("Envoi : le coffre actuel expédie vers un autre coffre. Rappel : un messager porte d'abord ta demande, puis le convoi revient vers toi.");
        mode.addLine("Le coût dépend de la distance et de l'encombrement réel du chargement. Un rappel coûte un peu plus cher et prend environ deux trajets au lieu d'un.");
        mode.addBackOption("Retour", "quest.city_vault.transfer.mode.back");
        mode.addOption(1, "Envoyer depuis ce coffre", "Un seul trajet logistique vers le coffre distant.", hasCargo(player.getCityVault()), "quest.city_vault.transfer.mode.send");
        mode.addOption(2, "Rappeler depuis un autre coffre", "Messager aller + convoi retour : un peu plus cher et plus long.", player.getOwnedCityVaultCount() > 1, "quest.city_vault.transfer.mode.recall");
        const int modeChoice = TerminalInterface::askMenuChoiceFromOptions(mode, "Choix invalide.");
        Console::clear();
        if (modeChoice == 0) return;
        const bool recall = modeChoice == 2;

        std::vector<std::pair<int, const City*>> cityChoices;
        MenuScreen cityScreen(recall ? "RAPPELER DEPUIS QUEL COFFRE ?" : "ENVOYER VERS QUEL COFFRE ?", recall ? "quest.city_vault.transfer.recall.city" : "quest.city_vault.transfer.send.city");
        cityScreen.addBackOption("Retour", "quest.city_vault.transfer.city.back");
        int option = 1;
        for (const City& city : City::getCatalog())
        {
            if (city.getId() == player.getCurrentCityId()) continue;
            const bool hasVault = player.hasCityVaultInCity(city.getId());
            const int currentOption = option++;
            if (hasVault) cityChoices.push_back({currentOption, &city});
            const int distance = CityTravelRules::getDistanceBetweenCities(player.getCurrentCityId(), city.getId());
            const int used = hasVault ? player.getCityVaultUsedSlotsForCity(city.getId()) : 0;
            const int capacity = hasVault ? player.getCityVaultCapacityForCity(city.getId()) : 0;
            const std::string detail = hasVault
                ? ("Coffre " + std::to_string(used) + "/" + std::to_string(capacity) + " | distance " + std::to_string(distance) + " km.")
                : "Aucun coffre possédé ici.";
            cityScreen.addOption(currentOption, city.getName(), detail, hasVault, "quest.city_vault.transfer.city." + city.getId());
        }
        const int cityChoice = TerminalInterface::askMenuChoiceFromOptions(cityScreen, "Choix invalide.");
        Console::clear();
        const City* remoteCity = nullptr;
        for (const auto& entry : cityChoices)
        {
            if (entry.first == cityChoice) { remoteCity = entry.second; break; }
        }
        if (remoteCity == nullptr) return;

        const std::string sourceCityId = recall ? remoteCity->getId() : player.getCurrentCityId();
        const std::string destinationCityId = recall ? player.getCurrentCityId() : remoteCity->getId();
        const Inventory& sourceVault = player.getCityVaultForCity(sourceCityId);
        if (!hasCargo(sourceVault))
        {
            MessageScreen::show("COFFRE SOURCE VIDE", "quest.city_vault.transfer.empty", {"Ce coffre ne contient actuellement rien à transporter."}, false);
            return;
        }

        MenuScreen category("TYPE DE CHARGEMENT", recall ? "quest.city_vault.transfer.recall.category" : "quest.city_vault.transfer.send.category");
        category.addLine("Les équipements gardent leur identité exacte pendant le transport. La monnaie conserve ses dénominations physiques.");
        category.addBackOption("Retour", "quest.city_vault.transfer.category.back");
        category.addOption(1, "Arme", std::to_string(sourceVault.getWeaponCount()) + " disponible(s).", sourceVault.getWeaponCount() > 0, "quest.city_vault.transfer.weapon");
        category.addOption(2, "Armure", std::to_string(sourceVault.getArmorCount()) + " disponible(s).", sourceVault.getArmorCount() > 0, "quest.city_vault.transfer.armor");
        category.addOption(3, "Consommable", std::to_string(sourceVault.getConsumableCount()) + " disponible(s).", sourceVault.getConsumableCount() > 0, "quest.city_vault.transfer.consumable");
        category.addOption(4, "Pile de matériau", std::to_string(sourceVault.getMaterialCount()) + " pile(s).", sourceVault.getMaterialCount() > 0, "quest.city_vault.transfer.material");
        category.addOption(5, "Toute la monnaie physique", sourceVault.getTotalCopper() > 0 ? sourceVault.getWalletLine() : "Aucune monnaie.", sourceVault.getTotalCopper() > 0, "quest.city_vault.transfer.coins");
        const int categoryChoice = TerminalInterface::askMenuChoiceFromOptions(category, "Choix invalide.");
        Console::clear();
        if (categoryChoice == 0) return;

        Inventory sourcePreview = sourceVault;
        Inventory destinationPreview = player.getCityVaultForCity(destinationCityId);
        int selectedIndex = -1;
        int selectedQuantity = 1;
        int cargoLoad = 1;
        std::string cargoLabel;

        if (categoryChoice == 1)
        {
            std::vector<std::string> labels;
            for (int i = 0; i < sourceVault.getWeaponCount(); ++i)
            {
                const Weapon w = sourceVault.getWeapon(i);
                labels.push_back(w.getName() + " — encombrement " + std::to_string(CarryLoadSystem::weaponLoad(w)));
            }
            selectedIndex = askVaultEntryChoice(recall ? "ARME À RAPPELER" : "ARME À ENVOYER", "quest.city_vault.transfer.weapon.pick", labels);
            if (selectedIndex < 0) return;
            const Weapon selected = sourceVault.getWeapon(selectedIndex);
            cargoLabel = selected.getName();
            cargoLoad = CarryLoadSystem::weaponLoad(selected);
            sourcePreview.removeWeapon(selectedIndex);
            destinationPreview.addWeapon(selected);
        }
        else if (categoryChoice == 2)
        {
            std::vector<std::string> labels;
            for (int i = 0; i < sourceVault.getArmorCount(); ++i)
            {
                const Armor a = sourceVault.getArmor(i);
                labels.push_back(a.getName() + " — encombrement " + std::to_string(CarryLoadSystem::armorLoad(a)));
            }
            selectedIndex = askVaultEntryChoice(recall ? "ARMURE À RAPPELER" : "ARMURE À ENVOYER", "quest.city_vault.transfer.armor.pick", labels);
            if (selectedIndex < 0) return;
            const Armor selected = sourceVault.getArmor(selectedIndex);
            cargoLabel = selected.getName();
            cargoLoad = CarryLoadSystem::armorLoad(selected);
            sourcePreview.removeArmor(selectedIndex);
            destinationPreview.addArmor(selected);
        }
        else if (categoryChoice == 3)
        {
            std::vector<std::string> labels;
            for (int i = 0; i < sourceVault.getConsumableCount(); ++i)
                labels.push_back(sourceVault.getConsumable(i).getName());
            selectedIndex = askVaultEntryChoice(recall ? "CONSOMMABLE À RAPPELER" : "CONSOMMABLE À ENVOYER", "quest.city_vault.transfer.consumable.pick", labels);
            if (selectedIndex < 0) return;
            const Consumable selected = sourceVault.getConsumable(selectedIndex);
            cargoLabel = selected.getName();
            Inventory one; one.addConsumable(selected);
            cargoLoad = std::max(1, CarryLoadSystem::inventoryLoad(one));
            sourcePreview.removeConsumable(selectedIndex);
            destinationPreview.addConsumable(selected);
        }
        else if (categoryChoice == 4)
        {
            std::vector<std::string> labels;
            for (const Material& material : sourceVault.getMaterials())
                labels.push_back(material.getName() + " x" + std::to_string(material.getQuantity()) + " [" + material.getQualityLabel() + "] — encombrement " + std::to_string(CarryLoadSystem::materialLoad(material)));
            selectedIndex = askVaultEntryChoice(recall ? "PILE À RAPPELER" : "PILE À ENVOYER", "quest.city_vault.transfer.material.pick", labels);
            if (selectedIndex < 0) return;
            const Material selected = sourceVault.getMaterial(selectedIndex);
            cargoLabel = selected.getName() + " x" + std::to_string(selected.getQuantity());
            selectedQuantity = selected.getQuantity();
            cargoLoad = CarryLoadSystem::materialLoad(selected);
            sourcePreview.removeMaterialQuantity(selectedIndex, selectedQuantity);
            destinationPreview.addMaterial(selected);
        }
        else if (categoryChoice == 5)
        {
            cargoLabel = sourceVault.getWalletLine();
            cargoLoad = std::max(1, CarryLoadSystem::coinLoad(sourceVault));
            const CoinBreakdown coins = sourceVault.getCoinStacks();
            sourcePreview.setCoinStacks(CoinBreakdown{});
            destinationPreview.earnCoinStacks(coins);
        }
        else return;

        const int distance = CityTravelRules::getDistanceBetweenCities(sourceCityId, destinationCityId);
        int cost = EconomyBalance::cityVaultCargoTransferCostCopper(sourceCityId, destinationCityId, distance, cargoLoad);
        const bool reuseRecallMessenger = recall && player.canReuseRecentCityVaultRecallMessenger(sourceCityId, destinationCityId);
        if (recall) cost += player.getCityVaultRecallMessengerSurchargeCopper(sourceCityId, destinationCityId, distance);
        const int travelUnits = std::max(1, EconomyBalance::travelTimeUnitsForDistance(distance));
        const int timeUnits = recall ? travelUnits * 2 : travelUnits;
        const int sourceBefore = player.getCityVaultUsedSlotsForCity(sourceCityId);
        const int sourceAfter = previewVaultUsedForCity(player, sourceCityId, sourcePreview);
        const int destinationBefore = player.getCityVaultUsedSlotsForCity(destinationCityId);
        const int destinationAfter = previewVaultUsedForCity(player, destinationCityId, destinationPreview);
        const int destinationCapacity = player.getCityVaultCapacityForCity(destinationCityId);

        MenuScreen confirm(recall ? "CONFIRMER LE RAPPEL" : "CONFIRMER L'ENVOI", recall ? "quest.city_vault.transfer.recall.confirm" : "quest.city_vault.transfer.send.confirm");
        confirm.addLine("Chargement : " + cargoLabel + " — encombrement brut " + std::to_string(cargoLoad) + ".");
        confirm.addLine("Source : " + vaultCityName(sourceCityId) + " — " + std::to_string(sourceBefore) + " → " + std::to_string(sourceAfter) + "/" + std::to_string(player.getCityVaultCapacityForCity(sourceCityId)) + ".");
        confirm.addLine("Destination : " + vaultCityName(destinationCityId) + " — " + std::to_string(destinationBefore) + " → " + std::to_string(destinationAfter) + "/" + std::to_string(destinationCapacity) + ".");
        confirm.addLine("Coût : " + Money::formatCopper(cost) + ".");
        confirm.addLine("Temps logistique : +" + std::to_string(timeUnits) + " moment(s) de journée." + std::string(recall ? " Le rappel inclut le messager qui porte d'abord la demande." : ""));
        if (recall && reuseRecallMessenger) confirm.addLine("Messager mutualisé : une demande récente vers ce même coffre date de moins d'une demi-journée. Seul un petit supplément administratif est facturé.");
        confirm.addBackOption("Annuler", "quest.city_vault.transfer.confirm.back");
        confirm.addOption(1, recall ? "Envoyer la demande de rappel" : "Expédier le chargement", destinationAfter <= destinationCapacity ? "La place de destination est suffisante." : "Destination trop pleine.", destinationAfter <= destinationCapacity, "quest.city_vault.transfer.confirm.go");
        const int confirmChoice = TerminalInterface::askMenuChoiceFromOptions(confirm, "Choix invalide.");
        Console::clear();
        if (confirmChoice != 1) return;

        const int dayBefore = player.getWorldDaysElapsed();
        const int unitBefore = player.getWorldDayProgressUnits();
        bool moved = false;
        if (categoryChoice == 1) moved = player.transferWeaponBetweenOwnedCityVaults(sourceCityId, destinationCityId, selectedIndex, cost);
        else if (categoryChoice == 2) moved = player.transferArmorBetweenOwnedCityVaults(sourceCityId, destinationCityId, selectedIndex, cost);
        else if (categoryChoice == 3) moved = player.transferConsumableBetweenOwnedCityVaults(sourceCityId, destinationCityId, selectedIndex, cost);
        else if (categoryChoice == 4) moved = player.transferMaterialBetweenOwnedCityVaults(sourceCityId, destinationCityId, selectedIndex, selectedQuantity, cost);
        else if (categoryChoice == 5) moved = player.transferAllCoinsBetweenOwnedCityVaults(sourceCityId, destinationCityId, cost);

        if (moved)
        {
            if (recall && !reuseRecallMessenger)
            {
                // Mark the messenger at dispatch time, before logistics time elapses. A second short-route
                // order may still share the courier if less than roughly half a day has passed.
                player.markCityVaultRecallMessengerSent(sourceCityId, destinationCityId);
            }
            player.advanceWorldDayUnits(timeUnits);
            recordRecentAction(player, recall ? "vault_cargo_recall" : "vault_cargo_transfer", (recall ? "Rappel" : "Envoi") + std::string(" de coffre : ") + cargoLabel);
        }
        std::vector<std::string> lines = moved
            ? std::vector<std::string>{"Chargement déplacé sans duplication.", "Coût payé : " + Money::formatCopper(cost) + ".", "Source : " + std::to_string(sourceBefore) + " → " + std::to_string(sourceAfter) + ".", "Destination : " + std::to_string(destinationBefore) + " → " + std::to_string(destinationAfter) + ".", player.formatWorldTimeChange(dayBefore, unitBefore)}
            : std::vector<std::string>{"Aucun chargement n'a bougé.", "Cause possible : argent insuffisant, coffre plein/manquant ou sélection devenue invalide."};
        if (moved)
        {
            std::vector<std::string> timeLines = player.consumeWorldTimeReportLines();
            lines.insert(lines.end(), timeLines.begin(), timeLines.end());
        }
        MessageScreen::show(moved ? (recall ? "RAPPEL EFFECTUÉ" : "ENVOI EFFECTUÉ") : "TRANSPORT REFUSÉ", recall ? "quest.city_vault.transfer.recall.done" : "quest.city_vault.transfer.send.done", lines, false);
    }

    void openVaultTheftContactMenu(Player& player)
    {
        const PlayerVaultTheftCase* theftCase = player.getOpenVaultTheftCaseForCity(player.getCurrentCityId());
        if (theftCase == nullptr)
        {
            MessageScreen::show("PISTE DU CAMBRIOLAGE", "quest.city_vault.theft_contact.none", {"Aucun dossier local non résolu."}, false);
            return;
        }

        MenuScreen screen("PISTE LOCALE DU CAMBRIOLAGE", "quest.city_vault.theft_contact");
        screen.addLine("Dossier : " + theftCase->caseId);
        screen.addLine("Détenteur/piste : " + theftCase->holderLabel);
        if (!theftCase->holderLocationLabel.empty()) screen.addLine("Lieu : " + theftCase->holderLocationLabel);
        screen.addLine("Indices : " + std::to_string(theftCase->clues) + " | observations locales : " + std::to_string(theftCase->holderObservationCount));
        if (theftCase->holderType == "receiver" || theftCase->holderType == "accomplice")
        {
            screen.addLine("Pression : " + std::to_string(theftCase->holderPressure) + "/100 | coopération : " + std::to_string(theftCase->holderCooperation) + "/100.");
            if (!theftCase->holderWorldContactId.empty())
            {
                const CrimeContactRelationshipAssessment relation = CrimeContactRelationshipSystem::assess(player, theftCase->holderWorldContactId);
                if (relation.known)
                {
                    screen.addLine("Relation persistante : " + relation.relationshipLabel
                        + " | fiabilité : " + relation.reliabilityLabel
                        + " | " + relation.debtLabel + ".");

                    const CrimeContactDialogueSnapshot dialogue = CrimeContactDialogueSystem::build(player, theftCase->holderWorldContactId);
                    for (const std::string& line : dialogue.lines)
                        screen.addLine("Contact : " + line);

                    const CrimeJusticeAssessment justice = CrimeJusticeSystem::assessVaultTheftContact(
                        player, *theftCase, theftCase->holderWorldContactId);
                    if (justice.knownContact)
                    {
                        screen.addLine("Dossier judiciaire : score de preuve " + std::to_string(justice.evidenceScore)
                            + (justice.enoughEvidenceForDurableArrest
                                ? " — seuil durable atteint."
                                : " — arrestation durable non justifiée à ce stade."));
                    }
                }
            }
            if (theftCase->holderReportedToGuard) screen.addLine("La garde connaît désormais officiellement ce contact dans ce dossier.");
            if (theftCase->holderClosedUntilDay > player.getWorldDaysElapsed()) screen.addLine("Contact fermé jusqu'au jour " + std::to_string(theftCase->holderClosedUntilDay) + ".");
        }
        if (theftCase->holderAlerted) screen.addLine("Attention : le contact semble déjà méfiant.");
        const CrimeNetworkCompetitionAssessment competition = CrimeNetworkCompetitionSystem::assessKnownCity(
            player, player.getCurrentCityId());
        if (competition.observable)
        {
            screen.addLine("Concurrence criminelle connue : tension " + std::to_string(competition.tension) + "/100 entre "
                + std::to_string(competition.knownNetworks) + " réseaux déjà identifiés localement.");
        }
        screen.addBackOption("Retour", "quest.city_vault.theft_contact.back");
        screen.addOption(1, "Observer discrètement", "Consomme 1 unité de temps. Ajoute une piste locale réelle sans accusation automatique.", true, "quest.city_vault.theft_contact.observe");
        const bool canNegotiate = (theftCase->holderType == "receiver" || theftCase->holderType == "accomplice") && theftCase->clues >= 3;
        const bool canSearchCache = theftCase->holderType == "cache" && theftCase->clues >= 3;
        screen.addOption(2, "Négocier une restitution", canNegotiate ? "Tenter de récupérer les biens sans combat. Aucun paiement si l'accord est refusé." : "Il faut une personne identifiée comme détenteur crédible et au moins 3 indices.", canNegotiate, "quest.city_vault.theft_contact.negotiate");
        const bool canRequestGuard = (theftCase->holderType == "receiver" || theftCase->holderType == "accomplice") && theftCase->clues >= 4;
        screen.addOption(3, "Demander une confrontation par la garde", canRequestGuard ? "Consomme 2 unités de temps. La garde confronte le contact sur les preuves ; coopération, fermeture ou restitution sont possibles." : "Il faut un contact humain et au moins 4 indices cohérents.", canRequestGuard, "quest.city_vault.theft_contact.guard_interrogation");
        screen.addOption(4, "Fouiller la cache", canSearchCache ? "Consomme 2 unités de temps. Une cache peut rendre les biens sans inventer de coupable présent." : "Disponible uniquement lorsqu'une cache est localisée avec assez d'indices.", canSearchCache, "quest.city_vault.theft_contact.search_cache");

        const int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choisis comment exploiter cette piste.");
        Console::clear();
        if (choice <= 0) return;

        Random random;
        const VaultTheftContactAction action = choice == 2
            ? VaultTheftContactAction::NegotiateReturn
            : (choice == 3 ? VaultTheftContactAction::RequestGuardIntervention
                : (choice == 4 ? VaultTheftContactAction::SearchCache : VaultTheftContactAction::Observe));
        const std::string worldContactId = theftCase->holderWorldContactId;
        const VaultTheftContactInteractionResult result = player.interactWithVaultTheftHolder(player.getCurrentCityId(), action, random);
        std::vector<std::string> lines = result.lines;
        if (!worldContactId.empty() && action == VaultTheftContactAction::NegotiateReturn && result.propertyRecovered)
        {
            const CrimeContactInteractionResult relationshipResult = CrimeContactRelationshipSystem::recordReliableDeal(
                player, worldContactId, 0, "Restitution négociée respectée");
            lines.insert(lines.end(), relationshipResult.lines.begin(), relationshipResult.lines.end());
        }
        if (lines.empty()) lines.push_back("Aucun résultat exploitable pour cette approche.");
        if (result.economyCost > 0) lines.push_back("Montant de négociation : " + Money::formatEconomyUnits(result.economyCost) + ".");
        if (result.cluesAfter > 0) lines.push_back("Indices du dossier : " + std::to_string(result.cluesBefore) + " → " + std::to_string(result.cluesAfter) + ".");
        MessageScreen::show(result.propertyRecovered ? "BIENS RÉCUPÉRÉS" : "PISTE EXPLOITÉE", "quest.city_vault.theft_contact.result", lines, false);
    }

    void openCityVault(Player& player, DifficultyMode difficulty, DeathRuleMode deathRule)
    {
        const CarryLoadSummary storageSessionStart = CarryLoadSystem::evaluate(player);
        auto finalizeStorageSessionTitles = [&]() {
            player.evaluateStorageSessionTitles(storageSessionStart.used, storageSessionStart.capacity);
        };

        while (true)
        {
            MenuScreen screen("COFFRE MUNICIPAL — " + currentCityName(player), "quest.city_vault");
            screen.addLine("Stockage personnel sécurisé : son contenu n'est pas accessible depuis l'inventaire normal.");
            screen.addLine("Une mort ou un vol d'inventaire n'atteint pas les objets déjà déposés ici.");
            screen.addLine("Argent transporté : " + player.getInventory().getWalletLine() + ".");
            const CarryLoadSummary carryLoad = CarryLoadSystem::evaluate(player);
            screen.addLine("Encombrement porté : " + carryLoad.label + " — " + std::to_string(carryLoad.used) + "/" + std::to_string(carryLoad.capacity) + " unités simples.");
            screen.addLine("Déposer les doublons et réserves ici réduit l’encombrement sans transformer l’inventaire en gestion de kilos.");
            screen.addBackOption("Retour aux lieux", "quest.city_vault.back");

            if (!player.hasCityVault())
            {
                screen.addLine("Statut : aucun coffre acheté dans cette ville.");
                screen.addLine("Coffre N1 de cette ville : 250 unités d'encombrement sécurisé (plus de 6 fois le sac de départ).");
                screen.addLine(player.getCurrentCityVaultPurchaseDiscountLabel());
                screen.addLine("Chaque coffre supplémentaire coûte au minimum +2 PO par rapport au palier d'achat précédent.");
                screen.addOption(1, "Acheter le coffre personnel", "Coût actuel : " + Money::formatEconomyUnits(player.getCityVaultPurchaseCost()) + ". Investissement patrimonial important.", true, "quest.city_vault.purchase");
                screen.addOption(2, "Voir les villes et les règles distantes", "Consulter le réseau municipal sans inventer un voyage encore verrouillé.", true, "quest.city_vault.cities");
                screen.addOption(3, "Consulter un autre coffre", "Lecture seule si un coffre existe ailleurs.", true, "quest.city_vault.remote_browser");
                screen.addOption(4, "Améliorer le sac à dos", player.canUpgradeBackpack() ? ("Passer de " + std::to_string(player.getBackpackBaseCapacity()) + " à " + std::to_string(CarryLoadSystem::backpackBaseCapacity(player.getBackpackTier() + 1)) + " unités de base — " + Money::formatEconomyUnits(player.getNextBackpackUpgradeCost()) + ".") : "Sac maximal atteint : 80 unités de base.", player.canUpgradeBackpack(), "quest.city_vault.backpack.upgrade");

                const int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choix invalide.");
                Console::clear();
                if (choice == 0) { finalizeStorageSessionTitles(); return; }
                if (choice == 1)
                {
                    if (player.purchaseCityVault())
                    {
                        MessageScreen::show("COFFRE ACHETÉ", "quest.city_vault.purchase.success", {"Niveau 1 débloqué : 250 unités sécurisées.", "Les coffres sont locaux : en acheter dans plusieurs villes crée un vrai réseau de stockage.", "Les améliorations coûteront progressivement plus cher."}, false);
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
                else if (choice == 4)
                {
                    const int before = player.getBackpackBaseCapacity();
                    if (player.upgradeBackpack())
                        MessageScreen::show("NOUVEAU SAC", "quest.city_vault.backpack.success", {"Capacité de base : " + std::to_string(before) + " → " + std::to_string(player.getBackpackBaseCapacity()) + ".", "Race, morphologie, classe pure et Force ajustent ensuite légèrement cette valeur."}, false);
                    else
                        MessageScreen::show("ACHAT IMPOSSIBLE", "quest.city_vault.backpack.failed", {"Sac maximal ou fonds insuffisants."}, false);
                }
                continue;
            }

            screen.addLine("Niveau : " + std::to_string(player.getCityVaultLevel()) + "/5.");
            screen.addLine("Occupation : " + std::to_string(player.getCityVaultUsedSlots()) + "/" + std::to_string(player.getCityVaultCapacity()) + " unités d'encombrement.");
            screen.addLine("Optimisation du rangement : -" + std::to_string(player.getCityVaultStorageReductionPercent()) + " % de la place de base utilisée.");
            screen.addLine("Même logique que le sac : armures/armes lourdes prennent plus de place, piles compactes et monnaie beaucoup moins.");
            const int securityDays = player.getCityVaultSecurityDaysRemainingForCity(player.getCurrentCityId());
            screen.addLine("Sécurité et entretien : " + std::string(securityDays > 0 ? (std::to_string(securityDays) + " jour(s) protégés restant(s), compteur en pause tant que tu restes dans cette ville/alentours.") : "aucune couverture payée ; tant que tu es ici, tu surveilles toi-même le coffre."));
            if (player.hasCityVaultRaidPendingForCity(player.getCurrentCityId()))
            {
                screen.addLine("ALERTE : tentative de pillage active — délai jour " + std::to_string(player.getCityVaultRaidDeadlineDayForCity(player.getCurrentCityId()) + 1) + ".");
            }
            const std::string upgradeHint = player.canUpgradeCityVault()
                ? "Coût : " + Money::formatEconomyUnits(player.getCityVaultUpgradeCost()) + ". Prochaine capacité : " + std::to_string(EconomyBalance::cityVaultCapacityForLevel(player.getCityVaultLevel() + 1)) + "."
                : "Niveau maximal atteint.";
            screen.addOption(1, "Améliorer le coffre", upgradeHint, player.canUpgradeCityVault(), "quest.city_vault.upgrade");
            screen.addOption(2, "Consulter le contenu", "Vue complète en lecture seule.", true, "quest.city_vault.contents");
            screen.addOption(3, "Déposer une arme", "Impossible pour l'arme actuellement équipée. L'encombrement dépend de sa classe de poids.", player.getInventory().getWeaponCount() > 0, "quest.city_vault.deposit.weapon");
            screen.addOption(4, "Déposer une armure", "Impossible pour l'armure équipée et la tenue simple. L'encombrement dépend de sa classe de poids.", player.getInventory().getArmorCount() > 0, "quest.city_vault.deposit.armor");
            screen.addOption(5, "Déposer un consommable", "Les consommables identiques sont compactés par petits lots.", player.getInventory().getConsumableCount() > 0, "quest.city_vault.deposit.consumable");
            screen.addOption(6, "Déposer une pile de matériau", "La place dépend de la quantité et de la densité du matériau.", player.getInventory().getMaterialCount() > 0, "quest.city_vault.deposit.material");
            screen.addOption(7, "Retirer une arme", "Retourne l'objet dans l'inventaire transporté.", player.getCityVault().getWeaponCount() > 0, "quest.city_vault.withdraw.weapon");
            screen.addOption(8, "Retirer une armure", "Retourne l'objet dans l'inventaire transporté.", player.getCityVault().getArmorCount() > 0, "quest.city_vault.withdraw.armor");
            screen.addOption(9, "Retirer un consommable", "Retourne l'objet dans l'inventaire transporté.", player.getCityVault().getConsumableCount() > 0, "quest.city_vault.withdraw.consumable");
            screen.addOption(10, "Retirer une pile de matériau", "Retourne toute la pile dans l'inventaire transporté.", player.getCityVault().getMaterialCount() > 0, "quest.city_vault.withdraw.material");
            screen.addOption(11, "Voir les villes et les règles distantes", "Réseau municipal et spécialités prévues.", true, "quest.city_vault.cities");
            screen.addOption(12, "Consulter un autre coffre", "Lecture seule : aucun retrait à distance.", true, "quest.city_vault.remote_browser");
            screen.addOption(13, "Transport inter-ville / rappel", "Armes, armures, consommables, matériaux et monnaie physique : coût et temps selon distance/encombrement.", player.getOwnedCityVaultCount() > 1, "quest.city_vault.transfer.cargo");
            screen.addOption(14, "Améliorer le sac à dos", player.canUpgradeBackpack() ? ("Prochaine base : " + std::to_string(CarryLoadSystem::backpackBaseCapacity(player.getBackpackTier() + 1)) + " — " + Money::formatEconomyUnits(player.getNextBackpackUpgradeCost()) + ".") : "Sac maximal : 80 unités de base.", player.canUpgradeBackpack(), "quest.city_vault.backpack.upgrade");
            screen.addOption(15, "Place optimisée", player.hasCityVaultPlaceOptimization() ? "Installée : -10 % de place utilisée par rapport à la valeur de base." : ("Disponible dès le niveau 2 — " + Money::formatEconomyUnits(player.getCityVaultPlaceOptimizationCost()) + "."), player.canBuyCityVaultPlaceOptimization(), "quest.city_vault.optimize.place");
            screen.addOption(16, "Rangement anticipé", player.hasCityVaultPreparedStorage() ? "Installé : -15 % supplémentaires par rapport à la valeur de base (-25 % total)." : ("Disponible uniquement au niveau 5 après Place optimisée — " + Money::formatEconomyUnits(player.getCityVaultPreparedStorageCost()) + "."), player.canBuyCityVaultPreparedStorage(), "quest.city_vault.optimize.prepared");
            screen.addOption(17, "Déposer toute la monnaie physique", "Transfère les piles exactes de pièces dans ce coffre. 100 pièces physiques = 1 unité de base.", player.getInventory().getTotalCopper() > 0, "quest.city_vault.deposit.coins");
            screen.addOption(18, "Retirer toute la monnaie physique", "Rend les mêmes dénominations au portefeuille ; peut provoquer une surcharge du sac.", player.getCityVault().getTotalCopper() > 0, "quest.city_vault.withdraw.coins");
            screen.addOption(19, "Sécurité et entretien", "7 jours réellement consommés seulement quand tu es loin — " + Money::formatEconomyUnits(player.getCityVaultSecuritySubscriptionCost()) + ". Cumul limité à 35 jours. Une alerte déjà active doit quand même être défendue.", true, "quest.city_vault.security");
            screen.addOption(20, "Défendre le coffre", player.hasCityVaultRaidPendingForCity(player.getCurrentCityId()) ? "Affronter les pillards signalés par le messager. Un garde peut aider si l'équipe a une place." : "Aucune tentative de pillage active ici.", player.hasCityVaultRaidPendingForCity(player.getCurrentCityId()), "quest.city_vault.defend");
            const PlayerVaultTheftCase* theftCase = player.getOpenVaultTheftCaseForCity(player.getCurrentCityId());
            screen.addOption(21, "Enquêter sur un cambriolage", theftCase != nullptr
                ? (theftCase->culpritIdentified
                    ? ("Cible déjà identifiée : " + theftCase->holderLabel + ". Continuer l'enquête peut confirmer sa piste actuelle.")
                    : ("Auteur inconnu — " + std::to_string(theftCase->clues) + "/3 indice(s) minimum avant identification crédible."))
                : "Aucun dossier de cambriolage non résolu dans cette ville.", theftCase != nullptr, "quest.city_vault.investigate");
            const bool theftContactAvailable = theftCase != nullptr
                && (theftCase->holderType == "receiver" || theftCase->holderType == "accomplice" || theftCase->holderType == "cache")
                && !theftCase->holderLocationLabel.empty();
            screen.addOption(22, "Suivre la piste locale", theftContactAvailable
                ? (theftCase->holderLabel + " — " + theftCase->holderLocationLabel + ". Observer, négocier ou fouiller selon la piste.")
                : "Aucun contact/cache localisé à exploiter pour l'instant.", theftContactAvailable, "quest.city_vault.theft_contact");

            const int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choix invalide.");
            Console::clear();
            if (choice == 0) { finalizeStorageSessionTitles(); return; }
            if (choice == 1)
            {
                if (player.upgradeCityVault())
                {
                    MessageScreen::show("COFFRE AMÉLIORÉ", "quest.city_vault.upgrade.success", {"Niveau actuel : " + std::to_string(player.getCityVaultLevel()) + ".", "Capacité : " + std::to_string(player.getCityVaultCapacity()) + " unités."}, false);
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
            if (choice == 14)
            {
                const int before = player.getBackpackBaseCapacity();
                if (player.upgradeBackpack()) MessageScreen::show("NOUVEAU SAC", "quest.city_vault.backpack.success", {"Capacité de base : " + std::to_string(before) + " → " + std::to_string(player.getBackpackBaseCapacity()) + "."}, false);
                else MessageScreen::show("ACHAT IMPOSSIBLE", "quest.city_vault.backpack.failed", {"Sac maximal ou fonds insuffisants."}, false);
                continue;
            }
            if (choice == 15)
            {
                const bool ok = player.buyCityVaultPlaceOptimization();
                MessageScreen::show(ok ? "PLACE OPTIMISÉE" : "AMÉLIORATION IMPOSSIBLE", ok ? "quest.city_vault.optimize.place.success" : "quest.city_vault.optimize.place.failed", ok ? std::vector<std::string>{"Les objets stockés utilisent désormais exactement 10 % de place de base en moins."} : std::vector<std::string>{"Il faut un coffre niveau 2 minimum, ne pas déjà posséder l'amélioration et avoir les fonds."}, false);
                continue;
            }
            if (choice == 16)
            {
                const bool ok = player.buyCityVaultPreparedStorage();
                MessageScreen::show(ok ? "RANGEMENT ANTICIPÉ" : "AMÉLIORATION IMPOSSIBLE", ok ? "quest.city_vault.optimize.prepared.success" : "quest.city_vault.optimize.prepared.failed", ok ? std::vector<std::string>{"-15 % supplémentaires par rapport à la place de base.", "Avec Place optimisée : -25 % exact au total."} : std::vector<std::string>{"Il faut un coffre niveau 5, Place optimisée et les fonds nécessaires."}, false);
                continue;
            }

            if (choice == 17)
            {
                const bool ok = player.depositAllCoinsInCityVault();
                MessageScreen::show(ok ? "MONNAIE DÉPOSÉE" : "DÉPÔT IMPOSSIBLE", ok ? "quest.city_vault.deposit.coins.success" : "quest.city_vault.deposit.coins.failed", ok ? std::vector<std::string>{"Les dénominations physiques ont été conservées dans le coffre.", "Occupation : " + std::to_string(player.getCityVaultUsedSlots()) + "/" + std::to_string(player.getCityVaultCapacity()) + "."} : std::vector<std::string>{"Pas de monnaie à déposer ou capacité insuffisante."}, false);
                continue;
            }
            if (choice == 18)
            {
                const bool ok = player.withdrawAllCoinsFromCityVault();
                MessageScreen::show(ok ? "MONNAIE RETIRÉE" : "RETRAIT IMPOSSIBLE", ok ? "quest.city_vault.withdraw.coins.success" : "quest.city_vault.withdraw.coins.failed", ok ? std::vector<std::string>{"Les piles physiques ont rejoint le sac.", "Vérifie l'encombrement avant de repartir."} : std::vector<std::string>{"Ce coffre ne contient aucune monnaie physique."}, false);
                continue;
            }
            if (choice == 19)
            {
                const int before = player.getCityVaultSecurityDaysRemainingForCity(player.getCurrentCityId());
                const int cost = player.getCityVaultSecuritySubscriptionCost();
                const bool ok = player.buyCityVaultSecuritySubscription();
                MessageScreen::show(ok ? "SÉCURITÉ PROLONGÉE" : "ABONNEMENT IMPOSSIBLE", ok ? "quest.city_vault.security.success" : "quest.city_vault.security.failed", ok ? std::vector<std::string>{"Couverture : " + std::to_string(before) + " → " + std::to_string(player.getCityVaultSecurityDaysRemainingForCity(player.getCurrentCityId())) + " jours réellement passés loin de la ville.", "Coût : " + Money::formatEconomyUnits(cost) + ".", "Le compteur reste en pause ici et dans les explorations rattachées à cette ville."} : std::vector<std::string>{"Fonds insuffisants ou plafond de couverture déjà atteint."}, false);
                continue;
            }
            if (choice == 20)
            {
                runCityVaultDefense(player, difficulty, deathRule);
                continue;
            }
            if (choice == 21)
            {
                const std::string result = player.investigateVaultTheftCase(player.getCurrentCityId());
                MessageScreen::show("ENQUÊTE DU COFFRE", "quest.city_vault.investigation", {result, "Une enquête consomme du temps local : témoins, garde, traces et éventuels receleurs sont recoupés. L'auteur n'est jamais révélé sans indices crédibles."}, false);
                continue;
            }

            if (choice == 22)
            {
                openVaultTheftContactMenu(player);
                continue;
            }

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
