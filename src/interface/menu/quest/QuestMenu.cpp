// EN: QuestMenu.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: QuestMenu.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu.
// Description: Implements quest hub and read-only quest journal for Dinotofu.

#include "interface/menu/quest/QuestMenu.hpp"

#include "core/Console.hpp"
#include "core/Random.hpp"
#include "quest/QuestCatalog.hpp"
#include "item/material/MaterialCatalog.hpp"
#include "entity/MonsterCatalog.hpp"
#include "combat/modes/pve/MonsterPveMode.hpp"
#include "combat/system/ElementalAffinitySystem.hpp"
#include "character/RaceCatalog.hpp"
#include "economy/EconomyBalance.hpp"
#include "economy/shop/ShopTransactionSystem.hpp"
#include "economy/Money.hpp"
#include "interface/menu/InventoryMenu.hpp"
#include "interface/menu/common/PagedMenu.hpp"
#include "interface/menu/common/MessageScreen.hpp"
#include "interface/TerminalInterface.hpp"
#include "interface/model/MenuScreen.hpp"
#include "progression/bestiary/BestiaryRuntimeProgress.hpp"
#include "story/StoryCampaign.hpp"
#include "world/City.hpp"
#include "world/CityTravelRules.hpp"

#include <iostream>
#include <vector>
#include <algorithm>
#include <cstddef>
#include <cctype>
#include <set>
#include <utility>
#include <sstream>

namespace
{
    std::string questPlayableLocationHint(const Quest& quest);
    std::string questProgressMethodText(const Quest& quest);
    void expireOverdueQuestDeadlines(Player& player, const std::string& screenId, bool notify);
    Quest createChapterOneReferentMainQuest(const std::string& clientName);
    bool addNonRefusableQuestIfMissing(Player& player, Quest quest);
    int prunigilTrustScore(const Player& player);
    std::string prunigilTrustRankLabel(int score);
    std::string prunigilNextMilestoneLine(int score);
    void openChallengeMarkCounter(Player& player);
    std::string questKindText(const Quest& quest);
    void openCityVault(Player& player);
    void openInnMenu(Player& player);
    void openDelegatedMissionBoard(Player& player);
    void openGuildTribunalMenu(Player& player);
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

    bool guildRequestRankEUnlocked(const Player& player)
    {
        return guildRankPowerForRequests(guildRankForRequestGate(player)) >= guildRankPowerForRequests("E");
    }

    std::vector<std::string> guildRequestRankGateLines(const Player& player)
    {
        int completedGuildContracts = 0;
        for (const Quest& quest : player.getQuestLog().getQuests())
        {
            if (quest.guildQuest && quest.turnedIn)
            {
                ++completedGuildContracts;
            }
        }

        return {
            "Accès officiel refusé : les demandes, mandats directs, contacts de groupes et quêtes publiées demandent au moins le rang D de guilde.",
            "Rang actuel : " + guildRankForRequestGate(player) + ".",
            "Progression actuelle : " + std::to_string(completedGuildContracts) + "/5 contrats officiels validés et niveau " + std::to_string(player.getLevel()) + "/5 requis pour le rang D.",
            "Raison d'équilibrage : avant le rang D, payer des PNJ pour travailler à ta place serait trop fort et casserait la progression.",
            "Exception risquée : certains contacts douteux peuvent accepter illégalement, avec prix gonflé, vol possible et suspension de guilde si ça se sait.",
            "Tu peux encore faire les quêtes de guilde normales, découvrir le monde, rendre des services et revenir quand ton dossier sera assez solide."
        };
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

    std::vector<std::string> splitMissionKey(const std::string& key)
    {
        std::vector<std::string> parts;
        std::stringstream ss(key);
        std::string part;
        while (std::getline(ss, part, '|'))
        {
            parts.push_back(part);
        }
        return parts;
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
        lines.push_back("Effet actuel : contact découvert, rumeur ajoutée, micro-quête active enregistrée. La résolution complète restera à brancher dans le futur panneau de routes.");
        return lines;
    }

    struct DelegatedMissionTemplate
    {
        std::string type;
        std::string label;
        std::string detail;
        int costCopper = 0;
        int durationDays = 1;
        int successPercent = 50;
        int guildAcceptancePercent = 50;
        bool dangerous = false;
    };

    struct ContractorProfile
    {
        std::string id;
        std::string name;
        std::string kind;
        std::string detail;
        std::vector<std::string> strengths;
        int reliability = 50;
        int acceptance = 60;
        int priceModifierPercent = 0;
        bool refusesDanger = false;
        bool rareGroup = false;
        bool available = true;
        std::string unavailableReason;
        int unavailableUntilDay = -1;
    };

    bool brasCassesInTownToday(const Player& player);
    bool guildProbationActive(const Player& player);
    int activeGuildProbationUntilDay(const Player& player);
    int missionIntFieldFromKey(const std::string& key, const std::string& field, int fallback);
    std::string missionTextFieldFromKey(const std::string& key, const std::string& field, const std::string& fallback);

    bool profileHasStrength(const ContractorProfile& profile, const std::string& strength)
    {
        return std::find(profile.strengths.begin(), profile.strengths.end(), strength) != profile.strengths.end();
    }

    bool hasCanonicalRecord(const Player& player, const std::string& category, const std::string& key)
    {
        return canonicalRecordCount(player, category, key) > 0;
    }

    bool contractorDiscovered(const Player& player, const std::string& profileId)
    {
        return hasCanonicalRecord(player, "groupes_pnj_decouverts", profileId);
    }

    int contractorRequiredLevel(const std::string& profileId)
    {
        if (profileId == "bras_casses") return 10;
        if (profileId == "ordo_pierre") return 8;
        if (profileId == "eclats_azur" || profileId == "loups_lanterne") return 5;
        if (profileId == "sables_gris" || profileId == "hirondelles_nuit") return 4;
        if (profileId == "bande_nero" || profileId == "voiles_de_sel" || profileId == "coureurs_virevent") return 3;
        if (profileId == "crocs_tordus" || profileId == "deux_lames_chariot" || profileId == "marmites_bossues" || profileId == "fer_doux" || profileId == "becs_cuivre") return 2;
        return 1;
    }

    std::string contractorNameFromId(const std::string& profileId)
    {
        if (profileId == "lanternes") return "Les Lanternes de Prunigil";
        if (profileId == "crocs_tordus") return "Chasseurs du Croc Tordu";
        if (profileId == "glaneurs_mousse") return "Glaneurs de mousse";
        if (profileId == "deux_lames_chariot") return "Deux Lames et un Chariot";
        if (profileId == "scribes_ecu") return "Scribes de l'Écu";
        if (profileId == "bande_nero") return "Bande de Néro";
        if (profileId == "sables_gris") return "Les Sables Gris";
        if (profileId == "marmites_bossues") return "Les Marmites Bossues";
        if (profileId == "eclats_azur") return "Les Éclats d'Azur";
        if (profileId == "marteaux_de_traverse") return "Les Marteaux de Traverse";
        if (profileId == "voiles_de_sel") return "Les Voiles de Sel";
        if (profileId == "ordo_pierre") return "L'Ordo de Pierre";
        if (profileId == "hirondelles_nuit") return "Les Hirondelles de Nuit";
        if (profileId == "fer_doux") return "La Compagnie du Fer Doux";
        if (profileId == "becs_cuivre") return "Les Becs de Cuivre";
        if (profileId == "coureurs_virevent") return "Les Coureurs de Virevent";
        if (profileId == "loups_lanterne") return "Les Loups de Lanterne";
        if (profileId == "atelier_ambulant") return "L'Atelier Ambulant";
        if (profileId == "bras_casses") return "Les Bras Cassés";
        return profileId;
    }

    std::string contractorInjuryRoleText(const std::string& profileId)
    {
        if (profileId == "crocs_tordus" || profileId == "loups_lanterne") return "Rôle touché : pisteur/chasseur, morsure ou fracture probable.";
        if (profileId == "scribes_ecu") return "Rôle touché : scribe/assistant, choc administratif et soins prolongés plus que blessure de guerre.";
        if (profileId == "glaneurs_mousse" || profileId == "marmites_bossues") return "Rôle touché : récolteur/porteur, fatigue, chute ou intoxication légère.";
        if (profileId == "lanternes" || profileId == "hirondelles_nuit" || profileId == "coureurs_virevent") return "Rôle touché : éclaireur/coursier, entorse ou épuisement de route.";
        if (profileId == "bras_casses") return "Rôle touché : ego et armure. Ils vont nier, évidemment.";
        return "Rôle touché : membre de groupe non précisé, soin prolongé requis.";
    }

    std::string contractorDiscoveryHint(const ContractorProfile& profile)
    {
        if (profile.id == "bras_casses") return "Rumeur héroïque presque absurde : il faut être assez reconnu et tomber le bon jour sur eux.";
        if (profile.id == "ordo_pierre") return "Vétérans exigeants : ils ne traitent pas avec un aventurier trop bas niveau.";
        if (profile.id == "bande_nero") return "Contact louche : la guilde donne l'adresse seulement après quelques preuves de survie.";
        return "Contact recommandé par la guilde : faire connaissance avant tout mandat direct.";
    }

    bool hasActiveMissingGroupRaw(const Player& player)
    {
        for (const PlayerJournalRecord& record : player.getCanonicalJournalRecords())
        {
            if (record.category != "groupes_pnj_disparus_actifs") continue;
            bool resolved = false;
            for (const PlayerJournalRecord& resolvedRecord : player.getCanonicalJournalRecords())
            {
                if (resolvedRecord.category == "groupes_pnj_sauvetages_resolus" && resolvedRecord.key == record.key)
                {
                    resolved = true;
                    break;
                }
            }
            if (!resolved) return true;
        }
        return false;
    }

    int contractorExperienceScore(const Player& player, const std::string& profileId)
    {
        const int mandates = canonicalRecordCount(player, "profils_pnj_mandates", profileId);
        const int successes = canonicalRecordCount(player, "missions_deleguees_reussies_par_profil", profileId);
        const int failures = canonicalRecordCount(player, "missions_deleguees_echouees_par_profil", profileId);
        return std::max(0, mandates + successes * 3 - failures);
    }

    int contractorRankLevel(const Player& player, const std::string& profileId)
    {
        const int xp = contractorExperienceScore(player, profileId);
        return 1 + std::min(4, xp / 4);
    }

    std::string contractorRankLabel(const Player& player, const ContractorProfile& profile)
    {
        const int rank = contractorRankLevel(player, profile.id);
        if (profile.id == "bras_casses")
        {
            return "Rang narratif : héros principaux — toujours un cran au-dessus du joueur, mais pas gratuits.";
        }
        switch (rank)
        {
            case 1: return "Rang relation : contact récent";
            case 2: return "Rang relation : habitués du comptoir";
            case 3: return "Rang relation : partenaires fiables";
            case 4: return "Rang relation : groupe entraîné par tes contrats";
            default: return "Rang relation : alliés reconnus";
        }
    }

    int contractorRankBonus(const Player& player, const std::string& profileId)
    {
        return (contractorRankLevel(player, profileId) - 1) * 4;
    }

    bool contractorDiscoveryConditionsMet(const Player& player, const ContractorProfile& profile)
    {
        if (player.getLevel() < contractorRequiredLevel(profile.id))
        {
            return false;
        }
        if (profile.id == "bras_casses" && !contractorDiscovered(player, profile.id) && !brasCassesInTownToday(player))
        {
            return false;
        }
        return true;
    }

    void discoverContractor(Player& player, const ContractorProfile& profile, const std::string& reason)
    {
        if (contractorDiscovered(player, profile.id))
        {
            return;
        }
        player.recordCanonicalEvent("groupes_pnj_decouverts", profile.id, profile.name + " — " + reason);
        player.recordCanonicalEvent("groupes_pnj_contacts", player.getCurrentCityId(), "Contact établi avec " + profile.name);
        recordRecentAction(player, "contractor_discovered", "Contact découvert : " + profile.name);
    }

    int clampPercent(int value)
    {
        return std::max(5, std::min(95, value));
    }

    std::vector<DelegatedMissionTemplate> buildDelegatedMissionTemplates(const Player& player)
    {
        const int level = std::max(1, player.getLevel());
        std::vector<DelegatedMissionTemplate> missions = {
            {"materials", "Récupération de matériaux", "Une petite équipe cherche des matériaux communs proches, sans te téléporter du butin rare.", 150 + level * 8, 2, 68, 70, false},
            {"route_scout", "Reconnaissance de route", "Des éclaireurs observent une liaison, rapportent une rumeur ou confirment un danger limité.", 125 + level * 6, 2, 72, 76, false},
            {"local_service", "Service local pour un PNJ", "Un employé règle une petite tâche de ville et peut améliorer légèrement ta réputation locale.", 110 + level * 5, 1, 80, 82, false},
            {"guard_job", "Escorte indirecte", "Des aventuriers protègent un trajet simple. Plus cher, moins sûr, mais utile quand tu as autre chose à faire.", 260 + level * 10, 3, 58, 62, true},
            {"monster_hunt", "Chasse de monstre ciblée", "Une équipe tente de tuer quelques monstres pour récupérer des matériaux de créature. Plus risqué, surtout avec le mauvais profil.", 340 + level * 16, 3, 46, 52, true},
            {"rare_search", "Recherche spéciale", "Mission chère et incertaine pour chercher une piste, un indice ou un matériau inhabituel.", 420 + level * 14, 4, 38, 44, true},
            {"boss_materials", "Extermination de boss pour matériaux", "Demande extrêmement dangereuse : tenter d'abattre un boss déjà connu pour ramener un fragment. Presque aucun groupe n'accepte, sauf héros rarissimes.", 1250 + level * 45, 5, 22, 18, true}
        };
        if (hasActiveMissingGroupRaw(player))
        {
            missions.push_back({"rescue_group", "Sauvetage d'un groupe disparu", "Un groupe mandaté n'est pas rentré. La guilde peut envoyer une équipe pour le retrouver : disparition rare, jamais mort définitive automatique.", 520 + level * 18, 3, 52, 58, true});
        }
        return missions;
    }

    DelegatedMissionTemplate getMissionTemplateByType(const Player& player, const std::string& type)
    {
        const std::vector<DelegatedMissionTemplate> templates = buildDelegatedMissionTemplates(player);
        for (const DelegatedMissionTemplate& mission : templates)
        {
            if (mission.type == type)
            {
                return mission;
            }
        }
        return {"unknown", "Mission inconnue", "Mission dont le type n'est plus reconnu par le bureau.", 100, 2, 35, 35, false};
    }

    std::string contractorFieldFromMissionKey(const std::string& key, const std::string& field)
    {
        const std::string token = field + ":";
        const std::size_t pos = key.find(token);
        if (pos == std::string::npos)
        {
            return "";
        }
        const std::size_t startValue = pos + token.size();
        const std::size_t endValue = key.find('|', startValue);
        return key.substr(startValue, endValue == std::string::npos ? std::string::npos : endValue - startValue);
    }

    int contractorIntFieldFromMissionKey(const std::string& key, const std::string& field, int fallback)
    {
        const std::string value = contractorFieldFromMissionKey(key, field);
        if (value.empty())
        {
            return fallback;
        }
        try
        {
            return std::stoi(value);
        }
        catch (...)
        {
            return fallback;
        }
    }

    bool contractorProfileBusyFromActiveMission(const Player& player, const std::string& profileId, int& dueDay)
    {
        dueDay = -1;
        for (const PlayerJournalRecord& record : player.getCanonicalJournalRecords())
        {
            if (record.category != "missions_deleguees_actives")
            {
                continue;
            }

            bool resolved = false;
            for (const PlayerJournalRecord& resolvedRecord : player.getCanonicalJournalRecords())
            {
                if (resolvedRecord.category == "missions_deleguees_resolues" && resolvedRecord.key == record.key)
                {
                    resolved = true;
                    break;
                }
            }
            if (resolved)
            {
                continue;
            }

            const std::string activeProfile = contractorFieldFromMissionKey(record.key, "profile");
            if (activeProfile == profileId)
            {
                dueDay = contractorIntFieldFromMissionKey(record.key, "due", player.getWorldDaysElapsed() + 1);
                return true;
            }
        }
        return false;
    }

    bool contractorProfileInForcedRest(const Player& player, const std::string& profileId, int& untilDay)
    {
        untilDay = -1;
        for (const PlayerJournalRecord& record : player.getCanonicalJournalRecords())
        {
            if (record.category != "groupes_pnj_repos_actifs") continue;
            if (contractorFieldFromMissionKey(record.key, "profile") != profileId) continue;
            const int until = contractorIntFieldFromMissionKey(record.key, "until", -1);
            if (until > player.getWorldDaysElapsed())
            {
                untilDay = std::max(untilDay, until);
            }
        }
        return untilDay > player.getWorldDaysElapsed();
    }

    bool contractorProfileMissing(const Player& player, const std::string& profileId)
    {
        for (const PlayerJournalRecord& record : player.getCanonicalJournalRecords())
        {
            if (record.category != "groupes_pnj_disparus_actifs") continue;
            if (contractorFieldFromMissionKey(record.key, "profile") != profileId) continue;
            bool resolved = false;
            for (const PlayerJournalRecord& resolvedRecord : player.getCanonicalJournalRecords())
            {
                if (resolvedRecord.category == "groupes_pnj_sauvetages_resolus" && resolvedRecord.key == record.key)
                {
                    resolved = true;
                    break;
                }
            }
            if (!resolved) return true;
        }
        return false;
    }


    void markProfileAvailability(const Player& player, ContractorProfile& profile)
    {
        int busyUntil = -1;
        if (contractorProfileBusyFromActiveMission(player, profile.id, busyUntil))
        {
            profile.available = false;
            profile.unavailableUntilDay = busyUntil;
            profile.unavailableReason = "Déjà en mission jusqu'au jour " + std::to_string(busyUntil + 1) + ".";
            return;
        }

        int forcedRestUntil = -1;
        if (contractorProfileInForcedRest(player, profile.id, forcedRestUntil))
        {
            profile.available = false;
            profile.unavailableUntilDay = forcedRestUntil;
            profile.unavailableReason = "Repos forcé / coma léger jusqu'au jour " + std::to_string(forcedRestUntil + 1) + ". Pas de mort définitive de PNJ mandaté.";
            return;
        }

        if (contractorProfileMissing(player, profile.id))
        {
            profile.available = false;
            profile.unavailableReason = "Disparu en mission. Une mission de sauvetage peut le ramener.";
            return;
        }

        const int roll = stableMissionRoll(profile.id + ":away:" + player.getCurrentCityId() + ":" + std::to_string(player.getWorldDaysElapsed() / 2));
        const int awayThreshold = profile.rareGroup ? 0 : (profile.reliability >= 70 ? 9 : 15);
        if (roll < awayThreshold)
        {
            profile.available = false;
            profile.unavailableUntilDay = player.getWorldDaysElapsed() + 1 + (roll % 2);
            profile.unavailableReason = "Indisponible : déjà parti sur une courte mission locale.";
        }
    }

    bool brasCassesInTownToday(const Player& player)
    {
        return stableMissionRoll("bras_casses:" + player.getCurrentCityId() + ":" + std::to_string(player.getWorldDaysElapsed())) < 4;
    }

    std::vector<ContractorProfile> buildAllContractorProfiles(const Player& player)
    {
        const std::string cityId = player.getCurrentCityId();
        std::vector<ContractorProfile> profiles = {
            {"lanternes", "Les Lanternes de Prunigil", "groupe d'éclaireurs", "Patrouilleurs prudents : excellents en route et observation, moins adaptés aux chasses brutales.", {"route_scout", "guard_job"}, 72, 80, 18, false, false, true, "", -1},
            {"crocs_tordus", "Chasseurs du Croc Tordu", "groupe de chasse", "Combattants spécialisés dans les monstres et les matériaux de créature. Ils refusent rarement le danger, mais demandent cher.", {"monster_hunt", "materials"}, 66, 72, 22, false, false, true, "", -1},
            {"glaneurs_mousse", "Glaneurs de mousse", "petite équipe de récolte", "Récolteurs discrets : bons sur les plantes et matériaux simples, très mauvais pour les monstres dangereux.", {"materials", "rare_search"}, 58, 74, -8, true, false, true, "", -1},
            {"deux_lames_chariot", "Deux Lames et un Chariot", "escorte marchande", "Groupe équilibré pour escorte et service local. Pas brillant, mais fiable quand la mission est claire.", {"guard_job", "local_service"}, 62, 78, 10, false, false, true, "", -1},
            {"scribes_ecu", "Scribes de l'Écu", "employés de guilde", "Profils administratifs : parfaits pour les services et les quêtes publiées, inutiles pour tuer un monstre.", {"local_service", "route_scout"}, 76, 86, 4, true, false, true, "", -1},
            {"bande_nero", "Bande de Néro", "aventuriers opportunistes", "Ils acceptent beaucoup de choses, mais leur méthode est instable. Moins cher, plus risqué.", {"monster_hunt", "rare_search", "guard_job"}, 44, 88, -18, false, false, true, "", -1},
            {"sables_gris", "Les Sables Gris", "traqueurs de ruines", "Ils lisent les traces, supportent les longues marches et reviennent souvent avec une rumeur exploitable.", {"route_scout", "rare_search"}, 64, 70, 12, false, false, true, "", -1},
            {"marmites_bossues", "Les Marmites Bossues", "cuisiniers-récolteurs", "Ils savent négocier, porter et récolter. En combat, ils préfèrent clairement courir dans l'autre sens.", {"materials", "local_service"}, 61, 81, -5, true, false, true, "", -1},
            {"eclats_azur", "Les Éclats d'Azur", "mages itinérants", "Bons pour analyser une piste étrange ou sécuriser une escorte magique. Plus chers et un peu hautains.", {"rare_search", "guard_job", "route_scout"}, 69, 66, 28, false, false, true, "", -1},
            {"marteaux_de_traverse", "Les Marteaux de Traverse", "ouvriers armés", "Robustes, efficaces pour escorte et service local. Ils avancent lentement, mais abandonnent rarement.", {"guard_job", "local_service", "materials"}, 70, 73, 14, false, false, true, "", -1},
            {"voiles_de_sel", "Les Voiles de Sel", "coursiers portuaires", "Rapides sur les routes commerciales et bons pour rapporter des contacts, moins bons hors des chemins connus.", {"route_scout", "local_service"}, 63, 79, 6, false, false, true, "", -1},
            {"ordo_pierre", "L'Ordo de Pierre", "vétérans disciplinés", "Très fiables pour escorte et chasse, mais ils exigent un paiement correct et refusent les plans absurdes.", {"guard_job", "monster_hunt"}, 78, 62, 34, false, false, true, "", -1},
            {"hirondelles_nuit", "Les Hirondelles de Nuit", "éclaireurs discrets", "Très bons pour observer sans se montrer, mais ils refusent les massacres et les contrats trop bruyants.", {"route_scout", "rare_search"}, 67, 68, 24, true, false, true, "", -1},
            {"fer_doux", "La Compagnie du Fer Doux", "gardes salariés", "Groupe sérieux pour escorte et protection. Peu spectaculaire, mais ils savent tenir une route.", {"guard_job", "local_service"}, 71, 74, 18, false, false, true, "", -1},
            {"becs_cuivre", "Les Becs de Cuivre", "négociants débrouillards", "Ils savent obtenir des matériaux ordinaires, porter des messages et trouver des petits arrangements locaux.", {"materials", "local_service"}, 57, 83, -2, true, false, true, "", -1},
            {"coureurs_virevent", "Les Coureurs de Virevent", "coursiers rapides", "Ils excellent dans les trajets et les rapports courts. En combat prolongé, ils évitent de jouer aux héros.", {"route_scout", "local_service", "guard_job"}, 60, 79, 8, false, false, true, "", -1},
            {"loups_lanterne", "Les Loups de Lanterne", "chasseurs nocturnes", "Chasseurs prudents de monstres, utiles quand la cible mord. Plus chers dès que le danger augmente.", {"monster_hunt", "guard_job"}, 68, 69, 30, false, false, true, "", -1},
            {"atelier_ambulant", "L'Atelier Ambulant", "artisans itinérants", "Ils ne gagnent pas les duels, mais savent réparer, transporter et reconnaître des matériaux utiles.", {"materials", "local_service", "rare_search"}, 65, 76, 16, true, false, true, "", -1}
        };

        const bool brasKnown = contractorDiscovered(player, "bras_casses");
        const bool brasPresentToday = brasCassesInTownToday(player);
        if (brasKnown || brasPresentToday)
        {
            ContractorProfile bras{
                "bras_casses",
                "Les Bras Cassés",
                "groupe héroïque principal",
                "Le fameux groupe. Rare de fou à croiser au bureau : presque tout est possible en combat, mais la récolte pure les ennuie très vite.",
                {"monster_hunt", "boss_materials", "guard_job", "rare_search"},
                88,
                54,
                85,
                false,
                true,
                true,
                "",
                -1
            };
            if (!brasPresentToday)
            {
                bras.available = false;
                bras.unavailableReason = "Contact connu, mais absents aujourd'hui. Les Bras Cassés ne restent jamais longtemps au même comptoir.";
            }
            profiles.push_back(bras);
        }

        for (ContractorProfile& profile : profiles)
        {
            markProfileAvailability(player, profile);
        }

        if (!profiles.empty())
        {
            const int rotation = stableMissionRoll(cityId + ":contractors:" + std::to_string(player.getWorldDaysElapsed())) % static_cast<int>(profiles.size());
            std::rotate(profiles.begin(), profiles.begin() + rotation, profiles.end());
        }
        return profiles;
    }

    std::vector<ContractorProfile> buildContractorProfiles(const Player& player)
    {
        std::vector<ContractorProfile> known;
        for (const ContractorProfile& profile : buildAllContractorProfiles(player))
        {
            if (contractorDiscovered(player, profile.id))
            {
                known.push_back(profile);
            }
        }
        return known;
    }

    std::vector<ContractorProfile> availableContractorProfilesForGuild(const Player& player)
    {
        std::vector<ContractorProfile> profiles;
        for (const ContractorProfile& profile : buildAllContractorProfiles(player))
        {
            if (profile.available && contractorDiscoveryConditionsMet(player, profile))
            {
                profiles.push_back(profile);
            }
        }
        return profiles;
    }

    std::string contractorDialogueLine(const ContractorProfile& profile, const DelegatedMissionTemplate& mission, bool accepted)
    {
        if (profile.id == "bras_casses")
        {
            if (mission.type == "boss_materials") return accepted ? "« Un boss ? Enfin un truc drôle. Par contre, tu paies d'avance. »" : "« Pas aujourd'hui. On a déjà cassé assez de trucs pour la semaine. »";
            return accepted ? "« On peut le faire. Probablement. Enfin, sûrement. »" : "« Franchement ? Là, même nous on passe notre tour. »";
        }
        if (profile.id == "scribes_ecu") return accepted ? "« Le formulaire est propre. Nous pouvons traiter cette demande. »" : "« Cette demande ne relève pas de notre service, navrés. »";
        if (profile.id == "crocs_tordus") return accepted ? "« Tant que ça saigne ou mord, on sait faire. »" : "« Pas pour ce prix-là. Un monstre, ça mange aussi les chasseurs. »";
        if (profile.id == "glaneurs_mousse") return accepted ? "« On part léger, on revient avec ce qu'on peut porter. »" : "« Tuer des trucs ? Non, non, nous on cueille. »";
        if (profile.id == "bande_nero") return accepted ? "« On prend. Si ça tourne mal, on dira que c'était ton idée. »" : "« Trop carré pour nous. Ou pas assez payé. Choisis. »";
        return accepted ? "« Marché conclu. On revient faire rapport. »" : "« On refuse. Mauvais moment, mauvais risque, mauvais contrat. »";
    }

    std::string contractorAvailabilityLine(const ContractorProfile& profile)
    {
        if (profile.available)
        {
            return profile.rareGroup ? "Disponible aujourd'hui — présence rarissime." : "Disponible.";
        }
        return profile.unavailableReason.empty() ? "Indisponible pour le moment." : profile.unavailableReason;
    }

    int profileSuitabilityBonus(const ContractorProfile& profile, const DelegatedMissionTemplate& mission)
    {
        if (profileHasStrength(profile, mission.type))
        {
            return mission.dangerous ? 18 : 12;
        }
        if (mission.type == "monster_hunt" && profile.refusesDanger)
        {
            return -30;
        }
        if (mission.dangerous && profile.refusesDanger)
        {
            return -22;
        }
        if (profile.kind.find("employés") != std::string::npos && mission.type != "local_service" && mission.type != "route_scout")
        {
            return -18;
        }
        return mission.dangerous ? -12 : -6;
    }

    int profileMissionCost(const DelegatedMissionTemplate& mission, const ContractorProfile& profile)
    {
        int cost = mission.costCopper + (mission.costCopper * profile.priceModifierPercent) / 100;
        if (mission.type == "monster_hunt" && !profileHasStrength(profile, "monster_hunt"))
        {
            cost += 45;
        }
        if (mission.type == "boss_materials")
        {
            cost += profile.id == "bras_casses" ? 220 : 600;
        }
        return std::max(25, cost);
    }

    int profileMissionAcceptance(const Player& player, const DelegatedMissionTemplate& mission, const ContractorProfile& profile);

    int officialMissionCost(const Player& player, const DelegatedMissionTemplate& mission, const ContractorProfile& profile)
    {
        int cost = profileMissionCost(mission, profile);
        if (guildProbationActive(player))
        {
            cost += std::max(20, cost / 4);
        }
        return cost;
    }

    int officialMissionAcceptance(const Player& player, const DelegatedMissionTemplate& mission, const ContractorProfile& profile)
    {
        int acceptance = profileMissionAcceptance(player, mission, profile);
        if (guildProbationActive(player))
        {
            const bool seriousGroup = profile.id == "lanternes" || profile.id == "scribes_ecu" || profile.id == "fer_doux" || profile.id == "ordo_pierre" || profile.id == "marteaux_de_traverse";
            acceptance -= seriousGroup ? 12 : 6;
        }
        return clampPercent(acceptance);
    }

    int contractorRelationshipAcceptanceModifier(const Player& player, const std::string& profileId);

    int profileMissionSuccess(const Player& player, const DelegatedMissionTemplate& mission, const ContractorProfile& profile)
    {
        int result = mission.successPercent + profileSuitabilityBonus(profile, mission) + (profile.reliability - 60) / 2 + contractorRankBonus(player, profile.id);
        if (mission.type == "monster_hunt" && !profileHasStrength(profile, "monster_hunt"))
        {
            result -= 10;
        }
        if (mission.type == "boss_materials")
        {
            result += profile.id == "bras_casses" ? 30 : -32;
        }
        if (profile.id == "bras_casses")
        {
            result += std::min(10, std::max(2, player.getLevel() / 3));
        }
        return clampPercent(result);
    }

    int profileMissionAcceptance(const Player& player, const DelegatedMissionTemplate& mission, const ContractorProfile& profile)
    {
        int result = profile.acceptance + contractorRankBonus(player, profile.id) / 2 + contractorRelationshipAcceptanceModifier(player, profile.id);
        if (profileHasStrength(profile, mission.type)) result += 8;
        if (mission.dangerous) result -= 8;
        if (mission.type == "monster_hunt" && profile.refusesDanger) result -= 35;
        if (mission.type == "boss_materials") result += profile.id == "bras_casses" ? 18 : -45;
        if (mission.dangerous && profile.refusesDanger) result -= 18;
        return clampPercent(result);
    }

    std::string profileFitLabel(const DelegatedMissionTemplate& mission, const ContractorProfile& profile)
    {
        if (mission.type == "boss_materials")
        {
            return profile.id == "bras_casses" ? "héros presque parfaits pour ce suicide organisé" : "profil presque suicidaire";
        }
        const int bonus = profileSuitabilityBonus(profile, mission);
        if (bonus >= 15) return "profil idéal";
        if (bonus >= 8) return "bon profil";
        if (bonus >= -5) return "profil acceptable";
        if (bonus <= -22) return "très mauvais profil";
        return "profil risqué";
    }

    std::string missionTypeDisplayName(const std::string& type)
    {
        if (type == "materials") return "récolte";
        if (type == "route_scout") return "éclaireur / route";
        if (type == "local_service") return "service local";
        if (type == "guard_job") return "escorte / protection";
        if (type == "monster_hunt") return "chasse de monstres";
        if (type == "rare_search") return "recherche rare";
        if (type == "boss_materials") return "boss / matériaux héroïques";
        if (type == "rescue_group") return "sauvetage";
        return type;
    }

    std::string contractorPreferenceLine(const ContractorProfile& profile)
    {
        std::string line = "Contrats préférés : ";
        if (profile.strengths.empty())
        {
            line += "aucun profil clair";
        }
        else
        {
            for (std::size_t i = 0; i < profile.strengths.size(); ++i)
            {
                if (i > 0) line += ", ";
                line += missionTypeDisplayName(profile.strengths[i]);
            }
        }
        line += ".";
        if (profile.refusesDanger)
        {
            line += " Déteste ou refuse souvent les missions trop violentes.";
        }
        if (profile.id == "bras_casses")
        {
            line += " La récolte pure les ennuie : ils restent faits pour l'héroïque, pas pour ramasser des champignons.";
        }
        return line;
    }

    int contractorRelationshipAcceptanceModifier(const Player& player, const std::string& profileId)
    {
        int modifier = 0;
        const int mandates = canonicalRecordCount(player, "profils_pnj_mandates", profileId);
        const int successes = canonicalRecordCount(player, "missions_deleguees_reussies_par_profil", profileId);
        const int failures = canonicalRecordCount(player, "missions_deleguees_echouees_par_profil", profileId);
        const int rescues = canonicalRecordCount(player, "sauvetages_groupes_reussis", profileId);
        modifier += std::min(10, mandates / 2 + successes * 2 + rescues * 3);
        modifier -= std::min(8, failures * 2);

        const int neroMandates = canonicalRecordCount(player, "profils_pnj_mandates", "bande_nero");
        const bool seriousGroup = profileId == "lanternes" || profileId == "scribes_ecu" || profileId == "fer_doux" || profileId == "ordo_pierre" || profileId == "marteaux_de_traverse";
        if (seriousGroup && neroMandates >= 3)
        {
            modifier -= std::min(10, (neroMandates - 2) * 2);
        }

        if (profileId == "bras_casses")
        {
            modifier -= std::min(16, canonicalRecordCount(player, "incidents_bras_casses", "rebellion_non_legale") * 8);
            modifier -= std::min(6, canonicalRecordCount(player, "incidents_bras_casses", "remarque_ratee") * 2);
        }
        return modifier;
    }

    std::string contractorRelationshipLine(const Player& player, const ContractorProfile& profile)
    {
        const int mandates = canonicalRecordCount(player, "profils_pnj_mandates", profile.id);
        const int successes = canonicalRecordCount(player, "missions_deleguees_reussies_par_profil", profile.id);
        const int failures = canonicalRecordCount(player, "missions_deleguees_echouees_par_profil", profile.id);
        const int rescues = canonicalRecordCount(player, "sauvetages_groupes_reussis", profile.id);
        const int incidents = canonicalRecordCount(player, "incidents_groupes_pnj", profile.id);
        const int modifier = contractorRelationshipAcceptanceModifier(player, profile.id);
        std::string mood = "neutre";
        if (modifier >= 8) mood = "confiance forte";
        else if (modifier >= 3) mood = "respect prudent";
        else if (modifier <= -8) mood = "rancune / méfiance";
        else if (modifier <= -3) mood = "méfiance légère";

        return "Relation : " + mood
            + " | mandats " + std::to_string(mandates)
            + ", réussites " + std::to_string(successes)
            + ", échecs " + std::to_string(failures)
            + ", sauvetages " + std::to_string(rescues)
            + ", incidents " + std::to_string(incidents)
            + ", mod. acceptation " + (modifier >= 0 ? "+" : "") + std::to_string(modifier) + ".";
    }


    int totalGuildBanReductionDays(const Player& player, const std::string& cityId)
    {
        int total = 0;
        for (const PlayerJournalRecord& record : player.getCanonicalJournalRecords())
        {
            if (record.category != "bannissements_guilde_reductions") continue;
            if (record.key == cityId && record.count > 0)
            {
                total += record.count;
            }
        }
        return std::min(6, total);
    }

    int activeGuildBanUntilDay(const Player& player)
    {
        int until = -1;
        for (const PlayerJournalRecord& record : player.getCanonicalJournalRecords())
        {
            if (record.category != "bannissements_guilde_actifs") continue;
            const std::string city = missionTextFieldFromKey(record.key, "city", "");
            if (!city.empty() && city != player.getCurrentCityId()) continue;
            const int candidate = missionIntFieldFromKey(record.key, "until", -1);
            if (candidate > player.getWorldDaysElapsed())
            {
                until = std::max(until, candidate);
            }
        }
        if (until > player.getWorldDaysElapsed())
        {
            until -= totalGuildBanReductionDays(player, player.getCurrentCityId());
        }
        return until;
    }

    bool guildBanActive(const Player& player)
    {
        return activeGuildBanUntilDay(player) > player.getWorldDaysElapsed();
    }

    int activeGuildProbationUntilDay(const Player& player)
    {
        int until = -1;
        for (const PlayerJournalRecord& record : player.getCanonicalJournalRecords())
        {
            if (record.category != "probations_guilde_actives") continue;
            const std::string city = missionTextFieldFromKey(record.key, "city", "");
            if (!city.empty() && city != player.getCurrentCityId()) continue;
            const int candidate = missionIntFieldFromKey(record.key, "until", -1);
            if (candidate > player.getWorldDaysElapsed())
            {
                until = std::max(until, candidate);
            }
        }
        return until;
    }

    bool guildProbationActive(const Player& player)
    {
        return !guildBanActive(player) && activeGuildProbationUntilDay(player) > player.getWorldDaysElapsed();
    }

    int undergroundReputationScore(const Player& player)
    {
        int score = 0;
        score += player.getCanonicalJournalCategoryTotal("demandes_illegales_lancees") * 2;
        score += player.getCanonicalJournalCategoryTotal("demandes_illegales_reussies") * 3;
        score += player.getCanonicalJournalCategoryTotal("demandes_illegales_vols");
        score += player.getCanonicalJournalCategoryTotal("bannissements_guilde_actifs") * 2;
        return std::min(40, score);
    }

    std::vector<std::string> guildBanLines(const Player& player)
    {
        const int until = activeGuildBanUntilDay(player);
        return {
            "Accès refusé : la guilde locale a suspendu ton dossier à cause d'une demande illégale ou d'un contact qui a parlé trop vite.",
            "Ville concernée : " + currentCityName(player) + ".",
            "Suspension jusqu'au jour " + std::to_string(until + 1) + ".",
            "Effet : mandats officiels, publication de quête, recherche de contacts et services semi-officiels bloqués ici.",
            "Les comptes rendus déjà dus peuvent encore être consultés pour ne pas casser une mission active.",
            "Médiation possible : payer une amende ou rendre un service propre peut réduire la suspension, sans l'effacer gratuitement."
        };
    }

    void applyGuildBan(Player& player, const std::string& reason, int durationDays, std::vector<std::string>& lines)
    {
        const int until = player.getWorldDaysElapsed() + std::max(1, durationDays);
        const std::string key = "city:" + player.getCurrentCityId() + "|until:" + std::to_string(until) + "|seq:" + std::to_string(player.getCanonicalJournalRecords().size());
        player.recordCanonicalEvent("bannissements_guilde_actifs", key, reason);
        player.recordCanonicalEvent("incidents_demandes_illegales", player.getCurrentCityId(), reason);
        player.recordCanonicalEvent("enquetes_guilde", player.getCurrentCityId(), "Enquête ouverte après dénonciation : " + reason);
        const int probationUntil = until + 3;
        const std::string probationKey = "city:" + player.getCurrentCityId() + "|until:" + std::to_string(probationUntil) + "|from:" + std::to_string(until) + "|seq:" + std::to_string(player.getCanonicalJournalRecords().size());
        player.recordCanonicalEvent("probations_guilde_actives", probationKey, "Probation après suspension : " + reason);
        recordRecentAction(player, "guild_ban", "Suspension de guilde : " + reason);
        lines.push_back("Sanction : la guilde locale suspend ton dossier jusqu'au jour " + std::to_string(until + 1) + ".");
        lines.push_back("Enquête : la guilde ouvre un dossier. Plus tard, ce panneau pourra proposer nier, avouer, payer, accuser le groupe ou apporter une preuve.");
        lines.push_back("Après la suspension : probation locale quelques jours, avec coûts plus élevés et moins d'acceptation chez les groupes sérieux.");
        lines.push_back("Raison : " + reason + ".");
    }

    void openGuildMediationMenu(Player& player)
    {
        if (!guildBanActive(player))
        {
            MessageScreen::show("MÉDIATION", "quest.guild_mediation.none", {"Aucune suspension active dans cette ville."}, false);
            return;
        }

        while (true)
        {
            const int fineCopper = 180 + player.getLevel() * 12 + player.getCanonicalJournalCategoryTotal("bannissements_guilde_actifs") * 25;
            MenuScreen screen("MÉDIATEUR DE GUILDE", "quest.guild_mediation");
            screen.addLine("Le médiateur n'efface pas la faute gratuitement : il propose réparation officielle.");
            screen.addLine("Ville : " + currentCityName(player) + ".");
            screen.addLine("Suspension actuelle jusqu'au jour " + std::to_string(activeGuildBanUntilDay(player) + 1) + ".");
            screen.addLine("Réputation souterraine : " + std::to_string(undergroundReputationScore(player)) + " — utile aux contacts louches, mauvaise pour les guildes sérieuses.");
            screen.addBackOption("Retour", "quest.guild_mediation.back");
            screen.addOption(1, "Payer une amende officielle", "Réduit la suspension locale d'un jour. Coût : " + Money::formatCopper(fineCopper) + ".", true, "quest.guild_mediation.fine");
            screen.addOption(2, "Présenter des excuses et aider le comptoir", "Réduit d'un jour, prend 1 segment, améliore légèrement le dossier officiel.", true, "quest.guild_mediation.service");

            const int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choix invalide.");
            Console::clear();
            if (choice == 0) return;
            if (choice == 1)
            {
                if (!player.getInventory().spendCopper(fineCopper))
                {
                    MessageScreen::show("AMENDE IMPOSSIBLE", "quest.guild_mediation.no_money", {"Coût : " + Money::formatCopper(fineCopper) + ".", "Argent actuel : " + player.getInventory().getWalletLine() + "."}, false);
                    continue;
                }
                player.recordCanonicalEvent("bannissements_guilde_reductions", player.getCurrentCityId(), "Amende officielle payée à " + currentCityName(player), 1);
                player.recordCanonicalEvent("amendes_guilde_payees", player.getCurrentCityId(), "Amende officielle à " + currentCityName(player), fineCopper);
                recordRecentAction(player, "guild_mediation_fine", "Amende officielle payée : suspension réduite");
                MessageScreen::show("AMENDE PAYÉE", "quest.guild_mediation.fine.done", {"La guilde réduit la suspension d'un jour.", "Ce n'est pas un pardon : la probation peut rester après."}, false);
                return;
            }
            if (choice == 2)
            {
                player.advanceWorldDayUnits(1);
                player.recordCanonicalEvent("bannissements_guilde_reductions", player.getCurrentCityId(), "Service propre rendu au comptoir de " + currentCityName(player), 1);
                player.recordCanonicalEvent("reparations_officielles_guilde", player.getCurrentCityId(), "Service propre / excuses à " + currentCityName(player));
                recordRecentAction(player, "guild_mediation_service", "Service de réparation officielle : suspension réduite");
                MessageScreen::show("SERVICE RENDU", "quest.guild_mediation.service.done", {"Tu aides le comptoir sans gagner de récompense.", "La guilde réduit la suspension d'un jour et note une réparation propre.", player.formatWorldDateTimeLine()}, false);
                return;
            }
        }
    }



    void openGuildTribunalMenu(Player& player)
    {
        while (true)
        {
            const bool banned = guildBanActive(player);
            const bool probation = guildProbationActive(player);
            const int underground = undergroundReputationScore(player);
            const int pardonCost = 95 + player.getLevel() * 6 + underground * 3;

            MenuScreen screen("CONSEIL DE GUILDE — " + currentCityName(player), "quest.guild_tribunal");
            screen.addLine("Panneau rare et propre : sanctions, pardon progressif, réputation noire et réparations officielles.");
            screen.addLine("État local : " + std::string(banned ? "suspendu" : (probation ? "en probation" : "dossier accessible")) + ".");
            screen.addLine("Réputation souterraine connue : " + std::to_string(underground) + ".");
            screen.addBackOption("Retour", "quest.guild_tribunal.back");
            screen.addOption(1, "Consulter le dossier", "Voir ce que la guilde retient sans ouvrir de nouveau choix risqué.", true, "quest.guild_tribunal.file");
            screen.addOption(2, "Demander un pardon progressif", "Coût : " + Money::formatCopper(pardonCost) + ". N'efface pas l'historique, mais améliore le dossier officiel.", !banned && (probation || underground > 0), "quest.guild_tribunal.pardon");
            screen.addOption(3, "Rendre un service de réparation", "Prend 1 segment, note une réparation propre et baisse la méfiance officielle future.", !banned, "quest.guild_tribunal.service");
            screen.addOption(4, "Parler au médiateur", "Disponible pendant une suspension active.", banned, "quest.guild_tribunal.mediation");

            const int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choix invalide.");
            Console::clear();
            if (choice == 0) return;
            if (choice == 1)
            {
                std::vector<std::string> lines;
                lines.push_back("Ville : " + currentCityName(player) + ".");
                lines.push_back("Suspension active : " + std::string(banned ? "oui" : "non") + ".");
                if (banned) lines.push_back("Fin estimée : jour " + std::to_string(activeGuildBanUntilDay(player) + 1) + ".");
                lines.push_back("Probation active : " + std::string(probation ? "oui" : "non") + ".");
                if (probation) lines.push_back("Fin estimée : jour " + std::to_string(activeGuildProbationUntilDay(player) + 1) + ".");
                lines.push_back("Réputation souterraine : " + std::to_string(underground) + ".");
                lines.push_back("Réparations officielles : " + std::to_string(player.getCanonicalJournalCategoryTotal("reparations_officielles_guilde")) + ".");
                lines.push_back("Pardons progressifs : " + std::to_string(player.getCanonicalJournalCategoryTotal("pardons_guilde_progressifs")) + ".");
                MessageScreen::show("DOSSIER DE GUILDE", "quest.guild_tribunal.file", lines, false);
                continue;
            }
            if (choice == 2)
            {
                if (!player.getInventory().spendCopper(pardonCost))
                {
                    MessageScreen::show("PARDON IMPOSSIBLE", "quest.guild_tribunal.pardon.no_money", {"Coût : " + Money::formatCopper(pardonCost) + ".", "Argent actuel : " + player.getInventory().getWalletLine() + "."}, false);
                    continue;
                }
                player.recordCanonicalEvent("pardons_guilde_progressifs", player.getCurrentCityId(), "Pardon progressif demandé à " + currentCityName(player));
                player.recordCanonicalEvent("amendes_guilde_payees", player.getCurrentCityId(), "Pardon progressif à " + currentCityName(player), pardonCost);
                recordRecentAction(player, "guild_pardon", "Pardon progressif demandé à la guilde");
                MessageScreen::show("PARDON PROGRESSIF", "quest.guild_tribunal.pardon.done", {"La guilde note l'effort, mais ne réécrit pas l'histoire.", "Les groupes sérieux verront surtout que tu as réparé au lieu de nier."}, false);
                continue;
            }
            if (choice == 3)
            {
                player.advanceWorldDayUnits(1);
                player.recordCanonicalEvent("reparations_officielles_guilde", player.getCurrentCityId(), "Service de réparation au conseil de " + currentCityName(player));
                recordRecentAction(player, "guild_repair_service", "Service officiel rendu au conseil de guilde");
                MessageScreen::show("SERVICE OFFICIEL", "quest.guild_tribunal.service.done", {"Tu aides la guilde sans recevoir de récompense.", "C'est une réparation officielle : utile pour le lore et les futures décisions de relation.", player.formatWorldDateTimeLine()}, false);
                continue;
            }
            if (choice == 4)
            {
                openGuildMediationMenu(player);
                continue;
            }
        }
    }


    bool missionStartedIllegally(const std::string& key)
    {
        return missionTextFieldFromKey(key, "illegal", "0") == "1";
    }

    std::vector<ContractorProfile> buildIllegalContractorProfiles(const Player& player)
    {
        std::vector<ContractorProfile> profiles;
        const std::set<std::string> allowed = {
            "bande_nero",
            "becs_cuivre",
            "coureurs_virevent",
            "hirondelles_nuit",
            "glaneurs_mousse",
            "voiles_de_sel"
        };
        for (ContractorProfile profile : buildAllContractorProfiles(player))
        {
            if (allowed.count(profile.id) == 0) continue;
            if (!profile.available) continue;
            if (profile.id != "bande_nero" && player.getLevel() < std::max(1, contractorRequiredLevel(profile.id) - 1)) continue;
            profiles.push_back(profile);
        }
        if (profiles.empty())
        {
            ContractorProfile fallback{
                "bande_nero",
                "Bande de Néro",
                "aventuriers opportunistes",
                "Contact louche : accepte parfois les bas rangs, mais peut voler l'argent ou dénoncer pour sauver sa peau.",
                {"monster_hunt", "rare_search", "guard_job", "materials"},
                39,
                78,
                32,
                false,
                false,
                true,
                "",
                -1
            };
            profiles.push_back(fallback);
        }
        return profiles;
    }

    int illegalMissionCost(const DelegatedMissionTemplate& mission, const ContractorProfile& profile)
    {
        int cost = profileMissionCost(mission, profile);
        cost += std::max(70, mission.costCopper / 2);
        if (mission.dangerous) cost += 80;
        if (profile.id == "bande_nero") cost = std::max(45, cost - mission.costCopper / 5);
        return std::max(60, cost);
    }

    int illegalTheftPercent(const ContractorProfile& profile, const DelegatedMissionTemplate& mission)
    {
        int risk = 16;
        if (profile.id == "bande_nero") risk += 18;
        if (profile.id == "becs_cuivre" || profile.id == "voiles_de_sel") risk += 7;
        if (mission.dangerous) risk += 8;
        if (profileHasStrength(profile, mission.type)) risk -= 5;
        return clampPercent(risk);
    }

    int illegalDenouncePercent(const ContractorProfile& profile, const DelegatedMissionTemplate& mission)
    {
        int risk = mission.dangerous ? 18 : 11;
        if (profile.id == "bande_nero") risk += 8;
        if (profile.id == "hirondelles_nuit") risk -= 4;
        if (profile.id == "glaneurs_mousse" && mission.dangerous) risk += 9;
        return clampPercent(risk);
    }

    ContractorProfile pickGuildAcceptedProfile(const Player& player, const DelegatedMissionTemplate& mission, const std::string& questKey)
    {
        std::vector<ContractorProfile> profiles = availableContractorProfilesForGuild(player);
        if (profiles.empty())
        {
            profiles = buildContractorProfiles(player);
        }
        std::sort(profiles.begin(), profiles.end(), [&](const ContractorProfile& a, const ContractorProfile& b) {
            const int scoreA = profileMissionSuccess(player, mission, a) + profileMissionAcceptance(player, mission, a) / 2;
            const int scoreB = profileMissionSuccess(player, mission, b) + profileMissionAcceptance(player, mission, b) / 2;
            if (scoreA != scoreB) return scoreA > scoreB;
            return a.name < b.name;
        });
        const int offset = stableMissionRoll(questKey + ":profile") % std::min<int>(3, profiles.size());
        return profiles[static_cast<std::size_t>(offset)];
    }

    bool delegatedMissionResolved(const Player& player, const std::string& missionKey)
    {
        for (const PlayerJournalRecord& record : player.getCanonicalJournalRecords())
        {
            if (record.category == "missions_deleguees_resolues" && record.key == missionKey)
            {
                return true;
            }
        }
        return false;
    }

    bool postedQuestResolved(const Player& player, const std::string& questKey)
    {
        for (const PlayerJournalRecord& record : player.getCanonicalJournalRecords())
        {
            if (record.category == "quetes_postees_resolues" && record.key == questKey)
            {
                return true;
            }
        }
        return false;
    }

    std::vector<PlayerJournalRecord> getActiveDelegatedMissions(const Player& player)
    {
        std::vector<PlayerJournalRecord> missions;
        for (const PlayerJournalRecord& record : player.getCanonicalJournalRecords())
        {
            if (record.category == "missions_deleguees_actives" && !delegatedMissionResolved(player, record.key))
            {
                missions.push_back(record);
            }
        }
        std::sort(missions.begin(), missions.end(), [](const PlayerJournalRecord& a, const PlayerJournalRecord& b) {
            if (a.lastDay != b.lastDay) return a.lastDay < b.lastDay;
            return a.label < b.label;
        });
        return missions;
    }

    std::vector<PlayerJournalRecord> getActivePostedQuests(const Player& player)
    {
        std::vector<PlayerJournalRecord> quests;
        for (const PlayerJournalRecord& record : player.getCanonicalJournalRecords())
        {
            if (record.category == "quetes_postees_actives" && !postedQuestResolved(player, record.key))
            {
                quests.push_back(record);
            }
        }
        std::sort(quests.begin(), quests.end(), [](const PlayerJournalRecord& a, const PlayerJournalRecord& b) {
            if (a.lastDay != b.lastDay) return a.lastDay < b.lastDay;
            return a.label < b.label;
        });
        return quests;
    }

    int missionIntFieldFromKey(const std::string& key, const std::string& field, int fallback)
    {
        const std::vector<std::string> parts = splitMissionKey(key);
        const std::string prefix = field + ":";
        for (const std::string& part : parts)
        {
            if (part.rfind(prefix, 0) == 0)
            {
                try { return std::stoi(part.substr(prefix.size())); } catch (...) { return fallback; }
            }
        }
        return fallback;
    }

    std::string missionTextFieldFromKey(const std::string& key, const std::string& field, const std::string& fallback)
    {
        const std::vector<std::string> parts = splitMissionKey(key);
        const std::string prefix = field + ":";
        for (const std::string& part : parts)
        {
            if (part.rfind(prefix, 0) == 0)
            {
                return part.substr(prefix.size());
            }
        }
        return fallback;
    }

    int missionDueDayFromKey(const std::string& key)
    {
        return missionIntFieldFromKey(key, "due", 999999);
    }

    int postedQuestDueDayFromKey(const std::string& key)
    {
        return missionIntFieldFromKey(key, "posted_due", 999999);
    }

    int missionRateFromKey(const std::string& key)
    {
        return clampPercent(missionIntFieldFromKey(key, "rate", 50));
    }

    int missionCostFromKey(const std::string& key)
    {
        return std::max(0, missionIntFieldFromKey(key, "cost", 0));
    }

    int missionDurationFromKey(const std::string& key)
    {
        return std::max(1, missionIntFieldFromKey(key, "duration", 2));
    }

    int postedQuestAcceptanceFromKey(const std::string& key)
    {
        return clampPercent(missionIntFieldFromKey(key, "accept", 50));
    }

    std::string missionTypeFromKey(const std::string& key)
    {
        return missionTextFieldFromKey(key, "type", "unknown");
    }

    std::string firstActiveMissingGroupKey(const Player& player)
    {
        for (const PlayerJournalRecord& record : player.getCanonicalJournalRecords())
        {
            if (record.category != "groupes_pnj_disparus_actifs") continue;
            bool resolved = false;
            for (const PlayerJournalRecord& resolvedRecord : player.getCanonicalJournalRecords())
            {
                if (resolvedRecord.category == "groupes_pnj_sauvetages_resolus" && resolvedRecord.key == record.key)
                {
                    resolved = true;
                    break;
                }
            }
            if (!resolved) return record.key;
        }
        return "";
    }

    void maybeRecordGroupTroubleAfterFailure(Player& player, const PlayerJournalRecord& mission, const std::string& type, std::vector<std::string>& lines)
    {
        const std::string profileId = missionTextFieldFromKey(mission.key, "profile", "unknown");
        if (profileId.empty() || profileId == "unknown")
        {
            return;
        }

        const DelegatedMissionTemplate templateInfo = getMissionTemplateByType(player, type);
        const int roll = stableMissionRoll(mission.key + ":trouble:" + std::to_string(player.getWorldDaysElapsed()));
        if (templateInfo.dangerous && roll < 6)
        {
            const std::string missingKey = "profile:" + profileId + "|from:" + mission.key + "|day:" + std::to_string(player.getWorldDaysElapsed());
            player.recordCanonicalEvent("groupes_pnj_disparus_actifs", missingKey, contractorNameFromId(profileId) + " ne rentre pas de mission");
            player.recordCanonicalEvent("incidents_groupes_pnj", profileId, contractorNameFromId(profileId) + " disparu — sauvetage requis");
            lines.push_back("Incident rare : " + contractorNameFromId(profileId) + " ne rentre pas tout de suite. La guilde parle de disparition, pas de mort.");
            lines.push_back(contractorInjuryRoleText(profileId));
            lines.push_back("Nouvelle possibilité : une mission de sauvetage peut être publiée ou mandatée pour les retrouver.");
            return;
        }

        const int comaThreshold = templateInfo.dangerous ? 18 : 7;
        if (roll < comaThreshold)
        {
            const int untilDay = player.getWorldDaysElapsed() + (templateInfo.dangerous ? 6 : 3);
            const std::string restKey = "profile:" + profileId + "|until:" + std::to_string(untilDay) + "|day:" + std::to_string(player.getWorldDaysElapsed());
            player.recordCanonicalEvent("groupes_pnj_repos_actifs", restKey, contractorNameFromId(profileId) + " en soin prolongé");
            player.recordCanonicalEvent("incidents_groupes_pnj", profileId, contractorNameFromId(profileId) + " en coma léger / soin prolongé");
            lines.push_back("Incident rare : " + contractorNameFromId(profileId) + " revient très mal en point.");
            lines.push_back(contractorInjuryRoleText(profileId));
            lines.push_back("Conséquence : repos forcé / coma léger jusqu'au jour " + std::to_string(untilDay + 1) + ". Pas de mort définitive de PNJ mandaté.");
        }
    }

    bool grantPartialDelegatedMissionReward(Player& player, const std::string& type, std::vector<std::string>& lines)
    {
        if (type == "materials")
        {
            player.getInventory().addMaterial(MaterialCatalog::createRustedMetalFragment(1));
            player.recordMaterialCollected("rusted_metal_fragment", "Fragment de métal rouillé", 1);
            lines.push_back("Réussite partielle : ils ne trouvent presque rien, mais ramènent 1 fragment de métal rouillé.");
            return true;
        }
        if (type == "route_scout")
        {
            player.recordCanonicalEvent("rumeurs_route", player.getCurrentCityId(), "Rumeur de route incomplète");
            lines.push_back("Réussite partielle : aucun vrai loot, mais une rumeur de route est notée.");
            return true;
        }
        if (type == "local_service")
        {
            player.recordPnjServed("Service local partiellement aidé");
            lines.push_back("Réussite partielle : le service n'est pas parfaitement rendu, mais le PNJ concerné retient l'effort.");
            return true;
        }
        if (type == "guard_job")
        {
            player.getInventory().earnCopper(30);
            lines.push_back("Réussite partielle : l'escorte tourne court, mais le client verse 30 cuivre pour le trajet protégé.");
            return true;
        }
        if (type == "monster_hunt")
        {
            player.getInventory().addMaterial(MaterialCatalog::createSlimeResidue(1));
            player.recordMaterialCollected("slime_residue", "Résidu de slime", 1);
            lines.push_back("Réussite partielle : pas de vraie chasse propre, mais 1 résidu de slime est récupéré.");
            return true;
        }
        if (type == "rare_search")
        {
            player.recordCanonicalEvent("pistes_rares_incompletes", player.getCurrentCityId(), "Piste rare confirmée sans objet ramené");
            lines.push_back("Réussite partielle : rien de ramené, mais une piste rare est confirmée pour plus tard.");
            return true;
        }
        return false;
    }

    void grantDelegatedMissionReward(Player& player, const std::string& type, std::vector<std::string>& lines)
    {
        if (type == "rescue_group")
        {
            const std::string missingKey = firstActiveMissingGroupKey(player);
            if (!missingKey.empty())
            {
                const std::string profileId = missionTextFieldFromKey(missingKey, "profile", "unknown");
                player.recordCanonicalEvent("groupes_pnj_sauvetages_resolus", missingKey, contractorNameFromId(profileId) + " retrouvé");
                player.recordCanonicalEvent("sauvetages_groupes_reussis", profileId, contractorNameFromId(profileId));
                lines.push_back("Sauvetage réussi : " + contractorNameFromId(profileId) + " est retrouvé vivant.");
                lines.push_back("Ils ne reviennent pas instantanément au top : le groupe reste marqué, mais il n'est pas supprimé définitivement.");
            }
            else
            {
                lines.push_back("Sauvetage réussi, mais aucun groupe disparu actif n'était encore enregistré. La guilde archive la recherche.");
            }
        }
        else if (type == "materials")
        {
            player.getInventory().addMaterial(MaterialCatalog::createRustedMetalFragment(2));
            player.getInventory().addMaterial(MaterialCatalog::createBitterHealingLeaf(1));
            player.recordMaterialCollected("rusted_metal_fragment", "Fragment de métal rouillé", 2);
            player.recordMaterialCollected("bitter_healing_leaf", "Feuille médicinale amère", 1);
            lines.push_back("Récompense : 2 fragments de métal rouillé et 1 feuille médicinale amère.");
        }
        else if (type == "route_scout")
        {
            player.getInventory().addMaterial(MaterialCatalog::createRouteScoutNote(1));
            player.recordCanonicalEvent("rumeurs_route", player.getCurrentCityId(), "Rapport d'éclaireurs de route");
            lines.push_back("Récompense : note d'éclaireur de route ajoutée à l'inventaire.");
        }
        else if (type == "local_service")
        {
            player.recordPnjServed("Service local délégué");
            player.getInventory().earnCopper(35);
            lines.push_back("Récompense : réputation locale notée et 35 cuivre de dédommagement.");
        }
        else if (type == "guard_job")
        {
            player.getInventory().earnCopper(90);
            player.recordCanonicalEvent("escortes_reussies", player.getCurrentCityId(), "Escorte déléguée réussie");
            lines.push_back("Récompense : 90 cuivre reversés par le client protégé.");
        }
        else if (type == "monster_hunt")
        {
            const int roll = stableMissionRoll("monster_reward:" + player.getCurrentCityId() + ":" + std::to_string(player.getCanonicalJournalRecords().size()));
            if (roll < 34)
            {
                player.getInventory().addMaterial(MaterialCatalog::createGoblinEar(2));
                player.recordMaterialCollected("goblin_ear", "Oreille de gobelin", 2);
                lines.push_back("Récompense : 2 oreilles de gobelin récupérées proprement.");
            }
            else if (roll < 67)
            {
                player.getInventory().addMaterial(MaterialCatalog::createWolfFang(2));
                player.recordMaterialCollected("wolf_fang", "Croc de loup", 2);
                lines.push_back("Récompense : 2 crocs de loup utilisables.");
            }
            else
            {
                player.getInventory().addMaterial(MaterialCatalog::createSlimeResidue(3));
                player.recordMaterialCollected("slime_residue", "Résidu de slime", 3);
                lines.push_back("Récompense : 3 résidus de slime. La chasse n'a pas donné un trophée rare, mais rien n'est perdu.");
            }
            player.recordCanonicalEvent("monstres_chasses_par_pnj", player.getCurrentCityId(), "Chasse de monstre déléguée");
        }
        else if (type == "rare_search")
        {
            player.getInventory().addMaterial(MaterialCatalog::createArcaneDust(1));
            player.recordMaterialCollected("arcane_dust", "Poussière arcanique", 1);
            lines.push_back("Récompense : 1 poussière arcanique. Rien de légendaire gratuit, mais une vraie piste utile.");
        }
        else if (type == "boss_materials")
        {
            static const std::vector<std::pair<std::string, std::string>> bossMaterials = {
                {"fitoria_feather", "Plume lumineuse de Fitoria"},
                {"zelef_demon_blood", "Sang démoniaque de Zelef"},
                {"atlas_broken_plate", "Plaque brisée d'Atlas"},
                {"lyknir_hunt_shard", "Fragment de chasse silencieuse"},
                {"grinka_avarice_coin", "Pièce d'avarice tordue"}
            };
            const int roll = stableMissionRoll("boss_reward:" + player.getCurrentCityId() + ":" + std::to_string(player.getCanonicalJournalRecords().size())) % static_cast<int>(bossMaterials.size());
            const auto& chosen = bossMaterials[static_cast<std::size_t>(roll)];
            player.getInventory().addMaterial(MaterialCatalog::createById(chosen.first, 1));
            player.recordMaterialCollected(chosen.first, chosen.second, 1);
            player.recordCanonicalEvent("boss_extermines_par_pnj", chosen.first, "Boss abattu par un groupe mandaté : " + chosen.second);
            lines.push_back("Récompense : 1 fragment de boss ramené — " + chosen.second + ".");
            lines.push_back("Note : réussite rare et chère. Le jeu ne considère pas que le joueur a vaincu ce boss personnellement.");
        }
    }

    std::vector<std::string> resolveDueDelegatedMissions(Player& player)
    {
        std::vector<std::string> lines;
        const std::vector<PlayerJournalRecord> active = getActiveDelegatedMissions(player);
        for (const PlayerJournalRecord& mission : active)
        {
            const int dueDay = missionDueDayFromKey(mission.key);
            if (player.getWorldDaysElapsed() < dueDay)
            {
                continue;
            }

            const int rate = missionRateFromKey(mission.key);
            const bool illegalMission = missionStartedIllegally(mission.key);
            const bool success = stableMissionRoll(mission.key + ":" + std::to_string(dueDay)) < rate;
            const std::string type = missionTypeFromKey(mission.key);
            const std::string profileId = missionTextFieldFromKey(mission.key, "profile", "unknown");
            player.recordCanonicalEvent("missions_deleguees_resolues", mission.key, mission.label + (success ? " — réussite" : " — échec"));
            player.recordCanonicalEvent(success ? "missions_deleguees_reussies" : "missions_deleguees_echouees", type, mission.label);
            if (illegalMission)
            {
                player.recordCanonicalEvent(success ? "demandes_illegales_reussies" : "demandes_illegales_echouees", type, mission.label);
            }
            if (profileId != "unknown")
            {
                player.recordCanonicalEvent(success ? "missions_deleguees_reussies_par_profil" : "missions_deleguees_echouees_par_profil", profileId, contractorNameFromId(profileId));
            }
            recordRecentAction(player, success ? "delegated_success" : "delegated_fail", mission.label);

            lines.push_back("Compte rendu : " + mission.label + ".");
            if (success)
            {
                lines.push_back("Résultat : réussite. Les aventuriers reviennent te parler d'eux-mêmes après plusieurs jours.");
                if (profileId != "unknown")
                {
                    lines.push_back("Progression de groupe : " + contractorNameFromId(profileId) + " gagne de l'expérience relationnelle grâce à ce mandat.");
                }
                grantDelegatedMissionReward(player, type, lines);
            }
            else
            {
                const int partialRoll = stableMissionRoll(mission.key + ":partial:" + std::to_string(player.getWorldDaysElapsed()));
                const bool partial = partialRoll < 24 && type != "boss_materials" && type != "rescue_group";
                if (partial && grantPartialDelegatedMissionReward(player, type, lines))
                {
                    player.recordCanonicalEvent("missions_deleguees_partielles", type, mission.label);
                    if (profileId != "unknown")
                    {
                        player.recordCanonicalEvent("missions_deleguees_partielles_par_profil", profileId, contractorNameFromId(profileId));
                    }
                    lines.push_back("Résultat : mission officiellement ratée, mais pas inutile. Le groupe revient avec un résultat partiel.");
                }
                else
                {
                    lines.push_back("Résultat : échec. Ils reviennent quand même faire un rapport, mais sans miracle ni remboursement complet.");
                    if (illegalMission)
                    {
                        const int snitchRoll = stableMissionRoll(mission.key + ":illegal_snitch:" + std::to_string(player.getWorldDaysElapsed()));
                        if (snitchRoll < 24)
                        {
                            applyGuildBan(player, "Un groupe illégal a cafté après l'échec du mandat", 3 + (snitchRoll % 3), lines);
                        }
                        else
                        {
                            lines.push_back("Illégal : personne ne parle à la guilde cette fois, mais le dossier reste risqué.");
                        }
                    }
                    maybeRecordGroupTroubleAfterFailure(player, mission, type, lines);
                }
            }
            lines.push_back("");
        }
        return lines;
    }

    std::vector<std::string> resolveDuePostedQuests(Player& player)
    {
        std::vector<std::string> lines;
        const std::vector<PlayerJournalRecord> active = getActivePostedQuests(player);
        for (const PlayerJournalRecord& quest : active)
        {
            const int dueDay = postedQuestDueDayFromKey(quest.key);
            if (player.getWorldDaysElapsed() < dueDay)
            {
                continue;
            }

            const std::string type = missionTypeFromKey(quest.key);
            const DelegatedMissionTemplate mission = getMissionTemplateByType(player, type);
            const ContractorProfile profile = pickGuildAcceptedProfile(player, mission, quest.key);
            int acceptRate = postedQuestAcceptanceFromKey(quest.key);
            if (profileHasStrength(profile, type)) acceptRate += 8;
            if (mission.dangerous && profile.refusesDanger) acceptRate -= 18;
            acceptRate = clampPercent(acceptRate);
            const bool accepted = stableMissionRoll(quest.key + ":accept:" + profile.id) < acceptRate;
            player.recordCanonicalEvent("quetes_postees_resolues", quest.key, quest.label + (accepted ? " — acceptée" : " — non acceptée"));

            lines.push_back("Retour de la guilde : " + quest.label + ".");
            if (accepted)
            {
                const int duration = missionDurationFromKey(quest.key);
                const int rate = profileMissionSuccess(player, mission, profile);
                const int missionDue = player.getWorldDaysElapsed() + duration;
                const std::string delegatedKey = "type:" + type + "|city:" + player.getCurrentCityId() + "|due:" + std::to_string(missionDue) + "|duration:" + std::to_string(duration) + "|cost:0|rate:" + std::to_string(rate) + "|profile:" + profile.id + "|posted:1|seq:" + std::to_string(player.getCanonicalJournalRecords().size());
                const std::string delegatedLabel = "Quête publiée acceptée : " + mission.label + " par " + profile.name;
                player.recordCanonicalEvent("missions_deleguees_actives", delegatedKey, delegatedLabel);
                player.recordCanonicalEvent("quetes_postees_acceptees", type, mission.label);
                discoverContractor(player, profile, "groupe rencontré après avoir accepté une quête publiée");
                recordRecentAction(player, "posted_quest_accepted", delegatedLabel);
                lines.push_back("Résultat : un groupe accepte la quête après deux jours d'affichage.");
                lines.push_back("Groupe : " + profile.name + " — " + profileFitLabel(mission, profile) + ".");
                lines.push_back("Retour de mission prévu : jour " + std::to_string(missionDue + 1) + ".");
            }
            else
            {
                const int cost = missionCostFromKey(quest.key);
                const int refund = std::max(0, cost / 2);
                if (refund > 0)
                {
                    player.getInventory().earnCopper(refund);
                }
                player.recordCanonicalEvent("quetes_postees_non_acceptees", type, mission.label);
                player.recordCanonicalEvent("remboursements_guilde", type, "Remboursement partiel : " + mission.label, refund);
                recordRecentAction(player, "posted_quest_refund", "Quête non acceptée, remboursement partiel : " + mission.label);
                lines.push_back("Résultat : personne n'a accepté la quête dans les deux jours.");
                lines.push_back("La guilde te rembourse la moitié de la mise : " + Money::formatCopper(refund) + ".");
            }
            lines.push_back("");
        }
        return lines;
    }

    void openPostedQuestBoard(Player& player)
    {
        while (true)
        {
            std::vector<std::string> reports = resolveDuePostedQuests(player);
            if (!reports.empty())
            {
                MessageScreen::show("RETOUR DE LA GUILDE", "quest.posted_quests.reports", reports, false);
            }

            if (guildBanActive(player))
            {
                MessageScreen::show("GUILDE EN SUSPENSION", "quest.posted_quests.guild_ban", guildBanLines(player), false);
                return;
            }

            if (!guildRequestRankDUnlocked(player))
            {
                MessageScreen::show("RANG DE GUILDE INSUFFISANT", "quest.posted_quests.rank_gate", guildRequestRankGateLines(player), false);
                return;
            }

            MenuScreen screen("PUBLIER UNE QUÊTE", "quest.posted_quests");
            screen.addLine("Tu peux devenir le client : tu paies la guilde pour afficher une demande aux PNJ aventuriers.");
            screen.addLine("Si personne n'accepte après 2 jours, la guilde te rembourse seulement la moitié de la mise.");
            screen.addLine("Les groupes choisis dépendent du type de mission : envoyer des scribes tuer des monstres est une mauvaise idée.");
            const std::vector<PlayerJournalRecord> pending = getActivePostedQuests(player);
            screen.addLine("Quêtes affichées en attente : " + std::to_string(pending.size()) + "/3.");
            if (!pending.empty())
            {
                for (const PlayerJournalRecord& quest : pending)
                {
                    screen.addLine("- " + quest.label + " | décision guilde jour " + std::to_string(postedQuestDueDayFromKey(quest.key) + 1) + ".");
                }
            }
            screen.addBackOption("Retour", "quest.posted_quests.back");

            const std::vector<DelegatedMissionTemplate> templates = buildDelegatedMissionTemplates(player);
            for (std::size_t i = 0; i < templates.size(); ++i)
            {
                const DelegatedMissionTemplate& t = templates[i];
                int postedCost = std::max(80, t.costCopper + t.costCopper / 3);
                if (guildProbationActive(player)) postedCost += std::max(25, postedCost / 4);
                std::string detail = t.detail + " | mise " + Money::formatCopper(postedCost) + " | décision sous 2 jours | acceptation estimée " + std::to_string(t.guildAcceptancePercent) + "%.";
                if (t.type == "monster_hunt")
                {
                    detail += " Matériaux de monstre possibles, mais échec plus probable.";
                }
                if (t.type == "boss_materials")
                {
                    detail += " Mission héroïque : presque personne ne l'accepte, sauf groupe exceptionnel.";
                }
                screen.addOption(static_cast<int>(i + 1), "Publier : " + t.label, detail, pending.size() < 3, "quest.posted_quests." + t.type);
            }

            const int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choix invalide.");
            Console::clear();
            if (choice == 0)
            {
                return;
            }
            if (choice < 1 || choice > static_cast<int>(templates.size()) || pending.size() >= 3)
            {
                continue;
            }

            const DelegatedMissionTemplate selected = templates[static_cast<std::size_t>(choice - 1)];
            int postedCost = std::max(80, selected.costCopper + selected.costCopper / 3);
            if (guildProbationActive(player)) postedCost += std::max(25, postedCost / 4);
            MenuScreen confirm("CONFIRMER LA PUBLICATION", "quest.posted_quests.confirm");
            confirm.addLine("Quête : " + selected.label + ".");
            confirm.addLine(selected.detail);
            confirm.addLine("Mise à payer : " + Money::formatCopper(postedCost) + ".");
            confirm.addLine("Si non acceptée après 2 jours : remboursement de moitié seulement.");
            confirm.addLine("Si acceptée : une mission PNJ partira ensuite plusieurs jours avec son propre taux de réussite.");
            confirm.addBackOption("Annuler", "quest.posted_quests.confirm.back");
            confirm.addOption(1, "Payer et afficher", "La guilde affiche la quête sur son panneau pendant 2 jours.", true, "quest.posted_quests.confirm.pay");
            const int confirmChoice = TerminalInterface::askMenuChoiceFromOptions(confirm, "Choix invalide.");
            Console::clear();
            if (confirmChoice != 1)
            {
                continue;
            }
            if (!player.getInventory().spendCopper(postedCost))
            {
                MessageScreen::show("ARGENT INSUFFISANT", "quest.posted_quests.no_money", {"Mise demandée : " + Money::formatCopper(postedCost) + ".", "Argent actuel : " + player.getInventory().getWalletLine() + "."}, false);
                continue;
            }

            const int decisionDay = player.getWorldDaysElapsed() + 2;
            const std::string key = "type:" + selected.type + "|city:" + player.getCurrentCityId() + "|posted_due:" + std::to_string(decisionDay) + "|duration:" + std::to_string(selected.durationDays) + "|cost:" + std::to_string(postedCost) + "|rate:" + std::to_string(selected.successPercent) + "|accept:" + std::to_string(selected.guildAcceptancePercent) + "|seq:" + std::to_string(player.getCanonicalJournalRecords().size());
            const std::string label = selected.label + " affichée à " + currentCityName(player);
            player.recordCanonicalEvent("quetes_postees_actives", key, label);
            player.recordCanonicalEvent("quetes_postees_couts", selected.type, selected.label, postedCost);
            recordRecentAction(player, "posted_quest_start", "Quête publiée : " + selected.label);
            MessageScreen::show("QUÊTE AFFICHÉE", "quest.posted_quests.started", {
                "Quête : " + selected.label + ".",
                "Décision de la guilde : jour " + std::to_string(decisionDay + 1) + ".",
                "Si aucun groupe ne l'accepte, la moitié de la mise sera remboursée.",
                "La mission reste volontairement incertaine : les PNJ ne sont pas des machines à loot."
            }, false);
        }
    }

    void showContractorRumors(Player& player)
    {
        std::vector<std::string> lines;
        lines.push_back("Rumeurs de groupes : elles ne débloquent pas l'embauche. Elles servent seulement à savoir qu'un groupe existe peut-être.");
        lines.push_back("Pour mandater un groupe, il faut ensuite une vraie découverte, une présentation ou une rencontre.");
        int shown = 0;
        for (const ContractorProfile& profile : buildAllContractorProfiles(player))
        {
            if (contractorDiscovered(player, profile.id)) continue;
            const int rumorRoll = stableMissionRoll("rumor:" + profile.id + ":" + player.getCurrentCityId() + ":" + std::to_string(player.getWorldDaysElapsed() / 2));
            if (shown >= 6) break;
            if (rumorRoll > 55 && profile.id != "bras_casses") continue;
            std::string partialName = profile.rareGroup ? "un groupe héroïque dont le nom change selon la taverne" : ("un groupe de " + profile.kind);
            std::string certainty = rumorRoll < 18 ? "rumeur faible" : (rumorRoll < 38 ? "rumeur crédible" : "rumeur floue");
            lines.push_back("- " + partialName + " — " + certainty + " | spécialité supposée : " + (profile.strengths.empty() ? std::string("inconnue") : missionTypeDisplayName(profile.strengths.front())) + " | condition probable : " + contractorDiscoveryHint(profile));
            player.recordCanonicalEvent("rumeurs_groupes_pnj", profile.id, "Rumeur entendue : " + partialName);
            ++shown;
        }
        if (shown == 0)
        {
            lines.push_back("Aucune rumeur utile aujourd'hui. Les tavernes répètent surtout des histoires déjà trop embellies.");
        }
        MessageScreen::show("RUMEURS DE GROUPES", "quest.delegated_missions.rumors", lines, false);
    }

    void openContractorDiscoveryBoard(Player& player)
    {
        if (guildBanActive(player))
        {
            MessageScreen::show("GUILDE EN SUSPENSION", "quest.delegated_missions.contacts.guild_ban", guildBanLines(player), false);
            return;
        }

        if (!guildRequestRankDUnlocked(player))
        {
            MessageScreen::show("RANG DE GUILDE INSUFFISANT", "quest.delegated_missions.contacts.rank_gate", guildRequestRankGateLines(player), false);
            return;
        }

        while (true)
        {
            std::vector<ContractorProfile> candidates;
            for (const ContractorProfile& profile : buildAllContractorProfiles(player))
            {
                if (!contractorDiscovered(player, profile.id) && contractorDiscoveryConditionsMet(player, profile))
                {
                    candidates.push_back(profile);
                }
            }

            MenuScreen screen("CONTACTS DE GUILDE", "quest.delegated_missions.contacts");
            screen.addLine("Les groupes ne sont plus affichés gratuitement : il faut d'abord en entendre parler et faire connaissance.");
            screen.addLine("Une recommandation coûte peu, mais elle ne garantit pas que le groupe acceptera ensuite tes mandats.");
            screen.addBackOption("Retour", "quest.delegated_missions.contacts.back");

            if (candidates.empty())
            {
                screen.addLine("Aucun nouveau contact accessible aujourd'hui avec ton niveau, tes preuves et la ville actuelle.");
                screen.addLine("Certains groupes demandent un niveau, une réputation, une présence rare, ou un événement avant d'être présentés.");
            }
            else
            {
                for (std::size_t i = 0; i < candidates.size(); ++i)
                {
                    const ContractorProfile& profile = candidates[i];
                    const int contactCost = std::max(20, 35 + player.getLevel() * 3 + contractorRequiredLevel(profile.id) * 6 + (profile.rareGroup ? 220 : 0));
                    std::string label = "Demander une présentation — " + profile.kind;
                    std::string detail = contractorDiscoveryHint(profile) + " | niveau requis " + std::to_string(contractorRequiredLevel(profile.id)) + " | coût " + Money::formatCopper(contactCost) + ".";
                    if (profile.rareGroup) detail += " Présence rarissime : si tu rates le jour, ils ne restent pas au comptoir.";
                    screen.addOption(static_cast<int>(i + 1), label, detail, true, "quest.delegated_missions.contacts." + profile.id);
                }
            }

            const int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choix invalide.");
            Console::clear();
            if (choice == 0)
            {
                return;
            }
            if (choice < 1 || choice > static_cast<int>(candidates.size()))
            {
                continue;
            }

            ContractorProfile profile = candidates[static_cast<std::size_t>(choice - 1)];
            const int contactCost = std::max(20, 35 + player.getLevel() * 3 + contractorRequiredLevel(profile.id) * 6 + (profile.rareGroup ? 220 : 0));
            if (!player.getInventory().spendCopper(contactCost))
            {
                MessageScreen::show("ARGENT INSUFFISANT", "quest.delegated_missions.contacts.no_money", {
                    "Présentation demandée : " + Money::formatCopper(contactCost) + ".",
                    "Argent actuel : " + player.getInventory().getWalletLine() + "."
                }, false);
                continue;
            }

            discoverContractor(player, profile, "présentation officielle au comptoir de " + currentCityName(player));
            MessageScreen::show("CONTACT DÉCOUVERT", "quest.delegated_missions.contacts.discovered", {
                "Nouveau groupe connu : " + profile.name + ".",
                "Type : " + profile.kind + ".",
                profile.detail,
                contractorRankLabel(player, profile),
                contractorPreferenceLine(profile),
                "Ils apparaîtront maintenant dans les mandats directs quand ils sont disponibles."
            }, false);
        }
    }


    void openGuidedLowRankContractBoard(Player& player)
    {
        if (!guildRequestRankEUnlocked(player))
        {
            MessageScreen::show("CONTRAT ENCADRÉ BLOQUÉ", "quest.delegated_missions.guided.rank_gate", {"La guilde réserve même les petits contrats encadrés aux rangs E minimum.", "But : apprendre le système sans donner une délégation gratuite à un personnage encore tout débutant."}, false);
            return;
        }
        if (guildBanActive(player))
        {
            MessageScreen::show("GUILDE EN SUSPENSION", "quest.delegated_missions.guided.banned", guildBanLines(player), false);
            return;
        }

        std::vector<DelegatedMissionTemplate> templates;
        for (const DelegatedMissionTemplate& mission : buildDelegatedMissionTemplates(player))
        {
            if (mission.type == "local_service" || mission.type == "route_scout")
            {
                DelegatedMissionTemplate limited = mission;
                limited.costCopper = std::max(70, mission.costCopper / 2);
                limited.durationDays = 1;
                limited.successPercent = std::min(78, mission.successPercent);
                limited.dangerous = false;
                templates.push_back(limited);
            }
        }

        while (true)
        {
            MenuScreen screen("CONTRATS ENCADRÉS BAS RANG", "quest.delegated_missions.guided");
            screen.addLine("Service propre proposé par la guilde aux rangs E : peu rentable, sans combat, sans rareté, mais légal.");
            screen.addLine("Cela apprend le système avant le rang D sans ouvrir les mandats forts.");
            screen.addBackOption("Retour", "quest.delegated_missions.guided.back");
            for (std::size_t i = 0; i < templates.size(); ++i)
            {
                const DelegatedMissionTemplate& t = templates[i];
                screen.addOption(static_cast<int>(i + 1), "Contrat encadré : " + t.label, t.detail + " | coût " + Money::formatCopper(t.costCopper) + " | retour demain | récompense très limitée.", true, "quest.delegated_missions.guided." + t.type);
            }
            const int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choix invalide.");
            Console::clear();
            if (choice == 0) return;
            if (choice < 1 || choice > static_cast<int>(templates.size())) continue;

            const DelegatedMissionTemplate selected = templates[static_cast<std::size_t>(choice - 1)];
            const int cost = selected.costCopper;
            if (!player.getInventory().spendCopper(cost))
            {
                MessageScreen::show("ARGENT INSUFFISANT", "quest.delegated_missions.guided.no_money", {"Coût : " + Money::formatCopper(cost) + ".", "Argent actuel : " + player.getInventory().getWalletLine() + "."}, false);
                continue;
            }
            const std::string profileId = selected.type == "local_service" ? "scribes_ecu" : "lanternes";
            const std::string profileName = contractorNameFromId(profileId);
            const int dueDay = player.getWorldDaysElapsed() + 1;
            const std::string key = "type:" + selected.type + "|city:" + player.getCurrentCityId() + "|due:" + std::to_string(dueDay) + "|duration:1|cost:" + std::to_string(cost) + "|rate:" + std::to_string(selected.successPercent) + "|profile:" + profileId + "|guided:1|seq:" + std::to_string(player.getCanonicalJournalRecords().size());
            player.recordCanonicalEvent("missions_deleguees_actives", key, "Contrat encadré : " + selected.label + " par " + profileName);
            player.recordCanonicalEvent("contrats_encadres_bas_rang", selected.type, selected.label);
            recordRecentAction(player, "guided_low_rank_contract", "Contrat encadré lancé : " + selected.label);
            MessageScreen::show("CONTRAT ENCADRÉ LANCÉ", "quest.delegated_missions.guided.started", {"Mission : " + selected.label + ".", "Groupe encadré : " + profileName + ".", "Retour prévu : jour " + std::to_string(dueDay + 1) + ".", "Rappel : légal mais volontairement limité avant le rang D."}, false);
            return;
        }
    }

    void openIllegalLowRankRequestBoard(Player& player)
    {
        if (guildBanActive(player))
        {
            MessageScreen::show("GUILDE EN SUSPENSION", "quest.delegated_missions.illegal.banned", guildBanLines(player), false);
            return;
        }

        while (true)
        {
            MenuScreen screen("DEMANDE NON OFFICIELLE", "quest.delegated_missions.illegal");
            screen.addLine("Tu n'as pas encore le rang D : la guilde ne valide normalement pas les mandats de client.");
            screen.addLine("Certains groupes douteux acceptent quand même, mais ce n'est pas légal : prix gonflé, vol possible, aucune garantie et risque de suspension de guilde.");
            screen.addLine("Aucun remboursement officiel. Si le groupe se fait attraper et te balance, tu peux être banni quelques jours de cette guilde.");
            screen.addLine("Réputation souterraine : " + std::to_string(undergroundReputationScore(player)) + " — aide un peu avec les contacts louches, mais augmente le risque d'être connu des mauvaises personnes.");
            screen.addBackOption("Retour", "quest.delegated_missions.illegal.back");

            std::vector<DelegatedMissionTemplate> templates;
            for (const DelegatedMissionTemplate& mission : buildDelegatedMissionTemplates(player))
            {
                if (mission.type == "boss_materials" || mission.type == "rescue_group") continue;
                templates.push_back(mission);
            }
            for (std::size_t i = 0; i < templates.size(); ++i)
            {
                const DelegatedMissionTemplate& mission = templates[i];
                std::string detail = mission.detail + " | illégal : coût plus élevé, pas de remboursement, risque de vol ou de dénonciation.";
                if (mission.type == "monster_hunt") detail += " Chasse de monstre : plus dangereuse, donc plus de chance que ça tourne mal.";
                screen.addOption(static_cast<int>(i + 1), "Demander hors guilde : " + mission.label, detail, true, "quest.delegated_missions.illegal." + mission.type);
            }

            const int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choix invalide.");
            Console::clear();
            if (choice == 0) return;
            if (choice < 1 || choice > static_cast<int>(templates.size())) continue;

            const DelegatedMissionTemplate selected = templates[static_cast<std::size_t>(choice - 1)];
            std::vector<ContractorProfile> profiles = buildIllegalContractorProfiles(player);
            MenuScreen profileScreen("CONTACT LOUCHE", "quest.delegated_missions.illegal.profile");
            profileScreen.addLine("Mission : " + selected.label + ".");
            profileScreen.addLine("Ces profils ne sont pas présentés proprement par la guilde. Certains peuvent être inconnus officiellement.");
            profileScreen.addBackOption("Annuler", "quest.delegated_missions.illegal.profile.back");
            for (std::size_t i = 0; i < profiles.size(); ++i)
            {
                const ContractorProfile& profile = profiles[i];
                const int cost = illegalMissionCost(selected, profile);
                const int underground = undergroundReputationScore(player);
                const int success = std::max(5, profileMissionSuccess(player, selected, profile) - 12);
                const int accept = clampPercent(profileMissionAcceptance(player, selected, profile) - 10 + underground / 4);
                const int theft = clampPercent(illegalTheftPercent(profile, selected) - underground / 8);
                const int denounce = clampPercent(illegalDenouncePercent(profile, selected) + underground / 5);
                std::string detail = profile.kind + " | " + profileFitLabel(selected, profile)
                    + " | accepte " + std::to_string(accept) + "%"
                    + " | réussite " + std::to_string(success) + "%"
                    + " | vol " + std::to_string(theft) + "%"
                    + " | caftage " + std::to_string(denounce) + "%"
                    + " | coût " + Money::formatCopper(cost) + ".";
                if (!contractorDiscovered(player, profile.id)) detail += " Contact non officiel : ne débloque pas automatiquement une relation saine.";
                profileScreen.addOption(static_cast<int>(i + 1), profile.name, detail, profile.available, "quest.delegated_missions.illegal.profile." + profile.id);
            }

            const int profileChoice = TerminalInterface::askMenuChoiceFromOptions(profileScreen, "Choix invalide.");
            Console::clear();
            if (profileChoice <= 0 || profileChoice > static_cast<int>(profiles.size())) continue;
            const ContractorProfile profile = profiles[static_cast<std::size_t>(profileChoice - 1)];

            const int finalCost = illegalMissionCost(selected, profile);
            const int underground = undergroundReputationScore(player);
            const int success = std::max(5, profileMissionSuccess(player, selected, profile) - 12);
            const int accept = clampPercent(profileMissionAcceptance(player, selected, profile) - 10 + underground / 4);
            const int theft = clampPercent(illegalTheftPercent(profile, selected) - underground / 8);
            const int denounce = clampPercent(illegalDenouncePercent(profile, selected) + underground / 5);

            MenuScreen confirm("CONFIRMER LA DEMANDE ILLÉGALE", "quest.delegated_missions.illegal.confirm");
            confirm.addLine("Mission : " + selected.label + ".");
            confirm.addLine("Contact : " + profile.name + " — " + profile.kind + ".");
            confirm.addLine("Coût non officiel : " + Money::formatCopper(finalCost) + ".");
            confirm.addLine("Durée si ça part vraiment : " + std::to_string(selected.durationDays) + " jour(s).");
            confirm.addLine("Chance d'acceptation : " + std::to_string(accept) + "%.");
            confirm.addLine("Taux de réussite : " + std::to_string(success) + "%.");
            confirm.addLine("Risque de vol immédiat : " + std::to_string(theft) + "%.");
            confirm.addLine("Risque de dénonciation si ça tourne mal : " + std::to_string(denounce) + "%.");
            confirm.addLine("C'est volontairement risqué : cela contourne le rang D, donc pas de confort gratuit.");
            confirm.addBackOption("Annuler", "quest.delegated_missions.illegal.confirm.back");
            confirm.addOption(1, "Payer hors registre", "Aucun remboursement officiel, et le contact peut disparaître avec l'argent.", true, "quest.delegated_missions.illegal.confirm.pay");
            const int confirmChoice = TerminalInterface::askMenuChoiceFromOptions(confirm, "Choix invalide.");
            Console::clear();
            if (confirmChoice != 1) continue;

            if (!player.getInventory().spendCopper(finalCost))
            {
                MessageScreen::show("ARGENT INSUFFISANT", "quest.delegated_missions.illegal.no_money", {
                    "Somme demandée : " + Money::formatCopper(finalCost) + ".",
                    "Argent actuel : " + player.getInventory().getWalletLine() + "."
                }, false);
                continue;
            }

            std::vector<std::string> lines;
            lines.push_back("Tu paies hors registre : " + Money::formatCopper(finalCost) + ".");
            player.recordCanonicalEvent("demandes_illegales_couts", selected.type, selected.label, finalCost);
            player.recordCanonicalEvent("demandes_illegales_tentees", profile.id, profile.name);

            const int roll = stableMissionRoll("illegal_start:" + profile.id + ":" + selected.type + ":" + std::to_string(player.getWorldDaysElapsed()) + ":" + std::to_string(player.getCanonicalJournalRecords().size()));
            if (roll < theft)
            {
                player.recordCanonicalEvent("demandes_illegales_vols", profile.id, profile.name + " disparaît avec l'argent");
                recordRecentAction(player, "illegal_stolen", "Demande illégale volée : " + selected.label + " par " + profile.name);
                lines.push_back(profile.name + " prend l'argent et ne revient pas. Aucune guilde ne rembourse une demande qui n'existe officiellement pas.");
                if (stableMissionRoll("illegal_theft_snitch:" + profile.id + ":" + std::to_string(player.getCanonicalJournalRecords().size())) < denounce)
                {
                    applyGuildBan(player, profile.name + " a cafté pour éviter les ennuis après un mandat illégal", 2 + (roll % 3), lines);
                }
                MessageScreen::show("ARGENT VOLÉ", "quest.delegated_missions.illegal.stolen", lines, false);
                continue;
            }

            const int acceptRoll = stableMissionRoll("illegal_accept:" + profile.id + ":" + selected.type + ":" + std::to_string(player.getWorldDaysElapsed()) + ":" + std::to_string(player.getCanonicalJournalRecords().size()));
            if (acceptRoll >= accept)
            {
                player.recordCanonicalEvent("demandes_illegales_refusees", selected.type, selected.label + " refusée par " + profile.name);
                lines.push_back(profile.name + " refuse finalement de partir. L'argent déjà glissé sous la table n'est pas rendu complètement.");
                const int consolation = std::max(0, finalCost / 4);
                if (consolation > 0)
                {
                    player.getInventory().earnCopper(consolation);
                    lines.push_back("Ils rendent quand même une petite partie pour éviter une bagarre : " + Money::formatCopper(consolation) + ".");
                }
                MessageScreen::show("DEMANDE REFUSÉE", "quest.delegated_missions.illegal.refused", lines, false);
                continue;
            }

            const int dueDay = player.getWorldDaysElapsed() + selected.durationDays;
            const std::string key = "type:" + selected.type
                + "|city:" + player.getCurrentCityId()
                + "|due:" + std::to_string(dueDay)
                + "|duration:" + std::to_string(selected.durationDays)
                + "|cost:" + std::to_string(finalCost)
                + "|rate:" + std::to_string(success)
                + "|profile:" + profile.id
                + "|illegal:1"
                + "|snitch:" + std::to_string(denounce)
                + "|seq:" + std::to_string(player.getCanonicalJournalRecords().size());
            const std::string label = "Demande illégale : " + selected.label + " par " + profile.name + " depuis " + currentCityName(player);
            player.recordCanonicalEvent("missions_deleguees_actives", key, label);
            player.recordCanonicalEvent("demandes_illegales_lancees", selected.type, selected.label);
            recordRecentAction(player, "illegal_start", label);
            if (contractorDiscovered(player, profile.id))
            {
                player.recordCanonicalEvent("profils_pnj_mandates", profile.id, profile.name);
            }
            lines.push_back("Le contact accepte hors registre. Retour prévu : jour " + std::to_string(dueDay + 1) + ".");
            lines.push_back("Attention : si ça tourne mal et qu'ils parlent, la guilde peut suspendre ton dossier localement.");
            MessageScreen::show("DEMANDE ILLÉGALE LANCÉE", "quest.delegated_missions.illegal.started", lines, false);
        }
    }

    void openKnownContractorDossiers(Player& player)
    {
        while (true)
        {
            const std::vector<ContractorProfile> profiles = buildContractorProfiles(player);
            MenuScreen screen("DOSSIERS DE GROUPES", "quest.delegated_missions.dossiers");
            screen.addLine("Fiches des groupes déjà découverts. Les inconnus restent cachés pour garder une progression naturelle.");
            screen.addLine("Ces fiches montrent la relation, les préférences, les incidents et les indisponibilités sans donner un groupe gratuit.");
            screen.addBackOption("Retour", "quest.delegated_missions.dossiers.back");
            if (profiles.empty())
            {
                screen.addLine("Aucun groupe connu pour le moment. Cherche des contacts ou croise des groupes en route.");
            }
            for (std::size_t i = 0; i < profiles.size(); ++i)
            {
                const ContractorProfile& profile = profiles[i];
                std::string detail = profile.kind + " | " + contractorRankLabel(player, profile) + " | " + contractorAvailabilityLine(profile) + ".";
                screen.addOption(static_cast<int>(i + 1), profile.name, detail, true, "quest.delegated_missions.dossiers." + profile.id);
            }

            const int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choix invalide.");
            Console::clear();
            if (choice == 0)
            {
                return;
            }
            if (choice < 1 || choice > static_cast<int>(profiles.size()))
            {
                continue;
            }

            const ContractorProfile& profile = profiles[static_cast<std::size_t>(choice - 1)];
            std::vector<std::string> lines;
            lines.push_back(profile.name + " — " + profile.kind + ".");
            lines.push_back(profile.detail);
            lines.push_back(contractorRankLabel(player, profile));
            lines.push_back(contractorPreferenceLine(profile));
            lines.push_back(contractorRelationshipLine(player, profile));
            lines.push_back("Disponibilité : " + contractorAvailabilityLine(profile));
            if (profile.id == "bande_nero")
            {
                lines.push_back("Rivalité : les groupes sérieux peuvent moins aimer que tu relies trop souvent ton dossier à la Bande de Néro.");
            }
            if (profile.id == "bras_casses")
            {
                lines.push_back("Lore : groupe héroïque principal, presque toujours un cran au-dessus du joueur. Les vexer peut finir en coma narratif, pas en exécution.");
            }
            lines.push_back("");
            lines.push_back("Historique rapide :");
            lines.push_back("- Mandats confiés : " + std::to_string(canonicalRecordCount(player, "profils_pnj_mandates", profile.id)) + ".");
            lines.push_back("- Réussites : " + std::to_string(canonicalRecordCount(player, "missions_deleguees_reussies_par_profil", profile.id)) + ".");
            lines.push_back("- Échecs : " + std::to_string(canonicalRecordCount(player, "missions_deleguees_echouees_par_profil", profile.id)) + ".");
            lines.push_back("- Sauvetages reçus : " + std::to_string(canonicalRecordCount(player, "sauvetages_groupes_reussis", profile.id)) + ".");
            lines.push_back("- Incidents : " + std::to_string(canonicalRecordCount(player, "incidents_groupes_pnj", profile.id)) + ".");
            MessageScreen::show("FICHE DE GROUPE", "quest.delegated_missions.dossiers.view", lines, false);
        }
    }

    void openDelegatedMissionBoard(Player& player)
    {
        while (true)
        {
            std::vector<std::string> resolvedLines = resolveDueDelegatedMissions(player);
            std::vector<std::string> postedLines = resolveDuePostedQuests(player);
            resolvedLines.insert(resolvedLines.end(), postedLines.begin(), postedLines.end());
            if (!resolvedLines.empty())
            {
                MessageScreen::show("COMPTES RENDUS", "quest.delegated_missions.reports", resolvedLines, false);
            }

            if (guildBanActive(player))
            {
                MenuScreen banScreen("GUILDE EN SUSPENSION", "quest.delegated_missions.guild_ban");
                for (const std::string& line : guildBanLines(player))
                {
                    banScreen.addLine(line);
                }
                banScreen.addBackOption("Retour", "quest.delegated_missions.guild_ban.back");
                banScreen.addOption(1, "Parler au médiateur", "Amende, excuses ou service propre pour réduire la suspension sans effacer la faute.", true, "quest.delegated_missions.guild_ban.mediation");
                const int banChoice = TerminalInterface::askMenuChoiceFromOptions(banScreen, "Choix invalide.");
                Console::clear();
                if (banChoice == 1)
                {
                    openGuildMediationMenu(player);
                    continue;
                }
                return;
            }

            if (!guildRequestRankDUnlocked(player))
            {
                MenuScreen lowRankScreen("RANG DE GUILDE INSUFFISANT", "quest.delegated_missions.rank_gate");
                for (const std::string& line : guildRequestRankGateLines(player))
                {
                    lowRankScreen.addLine(line);
                }
                lowRankScreen.addLine("");
                lowRankScreen.addLine("Exception non officielle : certains contacts douteux acceptent parfois les bas rangs, mais c'est illégal, cher et risqué.");
                lowRankScreen.addBackOption("Retour", "quest.delegated_missions.rank_gate.back");
                lowRankScreen.addOption(1, "Tenter une demande illégale", "Risque : vol de l'argent, aucun remboursement, dénonciation et suspension de guilde.", true, "quest.delegated_missions.rank_gate.illegal");
                lowRankScreen.addOption(2, "Contrat encadré légal bas rang", "Disponible dès le rang E : service local ou reconnaissance sans combat ni rareté.", guildRequestRankEUnlocked(player), "quest.delegated_missions.rank_gate.guided");
                const int lowRankChoice = TerminalInterface::askMenuChoiceFromOptions(lowRankScreen, "Choix invalide.");
                Console::clear();
                if (lowRankChoice == 1)
                {
                    openIllegalLowRankRequestBoard(player);
                    continue;
                }
                if (lowRankChoice == 2)
                {
                    openGuidedLowRankContractBoard(player);
                    continue;
                }
                return;
            }

            MenuScreen screen("MISSIONS DÉLÉGUÉES", "quest.delegated_missions");
            screen.addLine("Tu peux payer directement des groupes connus ou publier une quête à la guilde comme un vrai client.");
            screen.addLine("Les groupes inconnus ne sont plus affichés : il faut d'abord les découvrir et faire connaissance.");
            screen.addLine("Les PNJ peuvent refuser, surtout si le profil ne correspond pas à la mission ou si elle est dangereuse.");
            screen.addLine("Date actuelle : " + player.formatWorldDateTimeLine() + ".");
            if (guildProbationActive(player))
            {
                screen.addLine("Probation locale : coûts officiels plus élevés et groupes sérieux plus prudents jusqu'au jour " + std::to_string(activeGuildProbationUntilDay(player) + 1) + ".");
            }
            const std::vector<PlayerJournalRecord> active = getActiveDelegatedMissions(player);
            const std::vector<PlayerJournalRecord> pendingPosted = getActivePostedQuests(player);
            const std::vector<ContractorProfile> knownProfiles = buildContractorProfiles(player);
            screen.addLine("Groupes connus : " + std::to_string(knownProfiles.size()) + ".");
            if (active.empty())
            {
                screen.addLine("Mission active : aucune.");
            }
            else
            {
                screen.addLine("Missions actives :");
                for (const PlayerJournalRecord& mission : active)
                {
                    const int dueDay = missionDueDayFromKey(mission.key);
                    screen.addLine("- " + mission.label + " | retour prévu jour " + std::to_string(dueDay + 1) + ".");
                }
            }
            if (!pendingPosted.empty())
            {
                screen.addLine("Quêtes publiées en attente : " + std::to_string(pendingPosted.size()) + ".");
            }
            screen.addBackOption("Retour", "quest.delegated_missions.back");
            screen.addOption(90, "Publier une quête à la guilde", "Coûte une mise. Si aucun PNJ n'accepte sous 2 jours, remboursement de moitié.", true, "quest.delegated_missions.post_quest");
            screen.addOption(91, "Chercher un nouveau contact", "Demander à la guilde une présentation progressive avant de pouvoir mandater un groupe.", true, "quest.delegated_missions.contacts");
            screen.addOption(92, "Voir les dossiers des groupes connus", "Relation, préférences, historique, incidents et disponibilité des groupes déjà découverts.", !knownProfiles.empty(), "quest.delegated_missions.dossiers");
            screen.addOption(93, "Écouter les rumeurs de groupes", "Rumeurs partielles : nom incertain, spécialité supposée, conditions probables. Ne débloque pas l'embauche.", true, "quest.delegated_missions.rumors");

            const std::vector<DelegatedMissionTemplate> templates = buildDelegatedMissionTemplates(player);
            const bool missionLimitReached = active.size() >= 4;
            for (std::size_t i = 0; i < templates.size(); ++i)
            {
                const DelegatedMissionTemplate& t = templates[i];
                std::string detail = t.detail + " | base " + Money::formatCopper(t.costCopper) + " | durée " + std::to_string(t.durationDays) + " jour(s) | réussite selon profil.";
                if (t.type == "monster_hunt") detail += " Matériaux de monstre possibles, mais mission risquée.";
                if (t.type == "boss_materials") detail += " Matériaux de boss possibles, taux très faible hors Bras Cassés.";
                if (knownProfiles.empty()) detail += " Aucun groupe connu : cherche d'abord un contact.";
                screen.addOption(static_cast<int>(i + 1), "Mandater directement : " + t.label, detail, !missionLimitReached && !knownProfiles.empty(), "quest.delegated_missions." + t.type);
            }

            const int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choix invalide.");
            Console::clear();
            if (choice == 0)
            {
                return;
            }
            if (choice == 90)
            {
                openPostedQuestBoard(player);
                continue;
            }
            if (choice == 91)
            {
                openContractorDiscoveryBoard(player);
                continue;
            }
            if (choice == 92)
            {
                openKnownContractorDossiers(player);
                continue;
            }
            if (choice == 93)
            {
                showContractorRumors(player);
                continue;
            }
            if (choice < 1 || choice > static_cast<int>(templates.size()) || missionLimitReached || knownProfiles.empty())
            {
                continue;
            }

            const DelegatedMissionTemplate selected = templates[static_cast<std::size_t>(choice - 1)];
            const std::vector<ContractorProfile> profiles = buildContractorProfiles(player);
            MenuScreen profileScreen("CHOISIR UN PROFIL", "quest.delegated_missions.profile");
            profileScreen.addLine("Mission : " + selected.label + ".");
            profileScreen.addLine("Choisir le bon profil augmente la réussite. Choisir n'importe qui peut provoquer un refus ou un échec.");
            profileScreen.addBackOption("Annuler", "quest.delegated_missions.profile.back");
            for (std::size_t i = 0; i < profiles.size(); ++i)
            {
                const ContractorProfile& profile = profiles[i];
                const int cost = officialMissionCost(player, selected, profile);
                const int success = profileMissionSuccess(player, selected, profile);
                const int acceptance = officialMissionAcceptance(player, selected, profile);
                std::string detail = profile.detail + " | " + contractorRankLabel(player, profile) + " | " + profileFitLabel(selected, profile) + " | accepte " + std::to_string(acceptance) + "% | réussite " + std::to_string(success) + "% | coût " + Money::formatCopper(cost) + ".";
                detail += " | " + contractorPreferenceLine(profile);
                detail += " | " + contractorRelationshipLine(player, profile);
                detail += " | " + contractorAvailabilityLine(profile);
                profileScreen.addOption(static_cast<int>(i + 1), profile.name, detail, profile.available, "quest.delegated_missions.profile." + profile.id);
            }
            const int profileChoice = TerminalInterface::askMenuChoiceFromOptions(profileScreen, "Choix invalide.");
            Console::clear();
            if (profileChoice <= 0 || profileChoice > static_cast<int>(profiles.size()))
            {
                continue;
            }
            const ContractorProfile profile = profiles[static_cast<std::size_t>(profileChoice - 1)];
            if (!profile.available)
            {
                MessageScreen::show("GROUPE INDISPONIBLE", "quest.delegated_missions.profile.unavailable", {
                    profile.name + " n'est pas disponible.",
                    contractorAvailabilityLine(profile),
                    "Reviens plus tard ou choisis un autre groupe."
                }, false);
                continue;
            }
            const int finalCost = officialMissionCost(player, selected, profile);
            const int finalSuccess = profileMissionSuccess(player, selected, profile);
            const int finalAcceptance = officialMissionAcceptance(player, selected, profile);

            MenuScreen confirm("MANDATER UNE ÉQUIPE", "quest.delegated_missions.confirm");
            confirm.addLine(selected.label);
            confirm.addLine(selected.detail);
            confirm.addLine("Profil : " + profile.name + " — " + profileFitLabel(selected, profile) + ".");
            confirm.addLine("Coût : " + Money::formatCopper(finalCost) + ".");
            confirm.addLine("Durée : " + std::to_string(selected.durationDays) + " jour(s), pas segment(s).");
            confirm.addLine("Chance que le profil accepte : " + std::to_string(finalAcceptance) + "%.");
            confirm.addLine("Taux de réussite si accepté : " + std::to_string(finalSuccess) + "%.");
            confirm.addLine("Dialogue : " + contractorDialogueLine(profile, selected, true));
            confirm.addBackOption("Annuler", "quest.delegated_missions.confirm.back");
            confirm.addOption(1, "Proposer le mandat", "Le PNJ/groupe peut refuser avant paiement.", true, "quest.delegated_missions.confirm.pay");
            if (profile.id == "bras_casses")
            {
                confirm.addOption(2, "Faire une remarque de travers", "Anecdotique et très rare, mais les Bras Cassés peuvent très mal le prendre.", true, "quest.delegated_missions.confirm.taunt_bras_casses");
            }
            const int confirmChoice = TerminalInterface::askMenuChoiceFromOptions(confirm, "Choix invalide.");
            Console::clear();
            if (confirmChoice == 2 && profile.id == "bras_casses")
            {
                const int rebelRoll = stableMissionRoll("bras_casses_taunt:" + std::to_string(player.getWorldDaysElapsed()) + ":" + std::to_string(player.getCanonicalJournalRecords().size()));
                if (rebelRoll < 7)
                {
                    const int damage = player.getHp() > 1 ? std::min(player.getHp() - 1, std::max(1, player.getMaxHp() / 5)) : 0;
                    if (damage > 0) player.takeDamage(damage);
                    player.advanceWorldDayUnits(2);
                    player.recordCanonicalEvent("incidents_bras_casses", "rebellion_non_legale", "Les Bras Cassés remettent le joueur à sa place sans le tuer");
                    recordRecentAction(player, "bras_casses_rebellion", "Les Bras Cassés t'ont mis au sol après une remarque stupide");
                    MessageScreen::show("LES BRAS CASSÉS SE VEXENT", "quest.delegated_missions.bras_casses.rebellion", {
                        "Tu as dit exactement le genre de phrase qu'il ne fallait pas dire.",
                        "Réaction rare : ils ne te tuent pas, même en règles définitives. Ils te mettent en coma narratif / au sol pour te calmer.",
                        "PV perdus : " + std::to_string(damage) + ". Temps perdu : 2 segments.",
                        "Illégal, anecdotique, et clairement pas une bonne stratégie."
                    }, false);
                }
                else
                {
                    player.recordCanonicalEvent("incidents_bras_casses", "remarque_ratee", "Les Bras Cassés ignorent une provocation maladroite");
                    MessageScreen::show("REMARQUE IGNORÉE", "quest.delegated_missions.bras_casses.ignored", {
                        "Les Bras Cassés te regardent comme si tu venais d'essayer d'intimider une tempête.",
                        "Ils ne se rebellent pas cette fois. Ils partent juste sans prendre le mandat."
                    }, false);
                }
                continue;
            }
            if (confirmChoice != 1)
            {
                continue;
            }

            const int refusalRoll = stableMissionRoll(profile.id + ":" + selected.type + ":" + std::to_string(player.getWorldDaysElapsed()) + ":" + std::to_string(player.getCanonicalJournalRecords().size()));
            if (refusalRoll >= finalAcceptance)
            {
                player.recordCanonicalEvent("missions_deleguees_refusees", selected.type, selected.label + " refusée par " + profile.name);
                recordRecentAction(player, "delegated_refused", profile.name + " refuse : " + selected.label);
                MessageScreen::show("MANDAT REFUSÉ", "quest.delegated_missions.refused", {
                    profile.name + " refuse le mandat.",
                    "Dialogue : " + contractorDialogueLine(profile, selected, false),
                    "Raison probable : risque, mauvais profil, prix jugé insuffisant ou disponibilité limitée.",
                    "Aucun argent n'a été retiré."
                }, false);
                continue;
            }

            if (!player.getInventory().spendCopper(finalCost))
            {
                MessageScreen::show("ARGENT INSUFFISANT", "quest.delegated_missions.no_money", {"Coût demandé : " + Money::formatCopper(finalCost) + ".", "Argent actuel : " + player.getInventory().getWalletLine() + "."}, false);
                continue;
            }

            const int dueDay = player.getWorldDaysElapsed() + selected.durationDays;
            const std::string key = "type:" + selected.type + "|city:" + player.getCurrentCityId() + "|due:" + std::to_string(dueDay) + "|duration:" + std::to_string(selected.durationDays) + "|cost:" + std::to_string(finalCost) + "|rate:" + std::to_string(finalSuccess) + "|profile:" + profile.id + "|seq:" + std::to_string(player.getCanonicalJournalRecords().size());
            const std::string label = selected.label + " par " + profile.name + " depuis " + currentCityName(player);
            player.recordCanonicalEvent("missions_deleguees_actives", key, label);
            player.recordCanonicalEvent("missions_deleguees_couts", selected.type, selected.label, finalCost);
            player.recordCanonicalEvent("profils_pnj_mandates", profile.id, profile.name);
            recordRecentAction(player, "delegated_start", "Mission déléguée lancée : " + selected.label + " avec " + profile.name);
            MessageScreen::show("MISSION LANCÉE", "quest.delegated_missions.started", {
                "Mission : " + selected.label + ".",
                "Profil : " + profile.name + ".",
                "Dialogue : " + contractorDialogueLine(profile, selected, true),
                "Retour prévu : jour " + std::to_string(dueDay + 1) + ".",
                "Les aventuriers viendront faire leur compte rendu quand tu repasseras par ce bureau après la date prévue.",
                "Rappel : ce système sert au confort, pas à automatiser tout le jeu."
            }, false);
        }
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
        lines.push_back("Un futur service coûteux pourra transférer une petite quantité entre coffres municipaux avec plusieurs jours de délai.");
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
        lines.push_back("But : préparer helpmerefundmyaction et éviter de deviner l'historique depuis le texte affiché.");
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
                lines.push_back("Résultat : partiel. Pas de vraie récompense d'or, mais une note de route utile.");
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
        std::vector<std::string> lines = CityTravelRules::buildExplorationMapLines(player);
        const std::vector<std::string> visualLines = CityTravelRules::buildFutureVisualPlanningLines();
        lines.push_back("");
        lines.insert(lines.end(), visualLines.begin(), visualLines.end());
        MessageScreen::show("CARTE D'EXPLORATION — PRÉPARATION", "quest.city_travel.exploration_map", lines, false);
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
            screen.addLine("Hub jouable actuel : les bâtiments sont encore en terminal, mais chaque entrée prépare le futur clic pixel-art.");
            screen.addLine("Plus tard : vraie image de ville, bâtiments cliquables, arène visible et grande porte vers la carte.");
            screen.addBackOption("Retour", "quest.city_hub.menu.back");
            screen.addOption(90, "Résumé de la ville", "Identité locale, ressources, stocks, bâtiments et conditions.", true, "quest.city_hub.summary");
            for (std::size_t i = 0; i < buildings.size(); ++i)
            {
                const CityBuildingPreview& building = buildings[i];
                std::string detail = building.category + " — " + building.contact + " | " + building.detail + " | Pixel-art futur : " + building.pixelArtHint + ".";
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
            if (choice < 1 || choice > static_cast<int>(buildings.size())) continue;

            const CityBuildingPreview building = buildings[static_cast<std::size_t>(choice - 1)];
            if (!building.unlocked)
            {
                MessageScreen::show("BÂTIMENT VERROUILLÉ", "quest.city_hub.building.locked", {building.name + " n'est pas encore accessible.", building.detail}, false);
                continue;
            }

            if (building.id == "guild")
            {
                openGuildTribunalMenu(player);
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
                openDelegatedMissionBoard(player);
            }
            else if (building.id == "gate" || building.id == "mine_lift" || building.id == "frost_gate")
            {
                showExplorationMapPreview(player);
            }
            else if (building.id == "arena")
            {
                MessageScreen::show("ARÈNE DE VILLE", "quest.city_hub.arena", {
                    "Accès logique préparé : combat unique / entraînement via bâtiment d'arène.",
                    "La sélection réelle de combat reste dans les modes de combat actuels pour ne pas dupliquer les règles.",
                    "Future IG : ce bâtiment deviendra le point cliquable d'entrée vers le combat unique."
                }, false);
            }
            else if (building.id == "market" || building.id == "harbor" || building.id == "underbridge")
            {
                std::vector<std::string> lines = CityTravelRules::buildLocalCityDifferentiationLines(player);
                lines.insert(lines.begin(), "Commerce local : " + building.name + ".");
                lines.push_back("Future économie : stocks locaux, taxe, arrivages, réservation et négociation utiliseront cette identité régionale.");
                MessageScreen::show("COMMERCE LOCAL", "quest.city_hub.market", lines, false);
            }
            else if (building.id == "archives")
            {
                MessageScreen::show("ARCHIVES LOCALES", "quest.city_hub.archives", {
                    "Archives de " + city->getName() + ".",
                    "Rôle actuel : rappeler les rumeurs, biomes et connaissances régionales sans tout révéler gratuitement.",
                    "Future IG : cartes murales, livres, légendes, informations achetables et petites illustrations de lore."
                }, false);
            }
            else
            {
                MessageScreen::show("BÂTIMENT LOCAL", "quest.city_hub.building.info", {
                    building.name + " — " + building.category + ".",
                    "Contact : " + building.contact + ".",
                    building.detail,
                    "Indice pixel-art : " + building.pixelArtHint + "."
                }, false);
            }
        }
    }

    void openInnMenu(Player& player)
    {
        while (true)
        {
            const int commonBedCost = EconomyBalance::innCommonBedCost(player.getCurrentCityId(), player.getLevel());
            const int roomCost = EconomyBalance::innSafeRoomCost(player.getCurrentCityId(), player.getLevel());
            const int mealCost = EconomyBalance::innWarmMealCost(player.getCurrentCityId(), player.getLevel());
            MenuScreen screen("AUBERGE — " + currentCityName(player), "quest.city_hub.inn");
            screen.addLine("Repos réel : l'auberge existe pour éviter que les routes deviennent un lit gratuit.");
            screen.addLine("PV : " + std::to_string(player.getHp()) + "/" + std::to_string(player.getMaxHp()) + ".");
            screen.addLine("Argent : " + player.getInventory().getWalletLine() + ".");
            screen.addLine("Fatigue de route estimée : " + std::to_string(std::max(0, player.getCanonicalJournalCategoryTotal("distance_route") / 60 - player.getCanonicalJournalCategoryTotal("repos_auberge") * 2 - player.getCanonicalJournalCategoryTotal("repas_auberge"))) + " cran(s) narratif(s).");
            screen.addBackOption("Retour", "quest.city_hub.inn.back");
            screen.addOption(1, "Lit commun", "Peu cher, avance jusqu'au lendemain et soigne correctement. Coût : " + Money::formatCopper(commonBedCost) + ".", true, "quest.city_hub.inn.common_bed");
            screen.addOption(2, "Chambre sûre", "Plus chère, meilleure sécurité, soin complet et registre propre. Coût : " + Money::formatCopper(roomCost) + ".", true, "quest.city_hub.inn.room");
            screen.addOption(3, "Repas chaud", "Petit soin et baisse narrative de fatigue sans dormir. Coût : " + Money::formatCopper(mealCost) + ".", true, "quest.city_hub.inn.meal");
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
                player.heal(std::max(1, player.getMaxHp() * 70 / 100));
                player.recordCanonicalEvent("repos_auberge", player.getCurrentCityId(), "Lit commun à " + currentCityName(player));
                if (stableMissionRoll("inn_common:" + player.getCurrentCityId() + ":" + std::to_string(player.getWorldDaysElapsed())) < 9)
                {
                    player.recordCanonicalEvent("evenements_nocturnes_auberge", player.getCurrentCityId(), "Bruit, voisin bizarre ou rumeur pendant la nuit d'auberge");
                }
                recordRecentAction(player, "inn_common_bed", "Repos en lit commun à " + currentCityName(player));
                MessageScreen::show("REPOS À L'AUBERGE", "quest.city_hub.inn.common_bed.done", {"Tu dors dans un lit commun. Ce n'est pas luxueux, mais c'est légal et plus sûr qu'une route de nuit.", "PV actuels : " + std::to_string(player.getHp()) + "/" + std::to_string(player.getMaxHp()) + ".", player.formatWorldDateTimeLine()}, false);
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
                player.heal(player.getMaxHp());
                player.recordCanonicalEvent("repos_auberge", player.getCurrentCityId(), "Chambre sûre à " + currentCityName(player));
                player.recordCanonicalEvent("nuits_securisees", player.getCurrentCityId(), "Chambre sûre à " + currentCityName(player));
                player.recordCanonicalEvent("fatigue_route_reduite", player.getCurrentCityId(), "Chambre sûre : fatigue de route calmée");
                recordRecentAction(player, "inn_safe_room", "Chambre sûre à " + currentCityName(player));
                MessageScreen::show("CHAMBRE SÛRE", "quest.city_hub.inn.room.done", {"Tu prends une vraie chambre. Les portes ferment, le lit tient debout, et personne ne fouille ton sac dans le couloir.", "PV entièrement restaurés.", player.formatWorldDateTimeLine()}, false);
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
                MessageScreen::show("REPAS CHAUD", "quest.city_hub.inn.meal.done", {"Tu prends un repas chaud. Ce n'est pas un sommeil complet, mais ça évite de traiter la fatigue comme une ligne invisible.", "PV actuels : " + std::to_string(player.getHp()) + "/" + std::to_string(player.getMaxHp()) + ".", player.formatWorldDateTimeLine()}, false);
                continue;
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

            screen.addOption(89, "Voir la ville actuelle", "Bâtiments locaux, services, accès et préparation de la future image cliquable.", true, "quest.city_travel.city_hub");
            screen.addOption(90, "Voir la carte d'exploration prévue", "Distances vers biomes, zones grisées/enfumées et plan visuel futur.", true, "quest.city_travel.map_preview");
            screen.addOption(91, "Voir les règles visuelles futures", "Ville pixel-art, bâtiments cliquables, porte des remparts, arène et backgrounds.", true, "quest.city_travel.visual_future");
            screen.addOption(92, "Voir le registre moteur", "Top 3 basé sur des événements moteur enregistrés, pas sur du texte deviné.", true, "quest.city_travel.canonical_journal");
            screen.addOption(93, "Voir les dernières actions", "Historique court utile pour comprendre les erreurs et préparer helpmerefundmyaction.", true, "quest.city_travel.recent_actions");
            screen.addOption(94, "Bureau des missions déléguées", "Payer des aventuriers/PNJ pour une mission avec coût, jours et taux de réussite.", true, "quest.city_travel.delegated_missions");
            screen.addOption(95, "Carte schématique", "Voir la carte simple actuelle avant la vraie carte pixel-art cliquable.", true, "quest.city_travel.schematic_map");
            screen.addOption(96, "Registre avancé / incidents", "Illégal, sanctions, ramasse-miettes, aides rares et debug lore. Caché du Top 3 principal.", true, "quest.city_travel.canonical_journal_advanced");
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
            if (choice == 91)
            {
                MessageScreen::show("VILLE ET EXPLORATION — FUTUR PIXEL-ART", "quest.city_travel.visual_future", CityTravelRules::buildFutureVisualPlanningLines(), false);
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
                openDelegatedMissionBoard(player);
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
                "Future animation : route entre villes, puis arrivée devant les portes/remparts de la ville."
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
        cityScreen.addLine("Transport encadré : seules les piles de matériaux sont gérées pour l'instant, avec coût et coffre de destination requis.");
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
            const int cost = EconomyBalance::cityVaultMaterialTransferCost(player.getCurrentCityId(), destination->getId(), distance, material.getQuantity());
            labels.push_back(material.getName() + " x" + std::to_string(material.getQuantity()) + " [" + material.getQualityLabel() + "] — coût " + Money::formatCopper(cost));
        }
        const int materialIndex = askVaultEntryChoice("CHOISIR UNE PILE À TRANSPORTER", "quest.city_vault.transfer.material", labels);
        if (materialIndex < 0) return;
        const Material selected = player.getCityVault().getMaterial(materialIndex);
        const int distance = CityTravelRules::getDistanceBetweenCities(player.getCurrentCityId(), destination->getId());
        const int cost = EconomyBalance::cityVaultMaterialTransferCost(player.getCurrentCityId(), destination->getId(), distance, selected.getQuantity());

        MenuScreen confirm("CONFIRMER LE TRANSPORT", "quest.city_vault.transfer.confirm");
        confirm.addLine("Pile : " + selected.getName() + " x" + std::to_string(selected.getQuantity()) + ".");
        confirm.addLine("Destination : " + destination->getName() + ".");
        confirm.addLine("Coût : " + Money::formatCopper(cost) + ".");
        confirm.addLine("Règle : transport de matériaux seulement pour cette première version ; pas de retrait distant.");
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
            screen.addLine("Argent transporté : " + std::to_string(player.getInventory().getGold()) + " pièces.");
            screen.addBackOption("Retour aux lieux", "quest.city_vault.back");

            if (!player.hasCityVault())
            {
                screen.addLine("Statut : aucun coffre acheté.");
                screen.addLine("Premier coffre : 12 emplacements.");
                screen.addOption(1, "Acheter le coffre personnel", "Coût : " + std::to_string(player.getCityVaultPurchaseCost()) + " pièces.", true, "quest.city_vault.purchase");
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
                        MessageScreen::show("ACHAT IMPOSSIBLE", "quest.city_vault.purchase.failed", {"Or insuffisant ou coffre déjà possédé.", "Aucune pièce n'a été dépensée."}, false);
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
                ? "Coût : " + std::to_string(player.getCityVaultUpgradeCost()) + " pièces. Ajoute 8 emplacements."
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

    bool isStoryReferentClientName(const std::string& clientName)
    {
        return clientName == "Mira"
            || clientName == "Orren"
            || clientName == "Lysa"
            || clientName == "Bram"
            || clientName == "Soryn";
    }

    bool hasStoryReferentReferral(const Player& player, const std::string& clientName)
    {
        return isStoryReferentClientName(clientName)
            && player.hasStoryModeStarted()
            && player.getStoryChapter() == 1
            && player.getStoryStep() >= 3;
    }

    std::string storyReferentProfession(const std::string& clientName)
    {
        if (clientName == "Mira") return "intendante de quartier";
        if (clientName == "Orren") return "vieux garde / référent de route";
        if (clientName == "Lysa") return "soigneuse de fortune";
        if (clientName == "Bram") return "forgeron fatigué";
        if (clientName == "Soryn") return "archiviste";
        if (clientName == "Eda") return "comptable des routes courtes";
        return "référent de ville";
    }

    std::string storyReferentRoleLine(const std::string& clientName)
    {
        if (clientName == "Mira") return "Mira garde les priorités de quartier : murs, réserves, urgences et validation de la suite.";
        if (clientName == "Orren") return "Orren surveille les routes, les ponts, les bornes déplacées et les disparitions hors des murs.";
        if (clientName == "Lysa") return "Lysa soigne ce qu'elle peut, prépare les premiers remèdes et repère les symptômes qui ne collent pas aux blessures normales.";
        if (clientName == "Bram") return "Bram maintient la forge debout : outils, réparations, plaques de porte et métal récupérable.";
        if (clientName == "Soryn") return "Soryn conserve les archives, trie les rumeurs et refuse qu'une légende remplace une preuve.";
        if (clientName == "Eda") return "Eda vérifie les retours réels, les stocks confirmés et les temps de réparation quand les cartes mentent.";
        return clientName + " aide la ville à tenir.";
    }

    std::string storyAskHelpQuestId(const std::string& clientName)
    {
        if (clientName == "Orren") return "story_ch1_ask_help_orren";
        if (clientName == "Lysa") return "story_ch1_ask_help_lysa";
        if (clientName == "Bram") return "story_ch1_ask_help_bram";
        if (clientName == "Soryn") return "story_ch1_ask_help_soryn";
        return "";
    }

    std::string storyMainQuestIdForClient(const std::string& clientName)
    {
        if (clientName == "Mira") return "story_ch1_mira_main";
        if (clientName == "Orren") return "story_ch1_orren_main";
        if (clientName == "Lysa") return "story_ch1_lysa_main";
        if (clientName == "Bram") return "story_ch1_bram_main";
        if (clientName == "Soryn") return "story_ch1_soryn_main";
        return "";
    }

    bool isMainStoryQuest(const Quest& quest)
    {
        // Les anciennes quêtes techniques « demander de l'aide » servent uniquement
        // à migrer les sauvegardes des versions précédentes. Elles ne doivent pas
        // apparaître comme de vraies quêtes principales dans les archives.
        if (quest.id.rfind("story_ch1_ask_help_", 0) == 0)
        {
            return false;
        }

        return quest.origin == "Quête principale"
            || quest.id.rfind("story_ch", 0) == 0;
    }

    bool questExistsInAnyState(const Player& player, const std::string& questId)
    {
        if (questId.empty())
        {
            return false;
        }

        for (const Quest& quest : player.getQuestLog().getQuests())
        {
            if (quest.id == questId)
            {
                return true;
            }
        }
        return false;
    }

    bool questIsActiveInLog(const Player& player, const std::string& questId)
    {
        if (questId.empty())
        {
            return false;
        }

        for (const Quest& quest : player.getQuestLog().getQuests())
        {
            if (quest.id == questId && quest.accepted && !quest.completed && !quest.turnedIn && !quest.failed)
            {
                return true;
            }
        }
        return false;
    }

    bool questIsCompletedInLog(const Player& player, const std::string& questId)
    {
        if (questId.empty())
        {
            return false;
        }

        for (const Quest& quest : player.getQuestLog().getQuests())
        {
            if (quest.id == questId && quest.completed && !quest.turnedIn && !quest.failed)
            {
                return true;
            }
        }
        return false;
    }

    bool questIsTurnedInInLog(const Player& player, const std::string& questId)
    {
        if (questId.empty())
        {
            return false;
        }

        for (const Quest& quest : player.getQuestLog().getQuests())
        {
            if (quest.id == questId && quest.turnedIn && !quest.failed)
            {
                return true;
            }
        }
        return false;
    }

    std::vector<std::string> splitQuestStageLabels(const std::string& value)
    {
        std::vector<std::string> labels;
        std::stringstream stream(value);
        std::string label;
        while (std::getline(stream, label, '|'))
        {
            label.erase(label.begin(), std::find_if(label.begin(), label.end(), [](unsigned char c) { return !std::isspace(c); }));
            label.erase(std::find_if(label.rbegin(), label.rend(), [](unsigned char c) { return !std::isspace(c); }).base(), label.end());
            if (!label.empty()) labels.push_back(label);
        }
        return labels;
    }

    std::string storyQuestStatusForId(const Player& player, const std::string& questId)
    {
        if (questId.empty())
        {
            return "non concerné";
        }

        for (const Quest& quest : player.getQuestLog().getQuests())
        {
            if (quest.id != questId)
            {
                continue;
            }

            if (quest.failed) return "bloquée / à reprendre";
            if (quest.turnedIn) return "validée";
            if (quest.completed) return "prête à notifier";
            if (quest.accepted) return "en cours " + std::to_string(quest.progress) + "/" + std::to_string(quest.target);
            return "connue";
        }

        return "à débloquer";
    }


    std::string storyQuestMarkerForId(const Player& player, const std::string& questId)
    {
        if (questId.empty())
        {
            return "[verrouillé]";
        }

        for (const Quest& quest : player.getQuestLog().getQuests())
        {
            if (quest.id != questId)
            {
                continue;
            }

            if (quest.failed) return "[à reprendre]";
            if (quest.turnedIn) return "[fait]";
            if (quest.completed) return "[fait - à notifier]";
            if (quest.accepted) return "[en cours]";
            return "[connue]";
        }

        return "[verrouillé]";
    }

    std::string storyMilestoneMarker(bool done, bool current)
    {
        if (done) return "[fait]";
        if (current) return "[étape actuelle]";
        return "[verrouillé]";
    }

    struct StoryStepDescriptor
    {
        int number = 0;
        std::string id;
        std::string client;
        std::string title;
        std::string guidance;
    };

    std::vector<StoryStepDescriptor> chapterTwoStoryStepDescriptors()
    {
        return {
            {1, "story_ch2_relay_briefing", "Mira", "Le nom du relais silencieux", "Briefing avec Mira et Orren."},
            {2, "story_ch2_north_road_scout", "Orren", "La route qui s'allonge", "Explorer la Route commerciale, puis rendre le rapport à Orren."},
            {3, "story_ch2_turned_marker", "Soryn", "La borne retournée", "Obtenir une preuve assez nette pour Soryn."},
            {4, "story_ch2_relay_threat", "Orren", "Les guetteurs sans feu", "Affronter la menace du relais, puis rendre le rapport à Orren."},
            {5, "story_ch2_relay_signal", "Mira", "Le relais doit répondre", "Faire répondre le relais, puis notifier Mira."},
            {6, "story_ch2_first_rescue", "Nell", "La voix derrière les caisses", "Sauver Nell la messagère, puis lui rendre le rapport."},
            {7, "story_ch2_route_sack", "Nell", "La sacoche qui parle", "Analyser la sacoche de Nell."},
            {8, "story_ch2_city_recovery", "Mira", "Les comptoirs rouvrent un œil", "Distribuer les informations utiles aux comptoirs."},
            {9, "story_ch2_cold_ink_trail", "Soryn", "L'encre froide de la route", "Suivre la trace d'encre froide."},
            {10, "story_ch2_route_rewrite", "Soryn", "La carte qui se réécrit", "Identifier le procédé de réécriture."},
            {11, "story_ch2_short_route_counter", "Mira", "Le contre-registre des routes courtes", "Installer une vérification fiable des stocks."},
            {12, "story_ch2_black_knot_warning", "Orren", "Le nœud noir au bout du relais", "Reconnaître le nœud noir sans envoyer un convoi."},
            {13, "story_ch2_repair_downtime", "Eda", "Tenir pendant les travaux", "Aider utilement pendant les réparations."},
            {14, "story_ch2_hidden_guardian_hint", "Soryn", "La chose qui garde la borne", "Identifier la présence sans dévoiler son vrai nom."},
            {15, "story_ch2_black_knot_seal", "Orren", "Le verrou de la borne", "Briser le verrou de la borne noire."},
            {16, "story_ch2_black_knot_scars", "Soryn", "Les cicatrices du verrou", "Lire les marques laissées par le verrou."},
            {17, "story_ch2_guarded_route", "Mira", "Une route à garder ouverte", "Organiser les premiers retours gardés."}
        };
    }

    std::vector<StoryStepDescriptor> chapterThreeStoryStepDescriptors()
    {
        return {
            {1, "story_ch3_lonely_convoy", "Mira", "Le convoi qui revient seul", "Inspecter le convoi sans le déplacer."},
            {2, "story_ch3_three_routes", "Orren", "Trois routes pour une même borne", "Comparer la même borne à trois moments de la journée."},
            {3, "story_ch3_signatures", "Soryn", "Les signatures sans voyageurs", "Faire identifier les sceaux par les personnes capables de les reconnaître."},
            {4, "story_ch3_escort_withdrawal", "Orren", "Une escorte qui sait renoncer", "Protéger un petit convoi et sécuriser un demi-tour lorsque la route change."},
            {5, "story_ch3_margin_village", "Nell", "Le village écrit dans la marge", "Découvrir la première preuve du village absent des cartes."},
            {6, "story_ch3_corrected_route", "Mira", "La route corrigée", "Choisir la version de trajet conservée dans le contre-registre."},
            {7, "story_ch3_map_guardian", "Soryn", "Le Gardien de la Carte Juste", "Affronter le mini-boss unique qui protège la cohérence de la carte."},
            {8, "story_ch3_convoy_return", "Mira", "Ce que le convoi a rapporté", "Décider ce qui peut entrer en ville et fermer le chapitre."}
        };
    }

    void addGuidedStoryLine(
        MenuScreen& screen,
        int number,
        const std::string& title,
        const std::string& marker,
        const std::string& detail
    )
    {
        screen.addLine(std::to_string(number) + ". " + title + " " + marker + " — " + detail);
    }

    void addQuestGuidedStoryLine(MenuScreen& screen, const Player& player, const StoryStepDescriptor& step)
    {
        addGuidedStoryLine(
            screen,
            step.number,
            step.client + " — " + step.title,
            storyQuestMarkerForId(player, step.id),
            storyQuestStatusForId(player, step.id) + ". " + step.guidance
        );
    }

    const std::vector<std::string>& chapterOneReferentNames()
    {
        static const std::vector<std::string> names = {"Orren", "Lysa", "Bram", "Soryn"};
        return names;
    }

    int countKnownChapterOneReferentQuests(const Player& player)
    {
        int count = 0;
        for (const std::string& clientName : chapterOneReferentNames())
        {
            const std::string mainQuestId = storyMainQuestIdForClient(clientName);
            const std::string legacyAskQuestId = storyAskHelpQuestId(clientName);
            if (questExistsInAnyState(player, mainQuestId) || questIsTurnedInInLog(player, legacyAskQuestId))
            {
                ++count;
            }
        }
        return count;
    }

    int countTurnedInChapterOneReferentQuests(const Player& player)
    {
        int count = 0;
        for (const std::string& clientName : chapterOneReferentNames())
        {
            if (questIsTurnedInInLog(player, storyMainQuestIdForClient(clientName)))
            {
                ++count;
            }
        }
        return count;
    }


    void syncChapterOneLinkedQuestProgress(Player& player)
    {
        // Migration douce : une ancienne sauvegarde peut avoir validé la petite
        // quête technique de dialogue sans encore posséder la vraie demande du PNJ.
        for (const std::string& clientName : chapterOneReferentNames())
        {
            const std::string legacyAskQuestId = storyAskHelpQuestId(clientName);
            const std::string mainQuestId = storyMainQuestIdForClient(clientName);
            if (questIsTurnedInInLog(player, legacyAskQuestId) && !questExistsInAnyState(player, mainQuestId))
            {
                addNonRefusableQuestIfMissing(player, createChapterOneReferentMainQuest(clientName));
            }
        }

        const std::string referentIds = "story_ch1_orren_main|story_ch1_lysa_main|story_ch1_bram_main|story_ch1_soryn_main";

        for (Quest& quest : player.getQuestLog().getQuests())
        {
            if (quest.id == "story_ch1_meet_referents")
            {
                quest.title = "Rencontrer les quatre référents";
                quest.location = "Ville — quatre référents";
                quest.objective = "Parler séparément à Orren, Lysa, Bram et Soryn. Chacun confie immédiatement sa propre quête principale, réalisable dans n'importe quel ordre.";
                quest.objectiveType = "dialogue";
                quest.targetFamily = "Orren, Lysa, Bram et Soryn";
                quest.linkedQuestIds = referentIds;
                quest.stageLabels = "Orren rencontré|Lysa rencontrée|Bram rencontré|Soryn rencontré";
                quest.linkedQuestRequiredState = "known";
                quest.retroactiveProgress = true;
                quest.hideFutureSteps = true;
                quest.target = 4;
            }
            else if (quest.id == "story_ch1_mira_main")
            {
                quest.title = "Faire respirer les murs";
                quest.location = "Ville et alentours";
                quest.objective = "Terminer puis rendre les quatre quêtes principales données par Orren, Lysa, Bram et Soryn. Les quêtes déjà validées avant ce bilan sont comptées automatiquement.";
                quest.objectiveType = "story_bundle";
                quest.targetFamily = "Quêtes principales des quatre référents";
                quest.linkedQuestIds = referentIds;
                quest.stageLabels = "Quête d'Orren rendue|Quête de Lysa rendue|Quête de Bram rendue|Quête de Soryn rendue";
                quest.linkedQuestRequiredState = "turned_in";
                quest.retroactiveProgress = true;
                quest.hideFutureSteps = true;
                quest.target = 4;
            }
        }
        player.getQuestLog().refreshLinkedQuestProgress();
    }

    std::vector<std::string> chapterOneReferentStatusLines(const Player& player)
    {
        std::vector<std::string> lines;
        for (const std::string& clientName : chapterOneReferentNames())
        {
            const std::string questId = storyMainQuestIdForClient(clientName);
            if (!questExistsInAnyState(player, questId))
            {
                lines.push_back("[à rencontrer] " + clientName + " — parle-lui dans PNJ notables > PNJ d'histoire.");
                continue;
            }

            lines.push_back(storyQuestMarkerForId(player, questId) + " " + clientName + " — " + storyQuestStatusForId(player, questId) + ".");
        }
        return lines;
    }

    std::vector<std::string> questStepProgressLines(const Quest& quest)
    {
        std::vector<std::string> lines;
        const int target = std::max(1, quest.target);
        const int progress = std::max(0, std::min(quest.progress, target));
        const bool staged = quest.hideFutureSteps || isMainStoryQuest(quest) || quest.retroactiveProgress;

        if (target <= 1)
        {
            if (quest.turnedIn || quest.completed || progress >= target)
            {
                lines.push_back("Étape : [fait] objectif terminé.");
            }
            else
            {
                lines.push_back("Étape actuelle : 1/1 — " + questProgressMethodText(quest) + ".");
            }
            return lines;
        }

        if (!staged)
        {
            lines.push_back("Progression : " + std::to_string(progress) + "/" + std::to_string(target) + ".");
            if (quest.completed && !quest.turnedIn) lines.push_back("Rendu : [fait - à notifier] retourne voir le bon contact.");
            if (quest.turnedIn) lines.push_back("Rendu : [fait] demande validée.");
            return lines;
        }

        const std::vector<std::string> stageLabels = splitQuestStageLabels(quest.stageLabels);
        const auto stageLabel = [&](int zeroBasedIndex) {
            if (zeroBasedIndex >= 0 && zeroBasedIndex < static_cast<int>(stageLabels.size()))
            {
                return stageLabels[zeroBasedIndex];
            }
            return std::string("Objectif ") + std::to_string(zeroBasedIndex + 1);
        };

        for (int index = 0; index < progress; ++index)
        {
            lines.push_back("Étape " + std::to_string(index + 1) + "/" + std::to_string(target) + " : [fait] " + stageLabel(index) + ".");
        }

        if (!(quest.turnedIn || quest.completed) && progress < target)
        {
            lines.push_back("Étape actuelle " + std::to_string(progress + 1) + "/" + std::to_string(target) + " : " + stageLabel(progress) + " — " + questProgressMethodText(quest) + ".");
            if (progress + 1 < target)
            {
                lines.push_back("Étapes suivantes : masquées jusqu'à validation de l'étape actuelle.");
            }
        }
        else if (quest.completed && !quest.turnedIn)
        {
            lines.push_back("Rendu : [fait - à notifier] retourne voir le bon contact.");
        }
        else if (quest.turnedIn)
        {
            lines.push_back("Rendu : [fait] demande validée.");
        }

        if (quest.retroactiveProgress)
        {
            lines.push_back("Suivi : les prérequis déjà accomplis sont reconnus automatiquement.");
        }
        return lines;
    }

    Quest buildChapterOneStoryQuest(
        const std::string& id,
        const std::string& title,
        const std::string& client,
        const std::string& location,
        const std::string& objective,
        const std::string& objectiveType,
        const std::string& targetFamily,
        int target,
        int rewardExperience,
        int rewardGold
    )
    {
        Quest quest;
        quest.id = id;
        quest.rank = "Histoire";
        quest.title = title;
        quest.origin = "Quête principale";
        quest.client = client;
        quest.location = location;
        quest.objective = objective;
        quest.objectiveType = objectiveType;
        quest.targetFamily = targetFamily;
        quest.rewardExperience = rewardExperience;
        quest.rewardGold = rewardGold;
        quest.progress = 0;
        quest.target = std::max(1, target);
        quest.guildQuest = false;
        quest.availableFromDay = 0;
        quest.expiresAtDay = -1;
        quest.accepted = true;
        quest.completed = false;
        quest.turnedIn = false;
        quest.failed = false;
        quest.rewardNote = "Quête principale : non refusable.";
        quest.hideFutureSteps = target > 1;
        return quest;
    }

    void prepareNonRefusableStoryQuest(Quest& quest, int currentDay)
    {
        quest.origin = "Quête principale";
        quest.accepted = true;
        quest.failed = false;
        quest.failureReason.clear();
        quest.availableFromDay = std::max(0, currentDay);
        quest.expiresAtDay = -1;
    }

    Quest createChapterOneMeetReferentsQuest()
    {
        Quest quest = buildChapterOneStoryQuest(
            "story_ch1_meet_referents",
            "Rencontrer les quatre référents",
            "Mira",
            "Ville — quatre référents",
            "Parler séparément à Orren, Lysa, Bram et Soryn. Chacun confie immédiatement sa propre quête principale, réalisable dans n'importe quel ordre, puis il faut revenir prévenir Mira.",
            "dialogue",
            "Orren, Lysa, Bram et Soryn",
            4,
            0,
            0
        );
        quest.linkedQuestIds = "story_ch1_orren_main|story_ch1_lysa_main|story_ch1_bram_main|story_ch1_soryn_main";
        quest.stageLabels = "Orren rencontré|Lysa rencontrée|Bram rencontré|Soryn rencontré";
        quest.linkedQuestRequiredState = "known";
        quest.retroactiveProgress = true;
        return quest;
    }

    Quest createChapterOneMiraMainQuest()
    {
        Quest quest = buildChapterOneStoryQuest(
            "story_ch1_mira_main",
            "Faire respirer les murs",
            "Mira",
            "Ville et alentours",
            "Terminer puis rendre les quatre quêtes principales données par Orren, Lysa, Bram et Soryn. Les quêtes déjà validées avant ce bilan sont comptées automatiquement.",
            "story_bundle",
            "Quêtes principales des quatre référents",
            4,
            65,
            22
        );
        quest.linkedQuestIds = "story_ch1_orren_main|story_ch1_lysa_main|story_ch1_bram_main|story_ch1_soryn_main";
        quest.stageLabels = "Quête d'Orren rendue|Quête de Lysa rendue|Quête de Bram rendue|Quête de Soryn rendue";
        quest.linkedQuestRequiredState = "turned_in";
        quest.retroactiveProgress = true;
        return quest;
    }

    Quest createChapterOneReferentMainQuest(const std::string& clientName)
    {
        if (clientName == "Orren")
        {
            return buildChapterOneStoryQuest(
                "story_ch1_orren_main",
                "Les bornes qui mentent",
                "Orren",
                "Route commerciale",
                "Explorer la Route commerciale et noter les repères retournés, traces de passage ou signes d'embuscade près des premiers ponts.",
                "exploration",
                "Route commerciale",
                2,
                48,
                18
            );
        }

        if (clientName == "Lysa")
        {
            Quest quest = buildChapterOneStoryQuest(
                "story_ch1_lysa_main",
                "Les blessés de la nuit",
                "Lysa",
                "Infirmerie de Lysa",
                "Rapporter des feuilles amères de soin pour préparer les premiers remèdes et vérifier si les blessures venues des portes réagissent normalement. Sources claires : herboriste, achats de plantes, événements de ville ou exploration végétale.",
                "livraison",
                "Plantes médicinales",
                1,
                36,
                10
            );
            quest.requiredMaterialId = "bitter_healing_leaf";
            quest.requiredMaterialName = "Feuille amère de soin";
            quest.requiredMaterialQuantity = 2;
            return quest;
        }

        if (clientName == "Bram")
        {
            return buildChapterOneStoryQuest(
                "story_ch1_bram_main",
                "Les plaques qui tiennent encore",
                "Bram",
                "Forge de Bram",
                "Aider Bram à trier les plaques, sangles et outils récupérés afin de savoir ce qui peut vraiment renforcer les portes sans casser au premier choc.",
                "service",
                "Forge et réparations",
                2,
                32,
                8
            );
        }

        return buildChapterOneStoryQuest(
            "story_ch1_soryn_main",
            "Une rumeur à clouer au sol",
            "Soryn",
            "Plaine sauvage",
            "Vérifier sur le terrain une rumeur de monstre poussé vers les murs, afin que Soryn classe une preuve au lieu d'une panique.",
            "bestiaire",
            "Plaine sauvage",
            1,
            44,
            10
        );
    }

    Quest createChapterTwoBriefingQuest()
    {
        return buildChapterOneStoryQuest(
            "story_ch2_relay_briefing",
            "Le nom du relais silencieux",
            "Mira",
            "Registre de Mira",
            "Écouter Mira et Orren recouper les noms, les dates et les bornes avant de sortir sur la route du nord.",
            "dialogue",
            "Relais silencieux",
            1,
            0,
            0
        );
    }

    Quest createChapterTwoNorthRoadQuest()
    {
        Quest quest = buildChapterOneStoryQuest(
            "story_ch2_north_road_scout",
            "La route qui s'allonge",
            "Orren",
            "Route commerciale",
            "Explorer la Route commerciale depuis la sortie nord et confirmer si les distances, bornes ou traces changent réellement.",
            "exploration",
            "Route commerciale",
            2,
            58,
            20
        );
        quest.stageLabels = "Relever les bornes de la route nord|Comparer les ornières et le retour vers le relais";
        return quest;
    }

    Quest createChapterTwoTurnedMarkerQuest()
    {
        return buildChapterOneStoryQuest(
            "story_ch2_turned_marker",
            "La borne retournée",
            "Soryn",
            "Route commerciale",
            "Vérifier une borne retournée ou une preuve de terrain assez nette pour que Soryn classe l'affaire autrement qu'en simple rumeur.",
            "bestiaire",
            "Route commerciale",
            1,
            52,
            14
        );
    }

    Quest createChapterTwoRelayThreatQuest()
    {
        return buildChapterOneStoryQuest(
            "story_ch2_relay_threat",
            "Les guetteurs sans feu",
            "Orren",
            "Route commerciale / Relais silencieux",
            "Affronter la patrouille qui garde la route sans torches ni voix, puis rapporter à Orren que le relais n'est pas simplement abandonné.",
            "combat",
            "Humanoïdes / embuscades",
            1,
            84,
            28
        );
    }

    Quest createChapterTwoRelaySignalQuest()
    {
        return buildChapterOneStoryQuest(
            "story_ch2_relay_signal",
            "Le relais doit répondre",
            "Mira",
            "Relais silencieux",
            "Réactiver un signal simple du relais avec Orren, Bram et Soryn : cloche basse, marque visible et registre propre, afin que la route ne reste pas seulement débarrassée des guetteurs.",
            "service",
            "Relais / organisation",
            1,
            72,
            24
        );
    }

    Quest createChapterTwoFirstRescueQuest()
    {
        return buildChapterOneStoryQuest(
            "story_ch2_first_rescue",
            "La voix derrière les caisses",
            "Nell la messagère",
            "Route commerciale / convoi brisé",
            "Suivre le premier signal rendu possible par le relais, dégager le convoi brisé et ramener Nell la messagère avec sa sacoche de routes.",
            "sauvetage",
            "Route commerciale / survivants",
            1,
            92,
            26
        );
    }

    Quest createChapterTwoRouteSackQuest()
    {
        return buildChapterOneStoryQuest(
            "story_ch2_route_sack",
            "La sacoche qui parle",
            "Nell la messagère",
            "Registre du relais / table des cartes",
            "Exploiter la sacoche de routes de Nell : trier cartes froissées, bons de convoi, noms de haltes et marque d'encre froide pour choisir une prochaine piste fiable.",
            "enquête",
            "Cartes / relais / témoins",
            1,
            64,
            18
        );
    }

    Quest createChapterTwoCityRecoveryQuest()
    {
        return buildChapterOneStoryQuest(
            "story_ch2_city_recovery",
            "Les comptoirs rouvrent un œil",
            "Mira",
            "Quartier de départ / comptoirs",
            "Distribuer les informations de Nell aux comptoirs : herboristerie, forge, guilde et relais doivent savoir ce qui peut revenir sur les routes courtes.",
            "ville",
            "Stocks / confiance / économie",
            1,
            70,
            20
        );
    }

    Quest createChapterTwoColdInkTrailQuest()
    {
        Quest quest = buildChapterOneStoryQuest(
            "story_ch2_cold_ink_trail",
            "L'encre froide de la route",
            "Soryn",
            "Route commerciale / ancienne halte",
            "Suivre deux traces de Route commerciale liées à la sacoche de Nell : l'encre froide sur la carte et la halte rayée dans le registre.",
            "exploration scénarisée",
            "Route commerciale / encre froide",
            2,
            96,
            30
        );
        quest.stageLabels = "Examiner la halte rayée|Mesurer la boucle du pont court";
        return quest;
    }


    Quest createChapterTwoRouteRewriteQuest()
    {
        return buildChapterOneStoryQuest(
            "story_ch2_route_rewrite",
            "La carte qui se réécrit",
            "Soryn",
            "Archives de Soryn / table des cartes",
            "Comparer l'encre froide, la sacoche de Nell et les bons de convoi pour prouver que la route est réécrite après le passage des vivants.",
            "enquête",
            "Cartes / archives / encre froide",
            1,
            74,
            20
        );
    }

    Quest createChapterTwoShortRouteCounterQuest()
    {
        return buildChapterOneStoryQuest(
            "story_ch2_short_route_counter",
            "Le contre-registre des routes courtes",
            "Mira",
            "Comptoirs de ville / registre d'Eda",
            "Installer un contre-registre avec Mira, Nell, Eda, Bram et Lysa afin que les boutiques suivent les retours réels plutôt que les cartes corrompues.",
            "ville",
            "Économie / stocks / routes courtes",
            1,
            78,
            22
        );
    }

    Quest createChapterTwoBlackKnotWarningQuest()
    {
        return buildChapterOneStoryQuest(
            "story_ch2_black_knot_warning",
            "Le nœud noir au bout du relais",
            "Orren",
            "Route commerciale / ancienne borne noire",
            "Reconnaître le nœud noir où plusieurs routes corrigées semblent revenir, repousser la surveillance de l'approche et rapporter l'alerte à Orren.",
            "reconnaissance combat",
            "Route commerciale / prochaine crise",
            1,
            105,
            34
        );
    }

    Quest createChapterTwoRepairDowntimeQuest()
    {
        Quest quest = buildChapterOneStoryQuest(
            "story_ch2_repair_downtime",
            "Tenir pendant les travaux",
            "Eda",
            "Quartier de départ / comptoirs",
            "Pendant que Bram, Lysa, Mira, Orren, Nell et Eda mettent en place les réparations, s'occuper utilement : patrouilles, services de comptoir, quêtes secondaires ou exploration courte pour préparer la crise suivante.",
            "préparation / quêtes secondaires",
            "Ville / réparations / expérience",
            3,
            88,
            24
        );
        quest.stageLabels = "Premier service utile pendant les travaux|Deuxième contribution utile|Dernière contribution avant la reprise";
        return quest;
    }

    Quest createChapterTwoHiddenGuardianHintQuest()
    {
        return buildChapterOneStoryQuest(
            "story_ch2_hidden_guardian_hint",
            "La chose qui garde la borne",
            "Soryn",
            "Archives de Soryn / ancienne borne noire",
            "Recouper les témoignages, stocks évités et retours de route pour comprendre qu'une présence garde la borne noire, sans révéler encore son vrai nom ni toute sa nature.",
            "piste de menace / enquête",
            "Borne noire / menace connue par rumeur",
            1,
            72,
            18
        );
    }

    Quest createChapterTwoBlackKnotSealQuest()
    {
        return buildChapterOneStoryQuest(
            "story_ch2_black_knot_seal",
            "Le verrou de la borne",
            "Orren",
            "Ancienne borne noire / route du relais",
            "Forcer le premier verrou vivant de la borne noire à se montrer, briser sa garde et revenir avec une preuve assez nette pour que la ville cesse de traiter ce silence comme une simple rumeur.",
            "combat",
            "Borne noire / menace d'étape non nommée",
            1,
            128,
            38
        );
    }

    Quest createChapterTwoBlackKnotScarsQuest()
    {
        return buildChapterOneStoryQuest(
            "story_ch2_black_knot_scars",
            "Les cicatrices du verrou",
            "Soryn",
            "Archives de Soryn / borne noire",
            "Relire les marques laissées par le verrou de la borne noire avec Soryn, Nell et Orren, afin de classer une preuve sans inventer un nom trop tôt.",
            "enquête après affrontement",
            "Borne noire / preuves classées",
            1,
            86,
            24
        );
    }

    Quest createChapterTwoGuardedRouteQuest()
    {
        return buildChapterOneStoryQuest(
            "story_ch2_guarded_route",
            "Une route à garder ouverte",
            "Mira",
            "Comptoirs / route courte du relais",
            "Organiser les premiers retours gardés après le verrou : marques de métal, trousses de soin, messagers prudents et stocks confirmés par des survivants.",
            "ville / route gardée",
            "Stocks / relais / conséquences",
            1,
            94,
            26
        );
    }

    Quest createChapterThreeLonelyConvoyQuest()
    {
        return buildChapterOneStoryQuest(
            "story_ch3_lonely_convoy", "Le convoi qui revient seul", "Mira",
            "Relais silencieux / convoi revenu", "Inspecter les roues, le registre et les marchandises supplémentaires sans déplacer le convoi avant le relevé.",
            "enquête", "Convoi sans équipage", 1, 105, 30
        );
    }

    Quest createChapterThreeThreeRoutesQuest()
    {
        Quest quest = buildChapterOneStoryQuest(
            "story_ch3_three_routes", "Trois routes pour une même borne", "Orren",
            "Route commerciale", "Mesurer la même borne à l'aube, au milieu du jour puis la nuit afin de prouver que la distance dépend du moment.",
            "exploration à étapes", "Route commerciale / moments de la journée", 3, 128, 36
        );
        quest.stageLabels = "Mesure à l'aube|Mesure au milieu du jour|Mesure de nuit";
        return quest;
    }

    Quest createChapterThreeSignaturesQuest()
    {
        Quest quest = buildChapterOneStoryQuest(
            "story_ch3_signatures", "Les signatures sans voyageurs", "Soryn",
            "Archives / comptoirs / relais", "Faire identifier les sceaux impossibles par Soryn, Eda et Nell. Chaque avis déjà obtenu reste compté.",
            "enquête à étapes", "Sceaux / registres / témoins", 3, 118, 34
        );
        quest.stageLabels = "Analyse de Soryn|Vérification des poids par Eda|Reconnaissance des sceaux par Nell";
        return quest;
    }

    Quest createChapterThreeEscortWithdrawalQuest()
    {
        Quest quest = buildChapterOneStoryQuest(
            "story_ch3_escort_withdrawal", "Une escorte qui sait renoncer", "Orren",
            "Route commerciale / convoi court", "Escorter un petit convoi, puis accepter un demi-tour propre si la route change au lieu de sacrifier les voyageurs.",
            "escorte à étapes", "Convoi / retour sécurisé", 2, 142, 42
        );
        quest.stageLabels = "Escorte engagée|Demi-tour sécurisé";
        return quest;
    }

    Quest createChapterThreeMarginVillageQuest()
    {
        return buildChapterOneStoryQuest(
            "story_ch3_margin_village", "Le village écrit dans la marge", "Nell",
            "Relais / carte corrigée", "Isoler la première preuve lisible d'un village absent des cartes actuelles sans encore y envoyer de groupe.",
            "découverte", "Village absent des cartes", 1, 122, 35
        );
    }

    Quest createChapterThreeCorrectedRouteQuest()
    {
        return buildChapterOneStoryQuest(
            "story_ch3_corrected_route", "La route corrigée", "Mira",
            "Contre-registre de la ville", "Choisir quelle version du trajet conserver : commerce, secours ou preuve de recherche. Le choix modifie le récit sans bloquer la suite.",
            "choix narratif", "Contre-registre / conséquence durable", 1, 136, 40
        );
    }

    Quest createChapterThreeMapGuardianQuest()
    {
        return buildChapterOneStoryQuest(
            "story_ch3_map_guardian", "Le Gardien de la Carte Juste", "Soryn",
            "Route corrigée / borne de cohérence", "Affronter le Gardien de la Carte Juste, mini-boss unique qui défend la route considérée correcte plutôt que la route la plus sûre.",
            "combat mini-boss unique", "Mini-boss unique / cohérence de la carte", 1, 190, 58
        );
    }

    Quest createChapterThreeConvoyReturnQuest()
    {
        return buildChapterOneStoryQuest(
            "story_ch3_convoy_return", "Ce que le convoi a rapporté", "Mira",
            "Porte de ville / registre d'entrée", "Décider avec Mira ce qui peut entrer en ville : marchandises, preuves et avertissements liés au village absent des cartes.",
            "conclusion", "Ville / convoi / conséquences", 1, 160, 48
        );
    }

    int countTurnedInChapterThreeRequests(const Player& player)
    {
        int count = 0;
        for (const StoryStepDescriptor& step : chapterThreeStoryStepDescriptors())
        {
            if (questIsTurnedInInLog(player, step.id)) ++count;
        }
        return count;
    }

    int countTurnedInChapterTwoRequests(const Player& player)
    {
        int count = 0;
        const std::vector<std::string> ids = {
            "story_ch2_relay_briefing",
            "story_ch2_north_road_scout",
            "story_ch2_turned_marker",
            "story_ch2_relay_threat",
            "story_ch2_relay_signal",
            "story_ch2_first_rescue",
            "story_ch2_route_sack",
            "story_ch2_city_recovery",
            "story_ch2_cold_ink_trail",
            "story_ch2_route_rewrite",
            "story_ch2_short_route_counter",
            "story_ch2_black_knot_warning",
            "story_ch2_repair_downtime",
            "story_ch2_hidden_guardian_hint",
            "story_ch2_black_knot_seal",
            "story_ch2_black_knot_scars",
            "story_ch2_guarded_route"
        };

        for (const std::string& id : ids)
        {
            if (questIsTurnedInInLog(player, id))
            {
                ++count;
            }
        }
        return count;
    }

    bool addNonRefusableQuestIfMissing(Player& player, Quest quest)
    {
        if (quest.id.empty() || questExistsInAnyState(player, quest.id))
        {
            return false;
        }

        prepareNonRefusableStoryQuest(quest, player.getWorldDaysElapsed());
        return player.getQuestLog().addQuest(quest);
    }

    bool completeAndTurnInQuestSilently(Player& player, const std::string& questId)
    {
        bool updated = false;
        for (Quest& quest : player.getQuestLog().getQuests())
        {
            if (quest.id == questId && !quest.turnedIn && !quest.failed)
            {
                quest.accepted = true;
                quest.progress = std::max(quest.target, 1);
                quest.completed = true;
                quest.turnedIn = true;
                quest.expiresAtDay = -1;
                player.recordPnjServed(quest.client.empty() ? std::string("Contact d'histoire") : quest.client);
                player.recordQuestTypeCompleted(questKindText(quest));
                updated = true;
            }
        }
        return updated;
    }

    int countTurnedInChapterOneMainRequests(const Player& player)
    {
        int count = 0;
        const std::vector<std::string> ids = {
            "story_ch1_mira_main",
            "story_ch1_orren_main",
            "story_ch1_lysa_main",
            "story_ch1_bram_main",
            "story_ch1_soryn_main"
        };

        for (const std::string& id : ids)
        {
            if (questIsTurnedInInLog(player, id))
            {
                ++count;
            }
        }
        return count;
    }

    bool handleStoryReferentMainQuestDialogue(Player& player, const std::string& clientName)
    {
        if (!isStoryReferentClientName(clientName)
            || !player.hasStoryModeStarted()
            || player.getStoryChapter() != 1
            || player.getStoryStep() < 3)
        {
            return false;
        }

        QuestMenu::syncMainStoryQuests(player);
        syncChapterOneLinkedQuestProgress(player);

        if (clientName == "Mira")
        {
            const bool metQuestReady = questIsCompletedInLog(player, "story_ch1_meet_referents");
            const bool metQuestDone = questIsTurnedInInLog(player, "story_ch1_meet_referents");

            if (metQuestReady && !metQuestDone)
            {
                completeAndTurnInQuestSilently(player, "story_ch1_meet_referents");
                player.setStoryProgress(1, 4, std::max(1, player.getStoryCityDevelopmentLevel()));
                addNonRefusableQuestIfMissing(player, createChapterOneMiraMainQuest());
                syncChapterOneLinkedQuestProgress(player);

                const int alreadyDone = countTurnedInChapterOneReferentQuests(player);
                std::vector<std::string> lines = {
                    "Mira coche les quatre noms, puis ouvre une nouvelle page du registre.",
                    "Mira : « Maintenant je sais qui t'a parlé. Il reste à savoir ce que la ville peut réellement tenir grâce à toi. »",
                    "Nouvelle quête principale : Faire respirer les murs.",
                    "Objectif : terminer puis rendre les quatre quêtes principales des référents.",
                    "Progression reprise automatiquement : " + std::to_string(alreadyDone) + "/4 quête(s) déjà validée(s)."
                };
                const std::vector<std::string> statuses = chapterOneReferentStatusLines(player);
                lines.insert(lines.end(), statuses.begin(), statuses.end());
                MessageScreen::show("MIRA — BILAN DES QUATRE", "quest.story.mira.referents_notified", lines, false);
                return true;
            }

            if (!metQuestDone)
            {
                const int met = countKnownChapterOneReferentQuests(player);
                std::vector<std::string> lines = {
                    "Mira garde une ligne vide pour chacun des quatre référents.",
                    "Référents rencontrés : " + std::to_string(met) + "/4.",
                    "Parle à Orren, Lysa, Bram et Soryn dans n'importe quel ordre. Chacun te donnera sa propre quête principale."
                };
                const std::vector<std::string> statuses = chapterOneReferentStatusLines(player);
                lines.insert(lines.end(), statuses.begin(), statuses.end());
                MessageScreen::show("MIRA — LE TOUR N'EST PAS FINI", "quest.story.mira.referents_pending", lines, false);
                return true;
            }

            if (questExistsInAnyState(player, "story_ch1_mira_main"))
            {
                std::vector<std::string> lines = {
                    "Mira relit le bilan des quatre référents.",
                    "Quête principale : " + storyQuestStatusForId(player, "story_ch1_mira_main") + ".",
                    "Les quêtes déjà rendues sont comptées automatiquement, même si elles ont été terminées avant le retour auprès de Mira."
                };
                const std::vector<std::string> statuses = chapterOneReferentStatusLines(player);
                lines.insert(lines.end(), statuses.begin(), statuses.end());
                lines.push_back(questIsCompletedInLog(player, "story_ch1_mira_main")
                    ? "Le bilan est complet : utilise Rendre une demande terminée auprès de Mira."
                    : (questIsTurnedInInLog(player, "story_ch1_mira_main")
                        ? "[fait] Mira a validé le bilan. La conclusion du chapitre est prête."
                        : "Il reste au moins une quête de référent à terminer ou à rendre auprès de son propriétaire."));
                MessageScreen::show("MIRA — FAIRE RESPIRER LES MURS", "quest.story.mira.bundle_status", lines, false);
                return true;
            }

            return false;
        }

        const std::string mainQuestId = storyMainQuestIdForClient(clientName);
        const std::string legacyAskQuestId = storyAskHelpQuestId(clientName);

        if (!questExistsInAnyState(player, mainQuestId))
        {
            if (questExistsInAnyState(player, legacyAskQuestId))
            {
                completeAndTurnInQuestSilently(player, legacyAskQuestId);
            }

            const bool added = addNonRefusableQuestIfMissing(player, createChapterOneReferentMainQuest(clientName));
            syncChapterOneLinkedQuestProgress(player);
            const int met = countKnownChapterOneReferentQuests(player);

            std::vector<std::string> lines = {
                clientName + " (" + storyReferentProfession(clientName) + ") écoute quand tu dis que Mira t'envoie.",
                storyReferentRoleLine(clientName),
                clientName + " ajoute immédiatement sa propre quête principale au journal.",
                added ? "Quête principale ajoutée : " + storyQuestStatusForId(player, mainQuestId) : "La quête principale était déjà connue dans le journal.",
                "Référents rencontrés : " + std::to_string(met) + "/4."
            };
            if (met >= 4)
            {
                lines.push_back("[fait] Les quatre référents ont été rencontrés. Retourne prévenir Mira, même si certaines de leurs quêtes sont déjà terminées.");
            }
            else
            {
                lines.push_back("Les autres référents restent disponibles dans n'importe quel ordre.");
            }
            MessageScreen::show("DE LA PART DE MIRA", "quest.story.referent.main_added", lines, false);
            return true;
        }

        std::vector<std::string> lines = {
            clientName + " — " + storyReferentProfession(clientName) + ".",
            storyReferentRoleLine(clientName),
            "Demande principale : " + storyQuestStatusForId(player, mainQuestId) + ".",
            questIsCompletedInLog(player, mainQuestId)
                ? "Cette demande est prête : utilise l'option de rendu auprès de ce contact."
                : (questIsTurnedInInLog(player, mainQuestId)
                    ? "[fait] Cette demande est validée. Le bilan de Mira la compte automatiquement."
                    : "Cette demande reste dans la section Quête principale et dans le journal.")
        };
        if (clientName == "Lysa" && !questIsTurnedInInLog(player, mainQuestId) && !questIsCompletedInLog(player, mainQuestId))
        {
            lines.push_back("Indice de Lysa : les Feuilles amères de soin s'obtiennent surtout chez l'Herboriste, via les stocks de plantes, certains services de ville ou l'exploration végétale comme la Forêt ancienne.");
            lines.push_back("Elle accepte des feuilles normales ou de meilleure qualité : la livraison vérifie l'équivalent de quantité dans l'inventaire.");
        }
        MessageScreen::show("DEMANDE PRINCIPALE", "quest.story.referent.main_status", lines, false);
        return true;
    }

    std::string questStateText(const Quest& quest)
    {
        if (quest.failed)
        {
            return "Échouée / délai dépassé";
        }

        if (quest.turnedIn)
        {
            return "Validée";
        }

        if (quest.completed)
        {
            return "À rendre au client";
        }

        return "En cours";
    }

    int rankPowerForQuestReward(const std::string& rank)
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

    struct GuildStanding
    {
        std::string rank = "F";
        std::string pellet = "verte";
        int completedGuildContracts = 0;
        int completedSimpleGuildContracts = 0;
        int failedGuildContracts = 0;
        int failedPersonalRequests = 0;
        int rawFailureScore = 0;
        int rehabilitationCredits = 0;
        int effectiveFailureScore = 0;
        int maxAllowedRankPower = 2;
    };

    std::string guildRankFromProgress(int completedGuildContracts, int playerLevel)
    {
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
            if (completedGuildContracts >= threshold.requiredContracts && playerLevel >= threshold.requiredLevel)
            {
                return threshold.rank;
            }
        }

        return "F";
    }

    std::string guildReliabilityPelletFromFailureScore(int effectiveFailureScore)
    {
        if (effectiveFailureScore >= 7) return "rouge";
        if (effectiveFailureScore >= 4) return "orange";
        if (effectiveFailureScore >= 1) return "jaune";
        return "verte";
    }

    int guildRehabilitationCreditsFromContracts(int completedGuildContracts, int completedSimpleGuildContracts)
    {
        // FR: les petits contrats propres réhabilitent plus vite un dossier qu'un seul gros exploit isolé.
        // EN: small clean contracts rehabilitate a record faster than one isolated big win.
        return completedSimpleGuildContracts / 2 + std::max(0, completedGuildContracts - completedSimpleGuildContracts) / 4;
    }

    int guildMaxAllowedRankPower(const std::string& rank, const std::string& pellet)
    {
        int maxPower = rankPowerForQuestReward(rank) + 1;

        if (pellet == "orange")
        {
            maxPower = std::min(maxPower, rankPowerForQuestReward("B"));
        }
        else if (pellet == "rouge")
        {
            maxPower = std::min(maxPower, rankPowerForQuestReward("D"));
        }

        return std::max(1, maxPower);
    }

    GuildStanding guildStandingForPlayer(const Player& player)
    {
        GuildStanding standing;

        for (const Quest& quest : player.getQuestLog().getQuests())
        {
            if (quest.guildQuest)
            {
                if (quest.turnedIn)
                {
                    standing.completedGuildContracts++;
                    if (rankPowerForQuestReward(quest.rank) <= rankPowerForQuestReward("D"))
                    {
                        standing.completedSimpleGuildContracts++;
                    }
                }
                if (quest.failed)
                {
                    standing.failedGuildContracts++;
                }
            }
            else if (quest.failed)
            {
                standing.failedPersonalRequests++;
            }
        }

        if (!player.hasTitle("Aventurier"))
        {
            standing.rank = "Non inscrit";
            standing.pellet = "aucune";
            standing.maxAllowedRankPower = 0;
            return standing;
        }

        standing.rank = guildRankFromProgress(standing.completedGuildContracts, player.getLevel());
        standing.rawFailureScore = standing.failedGuildContracts * 2 + standing.failedPersonalRequests;
        standing.rehabilitationCredits = guildRehabilitationCreditsFromContracts(
            standing.completedGuildContracts,
            standing.completedSimpleGuildContracts
        );
        standing.effectiveFailureScore = std::max(0, standing.rawFailureScore - standing.rehabilitationCredits);
        standing.pellet = guildReliabilityPelletFromFailureScore(standing.effectiveFailureScore);
        standing.maxAllowedRankPower = guildMaxAllowedRankPower(standing.rank, standing.pellet);
        return standing;
    }

    std::string guildNoticeIdForRank(const std::string& rank)
    {
        if (rank == "E") return "guild_rank_e_notice";
        if (rank == "D") return "guild_rank_d_notice";
        if (rank == "C") return "guild_rank_c_notice";
        if (rank == "B") return "guild_rank_b_notice";
        if (rank == "A") return "guild_rank_a_notice";
        if (rank == "S") return "guild_rank_s_notice";
        if (rank == "SS") return "guild_rank_ss_notice";
        if (rank == "SSS") return "guild_rank_sss_notice";
        if (rank == "Héros mondial") return "guild_rank_world_hero_notice";
        if (rank == "Légende") return "guild_rank_legend_notice";
        if (rank == "Dieu") return "guild_rank_god_notice";
        return "guild_rank_f_notice";
    }

    std::string guildTitleForRank(const std::string& rank)
    {
        if (rank == "Non inscrit") return "";
        if (rank == "Héros mondial") return "Aventurier - Héros mondial";
        if (rank == "Légende") return "Aventurier - Légende";
        if (rank == "Dieu") return "Aventurier - Rang divin";
        return "Aventurier rang " + rank;
    }

    std::string guildPelletIdForStanding(const std::string& pellet)
    {
        if (pellet == "jaune") return "guild_reliability_yellow_pellet";
        if (pellet == "orange") return "guild_reliability_orange_pellet";
        if (pellet == "rouge") return "guild_reliability_red_pellet";
        return "guild_reliability_green_pellet";
    }

    bool isGuildQuestRankAllowedForStanding(const Quest& quest, const GuildStanding& standing)
    {
        if (!quest.guildQuest)
        {
            return true;
        }

        return rankPowerForQuestReward(quest.rank) <= standing.maxAllowedRankPower;
    }

    int guildActiveQuestLimitForStanding(const GuildStanding& standing)
    {
        if (rankPowerForQuestReward(standing.rank) >= rankPowerForQuestReward("A"))
        {
            return 5;
        }
        if (rankPowerForQuestReward(standing.rank) >= rankPowerForQuestReward("D"))
        {
            return 4;
        }
        return 3;
    }

    int guildBoardOfferBonusForStanding(const GuildStanding& standing)
    {
        if (rankPowerForQuestReward(standing.rank) >= rankPowerForQuestReward("A"))
        {
            return 4;
        }
        if (rankPowerForQuestReward(standing.rank) >= rankPowerForQuestReward("D"))
        {
            return 2;
        }
        return 0;
    }

    std::vector<std::string> guildStandingSummaryLines(const Player& player)
    {
        const GuildStanding standing = guildStandingForPlayer(player);

        if (!player.hasTitle("Aventurier"))
        {
            return {"Carte de guilde : non inscrite."};
        }

        std::vector<std::string> lines;
        lines.push_back("Carte de guilde : rang " + standing.rank + " / pastille " + standing.pellet + ".");
        lines.push_back("Contrats officiels validés : " + std::to_string(standing.completedGuildContracts)
            + " | échecs officiels : " + std::to_string(standing.failedGuildContracts)
            + " | demandes informelles échouées : " + std::to_string(standing.failedPersonalRequests) + ".");
        lines.push_back("Réhabilitation : " + std::to_string(standing.rehabilitationCredits)
            + " crédit(s) de fiabilité | score brut " + std::to_string(standing.rawFailureScore)
            + " -> score actif " + std::to_string(standing.effectiveFailureScore) + ".");
        if (standing.pellet != "verte")
        {
            lines.push_back("Pour améliorer la pastille : réussir plusieurs petits contrats officiels propres avant de reprendre trop haut.");
        }
        lines.push_back("Accès conseillé : contrats jusqu'à rang "
            + (standing.pellet == "rouge" ? std::string("D") : (standing.pellet == "orange" ? std::string("B") : std::string("un rang au-dessus du dossier")))
            + ".");
        return lines;
    }

    void applyGuildStandingRewards(Player& player, std::vector<std::string>& resultLines)
    {
        const GuildStanding standing = guildStandingForPlayer(player);
        if (!player.hasTitle("Aventurier"))
        {
            return;
        }

        const std::string title = guildTitleForRank(standing.rank);
        if (!title.empty() && player.grantTitle(title))
        {
            resultLines.push_back("Titre de guilde reconnu : " + title + ".");
        }

        if (standing.rank == "Dieu" && player.grantTitle("Le registre n'a plus de rang"))
        {
            resultLines.push_back("Titre secret révélé : Le registre n'a plus de rang.");
        }
        if (standing.completedGuildContracts >= 6 && player.grantTitle("Main fiable de la guilde"))
        {
            resultLines.push_back("Titre de guilde reconnu : Main fiable de la guilde.");
        }

        const std::string noticeId = guildNoticeIdForRank(standing.rank);
        if (!noticeId.empty() && player.getInventory().countMaterialById(noticeId) <= 0)
        {
            player.getInventory().addMaterial(MaterialCatalog::createById(noticeId, 1));
            resultLines.push_back("Nouvelle notice ajoutée à la carte magique : rang " + standing.rank + ".");
        }

        const std::string pelletId = guildPelletIdForStanding(standing.pellet);
        if (!pelletId.empty())
        {
            const std::vector<std::string> pelletIds = {
                "guild_reliability_green_pellet",
                "guild_reliability_yellow_pellet",
                "guild_reliability_orange_pellet",
                "guild_reliability_red_pellet"
            };
            for (const std::string& oldPelletId : pelletIds)
            {
                const int owned = player.getInventory().countMaterialById(oldPelletId);
                if (owned > 0)
                {
                    player.getInventory().removeMaterialQuantityById(oldPelletId, owned);
                }
            }
            player.getInventory().addMaterial(MaterialCatalog::createById(pelletId, 1));
            resultLines.push_back("Pastille de dossier mise à jour : " + standing.pellet + ".");
        }

        resultLines.push_back("Dossier de guilde : " + std::to_string(standing.completedGuildContracts)
            + " contrat(s) officiel(s) validé(s), rang actuel " + standing.rank
            + ", pastille " + standing.pellet + ".");
        resultLines.push_back("Réhabilitation : " + std::to_string(standing.rehabilitationCredits)
            + " crédit(s), score de sanction " + std::to_string(standing.rawFailureScore)
            + " -> " + std::to_string(standing.effectiveFailureScore) + ".");
    }

    int balancedQuestExperience(const Quest& quest)
    {
        if (quest.rewardExperience <= 0)
        {
            return 0;
        }

        const int power = rankPowerForQuestReward(quest.rank);
        const int target = std::max(1, quest.target);
        int balanced = quest.rewardExperience;

        if (quest.objectiveType == "service")
        {
            balanced = std::min(quest.rewardExperience, 3 + target + power * 2);
        }
        else if (quest.objectiveType == "livraison")
        {
            balanced = quest.rewardExperience * 50 / 100;
        }
        else if (quest.objectiveType == "bestiaire")
        {
            balanced = quest.rewardExperience * 48 / 100;
        }
        else if (quest.objectiveType == "exploration")
        {
            balanced = quest.rewardExperience * 60 / 100;
        }
        else if (quest.objectiveType == "material")
        {
            balanced = quest.rewardExperience * 55 / 100;
        }

        return std::max(1, balanced);
    }

    int balancedQuestGold(const Quest& quest)
    {
        if (quest.rewardGold <= 0)
        {
            return 0;
        }

        const int power = rankPowerForQuestReward(quest.rank);
        const int target = std::max(1, quest.target);
        int balanced = quest.rewardGold;

        // FR: Filet de sécurité pour les anciennes quêtes acceptées avant le rééquilibrage du panneau.
        // EN: Safety net for old accepted quests created before the board rebalance.
        if (quest.objectiveType == "service")
        {
            balanced = std::min(quest.rewardGold, 3 + power * 3 + target * 2);
        }
        else if (quest.objectiveType == "livraison")
        {
            balanced = quest.rewardGold * 60 / 100;
        }
        else if (quest.objectiveType == "bestiaire")
        {
            balanced = quest.rewardGold * 55 / 100;
        }
        else if (quest.objectiveType == "exploration")
        {
            balanced = quest.rewardGold * 72 / 100;
        }
        else if (quest.objectiveType == "material")
        {
            balanced = quest.rewardGold * 65 / 100;
        }

        if (!quest.rewardMaterialId.empty() && quest.rewardMaterialQuantity > 0)
        {
            balanced = balanced * 75 / 100;
        }

        int displayCap = 60 + power * 18 + target * 4;
        if (quest.objectiveType == "service") displayCap = 10 + power * 5 + target;
        else if (quest.objectiveType == "livraison") displayCap = 28 + power * 8 + target * 3;
        else if (quest.objectiveType == "bestiaire") displayCap = 26 + power * 7 + target * 3;
        else if (quest.objectiveType == "material") displayCap = 34 + power * 10 + target * 3;
        else if (quest.objectiveType == "exploration") displayCap = 52 + power * 14 + target * 4;

        balanced = std::min(balanced, displayCap);
        return std::max(0, balanced);
    }

    std::string questRewardText(const Quest& quest)
    {
        std::string text = "XP +" + std::to_string(balancedQuestExperience(quest));
        const int displayedGold = balancedQuestGold(quest);

        if (displayedGold > 0)
        {
            text += " | Argent +" + Money::formatGoldWithRaw(displayedGold);
        }

        if (!quest.rewardMaterialId.empty() && quest.rewardMaterialQuantity > 0)
        {
            text += " | Objet : " + quest.rewardMaterialName + " x" + std::to_string(quest.rewardMaterialQuantity);
        }

        if (!quest.rewardNote.empty())
        {
            text += " | " + quest.rewardNote;
        }

        if (balancedQuestGold(quest) <= 0 && quest.rewardMaterialId.empty() && quest.rewardNote.empty())
        {
            text += " | Pas de prime en pièces";
        }

        return text;
    }


    std::string lowerQuestDialogueText(std::string value)
    {
        std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });
        return value;
    }

    bool questDialogueContainsAny(const Quest& quest, const std::vector<std::string>& needles)
    {
        const std::string combined = lowerQuestDialogueText(
            quest.title + " " + quest.objective + " " + quest.objectiveType + " " + quest.targetFamily + " " + quest.location
        );

        for (const std::string& needle : needles)
        {
            if (combined.find(lowerQuestDialogueText(needle)) != std::string::npos)
            {
                return true;
            }
        }

        return false;
    }

    bool isCountedHuntQuest(const Quest& quest)
    {
        return quest.objectiveType == "combat"
            && quest.target > 1
            && questDialogueContainsAny(quest, {"chasse", "chasser", "traquer", "nettoyer", "objectif chiffré"});
    }

    std::string countedHuntQuestLine(const Quest& quest)
    {
        return "Chasse chiffrée : " + std::to_string(quest.progress) + "/" + std::to_string(quest.target)
            + " cible" + (quest.target > 1 ? "s" : "")
            + " validée" + (quest.target > 1 ? "s" : "")
            + " pour ce rang.";
    }

    bool isArtificialHuntQuest(const Quest& quest)
    {
        return isCountedHuntQuest(quest)
            && questDialogueContainsAny(quest, {"construction", "automate", "golem", "armure", "sentinelle", "mannequin", "pantin", "artificielle"});
    }

    std::vector<std::string> guildServiceTrialLines(const Quest& quest)
    {
        std::vector<std::string> lines;
        if (quest.objectiveType != "service")
        {
            return lines;
        }

        if (questDialogueContainsAny(quest, {"tri de sac", "inventaire trop", "sac trop", "poids", "fragilité", "fragilite"}))
        {
            lines.push_back("Épreuve : tri logique d'inventaire.");
            lines.push_back("À faire : choisir quoi garder, déposer, vendre ou signaler selon poids, valeur, fragilité et utilité de quête.");
            lines.push_back("Réussite : les objets importants restent protégés et la guilde évite une perte bête avant le départ.");
            return lines;
        }

        if (questDialogueContainsAny(quest, {"armure mal ajustée", "armure mal ajustee", "sangles", "morphologie", "semi-humain", "ailes", "queue", "écailles", "ecailles"}))
        {
            lines.push_back("Épreuve : diagnostic d'équipement morphologique.");
            lines.push_back("À faire : repérer ce qui gêne la race ou sous-race puis proposer l'ajustement cohérent : ailes, queue, cornes, écailles, fourrure, taille ou masse.");
            lines.push_back("Réussite : l'équipement devient crédible sans créer un bonus de combat gratuit.");
            return lines;
        }

        if (questDialogueContainsAny(quest, {"caisse de réparation", "caisse de reparation", "entretien de terrain", "pièces réparables", "pieces reparables"}))
        {
            lines.push_back("Épreuve : tri d'entretien et de réparation.");
            lines.push_back("À faire : séparer pièces réparables, déchets dangereux et composants utiles avant livraison à l'artisan.");
            return lines;
        }

        return lines;
    }

    void tryGrantQuestTitle(Player& player, std::vector<std::string>& resultLines, const std::string& title, const std::string& reason)
    {
        if (title.empty())
        {
            return;
        }

        if (player.grantTitle(title))
        {
            resultLines.push_back("Titre obtenu : " + title + ".");
            if (!reason.empty())
            {
                resultLines.push_back("  > " + reason);
            }
        }
    }

    void applyQuestTitleRewards(Player& player, const Quest& quest, std::vector<std::string>& resultLines)
    {
        if (!quest.guildQuest)
        {
            return;
        }

        const int questPower = rankPowerForQuestReward(quest.rank);
        if (isCountedHuntQuest(quest))
        {
            tryGrantQuestTitle(player, resultLines, "Chasseur de guilde", "Première chasse chiffrée officiellement validée.");

            if (questPower >= rankPowerForQuestReward("B"))
            {
                tryGrantQuestTitle(player, resultLines, "Chasseur confirmé", "Chasse de rang B ou supérieur validée sans transformer le danger en simple routine.");
            }

            if (questPower >= rankPowerForQuestReward("S"))
            {
                tryGrantQuestTitle(player, resultLines, "Grand chasseur de guilde", "Chasse de rang S ou supérieur validée avec assez de preuves pour la carte magique.");
            }

            if (questDialogueContainsAny(quest, {"rat", "slime", "gobelin", "araignée", "araignee", "insect"}))
            {
                tryGrantQuestTitle(player, resultLines, "Nettoyeur de nuisibles", "Contrat de nuisibles validé : petit danger, vraie utilité pour les habitants.");
            }

            if (questDialogueContainsAny(quest, {"loup", "ours", "bête", "bete", "bêtes lourdes", "betes lourdes"}))
            {
                tryGrantQuestTitle(player, resultLines, "Pisteur de bêtes", "Chasse de créature naturelle validée avec suivi de terrain.");
            }

            if (questDialogueContainsAny(quest, {"squelette", "goule", "revenant", "mort-vivant", "morts-vivants", "tombe", "cimetière", "cimetiere"}))
            {
                tryGrantQuestTitle(player, resultLines, "Gardien des tombes", "Présence morte-vivante nettoyée sans laisser la guilde inventer une fausse paix.");
            }

            if (isArtificialHuntQuest(quest))
            {
                tryGrantQuestTitle(player, resultLines, "Briseur d'automates", "Créature artificielle neutralisée : automate, golem, armure vivante, statue ou pantin animé.");
            }

            if (questDialogueContainsAny(quest, {"draconide", "drake"}))
            {
                tryGrantQuestTitle(player, resultLines, "Traqueur de draconides", "Contrat draconique validé sans prétendre avoir réglé toute la région.");
            }

            if (questDialogueContainsAny(quest, {"dragon"}))
            {
                tryGrantQuestTitle(player, resultLines, "Tueur de dragon", "Chasse visant des dragons validée par la guilde.");
            }

            if (questDialogueContainsAny(quest, {"démon", "demon", "infernal"}))
            {
                tryGrantQuestTitle(player, resultLines, "Fléau infernal", "Présence démoniaque réduite et notée par la guilde.");
            }

            if (questDialogueContainsAny(quest, {"rat", "rats"}))
            {
                tryGrantQuestTitle(player, resultLines, "Ratier des caves", "Nuisibles de cave assez souvent nettoyés pour que les intendants retiennent le nom.");
            }
            if (questDialogueContainsAny(quest, {"gobelin", "gobelins"}))
            {
                tryGrantQuestTitle(player, resultLines, "Gobelinophobe administratif", "La guilde a dû écrire trop de fois le mot gobelin dans tes preuves de chasse.");
            }
            if (questDialogueContainsAny(quest, {"slime", "slimes"}))
            {
                tryGrantQuestTitle(player, resultLines, "Fléau des slimes", "Les résidus collants finissent par former une réputation.");
            }
            if (questDialogueContainsAny(quest, {"loup", "loups", "crocs"}))
            {
                tryGrantQuestTitle(player, resultLines, "Morsure rendue", "Chasses à crocs validées sans laisser la meute décider du rapport.");
            }
            if (questDialogueContainsAny(quest, {"araignée", "araignee", "insect", "nid"}))
            {
                tryGrantQuestTitle(player, resultLines, "Tisseur coupé", "Nids, fils ou insectoïdes nettoyés assez proprement pour marquer le registre.");
            }
            if (questPower >= rankPowerForQuestReward("SSS") && questDialogueContainsAny(quest, {"nid impossible", "source", "zone morte", "éradication", "eradication"}))
            {
                tryGrantQuestTitle(player, resultLines, "Bourreau des nids impossibles", "Source de monstres traitée à un niveau que la guilde garde volontairement flou.");
            }
        }

        if (questDialogueContainsAny(quest, {"anomalie", "interface", "hallucination", "caractères", "caracteres", "faux pve", "cible"}))
        {
            tryGrantQuestTitle(player, resultLines, "Lecteur d'anomalies", "Contrat lié aux affichages faux, cibles instables ou hallucinations validé.");
            if (questPower >= rankPowerForQuestReward("A"))
            {
                tryGrantQuestTitle(player, resultLines, "Œil fissuré", "Contrat d'anomalie assez sérieux pour apprendre à douter de l'affichage.");
            }
            if (questPower >= rankPowerForQuestReward("S"))
            {
                tryGrantQuestTitle(player, resultLines, "Celui qui recompte les cibles", "Contrat où le nombre d'ennemis, de témoins ou de cibles ne reste pas fiable.");
            }
        }

        if (quest.objectiveType == "service")
        {
            tryGrantQuestTitle(player, resultLines, "Aide de quartier", "Service de guilde validé : utile sans forcément devenir héroïque.");
            if (questDialogueContainsAny(quest, {"tri de sac", "inventaire trop", "sac trop"}))
            {
                tryGrantQuestTitle(player, resultLines, "Sac discipliné", "Inventaire trié sans sacrifier l'objet utile au nom du poids.");
            }
            if (questDialogueContainsAny(quest, {"armure mal ajustée", "armure mal ajustee", "sangles", "morphologie", "semi-humain"}))
            {
                tryGrantQuestTitle(player, resultLines, "Armurier qui écoute", "Équipement adapté à une morphologie au lieu d'être serré au hasard.");
            }
            if (questPower >= rankPowerForQuestReward("A") && questDialogueContainsAny(quest, {"noble", "client", "politique", "royal", "trois clients", "témoins", "temoins"}))
            {
                tryGrantQuestTitle(player, resultLines, "Négociateur de crise", "Mission sociale sensible réglée sans que le panneau devienne un champ de bataille.");
            }
        }

        if (quest.objectiveType == "exploration")
        {
            tryGrantQuestTitle(player, resultLines, "Éclaireur de route", "Exploration validée avec assez de retour pour aider les prochains.");
            if (questDialogueContainsAny(quest, {"frontière", "frontiere", "biome", "carte", "limite"}))
            {
                tryGrantQuestTitle(player, resultLines, "Cartographe de biome", "Mission où le terrain comptait vraiment, pas juste le niveau de la cible.");
            }
            if (questPower >= rankPowerForQuestReward("A") && questDialogueContainsAny(quest, {"frontière", "frontiere", "carte", "limite", "lieu impossible"}))
            {
                tryGrantQuestTitle(player, resultLines, "Marcheur de frontières", "Exploration d'une limite instable que la guilde ne peut pas dessiner simplement.");
            }
            if (questDialogueContainsAny(quest, {"ruine", "atelier", "caveau", "carrière", "carriere", "relais"}))
            {
                tryGrantQuestTitle(player, resultLines, "Pied sûr des ruines", "Retour d'un lieu ancien sans laisser le rapport devenir une épitaphe.");
            }
            if (questDialogueContainsAny(quest, {"brume", "canaux", "noyée", "noyee", "barque", "eau"}))
            {
                tryGrantQuestTitle(player, resultLines, "Respiration de brume", "Mission menée dans un lieu où l'eau ou la brume brouillait la route.");
            }
            if (questDialogueContainsAny(quest, {"falaise", "corniche", "montagne", "drake", "draconide"}))
            {
                tryGrantQuestTitle(player, resultLines, "Corniche tenue", "Mission de hauteur ou de corniche validée sans tomber dans le décor.");
            }
        }

        if (quest.objectiveType == "bestiaire")
        {
            tryGrantQuestTitle(player, resultLines, "Archiviste de terrain", "Observation de terrain utile ajoutée aux registres de la guilde.");
            if (questPower >= rankPowerForQuestReward("B"))
            {
                tryGrantQuestTitle(player, resultLines, "Lecteur de traces", "Dossier complété sans transformer une rumeur en vérité gratuite.");
            }
            if (questDialogueContainsAny(quest, {"archive", "registre", "bestiaire vivant", "vivante", "pages"}))
            {
                tryGrantQuestTitle(player, resultLines, "Catalogue vivant", "Le registre semblait presque répondre, mais tu es revenu avec des notes utiles.");
            }
        }

        if (questDialogueContainsAny(quest, {"matériau", "materiau", "matériaux", "materiaux", "composant", "cuir", "rare", "rareté", "rarete", "boss"}))
        {
            tryGrantQuestTitle(player, resultLines, "Trieur de matériaux", "Composants, qualités ou lots douteux classés proprement.");
            if (questPower >= rankPowerForQuestReward("A"))
            {
                tryGrantQuestTitle(player, resultLines, "Inspecteur de reliques", "Matériau rare ou lié à une menace supérieure étudié sans le vendre trop vite.");
            }
            if (questDialogueContainsAny(quest, {"maudite", "maudit", "interface maudite", "qualité affichée", "qualite affichee", "rareté affichée", "rarete affichee"}))
            {
                tryGrantQuestTitle(player, resultLines, "Œil des composants maudits", "Lot dont l'affichage ou la qualité mentait, identifié sans se fier au premier chiffre.");
            }
        }

        if (questDialogueContainsAny(quest, {"prix", "taxe", "taxes", "stock", "stocks", "monnaie", "facture", "économie", "economie", "prime", "récompense", "recompense"}))
        {
            tryGrantQuestTitle(player, resultLines, "Contrôleur de prix", "Mission économique validée : le registre ne tombe pas juste tout seul.");
            if (questDialogueContainsAny(quest, {"crise", "réparations", "reparations", "ville en réparations", "tension"}))
            {
                tryGrantQuestTitle(player, resultLines, "Commis de crise", "Aide économique ou logistique apportée pendant une vraie tension de ville.");
            }
            if (questDialogueContainsAny(quest, {"convoi", "livraison", "caravane", "route commerciale"}))
            {
                tryGrantQuestTitle(player, resultLines, "Livreur sous tension", "Livraison fragile ou convoi maintenu malgré les risques.");
            }
            if (questDialogueContainsAny(quest, {"facture", "monnaie", "registre", "calcul", "prix"}))
            {
                tryGrantQuestTitle(player, resultLines, "Marchand qui recompte", "Un compte douteux a été repris sans faire semblant qu'il était clair.");
            }
        }

        if (questDialogueContainsAny(quest, {"semi-humain", "semi-humains", "sous-race", "race", "loup", "chat", "renard", "piaf", "lézard", "lezard"})
            && questDialogueContainsAny(quest, {"dialogue", "dispute", "remarque", "malentendu", "ville", "guilde", "client"}))
        {
            tryGrantQuestTitle(player, resultLines, "Médiateur semi-humain", "Mission sociale liée aux races ou sous-races réglée sans caricaturer le passif racial.");
            if (questPower >= rankPowerForQuestReward("B"))
            {
                tryGrantQuestTitle(player, resultLines, "Parole sans morsure", "Différence raciale respectée sans donner au joueur une solution magique gratuite.");
            }
        }

        if (questDialogueContainsAny(quest, {"scellé", "scelle", "classée", "classee", "interdit", "interdite", "porte fermée", "porte fermee"}))
        {
            tryGrantQuestTitle(player, resultLines, "Nom scellé par la guilde", "Contrat que la guilde préfère garder moins visible que les primes ordinaires.");
        }

        if (questDialogueContainsAny(quest, {"matériau", "materiau", "matériaux", "materiaux", "rare", "relique", "boss"})
            && questPower >= rankPowerForQuestReward("S"))
        {
            tryGrantQuestTitle(player, resultLines, "Main qui ne vend pas tout", "Ressource assez rare pour apprendre à ne pas transformer tout le loot en monnaie rapide.");
        }

        if (questPower >= rankPowerForQuestReward("Héros mondial") && questDialogueContainsAny(quest, {"frontière", "frontiere", "cartes", "carte", "région", "region"}))
        {
            tryGrantQuestTitle(player, resultLines, "Frontière refusée", "Région ou carte stabilisée alors qu'elle refusait de rester simple.");
        }

        if (questPower >= rankPowerForQuestReward("S"))
        {
            tryGrantQuestTitle(player, resultLines, "Sang-froid de rang S", "Contrat de très haut rang validé sans que le panneau prétende que c'était normal.");
        }
        if (questPower >= rankPowerForQuestReward("SS"))
        {
            tryGrantQuestTitle(player, resultLines, "Vétéran des contrats scellés", "Contrat scellé ou catastrophique ajouté au dossier permanent.");
        }
        if (questPower >= rankPowerForQuestReward("Héros mondial"))
        {
            tryGrantQuestTitle(player, resultLines, "Mandataire des rois", "Mission dont l'impact dépasse une simple ville.");
        }
        if (questPower >= rankPowerForQuestReward("Légende"))
        {
            tryGrantQuestTitle(player, resultLines, "Ligne vivante du registre", "Contrat si haut que le registre semble hésiter à l'écrire.");
        }

        if (questDialogueContainsAny(quest, {"réparations", "reparations", "réparer", "reparer", "rations", "dépôt", "depot", "défense", "defense"}))
        {
            tryGrantQuestTitle(player, resultLines, "Main des réparations", "Aide concrète apportée à une ville ou un dépôt au lieu d'attendre une récompense gratuite.");
        }

        if (questDialogueContainsAny(quest, {"rumeur", "témoins", "temoins", "panneau", "affiche", "ligne", "brouillé", "brouille"}))
        {
            tryGrantQuestTitle(player, resultLines, "Rumeur calmée", "Information instable vérifiée avant que la guilde ne panique.");
        }
    }

    bool questHintsAllowedByFrequency(const Player& player, const Quest& quest, bool titleOrRaceLine = false)
    {
        const std::string frequency = player.getInterfaceHintFrequency();
        if (frequency == "null")
        {
            return false;
        }

        const int maxHp = player.getMaxHp();
        const bool lowHealth = maxHp > 0 && player.getHp() * 100 <= maxHp * 35;
        const bool dangerousRank = rankPowerForQuestReward(quest.rank) >= rankPowerForQuestReward("S");
        const bool solidRank = rankPowerForQuestReward(quest.rank) >= rankPowerForQuestReward("B");
        const bool explicitPreparation = questDialogueContainsAny(quest, {
            "préparation", "preparation", "avant départ", "avant depart", "réparation", "reparation",
            "équipement", "equipement", "armure", "arme", "boss", "zone morte", "route condamnée", "route condamnee",
            "inspection", "sangle", "durabilité", "durabilite", "biome", "tanière", "taniere", "piste"
        });
        const bool fieldQuest = quest.objectiveType == "combat"
            || quest.objectiveType == "exploration"
            || quest.objectiveType == "bestiaire";

        if (frequency == "forte")
        {
            return true;
        }
        if (frequency == "normal")
        {
            return lowHealth || dangerousRank || explicitPreparation || fieldQuest || (titleOrRaceLine && solidRank);
        }

        return lowHealth || dangerousRank || explicitPreparation;
    }

    std::vector<std::string> racialQuestContextLines(const Player& player, const Quest& quest)
    {
        std::vector<std::string> lines;
        if (!questHintsAllowedByFrequency(player, quest, true))
        {
            return lines;
        }

        const CharacterRace race = player.getRace();
        const bool isTracking = questDialogueContainsAny(quest, {"trace", "piste", "route", "sentier", "embuscade", "caravane", "convoi"});
        const bool isForest = questDialogueContainsAny(quest, {"forêt", "foret", "bocage", "ronce", "mousse", "plante", "vigne", "mycélium", "mycelium"});
        const bool isColdOrMountain = questDialogueContainsAny(quest, {"montagne", "froid", "givre", "glace", "neige", "falaise", "drake"});
        const bool isDesertOrHeat = questDialogueContainsAny(quest, {"désert", "desert", "argile", "sel lunaire", "dune", "chaleur"});
        const bool isRuinOrDeath = questDialogueContainsAny(quest, {"ruine", "archive", "cimetière", "cimetiere", "mort", "os", "ombre", "sépulture", "sepulture"});
        const bool isUrbanOrSocial = questDialogueContainsAny(quest, {"ville", "quartier", "client", "noble", "dette", "marché", "marche", "registre", "facture", "comptoir"});
        const bool isAnomaly = questDialogueContainsAny(quest, {"anomalie", "interface", "caractères", "caracteres", "cible", "hallucination", "faux pve"});

        switch (race)
        {
            case CharacterRace::SemiWolf:
                if (isTracking || isForest)
                {
                    lines.push_back("Réaction raciale : ton instinct de semi-loup accroche déjà une piste possible, sans confirmer l'identité de la cible.");
                }
                else
                {
                    lines.push_back("Réaction raciale : ton odorat capte la tension du lieu, utile pour sentir un danger mais pas pour révéler une faiblesse cachée.");
                }
                break;
            case CharacterRace::SemiDog:
                if (isTracking || isUrbanOrSocial)
                {
                    lines.push_back("Réaction raciale : ton côté semi-chien rend les consignes de protection et de piste plus naturelles à suivre.");
                }
                else
                {
                    lines.push_back("Réaction raciale : tu lis mieux l'humeur du client que le détail réel du danger.");
                }
                break;
            case CharacterRace::SemiCat:
                if (isRuinOrDeath || isAnomaly)
                {
                    lines.push_back("Réaction raciale : tes yeux de semi-chat remarquent les angles qui bougent trop vite, mais l'information reste une alerte, pas une vérité fiable.");
                }
                else
                {
                    lines.push_back("Réaction raciale : ton instinct d'évitement signale les endroits où il vaut mieux avancer lentement.");
                }
                break;
            case CharacterRace::SemiFox:
                if (isAnomaly || isUrbanOrSocial)
                {
                    lines.push_back("Réaction raciale : ton instinct de semi-renard sent une entourloupe possible dans les mots, les contrats ou les panneaux.");
                }
                else
                {
                    lines.push_back("Réaction raciale : tu remarques surtout ce que la demande évite de dire clairement.");
                }
                break;
            case CharacterRace::Kitsune:
                if (isAnomaly || isRuinOrDeath)
                {
                    lines.push_back("Réaction raciale : tes affinités d'illusion rendent la fiche plus suspecte, mais pas assez pour trier le vrai du faux sans enquête.");
                }
                else
                {
                    lines.push_back("Réaction raciale : tu sens que certaines formulations cachent une couche symbolique ou spirituelle.");
                }
                break;
            case CharacterRace::SemiBird:
                if (isColdOrMountain || isTracking)
                {
                    lines.push_back("Réaction raciale : ton sens du vent et des hauteurs donne une meilleure lecture du trajet prévu.");
                }
                else
                {
                    lines.push_back("Réaction raciale : tu repères vite les issues, mais la mission devra quand même être vérifiée sur place.");
                }
                break;
            case CharacterRace::SemiLizard:
                if (isDesertOrHeat)
                {
                    lines.push_back("Réaction raciale : ta résistance à la chaleur rend ce terrain moins intimidant qu'il ne devrait.");
                }
                else if (isColdOrMountain)
                {
                    lines.push_back("Réaction raciale : ton sang froid n'aime pas ce biome ; le journal note une prudence de température.");
                }
                else
                {
                    lines.push_back("Réaction raciale : tes écailles donnent confiance contre les petits frottements du terrain, pas contre les vraies erreurs.");
                }
                break;
            case CharacterRace::SemiHuman:
                lines.push_back("Réaction raciale : ton identité semi-humaine rend certains PNJ curieux ou prudents, sans bonus social automatique.");
                break;
            case CharacterRace::Elf:
                if (isForest)
                {
                    lines.push_back("Réaction raciale : ton affinité elfique rend les signes naturels plus lisibles, tant qu'ils ne sont pas corrompus.");
                }
                break;
            case CharacterRace::DarkElf:
                if (isRuinOrDeath || isUrbanOrSocial)
                {
                    lines.push_back("Réaction raciale : ton habitude des zones sombres rend les silences et les mensonges un peu moins confortables pour les autres.");
                }
                break;
            case CharacterRace::Dwarf:
                if (isColdOrMountain || questDialogueContainsAny(quest, {"mine", "forge", "métal", "metal", "pierre"}))
                {
                    lines.push_back("Réaction raciale : ton regard nain vérifie déjà la pierre, le métal et les risques d'effondrement.");
                }
                break;
            case CharacterRace::Gnome:
                if (questDialogueContainsAny(quest, {"machine", "automate", "registre", "archive", "interface", "engrenage"}))
                {
                    lines.push_back("Réaction raciale : ta curiosité gnome trouve la mécanique suspecte intéressante, ce qui est rarement rassurant.");
                }
                break;
            case CharacterRace::Halfling:
                lines.push_back("Réaction raciale : ton instinct de survie discret te rappelle qu'un petit détour vaut parfois mieux qu'un grand discours héroïque.");
                break;
            case CharacterRace::Tiefling:
                if (isRuinOrDeath || questDialogueContainsAny(quest, {"malédiction", "malediction", "démon", "demon", "infernal"}))
                {
                    lines.push_back("Réaction raciale : ton héritage infernal rend certaines traces plus familières, sans les rendre moins dangereuses.");
                }
                break;
            case CharacterRace::Aasimar:
                if (isRuinOrDeath || questDialogueContainsAny(quest, {"temple", "serment", "cloche", "malédiction", "malediction"}))
                {
                    lines.push_back("Réaction raciale : ta part céleste réagit faiblement, comme une mise en garde plutôt qu'une réponse.");
                }
                break;
            case CharacterRace::Vampire:
                if (isRuinOrDeath || isUrbanOrSocial)
                {
                    lines.push_back("Réaction raciale : ta présence vampirique attire quelques regards ; utile pour intimider, mauvais pour passer inaperçu.");
                }
                break;
            case CharacterRace::Demon:
                lines.push_back("Réaction raciale : la gérante surveille les mots avec prudence ; ta nature démoniaque peut tendre les échanges commerciaux ou sociaux.");
                break;
            case CharacterRace::Fairy:
                if (isForest || isAnomaly)
                {
                    lines.push_back("Réaction raciale : ta magie féerique frissonne devant les détails trop vivants ou trop faux du contrat.");
                }
                break;
            case CharacterRace::HalfDragon:
                if (isColdOrMountain || isDesertOrHeat || questDialogueContainsAny(quest, {"drake", "dragon", "draconique"}))
                {
                    lines.push_back("Réaction raciale : ton sang draconique réagit au terrain et aux traces de créatures anciennes, sans identifier la menace à lui seul.");
                }
                break;
            case CharacterRace::Orc:
                if (quest.objectiveType == "combat")
                {
                    lines.push_back("Réaction raciale : ton tempérament orc rend l'approche frontale tentante, mais la guilde insiste quand même sur la lecture du terrain.");
                }
                break;
            default:
                break;
        }

        if (player.hasActiveCurse("anomaly_interface_desync") && isAnomaly)
        {
            lines.push_back("Malédiction active : la fiche semble te regarder en retour. Le journal marque cette réaction comme parasite, pas comme information fiable.");
        }

        return lines;
    }


    bool playerHasEquippedTitleContaining(const Player& player, const std::vector<std::string>& needles)
    {
        for (const std::string& title : player.getActiveTitles())
        {
            const std::string loweredTitle = lowerQuestDialogueText(title);
            for (const std::string& needle : needles)
            {
                if (loweredTitle.find(lowerQuestDialogueText(needle)) != std::string::npos)
                {
                    return true;
                }
            }
        }

        return false;
    }

    std::vector<std::string> equippedTitleQuestContextLines(const Player& player, const Quest& quest)
    {
        std::vector<std::string> lines;
        if (!questHintsAllowedByFrequency(player, quest, true))
        {
            return lines;
        }

        const std::vector<std::string>& equippedTitles = player.getActiveTitles();
        if (equippedTitles.empty())
        {
            return lines;
        }

        const bool economyQuest = questDialogueContainsAny(quest, {"prix", "taxe", "stock", "monnaie", "facture", "économie", "economie", "prime", "récompense", "recompense", "crise"});
        const bool materialQuest = questDialogueContainsAny(quest, {"matériau", "materiau", "matériaux", "materiaux", "composant", "cuir", "forge", "rare", "rareté", "rarete"});
        const bool anomalyQuest = questDialogueContainsAny(quest, {"anomalie", "interface", "hallucination", "cible", "faux pve", "caractères", "caracteres"});
        const bool huntQuest = isCountedHuntQuest(quest) || quest.objectiveType == "combat";
        const bool socialQuest = questDialogueContainsAny(quest, {"client", "noble", "semi-humain", "semi-humains", "sous-race", "dialogue", "dispute", "ville", "guilde"});
        const bool biomeQuest = questDialogueContainsAny(quest, {"biome", "frontière", "frontiere", "forêt", "foret", "marais", "montagne", "désert", "desert", "brume", "corniche", "route"});

        bool matched = false;
        if (economyQuest && playerHasEquippedTitleContaining(player, {"prix", "marchand", "millionnaire", "banquier", "crise", "livreur", "coffre"}))
        {
            lines.push_back("Titres équipés : ta réputation économique aide surtout à obtenir des explications plus propres, pas une grosse remise gratuite.");
            matched = true;
        }
        if (materialQuest && playerHasEquippedTitleContaining(player, {"matériaux", "materiaux", "reliques", "composants", "vend pas tout", "automates"}))
        {
            lines.push_back("Titres équipés : le client te laisse regarder les composants d'un peu plus près, sans révéler leur usage final.");
            matched = true;
        }
        if (anomalyQuest && playerHasEquippedTitleContaining(player, {"anomal", "menu", "débogueur", "debug", "fissuré", "fissure", "cibles"}))
        {
            lines.push_back("Titres équipés : les mentions d'interface instable te rendent crédible... ou inquiétant. L'effet reste social, pas une immunité.");
            matched = true;
        }
        if (huntQuest && playerHasEquippedTitleContaining(player, {"chasseur", "tueur", "pisteur", "fléau", "fleau", "briseur", "traqueur", "tombeur"}))
        {
            lines.push_back("Titres équipés : la guilde note que ton identité affichée colle au danger. Petit respect, zéro garantie de survie.");
            matched = true;
        }
        if (socialQuest && playerHasEquippedTitleContaining(player, {"médiateur", "mediateur", "parole", "aide de quartier", "main fiable", "négociateur", "negociateur"}))
        {
            lines.push_back("Titres équipés : les PNJ commencent la discussion un peu moins sur la défensive, tant que tes actes suivent.");
            matched = true;
        }
        if (biomeQuest && playerHasEquippedTitleContaining(player, {"cartographe", "frontière", "frontiere", "corniche", "brume", "éclaireur", "eclaireur", "ruines"}))
        {
            lines.push_back("Titres équipés : les indications de terrain sont mieux prises au sérieux, mais le biome reste à lire sur place.");
            matched = true;
        }

        // FR: Pas de ligne générique à chaque quête : les titres équipés ne doivent ressortir que si le contrat leur donne vraiment un contexte.
        // EN: No generic line on every quest: equipped titles should surface only when the contract context matters.
        (void)matched;
        return lines;
    }

    bool shouldShowPassiveQuestAdvice(const Player& player, const Quest& quest)
    {
        return questHintsAllowedByFrequency(player, quest, false);
    }

    std::vector<std::string> passiveQuestContextLines(const Player& player, const Quest& quest)
    {
        std::vector<std::string> lines;
        if (!shouldShowPassiveQuestAdvice(player, quest))
        {
            return lines;
        }

        const bool combatQuest = quest.objectiveType == "combat";
        const bool explorationQuest = quest.objectiveType == "exploration" || quest.objectiveType == "bestiaire";
        const bool materialQuest = questDialogueContainsAny(quest, {"matériau", "materiau", "composant", "réparation", "reparation", "forge", "cuir", "métal", "metal", "armure", "arme"});
        const bool routeQuest = questDialogueContainsAny(quest, {"route", "sentier", "convoi", "caravane", "piste", "frontière", "frontiere", "carte", "zone morte"});
        const bool familyQuest = questDialogueContainsAny(quest, {"famille", "bête", "bete", "slime", "gobelin", "mort-vivant", "automate", "dragon", "draconide", "démon", "demon"});

        auto addLimited = [&lines](const std::string& line) {
            if (lines.size() < 2)
            {
                lines.push_back(line);
            }
        };

        if (materialQuest && player.hasPassiveSkill("material_sorting_habit"))
        {
            addLimited("Rappel rare de passif : Tri des composants aide surtout à préparer le sac ou une réparation, sans révéler l'usage caché des matériaux.");
        }
        if ((materialQuest || combatQuest) && player.hasPassiveSkill("weapon_care_habit"))
        {
            addLimited("Rappel rare de passif : Soin d'arme te pousse à vérifier lame, manche, corde ou catalyseur avant une sortie longue.");
        }
        if ((materialQuest || explorationQuest) && player.hasPassiveSkill("armor_fit_memory"))
        {
            addLimited("Rappel rare de passif : Mémoire d'ajustement sert surtout à l'inspection d'armure, sangles et frottements avant départ.");
        }
        if (routeQuest && player.hasPassiveSkill("guild_route_memory"))
        {
            addLimited("Rappel rare de passif : Mémoire de route aide à relire les détours et délais, mais ne rend pas le trajet sûr.");
        }
        if ((routeQuest || explorationQuest) && player.hasPassiveSkill("cautious_pathing"))
        {
            addLimited("Rappel rare de passif : Pas prudent te fait vérifier sorties et repères surtout quand la mission sent mauvais.");
        }
        if ((combatQuest || explorationQuest) && player.hasPassiveSkill("threat_route_planner"))
        {
            addLimited("Rappel rare de passif : Plan de route dangereux aide à préparer la chasse ou l'exploration avant le premier coup.");
        }
        if (familyQuest && player.hasPassiveSkill("bestiary_family_reader"))
        {
            addLimited("Rappel rare de passif : Lecture des familles aide à classer la menace, pas à connaître ses faiblesses gratuitement.");
        }

        return lines;
    }

    std::vector<std::string> guildQuestAcceptedDialogueLines(const Player& player, const Quest& quest)
    {
        std::vector<std::string> lines;
        lines.push_back("La gérante pose un doigt sur la ligne du contrat.");

        if (quest.objectiveType == "combat")
        {
            lines.push_back("Elle précise que les témoins parlent d'une menace mobile, pas d'un simple sac de PV qui attend poliment.");
            lines.push_back("Si la zone devient trop calme, c'est probablement que quelque chose écoute déjà.");
        }
        else if (quest.objectiveType == "exploration" || quest.objectiveType == "bestiaire")
        {
            lines.push_back("Elle te demande de revenir avec des notes propres, pas juste avec une phrase du style 'j'ai vu un truc bizarre'.");
            lines.push_back("La guilde paie mieux les aventuriers qui savent lire le terrain avant de le piétiner.");
        }
        else if (quest.objectiveType == "livraison" || quest.objectiveType == "service")
        {
            lines.push_back("Elle résume le service demandé : rien d'héroïque sur le papier, mais les petites affaires tiennent parfois une ville entière.");
            lines.push_back("Le client veut du sérieux, pas une grande légende avec trois fautes dans son nom.");
        }
        else
        {
            lines.push_back("Elle reste vague, ce qui est rarement bon signe dans une guilde qui vend normalement le danger au mot près.");
        }

        lines.push_back("Zone/action annoncée : " + questPlayableLocationHint(quest) + ". La gérante précise que le panneau donne une piste jouable, pas une promesse de sécurité.");

        if (!quest.targetFamily.empty())
        {
            lines.push_back("Famille ciblée : " + quest.targetFamily + ". Elle recommande de noter ce qui est observé avant de tout régler à coups de panique.");
        }

        if (isCountedHuntQuest(quest))
        {
            lines.push_back("La gérante précise le chiffre : ce contrat demande " + std::to_string(quest.target) + " cible" + (quest.target > 1 ? "s" : "") + ", pas juste une sortie au hasard. Le rang de la quête augmente la quantité demandée.");
            if (isArtificialHuntQuest(quest))
            {
                lines.push_back("Elle ajoute que 'créature artificielle' signifie automate, golem, armure vivante, statue ou pantin animé : la guilde ne te demande pas de casser une maison.");
            }
        }

        if (quest.rank == "S" || quest.rank == "SS" || quest.rank == "SSS" || quest.rank == "Légende" || quest.rank == "Dieu")
        {
            lines.push_back("Avant de te laisser partir, elle ajoute que ce rang n'est pas une décoration : c'est une manière polie de prévenir les inconscients.");
        }

        std::vector<std::string> racialLines = racialQuestContextLines(player, quest);
        lines.insert(lines.end(), racialLines.begin(), racialLines.end());
        std::vector<std::string> titleLines = equippedTitleQuestContextLines(player, quest);
        lines.insert(lines.end(), titleLines.begin(), titleLines.end());
        std::vector<std::string> passiveLines = passiveQuestContextLines(player, quest);
        lines.insert(lines.end(), passiveLines.begin(), passiveLines.end());

        return lines;
    }

    std::vector<std::string> clientQuestAcceptedDialogueLines(const Player& player, const Quest& quest)
    {
        std::vector<std::string> lines;
        lines.push_back(quest.client + " garde la voix basse en détaillant la demande.");

        if (quest.objectiveType == "combat")
        {
            lines.push_back("Le problème a commencé par des bruits au loin, puis par des traces, puis par des gens qui ont arrêté de faire les malins.");
            lines.push_back("Le client ne veut pas seulement une victoire : il veut pouvoir dormir sans compter les ombres.");
        }
        else if (quest.objectiveType == "livraison")
        {
            lines.push_back("Ce qui manque paraît banal, mais tout devient urgent quand une boutique, un atelier ou une famille attend dessus.");
            lines.push_back("Le client te décrit rapidement où la trace se perd et ce qu'il ne faut pas confondre avec la bonne marchandise.");
        }
        else if (quest.objectiveType == "exploration" || quest.objectiveType == "bestiaire")
        {
            lines.push_back("Il ne demande pas de ramener le monde entier dans un sac, seulement assez d'informations pour éviter au prochain idiot de se perdre.");
            lines.push_back("Les détails du terrain comptent : couleur des traces, bruit des pierres, odeur trop forte, tout ce qui semble inutile jusqu'au moment où ça sauve une jambe.");
        }
        else
        {
            lines.push_back("Il ajoute quelques détails personnels, pas assez pour faire un roman, mais assez pour que la mission ressemble enfin à autre chose qu'une ligne de menu.");
        }

        lines.push_back("Lieu ou action évoquée : " + questPlayableLocationHint(quest) + ". Le client n'est pas certain de tout, mais il sait où chercher en premier.");

        if (!quest.targetFamily.empty())
        {
            lines.push_back("Indice donné : la demande semble liée à " + quest.targetFamily + ", sans garantie officielle tant que la guilde n'a rien vérifié.");
        }

        std::vector<std::string> racialLines = racialQuestContextLines(player, quest);
        lines.insert(lines.end(), racialLines.begin(), racialLines.end());
        std::vector<std::string> titleLines = equippedTitleQuestContextLines(player, quest);
        lines.insert(lines.end(), titleLines.begin(), titleLines.end());
        std::vector<std::string> passiveLines = passiveQuestContextLines(player, quest);
        lines.insert(lines.end(), passiveLines.begin(), passiveLines.end());

        return lines;
    }


    MenuOptionItemData makeQuestNavigationItemData(
        const std::string& kind,
        const std::string& section,
        const std::string& actionType,
        const std::string& name,
        const std::string& detail,
        const std::string& owner = ""
    )
    {
        MenuOptionItemData itemData;
        itemData.structured = true;
        itemData.kind = kind;
        itemData.section = section;
        itemData.actionType = actionType;
        itemData.name = name;
        itemData.detail = detail;
        itemData.owner = owner;
        itemData.status = "Accessible";
        itemData.important = actionType == "quest" || actionType == "talk";
        return itemData;
    }

    std::string toLowerChoiceText(std::string text)
    {
        std::transform(text.begin(), text.end(), text.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });
        return text;
    }

    bool choiceTextContainsAny(const std::string& text, const std::vector<std::string>& needles)
    {
        const std::string lowerText = toLowerChoiceText(text);
        for (const std::string& needle : needles)
        {
            if (lowerText.find(toLowerChoiceText(needle)) != std::string::npos)
            {
                return true;
            }
        }

        return false;
    }

    MenuOptionItemData makeChoiceScreenItemData(
        const std::string& screenId,
        const std::string& title,
        int choiceNumber,
        const std::string& label
    )
    {
        const std::string context = screenId + " " + title + " " + label;

        MenuOptionItemData itemData;
        itemData.structured = true;
        itemData.name = label;
        itemData.detail = "Choix " + std::to_string(choiceNumber) + " de l'écran : " + title + ".";
        itemData.progress = "Choix " + std::to_string(choiceNumber);
        itemData.status = "Disponible";
        itemData.section = "Choix contextuel";
        itemData.kind = "entry";
        itemData.actionType = "select";

        if (choiceTextContainsAny(context, {"coffre", "chest"}))
        {
            itemData.kind = "chest";
            itemData.section = "Coffres";
            itemData.actionType = choiceTextContainsAny(context, {"ignorer", "laisser", "partir", "retour"}) ? "ignore" : "open";
            itemData.status = choiceTextContainsAny(context, {"piégé", "piege", "risque", "instable"}) ? "Risque possible" : "À examiner";
            itemData.important = true;
        }
        else if (choiceTextContainsAny(context, {"piège", "piege", "embuscade", "bruit", "trace", "odeur", "ombre"}))
        {
            itemData.kind = "trap";
            itemData.section = "Risques";
            itemData.actionType = choiceTextContainsAny(context, {"ignorer", "contourner", "éviter", "eviter"}) ? "ignore" : "inspect";
            itemData.status = "À surveiller";
            itemData.important = true;
        }
        else if (choiceTextContainsAny(context, {"client", "pnj", "parler", "demande", "contact"}))
        {
            itemData.kind = "npc";
            itemData.section = "PNJ / demandes";
            itemData.actionType = choiceTextContainsAny(context, {"accepter"}) ? "accept" : "talk";
            itemData.status = choiceTextContainsAny(context, {"à rendre", "a rendre", "termin"}) ? "À rendre" : "Dialogue";
            itemData.owner = label;
        }
        else if (choiceTextContainsAny(context, {"guilde", "quête", "quete", "mission", "contrat"}))
        {
            itemData.kind = "quest";
            itemData.section = "Quêtes";
            itemData.actionType = choiceTextContainsAny(context, {"accepter"}) ? "accept" : "quest";
            itemData.status = "Suivi de quête";
            itemData.important = true;
        }
        else if (choiceTextContainsAny(context, {"forêt", "foret", "plaine", "route", "marais", "ruines", "cimetière", "cimetiere", "biome", "exploration"}))
        {
            itemData.kind = "exploration";
            itemData.section = "Exploration";
            itemData.actionType = "travel";
            itemData.status = "Sortie";
        }
        else if (choiceTextContainsAny(context, {"affronter", "combat", "monstre", "mini-boss", "boss"}))
        {
            itemData.kind = "monster";
            itemData.section = choiceTextContainsAny(context, {"boss"}) ? "Boss" : "Combat";
            itemData.actionType = "combat";
            itemData.status = choiceTextContainsAny(context, {"mini-boss", "boss"}) ? "Danger" : "Rencontre";
            itemData.important = true;
        }
        else if (choiceTextContainsAny(context, {"lieu", "forge", "bibliothèque", "bibliotheque", "boutique", "herboristerie", "place", "armurerie"}))
        {
            itemData.kind = "location";
            itemData.section = "Lieux";
            itemData.actionType = "travel";
            itemData.status = "Visitables";
        }
        else if (choiceTextContainsAny(context, {"retour", "revenir", "quitter"}))
        {
            itemData.kind = "navigation";
            itemData.section = "Navigation";
            itemData.actionType = "continue";
            itemData.status = "Retour";
        }

        return itemData;
    }

    void applyQuestExtraReward(Player& player, const Quest& quest)
    {
        if (!quest.rewardMaterialId.empty() && quest.rewardMaterialQuantity > 0)
        {
            Material rewardMaterial = MaterialCatalog::createById(quest.rewardMaterialId, quest.rewardMaterialQuantity);
            player.getInventory().addMaterial(rewardMaterial);
            player.recordMaterialCollected(rewardMaterial.getId(), rewardMaterial.getName(), rewardMaterial.getQuantity());
        }
    }

    bool runTrackedExplorationWave(
        Player& player,
        Random& random,
        DifficultyMode difficulty,
        DeathRuleMode deathRule,
        const std::vector<Monster>& monsters,
        const std::string& context
    )
    {
        player.recordCombatStarted();
        expireOverdueQuestDeadlines(player, "exploration.combat", true);
        ShopTransactionSystem::clearBuybackAfterCombat();
        const bool victory = MonsterPveMode::runExplorationWave(player, random, difficulty, deathRule, monsters, context);
        std::vector<std::string> timeReportLines = player.consumeWorldTimeReportLines();
        if (!timeReportLines.empty())
        {
            MessageScreen::show("FIN DE JOURNÉE", "exploration.combat.time_report", timeReportLines);
        }
        return victory;
    }

    int askChoiceScreen(
        const std::string& title,
        const std::string& screenId,
        const std::vector<std::string>& lines,
        const std::vector<std::pair<int, std::string>>& options,
        int minChoice,
        int maxChoice,
        const std::string& invalidMessage = "Choix invalide."
    )
    {
        MenuScreen screen(title, screenId);

        for (const std::string& line : lines)
        {
            screen.addLine(line);
        }

        for (const auto& option : options)
        {
            screen.addOption(
                option.first,
                option.second,
                "",
                true,
                screenId + ".choice." + std::to_string(option.first),
                makeChoiceScreenItemData(screenId, title, option.first, option.second)
            );
        }

        (void)minChoice;
        (void)maxChoice;
        return TerminalInterface::askMenuChoiceFromOptions(screen, invalidMessage);
    }


    void showExplorationNotice(
        const std::string& title,
        const std::string& screenId,
        const std::vector<std::string>& lines,
        bool waitAndClear = false
    )
    {
        MessageScreen::show(title, screenId, lines, waitAndClear);
    }

    std::string recommendationPrefix()
    {
        return "Client supplémentaire recommandé : ";
    }

    std::string extractRecommendedClientName(const Quest& quest)
    {
        const std::string prefix = recommendationPrefix();
        if (quest.rewardNote.rfind(prefix, 0) == 0)
        {
            return quest.rewardNote.substr(prefix.size());
        }

        return "";
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
            if (!quest.turnedIn || quest.rewardMaterialId != "client_recommendation")
            {
                continue;
            }

            std::string clientName = extractRecommendedClientName(quest);
            if (!clientName.empty()
                && player.getQuestLog().hasRecommendedClientCapacity(clientName)
                && seen.insert(clientName).second)
            {
                clients.push_back(clientName);
            }
        }

        const int looseRecommendations = player.getInventory().countMaterialById("client_recommendation");
        if (looseRecommendations <= 0)
        {
            return clients;
        }

        const std::vector<std::string> fallbackClients = defaultRecommendedClientNames();
        const int desiredVisibleCount = std::max(static_cast<int>(clients.size()), looseRecommendations);

        for (const std::string& clientName : fallbackClients)
        {
            if (static_cast<int>(clients.size()) >= desiredVisibleCount)
            {
                break;
            }

            if (player.getQuestLog().hasRecommendedClientCapacity(clientName)
                && seen.insert(clientName).second)
            {
                clients.push_back(clientName);
            }
        }

        return clients;
    }

    bool isRecommendedClientName(const std::string& clientName)
    {
        for (const std::string& recommended : defaultRecommendedClientNames())
        {
            if (clientName == recommended)
            {
                return true;
            }
        }

        return false;
    }

    // EN: isMaterialDeliveryQuest declares or implements a focused behavior used by this module.
    // FR: isMaterialDeliveryQuest déclare ou implémente un comportement précis utilisé par ce module.
    bool isMaterialDeliveryQuest(const Quest& quest)
    {
        return !quest.requiredMaterialId.empty() && quest.requiredMaterialQuantity > 0;
    }

    // EN: canCompleteMaterialDelivery declares or implements a focused behavior used by this module.
    // FR: canCompleteMaterialDelivery déclare ou implémente un comportement précis utilisé par ce module.
    bool canCompleteMaterialDelivery(const Player& player, const Quest& quest)
    {
        return isMaterialDeliveryQuest(quest)
            && player.getInventory().countMaterialQualityPointsById(quest.requiredMaterialId) >= quest.requiredMaterialQuantity * 2;
    }

    // EN: isReadyToTurnIn declares or implements a focused behavior used by this module.
    // FR: isReadyToTurnIn déclare ou implémente un comportement précis utilisé par ce module.
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

        // FR: pour un service PNJ, une preuve achetée/obtenue ne remplace pas l'épreuve :
        // il faut traiter les étapes ET présenter la preuve au rendu.
        // EN: for a NPC service, a proof item does not replace the task: both are required.
        if (quest.objectiveType == "service")
        {
            return quest.completed && hasRequiredQuestMaterial(player, quest);
        }

        return quest.completed || canCompleteMaterialDelivery(player, quest);
    }

    bool guildIsOpen(const Player& player)
    {
        return player.getWorldDayProgressUnits() < 4;
    }

    std::string guildOpeningLine(const Player& player)
    {
        return guildIsOpen(player)
            ? "Horaires de guilde : ouverte matin, midi, après-midi et soir. Statut actuel : ouverte."
            : "Horaires de guilde : ouverte matin, midi, après-midi et soir. Statut actuel : fermée la nuit, même avec rotation d'employés.";
    }

    // EN: displayQuestLine declares or implements a focused behavior used by this module.
    // FR: displayQuestLine déclare ou implémente un comportement précis utilisé par ce module.
    bool isPersonalNpcQuest(const Quest& quest)
    {
        return !quest.guildQuest;
    }

    std::string questKindText(const Quest& quest)
    {
        if (quest.guildChallenge) return "Défi temporaire de guilde";
        if (quest.origin == "Défi du Hero Villager") return "Défi héroïque à validation directe";
        return quest.guildQuest ? "Contrat officiel de guilde" : "Demande informelle de PNJ";
    }


    bool questContainsText(const Quest& quest, const std::string& needle)
    {
        const std::string combined = quest.location + " " + quest.targetFamily + " " + quest.objective + " " + quest.title + " " + quest.objectiveType;
        return toLowerChoiceText(combined).find(toLowerChoiceText(needle)) != std::string::npos;
    }

    bool questContainsAnyText(const Quest& quest, const std::vector<std::string>& needles)
    {
        for (const std::string& needle : needles)
        {
            if (questContainsText(quest, needle))
            {
                return true;
            }
        }

        return false;
    }

    int suggestedActiveQuestDeadlineDays(const Quest& quest)
    {
        if (quest.guildChallenge)
        {
            // Jour d'acceptation + jour suivant : deux journées jouables au total.
            return 1;
        }

        const int power = rankPowerForQuestReward(quest.rank);
        int days = -1;

        const bool directService = quest.objectiveType == "service";
        const bool materialDelivery = isMaterialDeliveryQuest(quest);
        const bool deliveryLike = quest.objectiveType == "livraison"
            || questContainsAnyText(quest, {"livraison", "colis", "lettre", "message", "commande", "stock", "réassort", "reassort", "caisse", "caisses", "caravane", "chariot", "route", "auberge", "pass"});
        const bool urgentLike = questContainsAnyText(quest, {"urgent", "urgence", "aube", "retard", "au plus vite", "pressé", "presse", "taxe", "registre", "facture", "paperasse", "comptoir", "service"});
        const bool protectionLike = quest.objectiveType == "combat"
            && questContainsAnyText(quest, {"protéger", "proteger", "défense", "defense", "ferme", "village", "route", "menace", "peur", "convoi"});

        if (directService)
        {
            days = 4 + std::min(power, 5);
        }

        if (materialDelivery)
        {
            days = std::max(days, 13 + std::min(power, 8));
        }

        if (deliveryLike)
        {
            days = std::max(days, 11 + std::min(power, 8));
        }

        if (urgentLike)
        {
            days = std::max(days, 6 + std::min(power, 6));
        }

        if (protectionLike)
        {
            days = std::max(days, 11 + std::min(power, 8));
        }

        if (days < 0)
        {
            return -1;
        }

        // FR: On ajoute un vrai coussin, car une journée vaut 5 moments et les déplacements de biome peuvent manger du temps.
        // EN: Real buffer because a day has 5 moments and biome travel can consume time.
        if (quest.objectiveType == "exploration" || quest.objectiveType == "bestiaire")
        {
            days += 4;
        }
        else if (quest.objectiveType == "combat")
        {
            days += 3;
        }

        if (quest.target > 1)
        {
            days += std::min(4, quest.target - 1);
        }

        if (materialDelivery && quest.requiredMaterialQuantity > 1)
        {
            days += std::min(4, quest.requiredMaterialQuantity / 2);
        }

        if (power >= 10)
        {
            days += 4;
        }

        const bool travelHeavy = questContainsAnyText(quest, {"loin", "lointain", "éloigné", "eloigne", "inter-ville", "inter-paliers", "paliers", "caravane", "relais", "biome", "route", "trajet"});
        if (travelHeavy)
        {
            days += 2;
        }

        return std::clamp(days, 4, 36);
    }

    void prepareQuestForAcceptance(Quest& quest, int currentDay)
    {
        if (currentDay < 0)
        {
            currentDay = 0;
        }

        quest.accepted = true;
        quest.failed = false;
        quest.failureReason.clear();
        quest.availableFromDay = currentDay;

        const int deadlineDays = suggestedActiveQuestDeadlineDays(quest);
        quest.expiresAtDay = deadlineDays > 0 ? currentDay + deadlineDays : -1;
    }

    std::string activeQuestDeadlineStatusText(const Quest& quest, int currentDay)
    {
        if (quest.expiresAtDay < 0 || quest.turnedIn || quest.failed)
        {
            return "";
        }

        if (quest.completed)
        {
            return "Délai respecté";
        }

        const int remaining = quest.expiresAtDay - currentDay;
        if (remaining > 1)
        {
            return "Délai : " + std::to_string(remaining) + " jours restants";
        }
        if (remaining == 1)
        {
            return "Délai : dernier jour après celui-ci";
        }
        if (remaining == 0)
        {
            return "Délai : dernier jour";
        }

        return "Délai dépassé";
    }

    std::string activeQuestDeadlineDetailLine(const Quest& quest, int currentDay)
    {
        if (quest.failed)
        {
            return quest.failureReason.empty()
                ? "Délai : échoué, demande archivée sans validation."
                : quest.failureReason;
        }

        if (quest.expiresAtDay < 0)
        {
            return "";
        }

        const std::string status = activeQuestDeadlineStatusText(quest, currentDay);
        return status.empty()
            ? "Date limite : jour " + std::to_string(quest.expiresAtDay) + "."
            : status + " | Date limite : fin du jour " + std::to_string(quest.expiresAtDay) + ".";
    }

    std::string offeredQuestDeadlineLine(const Quest& quest, int currentDay)
    {
        const int days = suggestedActiveQuestDeadlineDays(quest);
        if (days <= 0)
        {
            return "";
        }

        const int deadlineDay = std::max(0, currentDay) + days;
        return "Délai si accepté : à terminer avant la fin du jour "
            + std::to_string(deadlineDay)
            + " (environ " + std::to_string(days) + " jour"
            + (days > 1 ? "s" : "") + ", journée en 5 moments + déplacements possibles).";
    }

    void appendDeadlineLine(std::vector<std::string>& lines, const Quest& quest, int currentDay)
    {
        const std::string deadlineLine = activeQuestDeadlineDetailLine(quest, currentDay);
        if (!deadlineLine.empty())
        {
            lines.push_back(deadlineLine);
        }
    }

    void applySoftServiceFailureCost(Player& player, Quest& quest, Random& random, std::vector<std::string>& lines)
    {
        const int dayBefore = player.getWorldDaysElapsed();
        const int unitBefore = player.getWorldDayProgressUnits();
        player.advanceWorldDayUnits(1);
        lines.push_back("Temps perdu : +1 segment de journée pour recompter, corriger ou faire tamponner à nouveau.");
        lines.push_back(player.formatWorldTimeChange(dayBefore, unitBefore));
        std::vector<std::string> timeReportLines = player.consumeWorldTimeReportLines();
        lines.insert(lines.end(), timeReportLines.begin(), timeReportLines.end());

        player.getInventory().addMaterial(MaterialCatalog::createById("local_service_warning", 1));
        lines.push_back("Réputation locale : une petite note d'incident est ajoutée au dossier de ville. Ce n'est pas une sanction de guilde, mais les guichets s'en souviennent un peu.");

        const int setbackRoll = random.between(1, 100);
        const bool canReduceReward = quest.rewardGold > 3;
        const bool canLoseProgress = quest.progress > 0 && quest.target > 1;

        if (canLoseProgress && setbackRoll <= 20)
        {
            quest.progress = std::max(0, quest.progress - 1);
            quest.completed = false;
            lines.push_back("Conséquence : une étape déjà préparée doit être reprise, le client ne valide pas la version brouillée.");
            lines.push_back("Progression ajustée : " + std::to_string(quest.progress) + "/" + std::to_string(quest.target) + ".");
        }
        else if (canReduceReward && setbackRoll <= 55)
        {
            const int oldReward = quest.rewardGold;
            quest.rewardGold = std::max(1, quest.rewardGold * 90 / 100);
            if (quest.rewardGold < oldReward)
            {
                lines.push_back("Conséquence : le contact réduit un peu la prime probable pour le temps perdu.");
                lines.push_back("Prime ajustée : " + Money::formatGoldWithRaw(oldReward) + " -> " + Money::formatGoldWithRaw(quest.rewardGold) + ".");
            }
            else
            {
                lines.push_back("Conséquence douce : pas de perte directe, mais le contact note que le service a dû être repris.");
            }
        }
        else
        {
            lines.push_back("Conséquence douce : pas de perte d'or ni d'objet, mais le contact demandera une réponse plus propre au prochain essai.");
        }

        const int expired = player.getQuestLog().expireOverdueQuests(player.getWorldDaysElapsed());
        if (expired > 0)
        {
            lines.push_back(std::to_string(expired) + " quête" + (expired > 1 ? "s" : "")
                + " vient d'être archivée pour délai dépassé pendant ce retard.");
        }
    }

    void expireOverdueQuestDeadlines(Player& player, const std::string& screenId, bool notify = true)
    {
        const int expired = player.getQuestLog().expireOverdueQuests(player.getWorldDaysElapsed());
        if (expired <= 0 || !notify)
        {
            return;
        }

        MessageScreen::show(
            "DÉLAI DÉPASSÉ",
            screenId + ".deadline_expired",
            {
                std::to_string(expired) + " quête" + (expired > 1 ? "s" : "") + " vient d'être archivée pour délai dépassé.",
                "Les délais restent volontairement généreux, mais les commandes urgentes, services de comptoir et livraisons ne peuvent pas attendre indéfiniment.",
                "Tu peux les retrouver dans le journal, filtre : rendues / archivées."
            }
        );
    }

    std::string questPlayableLocationHint(const Quest& quest)
    {
        if (quest.objectiveType == "service")
        {
            return quest.guildQuest ? "Guilde > Traiter un service de guilde" : "Retourner voir le contact concerné";
        }

        if (quest.objectiveType == "combat")
        {
            if (questContainsText(quest, "humano") || questContainsText(quest, "embuscade") || questContainsText(quest, "route")) return "Route commerciale ou combat contre humanoïdes";
            if (questContainsText(quest, "mort") || questContainsText(quest, "ombre") || questContainsText(quest, "os")) return "Cimetière oublié / Ruines effondrées";
            if (questContainsText(quest, "mini-boss") || questContainsText(quest, "menace") || questContainsText(quest, "élite") || questContainsText(quest, "elite")) return "Exploration audacieuse ou combat adapté au niveau";
            return "Combats PvE classiques contre la famille indiquée";
        }

        if (quest.objectiveType == "livraison" && !quest.requiredMaterialId.empty())
        {
            return "Explorer/récupérer le matériau demandé, puis revenir au contact";
        }

        if (quest.objectiveType == "livraison")
        {
            return "Exploration > Route commerciale";
        }

        if (quest.objectiveType == "exploration" || quest.objectiveType == "bestiaire")
        {
            if (questContainsText(quest, "lanterne") || questContainsText(quest, "mycélium") || questContainsText(quest, "mycelium") || questContainsText(quest, "résine") || questContainsText(quest, "resine")) return "Exploration > Bocage aux lanternes";
            if (questContainsText(quest, "cloche") || questContainsText(quest, "sanctuaire") || questContainsText(quest, "serment") || questContainsText(quest, "temple")) return "Exploration > Temple des cloches fendues";
            if (questContainsText(quest, "brume bleue") || questContainsText(quest, "canaux") || questContainsText(quest, "barque") || questContainsText(quest, "roseau")) return "Exploration > Canaux de brume bleue";
            if (questContainsText(quest, "craie") || questContainsText(quest, "carrière") || questContainsText(quest, "carriere") || questContainsText(quest, "géant") || questContainsText(quest, "geant")) return "Exploration > Carrière des os blancs";
            if (questContainsText(quest, "pont") || questContainsText(quest, "contreband") || questContainsText(quest, "dette") || questContainsText(quest, "jeton")) return "Exploration > Marché sous les ponts";
            if (questContainsText(quest, "statue") || questContainsText(quest, "jardin") || questContainsText(quest, "pierre pleureuse") || questContainsText(quest, "rose pétrifiée")) return "Exploration > Jardin des statues qui pleurent";
            if (questContainsText(quest, "argile") || questContainsText(quest, "sel lunaire") || questContainsText(quest, "désert") || questContainsText(quest, "desert")) return "Exploration > Désert d'argile rouge";
            if (questContainsText(quest, "quartier") || questContainsText(quest, "contrat") || questContainsText(quest, "vieilles pièces") || questContainsText(quest, "carte de verre")) return "Exploration > Quartier abandonné";
            if (questContainsText(quest, "mine") || questContainsText(quest, "ressort") || questContainsText(quest, "fer froid") || questContainsText(quest, "clou")) return "Exploration > Mine sifflante";
            if (questContainsText(quest, "ruine") || questContainsText(quest, "relais") || questContainsText(quest, "archive")) return "Exploration > Ruines effondrées";
            if (questContainsText(quest, "cimetière") || questContainsText(quest, "cimetiere") || questContainsText(quest, "mort") || questContainsText(quest, "ombre")) return "Exploration > Cimetière oublié";
            if (questContainsText(quest, "slime") || questContainsText(quest, "gélatine") || questContainsText(quest, "gelatine")) return "Exploration > Mares gélatineuses";
            if (questContainsText(quest, "marais") || questContainsText(quest, "boue") || questContainsText(quest, "noy")) return "Exploration > Marais trouble";
            if (questContainsText(quest, "forêt") || questContainsText(quest, "foret") || questContainsText(quest, "plante")) return "Exploration > Forêt ancienne";
            if (questContainsText(quest, "montagne") || questContainsText(quest, "froid") || questContainsText(quest, "métal") || questContainsText(quest, "metal") || questContainsText(quest, "forge")) return "Exploration > Montagne froide ou Ruines effondrées";
            if (questContainsText(quest, "route") || questContainsText(quest, "livraison") || questContainsText(quest, "village") || questContainsText(quest, "client") || questContainsText(quest, "caisse")) return "Exploration > Route commerciale";
            if (quest.location.empty() || questContainsText(quest, "guilde")) return "Exploration > zone marquée [Objectif de quête probable]";
            return "Exploration > " + quest.location;
        }

        if (quest.location.empty())
        {
            return "À confirmer depuis le journal ou le contact";
        }

        return quest.location;
    }

    std::string questProgressMethodText(const Quest& quest)
    {
        if (quest.objectiveType == "service")
        {
            if (questDialogueContainsAny(quest, {"tri de sac", "inventaire trop", "sac trop"}))
            {
                return "Va à la guilde et choisis Traiter un service de guilde : l'épreuve consiste à trier un inventaire selon poids, valeur, fragilité et utilité.";
            }
            if (questDialogueContainsAny(quest, {"armure mal ajustée", "armure mal ajustee", "sangles", "morphologie"}))
            {
                return "Va à la guilde et choisis Traiter un service de guilde : l'épreuve consiste à adapter l'équipement à la morphologie de la race ou sous-race.";
            }
            return quest.guildQuest
                ? "Va à la guilde et choisis Traiter un service de guilde."
                : "Retourne parler au PNJ concerné pour confirmer le service.";
        }

        if (quest.objectiveType == "combat")
        {
            if (isCountedHuntQuest(quest))
            {
                if (isArtificialHuntQuest(quest))
                {
                    return "Lance des combats contre automates, golems, armures vivantes, statues ou pantins animés : chaque cible artificielle compatible vaincue compte.";
                }
                return "Lance des combats correspondant à la cible/famille indiquée : chaque monstre compatible vaincu compte jusqu'au nombre demandé.";
            }
            return "Lance des combats correspondant à la cible/famille indiquée.";
        }

        if (quest.objectiveType == "exploration" || quest.objectiveType == "bestiaire")
        {
            return "Passe par Exploration et choisis la zone conseillée ; les traces et découvertes font avancer le journal.";
        }

        if (quest.objectiveType == "livraison")
        {
            if (!quest.requiredMaterialId.empty())
            {
                return "Récupère le matériau demandé, puis rends la quête au contact.";
            }

            return "Passe par Exploration dans la zone conseillée ; cette livraison se traite comme une sortie de terrain.";
        }

        return "Suis la cible et le lieu conseillés, puis reviens voir le contact.";
    }

    bool isActiveGuildServiceQuest(const Quest& quest)
    {
        return quest.guildQuest
            && quest.accepted
            && !quest.completed
            && !quest.turnedIn
            && !quest.failed
            && quest.objectiveType == "service";
    }

    int countActiveGuildServiceQuests(const Player& player)
    {
        int count = 0;
        for (const Quest& quest : player.getQuestLog().getQuests())
        {
            if (isActiveGuildServiceQuest(quest))
            {
                count++;
            }
        }
        return count;
    }

    bool isActivePersonalServiceQuestForClient(const Quest& quest, const std::string& clientName)
    {
        return !quest.guildQuest
            && quest.accepted
            && !quest.completed
            && !quest.turnedIn
            && !quest.failed
            && quest.objectiveType == "service"
            && quest.client == clientName;
    }

    int countActivePersonalServiceQuestsForClient(const Player& player, const std::string& clientName)
    {
        int count = 0;
        for (const Quest& quest : player.getQuestLog().getQuests())
        {
            if (isActivePersonalServiceQuestForClient(quest, clientName))
            {
                count++;
            }
        }
        return count;
    }

    std::string approximateQuestRewardText(const Quest& quest)
    {
        if (quest.guildQuest)
        {
            return questRewardText(quest);
        }

        std::string text = "Récompense probable : ";

        const int displayedGold = balancedQuestGold(quest);
        if (displayedGold > 0)
        {
            if (displayedGold < 40) text += "petite compensation";
            else if (displayedGold < 120) text += "paiement correct";
            else text += "prime intéressante";
        }
        else
        {
            text += "surtout de la reconnaissance ou un service";
        }

        if (!quest.rewardMaterialId.empty() && quest.rewardMaterialQuantity > 0)
        {
            text += " + objet ou contact possible";
        }

        const int balancedExperience = balancedQuestExperience(quest);
        if (balancedExperience > 0)
        {
            text += " | Expérience estimée : ";
            if (balancedExperience < 80) text += "faible";
            else if (balancedExperience < 220) text += "moyenne";
            else text += "élevée";
        }

        return text;
    }

    std::string questPotentialRewardText(const Quest& quest)
    {
        if (quest.guildQuest)
        {
            return questRewardText(quest);
        }

        std::string reward = approximateQuestRewardText(quest);
        const std::string prefix = "Récompense probable : ";
        if (reward.rfind(prefix, 0) == 0)
        {
            reward.erase(0, prefix.size());
        }
        return reward;
    }

    std::string questCardLabel(const Quest& quest)
    {
        const std::string marker = quest.turnedIn
            ? " [fait]"
            : (quest.completed ? " [fait - à notifier]" : "");

        std::ostringstream label;
        label << quest.title << marker
              << " | Lieu cible : " << questPlayableLocationHint(quest)
              << " | Récompenses potentielles : " << questPotentialRewardText(quest)
              << " | Avancement : " << quest.progress << "/" << quest.target
              << " (" << questStateText(quest) << ")";
        return label.str();
    }

    std::string questNextActionText(const Quest& quest)
    {
        if (quest.turnedIn)
        {
            return "Quête archivée : aucune action supplémentaire n'est nécessaire.";
        }
        if (quest.completed)
        {
            return quest.guildQuest
                ? "Retourne à la guilde pour rendre le contrat terminé."
                : "Retourne voir " + (quest.client.empty() ? std::string("le contact concerné") : quest.client) + " pour valider la demande.";
        }

        const int target = std::max(1, quest.target);
        const int progress = std::clamp(quest.progress, 0, target);
        const std::vector<std::string> labels = splitQuestStageLabels(quest.stageLabels);
        if (progress < target && progress < static_cast<int>(labels.size()) && !labels[progress].empty())
        {
            return labels[progress] + " — " + questProgressMethodText(quest) + " Lieu conseillé : " + questPlayableLocationHint(quest) + ".";
        }

        return questProgressMethodText(quest) + " Lieu conseillé : " + questPlayableLocationHint(quest) + ".";
    }

    std::vector<std::string> guildQuestDetailLines(const Quest& quest, int currentDay)
    {
        std::vector<std::string> lines;
        lines.push_back("Nature : contrat officiel de guilde");
        lines.push_back("Titre : " + quest.title);
        lines.push_back("Origine : " + quest.origin);
        lines.push_back("Client : " + quest.client);
        lines.push_back("Rang : " + quest.rank);
        lines.push_back("À rendre : Maître de guilde");
        lines.push_back("Zone/action jouable : " + questPlayableLocationHint(quest));
        lines.push_back("Type : " + (quest.objectiveType.empty() ? std::string("général") : quest.objectiveType));
        lines.push_back("Cible : " + (quest.targetFamily.empty() ? std::string("générale") : quest.targetFamily));
        lines.push_back("Objectif : " + quest.objective);
        for (const std::string& trialLine : guildServiceTrialLines(quest))
        {
            lines.push_back(trialLine);
        }
        if (isCountedHuntQuest(quest))
        {
            lines.push_back(countedHuntQuestLine(quest));
            if (isArtificialHuntQuest(quest))
            {
                lines.push_back("Précision : ici, la famille artificielle désigne automates, golems, armures vivantes, statues ou pantins animés. Pas des bâtiments à détruire.");
            }
        }
        lines.push_back("Comment faire : " + questProgressMethodText(quest));
        lines.push_back("Prochaine action : " + questNextActionText(quest));
        lines.push_back("Progression : " + std::to_string(quest.progress) + "/" + std::to_string(quest.target));
        for (const std::string& stepLine : questStepProgressLines(quest))
        {
            lines.push_back(stepLine);
        }
        lines.push_back("État : " + questStateText(quest));
        appendDeadlineLine(lines, quest, currentDay);
        lines.push_back("Récompenses : " + questRewardText(quest));

        if (!quest.requiredMaterialId.empty() && quest.requiredMaterialQuantity > 0)
        {
            lines.push_back("Livraison demandée : " + quest.requiredMaterialName + " x" + std::to_string(quest.requiredMaterialQuantity));
        }

        lines.push_back("Lecture : la guilde a assez cadré ce contrat pour que les informations soient fiables.");
        return lines;
    }

    std::vector<std::string> personalQuestEstimateLines(const Quest& quest, int currentDay)
    {
        std::vector<std::string> lines;
        lines.push_back("Nature : demande informelle de PNJ");
        lines.push_back("Ce n'est pas un contrat officiel : le journal ne peut pas tout certifier.");
        lines.push_back("Contact : " + quest.client);
        lines.push_back("Rang supposé : " + quest.rank);
        lines.push_back("Zone/action probable : " + questPlayableLocationHint(quest));
        lines.push_back("Type supposé : " + (quest.objectiveType.empty() ? std::string("service général") : quest.objectiveType));
        lines.push_back("Objectif rapporté : " + quest.objective);
        lines.push_back("Comment faire : " + questProgressMethodText(quest));
        lines.push_back("Prochaine action : " + questNextActionText(quest));
        lines.push_back("Avancée notée : " + std::to_string(quest.progress) + "/" + std::to_string(quest.target));
        for (const std::string& stepLine : questStepProgressLines(quest))
        {
            lines.push_back(stepLine);
        }
        lines.push_back("État : " + questStateText(quest));
        appendDeadlineLine(lines, quest, currentDay);
        lines.push_back(approximateQuestRewardText(quest));

        if (!quest.requiredMaterialId.empty() && quest.requiredMaterialQuantity > 0)
        {
            lines.push_back("Livraison estimée : " + quest.requiredMaterialName + " x" + std::to_string(quest.requiredMaterialQuantity));
        }

        if (!quest.targetFamily.empty())
        {
            lines.push_back("Supposition du journal : la demande semble liée à " + quest.targetFamily + ".");
        }

        lines.push_back("Conseil : retourne voir le PNJ concerné si tu veux une confirmation plus humaine que ce carnet griffonné.");
        return lines;
    }

    void showQuestDetail(const Player& player, const Quest& quest)
    {
        if (quest.guildQuest)
        {
            MessageScreen::show("INSPECTION DU CONTRAT", "quest.detail.guild", guildQuestDetailLines(quest, player.getWorldDaysElapsed()), true);
            return;
        }

        MessageScreen::show("ESTIMATION DE DEMANDE", "quest.detail.personal_estimate", personalQuestEstimateLines(quest, player.getWorldDaysElapsed()), true);
    }

    void openAcceptedQuestActions(const Player& player, const Quest& quest, const std::string& screenId)
    {
        while (true)
        {
            MenuScreen screen("QUÊTE SÉLECTIONNÉE", screenId);
            screen.addSubtitle(quest.guildQuest ? "Contrat officiel accepté" : "Demande PNJ acceptée");
            screen.addLine("Titre : " + quest.title);
            screen.addLine("Lieu cible : " + questPlayableLocationHint(quest));
            screen.addLine("Récompenses potentielles : " + questPotentialRewardText(quest));
            screen.addLine("Avancement : " + std::to_string(quest.progress) + "/" + std::to_string(quest.target) + " (" + questStateText(quest) + ")");
            screen.addLine("Prochaine action : " + questNextActionText(quest));
            for (const std::string& stepLine : questStepProgressLines(quest))
            {
                screen.addLine(stepLine);
            }

            MenuOptionItemData inspectData;
            inspectData.structured = true;
            inspectData.kind = "quest";
            inspectData.section = quest.guildQuest ? "Contrat accepté" : "Demande acceptée";
            inspectData.actionType = "inspect";
            inspectData.name = quest.title;
            inspectData.status = questStateText(quest);
            inspectData.progress = std::to_string(quest.progress) + "/" + std::to_string(quest.target);
            inspectData.important = true;

            screen.addOption(
                1,
                "Inspecter",
                quest.guildQuest
                    ? "Lire toutes les clauses, l'objectif et les conditions du contrat."
                    : "Relire les informations connues et les estimations du journal.",
                true,
                screenId + ".inspect",
                inspectData
            );
            screen.addBackOption("Retour", screenId + ".back");

            const int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choix invalide.");
            Console::clear();

            if (choice == 0)
            {
                return;
            }

            if (choice == 1)
            {
                showQuestDetail(player, quest);
            }
        }
    }

    std::string questRequiredMaterialStatusLine(const Player& player, const Quest& quest)
    {
        if (quest.requiredMaterialId.empty() || quest.requiredMaterialQuantity <= 0)
        {
            return "";
        }

        const std::string prefix = quest.objectiveType == "service"
            ? "Preuve à présenter au rendu : "
            : "Matériaux à rapporter : ";

        const bool enough = hasRequiredQuestMaterial(player, quest);
        return prefix + quest.requiredMaterialName
            + " x" + std::to_string(quest.requiredMaterialQuantity)
            + " (possédé : " + std::to_string(player.getInventory().countMaterialById(quest.requiredMaterialId))
            + ", équiv. normale : " + std::to_string(player.getInventory().countMaterialQualityPointsById(quest.requiredMaterialId) / 2)
            + ", état : " + (enough ? std::string("prêt") : std::string("manquant"))
            + ")";
    }

    void appendQuestRewardResultLines(std::vector<std::string>& lines, const Quest& quest)
    {
        lines.push_back("Quête validée : " + quest.title);
        lines.push_back("XP gagnée : " + std::to_string(balancedQuestExperience(quest)));

        const int displayedGold = balancedQuestGold(quest);
        if (displayedGold > 0)
        {
            lines.push_back("Argent gagné : " + Money::formatGoldWithRaw(displayedGold));
        }
        else
        {
            lines.push_back("Prime en pièces : aucune");
        }

        if (!quest.rewardMaterialId.empty() && quest.rewardMaterialQuantity > 0)
        {
            lines.push_back("Objet reçu : " + quest.rewardMaterialName + " x" + std::to_string(quest.rewardMaterialQuantity));

            if (quest.rewardMaterialId == "client_recommendation")
            {
                const std::string recommendedClient = extractRecommendedClientName(quest);
                if (!recommendedClient.empty())
                {
                    lines.push_back("Nouveau contact ajouté aux PNJ notables : " + recommendedClient + " [Recommandé par un habitant]");
                }
                else
                {
                    lines.push_back("Un nouveau contact pourra apparaître dans la section des PNJ recommandés.");
                }
            }
        }

        if (!quest.rewardNote.empty())
        {
            lines.push_back(quest.rewardNote);
        }
    }

    enum class QuestJournalFilter
    {
        Active,
        ReadyToTurnIn,
        Guild,
        Personal,
        Combat,
        Exploration,
        Delivery,
        MainTurnedIn,
        TurnedIn
    };

    std::string questJournalFilterTitle(QuestJournalFilter filter)
    {
        switch (filter)
        {
            case QuestJournalFilter::Active: return "Quêtes actives";
            case QuestJournalFilter::ReadyToTurnIn: return "Prêtes à rendre";
            case QuestJournalFilter::Guild: return "Contrats de guilde";
            case QuestJournalFilter::Personal: return "Demandes PNJ";
            case QuestJournalFilter::Combat: return "Objectifs de combat";
            case QuestJournalFilter::Exploration: return "Exploration / bestiaire";
            case QuestJournalFilter::Delivery: return "Livraisons";
            case QuestJournalFilter::MainTurnedIn: return "Principales finies";
            case QuestJournalFilter::TurnedIn: return "Rendues / archivées";
        }

        return "Journal";
    }

    std::string questJournalFilterHint(QuestJournalFilter filter)
    {
        switch (filter)
        {
            case QuestJournalFilter::Active:
                return "Tout ce qui n'est pas encore rendu.";
            case QuestJournalFilter::ReadyToTurnIn:
                return "Quêtes terminées ou livraisons dont les matériaux sont prêts.";
            case QuestJournalFilter::Guild:
                return "Contrats officiels : inspection fiable et cadrée.";
            case QuestJournalFilter::Personal:
                return "Demandes informelles : inspection limitée à des estimations.";
            case QuestJournalFilter::Combat:
                return "Objectifs qui progressent par combat ou chasse.";
            case QuestJournalFilter::Exploration:
                return "Objectifs qui progressent par notes, traces, terrain ou bestiaire.";
            case QuestJournalFilter::Delivery:
                return "Demandes de livraison : matériaux précis, objets ou sorties de terrain liées à un transport.";
            case QuestJournalFilter::MainTurnedIn:
                return "Archive séparée des étapes principales déjà validées dans l'histoire.";
            case QuestJournalFilter::TurnedIn:
                return "Archives des quêtes déjà rendues, hors lecture principale dédiée.";
        }

        return "";
    }

    bool questMatchesJournalFilter(const Player& player, const Quest& quest, QuestJournalFilter filter)
    {
        switch (filter)
        {
            case QuestJournalFilter::Active:
                return !quest.turnedIn && !quest.failed;
            case QuestJournalFilter::ReadyToTurnIn:
                return !quest.turnedIn && !quest.failed && isReadyToTurnIn(player, quest);
            case QuestJournalFilter::Guild:
                return !quest.turnedIn && !quest.failed && quest.guildQuest;
            case QuestJournalFilter::Personal:
                return !quest.turnedIn && !quest.failed && isPersonalNpcQuest(quest);
            case QuestJournalFilter::Combat:
                return !quest.turnedIn && !quest.failed && quest.objectiveType == "combat";
            case QuestJournalFilter::Exploration:
                return !quest.turnedIn && !quest.failed && (quest.objectiveType == "exploration" || quest.objectiveType == "bestiaire" || (quest.objectiveType == "livraison" && !isMaterialDeliveryQuest(quest)));
            case QuestJournalFilter::Delivery:
                return !quest.turnedIn && !quest.failed && (isMaterialDeliveryQuest(quest) || quest.objectiveType == "livraison");
            case QuestJournalFilter::MainTurnedIn:
                return quest.turnedIn && !quest.failed && isMainStoryQuest(quest);
            case QuestJournalFilter::TurnedIn:
                return (quest.turnedIn || quest.failed) && !isMainStoryQuest(quest);
        }

        return false;
    }

    std::vector<const Quest*> collectQuestsForJournalFilter(const Player& player, QuestJournalFilter filter)
    {
        std::vector<const Quest*> filtered;

        for (const Quest& quest : player.getQuestLog().getQuests())
        {
            if (questMatchesJournalFilter(player, quest, filter))
            {
                filtered.push_back(&quest);
            }
        }

        std::stable_sort(filtered.begin(), filtered.end(), [](const Quest* left, const Quest* right) {
            if (left->completed != right->completed)
            {
                return left->completed > right->completed;
            }

            if (left->guildQuest != right->guildQuest)
            {
                return left->guildQuest > right->guildQuest;
            }

            return left->title < right->title;
        });

        return filtered;
    }

    int countQuestsForJournalFilter(const Player& player, QuestJournalFilter filter)
    {
        int count = 0;
        for (const Quest& quest : player.getQuestLog().getQuests())
        {
            if (questMatchesJournalFilter(player, quest, filter))
            {
                ++count;
            }
        }
        return count;
    }

    struct ClientQuestCounts
    {
        int active = 0;
        int ready = 0;
        int turnedIn = 0;
        int total = 0;
    };

    ClientQuestCounts countQuestsForClient(const Player& player, const std::string& clientName)
    {
        ClientQuestCounts counts;

        for (const Quest& quest : player.getQuestLog().getQuests())
        {
            if (quest.client != clientName)
            {
                continue;
            }

            ++counts.total;

            if (quest.turnedIn || quest.failed)
            {
                ++counts.turnedIn;
                continue;
            }

            ++counts.active;

            if (isReadyToTurnIn(player, quest))
            {
                ++counts.ready;
            }
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

        if (counts.turnedIn > 0)
        {
            return "Aucune demande active | Rendues : " + std::to_string(counts.turnedIn);
        }

        return "Aucune demande enregistrée";
    }

    std::string clientQuestHintText(const ClientQuestCounts& counts)
    {
        if (counts.ready > 0)
        {
            return "Une demande peut être rendue ici.";
        }

        if (counts.active > 0)
        {
            return "Des demandes sont encore en cours.";
        }

        return "Aucune demande active avec ce contact.";
    }

    std::vector<std::string> clientAmbientDialogueLines(
        const Player& player,
        const std::string& clientName,
        const ClientQuestCounts& counts
    )
    {
        std::vector<std::string> lines;
        const std::string raceText = toLowerChoiceText(player.getRaceText());
        if ((clientName == "Marchand inquiet" || clientName == "Prunigil le marchand")
            && (raceText.find("semi-renard") != std::string::npos || raceText.find("kitsune") != std::string::npos || raceText.find("gnome") != std::string::npos))
        {
            lines.push_back("Réaction raciale : le marchand surveille tes mots de près, comme s'il savait déjà que tu sais négocier.");
        }
        if ((clientName == "Villageois nerveux" || clientName == "Safa la pisteuse")
            && (raceText.find("semi-loup") != std::string::npos || raceText.find("semi-chien") != std::string::npos))
        {
            lines.push_back("Réaction raciale : on te parle plus volontiers de pistes, d'odeurs et de traces qu'à un aventurier sans flair.");
        }
        if ((clientName == "Sœur Cléria" || clientName == "Père Lior" || clientName == "Noé le sonneur")
            && (raceText.find("vampire") != std::string::npos || raceText.find("tieffelin") != std::string::npos || raceText.find("elfe noir") != std::string::npos))
        {
            lines.push_back("Réaction raciale : le temple reste poli, mais les regards sont plus prudents que chaleureux.");
        }

        if (clientName == "Maître de guilde")
        {
            lines.push_back("La gérante de guilde lève les yeux de son registre avant même que tu t'approches du comptoir.");

            if (counts.ready > 0)
            {
                lines.push_back("Elle tapote une pile de contrats terminés : certains tampons t'attendent déjà.");
            }
            else if (counts.active > 0)
            {
                lines.push_back("Elle ne sourit pas vraiment, mais son regard glisse vers les contrats que tu n'as pas encore fermés.");
            }
            else
            {
                lines.push_back("Elle te rappelle que les petites missions propres valent mieux qu'une grande mort ridicule.");
            }

            return lines;
        }

        if (clientName == "Mira")
        {
            lines.push_back("Mira garde son registre ouvert sur les pages les moins rassurantes.");
            lines.push_back("Elle ne cherche pas un héros gratuit : elle cherche quelqu'un capable d'écouter les besoins avant de courir dehors.");
        }
        else if (clientName == "Orren")
        {
            lines.push_back("Orren suit les routes avec un doigt lourd, comme si chaque pont avait une dette envers lui.");
            lines.push_back("Il ne demande pas d'aller loin. Il demande de revenir avec des repères qui ne mentent pas.");
        }
        else if (clientName == "Lysa")
        {
            lines.push_back("Lysa recompte des bandes propres et des flacons presque vides.");
            lines.push_back("Elle parle doucement, mais ses priorités sont nettes : herbes simples, symptômes notés, blessés vivants.");
        }
        else if (clientName == "Bram")
        {
            lines.push_back("Bram garde un marteau à la main même quand il ne frappe plus rien.");
            lines.push_back("Il accepte les promesses, mais seulement quand elles reviennent avec du métal, du cuir ou des outils encore utilisables.");
        }
        else if (clientName == "Soryn")
        {
            lines.push_back("Soryn protège ses archives de la poussière, des rumeurs et des aventuriers trop pressés.");
            lines.push_back("Il donnera des pistes quand les faits auront assez de poids pour ne pas devenir une légende idiote.");
            if (player.hasTitle("Triplement maudit") || player.hasTitle("Le quatrième problème"))
            {
                lines.push_back("En voyant tes marques surnaturelles, Soryn ouvre directement le registre des malédictions multiples : il ne plaisante plus sur les coïncidences.");
            }
            if (player.hasTitle("Témoin du marchand bleu"))
            {
                lines.push_back("Ton témoignage sur le marchand en armure bleue reste classé comme légende confirmée par une seule source étonnamment cohérente.");
            }
        }
        else if (clientName == "Nell la messagère")
        {
            lines.push_back("Nell garde sa sacoche contre elle comme si une mauvaise route pouvait encore la lui arracher.");
            if (player.hasStoryModeStarted() && player.getStoryChapter() >= 2 && player.getStoryStep() >= 8)
            {
                lines.push_back("Elle parle maintenant de bons de convoi, de routes courtes et d'encre froide avec assez de précision pour aider la ville.");
            }
            else
            {
                lines.push_back("Elle n'est pas seulement une survivante : elle devient la première voix capable de confirmer ce que le relais a entendu.");
            }
        }
        else if (clientName == "Forgeron")
        {
            lines.push_back("Le forgeron frappe le métal une dernière fois avant de parler, comme si la phrase devait aussi être trempée.");
            lines.push_back(player.getLevel() >= 8
                ? "À ton niveau, il ne demande plus seulement du fer : il veut des preuves que la route n'a pas menti."
                : "Il commence par une demande simple, mais ses yeux vérifient déjà l'état de ton équipement.");
        }
        else if (clientName == "Alchimiste")
        {
            lines.push_back("L'alchimiste tient un flacon trop coloré pour être entièrement rassurant.");
            lines.push_back("Elle promet que cette fois, la fumée devrait rester dans le récipient. Le mot 'devrait' fait tout le travail.");
        }
        else if (clientName == "Villageois nerveux")
        {
            lines.push_back("Le villageois regarde derrière lui avant chaque phrase.");
            lines.push_back("Il ne sait pas nommer la menace, mais il sait très bien ce que ça fait quand les volets restent fermés trop tôt.");
        }
        else if (clientName == "Marchand inquiet")
        {
            lines.push_back("Le marchand protège sa bourse d'une main et sa dignité de l'autre.");
            lines.push_back("Il insiste sur le fait qu'il n'a pas peur, seulement une relation très prudente avec les routes commerciales.");
        }
        else if (clientName == "Vendeur de composants")
        {
            lines.push_back("Le vendeur de composants désigne des bocaux où certaines choses bougent encore un peu.");
            lines.push_back("Il cherche des restes propres, pas une soupe héroïque impossible à identifier.");
        }
        else if (clientName == "Vendeur de matériaux")
        {
            lines.push_back("Le vendeur de matériaux passe un doigt sur une étagère presque vide.");
            lines.push_back("Il préfère payer une bonne pierre aujourd'hui plutôt qu'expliquer demain pourquoi tout l'atelier attend un miracle.");
        }
        else if (clientName == "Herboriste")
        {
            lines.push_back("L'herboriste parle doucement, mais ses ciseaux claquent avec une précision inquiétante.");
            lines.push_back("Elle veut des plantes intactes, pas des souvenirs verts collés au fond du sac.");
        }
        else if (clientName == "Armurier")
        {
            lines.push_back("L'armurier examine une cuirasse cabossée et soupire comme si le métal l'avait personnellement déçu.");
            lines.push_back("Il ne cherche pas seulement de quoi réparer : il cherche de quoi éviter que le prochain porteur revienne en pièces.");
        }
        else if (clientName == "Vendeur d'armes")
        {
            lines.push_back("Le vendeur d'armes aligne ses lames par taille, par prix, puis par mauvaise idée potentielle.");
            lines.push_back("Il demande des ressources capables de tenir un vrai choc, pas juste de briller sous la lampe.");
        }
        else if (clientName == "Vendeur de consommables")
        {
            lines.push_back("Le vendeur de consommables recompte des flacons scellés d'un air trop sérieux pour un simple marchand.");
            lines.push_back("Il rappelle qu'une potion vide au mauvais moment ressemble beaucoup à un dernier regret.");
        }
        else if (clientName == "Bibliothécaire")
        {
            lines.push_back("La bibliothécaire referme un livre épais en gardant un doigt sur la page, comme si le savoir pouvait s'enfuir.");
            lines.push_back("Elle ne promet pas une vérité complète, seulement assez de traces pour reconnaître un mensonge plus tard.");
        }
        else if (clientName == "Mila des lanternes" || clientName == "Orvan le récolteur de spores" || clientName == "Lysandre aux fioles claires")
        {
            lines.push_back(clientName + " baisse la voix : dans le bocage, même les lumières semblent écouter.");
            lines.push_back("Il veut une récolte propre, parce qu'une lanterne abîmée attire souvent quelque chose de plus grand.");
        }
        else if (clientName == "Safa la pisteuse" || clientName == "Boro le potier" || clientName == "Nelia du sel froid")
        {
            lines.push_back(clientName + " secoue la poussière rouge de ses manches avant de parler.");
            lines.push_back("Le désert d'argile paie bien ceux qui lisent les traces avant de courir après les fausses oasis.");
        }
        else if (clientName == "Maître Hulan" || clientName == "Rika des clés" || clientName == "Tomo le veilleur de rue")
        {
            lines.push_back(clientName + " garde une clé ancienne entre deux doigts, sans dire quelle porte elle ouvre.");
            lines.push_back("Le quartier abandonné a trop de maisons vides pour être honnête, et trop de contrats pour être mort.");
        }
        else if (clientName == "Bram le foreur" || clientName == "Sœur Elga" || clientName == "Pip l'engreneur")
        {
            lines.push_back(clientName + " parle avec une oreille tournée vers le sol, comme si la mine répondait.");
            lines.push_back("Dans la mine sifflante, les pièces utiles se trouvent souvent juste avant les bruits inquiétants.");
        }
        else if (clientName == "Sœur Cléria" || clientName == "Père Lior" || clientName == "Noé le sonneur")
        {
            lines.push_back(clientName + " garde une main près d'une cloche fendue, comme si elle pouvait encore dénoncer les menteurs.");
            lines.push_back("Le temple paie les preuves propres, mais déteste les rapports écrits comme des excuses.");
        }
        else if (clientName == "Batia des barques" || clientName == "Malo du quai bleu" || clientName == "Ysée la brumeuse")
        {
            lines.push_back(clientName + " sent la pluie froide et les cordes humides des canaux.");
            lines.push_back("Les canaux de brume bleue cachent les bons raccourcis, les mauvaises dettes et les barques qui reviennent seules.");
        }
        else if (clientName == "Tarek le carrier" || clientName == "Blanche des fossiles" || clientName == "Gorin au marteau pâle")
        {
            lines.push_back(clientName + " laisse de la craie blanche sur le comptoir en posant sa demande.");
            lines.push_back("La carrière a l'air vide, mais les empreintes trop grandes y sont rarement décoratives.");
        }
        else if (clientName == "Niko sous le pont" || clientName == "Vera aux dettes" || clientName == "Gilda la troqueuse")
        {
            lines.push_back(clientName + " sourit comme quelqu'un qui connaît le prix d'un silence et le revend plus cher.");
            lines.push_back("Au marché sous les ponts, même une petite commission peut finir avec trois témoins et zéro facture.");
        }
        else if (clientName == "Rosalie des statues" || clientName == "Ilan le jardinier muet" || clientName == "Dame Séraphine")
        {
            lines.push_back(clientName + " parle doucement, comme si les statues du jardin pouvaient répéter chaque mot.");
            lines.push_back("Le jardin semble noble, mais les roses de pierre ne poussent jamais sans raison.");
        }
        else
        {
            lines.push_back(clientName + " t'accueille avec une prudence qui sent la recommandation récente.");
            lines.push_back("Ce contact n'a pas encore de comptoir officiel, mais son problème semble déjà bien réel.");
        }

        if (player.hasStoryModeStarted() && player.getStoryChapter() >= 3)
        {
            const std::string routeChoice = StoryCampaign::getChapterThreeRouteChoice(player);
            const std::string convoyDecision = StoryCampaign::getChapterThreeConvoyDecision(player);

            if (clientName == "Mira")
            {
                if (routeChoice == "commerce") lines.push_back("Conséquence : Mira réserve maintenant des gardes aux cargaisons utiles, mais exige une liste avant chaque entrée.");
                else if (routeChoice == "secours") lines.push_back("Conséquence : Mira fait libérer les passages courts pour les blessés avant les chariots marchands.");
                else if (routeChoice == "recherche") lines.push_back("Conséquence : Mira accepte que des enquêteurs occupent une partie des réserves, tant que chaque preuve est numérotée.");

                if (convoyDecision == "marchandises") lines.push_back("Le convoi admis nourrit les étals, mais Mira fait vérifier chaque caisse revenue sans témoin.");
                else if (convoyDecision == "preuves") lines.push_back("Le convoi ne décharge que des preuves : la ville manque encore de marchandises, mais comprend mieux ce qui la menace.");
                else if (convoyDecision == "quarantaine") lines.push_back("Le convoi reste isolé hors des murs. Mira préfère un manque temporaire à une ville contaminée.");
            }
            else if (clientName == "Orren")
            {
                if (routeChoice == "commerce") lines.push_back("Orren râle contre les roues trop nombreuses, mais les nouvelles escortes lui donnent enfin des témoins réguliers.");
                else if (routeChoice == "secours") lines.push_back("Orren marque les abris et les demi-tours sûrs plutôt que les raccourcis rentables.");
                else if (routeChoice == "recherche") lines.push_back("Orren conserve désormais deux mesures pour chaque borne : celle de la route, et celle que la route prétend avoir.");
            }
            else if (clientName == "Lysa")
            {
                if (routeChoice == "secours") lines.push_back("Lysa reçoit plus vite les plantes et les blessés ; ses demandes portent désormais sur les cas que la route a modifiés.");
                else if (convoyDecision == "quarantaine") lines.push_back("Lysa supervise les signes suspects du convoi isolé et refuse qu'un symptôme soit appelé fatigue sans examen.");
                else if (convoyDecision == "marchandises") lines.push_back("Les caisses admises améliorent ses stocks, mais elle met de côté tout flacon dont l'étiquette ne correspond pas au départ déclaré.");
            }
            else if (clientName == "Bram")
            {
                if (routeChoice == "commerce") lines.push_back("Bram reçoit davantage de métal et de cuir. Il répare plus vite, tout en marquant les pièces revenues avec un poids impossible.");
                else if (convoyDecision == "quarantaine") lines.push_back("Bram ne touche pas encore la cargaison isolée ; il prépare des pinces et des caisses sacrificielles pour l'ouvrir sans exposer l'atelier.");
            }
            else if (clientName == "Soryn")
            {
                if (routeChoice == "recherche") lines.push_back("Soryn dispose enfin d'une route dédiée aux archives et aux prélèvements. Il devient presque agréable, ce qui reste inquiétant.");
                if (convoyDecision == "preuves") lines.push_back("Les preuves du convoi occupent une table entière : dates incompatibles, sceaux corrects et poussière venue d'un lieu absent.");
                else if (convoyDecision == "marchandises") lines.push_back("Soryn prélève un échantillon sur chaque cargaison avant que les marchands ne dispersent les indices.");
            }
            else if (clientName == "Nell la messagère")
            {
                if (routeChoice == "secours") lines.push_back("Nell transporte d'abord les appels de détresse et les listes de blessés, même quand les commerçants protestent.");
                else if (routeChoice == "commerce") lines.push_back("Nell accompagne les premiers convois réguliers et note les écarts de trajet au lieu de faire confiance aux horaires.");
                else if (routeChoice == "recherche") lines.push_back("Nell porte des enveloppes scellées entre Soryn, Eda et les équipes de terrain ; aucune copie ne voyage seule.");
            }
            else if (clientName == "Eda")
            {
                if (convoyDecision == "marchandises") lines.push_back("Eda recompte les stocks admis et isole tout surplus que personne n'a déclaré au départ.");
                else if (convoyDecision == "preuves") lines.push_back("Eda ne comptabilise plus seulement les caisses : elle comptabilise les contradictions entre les caisses.");
                else if (convoyDecision == "quarantaine") lines.push_back("Eda tient deux inventaires séparés, l'un pour la ville et l'autre pour ce qui attend encore derrière les barrières.");
            }
        }

        if (counts.ready > 0)
        {
            lines.push_back("Le contact remarque aussi que tu as déjà quelque chose à rendre ici.");
        }
        else if (counts.active > 0)
        {
            lines.push_back("Il garde un oeil sur les demandes en cours, sans presser plus que nécessaire.");
        }

        return lines;
    }

    struct ReadyQuestClientEntry
    {
        std::string clientName;
        int readyCount = 0;
        int guildReadyCount = 0;
        int personalReadyCount = 0;
        std::string firstTitle;
        std::string firstReward;
    };

    std::vector<ReadyQuestClientEntry> collectReadyQuestClients(const Player& player)
    {
        std::vector<ReadyQuestClientEntry> entries;

        for (const Quest& quest : player.getQuestLog().getQuests())
        {
            if (quest.turnedIn || quest.failed || !isReadyToTurnIn(player, quest))
            {
                continue;
            }

            auto it = std::find_if(entries.begin(), entries.end(), [&quest](const ReadyQuestClientEntry& entry) {
                return entry.clientName == quest.client;
            });

            if (it == entries.end())
            {
                ReadyQuestClientEntry entry;
                entry.clientName = quest.client;
                entry.firstTitle = quest.title;
                entry.firstReward = quest.guildQuest ? questRewardText(quest) : approximateQuestRewardText(quest);
                entries.push_back(entry);
                it = entries.end() - 1;
            }

            ++it->readyCount;
            if (quest.guildQuest)
            {
                ++it->guildReadyCount;
            }
            else
            {
                ++it->personalReadyCount;
            }
        }

        std::stable_sort(entries.begin(), entries.end(), [](const ReadyQuestClientEntry& left, const ReadyQuestClientEntry& right) {
            if (left.guildReadyCount != right.guildReadyCount)
            {
                return left.guildReadyCount > right.guildReadyCount;
            }

            if (left.readyCount != right.readyCount)
            {
                return left.readyCount > right.readyCount;
            }

            return left.clientName < right.clientName;
        });

        return entries;
    }

    std::string readyQuestClientStatusText(const ReadyQuestClientEntry& entry)
    {
        std::string status = "Prêtes : " + std::to_string(entry.readyCount);

        if (entry.guildReadyCount > 0)
        {
            status += " | Guilde : " + std::to_string(entry.guildReadyCount);
        }

        if (entry.personalReadyCount > 0)
        {
            status += " | PNJ : " + std::to_string(entry.personalReadyCount);
        }

        return status;
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
        itemData.progress = counts.active > 0
            ? "Demandes actives " + std::to_string(counts.active)
            : "Aucune demande active";
        itemData.owner = clientName;
        itemData.important = counts.ready > 0;
        return itemData;
    }

    void addClientQuestSummaryLines(MenuScreen& screen, const Player& player, const std::string& clientName)
    {
        const ClientQuestCounts counts = countQuestsForClient(player, clientName);
        screen.addLine("Contact : " + clientName);
        screen.addLine("Demandes de ce contact : " + clientQuestStatusText(counts));

        if (counts.ready > 0)
        {
            screen.addLine("Priorité : une demande peut être rendue ici avant de repartir chercher autre chose.");
        }
        else if (counts.active > 0)
        {
            screen.addLine("Note : les informations PNJ restent des pourparlers, pas des contrats officiels de guilde.");
        }
        else
        {
            screen.addLine("Ce contact n'a pas de demande active dans ton journal pour le moment.");
        }
    }

    void showClientQuestOverview(const Player& player, const std::string& clientName)
    {
        constexpr std::size_t questsPerPage = 5;
        std::size_t pageIndex = 0;

        while (true)
        {
            std::vector<const Quest*> relatedQuests;
            for (const Quest& quest : player.getQuestLog().getQuests())
            {
                if (quest.client == clientName)
                {
                    relatedQuests.push_back(&quest);
                }
            }

            std::stable_sort(relatedQuests.begin(), relatedQuests.end(), [&player](const Quest* left, const Quest* right) {
                const bool leftReady = !left->turnedIn && !left->failed && isReadyToTurnIn(player, *left);
                const bool rightReady = !right->turnedIn && !right->failed && isReadyToTurnIn(player, *right);

                if (leftReady != rightReady)
                {
                    return leftReady > rightReady;
                }

                if (left->turnedIn != right->turnedIn)
                {
                    return left->turnedIn < right->turnedIn;
                }

                return left->title < right->title;
            });

            const std::size_t totalPages = PagedMenu::pageCount(relatedQuests.size(), questsPerPage);
            if (pageIndex >= totalPages)
            {
                pageIndex = totalPages == 0 ? 0 : totalPages - 1;
            }

            const std::size_t first = PagedMenu::firstIndex(pageIndex, questsPerPage);
            const std::size_t last = PagedMenu::lastIndexExclusive(relatedQuests.size(), pageIndex, questsPerPage);

            MenuScreen screen("DEMANDES DU CONTACT", "quest.client.overview");
            screen.addSubtitle(clientName);
            addClientQuestSummaryLines(screen, player, clientName);
            screen.addLine("Affichage : " + PagedMenu::rangeText(first, last, relatedQuests.size()));
            screen.addBackOption("Retour", "quest.client.overview.back");

            if (relatedQuests.empty())
            {
                screen.addLine("Aucune demande connue avec ce contact.");
            }
            else
            {
                for (std::size_t i = first; i < last; ++i)
                {
                    const Quest& quest = *relatedQuests[i];
                    const std::string questLabel = questCardLabel(quest);
                    MenuOptionItemData itemData;
                    itemData.structured = true;
                    itemData.kind = "quest";
                    itemData.section = clientName;
                    itemData.actionType = quest.guildQuest ? "inspect_contract" : "estimate_request";
                    itemData.name = quest.title;
                    itemData.detail = "";
                    itemData.status = isReadyToTurnIn(player, quest)
            ? (quest.guildQuest ? "Prête à rendre - contrat officiel" : "Prête à confirmer - demande PNJ")
            : (quest.guildQuest ? questStateText(quest) : questStateText(quest) + " - informations estimées");
                    const std::string statusDeadline = activeQuestDeadlineStatusText(quest, player.getWorldDaysElapsed());
                    if (!statusDeadline.empty())
                    {
                        itemData.status += " | " + statusDeadline;
                    }
                    itemData.reward = quest.guildQuest ? questRewardText(quest) : approximateQuestRewardText(quest);
                    itemData.progress = std::to_string(quest.progress) + "/" + std::to_string(quest.target);
                    itemData.owner = quest.client;
                    itemData.important = !quest.turnedIn && !quest.failed && isReadyToTurnIn(player, quest);

                    screen.addOption(
                        static_cast<int>(10 + (i - first)),
                        questLabel,
                        "",
                        true,
                        quest.guildQuest
                            ? "quest.client.overview.inspect.guild." + std::to_string(i)
                            : "quest.client.overview.estimate.personal." + std::to_string(i),
                        itemData
                    );
                }
            }

            PagedMenu::addNavigationOptions(screen, pageIndex, totalPages);

            int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choix invalide.");
            Console::clear();

            if (choice == 0)
            {
                return;
            }

            if (choice == 98 && pageIndex > 0)
            {
                --pageIndex;
                continue;
            }

            if (choice == 99 && pageIndex + 1 < totalPages)
            {
                ++pageIndex;
                continue;
            }

            const int localQuestIndex = choice - 10;
            if (localQuestIndex >= 0 && first + static_cast<std::size_t>(localQuestIndex) < last)
            {
                openAcceptedQuestActions(
                    player,
                    *relatedQuests[first + static_cast<std::size_t>(localQuestIndex)],
                    "quest.client.overview.selected"
                );
                continue;
            }

            MessageScreen::show(
                "ACTION INDISPONIBLE",
                "quest.client.overview.invalid",
                {"Ce choix ne correspond à aucune demande de ce contact."}
            );
        }
    }

    int askQuestOfferDecision(
        const std::string& title,
        const std::string& screenId,
        const Player& player,
        const Quest& quest,
        const std::vector<std::string>& introLines
    )
    {
        MenuScreen summaryScreen(title, screenId + ".summary");
        summaryScreen.addSubtitle(quest.guildQuest ? "Contrat officiel disponible" : "Demande informelle disponible");

        for (const std::string& line : introLines)
        {
            summaryScreen.addLine(line);
        }

        summaryScreen.addLine("Titre : " + quest.title);
        summaryScreen.addLine("Lieu cible : " + questPlayableLocationHint(quest));
        summaryScreen.addLine("Récompenses potentielles : " + questPotentialRewardText(quest));
        summaryScreen.addLine("Avancement : " + std::to_string(quest.progress) + "/" + std::to_string(quest.target) + " (non acceptée)");

        MenuOptionItemData informationData;
        informationData.structured = true;
        informationData.kind = "quest";
        informationData.section = quest.guildQuest ? "Contrat disponible" : "Demande disponible";
        informationData.actionType = "inspect";
        informationData.name = quest.title;
        informationData.status = "Informations disponibles";
        informationData.important = true;

        summaryScreen.addOption(
            1,
            "Demander plus d'informations",
            quest.guildQuest
                ? "Consulter les clauses, l'objectif précis et les conditions du contrat."
                : "Questionner le PNJ avant de décider.",
            true,
            screenId + ".request_information",
            informationData
        );
        summaryScreen.addOption(
            0,
            quest.guildQuest ? "Laisser de côté" : "Refuser la demande",
            quest.guildQuest
                ? "Reposer ce contrat sur le panneau sans l'accepter."
                : "Refuser immédiatement, sans demander davantage d'explications.",
            true,
            screenId + ".decline_before_information"
        );

        const int summaryChoice = TerminalInterface::askMenuChoiceFromOptions(summaryScreen, "Choix invalide.");
        Console::clear();
        if (summaryChoice != 1)
        {
            return 0;
        }

        MenuScreen detailScreen(title + " — INFORMATIONS", screenId + ".details");
        detailScreen.addSubtitle(quest.guildQuest ? "Contrat officiel" : "Pourparler / demande informelle");
        detailScreen.addLine("Nature : " + questKindText(quest));
        detailScreen.addLine((quest.guildQuest ? "Contrat proposé : [Rang " : "Demande proposée : [Rang estimé ") + quest.rank + "] " + quest.title);
        detailScreen.addLine((quest.guildQuest ? "Client officiel : " : "Contact : ") + quest.client);
        detailScreen.addLine((quest.guildQuest ? "Zone/action jouable : " : "Zone/action probable : ") + questPlayableLocationHint(quest));
        detailScreen.addLine((quest.guildQuest ? "Objectif : " : "Objectif raconté : ") + quest.objective);
        for (const std::string& trialLine : guildServiceTrialLines(quest))
        {
            detailScreen.addLine(trialLine);
        }
        detailScreen.addLine("Comment faire : " + questProgressMethodText(quest));
        detailScreen.addLine((quest.guildQuest ? "Récompenses : " : "Estimation : ") + (quest.guildQuest ? questRewardText(quest) : approximateQuestRewardText(quest)));
        const std::string deadlineLine = offeredQuestDeadlineLine(quest, player.getWorldDaysElapsed());
        if (!deadlineLine.empty())
        {
            detailScreen.addLine(deadlineLine);
        }
        if (!quest.guildQuest)
        {
            detailScreen.addLine("Note : ce PNJ parle de vive voix. Le journal pourra seulement estimer certaines informations.");
        }
        const std::string materialLine = questRequiredMaterialStatusLine(player, quest);
        if (!materialLine.empty())
        {
            detailScreen.addLine(materialLine);
        }

        MenuOptionItemData acceptData;
        acceptData.structured = true;
        acceptData.kind = "quest";
        acceptData.section = quest.guildQuest ? "Contrat officiel" : "Demande informelle";
        acceptData.actionType = "accept";
        acceptData.name = quest.title;
        acceptData.status = quest.guildQuest ? "Disponible" : "Disponible - informations estimées";
        acceptData.reward = questPotentialRewardText(quest);
        acceptData.progress = "0/" + std::to_string(quest.target);
        acceptData.owner = quest.client;
        acceptData.important = true;

        detailScreen.addOption(
            1,
            quest.guildQuest ? "Accepter le contrat" : "Accepter la demande",
            "Ajouter cette entrée au journal.",
            true,
            screenId + ".accept",
            acceptData
        );
        detailScreen.addOption(
            0,
            quest.guildQuest ? "Laisser de côté" : "Refuser la demande",
            quest.guildQuest ? "Reposer le contrat sur le panneau." : "Refuser après avoir entendu les explications.",
            true,
            screenId + ".decline_after_information"
        );

        return TerminalInterface::askMenuChoiceFromOptions(detailScreen, "Choix invalide.");
    }


    bool askQuestTurnInConfirmation(const Player& player, const Quest& quest, const std::string& clientName)
    {
        while (true)
        {
            MenuScreen screen(
                quest.guildQuest ? "VALIDATION DU CONTRAT" : "CONFIRMATION DE DEMANDE",
                quest.guildQuest ? "quest.turn_in.confirm.guild" : "quest.turn_in.confirm.personal"
            );

            screen.addSubtitle(quest.guildQuest ? "Contrat officiel" : "Pourparler / demande informelle");
            screen.addLine(quest.guildQuest
                ? "La guilde peut tamponner ce contrat, mais le choix reste le tien."
                : clientName + " peut confirmer cette demande, sans registre officiel de guilde.");
            screen.addLine("Titre : " + quest.title);
            screen.addLine(quest.guildQuest ? "Client officiel : " + quest.client : "Contact : " + quest.client);
            screen.addLine(quest.guildQuest ? "Zone/action jouable : " + questPlayableLocationHint(quest) : "Zone/action probable : " + questPlayableLocationHint(quest));
            screen.addLine(quest.guildQuest ? "Objectif vérifié : " + quest.objective : "Objectif rapporté : " + quest.objective);
            screen.addLine("Progression : " + std::to_string(quest.progress) + "/" + std::to_string(quest.target));
            screen.addLine(quest.guildQuest
                ? "Récompenses prévues : " + questRewardText(quest)
                : "Ce que le journal estime : " + approximateQuestRewardText(quest));

            const std::string materialLine = questRequiredMaterialStatusLine(player, quest);
            if (!materialLine.empty())
            {
                screen.addLine(materialLine);
            }

            if (!quest.guildQuest)
            {
                screen.addLine("Note : les récompenses exactes ne sont sûres qu'au moment où le contact accepte vraiment le service.");
            }

            MenuOptionItemData confirmData;
            confirmData.structured = true;
            confirmData.kind = "quest";
            confirmData.section = quest.guildQuest ? "Contrat officiel" : "Demande informelle";
            confirmData.actionType = "turn_in";
            confirmData.name = quest.title;
            confirmData.detail = quest.objective;
            confirmData.status = quest.guildQuest ? "Prête à tamponner" : "Prête à confirmer";
            confirmData.reward = quest.guildQuest ? questRewardText(quest) : approximateQuestRewardText(quest);
            confirmData.progress = std::to_string(quest.progress) + "/" + std::to_string(quest.target);
            confirmData.owner = quest.client;
            confirmData.important = true;

            MenuOptionItemData inspectData;
            inspectData.structured = true;
            inspectData.kind = "quest";
            inspectData.section = quest.guildQuest ? "Inspection" : "Estimation";
            inspectData.actionType = quest.guildQuest ? "inspect_contract" : "estimate_request";
            inspectData.name = quest.title;
            inspectData.detail = quest.guildQuest
                ? "Relire les clauses du contrat officiel."
                : "Relire les suppositions du journal sur ce pourparler.";
            inspectData.status = quest.guildQuest ? "Fiable" : "Vague / estimé";
            inspectData.owner = quest.client;

            screen.addOption(
                1,
                quest.guildQuest ? "Valider ce contrat" : "Confirmer cette demande",
                quest.guildQuest ? "Tamponner le contrat et recevoir les récompenses." : "Valider le service auprès du contact.",
                true,
                quest.guildQuest ? "quest.turn_in.confirm.guild.accept" : "quest.turn_in.confirm.personal.accept",
                confirmData
            );
            screen.addOption(
                2,
                quest.guildQuest ? "Inspecter le contrat" : "Inspecter la demande",
                quest.guildQuest ? "Voir les informations officielles du contrat." : "Voir les suppositions et infos vagues du journal.",
                true,
                quest.guildQuest ? "quest.turn_in.confirm.guild.inspect" : "quest.turn_in.confirm.personal.estimate",
                inspectData
            );
            screen.addOption(0, "Retour", "Ne rien rendre pour le moment.", true, "quest.turn_in.confirm.back");

            int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choix invalide.");
            Console::clear();

            if (choice == 1)
            {
                return true;
            }

            if (choice == 0)
            {
                return false;
            }

            if (choice == 2)
            {
                showQuestDetail(player, quest);
            }
        }
    }

    struct ExplorationBiome
    {
        std::string name;
        std::string style;
        std::string commonMaterialId;
        std::string rareMaterialId;
        int minLevel;
        int maxLevel;
        std::string commonMonsters;
        std::string rareMonsters;
    };

    struct ExplorationIntensity
    {
        std::string name;
        std::string description;
        int eventShift;
        int quantityBonus;
        int goldPercent;
        int carefulBonus;
        int durationUnits;
        int guaranteedEvents;
        int extraEventChance;
    };

    bool playerHasExplorationPassive(const Player& player, const std::string& skillId)
    {
        const std::vector<std::string>& skills = player.getUnlockedPassiveSkills();
        return std::find(skills.begin(), skills.end(), skillId) != skills.end();
    }

    int explorationTravelUnitsForBiome(const ExplorationBiome& biome)
    {
        const std::string name = biome.name;
        if (name == "Plaine sauvage" || name == "Route commerciale" || name == "Mares gélatineuses")
        {
            return 0;
        }

        if (name.find("Archipel") != std::string::npos
            || name.find("Cieux") != std::string::npos
            || name.find("Parvis") != std::string::npos
            || name.find("Nid draconique") != std::string::npos
            || name.find("Coulées") != std::string::npos
            || name.find("Glacier") != std::string::npos
            || name.find("Falaises") != std::string::npos
            || name.find("Désert") != std::string::npos
            || name.find("Sanctuaire") != std::string::npos
            || biome.minLevel >= 30)
        {
            if (name.find("Archipel") != std::string::npos
                || name.find("Cieux") != std::string::npos
                || name.find("Nid draconique") != std::string::npos
                || name.find("Confluence") != std::string::npos
                || biome.minLevel >= 50)
            {
                return 3;
            }
            return 2;
        }

        return 1;
    }

    std::string explorationDistanceLabel(int travelUnits)
    {
        if (travelUnits <= 0) return "à côté / déplacement négligeable";
        if (travelUnits == 1) return "trajet proche ou moyen (+1 segment)";
        if (travelUnits == 2) return "trajet éloigné (+2 segments)";
        return "trajet très éloigné (+3 segments)";
    }

    int explorationBiomeSizeUnits(const ExplorationBiome& biome)
    {
        const std::string name = biome.name;

        if (name == "Plaine sauvage" || name == "Route commerciale" || name == "Mares gélatineuses")
        {
            return 0;
        }

        if (name.find("Archipel") != std::string::npos
            || name.find("Cieux") != std::string::npos
            || name.find("Parvis") != std::string::npos
            || name.find("Confluence") != std::string::npos
            || name.find("Glacier") != std::string::npos
            || name.find("Désert") != std::string::npos
            || name.find("Sanctuaire") != std::string::npos
            || name.find("Carrière") != std::string::npos
            || biome.minLevel >= 60)
        {
            return 2;
        }

        if (name.find("Forêt") != std::string::npos
            || name.find("Marais") != std::string::npos
            || name.find("Ruines") != std::string::npos
            || name.find("Mine") != std::string::npos
            || name.find("Falaises") != std::string::npos
            || name.find("Archives") != std::string::npos
            || biome.minLevel >= 12)
        {
            return 1;
        }

        return 0;
    }

    std::string explorationBiomeSizeLabel(int sizeUnits)
    {
        if (sizeUnits <= 0) return "petit / abords faciles (+0 segment)";
        if (sizeUnits == 1) return "zone moyenne ou terrain pénible (+1 segment)";
        return "biome vaste, vertical ou difficile (+2 segments)";
    }

    std::string explorationTemperatureHazard(const ExplorationBiome& biome)
    {
        const std::string name = biome.name;
        if (name.find("Glacier") != std::string::npos
            || name.find("Montagne froide") != std::string::npos
            || name.find("Canaux de brume") != std::string::npos
            || name.find("Mine sifflante") != std::string::npos)
        {
            return "froid";
        }

        if (name.find("Coulées") != std::string::npos
            || name.find("lave") != std::string::npos
            || name.find("basalte") != std::string::npos)
        {
            return "feu";
        }

        if (name.find("Désert") != std::string::npos
            || name.find("argile rouge") != std::string::npos
            || name.find("Neuf Étincelles") != std::string::npos
            || name.find("braises") != std::string::npos)
        {
            return "chaleur";
        }

        return "";
    }

    std::string equippedArmorNameLower(const Player& player)
    {
        if (!player.hasEquippedArmor())
        {
            return "";
        }
        Armor armor = player.getEquippedArmor();
        return toLowerChoiceText(armor.getName() + " " + armor.getDescription() + " " + armor.getEnchantmentSummaryText());
    }

    int equippedTemperatureResistanceScore(const Player& player, const std::string& hazard, std::vector<std::string>& lines)
    {
        const std::string armorName = equippedArmorNameLower(player);
        if (armorName.empty())
        {
            return 0;
        }

        int score = 0;
        if (armorName.find("manteau isolant") != std::string::npos
            || armorName.find("tenue de survie") != std::string::npos
            || armorName.find("rune thermique") != std::string::npos
            || armorName.find("charme d'équilibre thermique") != std::string::npos)
        {
            score = std::max(score, 2);
            lines.push_back("Équipement : tenue de survie équipée, protection générale contre les températures difficiles.");
        }

        if (hazard == "froid"
            && (armorName.find("parka") != std::string::npos
                || armorName.find("glaciale") != std::string::npos
                || armorName.find("mineur") != std::string::npos
                || armorName.find("froid") != std::string::npos
                || armorName.find("rune anti-froid") != std::string::npos))
        {
            score = std::max(score, 3);
            lines.push_back("Équipement : protection contre le froid équipée à la place d'une armure classique.");
        }

        if ((hazard == "chaleur" || hazard == "feu")
            && (armorName.find("ignifug") != std::string::npos
                || armorName.find("braises") != std::string::npos
                || armorName.find("argile") != std::string::npos
                || armorName.find("drake") != std::string::npos
                || armorName.find("rune anti-feu") != std::string::npos))
        {
            score = std::max(score, hazard == "feu" ? 3 : 2);
            lines.push_back("Équipement : protection chaude/ignifugée équipée à la place d'une armure classique.");
        }

        return score;
    }

    bool isExtremeTemperatureBiome(const ExplorationBiome& biome, const std::string& hazard)
    {
        const std::string name = biome.name;
        if (hazard == "feu")
        {
            return name.find("Coulées") != std::string::npos
                || name.find("Nid draconique") != std::string::npos
                || biome.minLevel >= 36;
        }
        if (hazard == "froid")
        {
            return name.find("Glacier") != std::string::npos
                || biome.minLevel >= 36;
        }
        if (hazard == "chaleur")
        {
            return name.find("Désert des Protecteurs") != std::string::npos
                || name.find("Neuf Étincelles") != std::string::npos
                || biome.minLevel >= 34;
        }
        return false;
    }

    int applyTemperatureExplorationRisk(Player& player, const ExplorationBiome& biome, int exposureUnits, std::vector<std::string>& lines)
    {
        const std::string hazard = explorationTemperatureHazard(biome);
        if (hazard.empty())
        {
            return 0;
        }

        const bool extreme = isExtremeTemperatureBiome(biome, hazard);
        const int requiredScore = (hazard == "feu" ? 3 : 2) + (extreme ? 1 : 0);
        int score = 0;

        int racialScore = RaceCatalog::getEnvironmentalTemperatureScore(player.getRace(), hazard);
        if (racialScore > 0)
        {
            score += racialScore;
            lines.push_back("Passif racial : résistance naturelle à cette température (score +" + std::to_string(racialScore) + ").");
        }
        else if (racialScore < 0)
        {
            score += racialScore;
            lines.push_back("Faiblesse raciale : cette température s'accroche plus facilement à ton corps (score " + std::to_string(racialScore) + ").");
        }

        if (playerHasExplorationPassive(player, "temperature_adaptation"))
        {
            score += 1;
            lines.push_back("Passif racial : adaptation de température légère, la zone est moins brutale dès l'arrivée.");
        }

        if ((hazard == "chaleur" || hazard == "feu") && playerHasExplorationPassive(player, "minor_fire_resistance"))
        {
            score += hazard == "feu" ? 1 : 2;
            lines.push_back("Passif racial : résistance légère au feu/chaleur. Elle compte aussi contre les brûlures de combat.");
        }

        if ((hazard == "chaleur" || hazard == "feu") && playerHasExplorationPassive(player, "infernal_fire_resistance"))
        {
            score += hazard == "feu" ? 2 : 3;
            lines.push_back("Passif racial : résistance infernale, suffisante pour beaucoup de zones chaudes non extrêmes.");
        }

        if (hazard == "froid" && playerHasExplorationPassive(player, "minor_cold_resistance"))
        {
            score += 1;
            lines.push_back("Passif racial : résistance légère au froid, utile aussi contre le givre en combat.");
        }

        if (hazard == "froid"
            && player.getInventory().countMaterialById("owned_mount_registration") > 0
            && player.getInventory().countMaterialById("mount_weather_blanket") > 0
            && player.getInventory().countMaterialById("mount_minor_injury_marker") <= 0)
        {
            score += 1;
            lines.push_back("Monture : couverture météo prête. Elle aide à garder un rythme correct contre le froid léger, sans remplacer une vraie tenue.");
        }

        if ((hazard == "chaleur" || hazard == "feu") && playerHasExplorationPassive(player, "fire_vulnerability"))
        {
            score -= 1;
            lines.push_back("Faiblesse aux flammes : les ailes, plumes ou tissus fragiles supportent mal la chaleur.");
        }

        score += equippedTemperatureResistanceScore(player, hazard, lines);

        if (score < requiredScore)
        {
            const std::string backupId = hazard == "froid" ? "thermal_survival_blanket" : "cooling_survival_wrap";
            const std::string backupName = hazard == "froid" ? "Couverture de survie thermique" : "Voile anti-chaleur";
            if (player.getInventory().removeMaterialQuantityById(backupId, 1))
            {
                score += 2;
                lines.push_back(backupName + " consommé(e) : protection de secours utilisée pour cette sortie.");
            }
            else if (player.getInventory().removeMaterialQuantityById("temperature_survival_kit", 1))
            {
                score += 2;
                lines.push_back("Kit de survie thermique consommé : de quoi tenir cette température sans transformer l'exploration en suicide lent.");
            }
        }

        lines.push_back(
            "Température : " + hazard
            + (extreme ? " extrême" : "")
            + " | protection " + std::to_string(score)
            + "/" + std::to_string(requiredScore) + "."
        );

        if (score >= requiredScore)
        {
            lines.push_back("Température : protection suffisante pour ce biome. Les résistances naturelles peuvent remplacer l'équipement en zone normale, mais pas toujours en zone extrême.");
            return hazard == "feu" ? -1 : -2;
        }

        const int deficit = std::max(1, requiredScore - score);
        int rollShift = hazard == "feu" ? 14 : 10;
        rollShift += deficit * 3;
        if (extreme) rollShift += 4;

        int percentPerSegment = hazard == "feu" ? 5 : 3;
        if (hazard == "chaleur") percentPerSegment = 4;
        if (extreme) percentPerSegment += 2;
        percentPerSegment += std::max(0, deficit - 1);

        const int safeExposureUnits = std::max(1, exposureUnits);
        int damagePerSegment = std::max(1, player.getMaxHp() * percentPerSegment / 100);
        int totalDamage = damagePerSegment * safeExposureUnits;
        if (player.getHp() > 1)
        {
            totalDamage = std::min(totalDamage, player.getHp() - 1);
            if (totalDamage > 0)
            {
                player.takeDamage(totalDamage);
                lines.push_back(
                    "Température : dégâts d'exposition " + std::to_string(damagePerSegment)
                    + " x " + std::to_string(safeExposureUnits)
                    + " segment(s) = " + std::to_string(totalDamage)
                    + " PV. PV restants : " + std::to_string(player.getHp())
                    + "/" + std::to_string(player.getMaxHp()) + "."
                );
            }
        }

        if (hazard == "feu")
        {
            ElementalAffinitySystem::applyBurning(player, 2 + deficit, std::max(1, damagePerSegment / 2));
            lines.push_back("Combat : la chaleur laisse une brûlure persistante qui peut agir au début des tours si un combat démarre.");
        }
        else if (hazard == "froid")
        {
            ElementalAffinitySystem::applyFrost(player, 2 + deficit);
            lines.push_back("Combat : le froid raidit les gestes et peut ralentir les prochains tours.");
        }
        else if (hazard == "chaleur")
        {
            player.applyWeakening(2 + deficit, std::min(35, 8 + deficit * 5));
            lines.push_back("Combat : la chaleur fatigue le corps et peut affaiblir les prochains gestes.");
        }

        lines.push_back("Température : protection insuffisante. Une tenue équipée, une couverture adaptée, un kit thermique ou un futur enchantement serait conseillé.");
        return rollShift;
    }

    int reduceExplorationTravelWithPreparation(Player& player, int travelUnits, std::vector<std::string>& lines)
    {
        if (travelUnits <= 0)
        {
            lines.push_back("Distance : zone proche, aucun segment de déplacement ajouté.");
            return 0;
        }

        int reduced = travelUnits;

        if (player.getInventory().countMaterialById("owned_mount_registration") > 0)
        {
            const int fatigue = player.getInventory().countMaterialById("mount_fatigue_marker");
            const int bond = std::min(3, player.getInventory().countMaterialById("mount_bond_marker"));
            const bool hasReinforcedSaddle = player.getInventory().countMaterialById("stable_saddle_upgrade") > 0;
            const bool hasComfortBridle = player.getInventory().countMaterialById("mount_comfort_bridle") > 0;
            const bool hasPackHarness = player.getInventory().countMaterialById("mount_pack_harness") > 0;
            const bool hasRoadShoes = player.getInventory().countMaterialById("mount_road_shoes") > 0;
            const int surefoot = std::min(2, player.getInventory().countMaterialById("mount_surefoot_training_marker"));
            const int routeMemory = std::min(2, player.getInventory().countMaterialById("mount_route_memory_marker"));
            const bool hasMinorInjury = player.getInventory().countMaterialById("mount_minor_injury_marker") > 0;
            const int fatigueLimit = hasReinforcedSaddle ? 4 : 3;
            if (hasMinorInjury)
            {
                lines.push_back("Écurie : monture personnelle blessée légèrement. Elle ne sera pas poussée sur une vraie route avant soin.");
            }
            else if (fatigue >= fatigueLimit)
            {
                lines.push_back("Écurie : monture personnelle trop fatiguée (" + std::to_string(fatigue) + "/" + std::to_string(fatigueLimit) + "). Soin et repos de monture conseillé avant de compter sur elle.");
            }
            else
            {
                int reduction = travelUnits >= 3 ? 2 : 1;
                if (travelUnits >= 3 && hasReinforcedSaddle)
                {
                    reduction += 1;
                    lines.push_back("Écurie : selle renforcée de route, la monture porte mieux les longues charges.");
                }
                if (travelUnits >= 3 && bond >= 2)
                {
                    reduction += 1;
                    lines.push_back("Lien de monture : l'animal anticipe mieux le rythme, sans devenir infatigable.");
                }
                if (travelUnits >= 2 && hasComfortBridle)
                {
                    lines.push_back("Bridon confortable : les longues rênes fatiguent moins vite les gestes et les arrêts.");
                }
                if (travelUnits >= 3 && hasPackHarness)
                {
                    reduction += 1;
                    lines.push_back("Harnais de bât : les sacoches tirent moins sur les flancs pendant les longues sorties.");
                }
                if (travelUnits >= 3 && surefoot >= 2)
                {
                    lines.push_back("Assurance de monture : l'animal passe mieux les pierres, ponts et départs brusques sans gagner du temps gratuitement.");
                }
                if (travelUnits >= 4 && hasRoadShoes)
                {
                    lines.push_back("Ferrage de route : les sabots tiennent mieux les longues distances répétées.");
                }
                if (travelUnits >= 2 && routeMemory > 0)
                {
                    if (routeMemory >= 2)
                    {
                        reduction += 1;
                        lines.push_back("Mémoire de route : le chemin déjà répété évite quelques mauvais détours.");
                    }
                    else
                    {
                        lines.push_back("Mémoire de route : la monture reconnaît quelques repères, mais pas assez pour gagner un segment entier.");
                    }
                }
                reduced = std::max(0, reduced - reduction);
                const bool lightRouteHandledCleanly = (hasComfortBridle && bond >= 2 && travelUnits <= 2)
                    || (hasPackHarness && routeMemory >= 2 && travelUnits <= 3)
                    || (hasRoadShoes && surefoot >= 2 && travelUnits <= 3);
                if (lightRouteHandledCleanly)
                {
                    lines.push_back("Écurie : trajet court bien géré, aucune fatigue de monture ajoutée cette fois.");
                }
                else
                {
                    player.getInventory().addMaterial(MaterialCatalog::createById("mount_fatigue_marker", 1));
                    lines.push_back("Écurie : monture personnelle utilisée, le trajet est réduit (-" + std::to_string(reduction) + " segment(s)). Fatigue +1/" + std::to_string(fatigueLimit) + ".");
                    if (travelUnits >= 4 && fatigue + 1 >= fatigueLimit && !hasComfortBridle && !hasPackHarness && !hasRoadShoes && surefoot <= 0)
                    {
                        player.getInventory().addMaterial(MaterialCatalog::createById("mount_minor_injury_marker", 1));
                        lines.push_back("Écurie : le dernier effort laisse une gêne légère. L'animal devra être vérifié avant un autre gros trajet.");
                    }
                    else if (travelUnits >= 4 && fatigue + 1 >= fatigueLimit && (hasPackHarness || hasRoadShoes || surefoot > 0))
                    {
                        lines.push_back("Écurie : la route reste dure, mais le harnais, le ferrage ou l'assurance évite la petite blessure qui aurait pu arriver.");
                    }
                }
            }
        }

        if (reduced == travelUnits && player.getInventory().removeMaterialQuantityById("rental_mount_voucher", 1))
        {
            const int reduction = travelUnits >= 3 ? 2 : 1;
            reduced = std::max(0, reduced - reduction);
            lines.push_back("Écurie : Bon de monture consommé, le trajet long devient nettement plus rapide (-" + std::to_string(reduction) + " segment(s) de déplacement).");
        }
        else if (reduced == travelUnits && (player.hasActiveLocalSubscription("stable_relay_weekly") || player.hasActiveLocalSubscription("trade_route_weekly")))
        {
            reduced = std::max(0, reduced - 1);
            lines.push_back("Écurie/relais : abonnement actif, le trajet est mieux préparé (-1 segment de déplacement). ");
        }
        else if (player.getInventory().removeMaterialQuantityById("route_scout_note", 1))
        {
            reduced = std::max(0, reduced - 1);
            lines.push_back("Relais : Note d'éclaireur de route consommée, le chemin évite un vrai détour (-1 segment de déplacement).");
        }
        else if (player.getInventory().removeMaterialQuantityById("loaded_pack_saddle", 1))
        {
            reduced = std::max(0, reduced - 1);
            lines.push_back("Écurie : Selle de bât chargée consommée, le départ long évite les réglages de dernière minute (-1 segment de déplacement).");
        }
        else if (player.getInventory().removeMaterialQuantityById("prepared_saddlebags", 1))
        {
            reduced = std::max(0, reduced - 1);
            lines.push_back("Écurie : Sacoches préparées consommées, la charge ne ralentit pas le départ (-1 segment de déplacement).");
        }
        else if (player.getInventory().removeMaterialQuantityById("relay_route_badge", 1))
        {
            reduced = std::max(0, reduced - 1);
            lines.push_back("Relais : Badge de route consommé, un contrôle ou détour est évité (-1 segment de déplacement).");
        }
        else if (player.getInventory().removeMaterialQuantityById("stable_box_reservation", 1))
        {
            reduced = std::max(0, reduced - 1);
            lines.push_back("Écurie : Réservation de box consommée, la charge encombrante ne suit pas tout le trajet (-1 segment de déplacement).");
        }
        else if (player.getInventory().removeMaterialQuantityById("travel_distance_mark", 1))
        {
            reduced = std::max(0, reduced - 1);
            lines.push_back("Écurie/relais : Marque de distance de trajet consommée (-1 segment de déplacement).");
        }
        else if (player.getInventory().removeMaterialQuantityById("stable_stall_ticket", 1))
        {
            reduced = std::max(0, reduced - 1);
            lines.push_back("Écurie : Ticket d'écurie consommé pour préparer monture, sacoches ou relais (-1 segment de déplacement).");
        }
        else if (player.getInventory().removeMaterialQuantityById("route_toll_receipt", 1))
        {
            reduced = std::max(0, reduced - 1);
            lines.push_back("Relais : Reçu de péage utilisé pour éviter un détour administratif (-1 segment de déplacement).");
        }

        if (reduced > 0 && (playerHasExplorationPassive(player, "orcish_forced_march") || playerHasExplorationPassive(player, "dragon_weather_blood")) && travelUnits >= 2)
        {
            reduced = std::max(0, reduced - 1);
            lines.push_back("Passif racial : endurance naturelle sur longue distance (-1 segment de déplacement). ");
        }

        if (reduced == travelUnits)
        {
            lines.push_back("Distance : aucun préparatif d'écurie/relais utilisé, le déplacement garde son coût complet.");
        }

        return reduced;
    }

    bool explorationTouchesNight(int startUnit, int totalUnits, int unitsPerDay)
    {
        if (totalUnits <= 0) return startUnit >= 4;
        for (int offset = 0; offset <= totalUnits; ++offset)
        {
            int unit = (startUnit + offset) % std::max(1, unitsPerDay);
            if (unit == 4) return true;
        }
        return false;
    }

    int applyNightExplorationRisk(Player& player, bool touchesNight, Random& random, std::vector<std::string>& lines, int& extraFightChance)
    {
        extraFightChance = 0;
        if (!touchesNight)
        {
            return 0;
        }

        int rollShift = 12;
        extraFightChance = 18;
        lines.push_back("Nuit : l'exploration touche la nuit, les traces sont moins lisibles et les monstres sortent plus facilement.");

        if (playerHasExplorationPassive(player, "night_vision"))
        {
            rollShift -= 4;
            extraFightChance -= 6;
            lines.push_back("Vision nocturne : ton passif réduit une partie du risque de nuit.");
        }
        if (playerHasExplorationPassive(player, "halfling_lucky_step"))
        {
            rollShift -= 2;
            extraFightChance -= 3;
            lines.push_back("Pas chanceux : les petits accidents nocturnes ont un peu moins de prise.");
        }
        if (playerHasExplorationPassive(player, "fairy_mana_sense"))
        {
            rollShift -= 2;
            lines.push_back("Sens magique : les lumières et courants étranges sont repérés avant de devenir un piège.");
        }

        if (player.getInventory().countMaterialById("night_survival_kit") > 0)
        {
            player.getInventory().removeMaterialQuantityById("night_survival_kit", 1);
            rollShift = std::max(0, rollShift - 9);
            extraFightChance = std::max(0, extraFightChance - 12);
            lines.push_back("Kit de survie nocturne consommé : trajet balisé, feu couvert et risque nocturne fortement réduit.");
        }
        else if (player.getInventory().countMaterialById("fire_lantern") > 0 || player.getInventory().countMaterialById("mycelium_lantern") > 0)
        {
            rollShift = std::max(0, rollShift - 6);
            extraFightChance = std::max(0, extraFightChance - 8);
            lines.push_back("Lanterne active : la lumière limite les mauvaises surprises, sans annuler totalement le danger.");
        }
        else
        {
            lines.push_back("Aucun éclairage sérieux : lanterne à feu, lanterne de mycélium ou kit nocturne conseillé pour éviter le throw nocturne.");
        }

        if (random.between(1, 100) <= extraFightChance)
        {
            lines.push_back("Bruit dans l'obscurité : une rencontre supplémentaire devient possible pendant cette sortie.");
        }

        return rollShift;
    }

    struct ExplorationBossUnlockResult
    {
        bool unlocked = false;
        std::string line;
    };

    ExplorationBossUnlockResult tryUnlockExplorationBossVariation(Player& player, Random& random, bool dangerousSite, const std::string& discoveryLocation)
    {
        const std::size_t unlockedCount = player.getUnlockedBossIds().size();
        const std::size_t uniqueBossDefeats = player.getDefeatedBossIds().size();
        const int level = player.getLevel();

        if (level < 10)
        {
            return {false, "Trace trop faible : le registre ne stabilise encore aucun emplacement de boss fiable."};
        }

        if (!player.canUseRareBossDiscovery())
        {
            const int remainingDays = std::max(1, player.getRareBossDiscoveryCooldownExpiresAtDay() - player.getWorldDaysElapsed());
            return {false, "Les traces exceptionnelles se brouillent encore. Le registre estime qu'il lui faut environ "
                + std::to_string(remainingDays) + " jour(s) avant de pouvoir isoler un autre emplacement."};
        }

        // Exceptional exploration must never rush the near-final roster.
        if (unlockedCount >= 28)
        {
            return {false, "Trace verrouillée : les présences presque finales ne laissent aucun emplacement exploitable par simple exploration."};
        }

        if (unlockedCount >= 23 && (uniqueBossDefeats < 6 || level < 24))
        {
            return {false, "Trace trop haute : le personnage manque encore de victoires confirmées pour distinguer cette présence des faux témoignages."};
        }

        // This roll is intentionally extremely rare. It is evaluated only inside an already rare
        // exploration event, then followed by a thirty-day in-world cooldown after success.
        const int chance = dangerousSite ? 3 : 1;
        if (random.between(1, 100) > chance)
        {
            return {false, "Trace instable : un emplacement semble exister, mais les indices se contredisent avant que le registre puisse le conserver."};
        }

        const bool unlocked = player.unlockNextBossVariationFromRareDiscovery(discoveryLocation, 30);
        if (unlocked)
        {
            return {true, "Découverte exceptionnelle : le registre conserve l'emplacement approximatif d'une seule présence inconnue. Son identité reste brouillée."};
        }

        return {false, "Trace finale bloquée : FireFlight et les présences terminales ne peuvent pas être révélés par une simple piste d'exploration."};
    }

    struct QuestSearchHint
    {
        bool hasAny = false;
        bool wantsMaterial = false;
        bool wantsCombat = false;
        bool wantsExploration = false;
        bool wantsBestiary = false;
    };

    int biomeEvolutionTriggerMargin(const Player& player)
    {
        // EN: Late game waits a little longer before old zones adapt too strongly.
        // FR: En fin de jeu, les anciennes zones attendent un peu plus avant de se réadapter.
        return player.getLevel() >= 80 ? 15 : 10;
    }

    int biomeEvolutionDangerBonus(const ExplorationBiome& biome)
    {
        if (biome.name == "Ruines effondrées") return 16;
        if (biome.name == "Cimetière oublié") return 15;
        if (biome.name == "Marais trouble") return 14;
        if (biome.name == "Montagne froide") return 11;
        if (biome.name == "Forêt ancienne") return 9;
        if (biome.name == "Route commerciale") return 7;
        return 5;
    }

    bool isBiomeEvolvedForPlayer(const Player& player, const ExplorationBiome& biome)
    {
        return player.getLevel() > biome.maxLevel + biomeEvolutionTriggerMargin(player);
    }

    int evolvedBiomeMinLevel(const Player& player, const ExplorationBiome& biome)
    {
        if (!isBiomeEvolvedForPlayer(player, biome))
        {
            return biome.minLevel;
        }

        const int playerLevel = player.getLevel();
        const int gapWithNaturalMax = std::max(0, playerLevel - biome.maxLevel);
        const int dynamicFloor = biome.maxLevel + gapWithNaturalMax / 2;

        // EN: Old zones should become relevant again without erasing their easier identity.
        // FR: Les anciennes zones redeviennent utiles sans perdre leur identité plus accessible.
        return std::max(biome.maxLevel + 1, dynamicFloor);
    }

    int evolvedBiomeMaxLevel(const Player& player, const ExplorationBiome& biome)
    {
        if (!isBiomeEvolvedForPlayer(player, biome))
        {
            return biome.maxLevel;
        }

        const int playerLevel = player.getLevel();
        const int dangerBonus = biomeEvolutionDangerBonus(biome);
        const int ceiling = playerLevel + dangerBonus;

        return std::max(evolvedBiomeMinLevel(player, biome) + 3, ceiling);
    }

    std::string evolvedBiomeRangeText(const Player& player, const ExplorationBiome& biome)
    {
        const int minLevel = evolvedBiomeMinLevel(player, biome);
        const int maxLevel = evolvedBiomeMaxLevel(player, biome);

        if (!isBiomeEvolvedForPlayer(player, biome))
        {
            return "niv. " + std::to_string(biome.minLevel) + "-" + std::to_string(biome.maxLevel);
        }

        return "niv. " + std::to_string(biome.minLevel) + "-" + std::to_string(biome.maxLevel)
            + " -> zone évoluée " + std::to_string(minLevel) + "-" + std::to_string(maxLevel);
    }

    Monster createExplorationMonsterForBiome(const Player& player, Random& random, const ExplorationBiome& biome, const ExplorationIntensity& intensity)
    {
        int level = random.between(evolvedBiomeMinLevel(player, biome), evolvedBiomeMaxLevel(player, biome));

        if (!isBiomeEvolvedForPlayer(player, biome) && level > player.getLevel() + 15)
        {
            level = player.getLevel() + 15;
        }

        if (intensity.name == "Sortie prudente" && level > player.getLevel() + 6)
        {
            level = player.getLevel() + 6;
        }

        if (intensity.name == "Sortie audacieuse")
        {
            level += random.between(0, 2);
        }

        if (level < 1)
        {
            level = 1;
        }

        return MonsterCatalog::createRandomMonsterForBiome(biome.name, level, random);
    }

    Monster createExplorationEliteForBiome(const Player& player, Random& random, const ExplorationBiome& biome, const ExplorationIntensity& intensity)
    {
        const int minLevel = evolvedBiomeMinLevel(player, biome);
        const int maxLevel = evolvedBiomeMaxLevel(player, biome);
        int eliteMin = minLevel + std::max(0, maxLevel - minLevel) / 2;
        int eliteMax = maxLevel + 1;

        if (intensity.name == "Sortie prudente")
        {
            eliteMax = std::max(eliteMin, eliteMax - 1);
        }
        else if (intensity.name == "Sortie audacieuse")
        {
            eliteMax += 1;
        }

        int level = random.between(std::max(1, eliteMin), std::max(eliteMin, eliteMax));
        Monster base = MonsterCatalog::createRandomMonsterForBiome(biome.name, level, random);
        return MonsterCatalog::createEliteVariant(base, random);
    }

    std::string lowerCopy(std::string value)
    {
        std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });

        return value;
    }

    bool textContainsInsensitive(const std::string& value, const std::string& needle)
    {
        if (needle.empty())
        {
            return false;
        }

        return lowerCopy(value).find(lowerCopy(needle)) != std::string::npos;
    }

    bool questTextImpliesBiome(const std::string& value, const std::string& biomeName)
    {
        const std::string text = lowerCopy(value);
        const std::string biome = lowerCopy(biomeName);

        if (text.empty())
        {
            return false;
        }

        if (text.find(biome) != std::string::npos || biome.find(text) != std::string::npos)
        {
            return true;
        }

        if (biome.find("route") != std::string::npos)
        {
            return text.find("route") != std::string::npos
                || text.find("livraison") != std::string::npos
                || text.find("village") != std::string::npos
                || text.find("client") != std::string::npos
                || text.find("caisse") != std::string::npos
                || text.find("marchand") != std::string::npos
                || text.find("humano") != std::string::npos
                || text.find("embuscade") != std::string::npos;
        }

        if (biome.find("plaine") != std::string::npos)
        {
            return text.find("plaine") != std::string::npos
                || text.find("trace") != std::string::npos
                || text.find("créatures faibles") != std::string::npos
                || text.find("creatures faibles") != std::string::npos;
        }

        if (biome.find("ruines") != std::string::npos)
        {
            return text.find("ruine") != std::string::npos
                || text.find("relais") != std::string::npos
                || text.find("archive") != std::string::npos
                || text.find("poussière arcanique") != std::string::npos
                || text.find("poussiere arcanique") != std::string::npos;
        }

        if (biome.find("cimetière") != std::string::npos || biome.find("cimetiere") != std::string::npos)
        {
            return text.find("cimetière") != std::string::npos
                || text.find("cimetiere") != std::string::npos
                || text.find("mort") != std::string::npos
                || text.find("ombre") != std::string::npos
                || text.find("os") != std::string::npos;
        }

        if (biome.find("gélatine") != std::string::npos || biome.find("gelatine") != std::string::npos)
        {
            return text.find("slime") != std::string::npos
                || text.find("gélatine") != std::string::npos
                || text.find("gelatine") != std::string::npos;
        }

        if (biome.find("forêt") != std::string::npos || biome.find("foret") != std::string::npos)
        {
            return text.find("forêt") != std::string::npos
                || text.find("foret") != std::string::npos
                || text.find("plante") != std::string::npos
                || text.find("feuille") != std::string::npos;
        }

        if (biome.find("montagne") != std::string::npos)
        {
            return text.find("montagne") != std::string::npos
                || text.find("froid") != std::string::npos
                || text.find("métal") != std::string::npos
                || text.find("metal") != std::string::npos
                || text.find("forge") != std::string::npos;
        }

        if (biome.find("marais") != std::string::npos)
        {
            return text.find("marais") != std::string::npos
                || text.find("boue") != std::string::npos
                || text.find("noy") != std::string::npos;
        }

        if (biome.find("cloches") != std::string::npos || biome.find("temple") != std::string::npos)
        {
            return text.find("cloche") != std::string::npos
                || text.find("sanctuaire") != std::string::npos
                || text.find("serment") != std::string::npos
                || text.find("temple") != std::string::npos;
        }

        if (biome.find("brume bleue") != std::string::npos || biome.find("canaux") != std::string::npos)
        {
            return text.find("brume") != std::string::npos
                || text.find("canal") != std::string::npos
                || text.find("canaux") != std::string::npos
                || text.find("barque") != std::string::npos
                || text.find("passeur") != std::string::npos;
        }

        if (biome.find("carrière") != std::string::npos || biome.find("carriere") != std::string::npos)
        {
            return text.find("carrière") != std::string::npos
                || text.find("carriere") != std::string::npos
                || text.find("craie") != std::string::npos
                || text.find("géant") != std::string::npos
                || text.find("geant") != std::string::npos
                || text.find("os blanc") != std::string::npos;
        }

        if (biome.find("ponts") != std::string::npos || biome.find("marché") != std::string::npos || biome.find("marche") != std::string::npos)
        {
            return text.find("pont") != std::string::npos
                || text.find("marché") != std::string::npos
                || text.find("marche") != std::string::npos
                || text.find("dette") != std::string::npos
                || text.find("contreband") != std::string::npos
                || text.find("jeton") != std::string::npos;
        }

        if (biome.find("statues") != std::string::npos || biome.find("jardin") != std::string::npos)
        {
            return text.find("statue") != std::string::npos
                || text.find("jardin") != std::string::npos
                || text.find("rose") != std::string::npos
                || text.find("pierre") != std::string::npos
                || text.find("pétrifi") != std::string::npos
                || text.find("petrifi") != std::string::npos;
        }

        return false;
    }

    bool questTextMentionsBiome(const Quest& quest, const std::string& biomeName)
    {
        return questTextImpliesBiome(quest.location, biomeName)
            || questTextImpliesBiome(quest.targetFamily, biomeName)
            || questTextImpliesBiome(quest.objective, biomeName)
            || questTextImpliesBiome(quest.title, biomeName);
    }

    bool questCanUseBiomeMaterials(const Quest& quest, const ExplorationBiome& biome)
    {
        if (quest.requiredMaterialId.empty())
        {
            return false;
        }

        return quest.requiredMaterialId == biome.commonMaterialId
            || quest.requiredMaterialId == biome.rareMaterialId;
    }

    bool questLooksRelevantForBiome(const Quest& quest, const ExplorationBiome& biome)
    {
        if (quest.turnedIn || quest.failed || quest.completed || !quest.accepted)
        {
            return false;
        }

        if (questTextMentionsBiome(quest, biome.name) || questCanUseBiomeMaterials(quest, biome))
        {
            return true;
        }

        if (quest.objectiveType == "livraison")
        {
            if ((quest.targetFamily == "Plantes" || textContainsInsensitive(quest.targetFamily, "consommable"))
                && (biome.commonMaterialId == "bitter_healing_leaf" || biome.rareMaterialId == "mountain_blue_flower" || biome.commonMaterialId == "slime_residue"))
            {
                return true;
            }

            if ((textContainsInsensitive(quest.targetFamily, "forge") || textContainsInsensitive(quest.targetFamily, "arme") || textContainsInsensitive(quest.targetFamily, "armure") || textContainsInsensitive(quest.targetFamily, "matériaux"))
                && (biome.commonMaterialId == "rusted_metal_fragment" || biome.commonMaterialId == "worn_leather_piece" || biome.rareMaterialId == "arcane_dust"))
            {
                return true;
            }
        }

        return false;
    }

    QuestSearchHint getQuestSearchHintForBiome(const Player& player, const ExplorationBiome& biome)
    {
        QuestSearchHint hint;

        for (const Quest& quest : player.getQuestLog().getQuests())
        {
            if (!questLooksRelevantForBiome(quest, biome))
            {
                continue;
            }

            hint.hasAny = true;
            if (quest.objectiveType == "livraison") hint.wantsMaterial = true;
            if (quest.objectiveType == "combat") hint.wantsCombat = true;
            if (quest.objectiveType == "exploration") hint.wantsExploration = true;
            if (quest.objectiveType == "bestiaire") hint.wantsBestiary = true;
        }

        return hint;
    }

    int adjustExplorationRollForActiveQuests(int roll, Random& random, const QuestSearchHint& hint)
    {
        if (!hint.hasAny)
        {
            return roll;
        }

        // Bonus léger : environ 15% de chances de transformer une sortie neutre en piste liée à une quête.
        if (random.between(1, 100) > 15)
        {
            return roll;
        }

        if (hint.wantsCombat)
        {
            return random.between(67, 91);
        }

        if (hint.wantsExploration || hint.wantsBestiary)
        {
            return random.between(25, 37);
        }

        if (hint.wantsMaterial)
        {
            return random.between(1, 24);
        }

        return roll;
    }

    std::string miniBossNameForBiome(const ExplorationBiome& biome, bool evolved)
    {
        if (biome.name == "Forêt ancienne") return evolved ? "Loup ancien à mousse noire" : "Gardien de ronces";
        if (biome.name == "Montagne froide") return evolved ? "Yéti aux éclats de givre" : "Briseur de roche gelée";
        if (biome.name == "Marais trouble") return evolved ? "Slime putride couronné" : "Noyeur du marais";
        if (biome.name == "Route commerciale") return evolved ? "Pillard vétéran marqué" : "Chef de bande opportuniste";
        if (biome.name == "Ruines effondrées") return evolved ? "Sentinelle osseuse éveillée" : "Gardien fissuré des ruines";
        if (biome.name == "Bocage aux lanternes") return evolved ? "Roi-lanterne fongique" : "Gardien mycélien";
        if (biome.name == "Désert d'argile rouge") return evolved ? "Colosse d'argile solaire" : "Sentinelle d'argile cuite";
        if (biome.name == "Quartier abandonné") return evolved ? "Propriétaire sans visage" : "Receleur de cave";
        if (biome.name == "Mine sifflante") return evolved ? "Cœur de machine éveillé" : "Foreuse animée";
        if (biome.name == "Cimetière oublié") return evolved ? "Ombre de nom perdu" : "Veilleur sans sépulture";
        if (biome.name == "Temple des cloches fendues") return evolved ? "Sonneur creux du serment" : "Gardien de nef fissuré";
        if (biome.name == "Canaux de brume bleue") return evolved ? "Passeur sans visage" : "Nixe de quai brumeux";
        if (biome.name == "Carrière des os blancs") return evolved ? "Géant enfoui qui respire" : "Golem de craie blanche";
        if (biome.name == "Marché sous les ponts") return evolved ? "Arbitre de dette masqué" : "Collecteur de pont noir";
        if (biome.name == "Jardin des statues qui pleurent") return evolved ? "Muse pétrifiée en larmes" : "Jardinier de marbre";
        if (biome.name == "Plaine sauvage") return evolved ? "Alpha aux crocs longs" : "Bête territoriale";
        return evolved ? "Créature évoluée locale" : "Menace locale isolée";
    }

    std::string miniBossQuestFamilyForBiome(const ExplorationBiome& biome, bool evolved)
    {
        if (evolved) return "Mini-boss / menace évoluée";
        if (biome.name == "Route commerciale") return "Humanoïdes / embuscades";
        if (biome.name == "Cimetière oublié") return "Morts-vivants / ombres";
        if (biome.name == "Bocage aux lanternes") return "Plantes lumineuses / bêtes nocturnes";
        if (biome.name == "Désert d'argile rouge") return "Désert / argile / sel lunaire";
        if (biome.name == "Quartier abandonné") return "Humanoïdes urbains / automates";
        if (biome.name == "Mine sifflante") return "Mines / constructions / machines";
        if (biome.name == "Temple des cloches fendues") return "Sanctuaire / gardiens de nef";
        if (biome.name == "Canaux de brume bleue") return "Canaux / brume / passeurs";
        if (biome.name == "Carrière des os blancs") return "Carrière / os blancs / géants";
        if (biome.name == "Marché sous les ponts") return "Humanoïdes / dettes / contrebande";
        if (biome.name == "Jardin des statues qui pleurent") return "Statues / ronces / noblesse abandonnée";
        if (biome.name == "Forêt ancienne" || biome.name == "Plaine sauvage") return "Créatures locales";
        return "Élite / menace";
    }

    std::string dangerousSiteNameForBiome(const ExplorationBiome& biome)
    {
        if (biome.name == "Forêt ancienne") return "clairière aux racines closes";
        if (biome.name == "Montagne froide") return "faille bleue sous la glace";
        if (biome.name == "Marais trouble") return "mare noire qui respire";
        if (biome.name == "Route commerciale") return "ancien relais barricadé";
        if (biome.name == "Ruines effondrées") return "salle basse aux piliers brisés";
        if (biome.name == "Bocage aux lanternes") return "clairière où les lanternes respirent toutes ensemble";
        if (biome.name == "Désert d'argile rouge") return "oasis sèche entourée de statues fendues";
        if (biome.name == "Quartier abandonné") return "maison scellée avec des clés encore dans la porte";
        if (biome.name == "Mine sifflante") return "ascenseur de mine bloqué qui siffle sans vent";
        if (biome.name == "Cimetière oublié") return "allée de tombes qui ne portent plus de noms";
        if (biome.name == "Plaine sauvage") return "cercle d'herbes couchées";
        return "lieu dangereux sans nom";
    }

    std::string dangerousSiteWarningForBiome(const ExplorationBiome& biome)
    {
        if (biome.name == "Forêt ancienne") return "Les arbres se penchent vers le centre comme s'ils voulaient enfermer un souvenir.";
        if (biome.name == "Montagne froide") return "La neige tombe vers le haut pendant quelques secondes.";
        if (biome.name == "Marais trouble") return "La boue forme des bulles régulières, presque comme une respiration.";
        if (biome.name == "Route commerciale") return "Des roues abandonnées grincent alors qu'aucun chariot ne bouge.";
        if (biome.name == "Ruines effondrées") return "Les pierres portent des griffures trop longues pour venir d'un outil.";
        if (biome.name == "Bocage aux lanternes") return "Les champignons s'éteignent un par un, comme si quelqu'un fermait des yeux.";
        if (biome.name == "Désert d'argile rouge") return "Le sable rouge garde des empreintes qui ne sont pas encore passées.";
        if (biome.name == "Quartier abandonné") return "Une fenêtre s'ouvre alors que la maison est censée être vide depuis des années.";
        if (biome.name == "Mine sifflante") return "Un rail vibre doucement, mais aucun wagon ne bouge.";
        if (biome.name == "Cimetière oublié") return "Certaines tombes semblent plus récentes que les dates gravées dessus.";
        if (biome.name == "Plaine sauvage") return "Tous les insectes se taisent au même moment.";
        return "L'air devient lourd et refuse de circuler normalement.";
    }

    std::string bossTraceForBiome(const ExplorationBiome& biome)
    {
        if (biome.name == "Forêt ancienne") return "une silhouette de bois ancien et de mousse t'observe sans agressivité, mais sans faiblesse";
        if (biome.name == "Montagne froide") return "une masse draconique ou rocheuse fait vibrer la glace sans se montrer entièrement";
        if (biome.name == "Marais trouble") return "quelque chose sous l'eau déplace la surface comme une paupière immense";
        if (biome.name == "Route commerciale") return "une présence compte les pièces, les dettes et les battements de cœur";
        if (biome.name == "Ruines effondrées") return "une ombre ancienne rejoue le même pas entre deux piliers brisés";
        if (biome.name == "Bocage aux lanternes") return "une couronne fongique s'allume au loin, chaque lumière suivant ton souffle";
        if (biome.name == "Désert d'argile rouge") return "une silhouette d'argile immense laisse des traces sèches dans le sel lunaire";
        if (biome.name == "Quartier abandonné") return "quelqu'un compte les portes fermées depuis l'intérieur des maisons vides";
        if (biome.name == "Mine sifflante") return "un cœur mécanique bat quelque part derrière les rails et les clous froids";
        if (biome.name == "Cimetière oublié") return "un nom gravé disparaît lentement d'une pierre pendant que tu le regardes";
        if (biome.name == "Plaine sauvage") return "un alpha invisible tourne autour de toi, assez loin pour ne laisser qu'une pression";
        return "une variation d'énergie anormale refuse de porter un nom stable";
    }



    std::string environmentalHazardTextForBiome(const ExplorationBiome& biome)
    {
        if (biome.name == "Forêt ancienne") return "des lianes se referment sur un passage couvert de feuilles médicinales";
        if (biome.name == "Montagne froide") return "une corniche gelée cache un filon sous une plaque de neige instable";
        if (biome.name == "Marais trouble") return "une poche de gaz noir remonte sous des plantes utiles";
        if (biome.name == "Route commerciale") return "un ancien chariot renversé grince au bord d'une embuscade possible";
        if (biome.name == "Ruines effondrées") return "un plafond fissuré protège encore un fragment arcanique";
        if (biome.name == "Cimetière oublié") return "une dalle funéraire bouge comme si quelque chose respirait dessous";
        if (biome.name == "Mares gélatineuses") return "une nappe de gelée transparente recouvre des résidus encore propres";
        if (biome.name == "Bocage aux lanternes") return "des spores lumineuses flottent au-dessus d'une poche de résine encore fraîche";
        if (biome.name == "Désert d'argile rouge") return "une plaque d'argile creuse cache des cristaux de sel lunaire";
        if (biome.name == "Quartier abandonné") return "un plancher usé menace de céder sous une cache de vieilles pièces";
        if (biome.name == "Mine sifflante") return "une poutre rouillée retient un petit mécanisme encore récupérable";
        if (biome.name == "Plaine sauvage") return "un terrier frais cache des restes utiles, mais le sol tremble légèrement";
        return "un obstacle naturel bloque une ressource exploitable";
    }

    std::string environmentalObservationForBiome(const ExplorationBiome& biome)
    {
        if (biome.name == "Cimetière oublié") return "Observation : les morts-vivants du cimetière réagissent souvent aux noms, aux sépultures et aux objets volés aux tombes.";
        if (biome.name == "Mares gélatineuses") return "Observation : les slimes sont plus variés près des eaux stagnantes, surtout quand la gelée paraît trop propre ou trop brillante.";
        if (biome.name == "Route commerciale") return "Observation : les humanoïdes de route protègent souvent les caches, car elles servent de réserve ou de piège.";
        if (biome.name == "Forêt ancienne") return "Observation : la forêt répond aux gestes brusques. Une récolte propre attire moins les prédateurs végétaux.";
        if (biome.name == "Montagne froide") return "Observation : le froid cache autant les filons que les prédateurs. Les traces se lisent mieux près des corniches.";
        if (biome.name == "Marais trouble") return "Observation : le marais annonce souvent le danger par l'odeur avant de le montrer.";
        if (biome.name == "Ruines effondrées") return "Observation : les ruines gardent des sentinelles lentes mais tenaces près des fragments arcaniques.";
        if (biome.name == "Bocage aux lanternes") return "Observation : les lumières du bocage réagissent aux bruits. Une approche calme préserve mieux les lanternes de mycélium.";
        if (biome.name == "Désert d'argile rouge") return "Observation : l'argile rouge protège souvent le sel lunaire, mais les fausses oasis attirent les pilleurs.";
        if (biome.name == "Quartier abandonné") return "Observation : les maisons vides gardent surtout des preuves, contrats, cartes et petits objets oubliés.";
        if (biome.name == "Mine sifflante") return "Observation : la mine répond aux vibrations. Les ressorts et clous rares se trouvent près des machines encore tièdes.";
        return "Observation : ce biome récompense l'étude autant que la prise de risque.";
    }

    std::string chooseExplorationQuality(Random& random, bool carefulRecovery)
    {
        int roll = random.between(1, 100);

        if (carefulRecovery)
        {
            roll += 12;
        }

        if (roll >= 98) return "exceptional";
        if (roll >= 84) return "high";
        if (roll <= 12) return "low";
        return "normal";
    }

    int explorationGoldDifficultyPercent(DifficultyMode difficulty)
    {
        switch (difficulty)
        {
            case DifficultyMode::Easy: return 115;
            case DifficultyMode::Hard: return 90;
            case DifficultyMode::Nightmare: return 80;
            case DifficultyMode::Lethal: return 70;
            case DifficultyMode::Normal:
            default: return 100;
        }
    }

    int explorationGoldSoftCap(const Player& player, const ExplorationIntensity& intensity, DifficultyMode difficulty, int rewardTier)
    {
        int cap = 0;

        switch (rewardTier)
        {
            case 3: cap = 120 + player.getLevel() * 7; break; // cache rare / vraie découverte
            case 2: cap = 80 + player.getLevel() * 5; break;  // coffre correct / récompense improvisée
            case 1:
            default: cap = 45 + player.getLevel() * 3; break;  // petit trésor
        }

        if (intensity.name == "Sortie prudente")
        {
            cap = cap * 85 / 100;
        }
        else if (intensity.name == "Sortie audacieuse")
        {
            cap = cap * 115 / 100;
        }

        cap = cap * explorationGoldDifficultyPercent(difficulty) / 100;
        return std::max(8, cap);
    }

    // EN: applyExplorationGoldReward controls direct gold inflation from exploration events.
    // FR: applyExplorationGoldReward limite l'inflation d'or direct venant des événements d'exploration.
    int applyExplorationGoldReward(int baseGold, const Player& player, const ExplorationIntensity& intensity, DifficultyMode difficulty, int rewardTier)
    {
        int scaledGold = std::max(1, baseGold * intensity.goldPercent / 100);
        scaledGold = std::max(1, scaledGold * explorationGoldDifficultyPercent(difficulty) / 100);

        const int cap = explorationGoldSoftCap(player, intensity, difficulty, rewardTier);
        if (scaledGold <= cap)
        {
            return scaledGold;
        }

        // EN: Keep lucky finds exciting, but avoid a single normal exploration chain creating runaway economy.
        // FR: On garde les trouvailles chanceuses fortes, sans laisser une chaîne normale casser l'économie.
        int overflow = scaledGold - cap;
        return cap + overflow / 4;
    }

    // EN: applyExplorationQuantityBonus declares or implements a focused behavior used by this module.
    // FR: applyExplorationQuantityBonus déclare ou implémente un comportement précis utilisé par ce module.
    int applyExplorationQuantityBonus(int baseQuantity, const ExplorationIntensity& intensity)
    {
        return std::max(1, baseQuantity + intensity.quantityBonus);
    }

    // EN: adjustExplorationEventRoll declares or implements a focused behavior used by this module.
    // FR: adjustExplorationEventRoll déclare ou implémente un comportement précis utilisé par ce module.
    int adjustExplorationEventRoll(int roll, const ExplorationIntensity& intensity)
    {
        return std::clamp(roll + intensity.eventShift, 1, 100);
    }

    std::string randomBiomeForClient(Random& random, const std::string& clientName)
    {
        if (clientName == "Mira")
        {
            std::vector<std::string> biomes = {"Plaine sauvage", "Route commerciale", "Ruines effondrées"};
            return biomes[random.between(0, static_cast<int>(biomes.size()) - 1)];
        }

        if (clientName == "Orren")
        {
            return "Route commerciale";
        }

        if (clientName == "Lysa")
        {
            std::vector<std::string> biomes = {"Forêt ancienne", "Plaine sauvage", "Marais trouble"};
            return biomes[random.between(0, static_cast<int>(biomes.size()) - 1)];
        }

        if (clientName == "Bram")
        {
            std::vector<std::string> biomes = {"Ruines effondrées", "Montagne froide", "Plaine sauvage"};
            return biomes[random.between(0, static_cast<int>(biomes.size()) - 1)];
        }

        if (clientName == "Soryn")
        {
            std::vector<std::string> biomes = {"Archives noyées", "Ruines effondrées", "Forêt ancienne"};
            return biomes[random.between(0, static_cast<int>(biomes.size()) - 1)];
        }

        if (clientName == "Forgeron" || clientName == "Armurier" || clientName == "Vendeur d'armes")
        {
            std::vector<std::string> biomes = {"Montagne froide", "Ruines effondrées", "Route commerciale", "Plaine sauvage"};
            return biomes[random.between(0, static_cast<int>(biomes.size()) - 1)];
        }

        if (clientName == "Alchimiste" || clientName == "Herboriste" || clientName == "Vendeur de consommables")
        {
            std::vector<std::string> biomes = {"Forêt ancienne", "Mares gélatineuses", "Marais trouble", "Montagne froide", "Plaine sauvage"};
            return biomes[random.between(0, static_cast<int>(biomes.size()) - 1)];
        }

        if (clientName == "Marchand inquiet")
        {
            std::vector<std::string> biomes = {"Route commerciale", "Plaine sauvage", "Ruines effondrées"};
            return biomes[random.between(0, static_cast<int>(biomes.size()) - 1)];
        }

        if (clientName == "Vendeur de composants" || clientName == "Villageois nerveux")
        {
            std::vector<std::string> biomes = {"Forêt ancienne", "Mares gélatineuses", "Marais trouble", "Route commerciale", "Ruines effondrées", "Plaine sauvage"};
            return biomes[random.between(0, static_cast<int>(biomes.size()) - 1)];
        }

        if (clientName == "Bibliothécaire")
        {
            std::vector<std::string> biomes = {"Forêt ancienne", "Mares gélatineuses", "Montagne froide", "Marais trouble", "Ruines effondrées"};
            return biomes[random.between(0, static_cast<int>(biomes.size()) - 1)];
        }

        if (clientName == "Mila des lanternes" || clientName == "Orvan le récolteur de spores" || clientName == "Lysandre aux fioles claires")
        {
            return "Bocage aux lanternes";
        }

        if (clientName == "Safa la pisteuse" || clientName == "Boro le potier" || clientName == "Nelia du sel froid")
        {
            return "Désert d'argile rouge";
        }

        if (clientName == "Maître Hulan" || clientName == "Rika des clés" || clientName == "Tomo le veilleur de rue")
        {
            return "Quartier abandonné";
        }

        if (clientName == "Bram le foreur" || clientName == "Sœur Elga" || clientName == "Pip l'engreneur")
        {
            return "Mine sifflante";
        }

        if (clientName == "Nalia des lanternes" || clientName == "Owen le papillonnier")
        {
            return "Verger des lucioles de fer";
        }

        if (clientName == "Archiviste Meron" || clientName == "Scribe Ysolde")
        {
            return "Archives noyées";
        }

        if (clientName == "Kerr des corniches" || clientName == "Mira la cordeuse")
        {
            return "Falaises des drakes gris";
        }

        if (clientName == "Rollo l'ancien forain" || clientName == "Lili aux tickets")
        {
            return "Foire abandonnée";
        }

        if (clientName == "Sœur Cléria" || clientName == "Père Lior" || clientName == "Noé le sonneur")
        {
            return "Temple des cloches fendues";
        }

        if (clientName == "Batia des barques" || clientName == "Malo du quai bleu" || clientName == "Ysée la brumeuse")
        {
            return "Canaux de brume bleue";
        }

        if (clientName == "Tarek le carrier" || clientName == "Blanche des fossiles" || clientName == "Gorin au marteau pâle")
        {
            return "Carrière des os blancs";
        }

        if (clientName == "Niko sous le pont" || clientName == "Vera aux dettes" || clientName == "Gilda la troqueuse")
        {
            return "Marché sous les ponts";
        }

        if (clientName == "Rosalie des statues" || clientName == "Ilan le jardinier muet" || clientName == "Dame Séraphine")
        {
            return "Jardin des statues qui pleurent";
        }

        std::vector<std::string> biomes = {"Forêt ancienne", "Mares gélatineuses", "Montagne froide", "Marais trouble", "Route commerciale", "Ruines effondrées", "Plaine sauvage", "Verger des lucioles de fer", "Archives noyées", "Falaises des drakes gris", "Foire abandonnée", "Temple des cloches fendues", "Canaux de brume bleue", "Carrière des os blancs", "Marché sous les ponts", "Jardin des statues qui pleurent"};
        return biomes[random.between(0, static_cast<int>(biomes.size()) - 1)];
    }


    struct MicroChallengeResult
    {
        bool success = false;
        bool partial = false;
        std::vector<std::string> lines;
    };

    MicroChallengeResult runExplorationMicroChallenge(Player& player, const ExplorationBiome& biome, const ExplorationIntensity& intensity, Random& random)
    {
        (void)intensity;

        struct Challenge
        {
            std::string id;
            std::string title;
            std::string question;
            std::vector<std::pair<int, std::string>> options;
            int correctChoice = 1;
            std::string successLine;
            std::string failureLine;
            int cooldownDays = 2;
        };

        std::vector<Challenge> challenges = {
            {
                "generic_orientation",
                "ÉPREUVE D'ORIENTATION",
                "Tu dois choisir rapidement une méthode avant de t'enfoncer plus loin.",
                {{1, "Marquer un repère discret et écouter la zone"}, {2, "Courir vers le premier bruit"}, {3, "Jeter une pierre très loin pour voir"}},
                1,
                "Bonne approche : tu avances avec un vrai repère, pas juste au feeling.",
                "Mauvaise approche : tu avances quand même, mais ton repère est moins fiable."
            },
            {
                "generic_calculation_markers",
                "ÉPREUVE DE CALCUL",
                "Tu disposes de 15 balises à répartir sur 3 chemins égaux. Combien de balises par chemin ?",
                {{1, "4 balises"}, {2, "5 balises"}, {3, "6 balises"}},
                2,
                "Calcul propre : chaque chemin reçoit assez de repères.",
                "Erreur de calcul : tu corriges la répartition, mais tu perds un peu de temps."
            },
            {
                "generic_french_notes",
                "ÉPREUVE DE FRANÇAIS",
                "Quelle phrase est correcte pour ton carnet de terrain ?",
                {{1, "Les traces sont récentes."}, {2, "Les trace sont récente."}, {3, "Les traces est récentes."}},
                1,
                "Note claire : ton carnet reste lisible et exploitable pour la guilde.",
                "Note maladroite : tu comprends l'idée, mais le rapport sera moins utile."
            },
            {
                "generic_observation",
                "ÉPREUVE D'OBSERVATION",
                "Le vent efface les traces légères. Quel indice vérifier en premier ?",
                {{1, "Les marques profondes près des pierres"}, {2, "Une feuille qui bouge"}, {3, "Le nuage le plus proche"}},
                1,
                "Observation utile : tu conserves un indice que le vent ne peut pas effacer.",
                "Observation faible : tu reviens vers les pierres après avoir suivi un faux indice."
            },
            {
                "generic_supply_weight",
                "ÉPREUVE DE CHARGEMENT",
                "Quatre sacs pèsent 3 kg chacun. Quel poids total ajoutes-tu à ton équipement ?",
                {{1, "7 kg"}, {2, "12 kg"}, {3, "16 kg"}},
                2,
                "Charge calculée : tu répartis le poids avant qu'il ne devienne un problème.",
                "Charge mal estimée : une sangle te rappelle brutalement le vrai total."
            },
            {
                "generic_safe_water",
                "ÉPREUVE DE SURVIE",
                "Tu trouves une eau claire dans une zone inconnue. Quelle réaction est la plus sûre ?",
                {{1, "La boire immédiatement"}, {2, "La filtrer ou la faire bouillir avant"}, {3, "La mélanger avec une potion"}},
                2,
                "Prudence utile : l'eau devient une ressource au lieu d'un pari.",
                "Choix risqué : tu renonces juste avant de transformer une pause en maladie."
            },
            {
                "generic_signal_code",
                "ÉPREUVE DE SIGNAL",
                "Ton groupe utilise deux coups courts puis un long pour signaler un danger. Quel motif reproduire ?",
                {{1, "Court, court, long"}, {2, "Long, court, long"}, {3, "Trois coups longs"}},
                1,
                "Signal exact : la zone reçoit le message sans attirer tout ce qui vit autour.",
                "Signal confus : personne ne sait si tu annonces un danger ou le repas."
            },
            {
                "generic_distance_pace",
                "ÉPREUVE D'ALLURE",
                "Tu parcours 2 km par heure pendant 3 heures. Quelle distance as-tu couverte ?",
                {{1, "5 km"}, {2, "6 km"}, {3, "8 km"}},
                2,
                "Allure maîtrisée : ton estimation correspond enfin au terrain.",
                "Estimation fausse : la carte te semble soudain plus longue que prévu."
            },
            {
                "generic_footprint_order",
                "ÉPREUVE DE DÉDUCTION",
                "Une empreinte nette recouvre une empreinte effacée. Laquelle est la plus récente ?",
                {{1, "L'empreinte nette au-dessus"}, {2, "L'empreinte effacée dessous"}, {3, "Impossible à savoir"}},
                1,
                "Déduction propre : tu lis l'ordre des passages sans inventer une histoire.",
                "Déduction fragile : tu reprends les couches une par une avant de continuer."
            },
            {
                "generic_inventory_count",
                "ÉPREUVE D'INVENTAIRE",
                "Tu avais 9 torches et en utilises 2. Combien t'en reste-t-il ?",
                {{1, "6"}, {2, "7"}, {3, "11"}},
                2,
                "Inventaire juste : tu sais exactement combien de nuits tu peux encore éclairer.",
                "Inventaire faux : tu recompte avant que l'obscurité ne fasse le calcul pour toi."
            }
        };

        auto addBiomeChallenge = [&](const Challenge& challenge) {
            challenges.push_back(challenge);
        };

        if (biome.name == "Plaine sauvage")
        {
            addBiomeChallenge({"plain_grass_direction", "ÉPREUVE DE PLAINE", "Les herbes sont couchées vers l'est sur une large bande. Quelle hypothèse vérifier en premier ?", {{1, "Un passage récent venant de l'ouest"}, {2, "Une pluie verticale"}, {3, "Une pierre immobile"}}, 1, "Lecture juste : tu distingues un passage d'un simple mouvement du vent.", "Lecture trop rapide : tu observes plus longtemps avant de choisir une direction."});
            addBiomeChallenge({"plain_watch_rotation", "ÉPREUVE DE GARDE", "Trois aventuriers se relaient pendant 6 heures à parts égales. Combien de temps chacun surveille-t-il ?", {{1, "1 heure"}, {2, "2 heures"}, {3, "3 heures"}}, 2, "Relais propre : personne ne s'endort en prétendant que c'était son tour.", "Relais faux : la dispute dure presque aussi longtemps que la garde."});
        }
        else if (biome.name == "Route commerciale")
        {
            addBiomeChallenge({"road_wagon_tracks", "ÉPREUVE DE ROUTE", "Deux roues parallèles laissent quatre longues traces après deux chariots identiques. Combien de chariots sont passés ?", {{1, "Un"}, {2, "Deux"}, {3, "Quatre"}}, 2, "Comptage utile : tu sépares les véhicules des simples lignes dans la boue.", "Comptage hésitant : tu vérifies l'écartement avant de poursuivre."});
            addBiomeChallenge({"road_toll_change", "ÉPREUVE DE PÉAGE", "Un passage coûte 7 pièces et tu paies avec 10. Quelle monnaie doit revenir ?", {{1, "2 pièces"}, {2, "3 pièces"}, {3, "17 pièces"}}, 2, "Monnaie exacte : le péager comprend que tu comptes aussi vite que lui.", "Monnaie fausse : le sourire du péager devient beaucoup trop large."});
        }
        else if (biome.name == "Mares gélatineuses")
        {
            addBiomeChallenge({"slime_color_count", "ÉPREUVE GÉLATINEUSE", "Deux slimes bleus rejoignent trois slimes verts. Combien de slimes vois-tu au total ?", {{1, "4"}, {2, "5"}, {3, "6"}}, 2, "Comptage net : aucune masse colorée ne se cache dans ton total.", "Comptage faux : l'un des slimes se divise juste pour se moquer."});
            addBiomeChallenge({"slime_safe_step", "ÉPREUVE D'APPUI", "Une gelée brillante recouvre seulement le centre du passage. Où poses-tu le pied ?", {{1, "Au centre pour aller vite"}, {2, "Sur le bord sec et stable"}, {3, "Dans la flaque la plus profonde"}}, 2, "Appui sûr : tes bottes restent à toi.", "Appui mauvais : tu récupères ton pied avec un bruit humiliant."});
        }
        else if (biome.name == "Forêt ancienne")
        {
            addBiomeChallenge({"forest_moss", "ÉPREUVE DE SOUS-BOIS", "Quelle marque résiste le mieux sans blesser un arbre ancien ?", {{1, "Une entaille profonde"}, {2, "Un ruban récupérable sur une branche basse"}, {3, "Du feu sur l'écorce"}}, 2, "Repère respectueux : tu retrouves ta route sans transformer la forêt en ennemi.", "Repère agressif : tu renonces avant que les branches ne semblent se rapprocher."});
            addBiomeChallenge({"forest_canopy", "ÉPREUVE D'ÉCOUTE", "Des oiseaux cessent de chanter uniquement devant toi. Quelle réaction est la plus prudente ?", {{1, "Ralentir et observer la direction"}, {2, "Crier pour les faire revenir"}, {3, "Courir droit devant"}}, 1, "Silence compris : tu repères la zone qui inquiète la faune.", "Silence ignoré : une branche cassée te rappelle que la forêt prévenait."});
        }
        else if (biome.name == "Montagne froide")
        {
            addBiomeChallenge({"mountain_layers", "ÉPREUVE DE FROID", "Tu portes déjà deux couches et en ajoutes une. Combien de couches protègent maintenant ton torse ?", {{1, "2"}, {2, "3"}, {3, "4"}}, 2, "Préparation correcte : le froid reste un ennemi, pas une condamnation.", "Préparation confuse : tu recompte tes vêtements avant de perdre la sensation de tes doigts."});
            addBiomeChallenge({"mountain_avalanche", "ÉPREUVE DE CORNICHE", "La neige craque au-dessus de toi. Où cherches-tu d'abord un abri ?", {{1, "Sous une corniche rocheuse latérale"}, {2, "Au milieu de la pente"}, {3, "Sur la plaque qui craque"}}, 1, "Réflexe utile : tu quittes l'axe le plus exposé.", "Réflexe dangereux : tu changes de direction avant que la pente décide pour toi."});
        }
        else if (biome.name == "Marais trouble")
        {
            addBiomeChallenge({"swamp_bubbles", "ÉPREUVE DE MARAIS", "Des bulles remontent régulièrement devant toi. Quel geste est le plus sûr ?", {{1, "Tester le sol avec une perche depuis le bord"}, {2, "Sauter au centre"}, {3, "Allumer une torche au-dessus des bulles"}}, 1, "Test prudent : tu évites une poche profonde et peut-être inflammable.", "Geste risqué : l'odeur te convainc de ne pas terminer ton idée."});
            addBiomeChallenge({"swamp_board_count", "ÉPREUVE DE PASSERELLE", "Une passerelle demande 12 planches réparties sur 4 appuis. Combien par appui ?", {{1, "2"}, {2, "3"}, {3, "4"}}, 2, "Répartition propre : la passerelle ne choisit pas ton pied comme faiblesse.", "Répartition fausse : tu corriges avant de tester ton poids."});
        }
        else if (biome.name == "Ruines effondrées")
        {
            addBiomeChallenge({"ruins_arch", "ÉPREUVE DE RUINES", "Une arche fissurée perd de la poussière à chaque vibration. Quel passage privilégier ?", {{1, "Sous le centre de l'arche"}, {2, "Le détour dégagé le long du mur stable"}, {3, "Le sommet de l'arche"}}, 2, "Détour intelligent : les pierres restent au-dessus de toi au lieu de te rejoindre.", "Passage mauvais : une pluie de poussière suffit à te faire changer d'avis."});
            addBiomeChallenge({"ruins_symbols", "ÉPREUVE DE SYMBOLES", "Les plaques portent I, II, III puis V. Quel symbole manque ?", {{1, "IV"}, {2, "VI"}, {3, "X"}}, 1, "Suite comprise : le mécanisme accepte ton ordre.", "Suite ratée : une dalle grince jusqu'à ce que tu reprennes depuis le début."});
        }
        else if (biome.name == "Bocage aux lanternes")
        {
            addBiomeChallenge({"bocage_harvest", "ÉPREUVE DE RÉCOLTE", "Une lanterne de mycélium pulse doucement. Que fais-tu pour ne pas l'abîmer ?", {{1, "Couper la base d'un coup sec"}, {2, "Attendre que la lumière baisse puis détacher la terre autour"}, {3, "Souffler dessus pour l'éteindre"}}, 2, "Récolte intelligente : la lanterne reste presque intacte.", "Geste approximatif : une partie de sa lumière se perd."});
        }
        else if (biome.name == "Désert d'argile rouge")
        {
            addBiomeChallenge({"desert_tracks", "ÉPREUVE DE CALCUL SEC", "Tu notes 7 groupes de 6 traces. Combien de traces cela fait-il ?", {{1, "36"}, {2, "42"}, {3, "48"}}, 2, "Comptage propre : tu identifies la vraie piste.", "Comptage faux : tu repères l'erreur un peu trop tard."});
        }
        else if (biome.name == "Quartier abandonné")
        {
            addBiomeChallenge({"district_paperwork", "ÉPREUVE DE PAPERASSE", "Sur un formulaire de guilde, quelle formulation est la plus propre ?", {{1, "Les documents ont été remis."}, {2, "Les document on été remit."}, {3, "Les documents a été remis."}}, 1, "Formulaire propre : la guilde pourra l'exploiter.", "Formulaire sale : il reste compréhensible, mais pénible à relire."});
        }
        else if (biome.name == "Mine sifflante")
        {
            addBiomeChallenge({"mine_vibration", "ÉPREUVE DE LOGIQUE", "Un rail vibre toutes les 4 secondes. Entre la première et la cinquième vibration, combien de secondes passent ?", {{1, "16 secondes"}, {2, "20 secondes"}, {3, "24 secondes"}}, 1, "Logique nickel : tu comprends le rythme de la mine.", "Logique bancale : la mine te donne la réponse en vibrant sous tes pieds."});
        }
        else if (biome.name == "Verger des lucioles de fer")
        {
            addBiomeChallenge({"orchard_light", "ÉPREUVE DE LUMIÈRE", "Trois lucioles clignotent 2, 4 puis 6 fois. Quel rythme semble logique ensuite ?", {{1, "7"}, {2, "8"}, {3, "12"}}, 2, "Suite propre : tu synchronises ta marche avec l'essaim.", "Suite ratée : les lucioles se dispersent."});
        }
        else if (biome.name == "Archives noyées")
        {
            addBiomeChallenge({"archives_sorting", "ÉPREUVE DE CLASSEMENT", "Une archive porte les cotes A-12, A-13 et A-15. Quelle cote manque probablement ?", {{1, "A-14"}, {2, "B-12"}, {3, "A-16"}}, 1, "Classement net : la page accepte d'être lue.", "Classement faux : l'archive se referme."});
        }
        else if (biome.name == "Falaises des drakes gris")
        {
            addBiomeChallenge({"cliffs_rope", "ÉPREUVE DE CORDE", "Tu as 24 mètres de corde et 4 points d'ancrage égaux. Combien de mètres par point ?", {{1, "5 mètres"}, {2, "6 mètres"}, {3, "8 mètres"}}, 2, "Ancrage propre : la falaise te respecte presque.", "Mauvais partage : la corde tient, mais ton cœur descend avant tes pieds.", 6});
            addBiomeChallenge({"cliffs_wind", "ÉPREUVE DE VENT", "Une rafale arrive depuis l'ouest. Où places-tu ton appui le plus solide ?", {{1, "Du côté ouest, contre la poussée"}, {2, "Sur une pierre mobile"}, {3, "Le plus loin possible de la paroi"}}, 1, "Appui propre : la rafale passe sans t'arracher à la corniche.", "Appui mauvais : tu récupères ton équilibre au prix d'une belle frayeur."});
        }
        else if (biome.name == "Foire abandonnée")
        {
            addBiomeChallenge({"fair_french", "ÉPREUVE DE FRANÇAIS FORAIN", "Quel panneau est écrit correctement ?", {{1, "Les tickets sont valables."}, {2, "Les ticket sont valable."}, {3, "Les tickets est valables."}}, 1, "Panneau propre : même la caisse semble moins te juger.", "Panneau faux : la foire applaudit probablement pour se moquer."});
        }
        else if (biome.name == "Temple des cloches fendues")
        {
            addBiomeChallenge({"temple_bells", "ÉPREUVE DE SERMENT", "Une cloche sonne 3 fois, puis 6, puis 9. Combien devrait-elle sonner ensuite ?", {{1, "10"}, {2, "12"}, {3, "18"}}, 2, "Rythme compris : la cloche cesse de vibrer juste assez longtemps.", "Rythme raté : la cloche attire des regards invisibles."});
        }
        else if (biome.name == "Canaux de brume bleue")
        {
            addBiomeChallenge({"canals_crates", "ÉPREUVE DE PASSAGE", "Un bac porte 3 caisses par traversée. Combien de traversées chargées pour 9 caisses ?", {{1, "2"}, {2, "3"}, {3, "4"}}, 2, "Calcul net : les caisses passent sans voyage inutile.", "Calcul faux : tu comprends pourquoi les passeurs facturent au trajet."});
        }
        else if (biome.name == "Carrière des os blancs")
        {
            addBiomeChallenge({"quarry_measure", "ÉPREUVE DE MESURE", "Une trace mesure 40 cm. Une autre est deux fois plus grande. Combien mesure la deuxième ?", {{1, "60 cm"}, {2, "80 cm"}, {3, "120 cm"}}, 2, "Mesure propre : tu sais quand une empreinte devient inquiétante.", "Mesure bancale : la carrière paraît soudain moins vide."});
        }
        else if (biome.name == "Marché sous les ponts")
        {
            addBiomeChallenge({"market_contract", "ÉPREUVE DE CONTRAT", "Quelle phrase évite le mieux une arnaque dans un reçu ?", {{1, "Payé après livraison vérifiée."}, {2, "Payer quand le vendeur dit que c'est bon."}, {3, "Payé peut-être demain hier."}}, 1, "Reçu propre : même le vendeur douteux respecte ton sérieux.", "Reçu faible : tu viens peut-être d'acheter une explication."});
        }
        else if (biome.name == "Jardin des statues qui pleurent")
        {
            addBiomeChallenge({"garden_statues", "ÉPREUVE D'OBSERVATION", "Trois statues regardent la fontaine, sauf une qui regarde la sortie. Laquelle surveiller ?", {{1, "Celle qui regarde la sortie"}, {2, "La plus jolie"}, {3, "Aucune"}}, 1, "Observation utile : tu repères celle qui connaît ton chemin de fuite.", "Observation ratée : le jardin change quand tu clignes des yeux."});
        }

        std::vector<std::size_t> availableIndexes;
        for (std::size_t index = 0; index < challenges.size(); ++index)
        {
            const std::string cooldownKey = "challenge:" + challenges[index].id;
            if (!player.wasExplorationChallengeRecentlySeen(challenges[index].id)
                && !player.isExplorationSceneOnCooldown(cooldownKey))
            {
                availableIndexes.push_back(index);
            }
        }
        if (availableIndexes.empty())
        {
            for (std::size_t index = 0; index < challenges.size(); ++index)
            {
                if (!player.isExplorationSceneOnCooldown("challenge:" + challenges[index].id))
                {
                    availableIndexes.push_back(index);
                }
            }
        }
        if (availableIndexes.empty())
        {
            for (std::size_t index = 0; index < challenges.size(); ++index) availableIndexes.push_back(index);
        }

        const std::size_t selectedIndex = availableIndexes[static_cast<std::size_t>(random.between(0, static_cast<int>(availableIndexes.size()) - 1))];
        const Challenge& challenge = challenges[selectedIndex];
        player.recordExplorationChallengeKey(challenge.id);
        player.startExplorationSceneCooldown("challenge:" + challenge.id, challenge.cooldownDays);

        int choice = askChoiceScreen(
            challenge.title,
            "exploration.micro_challenge." + challenge.id,
            {
                "Avant de continuer, la zone demande un petit choix actif.",
                challenge.question
            },
            challenge.options,
            1,
            3
        );
        Console::clear();

        MicroChallengeResult result;
        result.success = choice == challenge.correctChoice;
        result.lines.push_back(result.success ? challenge.successLine : challenge.failureLine);
        result.lines.push_back(result.success
            ? "Bonus : la suite de l'exploration est légèrement mieux préparée."
            : "Conséquence : rien de dramatique, mais la suite devient un peu moins propre.");
        return result;
    }

    MicroChallengeResult runGuildServiceMicroChallenge(Quest& quest, Random& random)
    {
        const std::string questText = toLowerChoiceText(quest.title + " " + quest.objective + " " + quest.location + " " + quest.targetFamily + " " + quest.client);

        struct Challenge
        {
            Challenge(
                std::string challengeTitle,
                std::string challengeQuestion,
                std::vector<std::pair<int, std::string>> challengeOptions,
                int challengeCorrectChoice,
                std::string challengeSuccessLine,
                std::string challengeFailureLine
            )
                : title(std::move(challengeTitle)),
                  question(std::move(challengeQuestion)),
                  options(std::move(challengeOptions)),
                  correctChoice(challengeCorrectChoice),
                  successLine(std::move(challengeSuccessLine)),
                  failureLine(std::move(challengeFailureLine))
            {
            }

            std::string title;
            std::string question;
            std::vector<std::pair<int, std::string>> options;
            int correctChoice = 1;
            std::string successLine;
            std::string failureLine;
            std::string id;
            std::string family;
            bool unusualDocument = false;
            std::string handlingQuestion;
            std::vector<std::pair<int, std::string>> handlingOptions;
            int correctHandlingChoice = 0;
            std::string handlingSuccessLine;
            std::string handlingFailureLine;
        };

        std::vector<Challenge> merchantSelectedChallenges;

        std::vector<Challenge> challenges = {
            {
                "PETITE PAPERASSE",
                "La guilde demande un total : 12 formulaires reçus, 4 refusés, 3 corrigés. Combien sont exploitables ?",
                {{1, "8"}, {2, "11"}, {3, "15"}},
                2,
                "Compte juste : le dossier passe sans aller-retour inutile.",
                "Compte faux : la gérante te rend la pile avec un regard de boss final administratif."
            },
            {
                "CORRECTION RAPIDE",
                "Quelle phrase est correcte dans le rapport ?",
                {{1, "Les caisses ont été livrées."}, {2, "Les caisse on été livrer."}, {3, "Les caisses a été livrées."}},
                1,
                "Phrase propre : le rapport peut être tamponné.",
                "Phrase ratée : le rapport reste compréhensible, mais pas tamponnable pour l'instant."
            },
            {
                "LOGIQUE DE GUILDE",
                "Un client doit signer avant le forgeron, et le forgeron avant la guilde. Quel ordre est correct ?",
                {{1, "Guilde > Forgeron > Client"}, {2, "Client > Forgeron > Guilde"}, {3, "Forgeron > Guilde > Client"}},
                2,
                "Ordre correct : le service avance vraiment.",
                "Ordre faux : personne ne signe, mais tout le monde perd du temps. Classic."
            },
            {
                "CALCUL DE REÇU",
                "Une facture indique 3 lots à 14 pièces. Total ?",
                {{1, "38 pièces"}, {2, "42 pièces"}, {3, "44 pièces"}},
                2,
                "Calcul juste : la guilde n'a rien à redire.",
                "Calcul faux : le reçu repart dans la pile maudite."
            },
            {
                "TAMPON MANQUANT",
                "Un dossier doit passer par Accueil, Vérification, puis Archive. Quel ordre est valide ?",
                {{1, "Archive > Accueil > Vérification"}, {2, "Accueil > Vérification > Archive"}, {3, "Vérification > Archive > Accueil"}},
                2,
                "Ordre nickel : le dossier évite le labyrinthe administratif.",
                "Ordre faux : le dossier revient avec plus de papier qu'au départ."
            },
            {
                "ERREUR DE FORMULAIRE",
                "Quelle phrase est correcte ?",
                {{1, "Les colis ont été pesés."}, {2, "Les colis on été peser."}, {3, "Les colis a été pesés."}},
                1,
                "Correction propre : la gérante tamponne sans soupirer.",
                "Correction ratée : la gérante soupire tellement fort que la quête perd 1 de dignité."
            },
            {
                "PETIT CALCUL DE PRIME",
                "Une prime de 80 pièces est partagée entre 4 porteurs. Combien chacun reçoit ?",
                {{1, "18 pièces"}, {2, "20 pièces"}, {3, "24 pièces"}},
                2,
                "Partage juste : personne ne crie au vol, ce qui est rare.",
                "Partage faux : même les gobelins trouveraient ça suspect."
            },
            {
                "REGISTRE DE LIVRAISON",
                "Une livraison comporte 5 caisses de fioles et 2 caisses de bandages. Combien de caisses noter au registre ?",
                {{1, "7"}, {2, "10"}, {3, "3"}},
                1,
                "Registre clair : la réserve sait enfin ce qu'elle possède.",
                "Registre faux : la réserve gagne une nouvelle légende administrative."
            },
            {
                "FAUTE DE RAPPORT",
                "Quelle phrase est correcte ?",
                {{1, "Le client a signé le reçu."}, {2, "Le client à signer le reçu."}, {3, "Le client a signé le reçus."}},
                1,
                "Phrase propre : le reçu peut rejoindre les archives sans honte.",
                "Phrase douteuse : l'archive accepte, mais elle jugera."
            },
            {
                "ORDRE DE TOURNÉE",
                "Tu dois passer au dépôt, au client, puis à la guilde. Quel ordre respecte la demande ?",
                {{1, "Client > Guilde > Dépôt"}, {2, "Dépôt > Client > Guilde"}, {3, "Guilde > Dépôt > Client"}},
                2,
                "Tournée efficace : tu évites l'aller-retour inutile qui donne envie de quitter la guilde.",
                "Tournée ratée : tu viens d'inventer la boucle administrative infinie."
            }
        };

        const bool merchantPaperwork = questText.find("marchand") != std::string::npos
            || questText.find("prunigil") != std::string::npos
            || questText.find("comptoir") != std::string::npos
            || questText.find("registre") != std::string::npos
            || questText.find("facture") != std::string::npos
            || questText.find("monnaie") != std::string::npos
            || questText.find("caravane") != std::string::npos
            || questText.find("client") != std::string::npos;

        if (questText.find("tri de sac") != std::string::npos
            || questText.find("inventaire trop") != std::string::npos
            || questText.find("sac trop") != std::string::npos)
        {
            challenges = {
                {
                    "TRI DE SAC — POIDS ET VALEUR",
                    "La caravane accepte 30 kg. Quel lot est le plus logique à garder ?",
                    {{1, "Minerai dense 25 kg + vieille épée rouillée 10 kg"}, {2, "Rations 5 kg + coffret scellé 4 kg + potion fragile 2 kg"}, {3, "Tout prendre, la caravane comprendra"}},
                    2,
                    "Tri propre : les objets utiles et fragiles sont protégés sans dépasser la limite.",
                    "Tri raté : le sac devient une punition logistique avant même le départ."
                },
                {
                    "TRI DE SAC — OBJET SUSPECT",
                    "Un coffret scellé pèse peu, vaut inconnu et porte une marque de quête. Que faire ?",
                    {{1, "Le vendre au premier marchand"}, {2, "Le jeter pour gagner du poids"}, {3, "Le signaler et le garder isolé jusqu'au client"}},
                    3,
                    "Objet suspect isolé : personne ne perd une preuve importante pour deux kilos de confort.",
                    "Objet suspect mal traité : le client sent déjà la catastrophe administrative."
                }
            };
        }

        if (questText.find("armure mal ajust") != std::string::npos
            || questText.find("sangles") != std::string::npos
            || questText.find("morphologie") != std::string::npos)
        {
            challenges = {
                {
                    "ARMURE — SEMI-PIAF",
                    "Une armure bloque les ailes d'un semi-piaf. Quelle adaptation est cohérente ?",
                    {{1, "Serrer les épaules pour qu'il bouge moins"}, {2, "Ouvrir et protéger les passages d'ailes"}, {3, "Ajouter du poids pour stabiliser le vol"}},
                    2,
                    "Ajustement propre : les ailes bougent sans transformer l'armure en passoire.",
                    "Ajustement raté : le client pourra peut-être marcher, mais sûrement pas voler."
                },
                {
                    "ARMURE — DEMI-DRAGON",
                    "Une cuirasse frotte contre les écailles d'un demi-dragon. Quelle solution évite l'usure ?",
                    {{1, "Doublure anti-friction et plaques mobiles"}, {2, "Plus de sangles serrées sur les écailles"}, {3, "Tissu fragile et très inflammable"}},
                    1,
                    "Morphologie comprise : l'équipement respecte les écailles au lieu de les poncer.",
                    "Morphologie ignorée : le forgeron t'enlève mentalement son titre d'artisan."
                },
                {
                    "ARMURE — QUEUE ET ÉQUILIBRE",
                    "Un semi-chat perd l'équilibre avec une ceinture trop basse. Que corriger ?",
                    {{1, "Laisser un passage de queue et répartir le poids"}, {2, "Bloquer la queue sous la ceinture"}, {3, "Ajouter une plaque lourde d'un seul côté"}},
                    1,
                    "Équilibre sauvé : la queue n'est pas traitée comme un accessoire décoratif.",
                    "Équilibre massacré : le client marche comme une chaise bancale."
                }
            };
        }

        if (merchantPaperwork)
        {
            std::vector<Challenge> merchantChallenges = {
                {
                    "MARCHAND — STOCK DE POTIONS",
                    "Stock : 7 potions à 12 fer, 3 potions à 2 électrum, 1 potion à 1 or et 5 électrum. Valeur totale ?",
                    {{1, "2 490 cuivre"}, {2, "2 940 cuivre"}, {3, "3 040 cuivre"}},
                    2,
                    "Compte juste : Prunigil arrête de regarder ses potions comme si elles allaient mentir.",
                    "Compte faux : les potions valent soudainement plus cher que la boutique, ce qui inquiète tout le monde."
                },
                {
                    "MARCHAND — CONVERSION",
                    "2 940 cuivre se convertissent comment avec 1 or = 10 électrum = 100 fer = 1000 cuivre ?",
                    {{1, "2 or, 9 électrum, 4 fer"}, {2, "2 or, 4 électrum, 9 fer"}, {3, "29 électrum, 40 cuivre"}},
                    1,
                    "Conversion propre : les pièces arrêtent de former une montagne inutile.",
                    "Conversion ratée : Prunigil soupire, puis recompte absolument tout depuis le début."
                },
                {
                    "MARCHAND — FAIRE LA MONNAIE",
                    "Un aventurier achète pour 3 électrum et 6 fer. Il paie avec 1 or. Combien rendre ?",
                    {{1, "460 cuivre"}, {2, "540 cuivre"}, {3, "640 cuivre"}},
                    3,
                    "Monnaie juste : le client ne peut pas prétendre que le marchand l'a volé.",
                    "Monnaie fausse : même la caisse semble vouloir te dénoncer."
                },
                {
                    "MARCHAND — MONNAIE MINIMALE",
                    "Pour rendre 640 cuivre avec le moins de pièces, quelle solution est logique ?",
                    {{1, "6 électrum et 4 fer"}, {2, "64 fer"}, {3, "640 cuivre"}},
                    1,
                    "Rendu efficace : peu de pièces, peu de drame.",
                    "Rendu nul : tu viens d'inventer le sac de monnaie le plus relou du royaume."
                },
                {
                    "MARCHAND — RÉDUCTION",
                    "25 rations à 15 cuivre coûtent 375 cuivre. Avec 10% de remise arrondie à 38 cuivre, prix final ?",
                    {{1, "337 cuivre"}, {2, "345 cuivre"}, {3, "413 cuivre"}},
                    1,
                    "Remise correcte : le gros client est content sans ruiner le marchand.",
                    "Remise fausse : quelqu'un va finir par appeler ça une arnaque pédagogique."
                },
                {
                    "MARCHAND — TAXE DU ROYAUME",
                    "La taxe est de 12% sur 3 or et 5 électrum, soit 3 500 cuivre. Taxe correcte ?",
                    {{1, "350 cuivre"}, {2, "420 cuivre"}, {3, "520 cuivre"}},
                    2,
                    "Taxe juste : le royaume ne viendra pas renifler le registre ce soir.",
                    "Taxe fausse : tu sens déjà l'ombre d'un contrôleur fiscal médiéval."
                },
                {
                    "MARCHAND — DEUX OFFRES",
                    "Fournisseur A : 40 bottes pour 2 or. Fournisseur B : 30 bottes pour 1 or et 5 électrum. Meilleure offre ?",
                    {{1, "A, car 40 bottes c'est plus grand"}, {2, "B, car 1 or semble moins cher"}, {3, "Aucune : les deux coûtent 50 cuivre par botte"}},
                    3,
                    "Piège évité : le nombre de bottes ne t'a pas hypnotisé.",
                    "Piège réussi : le marchand note 'facile à embrouiller' dans la marge."
                },
                {
                    "MARCHAND — REGISTRE AVEC ERREUR",
                    "Vente 1 : 2 potions à 12 fer = 24 fer. Vente 2 : 1 potion à 2 électrum = 2 électrum. Vente 3 : 5 antidotes à 18 cuivre = 90 cuivre. Où est l'erreur ?",
                    {{1, "Vente 1"}, {2, "Vente 3"}, {3, "Aucune, tout est correct"}},
                    3,
                    "Registre validé : parfois le piège, c'est qu'il n'y a pas de piège.",
                    "Erreur inventée : Prunigil te regarde comme si tu venais de créer une faute."
                },
                {
                    "MARCHAND — BÉNÉFICE DES GEMMES",
                    "10 gemmes achetées 4 électrum chacune, revendues 6 électrum et 5 fer chacune. Bénéfice total ?",
                    {{1, "1 or et 5 électrum"}, {2, "2 or et 5 électrum"}, {3, "3 or"}},
                    2,
                    "Bénéfice propre : Prunigil sourit, ce qui reste assez rare pour être noté.",
                    "Bénéfice faux : les gemmes deviennent mentalement plus dangereuses que des slimes."
                },
                {
                    "MARCHAND — CARAVANE À PAYER",
                    "4 gardes gagnent 8 fer par jour pendant 12 jours, puis 3 électrum pour le maître de caravane. Coût total ?",
                    {{1, "3 or, 8 électrum, 4 fer"}, {2, "4 or, 1 électrum, 4 fer"}, {3, "4 or, 4 électrum, 1 fer"}},
                    2,
                    "Caravane chiffrée : elle coûte cher, mais au moins tu sais pourquoi.",
                    "Caravane mal chiffrée : les gardes commencent à compter eux-mêmes, très mauvais signe."
                },
                {
                    "MARCHAND — RENDU DE MONNAIE",
                    "Un client paie 1 or pour une commande de 7 fer. Combien dois-tu rendre ?",
                    {{1, "9 électrum et 3 fer"}, {2, "7 fer"}, {3, "1 électrum"}},
                    1,
                    "Rendu propre : le client repart sans compter chaque pièce sous ton nez.",
                    "Rendu faux : Prunigil te regarde comme un coffre qui fuit."
                },
                {
                    "MARCHAND — LOT DE POTIONS",
                    "Une potion vaut 12 fer. Un lot de 3 potions coûte combien ?",
                    {{1, "36 fer"}, {2, "15 fer"}, {3, "3 or et 12 fer"}},
                    1,
                    "Lot chiffré : les potions restent dangereuses, mais la facture non.",
                    "Lot raté : la potion n'a même pas besoin d'effet secondaire pour faire mal."
                },
                {
                    "MARCHAND — REMISE DE FIDÉLITÉ",
                    "Une commande de 100 fer reçoit une remise de 10 fer. Montant final ?",
                    {{1, "90 fer"}, {2, "110 fer"}, {3, "10 fer"}},
                    1,
                    "Remise correcte : le client croit presque que la boutique est généreuse.",
                    "Remise ratée : la fidélité vient de perdre sa définition."
                },
                {
                    "MARCHAND — STOCK CASSÉ",
                    "Il y avait 18 fioles. 5 sont cassées, 4 sont vendues. Combien restent en stock ?",
                    {{1, "9"}, {2, "13"}, {3, "27"}},
                    1,
                    "Stock net : les fioles survivantes applaudissent en silence.",
                    "Stock faux : même les morceaux de verre se sentent mal comptés."
                },
                {
                    "MARCHAND — UNITÉ DE COMPTE",
                    "Quelle notation est la plus lisible sur une facture de ville ?",
                    {{1, "2 or, 4 électrum, 6 fer"}, {2, "2 gros trucs jaunes et des petites pièces"}, {3, "beaucoup, mais pas trop"}},
                    1,
                    "Notation claire : la banque peut lire sans invoquer un oracle.",
                    "Notation foireuse : la facture devient une énigme, donc invendable."
                },
                {
                    "FRANÇAIS — NOTE DE MARCHAND",
                    "Quelle correction est la plus propre ? 'J’ai reçu 14 caisse de blé, mais seulment 3 étais remplis...'",
                    {{1, "J’ai reçu 14 caisses de blé, mais seulement 3 étaient remplies."}, {2, "J’ai reçu 14 caisse de blé, mais seulement 3 était rempli."}, {3, "J’ai reçus 14 caisses de blé, mais seulemant 3 étais remplit."}},
                    1,
                    "Correction claire : même le blé paraît moins perdu.",
                    "Correction douteuse : les caisses restent grammaticalement traumatisées."
                },
                {
                    "FRANÇAIS — FACTURE NON FINALISÉE",
                    "La note dit : « Vente : 7 potions à 12 fer. Montant total : 84 [unité manquante] ». Quelle unité complète correctement le total ?",
                    {{1, "fer"}, {2, "feuilles"}, {3, "platines"}},
                    1,
                    "Unité corrigée : 7 potions à 12 fer, ça donne bien 84 fer.",
                    "Unité ratée : payer en feuilles reste interdit, même si c'est joli."
                },
                {
                    "FRANÇAIS — MESSAGE PRESSÉ",
                    "Quelle phrase est correcte ?",
                    {{1, "Urgent ! La caravane doit partir à l’aube, sinon le convoi sera compromis. Prévenez les gardes."}, {2, "Urgent ! la caravane doit partir a l’aube sinon le convoi sera compromi."}, {3, "Urgent ! La caravanes doit partire à l’aube sinon les garde sera compromis."}},
                    1,
                    "Message net : les gardes comprennent avant que le convoi parte sans eux.",
                    "Message bancal : le convoi est déjà compromis par la grammaire."
                },
                {
                    "FRANÇAIS — REGISTRE INCOMPLET",
                    "Dans 'j’ai oublié de noté le nom ?? client', quelle correction est la meilleure ?",
                    {{1, "de noter le nom du client"}, {2, "de noté le nom de client"}, {3, "de noter le nom des client"}},
                    1,
                    "Registre lisible : le client redevient une personne, pas une énigme.",
                    "Registre raté : le client reste anonyme, ce qui arrange surtout les mauvais payeurs."
                },
                {
                    "FRANÇAIS — LETTRE DE PLAINTE",
                    "Quelle correction garde le sens ?",
                    {{1, "Je vous signale un problème : une ration était moisie, je demande un remboursement."}, {2, "Je vous signale un problèm : une rations avais moisie."}, {3, "Je vous signales un problème : une ration avais moisies."}},
                    1,
                    "Plainte propre : le remboursement devient au moins discutable.",
                    "Plainte sale : même la ration moisie a honte."
                },
                {
                    "FRANÇAIS — LISTE DE STOCK",
                    "Quel objet complète le mieux '1 tonneau de [objet manquant]' dans un stock de marchand ?",
                    {{1, "vinaigre"}, {2, "silence administratif"}, {3, "probablement boss final"}},
                    1,
                    "Stock crédible : le tonneau peut être rangé sans prière.",
                    "Stock absurde : Prunigil refuse de vendre un boss final au litre."
                },
                {
                    "FRANÇAIS — ORDRE DE LIVRAISON",
                    "Quelle correction est la meilleure ?",
                    {{1, "Livrez au plus vite. Les emballages doivent être fermés correctement cette fois."}, {2, "Livré au plus vite. Les emballages doivent être fermer correctement."}, {3, "Livrez au plus vite. Les emballage doit être fermé correctement."}},
                    1,
                    "Ordre clair : les colis ont une chance de survivre.",
                    "Ordre flou : les colis préparent déjà leur chute."
                },
                {
                    "FRANÇAIS — NOTE INTERNE",
                    "Quelle phrase est correcte ?",
                    {{1, "N’oubliez pas de payer le garde du portail. Il se plaint depuis 2 jours qu’il n’a pas reçu sa solde."}, {2, "N’oublier pas de payer le garde. Il ce pleind depuis 2 jours."}, {3, "N'oubliez pas de payé le gardes. Il se plaint qu’il a pas reçus ça solde."}},
                    1,
                    "Note propre : le garde arrêtera peut-être de menacer la porte.",
                    "Note ratée : la solde se perd encore dans la syntaxe."
                },
                {
                    "FRANÇAIS — RAPPORT DE BANQUE",
                    "Quelle fin est correcte ? 'impossible de trouver...'",
                    {{1, "d’où vient l’erreur"}, {2, "d’ou vien l’erreure"}, {3, "d’où viens les erreurs"}},
                    1,
                    "Banque rassurée : l'erreur reste financière, pas orthographique.",
                    "Banque inquiète : le rapport perd encore 212 cuivres de dignité."
                },
                {
                    "FRANÇAIS — ÉTIQUETTE DE POTION",
                    "Quelle étiquette est la plus correcte ?",
                    {{1, "Effets secondaires possibles : tremblements, douleurs ou perte de conscience. Ne pas avaler plus de 2 par jour."}, {2, "Effait secondaire possible : trembloement, doulour. Ne pas avalé plus de 2 par jours."}, {3, "Effets secondaire possibles : tremblement, douleur. Ne pas avaler plus de 2 par jours."}},
                    1,
                    "Étiquette utile : quelqu'un évitera peut-être la troisième potion stupide.",
                    "Étiquette ratée : la potion semble corriger le lecteur en retour."
                },
                {
                    "FRANÇAIS — MESSAGE CODÉ FOIRÉ",
                    "Quelle correction est la plus propre ?",
                    {{1, "La clef est cachée dans le coffre, mais ne dis rien au marchand."}, {2, "La clef est cachée dans le ??, mais ne dit rien au marchant."}, {3, "La clef est cacher dans le coffre, mais ne dis rien au marchant."}},
                    1,
                    "Secret propre : au moins le complot sait écrire coffre.",
                    "Secret raté : même le message codé demande un correcteur."
                },
                {
                    "FRANÇAIS — JOURNAL PERSONNEL",
                    "Quelle phrase corrige le mieux ?",
                    {{1, "Je pense que quelqu’un me surveille. Les caisses bougent la nuit, j’en suis sûr."}, {2, "Je panse que quelqu’un me surveille. Les caisse bouge la nuit."}, {3, "Je pense que quelqu’un me surveilles. Les caisses bouge la nuit."}},
                    1,
                    "Journal propre : paranoïa lisible, c'est déjà ça.",
                    "Journal raté : les caisses gagnent contre la grammaire."
                },
                {
                    "FRANÇAIS — DEMANDE URGENTE",
                    "Quelle correction est la meilleure ?",
                    {{1, "Envoyez une équipe ! Le chariot s’est renversé, les caisses sont éventrées, tout part en miettes !"}, {2, "Envoyé une équipe ! Le chariot c’est renvrsé."}, {3, "Envoyez une équipe ! Les caisses sons éventré, tout par en miette."}},
                    1,
                    "Demande claire : l'équipe peut partir avant que tout devienne purée.",
                    "Demande ratée : le chariot n'est plus le seul renversé."
                },
                {
                    "FRANÇAIS — ENTRÉE COMPTABLE",
                    "Reçu 12 fer, dépensé 4 fer. Quelle différence faut-il noter ?",
                    {{1, "7 fer"}, {2, "8 fer"}, {3, "16 fer"}},
                    2,
                    "Calcul corrigé en note : le message original reste archivé, mais le compte est sauvé.",
                    "Calcul faux : le comptable commence à voir les chiffres danser."
                },
                {
                    "FRANÇAIS — CONTREMAÎTRE",
                    "Quelle correction est la meilleure ?",
                    {{1, "Les apprentis ont fait n’importe quoi, il faut tout recommencer depuis le début."}, {2, "Les apprenti on fait n’importe quoi, faut tout recomenssé depuis le débue."}, {3, "Les apprentis ont fait n’importe quoi, faut tout recommensé depuis le début."}},
                    1,
                    "Note corrigée : le chantier reste nul, mais lisible.",
                    "Note ratée : il faut aussi recommencer la phrase."
                },
                {
                    "FRANÇAIS — FORMULAIRE RATÉ",
                    "Quelle ligne est la plus propre ?",
                    {{1, "Profession : Livraison d’objets | Motif : Réclamation de remboursement"}, {2, "Profession : Livraysson d’objé | Motif : Reclamassion de remboussemement"}, {3, "Profession : Livraison d’objet | Motif : Réclamassion de remboursement"}},
                    1,
                    "Formulaire sauvé : même la bave noire paraît plus professionnelle.",
                    "Formulaire perdu : la bave noire reste la partie la plus claire."
                },
                {
                    "FRANÇAIS — NOTE DE L’APPRENTI",
                    "Quelle correction est la meilleure ?",
                    {{1, "J’ai essayé de ranger les caisses, mais le sol était trop glissant et je suis tombé."}, {2, "J’ai essayer de ranger les caisse, mais le sol étais trop glissant."}, {3, "J’ai essayé de rangé les caisses, mais j’ai tomber."}},
                    1,
                    "Apprenti compris : il est nul, mais on sait pourquoi.",
                    "Apprenti illisible : le blé explose une deuxième fois."
                },
                {
                    "FRANÇAIS — LETTRE AU MARCHAND",
                    "Quelle correction est la plus correcte ?",
                    {{1, "Monsieur le marchand, votre employé n’est pas compétent. Il m’a vendu une potion qui m’a fait du feu dans la bouche."}, {2, "Monsieure le marchan, votre employé n’est pas compaitant."}, {3, "Monsieur le marchand, il ma vendu une potion qui ma fais du feu."}},
                    1,
                    "Lettre propre : la plainte brûle moins que la potion.",
                    "Lettre ratée : la potion a visiblement touché la grammaire aussi."
                },
                {
                    "FRANÇAIS — FACTURE FOIRÉE",
                    "Une facture indique '3 or et 41 feuilles'. Que faut-il signaler ?",
                    {{1, "Les feuilles ne sont pas une monnaie officielle du système local."}, {2, "41 feuilles valent 4 électrum."}, {3, "C'est forcément un paiement noble."}},
                    1,
                    "Facture signalée : les arbres ne remplacent pas encore la banque.",
                    "Facture acceptée : le marchand vient d'être payé en automne."
                },
                {
                    "FRANÇAIS — ACHAT SUSPECT",
                    "Quelle correction est la plus propre ?",
                    {{1, "Acheté 7 frigo-froid à un vendeur itinérant. Je ne suis pas sûr que ça existe, mais il était convaincant."}, {2, "Acheté 7 frigo-froid à un vendeur itinairaire."}, {3, "Acheter 7 frigo-froid a un vendeur convainquand."}},
                    1,
                    "Note propre : l'objet reste suspect, mais le rapport est lisible.",
                    "Note ratée : le frigo-froid gagne en crédibilité par comparaison."
                },
                {
                    "FRANÇAIS — JOURNAL DE BORD",
                    "Quelle correction est la meilleure ?",
                    {{1, "Le maître de caravane dit que les roues sont fatiguées."}, {2, "Le maitre de caravane dit que les roue sont fatigué."}, {3, "Le maître de caravane dit que des roues pouvait être fatigue."}},
                    1,
                    "Journal propre : personne ne sait si les roues sont vraiment fatiguées, mais c'est écrit correctement.",
                    "Journal raté : même les roues demandent une pause."
                },
                {
                    "FRANÇAIS — BIDON BLEU",
                    "Quelle correction est la meilleure ?",
                    {{1, "NE PAS TOUCHER LE BIDON BLEU !!! J’ai respiré dedans et j’ai eu des hallucinations d’un lapin géant violet."}, {2, "NE PAS TOUCHÉ LE BIDON BLEU !!! j’ai réspirez dedans."}, {3, "Ne pas toucher le bidon bleu, j’ai eu des allusinassion."}},
                    1,
                    "Avertissement clair : le lapin géant reste inquiétant, mais documenté.",
                    "Avertissement raté : le bidon a gagné le combat contre l'orthographe."
                },
                {
                    "FRANÇAIS — COFFRE MAUDIT",
                    "Quelle correction est la meilleure ?",
                    {{1, "Je vous préviens : le coffre du fond est maudit. Quand je l’ai touché, il m’a parlé."}, {2, "Je vous prévien, le coffre du fond il est maudi."}, {3, "Je vous préviens, quand je l’ai toucher il ma parler."}},
                    1,
                    "Lettre dramatique propre : le coffre peut maintenant nier avec élégance.",
                    "Lettre ratée : le coffre parle peut-être mieux que l'auteur."
                },
                {
                    "FRANÇAIS — LISTE DE PRIX",
                    "Quelle ligne est correcte ?",
                    {{1, "Potion bleue : 1 électrum"}, {2, "Potion bleu : 1 electom"}, {3, "Potion bleue : 1 électom"}},
                    1,
                    "Prix corrigé : la boutique peut ouvrir sans provoquer une guerre des accents.",
                    "Prix raté : l'électom n'existe toujours pas."
                },
                {
                    "FRANÇAIS — REGISTRE CASSÉ",
                    "4 bottes de plantes à 13 fer la botte. Le total de 52 fer est-il juste ?",
                    {{1, "Oui, le calcul est correct"}, {2, "Non, il faut 48 fer"}, {3, "Non, il faut 56 fer"}},
                    1,
                    "Calcul validé : cette fois le doute était plus cassé que le registre.",
                    "Calcul inventé : le registre était bancal, mais pas à cet endroit."
                },
                {
                    "FRANÇAIS — MESSAGE AU COLLÈGUE",
                    "Quelle correction est la plus propre ?",
                    {{1, "Pense à prendre les clefs du magasin. Hier, tu les as oubliées et j’ai dû passer par la fenêtre."}, {2, "Pense a prend les clef du magassin."}, {3, "Pense à prendre les clefs, hier tu les a oublier."}},
                    1,
                    "Message propre : les échardes deviennent au moins une preuve.",
                    "Message raté : la fenêtre refuse d'être impliquée."
                },
                {
                    "FRANÇAIS — NOTE AU COMPTABLE",
                    "Quelle correction est la meilleure ?",
                    {{1, "Les nombres n’arrêtaient pas de danser devant mes yeux, je n’en peux plus."}, {2, "Les nombres arrêtez pas de dansé devant mes yeu."}, {3, "Les nombres n'arrêter pas de danser devant mes yeux."}},
                    1,
                    "Note claire : le comptable est perdu, mais correctement.",
                    "Note ratée : les chiffres dansent encore plus fort."
                },
                {
                    "FRANÇAIS — PETITE INSCRIPTION",
                    "Quelle correction est la meilleure ?",
                    {{1, "Si tu lis ça, remets le parchemin sur la table. Je te vois."}, {2, "Si tu lit sa, remet le parchmin sur la table."}, {3, "Si tu lis sa, remet le parchemin sur la tables."}},
                    1,
                    "Inscription propre : elle reste flippante, mais propre.",
                    "Inscription ratée : le parchemin te juge en silence."
                },
                {
                    "FRANÇAIS — CONTRAT CHAOTIQUE",
                    "Quelle correction est la plus professionnelle ?",
                    {{1, "Par la présente, je soussigné, Prunigil, marchand, promets à… [nom du client manquant]."}, {2, "Par la présente je ssoussigné le marchan Prunigil promet a qui déjà ???"}, {3, "Par la présente, je sous-signé le marchan Prunigil promet à quelqu'un."}},
                    1,
                    "Contrat corrigé : l'identité manquante reste signalée au lieu d'être inventée.",
                    "Contrat raté : juridiquement, même un gobelin refuserait de signer."
                },
                {
                    "FRANÇAIS — MISE À JOUR DE CLIENT",
                    "Quelle correction transmet correctement la nouvelle demande ?",
                    {{1, "Le client de la commande 17 est revenu. Il souhaite remplacer deux caisses et conserver le reste de la livraison."}, {2, "Le client de la commande 17 est revenue. Il veux changé deux caisse."}, {3, "Le clients veut tout changer sauf ce qu'il garde."}},
                    1,
                    "Mise à jour claire : la nouvelle demande peut être reliée au premier dossier.",
                    "Mise à jour floue : personne ne sait quelles caisses doivent encore partir."
                },
                {
                    "FRANÇAIS — LETTRE DE RECOMMANDATION",
                    "Quelle formulation est assez professionnelle pour recommander un aide-marchand ?",
                    {{1, "Je recommande cet aventurier pour son sérieux au comptoir, sa discrétion et la précision de ses vérifications."}, {2, "Je recommande cette aventurier car il est pas trop mauvais avec les papier."}, {3, "Prenez-le, il compte mieux que mon dernier apprenti."}},
                    1,
                    "Recommandation propre : un autre vendeur peut la prendre au sérieux.",
                    "Recommandation ratée : le prochain marchand risque surtout de plaindre Prunigil."
                },
                {
                    "FRANÇAIS — AVIS DE DÉSTOCKAGE",
                    "Quelle annonce explique correctement l'offre sans tromper les clients ?",
                    {{1, "Déstockage pendant trois jours : jusqu'à quatre paires de bottes usées sont proposées à prix réduit, dans la limite du stock disponible."}, {2, "Solde pour toujours pendant 3 jour sur toute les bottes qu'on a peut être."}, {3, "Tout est gratuit jusqu'à épuisement de Prunigil."}},
                    1,
                    "Annonce honnête : durée, quantité et état des invendus sont indiqués.",
                    "Annonce trompeuse : la garde commerciale finira par demander des explications."
                },
                {
                    "FRANÇAIS — VENDEUR TEMPORAIRE",
                    "Quelle affiche indique clairement la présence du vendeur ?",
                    {{1, "Mirette sera présente au marché pendant deux jours. Son stock est limité et dépend des tissus qu'elle a pu transporter."}, {2, "Mirette sera la tout le temps pendant deux jours sauf quand elle repart."}, {3, "Une vendeuse viendra quelque part bientôt, demandez à Prunigil."}},
                    1,
                    "Affiche claire : les clients savent qui vient, où et pour combien de temps.",
                    "Affiche floue : le vendeur temporaire risque de repartir sans avoir été trouvé."
                },
                {
                    "MARCHAND — LOT D'INVENDUS",
                    "Un déstockage porte sur 4 objets identiques à 18 fer chacun avec 20% de réduction. Quel total doit payer le client si la remise est arrondie au cuivre inférieur par objet ?",
                    {{1, "57 fer et 6 cuivre"}, {2, "72 fer"}, {3, "14 fer et 4 cuivre"}},
                    1,
                    "Déstockage calculé : quatre invendus quittent enfin l'étagère sans fausser la caisse.",
                    "Déstockage faux : Prunigil vient de retrouver une raison de recompter toute la soirée."
                }
            };

            auto challengeIdFromTitle = [](const std::string& title)
            {
                std::string id;
                for (unsigned char c : title)
                {
                    if (std::isalnum(c)) id.push_back(static_cast<char>(std::tolower(c)));
                    else if (!id.empty() && id.back() != '_') id.push_back('_');
                }
                while (!id.empty() && id.back() == '_') id.pop_back();
                return std::string("merchant_") + id;
            };

            auto historyContains = [&](const std::string& id)
            {
                std::stringstream stream(quest.serviceChallengeHistory);
                std::string value;
                while (std::getline(stream, value, '|'))
                {
                    if (value == id) return true;
                }
                return false;
            };

            auto configureMerchantChallenge = [&](Challenge& challenge)
            {
                challenge.id = challengeIdFromTitle(challenge.title);
                const std::string text = toLowerChoiceText(challenge.title + " " + challenge.question);

                if (text.find("caravane") != std::string::npos || text.find("livraison") != std::string::npos
                    || text.find("chariot") != std::string::npos || text.find("roue") != std::string::npos
                    || text.find("péage") != std::string::npos || text.find("peage") != std::string::npos)
                {
                    challenge.family = "transport";
                }
                else if (text.find("luxe") != std::string::npos || text.find("brocante") != std::string::npos
                    || text.find("offre") != std::string::npos || text.find("fournisseur") != std::string::npos
                    || text.find("achat suspect") != std::string::npos)
                {
                    challenge.family = "estimation";
                }
                else if (text.find("contrat") != std::string::npos || text.find("registre") != std::string::npos
                    || text.find("facture") != std::string::npos || text.find("comptable") != std::string::npos
                    || text.find("formulaire") != std::string::npos || text.find("note") != std::string::npos)
                {
                    challenge.family = "registre";
                }
                else if (text.find("monnaie") != std::string::npos || text.find("taxe") != std::string::npos
                    || text.find("réduction") != std::string::npos || text.find("reduction") != std::string::npos
                    || text.find("bénéfice") != std::string::npos || text.find("benefice") != std::string::npos
                    || text.find("stock") != std::string::npos || text.find("prix") != std::string::npos
                    || text.find("potions") != std::string::npos)
                {
                    challenge.family = "calcul";
                }
                else
                {
                    challenge.family = "francais";
                }

                challenge.unusualDocument = text.find("bidon bleu") != std::string::npos
                    || text.find("coffre maudit") != std::string::npos
                    || text.find("frigo-froid") != std::string::npos
                    || text.find("journal personnel") != std::string::npos
                    || text.find("message codé") != std::string::npos
                    || text.find("message code") != std::string::npos
                    || text.find("petite inscription") != std::string::npos;

                if (text.find("message codé") != std::string::npos || text.find("message code") != std::string::npos
                    || text.find("journal personnel") != std::string::npos)
                {
                    challenge.handlingQuestion = "Le texte ressemble à un message personnel ou confidentiel. Que fais-tu après la correction ?";
                    challenge.handlingOptions = {
                        {1, "J'avertis Prunigil, je lui lis le message, puis je poursuis sans le recopier dans le registre public."},
                        {2, "Je le publie sur le comptoir pour que tout le monde puisse aider."},
                        {3, "Je le détruis sans prévenir personne."}
                    };
                    challenge.correctHandlingChoice = 1;
                    challenge.handlingSuccessLine = "Confidentialité respectée : Prunigil est averti sans transformer une note privée en affiche publique.";
                    challenge.handlingFailureLine = "Le texte est corrigé, mais son traitement reste mauvais : Prunigil refuse de valider l'étape complète.";
                }
                else if (text.find("entrée comptable") != std::string::npos || text.find("entree comptable") != std::string::npos)
                {
                    challenge.handlingQuestion = "Le message original indique 7 fer alors que le calcul donne 8 fer. Comment archiver la correction ?";
                    challenge.handlingOptions = {
                        {1, "Je conserve le message original et j'ajoute une note séparée indiquant la différence correcte de 8 fer."},
                        {2, "Je remplace directement le 7 par un 8 sans laisser de trace."},
                        {3, "Je laisse 7 fer pour ne pas vexer l'apprenti."}
                    };
                    challenge.correctHandlingChoice = 1;
                    challenge.handlingSuccessLine = "Correction traçable : le document original reste intact et la note comptable répare le calcul.";
                    challenge.handlingFailureLine = "Le calcul est compris, mais la méthode d'archive est mauvaise : l'étape reste partiellement traitée.";
                }
                else if (text.find("contrat chaotique") != std::string::npos)
                {
                    challenge.handlingQuestion = "Le nom du client manque encore. Quelle décision est professionnelle ?";
                    challenge.handlingOptions = {
                        {1, "Je refuse la validation finale tant que l'identité du client n'est pas confirmée."},
                        {2, "J'invente un nom plausible pour gagner du temps."},
                        {3, "Je signe quand même parce que Prunigil est pressé."}
                    };
                    challenge.correctHandlingChoice = 1;
                    challenge.handlingSuccessLine = "Contrat suspendu proprement : mieux vaut une signature tardive qu'un engagement sans destinataire.";
                    challenge.handlingFailureLine = "La phrase est meilleure, mais le contrat reste juridiquement dangereux.";
                }
            };

            for (Challenge& challenge : merchantChallenges)
            {
                configureMerchantChallenge(challenge);
            }

            auto challengeMatchesQuest = [&](const Challenge& challenge)
            {
                if (questText.find("caravane") != std::string::npos || questText.find("péage") != std::string::npos || questText.find("peage") != std::string::npos)
                    return challenge.family == "transport" || challenge.family == "calcul";
                if (questText.find("brocante") != std::string::npos || questText.find("luxe") != std::string::npos || questText.find("estimation") != std::string::npos)
                    return challenge.family == "estimation" || challenge.family == "registre";
                if (questText.find("facture") != std::string::npos || questText.find("monnaie") != std::string::npos
                    || questText.find("réduction") != std::string::npos || questText.find("reduction") != std::string::npos
                    || questText.find("taxe") != std::string::npos || questText.find("auberge") != std::string::npos)
                    return challenge.family == "calcul" || challenge.family == "registre";
                if (questText.find("registre") != std::string::npos || questText.find("contrat") != std::string::npos
                    || questText.find("message") != std::string::npos)
                    return challenge.family == "registre" || challenge.family == "francais";
                return challenge.family == "registre" || challenge.family == "calcul" || challenge.family == "francais";
            };

            std::vector<Challenge> unusedAll;
            std::vector<Challenge> unusedRelevant;
            std::vector<Challenge> unusedUnusual;
            for (const Challenge& challenge : merchantChallenges)
            {
                if (historyContains(challenge.id)) continue;
                unusedAll.push_back(challenge);
                if (challengeMatchesQuest(challenge)) unusedRelevant.push_back(challenge);
                if (challenge.unusualDocument) unusedUnusual.push_back(challenge);
            }

            if (unusedAll.empty())
            {
                quest.serviceChallengeHistory.clear();
                unusedAll = merchantChallenges;
                for (const Challenge& challenge : merchantChallenges)
                {
                    if (challengeMatchesQuest(challenge)) unusedRelevant.push_back(challenge);
                    if (challenge.unusualDocument) unusedUnusual.push_back(challenge);
                }
            }

            const int poolRoll = random.between(1, 100);
            if (poolRoll <= 70 && !unusedRelevant.empty()) merchantSelectedChallenges = unusedRelevant;
            else if (poolRoll <= 90 || unusedUnusual.empty()) merchantSelectedChallenges = unusedAll;
            else merchantSelectedChallenges = unusedUnusual;
        }

        if (questText.find("papier") != std::string::npos
            || questText.find("formulaire") != std::string::npos
            || questText.find("archive") != std::string::npos
            || questText.find("rapport") != std::string::npos)
        {
            challenges.push_back({
                "DOSSIER À CLASSER",
                "Les dossiers A-01, A-02 et A-04 sont posés sur la table. Quel dossier manque ?",
                {{1, "A-03"}, {2, "A-05"}, {3, "B-01"}},
                1,
                "Classement propre : la gérante ne perd pas son âme dans la pile.",
                "Classement faux : la pile de papiers gagne un étage."
            });
            challenges.push_back({
                "ACCORD DU PARTICIPE",
                "Quelle phrase est correcte ?",
                {{1, "Les lettres ont été cachetées."}, {2, "Les lettres on été cacheté."}, {3, "Les lettre ont été cachetées."}},
                1,
                "Accord correct : même la plume semble fière.",
                "Accord raté : la plume préfère retourner dans l'encrier."
            });
        }

        if (questText.find("caisse") != std::string::npos
            || questText.find("stock") != std::string::npos
            || questText.find("inventaire") != std::string::npos
            || questText.find("réserve") != std::string::npos
            || questText.find("reserve") != std::string::npos)
        {
            challenges.push_back({
                "INVENTAIRE RÉEL",
                "Il y a 18 fioles, 6 sont cassées et 4 sont réservées. Combien sont disponibles ?",
                {{1, "8"}, {2, "12"}, {3, "14"}},
                1,
                "Stock lisible : le comptoir sait quoi vendre sans mentir.",
                "Stock faux : quelqu'un va promettre une fiole qui n'existe pas."
            });
        }

        if (questText.find("client") != std::string::npos
            || questText.find("dette") != std::string::npos
            || questText.find("marchand") != std::string::npos
            || questText.find("reçu") != std::string::npos
            || questText.find("recu") != std::string::npos)
        {
            challenges.push_back({
                "DETTE ET REÇU",
                "Un client devait 45 pièces. Il paie 20 puis 15. Combien reste-t-il ?",
                {{1, "5 pièces"}, {2, "10 pièces"}, {3, "15 pièces"}},
                2,
                "Compte juste : même le client ne peut pas faire semblant de ne pas comprendre.",
                "Compte faux : le client sourit, donc c'est probablement mauvais signe."
            });
        }

        if (questText.find("animal") != std::string::npos
            || questText.find("poule") != std::string::npos
            || questText.find("chat") != std::string::npos)
        {
            challenges.push_back({
                "TRACE D'ANIMAL",
                "Trois traces vont vers le grenier, une revient vers la cuisine. Où chercher d'abord ?",
                {{1, "Le grenier"}, {2, "La cuisine"}, {3, "Le puits"}},
                1,
                "Lecture propre : tu gagnes du temps avant que l'animal ne gagne une personnalité de boss.",
                "Lecture ratée : l'animal gagne une avance dramatique."
            });
        }


        if (questText.find("bibliothèque") != std::string::npos
            || questText.find("bibliotheque") != std::string::npos
            || questText.find("archiviste") != std::string::npos
            || questText.find("bestiaire") != std::string::npos
            || questText.find("connaissance") != std::string::npos
            || questText.find("magie") != std::string::npos)
        {
            challenges.push_back({
                "BIBLIOTHÈQUE — SPECTRE OU OMBRE",
                "Quelle différence est la plus logique entre un spectre et une ombre ?",
                {{1, "Le spectre garde une émotion ou un regret ; l'ombre est plus primitive et attaque lumière/âme"}, {2, "L'ombre vend des livres et le spectre tient la caisse"}, {3, "Il n'y a aucune différence utile"}},
                1,
                "Réponse claire : l'Archiviste peut enfin écrire une note qui ne tue pas les apprentis.",
                "Réponse floue : l'Archiviste note que le savoir a perdu contre le brouillard."
            });
            challenges.push_back({
                "BIBLIOTHÈQUE — TROLL",
                "Un troll se régénère. Qu'est-ce qui limite généralement cette régénération ?",
                {{1, "Le feu ou l'acide"}, {2, "Lui demander gentiment d'arrêter"}, {3, "Le chatouiller avec une plume sacrée"}},
                1,
                "Point faible validé : la fiche évite de recommander une décapitation inutile.",
                "Point faible raté : la fiche devient dangereuse, donc Meron la confisque."
            });
            challenges.push_back({
                "BIBLIOTHÈQUE — SORTS",
                "Quelle phrase décrit le mieux un sort canalisé ?",
                {{1, "Il doit être maintenu ou intensifié pendant un temps"}, {2, "Il touche toujours toute la carte"}, {3, "Il ne coûte jamais de mana"}},
                1,
                "Définition propre : les mages débutants éviteront peut-être de lâcher le sort au mauvais moment.",
                "Définition ratée : un apprenti vient probablement d'exploser une bougie."
            });
            challenges.push_back({
                "BIBLIOTHÈQUE — PLANTE DE SOMMEIL",
                "Quelle note doit être marquée comme dangereuse dans un herbier ?",
                {{1, "Une herbe qui provoque le sommeil en quelques secondes"}, {2, "Une carotte qui ressemble à une carotte"}, {3, "Une feuille qui fait tousser un peu"}},
                1,
                "Prudence validée : l'herbier ne sera pas utilisé comme oreiller mortel.",
                "Prudence ratée : l'herbier gagne un cadenas, par sécurité."
            });
        }

        if (questText.find("scribe") != std::string::npos
            || questText.find("administr") != std::string::npos
            || questText.find("inscription") != std::string::npos
            || questText.find("pastille") != std::string::npos
            || questText.find("abonnement") != std::string::npos
            || questText.find("litige") != std::string::npos)
        {
            challenges.push_back({
                "BUREAU — FICHE D'INSCRIPTION",
                "Une fiche manque la signature et la classe. Que faut-il faire avant validation magique ?",
                {{1, "La compléter ou demander confirmation"}, {2, "La tamponner plus fort"}, {3, "Inventer une classe stylée"}},
                1,
                "Fiche propre : Scribe Ysolde peut enregistrer sans maudire le registre.",
                "Fiche refusée : le registre refuse d'avaler n'importe quoi."
            });
            challenges.push_back({
                "BUREAU — PASTILLE",
                "Un aventurier abandonne une mission sans prévenir, mais revient avec une preuve valable. Quelle réaction est la plus juste ?",
                {{1, "Enquêter avant de passer directement en pastille noire"}, {2, "Pastille noire immédiate pour le style"}, {3, "Récompense bonus car il est revenu"}},
                1,
                "Jugement propre : l'administration distingue faute, urgence et trahison.",
                "Jugement raté : Ysolde range ta réponse dans le dossier 'abus de tampon'."
            });
            challenges.push_back({
                "BUREAU — ABONNEMENT",
                "Un abonnement donne 10% de réduction. Un service coûte 50 fer. Réduction ?",
                {{1, "5 fer"}, {2, "10 fer"}, {3, "45 fer"}},
                1,
                "Calcul propre : le reçu peut être signé sans duel comptable.",
                "Calcul faux : le client sourit trop, donc tu t'es sûrement trompé."
            });
            challenges.push_back({
                "BUREAU — LITIGE",
                "Quelle phrase est la plus correcte dans un rapport ?",
                {{1, "Le client affirme que la livraison est arrivée en retard."}, {2, "Le client affirme que la livraison et arriver en retard."}, {3, "Le client affirme que les livraison sont arrivé."}},
                1,
                "Rapport lisible : l'affaire peut avancer sans traducteur de catastrophe.",
                "Rapport raté : même le litige ne sait plus de quoi il parle."
            });
        }

        if (questText.find("alchim") != std::string::npos
            || questText.find("potion") != std::string::npos
            || questText.find("fiole") != std::string::npos
            || questText.find("dosage") != std::string::npos
            || questText.find("réactif") != std::string::npos
            || questText.find("reactif") != std::string::npos)
        {
            challenges.push_back({
                "ALCHIMIE — DOSE DE SOIN",
                "Une potion conseille maximum 2 prises par jour. Un client en veut 3 'pour aller plus vite'. Que répondre ?",
                {{1, "Refuser et expliquer le risque"}, {2, "Lui vendre 6 flacons, business"}, {3, "Mélanger avec du piment"}},
                1,
                "Sécurité validée : Maëra évite un client lumineux au sol.",
                "Sécurité ratée : Maëra éloigne doucement les fioles de toi."
            });
            challenges.push_back({
                "ALCHIMIE — INVENTAIRE",
                "Le labo a 18 fioles, 5 cassées et 4 contaminées. Combien restent utilisables ?",
                {{1, "9"}, {2, "13"}, {3, "17"}},
                1,
                "Inventaire juste : aucune potion ne sera servie dans du verre triste.",
                "Inventaire faux : le laboratoire gagne un nouveau danger administratif."
            });
            challenges.push_back({
                "ALCHIMIE — ÉTIQUETTE",
                "Quelle étiquette est correcte ?",
                {{1, "Potion de mana : effets secondaires possibles, ne pas dépasser deux prises par jour."}, {2, "Potion de manna, effait secondaire, avalé tout."}, {3, "Potion bleu magique truc, boire vite."}},
                1,
                "Étiquette propre : le client peut survivre à la lecture.",
                "Étiquette ratée : la potion est moins instable que la phrase."
            });
            challenges.push_back({
                "ALCHIMIE — MÉLANGE",
                "Un réactif fume déjà tout seul. Quelle action est la plus prudente ?",
                {{1, "L'isoler, noter l'anomalie et demander confirmation"}, {2, "Le secouer pour voir"}, {3, "Le mélanger au bidon bleu"}},
                1,
                "Prudence validée : le bidon bleu reste loin de l'histoire.",
                "Prudence ratée : quelque part, un lapin violet applaudit."
            });
        }

        if (questText.find("transport") != std::string::npos
            || questText.find("route") != std::string::npos
            || questText.find("caravane") != std::string::npos
            || questText.find("diligence") != std::string::npos
            || questText.find("portail") != std::string::npos
            || questText.find("pass") != std::string::npos)
        {
            challenges.push_back({
                "TRANSPORT — COÛT DE GARDE",
                "4 gardes coûtent 8 fer par jour chacun pendant 12 jours. Total ?",
                {{1, "384 fer"}, {2, "96 fer"}, {3, "32 fer"}},
                1,
                "Budget juste : la caravane part avec des gardes payés, donc moins grognons.",
                "Budget faux : les gardes regardent la caisse comme un monstre rare."
            });
            challenges.push_back({
                "TRANSPORT — PASS",
                "Quel document paraît le plus logique pour traverser plusieurs villes officiellement ?",
                {{1, "Un pass de commerce ou de voyage reconnu"}, {2, "Un dessin de cheval"}, {3, "Un reçu de soupe"}},
                1,
                "Document correct : Noro évite d'envoyer quelqu'un au contrôle avec une blague.",
                "Document raté : le garde du pont va rire, puis refuser."
            });
            challenges.push_back({
                "TRANSPORT — CHARGEMENT",
                "Une diligence porte 6 passagers. 4 places sont prises. Combien restent libres ?",
                {{1, "2"}, {2, "3"}, {3, "10"}},
                1,
                "Chargement propre : personne ne voyage sur le toit par accident.",
                "Chargement faux : le toit devient une option commerciale."
            });
            challenges.push_back({
                "TRANSPORT — BON DE LIVRAISON",
                "Quelle phrase est correcte ?",
                {{1, "Les caisses doivent être livrées avant l'aube."}, {2, "Les caisse doivent être livré avant l'aube."}, {3, "Les caisses doit être livrer avant l'aube."}},
                1,
                "Bon lisible : la marchandise part au bon endroit, ce qui est presque magique.",
                "Bon raté : une caisse va probablement découvrir le monde."
            });
        }

        if (questText.find("auberge") != std::string::npos
            || questText.find("hébergement") != std::string::npos
            || questText.find("hebergement") != std::string::npos
            || questText.find("chambre") != std::string::npos
            || questText.find("taverne") != std::string::npos
            || questText.find("repas") != std::string::npos)
        {
            challenges.push_back({
                "AUBERGE — CHAMBRES",
                "Une chambre commune a 6 lits. 4 voyageurs arrivent. Combien de lits restent libres ?",
                {{1, "2"}, {2, "4"}, {3, "10"}},
                1,
                "Répartition propre : personne ne dort dans le placard sauf décision personnelle.",
                "Répartition ratée : Tavia range ta réponse avec les chaussettes perdues."
            });
            challenges.push_back({
                "AUBERGE — ADDITION",
                "3 repas à 8 cuivre et 1 nuit à 3 fer. Total en cuivre ?",
                {{1, "54 cuivre"}, {2, "33 cuivre"}, {3, "240 cuivre"}},
                1,
                "Addition juste : la table ne se transforme pas en tribunal.",
                "Addition fausse : un client compte sur ses doigts avec colère."
            });
            challenges.push_back({
                "AUBERGE — PLAINTE",
                "Quelle phrase est correcte ?",
                {{1, "La soupe était froide, mais le pain était bon."}, {2, "La soupe étais froid, mais les pain été bon."}, {3, "La soupe été froide mais le pain étais bonnes."}},
                1,
                "Plainte propre : Tavia peut répondre sans deviner la langue utilisée.",
                "Plainte ratée : la soupe demande un avocat."
            });
            challenges.push_back({
                "AUBERGE — OBJETS OUBLIÉS",
                "Chambre 1 : cape. Chambre 2 : bottes. Chambre 3 : cape. Quel objet est unique ?",
                {{1, "Les bottes"}, {2, "La cape"}, {3, "Les murs"}},
                1,
                "Tri logique : l'objet oublié retrouve une chance d'avoir un propriétaire.",
                "Tri raté : l'auberge gagne une collection inutile."
            });
        }

        if (questText.find("service") != std::string::npos
            || questText.find("réparation") != std::string::npos
            || questText.find("reparation") != std::string::npos
            || questText.find("notaire") != std::string::npos
            || questText.find("garde") != std::string::npos
            || questText.find("lettre") != std::string::npos
            || questText.find("bal") != std::string::npos)
        {
            challenges.push_back({
                "VILLE — SERVICE ARTISANAL",
                "Une réparation d'armure coûte 4 fer et une gravure 3 fer. Total ?",
                {{1, "7 fer"}, {2, "12 fer"}, {3, "1 électrum et 7 fer"}},
                1,
                "Tarif propre : l'artisan peut travailler sans recompter dix fois.",
                "Tarif raté : l'artisan range son marteau par sécurité administrative."
            });
            challenges.push_back({
                "VILLE — LETTRE LOCALE",
                "Un envoi local coûte 2 cuivre. Trois lettres locales coûtent combien ?",
                {{1, "6 cuivre"}, {2, "6 fer"}, {3, "2 électrum"}},
                1,
                "Calcul simple validé : même un pigeon aurait compris.",
                "Calcul raté : le pigeon refuse d'être associé à cette facture."
            });
            challenges.push_back({
                "VILLE — CONTRAT OFFICIEL",
                "Quel service demande le plus logiquement un notaire ou un registre officiel ?",
                {{1, "Vente de terrain ou héritage"}, {2, "Acheter une soupe"}, {3, "Dormir sous un arbre"}},
                1,
                "Choix logique : le contrat peut éviter un futur procès idiot.",
                "Choix raté : le registre refuse de tamponner une soupe."
            });
            challenges.push_back({
                "VILLE — GARDE JOURNALIER",
                "Recruter 2 gardes à 1 électrum chacun pour une journée coûte combien ?",
                {{1, "2 électrum"}, {2, "2 fer"}, {3, "20 or"}},
                1,
                "Budget propre : les gardes seront payés, donc moins dangereux pour le client.",
                "Budget raté : les gardes deviennent soudainement très attentifs à ta bourse."
            });
        }


        if (questText.find("brocante") != std::string::npos
            || questText.find("troc") != std::string::npos
            || questText.find("luxe") != std::string::npos
            || questText.find("estimation") != std::string::npos
            || questText.find("marché") != std::string::npos
            || questText.find("marche") != std::string::npos)
        {
            challenges.push_back({
                "MARCHÉ — BROCANTE",
                "Un lot contient 3 vieilles lampes à 6 fer et 2 ressorts à 4 fer. Prix total ?",
                {{1, "26 fer"}, {2, "18 fer"}, {3, "30 électrum"}},
                1,
                "Estimation propre : le brocanteur ne vend pas une lampe cassée au prix d'une relique.",
                "Estimation ratée : quelqu'un vient d'inventer le luxe avec de la poussière."
            });
            challenges.push_back({
                "MARCHÉ — TROC",
                "Quel échange paraît le plus équilibré ?",
                {{1, "Un bon outil contre plusieurs petits composants utiles"}, {2, "Une chaussette humide contre une épée rare"}, {3, "Une promesse vague contre tout le stock"}},
                1,
                "Troc raisonnable : personne ne se sent assez volé pour appeler les gardes.",
                "Troc raté : même le marché noir trouve ça malhonnête."
            });
            challenges.push_back({
                "LUXE — ESTIMATION",
                "Un objet de luxe doit surtout être vérifié sur quoi avant achat ?",
                {{1, "Rareté, état, provenance et acheteur réel"}, {2, "La couleur la plus brillante seulement"}, {3, "Le fait que le vendeur parle vite"}},
                1,
                "Estimation prudente : le noble client évite le bijou maudit ou juste nul.",
                "Estimation ratée : le bijou brille, mais la facture aussi."
            });
        }

        if (questText.find("logement long") != std::string::npos
            || questText.find("long terme") != std::string::npos
            || questText.find("caution") != std::string::npos
            || questText.find("blanchisserie") != std::string::npos
            || questText.find("bain") != std::string::npos
            || questText.find("services ville") != std::string::npos)
        {
            challenges.push_back({
                "AUBERGE — LONG SÉJOUR",
                "Un voyageur loue 7 nuits à 3 fer la nuit et paie 5 fer de caution. Total avancé ?",
                {{1, "26 fer"}, {2, "21 fer"}, {3, "12 fer"}},
                1,
                "Contrat propre : la chambre ne devient pas une guerre de clés.",
                "Contrat raté : Tavia cache déjà les draps propres."
            });
            challenges.push_back({
                "VILLE — ORIENTATION",
                "Un client veut laver ses vêtements, envoyer une lettre et dormir. Quelle orientation est correcte ?",
                {{1, "Blanchisserie, messager, auberge"}, {2, "Forgeron, cimetière, armurerie"}, {3, "Bureau des boss, volcan, arène"}},
                1,
                "Orientation claire : le voyageur ne confond pas bain public et forge.",
                "Orientation ratée : quelqu'un va payer une épée pour laver une chemise."
            });
        }

        if (questText.find("inter-paliers") != std::string::npos
            || questText.find("inter paliers") != std::string::npos
            || questText.find("péage") != std::string::npos
            || questText.find("peage") != std::string::npos
            || questText.find("relais") != std::string::npos)
        {
            challenges.push_back({
                "TRANSPORT — INTER-PALIERS",
                "Pourquoi un voyage entre paliers demande souvent plus qu'un simple ticket ?",
                {{1, "Parce qu'il faut un pass, une escorte possible, une durée variable et un contrôle"}, {2, "Parce que les chevaux savent lire"}, {3, "Parce que la route disparaît quand on la regarde"}},
                1,
                "Plan réaliste : Noro peut prévenir le client sans promettre une téléportation gratuite.",
                "Plan raté : le voyageur partira sûrement avec trop peu d'eau et trop d'optimisme."
            });
            challenges.push_back({
                "TRANSPORT — PÉAGE",
                "Deux ponts coûtent 3 fer chacun, et un relais coûte 4 fer. Total ?",
                {{1, "10 fer"}, {2, "7 fer"}, {3, "14 électrum"}},
                1,
                "Péage compté : la caravane évite la surprise qui bloque tout le convoi.",
                "Péage raté : le pont devient soudainement un boss économique."
            });
        }

        if (questText.find("preuve") != std::string::npos
            || questText.find("réhabilitation") != std::string::npos
            || questText.find("rehabilitation") != std::string::npos
            || questText.find("service local") != std::string::npos)
        {
            challenges.push_back({
                "BUREAU — RÉHABILITATION",
                "Quelle preuve aide le mieux un dossier après des retards ?",
                {{1, "Plusieurs petits contrats officiels réussis proprement"}, {2, "Dire très fort que c'était pas grave"}, {3, "Changer de nom au comptoir"}},
                1,
                "Réhabilitation comprise : Ysolde peut corriger la pastille sans effacer l'historique.",
                "Réhabilitation ratée : Ysolde garde le tampon rouge loin de tes mains."
            });
        }

        const std::vector<Challenge>& selectedPool = !merchantSelectedChallenges.empty()
            ? merchantSelectedChallenges
            : challenges;
        const Challenge& challenge = selectedPool[random.between(0, static_cast<int>(selectedPool.size()) - 1)];
        int choice = askChoiceScreen(
            challenge.title,
            "quest.guild.service.micro_challenge",
            {
                "Ce service ne se règle pas en aller-retour automatique.",
                "Petite épreuve intellectuelle :",
                challenge.question
            },
            challenge.options,
            1,
            3
        );
        Console::clear();

        MicroChallengeResult result;
        const bool mainAnswerCorrect = choice == challenge.correctChoice;
        result.success = mainAnswerCorrect;
        result.lines.push_back(mainAnswerCorrect ? challenge.successLine : challenge.failureLine);

        if (!challenge.id.empty())
        {
            if (!quest.serviceChallengeHistory.empty()) quest.serviceChallengeHistory += "|";
            quest.serviceChallengeHistory += challenge.id;
        }

        if (mainAnswerCorrect && challenge.correctHandlingChoice > 0 && !challenge.handlingOptions.empty())
        {
            int handlingChoice = askChoiceScreen(
                "TRAITEMENT DU DOCUMENT",
                "quest.guild.service.document_handling",
                {
                    "La correction du texte ne suffit pas : il faut aussi traiter le document correctement.",
                    challenge.handlingQuestion
                },
                challenge.handlingOptions,
                1,
                3
            );
            Console::clear();

            if (handlingChoice == challenge.correctHandlingChoice)
            {
                result.lines.push_back(challenge.handlingSuccessLine);
            }
            else
            {
                result.success = false;
                result.partial = true;
                result.lines.push_back(challenge.handlingFailureLine);
            }
        }

        if (result.success)
        {
            result.lines.push_back("Le service peut progresser.");
        }
        else if (result.partial)
        {
            result.lines.push_back("Étape partiellement comprise : aucune progression, mais Prunigil ne facture pas l'erreur comme un échec complet.");
        }
        else
        {
            result.lines.push_back("Le service ne progresse pas cette fois. Tu pourras réessayer plus tard.");
        }
        return result;
    }

    // EN: chooseCarefulRecovery declares or implements a focused behavior used by this module.
    // FR: chooseCarefulRecovery déclare ou implémente un comportement précis utilisé par ce module.
    bool chooseCarefulRecovery(Random& random, const ExplorationIntensity& intensity)
    {
        return random.rollD20() + intensity.carefulBonus >= 15;
    }

    // EN: addExplorationMaterial declares or implements a focused behavior used by this module.
    // FR: addExplorationMaterial déclare ou implémente un comportement précis utilisé par ce module.
    std::string addExplorationMaterial(Player& player, const std::string& id, int quantity, const std::string& quality)
    {
        player.getInventory().addMaterial(MaterialCatalog::createById(id, quantity, quality));
        Material preview = MaterialCatalog::createById(id, quantity, quality);

        std::string line = "Récupéré : " + preview.getName();
        if (preview.hasSpecialQuality())
        {
            line += " [" + preview.getQualityLabel() + "]";
        }

        line += " x" + std::to_string(quantity);
        return line;
    }

    struct ExplorationRouteResult
    {
        int rollShift = 0;
        int questProgress = 0;
        bool carefulBoost = false;
        std::vector<std::string> lines;
    };

    std::string biomeRouteMoodLine(const ExplorationBiome& biome)
    {
        if (biome.name == "Temple des cloches fendues") return "Les cloches ne sonnent plus vraiment : elles corrigent surtout les imprudents.";
        if (biome.name == "Canaux de brume bleue") return "La brume avale les ponts et rend chaque raccourci un peu trop convaincant.";
        if (biome.name == "Carrière des os blancs") return "La craie blanche marque les bottes, les murs et parfois les choses qui te suivent.";
        if (biome.name == "Marché sous les ponts") return "Sous les ponts, tout le monde vend quelque chose, même les silences.";
        if (biome.name == "Jardin des statues qui pleurent") return "Les statues pleurent sans bouger ; c'est rarement bon signe.";
        if (biome.name == "Archives noyées") return "L'eau monte autour des rayons, mais les cotes de classement restent étrangement lisibles.";
        if (biome.name == "Foire abandonnée") return "Les stands grincent comme s'ils attendaient encore des clients.";
        if (biome.name == "Falaises des drakes gris") return "Le vent décide parfois avant toi quel chemin mérite d'exister.";
        if (biome.name == "Bois de la Corruption") return "Les arbres tordus semblent indiquer plusieurs routes, mais aucune ne promet de rester saine.";
        if (biome.name == "Crypte du Sombre-Lien") return "Chaque couloir porte un nom gravé, et certains noms veulent être suivis.";
        if (biome.name == "Désert des Protecteurs") return "Les statues du désert regardent surtout les gestes inutiles.";
        if (biome.name == "Sanctuaire antique des Veilleurs") return "Le sanctuaire ne ferme aucune porte, mais il juge chaque entrée.";
        if (biome.name == "Quartier des Lames Muettes") return "Ici, un raccourci trop évident ressemble souvent à une gorge offerte.";
        if (biome.name == "Toits des Assassins") return "Les hauteurs donnent une vue parfaite, surtout à ceux qui te visaient déjà.";
        if (biome.name == "Nid draconique rouge") return "La chaleur ne vient pas seulement du sol : quelque chose respire plus haut.";
        if (biome.name == "Coulées de lave noire") return "La lave noire avance lentement, comme si elle savait que tu finiras par hésiter.";
        if (biome.name == "Glacier des Serments froids") return "Le froid conserve les traces, les promesses et les erreurs avec la même patience.";
        if (biome.name == "Bosquet des Fées du Mana") return "Les lumières rient doucement ; ce n'est pas forcément une menace, ce qui est presque pire.";
        if (biome.name == "Sanctuaire kitsuné des Neuf Étincelles") return "Chaque torii semble mener au bon chemin, donc au moins huit mentent probablement.";
        if (biome.name == "Confluence du Mana pur") return "Le mana coule dans plusieurs directions et attend que tu choisisses laquelle va te contredire.";
        if (biome.name == "Bastion majeur scellé") return "Le bastion ne veut pas encore raconter son histoire, mais il accepte de mesurer ta présence.";
        if (biome.name == "Archipel des îles flottantes") return "Les îles ne sont pas toutes plates, et certaines changent de distance quand tu les fixes.";
        if (biome.name == "Ponts translucides de mana") return "Les ponts tiennent mieux quand personne ne doute d'eux. Mauvaise nouvelle : tu doutes.";
        if (biome.name == "Cieux des Légendes") return "Les récits flottent autour de toi comme des drapeaux prêts à choisir leur champion.";
        if (biome.name == "Parvis des Divinités") return "Même les marches semblent demander pourquoi tu penses avoir le droit de monter.";
        return "La zone n'est plus un simple aller-retour : tu dois choisir comment y entrer.";
    }

    ExplorationRouteResult runExplorationRouteChoice(Player& player, const ExplorationBiome& biome, const ExplorationIntensity& intensity, Random& random)
    {
        (void)intensity;

        const int choice = askChoiceScreen(
            "ROUTE D'EXPLORATION",
            "exploration.route_choice",
            {
                biomeRouteMoodLine(biome),
                "Avant la vraie fouille, choisis une approche. Ce choix peut rendre la sortie plus longue, plus sûre, ou plus rentable."
            },
            {
                {1, "Tracer une route sûre et noter les repères"},
                {2, "Fouiller les abords avant l'objectif principal"},
                {3, "Suivre une piste secondaire risquée"}
            },
            1,
            3
        );
        Console::clear();

        ExplorationRouteResult result;

        if (choice == 1)
        {
            result.rollShift = -5;
            result.carefulBoost = true;
            result.questProgress = 1;
            result.lines.push_back("Tu avances lentement, tu marques deux repères et tu évites de transformer la sortie en sprint idiot.");
            result.lines.push_back("Effet : danger légèrement réduit, meilleure récupération si une ressource apparaît.");
            return result;
        }

        if (choice == 2)
        {
            result.rollShift = -1;
            result.questProgress = 1;
            result.lines.push_back("Tu prends le temps d'inspecter les abords : traces, odeurs, sol, restes de campement, petites preuves.");

            if (random.between(1, 100) <= 45)
            {
                result.lines.push_back(addExplorationMaterial(player, biome.commonMaterialId, 1, "standard"));
                result.lines.push_back("Bonus : la sortie a déjà produit quelque chose avant même l'événement principal.");
            }
            else
            {
                result.lines.push_back("Tu ne trouves rien de vendable, mais ton carnet devient plus utile pour les quêtes de terrain.");
            }

            return result;
        }

        result.rollShift = 10;
        result.questProgress = 2;
        result.lines.push_back("Tu suis une piste secondaire. Clairement pas le choix le plus propre, mais souvent le plus intéressant.");

        if (random.between(1, 100) <= 25)
        {
            result.lines.push_back(addExplorationMaterial(player, biome.rareMaterialId, 1, "good"));
            result.lines.push_back("Trouvaille rare : le détour était dangereux, mais pas vide.");
        }
        else
        {
            result.lines.push_back("La piste ne donne pas de butin immédiat. Par contre, elle attire probablement l'attention de quelque chose.");
        }

        result.lines.push_back("Effet : meilleur progrès potentiel, mais danger augmenté pour l'événement principal.");
        return result;
    }



    void recordBiomeFieldObservation(const ExplorationBiome& biome, const std::string& clue)
    {
        BestiaryRuntimeProgress::recordEncounter(
            "Observation - " + biome.name,
            "Biomes et exploration",
            clue
        );
    }

    bool hasPotentialQuestForBiome(const Player& player, const ExplorationBiome& biome)
    {
        return getQuestSearchHintForBiome(player, biome).hasAny;
    }

    bool isBiomeDiscoveredForPlayer(const ExplorationBiome& biome)
    {
        return BestiaryRuntimeProgress::getEncounterCount(biome.name) > 0
            || BestiaryRuntimeProgress::getEncounterCount("Observation - " + biome.name) > 0;
    }

    bool isBiomeCloseEnoughToReveal(const Player& player, const ExplorationBiome& biome)
    {
        return biome.minLevel <= player.getLevel() + 2;
    }

    bool shouldShowBiomeToPlayer(const Player& player, const ExplorationBiome& biome)
    {
        return isBiomeDiscoveredForPlayer(biome) || isBiomeCloseEnoughToReveal(player, biome);
    }

    bool isBiomeUnknownToPlayer(const Player& player, const ExplorationBiome& biome)
    {
        return !isBiomeDiscoveredForPlayer(biome) && isBiomeCloseEnoughToReveal(player, biome);
    }

    std::string unknownBiomeLabel(const Player& player, const ExplorationBiome& biome, int rumorIndex)
    {
        (void)player;
        return "????? — zone inconnue " + std::to_string(rumorIndex)
            + " (danger pressenti niv. " + std::to_string(biome.minLevel) + "+)";
    }

    void recordBiomeDiscoveryForPlayer(const ExplorationBiome& biome)
    {
        BestiaryRuntimeProgress::recordEncounter(
            biome.name,
            "Habitats / zones",
            "Zone visitable découverte en exploration : " + biome.style + "."
        );
    }

    MenuOptionItemData makeUnknownExplorationBiomeItemData(int rumorIndex, const ExplorationBiome& biome)
    {
        MenuOptionItemData itemData;
        itemData.structured = true;
        itemData.kind = "biome_unknown";
        itemData.section = "Exploration";
        itemData.actionType = "travel";
        itemData.name = "?????";
        itemData.detail = "Rumeur de terrain non confirmée. Le nom, les ressources et les créatures restent masqués jusqu'à la première visite.";
        itemData.status = "Zone inconnue " + std::to_string(rumorIndex) + " | danger pressenti niv. " + std::to_string(biome.minLevel) + "+";
        itemData.reward = "Ressources : ??? | Rares : ???";
        itemData.progress = "Approche possible : partir vérifier la rumeur.";
        itemData.owner = "Monstres : ???";
        itemData.important = true;
        return itemData;
    }

    MenuOptionItemData makeExplorationBiomeItemData(
        const Player& player,
        const ExplorationBiome& biome,
        bool questLikely
    )
    {
        MenuOptionItemData itemData;
        itemData.structured = true;
        itemData.kind = "biome";
        itemData.section = "Exploration";
        itemData.actionType = "travel";
        itemData.name = biome.name;
        itemData.detail = biome.style;
        itemData.status = evolvedBiomeRangeText(player, biome);
        itemData.reward = "Commun : " + biome.commonMaterialId + " | Rare : " + biome.rareMaterialId;
        itemData.progress = questLikely ? "Objectif de quête probable" : "Aucun objectif actif évident";
        itemData.owner = "Monstres : " + biome.commonMonsters;
        itemData.important = questLikely || isBiomeEvolvedForPlayer(player, biome);
        return itemData;
    }

    MenuOptionItemData makeExplorationIntensityItemData(const ExplorationIntensity& intensity)
    {
        MenuOptionItemData itemData;
        itemData.structured = true;
        itemData.kind = "exploration_intensity";
        itemData.section = "Exploration";
        itemData.actionType = "select";
        itemData.name = intensity.name;
        itemData.detail = intensity.description;
        itemData.status = "Danger : " + std::to_string(intensity.eventShift) + "% | Durée : +" + std::to_string(intensity.durationUnits) + " segment(s)";
        itemData.reward = "Bonus de pièces directes : " + std::to_string(intensity.goldPercent) + "%";
        itemData.progress = "Événements garantis : " + std::to_string(intensity.guaranteedEvents)
            + " | Chance événement bonus : " + std::to_string(intensity.extraEventChance) + "%"
            + " | Trouvailles : " + std::to_string(intensity.quantityBonus);
        itemData.important = intensity.eventShift > 0 || intensity.quantityBonus > 0 || intensity.goldPercent > 100;
        return itemData;
    }

    // EN: progressExplorationQuests declares or implements a focused behavior used by this module.
    // FR: progressExplorationQuests déclare ou implémente un comportement précis utilisé par ce module.
    int progressExplorationQuests(Player& player, const std::string& biomeName, int amount)
    {
        int updated = 0;

        for (Quest& quest : player.getQuestLog().getQuests())
        {
            if (!quest.accepted || quest.completed || quest.turnedIn || quest.failed)
            {
                continue;
            }

            if (quest.objectiveType != "exploration" && quest.objectiveType != "bestiaire" && !(quest.objectiveType == "livraison" && quest.requiredMaterialId.empty()))
            {
                continue;
            }

            const bool hasPreciseHint = !quest.location.empty() || !quest.targetFamily.empty() || !quest.objective.empty();
            const bool matchesBiome = questTextMentionsBiome(quest, biomeName);

            if (hasPreciseHint && !matchesBiome)
            {
                continue;
            }

            quest.progress += amount;

            if (quest.progress >= quest.target)
            {
                quest.progress = quest.target;
                quest.completed = true;
            }

            updated++;
        }

        return updated;
    }

    void appendExplorationQuestProgressLine(
        Player& player,
        const ExplorationBiome& biome,
        int amount,
        std::vector<std::string>& lines,
        const std::string& successText,
        const std::string& noQuestText = ""
    )
    {
        int updated = progressExplorationQuests(player, biome.name, amount);

        if (updated > 0)
        {
            lines.push_back(successText + " (" + std::to_string(updated) + " note(s) mise(s) à jour.)");
        }
        else if (!noQuestText.empty())
        {
            lines.push_back(noQuestText);
        }
    }

    void appendCombatQuestProgressLine(
        Player& player,
        int amount,
        const std::string& family,
        std::vector<std::string>& lines,
        const std::string& successText
    )
    {
        int updated = player.getQuestLog().progressCombatQuestsByFamily(amount, family);

        if (updated > 0)
        {
            lines.push_back(successText + " (" + std::to_string(updated) + " contrat(s) mis à jour.)");
        }
    }

    void applyExplorationCurse(
        Player& player,
        const std::string& id,
        const std::string& name,
        const std::string& origin,
        const std::string& description,
        const std::string& removalHint,
        const std::string& categories,
        int level,
        int maxLevel,
        bool evolves,
        int escalationIntervalDays,
        int durationDays,
        int exorcismVisits,
        bool canBecomeTooHighForChurch,
        std::vector<std::string>& lines,
        bool removableByChurch = true,
        int bossIdRequiredToBreak = 0,
        bool forceLifeLong = false
    )
    {
        PlayerCurse curse;
        curse.id = id;
        curse.name = name;
        curse.severity = level >= 3 ? "majeure" : (level == 2 ? "moyenne" : "mineure");
        curse.origin = origin;
        curse.description = description;
        curse.removalHint = removalHint;
        curse.symptomCategories = categories;
        curse.discoveredSymptomCategories = "";
        curse.excludedSymptomCategories = "";
        curse.diagnosisLevel = 0;
        curse.appliedAtDay = player.getWorldDaysElapsed();
        curse.expiresAtDay = durationDays > 0 ? player.getWorldDaysElapsed() + durationDays : -1;
        curse.exorcismProgress = 0;
        curse.exorcismRequiredVisits = std::max(1, exorcismVisits);
        curse.curseLevel = std::max(1, level);
        curse.maxCurseLevel = std::max(curse.curseLevel, maxLevel);
        curse.evolvesOverTime = evolves;
        curse.escalationIntervalDays = evolves ? std::max(1, escalationIntervalDays) : 0;
        curse.nextEscalationDay = evolves ? player.getWorldDaysElapsed() + curse.escalationIntervalDays : -1;
        curse.churchRemovalMaxLevel = canBecomeTooHighForChurch ? 2 : 99;
        curse.becomesSpecialRemovalWhenTooHigh = canBecomeTooHighForChurch;
        curse.highLevelRemovalHint = canBecomeTooHighForChurch
            ? "retrouver l'objet ou le lieu source, puis demander une lecture totale avant le rite."
            : "";
        curse.removableByChurch = removableByChurch;
        curse.bossIdRequiredToBreak = bossIdRequiredToBreak;
        if (!removableByChurch || bossIdRequiredToBreak > 0)
        {
            curse.exorcismRequiredVisits = 0;
        }
        curse.lifeLong = forceLifeLong || durationDays <= 0;

        const bool added = player.addOrRefreshCurse(curse);
        lines.push_back(added
            ? "Une trace inconnue s'accroche au personnage. Statut : ?????."
            : "Une trace déjà présente se ravive. Statut : ????? tant que le diagnostic n'avance pas.");
        if (evolves)
        {
            lines.push_back("Attention : cette malédiction fait partie des rares traces pouvant empirer avec le temps si elle est ignorée.");
        }
    }

    std::string explorationEventLabelFromRoll(int roll)
    {
        if (roll <= 9) return "récolte exposée";
        if (roll <= 18) return "ressource dissimulée";
        if (roll <= 26) return "cueillette fragile";
        if (roll <= 31) return "piste interrompue";
        if (roll <= 36) return "marques de passage";
        if (roll <= 40) return "objet perdu";
        if (roll <= 46) return "bourse oubliée";
        if (roll <= 52) return "petit dépôt ancien";
        if (roll <= 59) return "fausses pièces";
        if (roll <= 64) return "coffre à demi enfoui";
        if (roll <= 70) return "coffre trop visible";
        if (roll <= 76) return "prédateurs territoriaux";
        if (roll <= 82) return "groupe en déplacement";
        if (roll == 83) return "champion errant";
        if (roll <= 84) return "gardien local";
        if (roll <= 87) return "voyageur en difficulté";
        if (roll <= 91) return "demande locale imprévue";
        if (roll <= 94) return "zone anormalement calme";
        if (roll <= 97) return "biome soudainement agité";
        if (roll == 98) return "passage interdit";
        if (roll <= 99) return "site instable";
        return "découverte rare";
    }

    std::string explorationEventKeyFromRoll(int roll)
    {
        if (roll <= 9) return "main_gather_exposed";
        if (roll <= 18) return "main_gather_hidden";
        if (roll <= 26) return "main_gather_fragile";
        if (roll <= 31) return "main_trace_broken";
        if (roll <= 36) return "main_trace_passage";
        if (roll <= 40) return "main_trace_lost_object";
        if (roll <= 46) return "main_treasure_purse";
        if (roll <= 52) return "main_treasure_deposit";
        if (roll <= 59) return "main_fake_coins";
        if (roll <= 64) return "main_chest_buried";
        if (roll <= 70) return "main_chest_obvious";
        if (roll <= 76) return "main_fight_territorial";
        if (roll <= 82) return "main_fight_moving_group";
        if (roll == 83) return "main_miniboss_wanderer";
        if (roll <= 84) return "main_miniboss_guardian";
        if (roll <= 87) return "main_npc_quest_traveler";
        if (roll <= 91) return "main_npc_quest_local";
        if (roll <= 94) return "main_active_event_quiet";
        if (roll <= 97) return "main_active_event_agitated";
        if (roll == 98) return "main_dangerous_site_forbidden";
        if (roll <= 99) return "main_dangerous_site_unstable";
        return "main_rare_discovery";
    }

    int explorationEventCooldownDays(const std::string& key)
    {
        if (key == "main_fake_coins") return 6;
        if (key.find("main_miniboss_") == 0) return 4;
        if (key.find("main_npc_quest_") == 0) return 2;
        if (key.find("main_dangerous_site_") == 0) return 7;
        if (key == "main_rare_discovery") return 12;
        if (key == "main_chest_obvious") return 2;
        return 0;
    }

    std::string activeExplorationEventKeyFromRoll(int roll)
    {
        if (roll <= 18) return "active_abandoned_camp";
        if (roll <= 32) return "active_local_den";
        if (roll <= 46) return "active_tracks";
        if (roll <= 58) return "active_hazard";
        if (roll <= 68) return "active_hidden_cache";
        if (roll <= 80) return "active_wave";
        if (roll <= 91) return "active_ancient_sign";
        return "active_distress_call";
    }

    int activeExplorationEventCooldownDays(const std::string& key)
    {
        if (key == "active_abandoned_camp") return 4;
        if (key == "active_local_den") return 3;
        if (key == "active_tracks") return 1;
        if (key == "active_hazard") return 2;
        if (key == "active_hidden_cache") return 5;
        if (key == "active_wave") return 3;
        if (key == "active_ancient_sign") return 8;
        if (key == "active_distress_call") return 2;
        return 0;
    }

    int chooseVariedMainExplorationRoll(
        Player& player,
        Random& random,
        int preferredRoll,
        const ExplorationIntensity& intensity,
        int extraShift,
        const std::set<std::string>& currentRunKeys
    )
    {
        int candidate = std::clamp(preferredRoll, 1, 100);
        for (int attempt = 0; attempt < 18; ++attempt)
        {
            const std::string key = explorationEventKeyFromRoll(candidate);
            if (!player.wasExplorationEventRecentlySeen(key)
                && currentRunKeys.find(key) == currentRunKeys.end()
                && !player.isExplorationSceneOnCooldown(key))
            {
                return candidate;
            }
            candidate = adjustExplorationEventRoll(random.between(1, 100), intensity);
            candidate = std::clamp(candidate + extraShift, 1, 100);
        }

        // The recent-history filter is deliberately softer than a true cooldown:
        // a related theme may reappear, but the exact illogical scene cannot.
        for (int roll = 1; roll <= 100; ++roll)
        {
            const std::string key = explorationEventKeyFromRoll(roll);
            if (currentRunKeys.find(key) == currentRunKeys.end()
                && !player.isExplorationSceneOnCooldown(key)
                && !player.wasExplorationEventRecentlySeen(key))
            {
                return roll;
            }
        }
        for (int roll = 1; roll <= 100; ++roll)
        {
            const std::string key = explorationEventKeyFromRoll(roll);
            if (currentRunKeys.find(key) == currentRunKeys.end()
                && !player.isExplorationSceneOnCooldown(key))
            {
                return roll;
            }
        }
        return candidate;
    }

    int chooseVariedActiveExplorationRoll(Player& player, Random& random)
    {
        int candidate = random.between(1, 100);
        for (int attempt = 0; attempt < 16; ++attempt)
        {
            const std::string key = activeExplorationEventKeyFromRoll(candidate);
            if (!player.wasExplorationEventRecentlySeen(key)
                && !player.isExplorationSceneOnCooldown(key))
            {
                return candidate;
            }
            candidate = random.between(1, 100);
        }
        for (int roll = 1; roll <= 100; ++roll)
        {
            const std::string key = activeExplorationEventKeyFromRoll(roll);
            if (!player.wasExplorationEventRecentlySeen(key)
                && !player.isExplorationSceneOnCooldown(key))
            {
                return roll;
            }
        }
        for (int roll = 1; roll <= 100; ++roll)
        {
            const std::string key = activeExplorationEventKeyFromRoll(roll);
            if (!player.isExplorationSceneOnCooldown(key)) return roll;
        }
        return candidate;
    }

    int applyChapterThreeExplorationChoiceBias(const Player& player, Random& random, int roll)
    {
        if (!player.hasStoryModeStarted() || player.getStoryChapter() < 3)
        {
            return std::clamp(roll, 1, 100);
        }

        const std::string routeChoice = StoryCampaign::getChapterThreeRouteChoice(player);
        const std::string convoyDecision = StoryCampaign::getChapterThreeConvoyDecision(player);
        int adjusted = std::clamp(roll, 1, 100);

        if (routeChoice == "commerce" && random.between(1, 100) <= 28)
        {
            const std::vector<int> commerceRolls = {random.between(1, 26), random.between(41, 52), random.between(60, 70)};
            adjusted = commerceRolls[static_cast<std::size_t>(random.between(0, static_cast<int>(commerceRolls.size()) - 1))];
        }
        else if (routeChoice == "secours" && adjusted >= 71 && random.between(1, 100) <= 34)
        {
            adjusted = random.between(85, 91);
        }
        else if (routeChoice == "recherche" && random.between(1, 100) <= 30)
        {
            adjusted = random.between(1, 100) <= 65 ? random.between(27, 40) : random.between(92, 97);
        }

        if (convoyDecision == "marchandises" && random.between(1, 100) <= 18)
        {
            adjusted = random.between(1, 100) <= 55 ? random.between(41, 52) : random.between(60, 70);
        }
        else if (convoyDecision == "preuves" && random.between(1, 100) <= 22)
        {
            adjusted = random.between(1, 100) <= 70 ? random.between(27, 40) : random.between(92, 97);
        }
        else if (convoyDecision == "quarantaine" && adjusted >= 71 && random.between(1, 100) <= 40)
        {
            adjusted = random.between(1, 100) <= 55 ? random.between(1, 26) : random.between(27, 40);
        }

        return std::clamp(adjusted, 1, 100);
    }

    void showExplorationRunSummary(
        const Player& player,
        const ExplorationBiome& biome,
        const ExplorationIntensity& intensity,
        const std::string& eventLabel,
        int hpBefore,
        int goldBefore,
        int readyBefore,
        int dayBeforeExploration,
        int unitBeforeExploration,
        int timeUnitsSpent
    )
    {
        const int hpAfter = player.getHp();
        const int goldAfter = player.getInventory().getGold();
        const int readyAfter = countQuestsForJournalFilter(player, QuestJournalFilter::ReadyToTurnIn);

        std::vector<std::string> lines = {
            "Zone : " + biome.name + ".",
            "Approche : " + intensity.name + ".",
            "Temps écoulé : +" + std::to_string(timeUnitsSpent) + " segment(s) de journée.",
            player.formatWorldTimeChange(dayBeforeExploration, unitBeforeExploration),
            "Événement principal : " + eventLabel + ".",
            "PV : " + std::to_string(hpBefore) + " -> " + std::to_string(hpAfter)
                + " / " + std::to_string(player.getMaxHp()) + ".",
            "Argent : " + Money::formatGold(goldBefore) + " -> " + Money::formatGold(goldAfter)
                + " (écart : " + std::to_string(goldAfter - goldBefore) + ").",
            "Demandes prêtes à rendre : " + std::to_string(readyBefore)
                + " -> " + std::to_string(readyAfter) + "."
        };

        if (readyAfter > readyBefore)
        {
            lines.push_back("Ton journal signale une nouvelle remise possible depuis le hub des quêtes.");
        }
        else if (readyAfter > 0)
        {
            lines.push_back("Tu as toujours au moins une demande prête à rendre.");
        }

        if (hpAfter < hpBefore)
        {
            lines.push_back("État : sortie marquée par des blessures, pense à vérifier tes soins avant de repartir.");
        }
        else
        {
            lines.push_back("État : aucune blessure supplémentaire visible dans ce résumé.");
        }

        MessageScreen::show("RÉSUMÉ D'EXPLORATION", "exploration.run.summary", lines, true);
    }

    // EN: simulateUnexpectedExplorationFight declares or implements a focused behavior used by this module.
    // FR: simulateUnexpectedExplorationFight déclare ou implémente un comportement précis utilisé par ce module.
    void simulateUnexpectedExplorationFight(Player& player, Random& random, const ExplorationBiome& biome, const ExplorationIntensity& intensity, DifficultyMode difficulty, DeathRuleMode deathRule)
    {
        Monster localMonster = createExplorationMonsterForBiome(player, random, biome, intensity);

        showExplorationNotice(
            "RENCONTRE IMPRÉVUE",
            "exploration.unexpected_fight",
            {
                "Un mouvement anormal coupe ta fouille.",
                "Un ennemi surgit sans prévenir : " + localMonster.getName()
                    + " [niveau " + std::to_string(localMonster.getLevel()) + "].",
                "Zone : " + biome.name + " | Présences communes : " + biome.commonMonsters
                    + " | Rares/élites : " + biome.rareMonsters + "."
            }
        );

        bool victory = runTrackedExplorationWave(
            player,
            random,
            difficulty,
            deathRule,
            std::vector<Monster>{localMonster},
            "Rencontre imprévue de " + biome.name
        );

        if (!victory)
        {
            return;
        }

        std::vector<std::string> resultLines = {
            "La menace imprévue est neutralisée."
        };

        if (random.between(1, 100) <= 70)
        {
            resultLines.push_back(addExplorationMaterial(
                player,
                biome.commonMaterialId,
                applyExplorationQuantityBonus(1, intensity),
                chooseExplorationQuality(random, false)
            ));
        }
        else
        {
            resultLines.push_back("Aucune ressource exploitable ne reste après l'affrontement.");
        }

        appendCombatQuestProgressLine(
            player,
            1,
            "Créatures locales",
            resultLines,
            "Une quête de combat progresse grâce à cette menace imprévue"
        );

        showExplorationNotice(
            "RENCONTRE TERMINÉE",
            "exploration.unexpected_fight.result",
            resultLines
        );
    }

    // EN: openExplorationChest declares or implements a focused behavior used by this module.
    // FR: openExplorationChest déclare ou implémente un comportement précis utilisé par ce module.
    void openExplorationChest(Player& player, Random& random, const ExplorationBiome& biome, const ExplorationIntensity& intensity, DifficultyMode difficulty, DeathRuleMode deathRule)
    {
        int choice = askChoiceScreen(
            "COFFRE SUSPECT",
            "exploration.chest.choice",
            {"Un coffre est posé là, presque trop calmement."},
            {{1, "Ouvrir le coffre"}, {0, "Le laisser tranquille"}},
            0,
            1
        );
        Console::clear();

        if (choice == 0)
        {
            showExplorationNotice(
                "COFFRE IGNORÉ",
                "exploration.chest.left",
                {"Tu décides que survivre vaut parfois mieux que satisfaire ta curiosité."}
            );
            return;
        }

        int roll = random.between(1, 100);

        if (roll <= 16)
        {
            int damage = std::min(random.between(5, 18 + player.getLevel()), std::max(0, player.getHp() - 1));
            if (damage > 0)
            {
                player.takeDamage(damage);
                std::vector<std::string> trapLines = {
                    "Un mécanisme claque.",
                    "Tu prends " + std::to_string(damage) + " dégâts, mais tu restes debout."
                };
                if (random.between(1, 100) <= 18)
                {
                    applyExplorationCurse(
                        player,
                        "haunted_chest_echo",
                        "Écho de coffre envouté",
                        "Coffre suspect de " + biome.name,
                        "Le piège n'a pas seulement touché la peau : il a laissé une petite trace de possession d'objet.",
                        "diagnostic niveau 1, puis rite court ou destruction de l'objet source si la trace revient.",
                        "luck,equipment,spirit",
                        1,
                        2,
                        false,
                        0,
                        5,
                        1,
                        false,
                        trapLines
                    );
                }
                showExplorationNotice(
                    "PIÈGE",
                    "exploration.chest.trap",
                    trapLines
                );
            }
            else
            {
                showExplorationNotice("PIÈGE", "exploration.chest.trap_no_damage", {"Un mécanisme claque, mais tu restes hors de portée."});
            }
        }
        else if (roll <= 28)
        {
            showExplorationNotice(
                "MIMIC",
                "exploration.chest.mimic",
                {"Le coffre se déplie d'un coup. Ce n'était pas un coffre. Mimic."}
            );
            simulateUnexpectedExplorationFight(player, random, biome, intensity, difficulty, deathRule);
            int gold = applyExplorationGoldReward(random.between(8, 24 + player.getLevel() * 2), player, intensity, difficulty, 1);
            player.getInventory().earnGold(gold);
            std::vector<std::string> mimicLines = {"Dans les restes visqueux, tu récupères " + Money::formatGoldWithRaw(gold) + "."};
            if (random.between(1, 100) <= 16)
            {
                applyExplorationCurse(
                    player,
                    "mimic_bite_memory",
                    "Mémoire de morsure",
                    "Mimic de " + biome.name,
                    "La morsure continue d'exister dans les réflexes du personnage, comme si le coffre mordait encore après sa mort.",
                    "diagnostic ciblé esprit ou santé, puis exorcisme progressif à l'église.",
                    "health,spirit,precision",
                    1,
                    3,
                    true,
                    4,
                    8,
                    2,
                    false,
                    mimicLines
                );
            }
            showExplorationNotice(
                "RESTES DU MIMIC",
                "exploration.chest.mimic.reward",
                mimicLines
            );
        }
        else if (roll <= 42)
        {
            showExplorationNotice("COFFRE VIDE", "exploration.chest.empty", {"Le coffre est vide. Quelqu'un a déjà eu l'idée avant toi."});
        }
        else if (roll <= 76)
        {
            int gold = applyExplorationGoldReward(random.between(8, 30 + player.getLevel() * 3), player, intensity, difficulty, 1);
            player.getInventory().earnGold(gold);
            std::vector<std::string> rewardLines = {
                "Le coffre est réel, mais son contenu reste modeste.",
                "Argent gagné : " + Money::formatGoldWithRaw(gold),
                addExplorationMaterial(player, biome.commonMaterialId, applyExplorationQuantityBonus(1, intensity), chooseExplorationQuality(random, true))
            };
            if (random.between(1, 100) <= 8)
            {
                rewardLines.push_back("Un bracelet rouillé colle une seconde à ton gant. Il retombe tout seul, mais le métal a laissé une sensation de froid.");
                applyExplorationCurse(
                    player,
                    "cursed_equipment_whisper",
                    "Murmure d'équipement maudit",
                    "Coffre d'équipement de " + biome.name,
                    "Un petit objet porteur de trace a touché l'équipement avant de tomber. La source doit être identifiée avant destruction sûre.",
                    "diagnostic total, objet source identifié, puis destruction contrôlée à l'église.",
                    "equipment,luck,corruption",
                    1,
                    2,
                    false,
                    0,
                    6,
                    0,
                    false,
                    rewardLines,
                    false,
                    0,
                    false
                );
            }
            showExplorationNotice("COFFRE MODESTE", "exploration.chest.modest", rewardLines);
        }
        else
        {
            int gold = applyExplorationGoldReward(random.between(35 + player.getLevel() * 3, 90 + player.getLevel() * 8), player, intensity, difficulty, 2);
            player.getInventory().earnGold(gold);
            std::vector<std::string> rewardLines = {
                "Le coffre est réel, et pour une fois il n'a pas décidé de te mordre.",
                "Argent gagné : " + Money::formatGoldWithRaw(gold),
                addExplorationMaterial(player, biome.rareMaterialId, applyExplorationQuantityBonus(1, intensity), chooseExplorationQuality(random, true))
            };
            if (random.between(1, 100) <= 12)
            {
                rewardLines.push_back("Une boucle d'armure sans propriétaire tinte comme si elle avait reconnu ton sac.");
                applyExplorationCurse(
                    player,
                    "cursed_equipment_whisper",
                    "Murmure d'équipement maudit",
                    "Coffre intact de " + biome.name,
                    "Un vestige d'équipement cherche à se faire porter sans être équipé. La trace vise surtout les gestes et le matériel.",
                    "diagnostic total, objet source identifié, puis destruction contrôlée à l'église.",
                    "equipment,precision,corruption",
                    1,
                    2,
                    false,
                    0,
                    0,
                    0,
                    false,
                    rewardLines,
                    false,
                    0,
                    true
                );
            }
            showExplorationNotice("COFFRE INTACT", "exploration.chest.good", rewardLines);
        }
    }

    void startCityRepairCrisis(Player& player, Random& random, const std::string& cause, std::vector<std::string>& lines)
    {
        const int currentRepairDays = player.getInventory().countMaterialById("city_repair_days_marker");
        if (currentRepairDays > 0)
        {
            lines.push_back("Ville : une crise est déjà en cours, les réparations ne sont pas empilées gratuitement.");
            lines.push_back("Réparations restantes : " + std::to_string(currentRepairDays) + " jour(s).");
            return;
        }

        const int repairDays = std::clamp(random.between(3, 21), 3, 21);
        player.getInventory().addMaterial(MaterialCatalog::createById("city_repair_days_marker", repairDays));
        player.getInventory().addMaterial(MaterialCatalog::createById("city_damage_notice", 1));
        lines.push_back("Conséquence : " + cause + ".");
        lines.push_back("La ville entre en réparations pendant " + std::to_string(repairDays) + " jour(s). Maximum prévu : 3 semaines.");
        lines.push_back("Pendant ce temps, presque toutes les boutiques ferment ; l'auberge, l'église, le bureau de ville et 1-2 comptoirs du jour restent accessibles.");
        lines.push_back("Les demandes locales deviennent surtout : réparer, récolter des ressources, garder les échoppes et remettre les rues en état.");
    }

    void triggerRareCityDefenseEvent(Player& player, Random& random, const ExplorationBiome& biome, const ExplorationIntensity& intensity, DifficultyMode difficulty, DeathRuleMode deathRule)
    {
        int choice = askChoiceScreen(
            "APPEL DE LA VILLE",
            "exploration.rare.city_defense",
            {
                "Un messager arrive à bout de souffle : une menace fonce vers la ville pendant que les gardes tiennent déjà plusieurs rues.",
                "Ce n'est pas un événement banal de tableau de quêtes. Si la défense échoue, les boutiques fermeront pendant les réparations.",
                "La guilde promet de noter chaque aide, mais personne ne garantit que la ville restera intacte."
            },
            {{1, "Revenir défendre la ville"}, {0, "Continuer l'exploration"}},
            0,
            1
        );
        Console::clear();

        if (choice == 0)
        {
            std::vector<std::string> lines = {
                "Tu ne peux pas être partout. La décision est logique... mais la ville encaisse sans toi."
            };
            startCityRepairCrisis(player, random, "défense de ville non assurée", lines);
            showExplorationNotice("VILLE ENDOMMAGÉE", "exploration.rare.city_defense.refused", lines);
            return;
        }

        showExplorationNotice(
            "DÉFENSE DE VILLE",
            "exploration.rare.city_defense.start",
            {
                "Tu reviens vers les portes. Les marchands ferment déjà leurs volets, les prêtres tirent les blessés derrière les bancs.",
                "Cette fois, le but n'est pas de farmer : il faut empêcher la ville de perdre ses comptoirs."
            }
        );

        std::vector<Monster> cityAttackers;
        const int attackerCount = random.between(3, 6);
        for (int i = 0; i < attackerCount; ++i)
        {
            cityAttackers.push_back(createExplorationMonsterForBiome(player, random, biome, intensity));
        }

        const bool victory = runTrackedExplorationWave(
            player,
            random,
            difficulty,
            deathRule,
            cityAttackers,
            "Défense de ville : menace venue de " + biome.name
        );

        if (victory)
        {
            const int rewardGold = applyExplorationGoldReward(random.between(40 + player.getLevel() * 2, 90 + player.getLevel() * 4), player, intensity, difficulty, 2);
            player.getInventory().earnGold(rewardGold);
            player.getInventory().addMaterial(MaterialCatalog::createById("city_defense_medal", 1));
            player.getInventory().addMaterial(MaterialCatalog::createById("city_service_stamp", 1));
            const int previousGratitudeDays = player.getInventory().countMaterialById("city_defense_gratitude_days_marker");
            if (previousGratitudeDays > 0)
            {
                player.getInventory().removeMaterialQuantityById("city_defense_gratitude_days_marker", previousGratitudeDays);
            }
            const int gratitudeDays = std::min(10, previousGratitudeDays + random.between(5, 7));
            player.getInventory().addMaterial(MaterialCatalog::createById("city_defense_gratitude_days_marker", gratitudeDays));
            std::vector<std::string> lines = {
                "La ligne tient. Quelques vitrines sont abîmées, mais la ville ne bascule pas en état de réparation générale.",
                "Attestation obtenue : défense de ville x1.",
                "Tampon de service municipal x1.",
                "Reconnaissance locale : certains commerçants feront une petite remise pendant " + std::to_string(gratitudeDays) + " jour(s).",
                "Limite : la gratitude commerciale ne dépasse jamais 10 jours, même si la ville te doit une fière chandelle.",
                "Prime de défense : " + Money::formatGoldWithRaw(rewardGold) + "."
            };
            appendCombatQuestProgressLine(player, 2, "Défense de ville", lines, "La défense de ville fait progresser certains contrats de protection");
            showExplorationNotice("VILLE DÉFENDUE", "exploration.rare.city_defense.victory", lines);
            return;
        }

        std::vector<std::string> lines = {
            "La défense ne suffit pas. Les habitants survivent, mais plusieurs rues doivent être reconstruites avant de rouvrir normalement."
        };
        startCityRepairCrisis(player, random, "défense de ville perdue", lines);
        showExplorationNotice("VILLE ABÎMÉE", "exploration.rare.city_defense.defeat", lines);
    }

    // EN: triggerRareExplorationDiscovery declares or implements a focused behavior used by this module.
    // FR: triggerRareExplorationDiscovery déclare ou implémente un comportement précis utilisé par ce module.
    void triggerRareExplorationDiscovery(Player& player, Random& random, const ExplorationBiome& biome, const ExplorationIntensity& intensity, DifficultyMode difficulty, DeathRuleMode deathRule)
    {
        int roll = random.between(1, 100);

        if (roll <= 10)
        {
            triggerRareCityDefenseEvent(player, random, biome, intensity, difficulty, deathRule);
            return;
        }

        if (roll <= 22)
        {
            std::vector<std::string> lines = {
                "Un filon / bouquet intact a survécu aux passages précédents.",
                "Tu prends le temps de récupérer proprement ce qui peut l'être.",
                addExplorationMaterial(player, biome.rareMaterialId, applyExplorationQuantityBonus(random.between(1, 2), intensity), chooseExplorationQuality(random, true)),
                addExplorationMaterial(player, biome.commonMaterialId, applyExplorationQuantityBonus(1, intensity), chooseExplorationQuality(random, true))
            };
            appendExplorationQuestProgressLine(player, biome, 2, lines, "Des notes d'exploration progressent grâce à cette découverte rare");
            showExplorationNotice("DÉCOUVERTE RARE", "exploration.rare.discovery.resource", lines);
            return;
        }

        if (roll <= 40)
        {
            int gold = applyExplorationGoldReward(random.between(45 + player.getLevel() * 4, 120 + player.getLevel() * 9), player, intensity, difficulty, 3);
            player.getInventory().earnGold(gold);
            std::vector<std::string> lines = {
                "Une cache ancienne est dissimulée sous des marques presque effacées.",
                "Ce n'est pas un trésor de roi, mais ce n'est clairement pas une trouvaille normale.",
                "Argent gagné : " + Money::formatGoldWithRaw(gold),
                addExplorationMaterial(player, biome.rareMaterialId, applyExplorationQuantityBonus(1, intensity), chooseExplorationQuality(random, true))
            };
            if (random.between(1, 100) <= 12)
            {
                lines.push_back("Sous les pièces, tu trouves un contrat trop propre pour son âge. Ton nom n'y est pas écrit, mais l'encre bouge quand même.");
                applyExplorationCurse(
                    player,
                    "oath_binding_trace",
                    "Trace de serment déplacé",
                    "Contrat ancien de " + biome.name,
                    "Un serment qui ne t'appartenait pas essaie pourtant de se faire reconnaître. Le problème n'est pas médical : il faut nommer puis briser la parole.",
                    "diagnostic total, témoignage de serment nommé, puis rupture du serment auprès de l'église.",
                    "social,spirit,health",
                    1,
                    2,
                    false,
                    0,
                    0,
                    0,
                    false,
                    lines,
                    false,
                    0,
                    true
                );
            }
            appendExplorationQuestProgressLine(player, biome, 1, lines, "Le journal d'exploration progresse grâce à cette cache");
            showExplorationNotice("CACHE ANCIENNE", "exploration.rare.discovery.cache", lines);
            return;
        }

        if (roll <= 58)
        {
            std::vector<std::string> lines = {
                "Tu trouves des traces parfaitement conservées.",
                "Elles ne donnent pas un objet immédiat, mais elles valent beaucoup pour les quêtes et le carnet de terrain."
            };
            recordBiomeFieldObservation(biome, "Trace rare conservée : " + biome.name + " révèle des présences locales plus anciennes que les rencontres normales.");
            appendExplorationQuestProgressLine(
                player,
                biome,
                3,
                lines,
                "Plusieurs notes de quête progressent grâce à ces traces",
                "Tu notes mentalement le lieu : ce genre de trace intéresserait clairement une guilde ou un client."
            );
            if (random.between(1, 100) <= 14)
            {
                applyExplorationCurse(
                    player,
                    "unread_legend_weight",
                    "Poids d'une légende non lue",
                    "Trace ancienne de " + biome.name,
                    "Le personnage a touché une histoire qui ne veut pas rester simple rumeur. Elle pèse surtout dans les rêves et les regards.",
                    "retrouver la légende correspondante en bibliothèque, lire la contre-version, puis laisser les archives refermer l'histoire.",
                    "sleep,spirit,social",
                    1,
                    2,
                    true,
                    5,
                    0,
                    0,
                    false,
                    lines,
                    false,
                    0,
                    true
                );
            }
            showExplorationNotice("TRACES CONSERVÉES", "exploration.rare.discovery.traces", lines);
            return;
        }

        if (roll <= 76)
        {
            std::vector<std::string> lines = {
                "Une petite anomalie de matériaux pulse au sol.",
                "Tu n'en comprends pas tout, mais tu arrives à détacher un résidu stable.",
                addExplorationMaterial(player, "variation_residue", applyExplorationQuantityBonus(1, intensity), chooseExplorationQuality(random, true))
            };
            if (random.between(1, 100) <= 18)
            {
                applyExplorationCurse(
                    player,
                    "anchored_relic_shadow",
                    "Ombre d'objet lié",
                    "Reliquat instable de " + biome.name,
                    "Un objet invisible semble avoir choisi le personnage comme nouveau coffret. L'église peut le comprendre, pas forcément le retirer.",
                    "retrouver ou reconstituer l'objet source, puis le détruire dans un cercle sûr au lieu de l'exorciser directement.",
                    "equipment,corruption,luck",
                    2,
                    2,
                    false,
                    0,
                    0,
                    0,
                    false,
                    lines,
                    false,
                    0,
                    true
                );
            }
            appendExplorationQuestProgressLine(player, biome, 2, lines, "Les notes d'exploration progressent grâce à l'anomalie");
            showExplorationNotice("ANOMALIE DE MATÉRIAUX", "exploration.rare.discovery.anomaly", lines);
            return;
        }

        if (roll <= 90)
        {
            showExplorationNotice(
                "SILENCE FISSURÉ",
                "exploration.rare.discovery.boss_trace",
                {
                    "Le silence se fissure autour de toi.",
                    "Le carnet des boss ne grave aucun nom complet, mais ses pages tremblent comme devant une présence éveillée.",
                    "Trace perçue : " + bossTraceForBiome(biome) + "."
                }
            );

            ExplorationBossUnlockResult bossTrace = tryUnlockExplorationBossVariation(player, random, false, biome.name);
            std::vector<std::string> registryLines = {
                bossTrace.line,
                "Nom : ???",
                "Statut : emplacement approximatif découvert par exploration rarissime."
            };
            appendExplorationQuestProgressLine(
                player,
                biome,
                2,
                registryLines,
                "La trace de boss fait progresser les notes d'exploration"
            );
            showExplorationNotice(
                "REGISTRE DES BOSS",
                bossTrace.unlocked ? "exploration.rare.discovery.boss_trace.new" : "exploration.rare.discovery.boss_trace.old",
                registryLines
            );
            return;
        }

        showExplorationNotice(
            "DÉCOUVERTE RARISSIME",
            "exploration.rare.discovery.predator",
            {
                "Quelque chose t'a vu avant que tu ne le voies.",
                "Ton instinct refuse de rester, mais la chose est déjà trop proche : les armes doivent parler."
            }
        );

        Monster hunter = createExplorationMonsterForBiome(player, random, biome, intensity);
        bool victory = runTrackedExplorationWave(
            player,
            random,
            difficulty,
            deathRule,
            std::vector<Monster>{hunter},
            "Découverte rarissime : prédateur de " + biome.name
        );

        if (victory)
        {
            std::vector<std::string> lines = {
                addExplorationMaterial(player, biome.rareMaterialId, applyExplorationQuantityBonus(1, intensity), "exceptional")
            };
            appendExplorationQuestProgressLine(player, biome, 3, lines, "Le journal d'exploration progresse après cette rencontre rarissime");
            showExplorationNotice("RÉCOMPENSE DU PRÉDATEUR", "exploration.rare.discovery.predator.reward", lines);
        }
    }

    Quest buildNpcQuestByRoll(Player& player, int roll, std::string& intro, const std::string& biomeName = "")
    {
        if (!biomeName.empty())
        {
            std::vector<std::string> clients = {
                "Villageois nerveux", "Marchand inquiet", "Prunigil le marchand", "Forgeron", "Alchimiste", "Vendeur de composants",
                "Vendeur de matériaux", "Herboriste", "Armurier", "Vendeur d'armes", "Vendeur de consommables", "Bibliothécaire",
                "Sœur Cléria", "Père Lior", "Noé le sonneur",
                "Batia des barques", "Malo du quai bleu", "Ysée la brumeuse",
                "Tarek le carrier", "Blanche des fossiles", "Gorin au marteau pâle",
                "Niko sous le pont", "Vera aux dettes", "Gilda la troqueuse",
                "Rosalie des statues", "Ilan le jardinier muet", "Dame Séraphine"
            };

            std::string client = clients[std::clamp(roll, 1, static_cast<int>(clients.size())) - 1];
            intro = client + " te confie une demande liée à " + biomeName + ". Ce n'est pas un contrat officiel : plutôt un pourparler griffonné à la hâte.";
            return QuestCatalog::createBiomeRequest(player.getLevel(), biomeName, client);
        }

        if (roll == 1)
        {
            intro = "Un villageois nerveux t'intercepte avant que tu ne repartes.";
            return QuestCatalog::createVillagerMonsterFearRequest(player.getLevel());
        }

        if (roll == 2)
        {
            intro = biomeName.empty()
                ? "Un marchand inquiet te fait signe depuis le bord de la route."
                : "Un marchand inquiet s'est visiblement perdu jusque dans cette zone.";
            return QuestCatalog::createMerchantDeliveryRequest(player.getLevel());
        }

        if (roll == 3)
        {
            intro = biomeName.empty()
                ? "Le forgeron semble avoir besoin d'un service rapide."
                : "Un forgeron itinérant inspecte les environs et cherche des matériaux exploitables.";
            return QuestCatalog::createForgemasterMaterialRequest(player.getLevel());
        }

        if (roll == 4)
        {
            intro = biomeName.empty()
                ? "L'alchimiste surgit avec une liste d'ingrédients griffonnée de travers."
                : "Un alchimiste fouille la zone avec beaucoup trop d'enthousiasme.";
            return QuestCatalog::createAlchemistIngredientRequest(player.getLevel());
        }

        if (roll == 5)
        {
            intro = biomeName.empty()
                ? "Un vendeur de composants t'appelle avec un bocal vide à la main."
                : "Un vendeur de composants observe les traces de monstres avec un sourire commercial.";
            return QuestCatalog::createMonsterMaterialVendorRequest(player.getLevel());
        }

        if (roll == 6)
        {
            intro = biomeName.empty()
                ? "Un vendeur de matériaux cherche quelqu'un qui n'a pas peur de fouiller les restes."
                : "Un vendeur de matériaux te demande si tu comptes vraiment laisser tout ça au sol.";
            return QuestCatalog::createMaterialVendorRequest(player.getLevel());
        }

        if (roll == 7)
        {
            intro = biomeName.empty()
                ? "Une herboriste te demande de surveiller les plantes avant qu'elles ne fanent."
                : "Une herboriste reconnaît plusieurs plantes du biome et te propose une demande.";
            return QuestCatalog::createHerbalistRequest(player.getLevel());
        }

        if (roll == 8)
        {
            intro = biomeName.empty()
                ? "Un armurier cherche des pièces assez solides pour arrêter autre chose que du vent."
                : "Un armurier inspecte les dangers du coin et comprend vite ce qu'il lui manque.";
            return QuestCatalog::createArmorerRequest(player.getLevel());
        }

        if (roll == 9)
        {
            intro = biomeName.empty()
                ? "Un vendeur d'armes a besoin de matériaux avant que ses clients ne deviennent agressifs."
                : "Un vendeur d'armes itinérant pense que cette zone cache de bons composants.";
            return QuestCatalog::createWeaponVendorRequest(player.getLevel());
        }

        if (roll == 10)
        {
            intro = biomeName.empty()
                ? "Un vendeur de consommables manque d'ingrédients et essaie de ne pas paniquer."
                : "Un vendeur de consommables cherche des ingrédients avant que sa réserve ne devienne une blague.";
            return QuestCatalog::createConsumableVendorRequest(player.getLevel());
        }

        if (roll == 11)
        {
            Quest quest;
            quest.id = "church_cursed_patient_" + std::to_string(player.getWorldDaysElapsed()) + "_" + std::to_string(player.getLevel());
            quest.rank = player.getLevel() >= 25 ? "C" : "D";
            quest.title = "Patient marqué à accompagner";
            quest.origin = "PNJ notable";
            quest.client = "Sœur Maëlys l'exorciste";
            quest.location = "Église et exorcisme";
            quest.objective = "Apporter de quoi aider un PNJ porteur d'une trace inconnue, sans prétendre connaître sa malédiction à sa place.";
            quest.objectiveType = "material";
            quest.targetFamily = "Malédiction de PNJ";
            quest.requiredMaterialId = "exorcism_incense";
            quest.requiredMaterialName = "Encens d'exorcisme";
            quest.requiredMaterialQuantity = 1;
            quest.rewardExperience = 18 + player.getLevel() * 2;
            quest.rewardGold = 55 + player.getLevel() * 3;
            quest.rewardMaterialId = "blessing_note";
            quest.rewardMaterialName = "Note de bénédiction";
            quest.rewardMaterialQuantity = 1;
            quest.rewardNote = "L'église note que les PNJ peuvent aussi porter des malédictions, mais leur diagnostic reste un travail séparé.";
            quest.target = 1;
            intro = "Sœur Maëlys te parle d'un patient non-joueur qui refuse d'entrer dans l'église. Elle ne donne pas son diagnostic : elle demande seulement de préparer le rite.";
            return quest;
        }

        if (roll == 12)
        {
            Quest quest;
            quest.id = "church_oath_witness_" + std::to_string(player.getWorldDaysElapsed()) + "_" + std::to_string(player.getLevel());
            quest.rank = player.getLevel() >= 35 ? "B" : "C";
            quest.title = "Témoin d'un serment qui serre trop";
            quest.origin = "PNJ notable";
            quest.client = "Père Orwan";
            quest.location = "Église et archives";
            quest.objective = "Aider l'église à préparer le dossier d'un PNJ lié par un serment étrange. Le but est d'enquêter, pas de frapper le patient.";
            quest.objectiveType = "material";
            quest.targetFamily = "Serment maudit";
            quest.requiredMaterialId = "sanctuary_wax_seal";
            quest.requiredMaterialName = "Sceau de cire sanctuaire";
            quest.requiredMaterialQuantity = 1;
            quest.rewardExperience = 28 + player.getLevel() * 2;
            quest.rewardGold = 70 + player.getLevel() * 4;
            quest.rewardMaterialId = "exorcist_note";
            quest.rewardMaterialName = "Note d'exorciste";
            quest.rewardMaterialQuantity = 1;
            quest.rewardNote = "Les serments maudits se brisent rarement avec une simple prière : il faut souvent une preuve, un témoin ou une condition précise.";
            quest.target = 1;
            intro = "Père Orwan prépare un dossier sur un PNJ lié par un serment. Il insiste : certaines malédictions sociales se soignent par vérité, pas par violence.";
            return quest;
        }

        intro = biomeName.empty()
            ? "Une bibliothécaire veut vérifier des notes de terrain avant de les classer."
            : "Une bibliothécaire de terrain prend des notes sur ce biome et te demande de vérifier une hypothèse.";
        return QuestCatalog::createLibrarianRequest(player.getLevel());
    }

    // EN: displayQuestOffer declares or implements a focused behavior used by this module.
    // FR: displayQuestOffer déclare ou implémente un comportement précis utilisé par ce module.
    void displayQuestOffer(Player& player, const Quest& offeredQuest, const std::string& intro)
    {
        std::vector<std::string> introLines;
        if (!intro.empty())
        {
            introLines.push_back(intro);
        }

        if (!player.getQuestLog().canAcceptPersonalQuestForClient(offeredQuest.client))
        {
            MessageScreen::show(
                "DEMANDE BLOQUÉE",
                "quest.event.offer.blocked",
                {
                    offeredQuest.client + " a déjà deux demandes actives dans ton journal.",
                    "Tant qu'au moins une de ses demandes n'est pas rendue, ce PNJ évite de t'en confier une autre."
                }
            );
            return;
        }

        int choice = askQuestOfferDecision("ÉVÉNEMENT DE QUÊTE", "quest.event.offer", player, offeredQuest, introLines);
        Console::clear();

        if (choice == 1)
        {
            Quest acceptedQuest = offeredQuest;
            prepareQuestForAcceptance(acceptedQuest, player.getWorldDaysElapsed());

            if (player.getQuestLog().addQuest(acceptedQuest))
            {
                player.getQuestLog().refreshMaterialDeliveryQuests(player.getInventory());
                std::vector<std::string> lines = {"Demande ajoutée au journal : " + acceptedQuest.title};
                std::vector<std::string> dialogue = clientQuestAcceptedDialogueLines(player, acceptedQuest);
                lines.insert(lines.end(), dialogue.begin(), dialogue.end());
                appendDeadlineLine(lines, acceptedQuest, player.getWorldDaysElapsed());
                MessageScreen::show("DEMANDE ACCEPTÉE", "quest.event.offer.accepted", lines);
            }
            else
            {
                MessageScreen::show(
                    "DEMANDE REFUSÉE PAR LE JOURNAL",
                    "quest.event.offer.failed",
                    {
                        "Impossible d'ajouter cette demande au journal.",
                        "Elle est peut-être déjà active ou incompatible avec tes demandes actuelles."
                    }
                );
            }
        }
        else
        {
            MessageScreen::show(
                "DEMANDE REFUSÉE",
                "quest.event.offer.declined",
                {"Tu refuses la demande pour l'instant."}
            );
        }
    }

    // EN: offerExplorationNpcQuest declares or implements a focused behavior used by this module.
    // FR: offerExplorationNpcQuest déclare ou implémente un comportement précis utilisé par ce module.
    void offerExplorationNpcQuest(Player& player, Random& random, const ExplorationBiome& biome)
    {
        std::string intro;
        Quest offeredQuest = buildNpcQuestByRoll(player, random.between(1, 26), intro, biome.name);
        displayQuestOffer(player, offeredQuest, intro);
    }

    // EN: simulateExplorationMiniBoss declares or implements a focused behavior used by this module.
    // FR: simulateExplorationMiniBoss déclare ou implémente un comportement précis utilisé par ce module.
    void simulateExplorationMiniBoss(Player& player, Random& random, const ExplorationBiome& biome, const ExplorationIntensity& intensity, DifficultyMode difficulty, DeathRuleMode deathRule)
    {
        const bool giantSlimeEncounter = player.getLevel() >= 4 && random.between(1, 100) <= 12;
        bool evolved = !giantSlimeEncounter && random.between(1, 100) <= 45;
        std::string miniBossName = giantSlimeEncounter
            ? "Géant slime des quatre divisions"
            : miniBossNameForBiome(biome, evolved);
        std::string questFamily = giantSlimeEncounter
            ? "Mini-boss / slime à divisions successives"
            : miniBossQuestFamilyForBiome(biome, evolved);

        Monster miniBoss = giantSlimeEncounter
            ? MonsterCatalog::createGiantSlimeMiniBoss(std::max(4, player.getLevel() + random.between(0, 2)))
            : createExplorationEliteForBiome(player, random, biome, intensity);

        std::vector<std::string> introLines = {
            "L'air se tasse autour de toi.",
            "Mini-boss d'exploration : " + miniBossName + ".",
            "Forme rencontrée : " + miniBoss.getName() + " [niveau " + std::to_string(miniBoss.getLevel()) + "].",
            "Zone : " + biome.name + " | Approche : " + intensity.name + "."
        };

        if (giantSlimeEncounter)
        {
            introLines.push_back("Quatre noyaux sont emboîtés dans sa masse : Géant slime, Gros slime, Slime, Petit slime, puis Âme du slime.");
            introLines.push_back("Chaque sous-forme se divisera à son tour. Les dégâts restent modérés, mais la file ennemie risque de déborder.");
        }
        else if (evolved)
        {
            introLines.push_back("Cette chose ressemble à une version évoluée d'un monstre local.");
        }

        showExplorationNotice("MINI-BOSS D'EXPLORATION", "exploration.miniboss.intro", introLines);

        bool victory = runTrackedExplorationWave(
            player,
            random,
            difficulty,
            deathRule,
            std::vector<Monster>{miniBoss},
            "Mini-boss d'exploration : " + miniBossName
        );

        if (!victory)
        {
            return;
        }

        std::vector<std::string> rewardLines = {
            addExplorationMaterial(
                player,
                giantSlimeEncounter ? "slime_residue" : (evolved ? biome.rareMaterialId : biome.commonMaterialId),
                giantSlimeEncounter ? applyExplorationQuantityBonus(random.between(4, 7), intensity) : applyExplorationQuantityBonus(1, intensity),
                chooseExplorationQuality(random, giantSlimeEncounter || evolved)
            )
        };
        if (giantSlimeEncounter)
        {
            rewardLines.push_back("Le noyau principal s'est dissous après quatre étages complets de division.");
            rewardLines.push_back("Les Âmes de slime finales ont été comptées comme des invocations ennemies.");
        }

        int updated = player.getQuestLog().progressCombatQuestsByFamily(evolved ? 2 : 1, questFamily);
        if (updated > 0)
        {
            rewardLines.push_back("Des quêtes de combat progressent grâce à cette rencontre.");
        }

        showExplorationNotice("RÉCOMPENSE DU MINI-BOSS", "exploration.miniboss.reward", rewardLines);
    }

    // EN: simulateAfterCombatMiniBoss declares or implements a focused behavior used by this module.
    // FR: simulateAfterCombatMiniBoss déclare ou implémente un comportement précis utilisé par ce module.
    void simulateAfterCombatMiniBoss(Player& player, Random& random, DifficultyMode difficulty, DeathRuleMode deathRule)
    {
        bool evolved = random.between(1, 100) <= 35;
        std::string miniBossName = evolved ? "forme évoluée attirée par le sang" : "menace opportuniste";
        std::string questFamily = evolved ? "Mini-boss / menace évoluée" : "Élite / menace";

        int enemyCount = evolved ? random.between(1, 2) : 1;
        std::vector<Monster> monsters;

        for (int i = 0; i < enemyCount; ++i)
        {
            int levelOffset = evolved ? random.between(1, 4) : random.between(-1, 2);
            Monster monster = MonsterCatalog::createRandomMonsterForLevel(
                std::max(1, player.getLevel() + levelOffset),
                random
            );

            if (evolved)
            {
                monster = MonsterCatalog::createEvolvedVariant(monster, random);
            }

            monsters.push_back(monster);
        }

        int choice = askChoiceScreen(
            "ÉVÉNEMENT APRÈS-COMBAT",
            "exploration.after_combat.choice",
            {
                "Tu pensais pouvoir souffler, mais quelque chose a suivi le bruit du combat.",
                "Mini-boss détecté : " + miniBossName + ".",
                "La menace est trop proche pour être ignorée : il va falloir survivre."
            },
            {{1, "Affronter la menace"}, {0, "Tenter de l'éviter avant contact"}},
            0,
            1
        );
        Console::clear();

        if (choice == 0)
        {
            int escapeChance = evolved ? 45 : 65;
            if (random.between(1, 100) <= escapeChance)
            {
                showExplorationNotice(
                    "MENACE ÉVITÉE",
                    "exploration.after_combat.escaped",
                    {
                        "Tu t'éloignes avant que la menace ne verrouille vraiment ta position.",
                        "L'événement est évité, mais aucune récompense supplémentaire n'est obtenue."
                    }
                );
                return;
            }

            showExplorationNotice(
                "TROP TARD",
                "exploration.after_combat.escape_failed",
                {"La menace a déjà senti ta fatigue."}
            );
        }

        bool victory = runTrackedExplorationWave(
            player,
            random,
            difficulty,
            deathRule,
            monsters,
            "Événement après-combat : " + miniBossName
        );

        if (!victory)
        {
            return;
        }

        std::vector<std::string> resultLines = {
            "La menace attirée par le combat est repoussée.",
            evolved ? "Nature : forme évoluée / dangereuse." : "Nature : menace opportuniste."
        };

        appendCombatQuestProgressLine(
            player,
            evolved ? 2 : 1,
            questFamily,
            resultLines,
            "Le sang versé après l'embuscade fait avancer les contrats de chasse"
        );

        showExplorationNotice(
            "APRÈS-COMBAT STABILISÉ",
            "exploration.after_combat.result",
            resultLines
        );
    }

    // EN: openDangerousExplorationSite declares or implements a focused behavior used by this module.
    // FR: openDangerousExplorationSite déclare ou implémente un comportement précis utilisé par ce module.
    void openDangerousExplorationSite(Player& player, Random& random, const ExplorationBiome& biome, const ExplorationIntensity& intensity, DifficultyMode difficulty, DeathRuleMode deathRule)
    {
        int visitChoice = askChoiceScreen(
            "LIEU DANGEREUX",
            "exploration.dangerous_site.choice",
            {
                "Tu remarques un passage récent vers un lieu qui n'a clairement pas envie d'être visité.",
                "Lieu repéré : " + dangerousSiteNameForBiome(biome) + ".",
                dangerousSiteWarningForBiome(biome),
                "L'air est trop lourd, les traces trop profondes, et ton instinct te conseille poliment de rentrer."
            },
            {{1, "Visiter quand même ce lieu dangereux"}, {0, "Ignorer l'endroit"}},
            0,
            1
        );
        Console::clear();

        if (visitChoice == 0)
        {
            showExplorationNotice(
                "LIEU IGNORÉ",
                "exploration.dangerous_site.ignored",
                {"Tu décides de ne pas offrir ton nom au premier trou suspect venu."}
            );
            return;
        }

        bool bossEntrance = random.between(1, 100) <= 35;

        if (!bossEntrance)
        {
            int fightChoice = askChoiceScreen(
                "EMBUSCADE NATURELLE",
                "exploration.dangerous_site.wave_choice",
                {
                    "Le lieu abrite une vague de gros monstres.",
                    "Ce n'est pas un simple détour : c'est une embuscade naturelle.",
                    "Tu reconnais assez la zone pour comprendre que ce danger appartient à " + biome.name + "."
                },
                {{1, "Tenter l'affrontement"}, {0, "Reculer maintenant"}},
                0,
                1
            );
            Console::clear();

            if (fightChoice == 0)
            {
                showExplorationNotice(
                    "REPLI",
                    "exploration.dangerous_site.wave_retreat",
                    {"Tu recules avant que la zone ne se referme sur toi."}
                );
                return;
            }

            std::vector<Monster> monsters;
            int enemyCount = random.between(2, 4);
            for (int i = 0; i < enemyCount; ++i)
            {
                monsters.push_back(createExplorationMonsterForBiome(player, random, biome, intensity));
            }

            bool victory = runTrackedExplorationWave(
                player,
                random,
                difficulty,
                deathRule,
                monsters,
                "Lieu dangereux : vague de " + biome.name
            );

            if (victory)
            {
                std::vector<std::string> rewardLines = {
                    addExplorationMaterial(player, biome.rareMaterialId, applyExplorationQuantityBonus(1, intensity), chooseExplorationQuality(random, true))
                };
                appendCombatQuestProgressLine(
                    player,
                    3,
                    "Menace avancée",
                    rewardLines,
                    "Des contrats de menace avancée progressent grâce à ce lieu"
                );
                showExplorationNotice("LIEU NETTOYÉ", "exploration.dangerous_site.wave_reward", rewardLines);
            }
            return;
        }

        int bossChoice = askChoiceScreen(
            "ENTRÉE DE BOSS",
            "exploration.dangerous_site.boss_choice",
            {
                "Ce n'est pas une simple tanière.",
                "C'est une entrée de boss.",
                "Description rapide : " + bossTraceForBiome(biome) + ",",
                "mais trop brouillée pour que le registre accepte son nom.",
                "Le sol vibre comme si une variation d'énergie anormale venait de respirer."
            },
            {{1, "Tenter l'affrontement malgré l'avertissement"}, {0, "Reculer et mémoriser l'entrée"}},
            0,
            1
        );
        Console::clear();

        if (bossChoice == 0)
        {
            int updated = progressExplorationQuests(player, biome.name, 1);
            std::vector<std::string> lines = {"Tu recules. Le registre note seulement : Boss potentiel — nom inconnu."};
            if (updated > 0)
            {
                lines.push_back("Des notes d'exploration progressent grâce à cette entrée mémorisée.");
            }
            showExplorationNotice("ENTRÉE MÉMORISÉE", "exploration.dangerous_site.boss_retreat", lines);
            return;
        }

        std::vector<std::string> bossLines = {
            "Tu franchis la limite... puis ton instinct te ramène brutalement en arrière.",
            "Le carnet des boss grave maintenant son sceau. Reviens par cette voie si tu veux vraiment l'affronter."
        };

        ExplorationBossUnlockResult bossTrace = tryUnlockExplorationBossVariation(player, random, true, biome.name);
        bossLines.push_back(bossTrace.line);
        if (bossTrace.unlocked)
        {
            bossLines.push_back("Nom : ???");
            bossLines.push_back("Statut : entrée approximative mémorisée après une découverte dangereuse rarissime.");
        }

        int updated = progressExplorationQuests(player, biome.name, 2);
        if (updated > 0)
        {
            bossLines.push_back("Des notes d'exploration progressent grâce à cette découverte dangereuse.");
        }

        if (random.between(1, 100) <= 22)
        {
            applyExplorationCurse(
                player,
                "boss_threshold_omen",
                "Présage de seuil",
                "Entrée de boss inconnue — " + biome.name,
                "Le personnage a touché une limite de boss sans l'affronter. Le seuil garde une copie imparfaite de son passage.",
                "retrouver l'entrée, effectuer un diagnostic total, puis affronter ou sceller la source selon le boss concerné.",
                "spirit,corruption,sleep",
                2,
                4,
                true,
                3,
                0,
                3,
                true,
                bossLines
            );
        }

        showExplorationNotice("REGISTRE DES BOSS", "exploration.dangerous_site.boss_register", bossLines);
    }


    std::vector<Monster> createExplorationGroup(Player& player, Random& random, const ExplorationBiome& biome, const ExplorationIntensity& intensity, int minCount, int maxCount, bool allowEvolved)
    {
        int count = random.between(minCount, maxCount);
        std::vector<Monster> monsters;

        for (int i = 0; i < count; ++i)
        {
            Monster monster = createExplorationMonsterForBiome(player, random, biome, intensity);
            if (allowEvolved && random.between(1, 100) <= 22)
            {
                monster = MonsterCatalog::createEvolvedVariant(monster, random);
            }
            monsters.push_back(monster);
        }

        return monsters;
    }

    void triggerActiveExplorationEvent(Player& player, Random& random, const ExplorationBiome& biome, const ExplorationIntensity& intensity, DifficultyMode difficulty, DeathRuleMode deathRule)
    {
        int eventRoll = chooseVariedActiveExplorationRoll(player, random);
        const std::string activeEventKey = activeExplorationEventKeyFromRoll(eventRoll);
        player.recordExplorationEventKey(activeEventKey);
        player.startExplorationSceneCooldown(activeEventKey, activeExplorationEventCooldownDays(activeEventKey));

        showExplorationNotice(
            "ÉVÉNEMENT D'EXPLORATION",
            "exploration.event.start",
            {"La zone répond à ta présence."}
        );

        if (eventRoll <= 18)
        {
            int choice = askChoiceScreen(
                "CAMP ABANDONNÉ",
                "exploration.event.abandoned_camp",
                {"Tu découvres un camp abandonné. Le feu est éteint, mais les cendres sont encore tièdes."},
                {{1, "Fouiller vite"}, {2, "Inspecter prudemment les traces"}, {0, "Quitter le camp"}},
                0,
                2
            );
            Console::clear();

            if (choice == 0)
            {
                showExplorationNotice("CAMP QUITTÉ", "exploration.event.abandoned_camp.leave", {"Tu quittes le camp. Certains silences ne méritent pas d'être ouverts."});
                return;
            }

            if (choice == 1 || random.between(1, 100) <= 45)
            {
                showExplorationNotice("CAMP HABITÉ", "exploration.event.abandoned_camp.ambush", {"Des silhouettes reviennent vers le camp. Ce n'était pas si abandonné."});
                bool victory = runTrackedExplorationWave(
                    player,
                    random,
                    difficulty,
                    deathRule,
                    createExplorationGroup(player, random, biome, intensity, 2, 3, false),
                    "Camp abandonné : retour des occupants"
                );

                if (!victory)
                {
                    return;
                }
            }
            else
            {
                std::vector<std::string> lines = {"Tu lis correctement les traces et évites l'embuscade avant qu'elle ne se referme."};
                appendExplorationQuestProgressLine(player, biome, 1, lines, "Les traces du camp font progresser ton journal");
                showExplorationNotice("EMBUSCADE ÉVITÉE", "exploration.event.abandoned_camp.avoid", lines);
            }

            int gold = applyExplorationGoldReward(random.between(12, 38 + player.getLevel() * 2), player, intensity, difficulty, 1);
            player.getInventory().earnGold(gold);
            std::vector<std::string> rewardLines = {
                "Tu récupères dans le camp : " + Money::formatGoldWithRaw(gold) + ".",
                addExplorationMaterial(player, biome.commonMaterialId, applyExplorationQuantityBonus(1, intensity), chooseExplorationQuality(random, true))
            };
            showExplorationNotice("CAMP FOUILLÉ", "exploration.event.abandoned_camp.reward", rewardLines);
            return;
        }

        if (eventRoll <= 32)
        {
            int choice = askChoiceScreen(
                "REPAIRE LOCAL",
                "exploration.event.local_den",
                {"Tu tombes sur un nid / repaire local.", "Il y a des ressources dedans, mais aussi des propriétaires."},
                {{1, "Nettoyer le repaire"}, {0, "Ne pas provoquer la zone"}},
                0,
                1
            );
            Console::clear();

            if (choice == 0)
            {
                std::vector<std::string> lines = {"Tu marques mentalement le lieu, mais tu ne vas pas mourir pour trois bouts de cuir."};
                appendExplorationQuestProgressLine(player, biome, 1, lines, "Le repaire noté fait progresser une demande d'exploration");
                showExplorationNotice("REPAIRE IGNORÉ", "exploration.event.local_den.leave", lines);
                return;
            }

            bool victory = runTrackedExplorationWave(
                player,
                random,
                difficulty,
                deathRule,
                createExplorationGroup(player, random, biome, intensity, 2, 5, isBiomeEvolvedForPlayer(player, biome)),
                "Repaire local : " + biome.name
            );

            if (victory)
            {
                std::vector<std::string> rewardLines = {
                    addExplorationMaterial(player, biome.commonMaterialId, applyExplorationQuantityBonus(random.between(1, 2), intensity), chooseExplorationQuality(random, true))
                };
                if (random.between(1, 100) <= 45)
                {
                    rewardLines.push_back(addExplorationMaterial(player, biome.rareMaterialId, applyExplorationQuantityBonus(1, intensity), chooseExplorationQuality(random, true)));
                }
                appendCombatQuestProgressLine(player, 2, "Créatures locales", rewardLines, "Des contrats de créatures locales progressent");
                showExplorationNotice("REPAIRE NETTOYÉ", "exploration.event.local_den.reward", rewardLines);
            }
            return;
        }

        if (eventRoll <= 46)
        {
            int choice = askChoiceScreen(
                "TRACE DE TERRITOIRE",
                "exploration.event.tracks",
                {
                    "Tu repères une trace de territoire : griffures, mucus, cendre ou ossements selon le lieu.",
                    "La trace ressemble surtout à une information de terrain, pas à une menace immédiate."
                },
                {{1, "Étudier les traces"}, {2, "Suivre la piste"}, {0, "Ne pas t'attarder"}},
                0,
                2
            );
            Console::clear();

            if (choice == 0)
            {
                std::vector<std::string> lines = {"Tu notes mentalement le lieu, sans jouer au héros inutilement."};
                appendExplorationQuestProgressLine(player, biome, 1, lines, "La trace notée fait progresser le journal");
                showExplorationNotice("TRACE IGNORÉE", "exploration.event.tracks.leave", lines);
                return;
            }

            std::string clue = "Trace étudiée : " + biome.name + " favorise " + biome.commonMonsters
                + ". Présences rares possibles : " + biome.rareMonsters + ".";
            recordBiomeFieldObservation(biome, clue);
            std::vector<std::string> lines = {"Le bestiaire ajoute une observation de terrain sur " + biome.name + "."};
            appendExplorationQuestProgressLine(player, biome, choice == 1 ? 2 : 1, lines, "Les traces étudiées font progresser le journal");
            showExplorationNotice("BESTIAIRE", "exploration.event.tracks.bestiary", lines);

            if (choice == 2)
            {
                showExplorationNotice("PISTE SUIVIE", "exploration.event.tracks.follow", {"Suivre la piste attire ce qui l'a laissée."});
                bool victory = runTrackedExplorationWave(
                    player,
                    random,
                    difficulty,
                    deathRule,
                    createExplorationGroup(player, random, biome, intensity, 1, 3, true),
                    "Piste suivie : " + biome.name
                );

                if (victory)
                {
                    showExplorationNotice(
                        "RESSOURCE RÉCUPÉRÉE",
                        "exploration.reward.material",
                        {addExplorationMaterial(player, biome.commonMaterialId, applyExplorationQuantityBonus(1, intensity), chooseExplorationQuality(random, true))}
                    );
                }
            }

            return;
        }

        if (eventRoll <= 58)
        {
            int choice = askChoiceScreen(
                "OBSTACLE DE TERRAIN",
                "exploration.event.hazard",
                {"Obstacle de terrain : " + environmentalHazardTextForBiome(biome) + "."},
                {
                    {1, "Observer et contourner prudemment"},
                    {2, "Récupérer proprement ce qui peut l'être"},
                    {3, "Forcer le passage"},
                    {0, "Ne pas prendre ce risque"}
                },
                0,
                3
            );
            Console::clear();

            if (choice == 0)
            {
                showExplorationNotice("OBSTACLE ÉVITÉ", "exploration.event.hazard.leave", {"Tu refuses le pari. Certains obstacles sont là pour faire perdre du temps aux survivants trop pressés."});
                return;
            }

            if (choice == 1)
            {
                std::vector<std::string> lines = {environmentalObservationForBiome(biome)};
                recordBiomeFieldObservation(biome, environmentalObservationForBiome(biome));
                appendExplorationQuestProgressLine(player, biome, 2, lines, "L'observation prudente fait progresser le journal");
                showExplorationNotice("OBSERVATION", "exploration.event.hazard.observe", lines);
                return;
            }

            if (choice == 2)
            {
                int successChance = 62 + intensity.carefulBonus * 4;
                if (random.between(1, 100) <= successChance)
                {
                    std::vector<std::string> rewardLines = {
                        "Tu récupères sans réveiller toute la zone.",
                        addExplorationMaterial(player, biome.commonMaterialId, applyExplorationQuantityBonus(1, intensity), chooseExplorationQuality(random, true))
                    };
                    if (random.between(1, 100) <= 28)
                    {
                        rewardLines.push_back(addExplorationMaterial(player, biome.rareMaterialId, 1, chooseExplorationQuality(random, true)));
                    }
                    appendExplorationQuestProgressLine(player, biome, 2, rewardLines, "Le terrain récupéré proprement fait progresser le journal");
                    showExplorationNotice("RÉCUPÉRATION RÉUSSIE", "exploration.event.hazard.collect_success", rewardLines);
                    return;
                }

                int damage = std::min(random.between(4, 12 + player.getLevel()), std::max(0, player.getHp() - 1));
                std::vector<std::string> lines = {"La zone répond mal à ta récupération."};
                if (damage > 0)
                {
                    player.takeDamage(damage);
                    lines.push_back("Tu subis " + std::to_string(damage) + " dégâts, mais tu gardes le contrôle.");
                }
                recordBiomeFieldObservation(biome, environmentalObservationForBiome(biome));
                appendExplorationQuestProgressLine(player, biome, 1, lines, "Même ratée, la récupération laisse des notes utiles");
                showExplorationNotice("RÉCUPÉRATION RISQUÉE", "exploration.event.hazard.collect_fail", lines);
                return;
            }

            showExplorationNotice("PASSAGE FORCÉ", "exploration.event.hazard.force", {"Tu forces le passage. La zone n'aime pas ça."});
            bool victory = runTrackedExplorationWave(
                player,
                random,
                difficulty,
                deathRule,
                createExplorationGroup(player, random, biome, intensity, 1, 2, true),
                "Obstacle forcé : " + biome.name
            );

            if (victory)
            {
                std::vector<std::string> rewardLines = {
                    addExplorationMaterial(player, biome.rareMaterialId, applyExplorationQuantityBonus(1, intensity), chooseExplorationQuality(random, true))
                };
                appendExplorationQuestProgressLine(player, biome, 2, rewardLines, "Le passage forcé fait progresser les notes d'exploration");
                showExplorationNotice("PASSAGE OUVERT", "exploration.event.hazard.force_reward", rewardLines);
            }
            return;
        }

        if (eventRoll <= 68)
        {
            int choice = askChoiceScreen(
                "CACHE CLANDESTINE",
                "exploration.event.hidden_cache",
                {
                    "Un marchand clandestin a caché une caisse sous des marques trop propres.",
                    "Ce n'est pas une boutique complète, plutôt une cache suspecte."
                },
                {{1, "Ouvrir la cache"}, {0, "La laisser tranquille"}},
                0,
                1
            );
            Console::clear();

            if (choice == 0)
            {
                showExplorationNotice("CACHE IGNORÉE", "exploration.event.hidden_cache.leave", {"Tu refuses de voler quelqu'un qui vend probablement déjà des choses volées."});
                return;
            }

            if (random.between(1, 100) <= 55)
            {
                showExplorationNotice("CACHE SURVEILLÉE", "exploration.event.hidden_cache.guards", {"La cache était surveillée. Des gardes privés ou voleurs reviennent la défendre."});
                bool victory = runTrackedExplorationWave(
                    player,
                    random,
                    difficulty,
                    deathRule,
                    createExplorationGroup(player, random, biome, intensity, 1, 3, false),
                    "Cache clandestine : défenseurs du marché noir"
                );

                if (!victory)
                {
                    return;
                }
            }

            std::vector<std::string> illegalFinds = {
                "barbed_arrows", "piercing_bolts", "balanced_throwing_knives", "ash_arrows", "frozen_bolts", "conductive_knives", "unstable_core", "shadow_thread"
            };
            std::string found = illegalFinds[random.between(0, static_cast<int>(illegalFinds.size()) - 1)];
            showExplorationNotice(
                "CACHE OUVERTE",
                "exploration.event.hidden_cache.reward",
                {addExplorationMaterial(player, found, random.between(1, 3), chooseExplorationQuality(random, true))}
            );
            return;
        }

        if (eventRoll <= 80)
        {
            showExplorationNotice(
                "VAGUE FORCÉE",
                "exploration.event.wave",
                {
                    "La zone change de rythme : plusieurs créatures semblent fuir quelque chose... vers toi.",
                    "Le sol tremble sous une vraie vague de présences hostiles."
                }
            );

            bool victory = runTrackedExplorationWave(
                player,
                random,
                difficulty,
                deathRule,
                createExplorationGroup(player, random, biome, intensity, 3, 6, isBiomeEvolvedForPlayer(player, biome)),
                "Vague forcée par la zone : " + biome.name
            );

            if (victory)
            {
                std::vector<std::string> rewardLines = {
                    addExplorationMaterial(player, biome.commonMaterialId, applyExplorationQuantityBonus(1, intensity), chooseExplorationQuality(random, false))
                };
                appendExplorationQuestProgressLine(player, biome, 2, rewardLines, "La vague forcée fait progresser les notes d'exploration");
                showExplorationNotice("VAGUE REPOUSSÉE", "exploration.event.wave.reward", rewardLines);
            }
            return;
        }

        if (eventRoll <= 91)
        {
            int choice = askChoiceScreen(
                "SIGNE ANCIEN",
                "exploration.event.ancient_sign",
                {"Tu trouves un autel / signe ancien lié au biome."},
                {{1, "Étudier le signe"}, {2, "Tenter de prélever un fragment"}, {3, "Murmurer une promesse au signe"}, {0, "Ne pas toucher"}},
                0,
                3
            );
            Console::clear();

            if (choice == 0)
            {
                std::vector<std::string> lines = {"Tu respectes l'endroit. Le registre note quand même la position."};
                appendExplorationQuestProgressLine(player, biome, 1, lines, "La position du signe fait progresser le journal");
                showExplorationNotice("SIGNE RESPECTÉ", "exploration.event.ancient_sign.leave", lines);
                return;
            }

            if (choice == 1)
            {
                std::vector<std::string> lines = {"Tu prends des notes. Le bestiaire garde maintenant cette observation de terrain."};
                recordBiomeFieldObservation(
                    biome,
                    "Signe ancien étudié : le biome " + biome.name + " semble lié à des variations locales et à des présences plus rares."
                );
                appendExplorationQuestProgressLine(player, biome, 3, lines, "L'étude du signe fait progresser fortement le journal");
                if (random.between(1, 100) <= 35)
                {
                    lines.push_back(addExplorationMaterial(player, "variation_residue", 1, chooseExplorationQuality(random, true)));
                }
                if (random.between(1, 100) <= 10)
                {
                    lines.push_back("Le signe ne t'attaque pas, mais il garde l'empreinte de ton regard plus longtemps que prévu.");
                    applyExplorationCurse(
                        player,
                        "abandoned_altar_brand",
                        "Empreinte d'autel abandonné",
                        "Signe ancien de " + biome.name,
                        "Un autel sans prêtre a reconnu un témoin. La trace ne demande pas un soin, mais un scellement du lieu compris.",
                        "diagnostic total, croquis de source scellable, puis scellement du lieu.",
                        "spirit,corruption,sleep",
                        1,
                        3,
                        true,
                        6,
                        0,
                        0,
                        true,
                        lines,
                        false,
                        0,
                        true
                    );
                }
                showExplorationNotice("SIGNE ÉTUDIÉ", "exploration.event.ancient_sign.study", lines);
                return;
            }

            if (choice == 3)
            {
                std::vector<std::string> lines = {
                    "Tu murmures une promesse courte au signe, sans vraiment savoir à qui tu parles.",
                    "La zone répond par une aide immédiate... et par un silence beaucoup trop poli."
                };
                player.getInventory().earnGold(applyExplorationGoldReward(random.between(20, 55 + player.getLevel() * 3), player, intensity, difficulty, 1));
                lines.push_back(addExplorationMaterial(player, "variation_residue", 1, chooseExplorationQuality(random, true)));
                applyExplorationCurse(
                    player,
                    "voluntary_pact_mark",
                    "Marque de pacte volontaire",
                    "Promesse murmurée à un signe ancien de " + biome.name,
                    "La trace vient d'un choix : une aide a été acceptée, donc la sortie demande de nommer la contrepartie avant de rompre le pacte.",
                    "diagnostic total, témoin de pacte rompu, puis rupture volontaire à l'église.",
                    "luck,social,spirit",
                    1,
                    3,
                    true,
                    4,
                    0,
                    0,
                    true,
                    lines,
                    false,
                    0,
                    true
                );
                showExplorationNotice("PACTE MURMURÉ", "exploration.event.ancient_sign.pact", lines);
                return;
            }

            showExplorationNotice("FRAGMENT INSTABLE", "exploration.event.ancient_sign.fragment", {"Le fragment refuse d'être prélevé gratuitement."});
            bool victory = runTrackedExplorationWave(
                player,
                random,
                difficulty,
                deathRule,
                createExplorationGroup(player, random, biome, intensity, 1, 2, true),
                "Autel instable : réaction de " + biome.name
            );

            if (victory)
            {
                std::vector<std::string> rewardLines = {
                    addExplorationMaterial(player, "variation_residue", applyExplorationQuantityBonus(1, intensity), chooseExplorationQuality(random, true)),
                    addExplorationMaterial(player, biome.rareMaterialId, 1, chooseExplorationQuality(random, true))
                };
                if (random.between(1, 100) <= 14)
                {
                    rewardLines.push_back("Le fragment se stabilise, mais l'autel garde une marque sur le passage que tu viens de forcer.");
                    applyExplorationCurse(
                        player,
                        "abandoned_altar_brand",
                        "Empreinte d'autel abandonné",
                        "Fragment prélevé à " + biome.name,
                        "L'autel a perdu un fragment, mais il a gardé l'idée de celui qui l'a pris. Le lieu doit être scellé ou compris.",
                        "diagnostic total, croquis de source scellable, puis scellement du lieu.",
                        "corruption,equipment,spirit",
                        1,
                        3,
                        true,
                        5,
                        0,
                        0,
                        true,
                        rewardLines,
                        false,
                        0,
                        true
                    );
                }
                appendExplorationQuestProgressLine(player, biome, 2, rewardLines, "Le fragment instable fait progresser les notes d'exploration");
                showExplorationNotice("FRAGMENT STABILISÉ", "exploration.event.ancient_sign.fragment_reward", rewardLines);
            }
            return;
        }

        int choice = askChoiceScreen(
            "APPEL AU SECOURS",
            "exploration.event.distress_call",
            {"Tu entends un appel humain ou semi-humain, blessé, quelque part hors du chemin."},
            {{1, "Porter secours"}, {0, "Rester concentré sur ta survie"}},
            0,
            1
        );
        Console::clear();

        if (choice == 0)
        {
            showExplorationNotice("APPEL IGNORÉ", "exploration.event.distress_call.leave", {"Tu continues ta route. Ce monde punit parfois les héros trop confiants."});
            return;
        }

        bool ambush = random.between(1, 100) <= 50;
        if (ambush)
        {
            showExplorationNotice("EMBUSCADE", "exploration.event.distress_call.ambush", {"L'appel était un piège, ou la personne était déjà suivie."});
            bool victory = runTrackedExplorationWave(
                player,
                random,
                difficulty,
                deathRule,
                createExplorationGroup(player, random, biome, intensity, 2, 4, false),
                "Secours dangereux : embuscade"
            );

            if (!victory)
            {
                return;
            }
        }
        else
        {
            showExplorationNotice("SECOURS RÉUSSI", "exploration.event.distress_call.saved", {"Cette fois, ce n'était pas un piège. Une personne te doit probablement la vie."});
        }

        int gold = applyExplorationGoldReward(random.between(18, 55 + player.getLevel() * 2), player, intensity, difficulty, 2);
        player.getInventory().earnGold(gold);
        std::vector<std::string> rewardLines = {"Récompense improvisée : " + Money::formatGoldWithRaw(gold) + "."};
        appendExplorationQuestProgressLine(
            player,
            biome,
            1,
            rewardLines,
            "Le secours laisse assez de traces pour faire progresser le journal",
            "Aucune quête active ne reprend ce secours, mais le registre garde l'écho de l'appel."
        );
        showExplorationNotice("RÉCOMPENSE IMPROVISÉE", "exploration.event.distress_call.reward", rewardLines);
        offerExplorationNpcQuest(player, random, biome);
    }

}

// EN: openQuestHub declares or implements a focused behavior used by this module.
// FR: openQuestHub déclare ou implémente un comportement précis utilisé par ce module.
void QuestMenu::syncMainStoryQuests(Player& player)
{
    player.getQuestLog().refreshLinkedQuestProgress();

    if (!player.hasStoryModeStarted())
    {
        return;
    }

    if (player.getStoryChapter() == 1)
    {
        if (player.getStoryStep() >= 3)
        {
            // Après Mira, les quatre référents existent en parallèle. Leur vraie
            // quête principale est créée au moment où le joueur leur parle.
            addNonRefusableQuestIfMissing(player, createChapterOneMeetReferentsQuest());
        }

        syncChapterOneLinkedQuestProgress(player);

        if (questIsTurnedInInLog(player, "story_ch1_meet_referents") || player.getStoryStep() >= 4)
        {
            addNonRefusableQuestIfMissing(player, createChapterOneMiraMainQuest());
            syncChapterOneLinkedQuestProgress(player);
        }

        if (questIsTurnedInInLog(player, "story_ch1_mira_main") && player.getStoryStep() < 5)
        {
            player.setStoryProgress(1, 5, std::max(1, player.getStoryCityDevelopmentLevel()));
        }
        return;
    }

    if (player.getStoryChapter() == 2)
    {
        addNonRefusableQuestIfMissing(player, createChapterTwoBriefingQuest());

        if (player.getStoryStep() >= 2)
        {
            completeAndTurnInQuestSilently(player, "story_ch2_relay_briefing");
            addNonRefusableQuestIfMissing(player, createChapterTwoNorthRoadQuest());
        }

        if (player.getStoryStep() <= 2 && questIsTurnedInInLog(player, "story_ch2_north_road_scout"))
        {
            player.setStoryProgress(2, 3, std::max(2, player.getStoryCityDevelopmentLevel()));
        }

        if (player.getStoryStep() >= 3)
        {
            addNonRefusableQuestIfMissing(player, createChapterTwoTurnedMarkerQuest());
        }

        if (player.getStoryStep() <= 3 && questIsTurnedInInLog(player, "story_ch2_turned_marker"))
        {
            player.setStoryProgress(2, 4, std::max(2, player.getStoryCityDevelopmentLevel()));
        }

        if (player.getStoryStep() >= 4)
        {
            addNonRefusableQuestIfMissing(player, createChapterTwoRelayThreatQuest());
        }

        if (player.getStoryStep() <= 4 && questIsTurnedInInLog(player, "story_ch2_relay_threat"))
        {
            player.setStoryProgress(2, 5, std::max(3, player.getStoryCityDevelopmentLevel()));
        }

        if (player.getStoryStep() >= 5)
        {
            addNonRefusableQuestIfMissing(player, createChapterTwoRelaySignalQuest());
        }

        if (player.getStoryStep() <= 5 && questIsTurnedInInLog(player, "story_ch2_relay_signal"))
        {
            player.setStoryProgress(2, 6, std::max(3, player.getStoryCityDevelopmentLevel()));
        }

        if (player.getStoryStep() >= 6)
        {
            addNonRefusableQuestIfMissing(player, createChapterTwoFirstRescueQuest());
        }

        if (player.getStoryStep() <= 6 && questIsTurnedInInLog(player, "story_ch2_first_rescue"))
        {
            player.setStoryProgress(2, 7, std::max(4, player.getStoryCityDevelopmentLevel()));
        }

        if (player.getStoryStep() >= 7)
        {
            addNonRefusableQuestIfMissing(player, createChapterTwoRouteSackQuest());
        }

        if (player.getStoryStep() <= 7 && questIsTurnedInInLog(player, "story_ch2_route_sack"))
        {
            player.setStoryProgress(2, 8, std::max(4, player.getStoryCityDevelopmentLevel()));
        }

        if (player.getStoryStep() >= 8)
        {
            addNonRefusableQuestIfMissing(player, createChapterTwoCityRecoveryQuest());
        }

        if (player.getStoryStep() <= 8 && questIsTurnedInInLog(player, "story_ch2_city_recovery"))
        {
            player.setStoryProgress(2, 9, std::max(5, player.getStoryCityDevelopmentLevel()));
        }

        if (player.getStoryStep() >= 9)
        {
            addNonRefusableQuestIfMissing(player, createChapterTwoColdInkTrailQuest());
        }

        if (player.getStoryStep() <= 9 && questIsTurnedInInLog(player, "story_ch2_cold_ink_trail"))
        {
            player.setStoryProgress(2, 10, std::max(5, player.getStoryCityDevelopmentLevel()));
        }

        if (player.getStoryStep() >= 10)
        {
            addNonRefusableQuestIfMissing(player, createChapterTwoRouteRewriteQuest());
        }

        if (player.getStoryStep() <= 10 && questIsTurnedInInLog(player, "story_ch2_route_rewrite"))
        {
            player.setStoryProgress(2, 11, std::max(5, player.getStoryCityDevelopmentLevel()));
        }

        if (player.getStoryStep() >= 11)
        {
            addNonRefusableQuestIfMissing(player, createChapterTwoShortRouteCounterQuest());
        }

        if (player.getStoryStep() <= 11 && questIsTurnedInInLog(player, "story_ch2_short_route_counter"))
        {
            player.setStoryProgress(2, 12, std::max(6, player.getStoryCityDevelopmentLevel()));
        }

        if (player.getStoryStep() >= 12)
        {
            addNonRefusableQuestIfMissing(player, createChapterTwoBlackKnotWarningQuest());
        }

        if (player.getStoryStep() <= 12 && questIsTurnedInInLog(player, "story_ch2_black_knot_warning"))
        {
            player.setStoryProgress(2, 13, std::max(6, player.getStoryCityDevelopmentLevel()));
        }

        if (player.getStoryStep() >= 13)
        {
            addNonRefusableQuestIfMissing(player, createChapterTwoRepairDowntimeQuest());
        }

        if (player.getStoryStep() <= 13 && questIsTurnedInInLog(player, "story_ch2_repair_downtime"))
        {
            player.setStoryProgress(2, 14, std::max(6, player.getStoryCityDevelopmentLevel()));
        }

        if (player.getStoryStep() >= 14)
        {
            addNonRefusableQuestIfMissing(player, createChapterTwoHiddenGuardianHintQuest());
        }

        if (player.getStoryStep() <= 14 && questIsTurnedInInLog(player, "story_ch2_hidden_guardian_hint"))
        {
            player.setStoryProgress(2, 15, std::max(7, player.getStoryCityDevelopmentLevel()));
        }

        if (player.getStoryStep() >= 15)
        {
            addNonRefusableQuestIfMissing(player, createChapterTwoBlackKnotSealQuest());
        }

        if (player.getStoryStep() <= 15 && questIsTurnedInInLog(player, "story_ch2_black_knot_seal"))
        {
            player.setStoryProgress(2, 16, std::max(8, player.getStoryCityDevelopmentLevel()));
        }

        if (player.getStoryStep() >= 16)
        {
            addNonRefusableQuestIfMissing(player, createChapterTwoBlackKnotScarsQuest());
        }

        if (player.getStoryStep() <= 16 && questIsTurnedInInLog(player, "story_ch2_black_knot_scars"))
        {
            player.setStoryProgress(2, 17, std::max(8, player.getStoryCityDevelopmentLevel()));
        }

        if (player.getStoryStep() >= 17)
        {
            addNonRefusableQuestIfMissing(player, createChapterTwoGuardedRouteQuest());
        }

        if (player.getStoryStep() <= 17 && questIsTurnedInInLog(player, "story_ch2_guarded_route"))
        {
            player.setStoryProgress(2, 18, std::max(9, player.getStoryCityDevelopmentLevel()));
        }
        return;
    }

    if (player.getStoryChapter() == 3)
    {
        addNonRefusableQuestIfMissing(player, createChapterThreeLonelyConvoyQuest());

        const std::vector<std::pair<std::string, Quest(*)()>> chain = {
            {"story_ch3_lonely_convoy", createChapterThreeThreeRoutesQuest},
            {"story_ch3_three_routes", createChapterThreeSignaturesQuest},
            {"story_ch3_signatures", createChapterThreeEscortWithdrawalQuest},
            {"story_ch3_escort_withdrawal", createChapterThreeMarginVillageQuest},
            {"story_ch3_margin_village", createChapterThreeCorrectedRouteQuest},
            {"story_ch3_corrected_route", createChapterThreeMapGuardianQuest},
            {"story_ch3_map_guardian", createChapterThreeConvoyReturnQuest}
        };

        int completedSteps = 0;
        for (std::size_t index = 0; index < chain.size(); ++index)
        {
            if (!questIsTurnedInInLog(player, chain[index].first)) break;
            ++completedSteps;
            addNonRefusableQuestIfMissing(player, chain[index].second());
        }

        const int expectedStep = std::min(9, completedSteps + 1);
        if (player.getStoryStep() < expectedStep)
        {
            player.setStoryProgress(3, expectedStep, std::max(9, player.getStoryCityDevelopmentLevel()));
        }

        if (questIsTurnedInInLog(player, "story_ch3_convoy_return") && player.getStoryStep() < 9)
        {
            player.setStoryProgress(3, 9, std::max(11, player.getStoryCityDevelopmentLevel()));
        }
        player.getQuestLog().refreshLinkedQuestProgress();
    }
}

void QuestMenu::openMainQuestSection(Player& player)
{
    while (true)
    {
        syncMainStoryQuests(player);

        MenuScreen screen("QUÊTE PRINCIPALE", "quest.main_story");
        screen.addSubtitle("Objectifs d'histoire non refusables");
        screen.addLine("Les quêtes principales ne sont pas des contrats à accepter ou refuser : elles suivent la route de l'histoire.");

        if (!player.hasStoryModeStarted())
        {
            screen.addLine("Aucune histoire active pour ce personnage.");
        }
        else if (player.getStoryChapter() == 1)
        {
            syncChapterOneLinkedQuestProgress(player);
            const int metReferents = countKnownChapterOneReferentQuests(player);
            const int referentQuestsDone = countTurnedInChapterOneReferentQuests(player);
            const int mainDone = countTurnedInChapterOneMainRequests(player);
            const bool meetQuestDone = questIsTurnedInInLog(player, "story_ch1_meet_referents");
            const bool meetQuestReady = questIsCompletedInLog(player, "story_ch1_meet_referents");
            const bool miraBundleKnown = questExistsInAnyState(player, "story_ch1_mira_main");
            const bool miraBundleDone = questIsTurnedInInLog(player, "story_ch1_mira_main");

            screen.addLine("Chapitre actuel : 1 — La ville qui tient à peine.");
            screen.addLine("Progression : " + player.getStoryProgressLabel());
            screen.addLine("Lecture : Mira ouvre seule le chapitre, puis les quatre référents peuvent être rencontrés et aidés dans n'importe quel ordre.");

            int lineNumber = 1;
            addGuidedStoryLine(
                screen,
                lineNumber++,
                "Rencontrer Mira",
                storyMilestoneMarker(player.getStoryStep() >= 3, player.getStoryStep() < 3),
                player.getStoryStep() >= 3 ? "[fait] première étape validée" : "continuer le chapitre 1 depuis le menu histoire"
            );

            if (player.getStoryStep() >= 3)
            {
                addGuidedStoryLine(
                    screen,
                    lineNumber++,
                    "Rencontrer Orren, Lysa, Bram et Soryn",
                    storyQuestMarkerForId(player, "story_ch1_meet_referents"),
                    std::to_string(metReferents) + "/4 rencontré(s). Chaque discussion ajoute une quête principale indépendante."
                );

                for (const std::string& clientName : chapterOneReferentNames())
                {
                    const std::string mainQuestId = storyMainQuestIdForClient(clientName);
                    if (!questExistsInAnyState(player, mainQuestId))
                    {
                        addGuidedStoryLine(
                            screen,
                            lineNumber++,
                            clientName,
                            "[à rencontrer]",
                            "PNJ notables > PNJ d'histoire : parle-lui pour recevoir sa quête principale."
                        );
                    }
                    else
                    {
                        addGuidedStoryLine(
                            screen,
                            lineNumber++,
                            clientName + " — " + [&]() {
                                for (const Quest& quest : player.getQuestLog().getQuests())
                                {
                                    if (quest.id == mainQuestId) return quest.title;
                                }
                                return std::string("Quête principale");
                            }(),
                            storyQuestMarkerForId(player, mainQuestId),
                            storyQuestStatusForId(player, mainQuestId) + ". La quête peut avancer avant même que les trois autres référents soient rencontrés."
                        );
                    }
                }

                if (meetQuestReady && !meetQuestDone)
                {
                    addGuidedStoryLine(
                        screen,
                        lineNumber++,
                        "Prévenir Mira après les quatre rencontres",
                        "[étape actuelle]",
                        "les quatre quêtes existent désormais ; retourne parler à Mira pour ouvrir le bilan des quatre"
                    );
                }
                else if (meetQuestDone)
                {
                    addGuidedStoryLine(
                        screen,
                        lineNumber++,
                        "Prévenir Mira après les quatre rencontres",
                        "[fait]",
                        "Mira a ouvert le bilan principal des quatre référents"
                    );
                }
            }

            if (miraBundleKnown)
            {
                addGuidedStoryLine(
                    screen,
                    lineNumber++,
                    "Mira — Faire respirer les murs",
                    storyQuestMarkerForId(player, "story_ch1_mira_main"),
                    std::to_string(referentQuestsDone) + "/4 quête(s) de référent rendue(s). Les validations obtenues avant l'ouverture de ce bilan sont déjà comptées."
                );
            }
            else if (player.getStoryStep() >= 3)
            {
                addGuidedStoryLine(
                    screen,
                    lineNumber++,
                    "Bilan des quatre référents",
                    "[verrouillé]",
                    "rencontre d'abord les quatre PNJ, puis retourne prévenir Mira"
                );
            }

            screen.addLine("Bilan : référents rencontrés " + std::to_string(metReferents) + "/4 | quêtes des référents rendues " + std::to_string(referentQuestsDone) + "/4 | principales validées " + std::to_string(mainDone) + "/5.");
            if (miraBundleDone && referentQuestsDone >= 4)
            {
                screen.addLine("[fait] La chaîne principale du chapitre est terminée. Retourne voir Mira depuis le menu histoire pour ouvrir la suite.");
            }
            else if (meetQuestReady && !meetQuestDone)
            {
                screen.addLine("Suite : retourne parler à Mira. Elle détectera aussi les quêtes déjà terminées avant ce retour.");
            }
            else if (miraBundleKnown)
            {
                screen.addLine("Suite : termine ou rends les quêtes manquantes auprès de leur propriétaire, puis rends le bilan final à Mira.");
            }
            else
            {
                screen.addLine("Suite : parle aux quatre référents depuis PNJ notables > PNJ d'histoire.");
            }
        }
        else if (player.getStoryChapter() == 2)
        {
            screen.addLine("Chapitre actuel : 2 — Le relais silencieux.");
            screen.addLine("Progression : " + player.getStoryProgressLabel());
            screen.addLine("Lecture : les étapes validées restent marquées [fait], l'étape actuelle reste lisible, la suite reste masquée.");

            bool currentShown = false;
            int hiddenSteps = 0;
            for (const StoryStepDescriptor& step : chapterTwoStoryStepDescriptors())
            {
                const bool exists = questExistsInAnyState(player, step.id);
                const bool done = questIsTurnedInInLog(player, step.id);
                const bool readyToNotify = questIsCompletedInLog(player, step.id);
                const bool active = questIsActiveInLog(player, step.id);

                if (done)
                {
                    addQuestGuidedStoryLine(screen, player, step);
                    continue;
                }

                if ((exists || readyToNotify || active) && !currentShown)
                {
                    addQuestGuidedStoryLine(screen, player, step);
                    currentShown = true;
                    continue;
                }

                if ((exists || readyToNotify || active) && currentShown)
                {
                    ++hiddenSteps;
                    continue;
                }

                ++hiddenSteps;
            }

            if (hiddenSteps > 0)
            {
                screen.addLine("Étapes suivantes : " + std::to_string(hiddenSteps) + " étape(s) masquée(s) jusqu'à validation de l'étape actuelle.");
            }

            screen.addLine("Bilan chapitre 2 actuel : " + std::to_string(countTurnedInChapterTwoRequests(player)) + "/17 étape(s) principales validées.");
            if (player.getStoryStep() >= 18)
            {
                screen.addLine("État publication : boucle chapitre 2 actuelle complète. La suite sera écrite avant d’être codée.");
            }

            std::string nextMainQuestLine;
            if (player.getStoryStep() >= 18)
            {
                nextMainQuestLine = "Suite : la route gardée tient. Le vrai nom de la menace reste à obtenir plus tard.";
            }
            else if (player.getStoryStep() >= 17)
            {
                nextMainQuestLine = "Suite : organise les premiers retours gardés avec Mira, Eda, Nell, Bram et Lysa.";
            }
            else if (player.getStoryStep() >= 16)
            {
                nextMainQuestLine = "Suite : lis les cicatrices du verrou avec Soryn, puis rends la preuve.";
            }
            else if (player.getStoryStep() >= 15)
            {
                nextMainQuestLine = "Suite : affronte le verrou de la borne noire, puis rends la preuve auprès d'Orren.";
            }
            else if (player.getStoryStep() >= 14)
            {
                nextMainQuestLine = "Suite : identifie la chose qui garde la borne avec Soryn, Nell et Orren.";
            }
            else if (player.getStoryStep() >= 13)
            {
                nextMainQuestLine = "Suite : occupe-toi utilement pendant les réparations, puis rends le bilan auprès d'Eda.";
            }
            else if (player.getStoryStep() >= 12)
            {
                nextMainQuestLine = "Suite : reconnais le nœud noir, puis rends l'alerte auprès d'Orren.";
            }
            else if (player.getStoryStep() >= 11)
            {
                nextMainQuestLine = "Suite : installe le contre-registre des routes courtes, puis notifie Mira.";
            }
            else if (player.getStoryStep() >= 10)
            {
                nextMainQuestLine = "Suite : identifie ce qui réécrit la route avec Soryn et Nell.";
            }
            else if (player.getStoryStep() >= 9)
            {
                nextMainQuestLine = "Suite : retourne sur la Route commerciale suivre l'encre froide, puis rends la preuve auprès de Soryn.";
            }
            else if (player.getStoryStep() >= 8)
            {
                nextMainQuestLine = "Suite : répartis les informations de Nell entre les comptoirs, puis notifie Mira.";
            }
            else if (player.getStoryStep() >= 7)
            {
                nextMainQuestLine = "Suite : exploite la sacoche de Nell, puis rends l'analyse auprès d'elle.";
            }
            else if (player.getStoryStep() >= 6)
            {
                nextMainQuestLine = "Suite : suis le premier appel du relais, puis rends le sauvetage auprès de Nell la messagère.";
            }
            else
            {
                nextMainQuestLine = "Suite : utilise le menu histoire, la Route commerciale, PNJ notables et le rendu auprès des bons contacts.";
            }
            screen.addLine(nextMainQuestLine);
        }
        else if (player.getStoryChapter() == 3)
        {
            screen.addLine("Chapitre actuel : 3 — Les routes qui répondent mal.");
            screen.addLine("Progression : " + player.getStoryProgressLabel());
            screen.addLine("Lecture : les étapes validées restent [fait], une seule étape actuelle est détaillée et les futures restent masquées.");

            bool currentShown = false;
            int hiddenSteps = 0;
            for (const StoryStepDescriptor& step : chapterThreeStoryStepDescriptors())
            {
                const bool exists = questExistsInAnyState(player, step.id);
                const bool done = questIsTurnedInInLog(player, step.id);
                const bool readyToNotify = questIsCompletedInLog(player, step.id);
                const bool active = questIsActiveInLog(player, step.id);
                if (done)
                {
                    addQuestGuidedStoryLine(screen, player, step);
                    continue;
                }
                if ((exists || readyToNotify || active) && !currentShown)
                {
                    addQuestGuidedStoryLine(screen, player, step);
                    currentShown = true;
                    continue;
                }
                ++hiddenSteps;
            }
            if (hiddenSteps > 0)
            {
                screen.addLine("Étapes suivantes : " + std::to_string(hiddenSteps) + " étape(s) masquée(s) jusqu'à validation de l'étape actuelle.");
            }
            screen.addLine("Bilan chapitre 3 : " + std::to_string(countTurnedInChapterThreeRequests(player)) + "/8 quête(s) principales rendue(s).");
            screen.addLine(player.getStoryStep() >= 9
                ? "[fait] Le convoi est classé et le village absent des cartes devient la prochaine direction connue."
                : "Suite : accomplis l'étape visible, puis rends-la auprès du PNJ indiqué pour dévoiler la suivante.");
        }
        else
        {
            screen.addLine("Le chapitre actuel n'a pas encore de tableau détaillé ici.");
            screen.addLine("Le journal complet reste disponible pour les demandes et contrats actifs.");
        }

        screen.addBackOption("Retour", "quest.main_story.back");
        screen.addOption(1, "Aller aux PNJ notables", "Parler aux référents et autres PNJ du même monde.", true, "quest.main_story.npcs");
        screen.addOption(2, "Consulter le journal complet", "Voir toutes les quêtes connues, principales ou non.", true, "quest.main_story.journal");

        int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choix invalide.");
        Console::clear();

        if (choice == 0)
        {
            return;
        }
        if (choice == 1)
        {
            openNotableNpcMenu(player);
        }
        else if (choice == 2)
        {
            displayQuestJournal(player);
        }
    }
}

void QuestMenu::openCompletedMainQuestSection(const Player& player)
{
    constexpr std::size_t questsPerPage = 5;
    std::size_t pageIndex = 0;

    while (true)
    {
        std::vector<const Quest*> completedMainQuests;
        for (const Quest& quest : player.getQuestLog().getQuests())
        {
            if (quest.turnedIn && !quest.failed && isMainStoryQuest(quest))
            {
                completedMainQuests.push_back(&quest);
            }
        }

        std::stable_sort(completedMainQuests.begin(), completedMainQuests.end(), [](const Quest* left, const Quest* right) {
            if (left->id != right->id)
            {
                return left->id < right->id;
            }
            return left->title < right->title;
        });

        const std::size_t totalPages = PagedMenu::pageCount(completedMainQuests.size(), questsPerPage);
        if (pageIndex >= totalPages)
        {
            pageIndex = totalPages == 0 ? 0 : totalPages - 1;
        }

        const std::size_t first = PagedMenu::firstIndex(pageIndex, questsPerPage);
        const std::size_t last = PagedMenu::lastIndexExclusive(completedMainQuests.size(), pageIndex, questsPerPage);

        MenuScreen screen("PRINCIPALES FINIES", "quest.main_story.completed");
        screen.setPagination(pageIndex, totalPages);
        screen.addSubtitle("Archive des étapes d'histoire validées");
        screen.addLine("Les quêtes principales terminées quittent la route active, mais restent consultables ici avec leur lieu, leur contact et leurs récompenses.");

        const int chapterOneDone = countTurnedInChapterOneMainRequests(player);
        const int chapterTwoDone = countTurnedInChapterTwoRequests(player);
        const int chapterThreeDone = countTurnedInChapterThreeRequests(player);
        screen.addLine("Chapitre 1 — La ville qui tient à peine : " + std::string(chapterOneDone >= 5 ? "[fait]" : "[en cours]") + " " + std::to_string(chapterOneDone) + "/5.");
        if (chapterTwoDone > 0 || player.getStoryChapter() >= 2)
        {
            screen.addLine("Chapitre 2 — Le relais silencieux : " + std::string(chapterTwoDone >= 17 ? "[fait]" : "[en cours]") + " " + std::to_string(chapterTwoDone) + "/17.");
        }
        if (chapterThreeDone > 0 || player.getStoryChapter() >= 3)
        {
            screen.addLine("Chapitre 3 — Les routes qui répondent mal : " + std::string(chapterThreeDone >= 8 ? "[fait]" : "[en cours]") + " " + std::to_string(chapterThreeDone) + "/8.");
        }
        screen.addLine("Affichage : " + PagedMenu::rangeText(first, last, completedMainQuests.size()));
        screen.addBackOption("Retour", "quest.main_story.completed.back");

        if (completedMainQuests.empty())
        {
            screen.addLine("Aucune quête principale n'a encore été rendue.");
        }
        else
        {
            for (std::size_t i = first; i < last; ++i)
            {
                const Quest& quest = *completedMainQuests[i];
                MenuOptionItemData itemData;
                itemData.structured = true;
                itemData.kind = "quest";
                itemData.section = "Principales finies";
                itemData.actionType = "inspect";
                itemData.name = quest.title;
                itemData.detail = "Lieu : " + questPlayableLocationHint(quest);
                itemData.status = "[fait] Validée auprès de " + quest.client;
                itemData.reward = questPotentialRewardText(quest);
                itemData.progress = std::to_string(quest.target) + "/" + std::to_string(quest.target);
                itemData.owner = quest.client;
                itemData.important = true;

                screen.addOption(
                    static_cast<int>(10 + (i - first)),
                    questCardLabel(quest),
                    "Inspecter cette étape principale terminée.",
                    true,
                    "quest.main_story.completed.inspect." + std::to_string(i),
                    itemData
                );
            }
        }

        PagedMenu::addNavigationOptions(screen, pageIndex, totalPages);
        const int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choix invalide.");
        Console::clear();

        if (choice == 0)
        {
            return;
        }
        if (choice == 98 && pageIndex > 0)
        {
            --pageIndex;
            continue;
        }
        if (choice == 99 && pageIndex + 1 < totalPages)
        {
            ++pageIndex;
            continue;
        }

        const int localIndex = choice - 10;
        if (localIndex >= 0 && first + static_cast<std::size_t>(localIndex) < last)
        {
            showQuestDetail(player, *completedMainQuests[first + static_cast<std::size_t>(localIndex)]);
        }
    }
}

// EN: openQuestHub declares or implements a focused behavior used by this module.
// FR: openQuestHub déclare ou implémente un comportement précis utilisé par ce module.
void QuestMenu::openQuestHub(Player& player)
{
    expireOverdueQuestDeadlines(player, "quest.hub");
    while (true)
    {
        syncMainStoryQuests(player);
        const int readyCount = countQuestsForJournalFilter(player, QuestJournalFilter::ReadyToTurnIn);
        MenuScreen screen("QUÊTES", "quest.hub");
        screen.addLine("Date actuelle : " + player.formatWorldDateLine());
        screen.addLine("Moment actuel : " + player.formatWorldDayPartLine());
        screen.addLine("Les quêtes progressent en combattant, explorant, récupérant des ressources ou battant les bonnes cibles.");
        screen.addLine("Quêtes principales : section séparée, non refusable quand l'histoire les ajoute.");
        screen.addLine("Quêtes de guilde actives : " + std::to_string(player.getQuestLog().getActiveGuildQuestCount()) + "/" + std::to_string(guildActiveQuestLimitForStanding(guildStandingForPlayer(player))) + ".");
        screen.addLine("Demandes prêtes à rendre : " + std::to_string(readyCount) + ".");
        screen.addBackOption("Retour", "quest.hub.back");

        MenuOptionItemData mainQuestData = makeQuestNavigationItemData(
            "quest",
            "Hub",
            "inspect",
            "Quête principale",
            "Voir les objectifs d'histoire non refusables et la prochaine étape."
        );
        mainQuestData.status = player.hasStoryModeStarted() ? "Histoire active" : "Aucune histoire active";
        mainQuestData.important = player.hasStoryModeStarted();

        MenuOptionItemData readyData = makeQuestNavigationItemData(
            "quest",
            "Hub",
            "turn_in",
            "Quêtes à rendre / terminées",
            readyCount > 0 ? "Priorité : choisir le bon contact pour valider une quête terminée." : "Aucune quête prête, mais le journal permet de relire les archives."
        );
        readyData.status = readyCount > 0 ? std::to_string(readyCount) + " prête(s)" : "Aucune prête";
        readyData.important = readyCount > 0;

        MenuOptionItemData journalData = makeQuestNavigationItemData(
            "quest",
            "Hub",
            "inspect",
            "Journal complet",
            "Consulter les contrats officiels, demandes informelles, filtres et archives."
        );
        journalData.status = "Consultation";

        MenuOptionItemData guildData = makeQuestNavigationItemData(
            "npc",
            "Hub",
            "quest",
            "Guilde",
            "Panneau officiel, contrats et remise auprès du maître de guilde.",
            "Maître de guilde"
        );
        guildData.status = "Contrats officiels";

        MenuOptionItemData completedMainData = makeQuestNavigationItemData(
            "quest",
            "Hub",
            "inspect",
            "Principales finies",
            "Relire séparément les étapes d'histoire déjà validées."
        );
        completedMainData.status = std::to_string(countQuestsForJournalFilter(player, QuestJournalFilter::MainTurnedIn)) + " terminée(s)";
        completedMainData.important = countQuestsForJournalFilter(player, QuestJournalFilter::MainTurnedIn) > 0;

        screen.addOption(1, "Quête principale", "Voir ce que l'histoire demande réellement, sans acceptation/refus.", true, "quest.hub.main_story", mainQuestData);
        screen.addOption(2, "Principales finies", "Consulter l'archive dédiée des quêtes principales déjà validées.", true, "quest.hub.main_story_completed", completedMainData);
        screen.addOption(3, "Quêtes à rendre / terminées" + (readyCount > 0 ? " [" + std::to_string(readyCount) + "]" : ""),
            readyCount > 0 ? "Priorité aux quêtes validables maintenant." : "Aucune quête prête : ouvre le journal pour relire les terminées.",
            true,
            "quest.hub.ready_or_done",
            readyData
        );
        screen.addOption(4, "Journal complet", "Voir les quêtes, filtres, estimations et archives.", true, "quest.hub.journal", journalData);
        screen.addOption(5, "Aller à la guilde", "Consulter le panneau officiel ou rendre un contrat de guilde.", true, "quest.hub.guild", guildData);

        int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choix invalide.");
        Console::clear();

        if (choice == 0)
        {
            return;
        }

        if (choice == 1)
        {
            openMainQuestSection(player);
        }
        else if (choice == 2)
        {
            openCompletedMainQuestSection(player);
        }
        else if (choice == 3)
        {
            if (readyCount > 0)
            {
                openReadyQuestTurnInMenu(player);
            }
            else
            {
                MessageScreen::show(
                    "QUÊTES À RENDRE / TERMINÉES",
                    "quest.hub.ready_or_done.empty",
                    {"Aucune quête n'est prête à rendre pour l'instant.", "Le journal complet permet de consulter les quêtes actives, prêtes, rendues et archivées."}
                );
                displayQuestJournal(player);
            }
        }
        else if (choice == 4)
        {
            displayQuestJournal(player);
        }
        else if (choice == 5)
        {
            openGuild(player);
        }
    }
}

// EN: consultOnly declares or implements a focused behavior used by this module.
// FR: consultOnly déclare ou implémente un comportement précis utilisé par ce module.
void QuestMenu::consultOnly(const Player& player)
{
    MessageScreen::show(
        "CONSULTATION SEULE",
        "quest.consult_only",
        {
            "Depuis ce menu, tu peux seulement consulter.",
            "Pour accepter ou valider une quête, retourne voir la guilde ou le client."
        },
        false
    );
    displayQuestJournal(player);
}

// EN: displayQuestJournal declares or implements a focused behavior used by this module.
// FR: displayQuestJournal déclare ou implémente un comportement précis utilisé par ce module.
void QuestMenu::displayQuestJournal(const Player& player)
{
    constexpr std::size_t questsPerPage = 5;
    QuestJournalFilter activeFilter = QuestJournalFilter::Active;
    std::size_t pageIndex = 0;

    while (true)
    {
        const std::vector<Quest>& quests = player.getQuestLog().getQuests();
        std::vector<const Quest*> displayedQuests = collectQuestsForJournalFilter(player, activeFilter);
        const std::size_t totalPages = PagedMenu::pageCount(displayedQuests.size(), questsPerPage);

        if (pageIndex >= totalPages)
        {
            pageIndex = totalPages == 0 ? 0 : totalPages - 1;
        }

        const std::size_t first = PagedMenu::firstIndex(pageIndex, questsPerPage);
        const std::size_t last = PagedMenu::lastIndexExclusive(displayedQuests.size(), pageIndex, questsPerPage);

        MenuScreen screen("JOURNAL DE QUÊTES", "quest.journal");
        screen.addSubtitle(questJournalFilterTitle(activeFilter));
        screen.addLine("Filtre actif : " + questJournalFilterTitle(activeFilter));
        screen.addLine(questJournalFilterHint(activeFilter));
        screen.addLine("Quêtes de guilde actives : " + std::to_string(player.getQuestLog().getActiveGuildQuestCount()) + "/" + std::to_string(guildActiveQuestLimitForStanding(guildStandingForPlayer(player))));
        screen.addLine("Demandes prêtes à rendre : " + std::to_string(countQuestsForJournalFilter(player, QuestJournalFilter::ReadyToTurnIn)));
        if (countQuestsForJournalFilter(player, QuestJournalFilter::ReadyToTurnIn) > 0)
        {
            screen.addLine("Astuce : passe par le hub des quêtes pour choisir directement le bon contact de validation.");
        }
        screen.addLine("Affichage : " + PagedMenu::rangeText(first, last, displayedQuests.size()));

        if (quests.empty())
        {
            screen.addLine("Aucune quête acceptée pour l'instant.");
            screen.addLine("La guilde propose des contrats officiels ; certains PNJ peuvent seulement demander un service de vive voix.");
            screen.addBackOption("Retour", "quest.journal.back");
            TerminalInterface::askMenuChoiceFromOptions(screen, "Entre 0 pour revenir.");
            Console::clear();
            return;
        }

        if (displayedQuests.empty())
        {
            screen.addLine("Aucune entrée dans ce filtre.");
        }
        else
        {
            for (std::size_t i = first; i < last; ++i)
            {
                const Quest& quest = *displayedQuests[i];
                const std::string questLabel = questCardLabel(quest);
                const int localNumber = static_cast<int>(10 + (i - first));
                MenuOptionItemData itemData;
                itemData.structured = true;
                itemData.kind = "quest";
                itemData.section = questJournalFilterTitle(activeFilter);
                itemData.actionType = quest.guildQuest ? "inspect_contract" : "estimate_request";
                itemData.name = quest.title;
                itemData.detail = "";
                itemData.status = isReadyToTurnIn(player, quest)
                    ? (quest.guildQuest ? "Prête à rendre - contrat officiel" : "Prête à confirmer - demande PNJ")
                    : (quest.guildQuest ? questStateText(quest) : questStateText(quest) + " - informations estimées");
                const std::string statusDeadline = activeQuestDeadlineStatusText(quest, player.getWorldDaysElapsed());
                if (!statusDeadline.empty())
                {
                    itemData.status += " | " + statusDeadline;
                }
                itemData.reward = quest.guildQuest ? questRewardText(quest) : approximateQuestRewardText(quest);
                itemData.progress = std::to_string(quest.progress) + "/" + std::to_string(quest.target);
                itemData.owner = quest.client;
                itemData.important = isReadyToTurnIn(player, quest) || !quest.guildQuest;

                screen.addOption(
                    localNumber,
                    questLabel,
                    "",
                    true,
                    quest.guildQuest
                        ? "quest.journal.inspect.guild." + std::to_string(i)
                        : "quest.journal.estimate.personal." + std::to_string(i),
                    itemData
                );
            }
        }

        screen.addOption(1, "Filtre : actives", "Tout ce qui n'est pas encore rendu.", true, "quest.journal.filter.active");
        screen.addOption(2, "Filtre : prêtes à rendre", "Quêtes terminées ou livraisons possibles.", true, "quest.journal.filter.ready");
        screen.addOption(3, "Filtre : guilde", "Contrats officiels inspectables proprement.", true, "quest.journal.filter.guild");
        screen.addOption(4, "Filtre : demandes PNJ", "Demandes informelles avec infos vagues/estimées.", true, "quest.journal.filter.personal");
        screen.addOption(5, "Filtre : combat", "Contrats ou demandes qui progressent par combat.", true, "quest.journal.filter.combat");
        screen.addOption(6, "Filtre : exploration / bestiaire", "Notes de terrain, traces et observations.", true, "quest.journal.filter.exploration");
        screen.addOption(7, "Filtre : livraison", "Matériaux ou objets à rapporter.", true, "quest.journal.filter.delivery");
        screen.addOption(8, "Filtre : principales finies", "Archive séparée des étapes principales déjà validées.", true, "quest.journal.filter.main_turned_in");
        screen.addOption(9, "Filtre : rendues", "Archives des quêtes secondaires, contrats et demandes déjà validés.", true, "quest.journal.filter.turned_in");
        PagedMenu::addNavigationOptions(screen, pageIndex, totalPages);

        int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choix invalide.");
        Console::clear();

        if (choice == 0)
        {
            return;
        }

        if (choice >= 1 && choice <= 9)
        {
            switch (choice)
            {
                case 1: activeFilter = QuestJournalFilter::Active; break;
                case 2: activeFilter = QuestJournalFilter::ReadyToTurnIn; break;
                case 3: activeFilter = QuestJournalFilter::Guild; break;
                case 4: activeFilter = QuestJournalFilter::Personal; break;
                case 5: activeFilter = QuestJournalFilter::Combat; break;
                case 6: activeFilter = QuestJournalFilter::Exploration; break;
                case 7: activeFilter = QuestJournalFilter::Delivery; break;
                case 8: activeFilter = QuestJournalFilter::MainTurnedIn; break;
                case 9: activeFilter = QuestJournalFilter::TurnedIn; break;
                default: break;
            }
            pageIndex = 0;
            continue;
        }

        if (choice == 98 && pageIndex > 0)
        {
            --pageIndex;
            continue;
        }

        if (choice == 99 && pageIndex + 1 < totalPages)
        {
            ++pageIndex;
            continue;
        }

        const int localQuestIndex = choice - 10;
        if (localQuestIndex >= 0 && first + static_cast<std::size_t>(localQuestIndex) < last)
        {
            openAcceptedQuestActions(
                player,
                *displayedQuests[first + static_cast<std::size_t>(localQuestIndex)],
                "quest.journal.selected"
            );
            continue;
        }

        MessageScreen::show(
            "ACTION INDISPONIBLE",
            "quest.journal.invalid",
            {"Ce choix ne correspond à aucune action du journal."}
        );
    }
}


// EN: openGuildRegistration runs the first adventurer inscription exam.
// FR: openGuildRegistration lance la première inscription d'aventurier.
void QuestMenu::openGuildRegistration(Player& player)
{
    if (player.hasTitle("Aventurier") && !player.isRegisteredAtCurrentCityGuild())
    {
        MessageScreen::show(
            "MISE À NIVEAU DE GUILDE — " + currentCityName(player),
            "quest.guild.registration.local_upgrade.intro",
            {
                "Ton inscription d'aventurier reste valable : tu ne recommences pas le grand QCM.",
                "La guilde locale vérifie seulement trois règles propres au nouveau comptoir.",
                "Deux bonnes réponses suffisent pour enregistrer ta carte magique dans cette ville."
            },
            false
        );

        struct LocalQuestion
        {
            std::string text;
            std::vector<std::string> options;
            int correct;
        };
        const std::vector<LocalQuestion> localQuestions = {
            {"Où rends-tu une quête acceptée dans cette ville ?", {"Dans n'importe quelle guilde.", "Au comptoir de la guilde qui a enregistré le contrat.", "Chez le marchand le plus proche."}, 2},
            {"L'inscription nationale suffit-elle à connaître les stocks et dangers locaux ?", {"Non, la mise à niveau locale sert justement à cela.", "Oui, toutes les villes sont identiques.", "Oui, si le personnage est haut niveau."}, 1},
            {"Une carte déjà valide doit-elle être recréée ?", {"Oui, avec les seize questions.", "Non, elle est seulement enregistrée et mise à niveau localement.", "Oui, mais sans conserver le rang."}, 2}
        };

        int correct = 0;
        for (std::size_t i = 0; i < localQuestions.size(); ++i)
        {
            MenuScreen questionScreen(
                "MISE À NIVEAU LOCALE " + std::to_string(i + 1) + "/" + std::to_string(localQuestions.size()),
                "quest.guild.registration.local_upgrade.question." + std::to_string(i + 1)
            );
            questionScreen.addLine(localQuestions[i].text);
            for (std::size_t option = 0; option < localQuestions[i].options.size(); ++option)
            {
                questionScreen.addOption(
                    static_cast<int>(option + 1),
                    localQuestions[i].options[option],
                    "Réponse locale.",
                    true,
                    "quest.guild.registration.local_upgrade.answer." + std::to_string(i + 1) + "." + std::to_string(option + 1)
                );
            }
            const int choice = TerminalInterface::askMenuChoiceFromOptions(questionScreen, "Choix invalide.");
            Console::clear();
            if (choice == localQuestions[i].correct) ++correct;
        }

        if (correct >= 2 && player.registerAtCurrentCityGuild())
        {
            MessageScreen::show(
                "MISE À NIVEAU VALIDÉE",
                "quest.guild.registration.local_upgrade.success",
                {
                    "Résultat : " + std::to_string(correct) + "/3.",
                    "Carte enregistrée auprès de la guilde de " + currentCityName(player) + ".",
                    "Les quêtes locales devront être rendues à ce même comptoir."
                },
                false
            );
        }
        else
        {
            MessageScreen::show(
                "MISE À NIVEAU REFUSÉE",
                "quest.guild.registration.local_upgrade.failed",
                {
                    "Résultat : " + std::to_string(correct) + "/3.",
                    "Ton titre Aventurier reste valide, mais ce comptoir local n'est pas encore enregistré.",
                    "Tu pourras refaire ce contrôle court."
                },
                false
            );
        }
        return;
    }

    if (player.hasTitle("Aventurier"))
    {
        std::vector<std::string> lines = {
            "Tu possèdes déjà le titre Aventurier.",
            "Guilde locale enregistrée : " + currentCityName(player) + ".",
            "Carte magique : " + std::to_string(player.getInventory().countMaterialById("guild_card")) + " exemplaire(s) dans l'inventaire."
        };
        std::vector<std::string> standingLines = guildStandingSummaryLines(player);
        lines.insert(lines.end(), standingLines.begin(), standingLines.end());
        lines.push_back("La carte magique suit maintenant le rang, la pastille et les contrats officiels validés.");

        MessageScreen::show(
            "DÉJÀ INSCRIT",
            "quest.guild.registration.already",
            lines,
            false
        );
        return;
    }

    MessageScreen::show(
        "INSCRIPTION À LA GUILDE",
        "quest.guild.registration.intro",
        {
            "La gérante sort une fiche propre, un cristal d'enregistrement et une carte magique encore vierge.",
            "Elle précise que l'inscription ne donne pas le droit de jouer au héros : elle donne surtout le droit d'être responsable de ses contrats.",
            "L'épreuve actuelle reprend toutes les questions de connaissance validées depuis tes fiches : monstres, magie, plantes et règles de guilde.",
            "Chaque question est en QCM. Il faut au moins 12 bonnes réponses sur 16 pour obtenir le titre Aventurier."
        },
        false
    );

    struct KnowledgeQuestion
    {
        std::string question;
        std::vector<std::string> options;
        int correctChoice;
    };

    const std::vector<KnowledgeQuestion> questions = {
        {"Que fais-tu si tu rencontres un monstre plus fort que toi ?", {"Je fonce pour prouver mon courage.", "J'évalue, je me replie ou j'appelle de l'aide.", "Je le provoque pour voir sa réaction.", "Je jette ma carte de guilde."}, 2},
        {"Pourquoi ne faut-il pas tuer tous les monstres d'une zone ?", {"Parce que c'est trop long.", "Parce que certains sont utiles, protégés ou liés à l'équilibre local.", "Parce que la guilde n'aime pas les trophées.", "Parce que les monstres ne donnent jamais de récompense."}, 2},
        {"Si un monstre est territorial, que dois-tu faire ?", {"Entrer plus profondément dans son territoire.", "Ignorer les signes et courir.", "Identifier la limite, éviter l'escalade et signaler la menace.", "Dormir dans son nid."}, 3},
        {"Un sort inconnu est découvert dans une ruine. Que fais-tu ?", {"Je le lance immédiatement.", "Je le copie sans le lire.", "Je le vends comme parchemin commun.", "Je le sécurise, le fais identifier et évite l'essai sauvage."}, 4},
        {"Pourquoi certains sorts sont interdits ?", {"Parce qu'ils sont trop beaux.", "Parce qu'ils peuvent corrompre, contrôler, tuer ou briser des règles vitales.", "Parce que la bibliothèque veut vendre plus cher.", "Parce que seuls les nobles savent lire."}, 2},
        {"Que risques-tu si tu utilises une magie au-dessus de tes capacités ?", {"Un contrecoup, une perte de contrôle, une blessure ou une corruption.", "Rien si tu cries assez fort.", "Un simple malus de style.", "Tu deviens automatiquement maître mage."}, 1},
        {"Pourquoi faut-il identifier une plante avant de l'utiliser ?", {"Pour savoir si elle soigne, empoisonne ou réagit mal au mana.", "Pour lui donner un joli nom.", "Pour que le vendeur soit content.", "Parce que toutes les plantes valent pareil."}, 1},
        {"Que faire si une plante rare pousse dans une zone protégée ?", {"Tout arracher avant les autres.", "Brûler les mauvaises herbes autour.", "Prélever proprement, ou demander autorisation si nécessaire.", "La cacher dans son sac sans note."}, 3},
        {"Pourquoi certaines plantes ne doivent-elles pas être coupées n'importe comment ?", {"Parce qu'elles peuvent repousser, stabiliser une zone ou devenir dangereuses si elles sont abîmées.", "Parce qu'elles donnent moins d'XP.", "Parce que la faux est interdite.", "Parce que les plantes n'ont aucune utilité."}, 1},
        {"À quoi sert la guilde ?", {"À donner un cadre, classer les missions, protéger les clients et suivre les aventuriers.", "À distribuer des primes gratuites.", "À remplacer toutes les lois.", "À garantir que personne ne meurt jamais."}, 1},
        {"Pourquoi les quêtes ont-elles des rangs ?", {"Pour décorer le panneau.", "Pour limiter les missions selon le danger, l'expérience et le niveau de l'aventurier.", "Pour empêcher les débutants de gagner de l'argent.", "Pour rendre les titres plus longs."}, 2},
        {"Peux-tu prendre une quête beaucoup trop dangereuse pour toi ?", {"Oui, toujours.", "Oui, si le client insiste.", "Non : la guilde peut bloquer ou refuser l'accès.", "Oui, mais seulement si elle est brillante."}, 3},
        {"Que dois-tu faire si tu échoues une mission ?", {"Mentir dans le rapport.", "Disparaître du village.", "Signaler l'échec, les pertes, les raisons et ce qui peut encore être sauvé.", "Accuser le premier marchand."}, 3},
        {"Pourquoi la carte de guilde est-elle magique ?", {"Pour suivre l'inscription, l'identité, les contrats et plus tard les rangs/pastilles.", "Pour attaquer les clients qui paient mal.", "Pour transformer les quêtes en or.", "Pour empêcher le joueur de revenir en arrière."}, 1},
        {"À quoi sert une pastille verte sur un dossier d'aventurier ?", {"À prouver que l'aventurier est immortel.", "À indiquer une fiabilité correcte tant qu'aucune sanction grave n'est connue.", "À forcer les marchands à faire 90% de réduction.", "À remplacer le rang de guilde."}, 2},
        {"Si une quête a une date limite, quelle attitude est la plus logique ?", {"L'oublier et espérer que le client dorme longtemps.", "Prioriser la quête ou prévenir vite si elle devient impossible.", "Attendre le dernier jour pour rendre tous les contrats.", "Mentir dans le journal."}, 2}
    };

    int correctAnswers = 0;

    for (std::size_t i = 0; i < questions.size(); ++i)
    {
        const KnowledgeQuestion& question = questions[i];
        MenuScreen screen(
            "TEST DE CONNAISSANCES " + std::to_string(i + 1) + "/" + std::to_string(questions.size()),
            "quest.guild.registration.question." + std::to_string(i + 1)
        );
        screen.addLine(question.question);
        screen.addLine("Réponds comme un aventurier qui veut rester vivant et éviter de ruiner le comptoir.");

        for (std::size_t optionIndex = 0; optionIndex < question.options.size(); ++optionIndex)
        {
            screen.addOption(
                static_cast<int>(optionIndex + 1),
                question.options[optionIndex],
                "Réponse " + std::to_string(optionIndex + 1),
                true,
                "quest.guild.registration.answer." + std::to_string(i + 1) + "." + std::to_string(optionIndex + 1)
            );
        }

        int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choix invalide.");
        Console::clear();

        if (choice == question.correctChoice)
        {
            correctAnswers++;
            MessageScreen::show(
                "RÉPONSE VALIDÉE",
                "quest.guild.registration.correct",
                {"La gérante coche la ligne sans commentaire. C'est probablement bon signe."},
                false
            );
        }
        else
        {
            MessageScreen::show(
                "RÉPONSE NOTÉE",
                "quest.guild.registration.wrong",
                {"La gérante te corrige rapidement avant de passer à la suite. Elle préfère un aventurier corrigé à un aventurier enterré."},
                false
            );
        }
    }

    if (correctAnswers >= 12)
    {
        player.grantTitle("Aventurier");
        player.setActiveTitle("Aventurier");
        player.registerAtCurrentCityGuild();
        player.getInventory().addMaterial(MaterialCatalog::createGuildCard(1));
        player.getInventory().addMaterial(MaterialCatalog::createGuildRankFNotice(1));
        player.getInventory().addMaterial(MaterialCatalog::createGuildReliabilityGreenPellet(1));

        MessageScreen::show(
            "INSCRIPTION VALIDÉE",
            "quest.guild.registration.success",
            {
                "Résultat : " + std::to_string(correctAnswers) + "/" + std::to_string(questions.size()) + ".",
                "Titre obtenu : Aventurier.",
                "Objets reçus : Carte magique de guilde, Inscription de rang F, Pastille verte de fiabilité.",
                "Tu peux maintenant accéder au panneau de quêtes officiel et accepter les demandes de vendeurs liées à la guilde.",
                "Note d'économie : " + Money::coinScaleText()
            },
            false
        );
    }
    else
    {
        MessageScreen::show(
            "INSCRIPTION REFUSÉE POUR L'INSTANT",
            "quest.guild.registration.failed",
            {
                "Résultat : " + std::to_string(correctAnswers) + "/" + std::to_string(questions.size()) + ".",
                "La guilde refuse de confier une carte magique à quelqu'un qui risque de confondre courage et suicide.",
                "Tu pourras retenter l'inscription plus tard."
            },
            false
        );
    }
}

// EN: openGuild declares or implements a focused behavior used by this module.
// FR: openGuild déclare ou implémente un comportement précis utilisé par ce module.
void QuestMenu::openGuild(Player& player)
{
    expireOverdueQuestDeadlines(player, "quest.guild");
    while (true)
    {
        const ClientQuestCounts guildCounts = countQuestsForClient(player, "Maître de guilde");
        const int activeServiceCount = countActiveGuildServiceQuests(player);
        const bool isAdventurer = player.hasTitle("Aventurier");
        const bool guildOpen = guildIsOpen(player);
        MenuScreen screen("GUILDE", "quest.guild");
        screen.addLine("La guilde centralise les quêtes officielles.");
        screen.addLine("Temps actuel : " + player.formatWorldDateTimeLine());
        screen.addLine(guildOpeningLine(player));
        screen.addLine("Ville actuelle : " + currentCityName(player) + ".");
        screen.addLine("Statut d'inscription : " + std::string(isAdventurer ? "Aventurier inscrit" : "Non inscrit"));
        if (isAdventurer)
        {
            screen.addLine("Enregistrement local : " + std::string(player.isRegisteredAtCurrentCityGuild() ? "validé" : "mise à niveau requise"));
        }
        if (isAdventurer)
        {
            std::vector<std::string> standingLines = guildStandingSummaryLines(player);
            for (const std::string& line : standingLines)
            {
                screen.addLine(line);
            }
        }
        screen.addLine("Économie locale : " + Money::coinScaleText());
        screen.addLine("Contrats officiels et défis utilisent deux limites séparées.");
        screen.addLine("Défis actifs : " + std::to_string(player.getQuestLog().getActiveGuildChallengeCount()) + "/3.");
        screen.addLine("Contrats de guilde : " + clientQuestStatusText(guildCounts));
        screen.addLine("Services de guilde à traiter au comptoir : " + std::to_string(activeServiceCount) + ".");
        if (!player.hasTitle("Témoin du marchand bleu") && player.getWorldDaysElapsed() % 5 == 0)
        {
            screen.addLine("Rumeur : un marchand très musclé en t-shirt bleu-vert, pantalon violet et armure de diamant bleu apparaîtrait sur certaines routes avant de disparaître sans laisser de traces.");
        }
        else if (player.hasTitle("Témoin du marchand bleu"))
        {
            screen.addLine("Légende confirmée : certains membres de la guilde ont cessé de rire quand tu as décrit l'homme à l'armure de diamant bleu.");
        }
        if (!player.hasTitle("Les deux du même comptoir") && player.getWorldDaysElapsed() % 4 == 1)
        {
            screen.addLine("Moquerie de comptoir : deux vendeurs ambulants auraient essayé de vendre la même caisse l'un à l'autre pendant une heure.");
        }
        else if (player.hasTitle("Les deux du même comptoir"))
        {
            screen.addLine("La guilde appelle désormais Bob et Maurice « les deux problèmes vendus ensemble ». Personne ne sait lequel a commencé.");
        }
        std::vector<std::string> guildAmbientLines = clientAmbientDialogueLines(player, "Maître de guilde", guildCounts);
        for (const std::string& line : guildAmbientLines)
        {
            screen.addLine(line);
        }
        screen.addBackOption("Retour", "quest.guild.back");

        MenuOptionItemData boardData = makeQuestNavigationItemData(
            "quest",
            "Guilde",
            "quest",
            "Panneau de quêtes",
            "Voir les contrats officiels disponibles.",
            "Maître de guilde"
        );
        boardData.status = isAdventurer ? "Officiel" : "Inscription requise";

        MenuOptionItemData turnInData = makeQuestNavigationItemData(
            "quest",
            "Guilde",
            "turn_in",
            "Contrats terminés",
            guildCounts.ready > 0 ? "Valider les contrats prêts auprès du maître de guilde." : "Aucun contrat de guilde prêt.",
            "Maître de guilde"
        );
        turnInData.status = guildCounts.ready > 0 ? std::to_string(guildCounts.ready) + " prêt(s)" : "Indisponible";
        turnInData.important = guildCounts.ready > 0;

        MenuOptionItemData serviceData = makeQuestNavigationItemData(
            "quest",
            "Guilde",
            "service",
            "Services de guilde",
            activeServiceCount > 0 ? "Traiter les petits contrats locaux directement au comptoir." : "Aucun service de guilde à traiter.",
            "Maître de guilde"
        );
        serviceData.status = activeServiceCount > 0 ? std::to_string(activeServiceCount) + " service(s)" : "Indisponible";
        serviceData.important = activeServiceCount > 0;

        MenuOptionItemData registrationData = makeQuestNavigationItemData(
            "quest",
            "Guilde",
            "register",
            "Inscription aventurier",
            isAdventurer ? "Déjà inscrit." : "Passer le QCM de connaissances et recevoir la carte magique.",
            "Maître de guilde"
        );
        registrationData.status = isAdventurer ? "Déjà obtenu" : "Disponible";
        registrationData.important = !isAdventurer;

        const std::string guildClosedReason = "Les portes de la guilde sont fermées pour la nuit. Les dossiers restent derrière le comptoir jusqu'au matin.";
        const std::string boardHint = !guildOpen
            ? guildClosedReason
            : (isAdventurer ? "Consulter les contrats officiels disponibles." : "Inscription Aventurier requise avant d'accepter un contrat officiel.");
        const std::string turnInHint = !guildOpen
            ? guildClosedReason
            : (guildCounts.ready > 0 ? "Valider un contrat terminé." : "Aucun contrat de guilde prêt à rendre.");
        const std::string serviceHint = !guildOpen
            ? guildClosedReason
            : (!isAdventurer
                ? "La guilde ne confie pas ses services officiels aux personnes non inscrites."
                : (activeServiceCount > 0 ? "Avancer un contrat de service local." : "Aucun service actif à traiter."));
        const std::string registrationHint = !guildOpen
            ? "La gérante a fermé le registre d'inscription pour la nuit. Reviens au matin."
            : (isAdventurer ? "Relire le statut d'inscription." : "Passer le test QCM de la guilde.");

        screen.addOption(1, "Voir le panneau de quêtes", boardHint, guildOpen && isAdventurer, "quest.guild.board", boardData);
        screen.addOption(2, "Rendre une quête de guilde terminée" + (guildCounts.ready > 0 ? " [" + std::to_string(guildCounts.ready) + "]" : ""),
            turnInHint,
            guildOpen && guildCounts.ready > 0,
            "quest.guild.turn_in",
            turnInData
        );
        screen.addOption(3, "Consulter le journal", "Lire le journal complet des quêtes.", true, "quest.guild.journal");
        screen.addOption(4, "Traiter un service de guilde" + (activeServiceCount > 0 ? " [" + std::to_string(activeServiceCount) + "]" : ""),
            serviceHint,
            guildOpen && isAdventurer && activeServiceCount > 0,
            "quest.guild.service",
            serviceData
        );
        screen.addOption(5, isAdventurer ? "Voir l'inscription aventurier" : "S'inscrire comme aventurier",
            registrationHint,
            guildOpen,
            "quest.guild.registration",
            registrationData
        );
        screen.addOption(6, "Défis de guilde [" + std::to_string(player.getQuestLog().getActiveGuildChallengeCount()) + "/3]",
            !guildOpen ? guildClosedReason : (isAdventurer ? "Trois défis renouvelés chaque jour, valables deux jours après acceptation." : "Inscription Aventurier requise."),
            guildOpen && isAdventurer,
            "quest.guild.challenges"
        );

        int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choix invalide.");
        Console::clear();

        if (choice == 0)
        {
            return;
        }

        if (!guildOpen && choice != 3)
        {
            MessageScreen::show(
                "GUILDE FERMÉE",
                "quest.guild.closed.action",
                {guildOpeningLine(player), "Seul le journal peut être consulté pendant la nuit."},
                false
            );
            continue;
        }

        if (choice == 1)
        {
            acceptGuildQuest(player);
        }
        else if (choice == 2)
        {
            completeQuestAtClient(player, "Maître de guilde");
        }
        else if (choice == 3)
        {
            displayQuestJournal(player);
        }
        else if (choice == 4)
        {
            resolveGuildServiceQuest(player);
        }
        else if (choice == 5)
        {
            openGuildRegistration(player);
        }
        else if (choice == 6)
        {
            openGuildChallenges(player);
        }
    }
}


void QuestMenu::openGuildChallenges(Player& player)
{
    if (!guildIsOpen(player) || !player.hasTitle("Aventurier"))
    {
        MessageScreen::show(
            "DÉFIS INDISPONIBLES",
            "quest.guild.challenges.blocked",
            {
                !guildIsOpen(player) ? guildOpeningLine(player) : "L'inscription Aventurier est requise.",
                "Les défis sont des contrats courts et séparés des quêtes officielles habituelles."
            },
            false
        );
        return;
    }

    QuestLog& questLog = player.getQuestLog();
    questLog.ensureGuildChallengeBoardReady(player.getLevel(), player.getWorldDaysElapsed());

    while (true)
    {
        std::vector<Quest>& offers = questLog.getGuildChallengeBoardOffers();
        MenuScreen screen("DÉFIS DE GUILDE", "quest.guild.challenges");
        screen.addLine("Trois défis sont tirés au début de chaque journée.");
        screen.addLine("Un défi accepté reste valable aujourd'hui et le jour suivant, puis disparaît s'il n'est pas réussi.");
        screen.addLine("Les défis expirés peuvent revenir plus tard dans un nouveau tirage.");
        screen.addLine("Défis actifs : " + std::to_string(questLog.getActiveGuildChallengeCount()) + "/3.");
        screen.addLine("Récompense spéciale : Marques de défi, avec un peu d'expérience et d'or seulement.");
        if (player.hasTitle("Porte-marque de la guilde"))
        {
            screen.addLine("Le maître de guilde reconnaît ton titre de porte-marque et te laisse consulter le comptoir sans commentaire supplémentaire.");
        }
        if (player.hasTitle("Personne ne reste derrière"))
        {
            screen.addLine("Plusieurs recrues murmurent que tu refuses de considérer une victoire comme propre si quelqu'un reste au sol.");
        }
        if (player.hasTitle("Seulement toi et le boss"))
        {
            screen.addLine("Le panneau porte une note manuscrite : « Oui, l'exploit sans compétence a été vérifié. Non, ne l'imitez pas sans préparation. »");
        }
        screen.addBackOption("Retour", "quest.guild.challenges.back");
        screen.addOption(90, "Comptoir des Marques de défi", "Échanges modestes, nouveau tirage limité et reconnaissance cosmétique.", true, "quest.guild.challenges.mark_counter");

        if (offers.empty())
        {
            screen.addLine("Tous les défis du jour ont déjà été pris. Le panneau changera au prochain jour.");
        }

        for (std::size_t i = 0; i < offers.size(); ++i)
        {
            const Quest& challenge = offers[i];
            MenuOptionItemData itemData;
            itemData.structured = true;
            itemData.kind = "challenge";
            itemData.section = "Défis de guilde";
            itemData.actionType = "accept";
            itemData.name = challenge.title;
            itemData.detail = challenge.objective;
            itemData.status = "Disponible aujourd'hui";
            itemData.reward = questRewardText(challenge);
            itemData.progress = "Durée après acceptation : 2 jours";
            itemData.owner = "Maître de guilde";
            itemData.important = true;

            screen.addOption(
                static_cast<int>(i) + 1,
                "[Défi " + challenge.rank + "] " + challenge.title,
                challenge.objective,
                questLog.getActiveGuildChallengeCount() < 3,
                "quest.guild.challenges.accept." + std::to_string(i + 1),
                itemData
            );
        }

        const int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choix invalide.");
        Console::clear();

        if (choice == 0)
        {
            return;
        }
        if (choice == 90)
        {
            openChallengeMarkCounter(player);
            continue;
        }

        if (choice < 1 || choice > static_cast<int>(offers.size()))
        {
            continue;
        }

        if (questLog.getActiveGuildChallengeCount() >= 3)
        {
            MessageScreen::show(
                "LIMITE DE DÉFIS ATTEINTE",
                "quest.guild.challenges.full",
                {"Tu as déjà trois défis actifs.", "Réussis-en un, rends-le ou attends son expiration avant d'en prendre un autre."},
                false
            );
            continue;
        }

        Quest selected = offers[static_cast<std::size_t>(choice - 1)];
        const int decision = askQuestOfferDecision(
            "DÉFI DE GUILDE",
            "quest.guild.challenges.offer",
            player,
            selected,
            {
                "La fiche porte une marque différente des contrats ordinaires.",
                "Elle ne promet pas une fortune : elle certifie surtout que l'exploit a réellement été accompli."
            }
        );
        Console::clear();

        if (decision != 1)
        {
            continue;
        }

        prepareQuestForAcceptance(selected, player.getWorldDaysElapsed());
        if (!questLog.addQuestWithGuildLimit(selected, 3))
        {
            MessageScreen::show(
                "DÉFI NON AJOUTÉ",
                "quest.guild.challenges.add_failed",
                {"Le défi existe peut-être déjà dans ton journal ou la limite est atteinte."},
                false
            );
            continue;
        }

        questLog.removeGuildChallengeBoardOfferAt(choice - 1);
        MessageScreen::show(
            "DÉFI ACCEPTÉ",
            "quest.guild.challenges.accepted",
            {
                selected.title,
                "Condition : " + selected.objective,
                "Délai : aujourd'hui et le jour suivant.",
                "Récompense : " + questRewardText(selected),
                "La réussite sera enregistrée automatiquement par le combat ou l'action concernée."
            },
            false
        );
    }
}


// EN: resolveGuildServiceQuest declares or implements a focused behavior used by this module.
// FR: resolveGuildServiceQuest déclare ou implémente un comportement précis utilisé par ce module.
void QuestMenu::resolveGuildServiceQuest(Player& player)
{
    constexpr std::size_t servicesPerPage = 5;
    std::size_t pageIndex = 0;

    while (true)
    {
        std::vector<Quest>& quests = player.getQuestLog().getQuests();
        std::vector<int> serviceIndexes;

        for (int i = 0; i < static_cast<int>(quests.size()); ++i)
        {
            if (isActiveGuildServiceQuest(quests[i]))
            {
                serviceIndexes.push_back(i);
            }
        }

        if (serviceIndexes.empty())
        {
            MessageScreen::show(
                "AUCUN SERVICE À TRAITER",
                "quest.guild.service.empty",
                {
                    "Aucun service de guilde n'attend au comptoir.",
                    "Les contrats de combat et d'exploration se font dehors ; les services se règlent ici."
                }
            );
            return;
        }

        const std::size_t totalPages = PagedMenu::pageCount(serviceIndexes.size(), servicesPerPage);
        if (pageIndex >= totalPages)
        {
            pageIndex = totalPages == 0 ? 0 : totalPages - 1;
        }

        const std::size_t first = PagedMenu::firstIndex(pageIndex, servicesPerPage);
        const std::size_t last = PagedMenu::lastIndexExclusive(serviceIndexes.size(), pageIndex, servicesPerPage);

        MenuScreen screen("SERVICES DE GUILDE", "quest.guild.service");
        screen.addLine("Ces contrats ne demandent pas de chercher une zone floue : ils se règlent depuis le comptoir de guilde.");
        screen.addLine("Chaque traitement avance le service. Une fois terminé, il passe dans les quêtes prêtes à rendre.");
        screen.addLine("Affichage : " + PagedMenu::rangeText(first, last, serviceIndexes.size()));
        screen.addBackOption("Retour", "quest.guild.service.back");

        for (std::size_t i = first; i < last; ++i)
        {
            const Quest& quest = quests[serviceIndexes[i]];
            std::string label = "[Service - Rang " + quest.rank + "] " + quest.title
                + " | Progression : " + std::to_string(quest.progress) + "/" + std::to_string(quest.target)
                + " | " + questRewardText(quest);

            MenuOptionItemData itemData;
            itemData.structured = true;
            itemData.kind = "quest";
            itemData.section = "Services de guilde";
            itemData.actionType = "service";
            itemData.name = quest.title;
            itemData.detail = "";
            itemData.status = "À traiter au comptoir";
            itemData.reward = questRewardText(quest);
            itemData.progress = std::to_string(quest.progress) + "/" + std::to_string(quest.target);
            itemData.owner = "Maître de guilde";
            itemData.important = true;

            screen.addOption(
                static_cast<int>(10 + (i - first)),
                label,
                "Traiter une étape de ce service local.",
                true,
                "quest.guild.service.select." + std::to_string(i + 1),
                itemData
            );
        }

        PagedMenu::addNavigationOptions(screen, pageIndex, totalPages);

        int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choix invalide.");
        Console::clear();

        if (choice == 0)
        {
            return;
        }

        if (choice == 98 && pageIndex > 0)
        {
            --pageIndex;
            continue;
        }

        if (choice == 99 && pageIndex + 1 < totalPages)
        {
            ++pageIndex;
            continue;
        }

        const int localQuestIndex = choice - 10;
        if (localQuestIndex < 0 || first + static_cast<std::size_t>(localQuestIndex) >= last)
        {
            MessageScreen::show(
                "SERVICE INDISPONIBLE",
                "quest.guild.service.invalid",
                {"Ce choix ne correspond à aucun service affiché."}
            );
            continue;
        }

        Quest& quest = quests[serviceIndexes[first + static_cast<std::size_t>(localQuestIndex)]];
        Random random;
        MicroChallengeResult microChallenge = runGuildServiceMicroChallenge(quest, random);

        const int before = quest.progress;
        if (microChallenge.success)
        {
            quest.progress = std::min(quest.target, quest.progress + 1);
            if (quest.progress >= quest.target)
            {
                quest.completed = true;
            }
        }

        std::vector<std::string> lines;
        lines.push_back("Service traité : " + quest.title);
        lines.push_back("La gérante ne laisse plus passer les allers-retours de 8 secondes : même un petit service demande une vraie action.");
        lines.insert(lines.end(), microChallenge.lines.begin(), microChallenge.lines.end());
        lines.push_back("Progression : " + std::to_string(before) + "/" + std::to_string(quest.target)
            + " -> " + std::to_string(quest.progress) + "/" + std::to_string(quest.target));

        if (!microChallenge.success)
        {
            if (!microChallenge.partial)
            {
                applySoftServiceFailureCost(player, quest, random, lines);
            }
            MessageScreen::show(
                "SERVICE BLOQUÉ",
                "quest.guild.service.micro_failed",
                lines
            );
            continue;
        }

        if (quest.completed)
        {
            lines.push_back("Service terminé : tu peux maintenant le rendre auprès du Maître de guilde.");
            lines.push_back("Chemin rapide : Quêtes > Rendre une quête prête, ou Guilde > Rendre une quête de guilde terminée.");
        }
        else
        {
            lines.push_back("Le service demande encore une étape au comptoir avant d'être tamponnable.");
        }

        MessageScreen::show(
            quest.completed ? "SERVICE TERMINÉ" : "SERVICE AVANCÉ",
            quest.completed ? "quest.guild.service.completed" : "quest.guild.service.progressed",
            lines
        );
    }
}

    void processPersonalServiceAtClient(Player& player, const std::string& clientName)
    {
        std::vector<Quest>& quests = player.getQuestLog().getQuests();
        std::vector<std::size_t> serviceIndexes;

        for (std::size_t i = 0; i < quests.size(); ++i)
        {
            if (isActivePersonalServiceQuestForClient(quests[i], clientName))
            {
                serviceIndexes.push_back(i);
            }
        }

        if (serviceIndexes.empty())
        {
            MessageScreen::show(
                "AUCUN SERVICE À TRAITER",
                "quest.client.service.empty",
                {
                    clientName + " n'a aucune demande de service active à traiter pour le moment.",
                    "Les quêtes avec questions doivent être acceptées auprès du PNJ notable concerné, puis rejouées ici."
                }
            );
            return;
        }

        constexpr std::size_t servicesPerPage = 5;
        std::size_t pageIndex = 0;
        while (true)
        {
            const std::size_t totalPages = PagedMenu::pageCount(serviceIndexes.size(), servicesPerPage);
            const std::size_t first = PagedMenu::firstIndex(pageIndex, servicesPerPage);
            const std::size_t last = PagedMenu::lastIndexExclusive(serviceIndexes.size(), pageIndex, servicesPerPage);

            MenuScreen screen("SERVICE DU CONTACT", "quest.client.service");
            screen.setPagination(pageIndex, totalPages);
            screen.addSubtitle(clientName);
            screen.addLine("Ce n'est pas un aller-retour de 8 secondes : le PNJ te fait vraiment résoudre l'étape.");
            screen.addLine("Les informations restent des estimations de pourparler : [objectif de quête probable] jusqu'à validation finale.");
            screen.addBackOption("Retour", "quest.client.service.back");

            for (std::size_t i = first; i < last; ++i)
            {
                const Quest& quest = quests[serviceIndexes[i]];
                MenuOptionItemData itemData;
                itemData.structured = true;
                itemData.kind = "quest";
                itemData.section = "Service PNJ";
                itemData.actionType = "service";
                itemData.name = quest.title;
                itemData.detail = "";
                itemData.status = "À traiter avec " + clientName;
                itemData.reward = approximateQuestRewardText(quest);
                itemData.progress = std::to_string(quest.progress) + "/" + std::to_string(quest.target);
                itemData.owner = clientName;
                itemData.important = true;

                screen.addOption(
                    static_cast<int>(10 + (i - first)),
                    questCardLabel(quest),
                    "",
                    true,
                    "quest.client.service.select." + std::to_string(i + 1),
                    itemData
                );
            }

            PagedMenu::addNavigationOptions(screen, pageIndex, totalPages);
            int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choix invalide.");
            Console::clear();

            if (choice == 0)
            {
                return;
            }
            if (choice == 98 && pageIndex > 0)
            {
                --pageIndex;
                continue;
            }
            if (choice == 99 && pageIndex + 1 < totalPages)
            {
                ++pageIndex;
                continue;
            }

            const int localQuestIndex = choice - 10;
            if (localQuestIndex < 0 || first + static_cast<std::size_t>(localQuestIndex) >= last)
            {
                MessageScreen::show(
                    "SERVICE INDISPONIBLE",
                    "quest.client.service.invalid",
                    {"Ce choix ne correspond à aucune demande affichée."}
                );
                continue;
            }

            Quest& quest = quests[serviceIndexes[first + static_cast<std::size_t>(localQuestIndex)]];
            Random random;
            MicroChallengeResult microChallenge = runGuildServiceMicroChallenge(quest, random);
            const int before = quest.progress;

            if (microChallenge.success)
            {
                quest.progress = std::min(quest.target, quest.progress + 1);
                if (quest.progress >= quest.target)
                {
                    quest.completed = true;
                }
            }

            std::vector<std::string> lines;
            lines.push_back("Service traité avec " + clientName + " : " + quest.title);
            lines.push_back("[Objectif de quête probable] " + quest.objective);
            lines.insert(lines.end(), microChallenge.lines.begin(), microChallenge.lines.end());
            lines.push_back("Progression : " + std::to_string(before) + "/" + std::to_string(quest.target)
                + " -> " + std::to_string(quest.progress) + "/" + std::to_string(quest.target));

            if (!microChallenge.success)
            {
                if (!microChallenge.partial)
                {
                    applySoftServiceFailureCost(player, quest, random, lines);
                }
                MessageScreen::show("SERVICE BLOQUÉ", "quest.client.service.micro_failed", lines);
                continue;
            }

            if (quest.completed)
            {
                lines.push_back("Demande terminée : retourne la rendre à " + clientName + ".");
            }
            else
            {
                lines.push_back("Le PNJ garde encore une étape de questions ou de vérification avant de valider.");
            }

            MessageScreen::show(
                quest.completed ? "DEMANDE TERMINÉE" : "DEMANDE AVANCÉE",
                quest.completed ? "quest.client.service.completed" : "quest.client.service.progressed",
                lines
            );
        }
    }

// EN: acceptGuildQuest declares or implements a focused behavior used by this module.
// FR: acceptGuildQuest déclare ou implémente un comportement précis utilisé par ce module.
void QuestMenu::acceptGuildQuest(Player& player)
{
    if (!guildIsOpen(player))
    {
        MessageScreen::show(
            "GUILDE FERMÉE",
            "quest.guild.closed.board",
            {
                guildOpeningLine(player),
                "Temps actuel : " + player.formatWorldDateTimeLine(),
                "Le journal reste consultable, mais les contrats, inscriptions et tampons attendront l'ouverture."
            },
            false
        );
        return;
    }

    if (!player.hasTitle("Aventurier"))
    {
        MessageScreen::show(
            "INSCRIPTION REQUISE",
            "quest.guild.board.registration_required",
            {
                "Le panneau officiel est visible, mais les contrats ne peuvent pas être acceptés sans titre Aventurier.",
                "Passe d'abord l'inscription à la guilde pour obtenir ta carte magique."
            },
            false
        );
        return;
    }

    QuestLog& questLog = player.getQuestLog();
    const GuildStanding standingForBoard = guildStandingForPlayer(player);
    questLog.ensureGuildBoardReady(player.getLevel(), player.getWorldDaysElapsed(), guildBoardOfferBonusForStanding(standingForBoard));

    const std::vector<Quest>& board = questLog.getGuildBoardOffers();
    constexpr std::size_t itemsPerPage = 6;
    std::size_t pageIndex = 0;
    int choice = 0;

    while (true)
    {
        const std::size_t totalPages = PagedMenu::pageCount(board.size(), itemsPerPage);
        if (pageIndex >= totalPages)
        {
            pageIndex = totalPages > 0 ? totalPages - 1 : 0;
        }

        const std::size_t first = PagedMenu::firstIndex(pageIndex, itemsPerPage);
        const std::size_t last = PagedMenu::lastIndexExclusive(board.size(), pageIndex, itemsPerPage);

        MenuScreen screen("PANNEAU DE GUILDE", "quest.guild.board");
        const GuildStanding standing = guildStandingForPlayer(player);
        const int activeQuestLimit = guildActiveQuestLimitForStanding(standing);
        screen.addLine("Quêtes actives : " + std::to_string(questLog.getActiveGuildQuestCount()) + "/" + std::to_string(activeQuestLimit));
        screen.addLine("Offres visibles : " + std::to_string(board.size()) + "/" + std::to_string(questLog.getGuildBoardTargetSize()));
        screen.addLine("Affichage : " + PagedMenu::rangeText(first, last, board.size()));
        screen.addLine("Carte : rang " + standing.rank + " / pastille " + standing.pellet + ".");

        int remainingBeforeRefresh = questLog.getGuildBoardCombatsBeforeRefresh(player.getWorldDaysElapsed());
        if (remainingBeforeRefresh <= 0)
        {
            screen.addLine("Une fiche expirera au prochain jour écoulé.");
        }
        else
        {
            screen.addLine("Prochaine expiration de fiche dans " + std::to_string(remainingBeforeRefresh)
                + " jour" + (remainingBeforeRefresh > 1 ? "s" : "") + ".");
        }

        if (questLog.getGuildBoardPendingReplacements() > 0)
        {
            screen.addLine("Des places prises seront remplacées après le prochain jour écoulé.");
        }

        if (board.empty())
        {
            screen.addLine("Le panneau est vide pour l'instant. Repasse après un jour écoulé.");
        }

        PagedMenu::addNavigationOptions(screen, pageIndex, totalPages);

        for (std::size_t i = first; i < last; ++i)
        {
            const std::string label = questCardLabel(board[i]);
            const bool alreadyTaken = questLog.hasQuest(board[i].id);
            const bool rankAllowed = isGuildQuestRankAllowedForStanding(board[i], standing);

            MenuOptionItemData itemData;
            itemData.structured = true;
            itemData.kind = "quest";
            itemData.section = "Panneau de guilde";
            itemData.actionType = "quest";
            itemData.name = board[i].title;
            itemData.detail = "";
            itemData.status = alreadyTaken ? "Déjà prise" : (rankAllowed ? "Disponible" : "Rang/pastille insuffisant");
            itemData.reward = questRewardText(board[i]);
            itemData.progress = "Rang " + board[i].rank;
            itemData.owner = "Guilde";
            itemData.important = !alreadyTaken && rankAllowed;

            screen.addOption(
                static_cast<int>(i) + 1,
                label,
                "",
                rankAllowed && !alreadyTaken,
                "quest.guild.board.accept." + std::to_string(i + 1),
                itemData
            );
        }

        choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choix invalide.");
        Console::clear();

        if (choice == 0)
        {
            return;
        }

        if (choice == 98 && pageIndex > 0)
        {
            --pageIndex;
            continue;
        }

        if (choice == 99 && pageIndex + 1 < totalPages)
        {
            ++pageIndex;
            continue;
        }

        break;
    }

    Quest selectedQuest = board[choice - 1];

    const GuildStanding standing = guildStandingForPlayer(player);
    const int activeQuestLimit = guildActiveQuestLimitForStanding(standing);
    if (!questLog.canAcceptGuildQuest(activeQuestLimit))
    {
        MessageScreen::show(
            "PANNEAU SATURÉ",
            "quest.guild.board.full",
            {
                "Tu as déjà " + std::to_string(activeQuestLimit) + " quêtes de guilde actives.",
                "Termine ou rends-en une avant d'en accepter une autre.",
                "Capacité actuelle : rang " + standing.rank + " / " + std::to_string(activeQuestLimit) + " contrat(s) actif(s)."
            }
        );
        return;
    }

    if (questLog.hasQuest(selectedQuest.id))
    {
        MessageScreen::show(
            "CONTRAT DÉJÀ PRIS",
            "quest.guild.board.already_taken",
            {
                "Ce contrat est déjà dans ton journal.",
                "La guilde refuse de tamponner deux fois le même papier, même avec un sourire."
            }
        );
        return;
    }

    if (!isGuildQuestRankAllowedForStanding(selectedQuest, standing))
    {
        MessageScreen::show(
            "CARTE REFUSÉE",
            "quest.guild.board.rank_blocked",
            {
                "La carte magique refuse de tamponner ce contrat pour l'instant.",
                "Rang du contrat : " + selectedQuest.rank + ".",
                "Dossier actuel : rang " + standing.rank + ", pastille " + standing.pellet + ".",
                "Valide des contrats plus simples pour faire monter le rang, ou stabilise ta pastille en évitant les échecs de délai."
            }
        );
        return;
    }

    int accept = askQuestOfferDecision(
        "CONTRAT DE GUILDE",
        "quest.guild.board.offer",
        player,
        selectedQuest,
        {
            "La gérante détache la fiche du panneau sans encore la signer.",
            "Ici, les informations sont cadrées : objectif, rang, zone et récompense sont notés officiellement."
        }
    );
    Console::clear();

    if (accept != 1)
    {
        MessageScreen::show(
            "CONTRAT LAISSÉ",
            "quest.guild.board.declined",
            {
                "Tu laisses le contrat sur le panneau.",
                "Quelqu'un d'autre le prendra peut-être, ou peut-être pas. La guilde adore ce genre de suspense administratif."
            }
        );
        return;
    }

    prepareQuestForAcceptance(selectedQuest, player.getWorldDaysElapsed());

    if (questLog.addQuestWithGuildLimit(selectedQuest, activeQuestLimit))
    {
        questLog.removeGuildBoardOfferAt(choice - 1, player.getWorldDaysElapsed());

        std::vector<std::string> lines = {
            "Quête acceptée : " + selectedQuest.title
        };
        std::vector<std::string> dialogue = guildQuestAcceptedDialogueLines(player, selectedQuest);
        lines.insert(lines.end(), dialogue.begin(), dialogue.end());
        lines.push_back("Objectif : " + selectedQuest.objective);
        lines.push_back("Zone/action jouable : " + questPlayableLocationHint(selectedQuest));
        lines.push_back("Comment faire : " + questProgressMethodText(selectedQuest));
        lines.push_back("Récompenses : " + questRewardText(selectedQuest));
        appendDeadlineLine(lines, selectedQuest, player.getWorldDaysElapsed());
        lines.push_back("Une nouvelle place sera préparée après le prochain jour écoulé.");

        MessageScreen::show("QUÊTE DE GUILDE ACCEPTÉE", "quest.guild.board.accepted", lines);
    }
    else
    {
        MessageScreen::show(
            "QUÊTE NON AJOUTÉE",
            "quest.guild.board.failed",
            {"Impossible d'accepter cette quête. Elle est peut-être déjà active."}
        );
    }
}

// EN: openExploration declares or implements a focused behavior used by this module.
// FR: openExploration déclare ou implémente un comportement précis utilisé par ce module.
void QuestMenu::openExploration(Player& player, DifficultyMode difficulty, DeathRuleMode deathRule)
{
    expireOverdueQuestDeadlines(player, "quest.exploration", false);
    openExplorationMenu(player, difficulty, deathRule);
}

// EN: openLocations declares or implements a focused behavior used by this module.
// FR: openLocations déclare ou implémente un comportement précis utilisé par ce module.
void QuestMenu::openLocations(Player& player)
{
    expireOverdueQuestDeadlines(player, "quest.locations");

    enum class LocationCategory
    {
        City,
        Outside,
        Shop
    };

    struct LocationEntry
    {
        std::string label;
        std::string detail;
        std::string client;
        LocationCategory category;
        bool guild = false;
        bool vault = false;
        bool travel = false;
    };

    std::vector<LocationEntry> entries = {
        {"Guilde", "Contrats officiels, panneau, journal et services de guilde.", "Maître de guilde", LocationCategory::City, true, false},
        {"Coffre municipal", "Stockage personnel sécurisé, améliorable et séparé de l'inventaire transporté.", "Intendant du coffre", LocationCategory::City, false, true},
        {"Intendance de Mira", "Priorités du village, murs, matériaux et validation de la route principale.", "Mira", LocationCategory::City},
        {"Poste d'Orren", "Routes, ponts, bornes déplacées et disparitions hors des murs.", "Orren", LocationCategory::City},
        {"Infirmerie de Lysa", "Blessés, remèdes simples et symptômes qui ne correspondent pas aux blessures normales.", "Lysa", LocationCategory::City},
        {"Forge de Bram", "Atelier de survie, outils, réparations urgentes et matériaux récupérables.", "Bram", LocationCategory::City},
        {"Archives de Soryn", "Traces, légendes, rumeurs et indices à vérifier.", "Soryn", LocationCategory::City},
        {"Place du village", "Rumeurs, habitants et petites demandes locales.", "Villageois nerveux", LocationCategory::City},
        {"Quartier abandonné", "Caves, maisons vides, contrats douteux, vieilles pièces et automates oubliés.", "Rika des clés", LocationCategory::City},
        {"Bureau des inscriptions", "Paperasse, pastilles, abonnements et litiges administratifs [objectif de quête probable].", "Scribe Ysolde", LocationCategory::City},
        {"Bibliothèque des cartes", "Cartes, bestiaire, magie, plantes et transitions [objectif de quête probable].", "Archiviste Meron", LocationCategory::City},
        {"Relais des routes", "Voyage entre villes, distances, conditions d'entrée, carte des biomes et bons de livraison [objectif de quête probable].", "Noro le palefrenier", LocationCategory::City, false, false, true},
        {"Auberge du Repos Bruyant", "Hébergement, additions et services de ville [objectif de quête probable].", "Tavia l'aubergiste", LocationCategory::City},

        {"Route commerciale", "Marchands, convois, bornes, risques de voyage et pistes à contrôler.", "Marchand inquiet", LocationCategory::Outside},
        {"Bocage aux lanternes", "Champignons-lampes, résine d'écho, spores calmes et bêtes attirées par la lumière.", "Mila des lanternes", LocationCategory::Outside},
        {"Désert d'argile rouge", "Argile rouge, sel lunaire, fausses oasis, pilleurs et constructions fissurées.", "Safa la pisteuse", LocationCategory::Outside},
        {"Mine sifflante", "Rails vibrants, fer froid, ressorts, vieux mécanismes et golems de mine.", "Bram le foreur", LocationCategory::Outside},

        {"Forge", "Commandes, réparations et demandes générales du forgeron.", "Forgeron", LocationCategory::Shop},
        {"Herboristerie", "Plantes, ingrédients, remèdes et demandes de l'alchimiste.", "Alchimiste", LocationCategory::Shop},
        {"Comptoir de Prunigil", "Marchandage, calcul, factures et registres [objectif de quête probable].", "Prunigil le marchand", LocationCategory::Shop},
        {"Boutique de monstres", "Composants de créatures et revente spécialisée.", "Vendeur de composants", LocationCategory::Shop},
        {"Boutique de matériaux", "Matériaux, stocks et approvisionnement.", "Vendeur de matériaux", LocationCategory::Shop},
        {"Armurerie défensive", "Protections, pièces d'armure et commandes.", "Armurier", LocationCategory::Shop},
        {"Forge d'armes", "Armes, réparation et approvisionnement.", "Vendeur d'armes", LocationCategory::Shop},
        {"Boutique de consommables", "Potions, consommables et réserves.", "Vendeur de consommables", LocationCategory::Shop},
        {"Bibliothèque", "Notes, savoirs, renseignements et pistes de recherche.", "Bibliothécaire", LocationCategory::Shop},
        {"Laboratoire de Maëra", "Alchimie, dosages, étiquettes de potions et sécurité [objectif de quête probable].", "Maëra l'alchimiste", LocationCategory::Shop}
    };

    if (const City* city = City::findById(player.getCurrentCityId()))
    {
        std::vector<CityBuildingPreview> cityBuildings = CityTravelRules::getBuildingsForCity(player, *city);
        for (const CityBuildingPreview& building : cityBuildings)
        {
            const bool guildBuilding = building.id == "guild";
            const bool vaultBuilding = building.id == "vault";
            const bool travelBuilding = building.id == "gate";
            const bool alreadyStatic = guildBuilding || vaultBuilding || travelBuilding;
            if (alreadyStatic)
            {
                continue;
            }

            entries.push_back({
                city->getName() + " — " + building.name + (building.unlocked ? "" : " [verrouillé]"),
                building.detail + " Asset futur : " + building.pixelArtHint + ".",
                building.contact,
                LocationCategory::City,
                false,
                false,
                false
            });
        }
    }

    if (player.hasStoryModeStarted() && !player.hasStorySkip())
    {
        const int chapter = player.getStoryChapter();
        const int step = player.getStoryStep();
        const auto storyLocationUnlocked = [&](const LocationEntry& entry)
        {
            if (entry.guild || entry.vault)
            {
                return true;
            }

            const std::string& client = entry.client;
            if (client == "Mira" || client == "Villageois nerveux")
            {
                return true;
            }

            const bool firstReferentsPresent = chapter >= 2 || step >= 3;
            if (firstReferentsPresent
                && (client == "Orren" || client == "Lysa" || client == "Bram" || client == "Soryn"
                    || client == "Forgeron" || client == "Alchimiste"))
            {
                return true;
            }

            if (chapter >= 2 && client == "Marchand inquiet")
            {
                return true;
            }

            if (chapter >= 2 && step >= 7 && client == "Noro le palefrenier")
            {
                return true;
            }

            if (chapter >= 2 && step >= 9
                && (client == "Prunigil le marchand" || client == "Vendeur de composants" || client == "Vendeur de matériaux"
                    || client == "Armurier" || client == "Vendeur d'armes" || client == "Vendeur de consommables"
                    || client == "Scribe Ysolde" || client == "Maëra l'alchimiste" || client == "Tavia l'aubergiste"
                    || client == "Mila des lanternes" || client == "Safa la pisteuse" || client == "Rika des clés"))
            {
                return true;
            }

            if (chapter >= 2 && step >= 10 && client == "Bibliothécaire")
            {
                return true;
            }

            if (chapter >= 2 && step >= 12 && (client == "Archiviste Meron" || client == "Bram le foreur"))
            {
                return true;
            }

            return false;
        };

        entries.erase(
            std::remove_if(entries.begin(), entries.end(), [&](const LocationEntry& entry) { return !storyLocationUnlocked(entry); }),
            entries.end()
        );
    }

    while (true)
    {
        std::vector<LocationEntry> cityEntries;
        std::vector<LocationEntry> outsideEntries;
        std::vector<LocationEntry> shopEntries;
        for (const LocationEntry& entry : entries)
        {
            if (entry.category == LocationCategory::City)
            {
                cityEntries.push_back(entry);
            }
            else if (entry.category == LocationCategory::Outside)
            {
                outsideEntries.push_back(entry);
            }
            else
            {
                shopEntries.push_back(entry);
            }
        }

        std::vector<LocationEntry> allEntries = entries;

        struct LocationCategoryView
        {
            int choice;
            std::string title;
            std::string hint;
            std::vector<LocationEntry>* entries;
            bool all = false;
        };

        std::vector<LocationCategoryView> categoryViews;
        int nextChoice = 1;
        auto addCategory = [&](const std::string& title, const std::string& hint, std::vector<LocationEntry>& categoryEntries, bool all = false) {
            if (!categoryEntries.empty())
            {
                categoryViews.push_back({nextChoice++, title, hint, &categoryEntries, all});
            }
        };

        addCategory("Tout afficher", "Tous les lieux actuellement accessibles dans une seule liste, sans devoir fouiller les sections.", allEntries, true);
        addCategory("Ville", "Guilde, bâtiments, places, archives, auberge et contacts installés dans le village.", cityEntries);
        addCategory("Extérieur", "Routes et lieux précis situés hors des zones habitées.", outsideEntries);
        addCategory("Boutiques", "Commerces, ateliers, comptoirs et services où acheter, vendre ou se renseigner.", shopEntries);

        MenuScreen categoryScreen("LIEUX NOTABLES", "quest.locations.categories");
        categoryScreen.addSubtitle("Vue complète ou lieux classés par type");
        categoryScreen.addLine("Tout afficher regroupe les lieux accessibles pour aller vite. Les sections restent disponibles pour une recherche plus propre.");
        categoryScreen.addLine("Exploration reste réservée aux sorties par biome. Ici, tu choisis un endroit précis du monde.");
        categoryScreen.addLine("Ce classement est identique en bac à sable et en histoire ; l'histoire masque seulement les lieux qui n'existent pas encore ou ne sont pas accessibles.");
        categoryScreen.addBackOption("Retour", "quest.locations.categories.back");

        for (const LocationCategoryView& view : categoryViews)
        {
            int ready = 0;
            int active = 0;
            for (const LocationEntry& entry : *view.entries)
            {
                const ClientQuestCounts counts = countQuestsForClient(player, entry.client);
                ready += counts.ready;
                active += counts.active;
            }

            MenuOptionItemData itemData;
            itemData.structured = true;
            itemData.kind = "location_group";
            itemData.section = "Lieux notables";
            itemData.actionType = "open";
            itemData.name = view.title;
            itemData.detail = view.hint;
            itemData.status = std::to_string(view.entries->size()) + " lieu(x)";
            if (ready > 0)
            {
                itemData.progress = std::to_string(ready) + " objectif(s) à rendre";
                itemData.important = true;
            }
            else if (active > 0)
            {
                itemData.progress = std::to_string(active) + " objectif(s) en cours";
            }
            else
            {
                itemData.progress = "Aucun objectif signalé";
            }

            categoryScreen.addOption(
                view.choice,
                view.title + (ready > 0 ? " [" + std::to_string(ready) + " à rendre]" : ""),
                view.hint,
                true,
                "quest.locations.category." + std::to_string(view.choice),
                itemData
            );
        }

        const int categoryChoice = TerminalInterface::askMenuChoiceFromOptions(
            categoryScreen,
            "Choisis une section de lieux notables affichée."
        );
        Console::clear();

        if (categoryChoice == 0)
        {
            return;
        }

        const auto selectedCategoryIt = std::find_if(
            categoryViews.begin(),
            categoryViews.end(),
            [&](const LocationCategoryView& view) { return view.choice == categoryChoice; }
        );
        if (selectedCategoryIt == categoryViews.end())
        {
            continue;
        }

        std::vector<LocationEntry>& selectedEntries = *selectedCategoryIt->entries;
        const std::string selectedSection = selectedCategoryIt->title;
        const std::string selectedHint = selectedCategoryIt->hint;
        const bool showingAllLocations = selectedCategoryIt->all;
        const auto locationSectionName = [](LocationCategory category) {
            if (category == LocationCategory::City) return std::string("Ville");
            if (category == LocationCategory::Outside) return std::string("Extérieur");
            return std::string("Boutiques");
        };
        constexpr std::size_t locationsPerPage = 8;
        std::size_t pageIndex = 0;
        bool sectionOpen = true;

        while (sectionOpen)
        {
            const std::size_t totalPages = PagedMenu::pageCount(selectedEntries.size(), locationsPerPage);
            if (pageIndex >= totalPages)
            {
                pageIndex = totalPages == 0 ? 0 : totalPages - 1;
            }

            const std::size_t first = PagedMenu::firstIndex(pageIndex, locationsPerPage);
            const std::size_t last = PagedMenu::lastIndexExclusive(selectedEntries.size(), pageIndex, locationsPerPage);

            MenuScreen screen("LIEUX NOTABLES — " + selectedSection, "quest.locations.category_list");
            screen.setPagination(pageIndex, totalPages);
            screen.addLine(selectedHint);
            screen.addLine("Sélectionne un lieu pour parler au contact associé, consulter ses demandes ou rendre un objectif terminé.");
            screen.addLine("Affichage : " + PagedMenu::rangeText(first, last, selectedEntries.size()));
            screen.addBackOption("Changer de section", "quest.locations.category_list.back");

            for (std::size_t i = first; i < last; ++i)
            {
                const LocationEntry& entry = selectedEntries[i];
                const ClientQuestCounts counts = countQuestsForClient(player, entry.client);
                const std::string entrySection = locationSectionName(entry.category);
                std::string label = (showingAllLocations ? "[" + entrySection + "] " : "") + entry.label + " — " + entry.client;
                if (counts.ready > 0)
                {
                    label += " [" + std::to_string(counts.ready) + " à rendre]";
                }
                else if (counts.active > 0)
                {
                    label += " [" + std::to_string(counts.active) + " en cours]";
                }

                MenuOptionItemData itemData = makeClientQuestNavigationItemData(
                    entry.client,
                    selectedSection,
                    entry.detail,
                    counts
                );
                itemData.kind = "location";
                itemData.section = entrySection;
                itemData.actionType = entry.guild ? "quest" : (entry.vault ? "storage" : (entry.travel ? "travel" : "talk"));
                itemData.name = entry.label;
                itemData.owner = entry.client;
                itemData.progress = "Lieu " + std::to_string(i + 1) + "/" + std::to_string(selectedEntries.size());
                itemData.important = counts.ready > 0;

                screen.addOption(
                    static_cast<int>(i - first + 1),
                    label,
                    entry.detail + " " + clientQuestHintText(counts),
                    true,
                    "quest.locations.category_list.select." + std::to_string(i + 1),
                    itemData
                );
            }

            PagedMenu::addNavigationOptions(screen, pageIndex, totalPages);
            const int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choisis un lieu affiché.");
            Console::clear();

            if (choice == 0)
            {
                sectionOpen = false;
                continue;
            }
            if (choice == 98 && pageIndex > 0)
            {
                --pageIndex;
                continue;
            }
            if (choice == 99 && pageIndex + 1 < totalPages)
            {
                ++pageIndex;
                continue;
            }
            if (choice < 1 || static_cast<std::size_t>(choice) > (last - first))
            {
                MessageScreen::show(
                    "LIEU INDISPONIBLE",
                    "quest.locations.invalid_choice",
                    {
                        "Cette entrée n'existe pas dans la section actuelle.",
                        "Utilise les choix affichés ou les boutons de pagination."
                    }
                );
                continue;
            }

            const LocationEntry& selected = selectedEntries[first + static_cast<std::size_t>(choice - 1)];
            if (selected.guild)
            {
                openGuild(player);
            }
            else if (selected.vault)
            {
                openCityVault(player);
            }
            else if (selected.travel)
            {
                openCityTravelMenu(player);
            }
            else
            {
                talkToClient(player, selected.client);
            }
        }
    }
}

// EN: openNotableNpcMenu declares or implements a focused behavior used by this module.
// FR: openNotableNpcMenu déclare ou implémente un comportement précis utilisé par ce module.
void QuestMenu::openNotableNpcMenu(Player& player)
{
    expireOverdueQuestDeadlines(player, "quest.notable_npcs");
    constexpr std::size_t clientsPerPage = 8;

    using NpcEntry = std::pair<std::string, std::string>;

    while (true)
    {
        syncMainStoryQuests(player);

        std::vector<NpcEntry> storyEntries = {
            {"Mira", "intendante de quartier / référente histoire"},
            {"Orren", "vieux garde / référent de route"},
            {"Lysa", "soigneuse de fortune"},
            {"Bram", "forgeron fatigué"},
            {"Soryn", "archiviste"},
            {"Nell la messagère", "messagère de relais / survivante de route"},
            {"Eda", "comptable des routes courtes / stocks réels"}
        };

        std::vector<NpcEntry> shopEntries = {
            {"Maître de guilde", "guilde / contrats officiels"},
            {"Forgeron", "forge / réparations et équipement"},
            {"Alchimiste", "alchimie / potions"},
            {"Prunigil le marchand", "comptoir / QCM marchand"},
            {"Vendeur de composants", "boutique de composants"},
            {"Vendeur de matériaux", "boutique de matériaux"},
            {"Herboriste", "plantes et remèdes"},
            {"Armurier", "armures et protections"},
            {"Vendeur d'armes", "armes"},
            {"Vendeur de consommables", "consommables"},
            {"Bibliothécaire", "bibliothèque et renseignements"},
            {"Archiviste Meron", "bibliothèque / QCM de connaissances"},
            {"Scribe Ysolde", "inscriptions / paperasse et guilde"},
            {"Maëra l'alchimiste", "laboratoire / alchimie QCM"},
            {"Noro le palefrenier", "relais / transport et routes"},
            {"Tavia l'aubergiste", "auberge / hébergement et services"}
        };

        std::vector<NpcEntry> otherEntries = {
            {"Villageois nerveux", "habitant / événement et rumeurs"},
            {"Marchand inquiet", "habitant / commerce et rumeurs"}
        };

        if (player.hasStoryModeStarted() && !player.hasStorySkip())
        {
            const int chapter = player.getStoryChapter();
            const int step = player.getStoryStep();

            storyEntries.erase(
                std::remove_if(storyEntries.begin(), storyEntries.end(), [&](const NpcEntry& entry)
                {
                    const std::string& name = entry.first;
                    if (name == "Mira") return false;
                    if (name == "Orren" || name == "Lysa" || name == "Bram" || name == "Soryn")
                    {
                        return chapter < 2 && step < 3;
                    }
                    if (name == "Nell la messagère") return chapter < 2 || step < 7;
                    if (name == "Eda") return chapter < 2 || step < 12;
                    return true;
                }),
                storyEntries.end()
            );

            shopEntries.erase(
                std::remove_if(shopEntries.begin(), shopEntries.end(), [&](const NpcEntry& entry)
                {
                    const std::string& name = entry.first;
                    if (name == "Maître de guilde") return false;
                    if (name == "Forgeron" || name == "Alchimiste" || name == "Herboriste")
                    {
                        return chapter < 2 && step < 3;
                    }
                    if (name == "Noro le palefrenier") return chapter < 2 || step < 7;
                    if (chapter >= 2 && step >= 9
                        && (name == "Prunigil le marchand" || name == "Vendeur de composants" || name == "Vendeur de matériaux"
                            || name == "Armurier" || name == "Vendeur d'armes" || name == "Vendeur de consommables"
                            || name == "Scribe Ysolde" || name == "Maëra l'alchimiste" || name == "Tavia l'aubergiste")) return false;
                    if (chapter >= 2 && step >= 10 && name == "Bibliothécaire") return false;
                    if (chapter >= 2 && step >= 12 && name == "Archiviste Meron") return false;
                    return true;
                }),
                shopEntries.end()
            );

            otherEntries.erase(
                std::remove_if(otherEntries.begin(), otherEntries.end(), [&](const NpcEntry& entry)
                {
                    return entry.first == "Marchand inquiet" && chapter < 2;
                }),
                otherEntries.end()
            );
        }

        std::vector<NpcEntry> temporaryEntries;
        const std::vector<std::string> recommendedClients = collectRecommendedClients(player);
        for (const std::string& clientName : recommendedClients)
        {
            temporaryEntries.push_back({clientName, "PNJ de quête temporaire / maximum 5 demandes"});
        }

        auto appendUnique = [](std::vector<NpcEntry>& destination, const std::vector<NpcEntry>& source) {
            for (const NpcEntry& entry : source)
            {
                const bool exists = std::any_of(destination.begin(), destination.end(), [&](const NpcEntry& current) {
                    return current.first == entry.first;
                });
                if (!exists)
                {
                    destination.push_back(entry);
                }
            }
        };

        std::vector<NpcEntry> allEntries;
        appendUnique(allEntries, storyEntries);
        appendUnique(allEntries, shopEntries);
        appendUnique(allEntries, otherEntries);
        appendUnique(allEntries, temporaryEntries);

        auto countReadyForEntries = [&](const std::vector<NpcEntry>& entries) {
            int ready = 0;
            for (const NpcEntry& entry : entries)
            {
                ready += countQuestsForClient(player, entry.first).ready;
            }
            return ready;
        };

        MenuScreen categoryScreen("PNJ NOTABLES", "quest.notable_npc.categories");
        categoryScreen.addSubtitle("Vue complète ou contacts classés par rôle");
        categoryScreen.addLine("Tout afficher regroupe les contacts accessibles pour aller vite. Les catégories restent disponibles pour une recherche plus propre.");
        categoryScreen.addLine("Un PNJ d'histoire existe aussi dans le bac à sable : cette organisation change seulement l'affichage, jamais le monde.");
        categoryScreen.addLine("Les PNJ de quête temporaires proviennent de recommandations et peuvent proposer au maximum 5 demandes par contact.");
        if (player.hasStoryModeStarted() && !player.hasStorySkip())
        {
            categoryScreen.addLine("Mode histoire : seuls les PNJ déjà arrivés ou rencontrés sont affichés. Les futurs contacts n'encombrent pas la liste.");
        }
        categoryScreen.addBackOption("Retour", "quest.notable_npc.categories.back");

        auto addCategoryOption = [&](int number, const std::string& label, const std::string& hint, const std::vector<NpcEntry>& entries, const std::string& id, bool enabled = true) {
            MenuOptionItemData itemData;
            itemData.structured = true;
            itemData.kind = "npc_group";
            itemData.section = "PNJ notables";
            itemData.actionType = "open";
            itemData.name = label;
            itemData.detail = hint;
            itemData.status = std::to_string(entries.size()) + " contact(s)";
            const int ready = countReadyForEntries(entries);
            itemData.progress = ready > 0 ? std::to_string(ready) + " demande(s) à rendre" : "Aucune demande prête";
            itemData.important = ready > 0;
            categoryScreen.addOption(number, label + (ready > 0 ? " [" + std::to_string(ready) + " à rendre]" : ""), hint, enabled, id, itemData);
        };

        addCategoryOption(1, "Tout afficher", "Afficher tous les contacts disponibles dans une seule liste paginée.", allEntries, "quest.notable_npc.category.all");
        addCategoryOption(2, "PNJ d'histoire", "Mira, référents, survivants et contacts liés aux chapitres.", storyEntries, "quest.notable_npc.category.story");
        addCategoryOption(3, "PNJ de boutique et services", "Guilde, forge, alchimie, commerces, bibliothèque, relais et auberge.", shopEntries, "quest.notable_npc.category.shops");
        addCategoryOption(4, "Autres PNJ notables", "Habitants, rumeurs, événements et contacts permanents hors histoire/boutiques.", otherEntries, "quest.notable_npc.category.other");
        addCategoryOption(5, "PNJ de quête temporaires", "Contacts recommandés, limités à cinq demandes chacun.", temporaryEntries, "quest.notable_npc.category.temporary", !temporaryEntries.empty());

        const int categoryChoice = TerminalInterface::askMenuChoiceFromOptions(categoryScreen, "Choisis une catégorie de PNJ.");
        Console::clear();
        if (categoryChoice == 0)
        {
            return;
        }

        const std::vector<NpcEntry>* selectedEntries = nullptr;
        std::string categoryTitle;
        std::string categoryHint;
        const bool showingAllNpcs = categoryChoice == 1;
        if (categoryChoice == 1)
        {
            selectedEntries = &allEntries;
            categoryTitle = "TOUS LES PNJ NOTABLES";
            categoryHint = "Vue complète de tous les contacts actuellement disponibles, regroupés visuellement par rôle dans l'IG.";
        }
        else if (categoryChoice == 2)
        {
            selectedEntries = &storyEntries;
            categoryTitle = "PNJ D'HISTOIRE";
            categoryHint = "Ces personnages restent présents dans le monde et dans le bac à sable, même lorsqu'ils servent aussi la route principale.";
        }
        else if (categoryChoice == 3)
        {
            selectedEntries = &shopEntries;
            categoryTitle = "PNJ DE BOUTIQUE ET SERVICES";
            categoryHint = "Contacts associés à une boutique, un comptoir, la guilde ou un service permanent.";
        }
        else if (categoryChoice == 4)
        {
            selectedEntries = &otherEntries;
            categoryTitle = "AUTRES PNJ NOTABLES";
            categoryHint = "Habitants et contacts permanents qui ne sont ni des référents d'histoire ni des vendeurs.";
        }
        else if (categoryChoice == 5 && !temporaryEntries.empty())
        {
            selectedEntries = &temporaryEntries;
            categoryTitle = "PNJ DE QUÊTE TEMPORAIRES";
            categoryHint = "Contacts obtenus par recommandation. Chacun disparaît de cette catégorie après avoir atteint sa limite de cinq demandes.";
        }
        else
        {
            continue;
        }

        const auto containsNpc = [](const std::vector<NpcEntry>& entries, const std::string& name) {
            return std::any_of(entries.begin(), entries.end(), [&](const NpcEntry& entry) { return entry.first == name; });
        };
        const auto npcSectionName = [&](const std::string& name) {
            if (containsNpc(temporaryEntries, name)) return std::string("PNJ de quête temporaires");
            if (containsNpc(storyEntries, name)) return std::string("PNJ d'histoire");
            if (containsNpc(shopEntries, name)) return std::string("Boutiques et services");
            return std::string("Autres PNJ");
        };

        std::size_t pageIndex = 0;
        bool categoryOpen = true;
        while (categoryOpen)
        {
            const std::size_t totalPages = PagedMenu::pageCount(selectedEntries->size(), clientsPerPage);
            if (pageIndex >= totalPages)
            {
                pageIndex = totalPages == 0 ? 0 : totalPages - 1;
            }

            const std::size_t first = PagedMenu::firstIndex(pageIndex, clientsPerPage);
            const std::size_t last = PagedMenu::lastIndexExclusive(selectedEntries->size(), pageIndex, clientsPerPage);

            MenuScreen screen(categoryTitle, "quest.notable_npc.category_list");
            screen.setPagination(pageIndex, totalPages);
            screen.addLine(categoryHint);
            screen.addLine("Sélectionne un contact pour parler, consulter ses demandes ou rendre ce qui est terminé.");
            screen.addLine("Affichage : " + PagedMenu::rangeText(first, last, selectedEntries->size()));
            screen.addBackOption("Changer de catégorie", "quest.notable_npc.category_list.back");

            for (std::size_t i = first; i < last; ++i)
            {
                const NpcEntry& entry = (*selectedEntries)[i];
                const ClientQuestCounts counts = countQuestsForClient(player, entry.first);
                const std::string entrySection = npcSectionName(entry.first);
                std::string label = (showingAllNpcs ? "[" + entrySection + "] " : "") + entry.first + " (" + entry.second + ")";
                if (counts.ready > 0)
                {
                    label += " [" + std::to_string(counts.ready) + " à rendre]";
                }
                else if (counts.active > 0)
                {
                    label += " [" + std::to_string(counts.active) + " en cours]";
                }

                MenuOptionItemData itemData = makeClientQuestNavigationItemData(entry.first, showingAllNpcs ? entrySection : categoryTitle, entry.second, counts);
                itemData.section = showingAllNpcs ? entrySection : categoryTitle;
                itemData.status = entry.first == "Maître de guilde" ? "Contrats officiels / panneau de guilde" : clientQuestStatusText(counts);
                itemData.actionType = entry.first == "Maître de guilde" ? "quest" : itemData.actionType;
                itemData.progress = "Contact " + std::to_string(i + 1) + "/" + std::to_string(selectedEntries->size());
                itemData.important = counts.ready > 0;

                screen.addOption(
                    static_cast<int>(i - first + 1),
                    label,
                    entry.first == "Maître de guilde" ? "Ouvrir le panneau officiel de guilde." : "Parler, consulter ou rendre une demande auprès de ce contact.",
                    true,
                    "quest.notable_npc.category_list.select." + std::to_string(i + 1),
                    itemData
                );
            }

            PagedMenu::addNavigationOptions(screen, pageIndex, totalPages);
            const int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choix invalide.");
            Console::clear();

            if (choice == 0)
            {
                categoryOpen = false;
                continue;
            }
            if (choice == 98 && pageIndex > 0)
            {
                --pageIndex;
                continue;
            }
            if (choice == 99 && pageIndex + 1 < totalPages)
            {
                ++pageIndex;
                continue;
            }
            if (choice < 1 || static_cast<std::size_t>(choice) > (last - first))
            {
                MessageScreen::show(
                    "CONTACT INDISPONIBLE",
                    "quest.notable_npc.invalid_choice",
                    {"Cette entrée n'existe pas sur la page actuelle.", "Utilise les choix affichés ou les boutons de pagination."}
                );
                continue;
            }

            const std::string selectedClient = (*selectedEntries)[first + static_cast<std::size_t>(choice - 1)].first;
            if (selectedClient == "Maître de guilde")
            {
                openGuild(player);
            }
            else
            {
                talkToClient(player, selectedClient);
            }
        }
    }
}

// EN: talkToClient declares or implements a focused behavior used by this module.
// FR: talkToClient déclare ou implémente un comportement précis utilisé par ce module.
void QuestMenu::talkToClient(Player& player, const std::string& clientName)
{
    while (true)
    {
        syncMainStoryQuests(player);
        const ClientQuestCounts counts = countQuestsForClient(player, clientName);
        const bool storyReferentReferral = hasStoryReferentReferral(player, clientName);
        const bool canAcceptLocalRequest = player.hasTitle("Aventurier") || storyReferentReferral;
        MenuScreen screen(clientName, "quest.client");
        addClientQuestSummaryLines(screen, player, clientName);
        if (!canAcceptLocalRequest)
        {
            screen.addLine("Statut : inscription Aventurier requise pour accepter de nouvelles demandes de boutique ou de PNJ.");
        }
        else if (storyReferentReferral && !player.hasTitle("Aventurier"))
        {
            screen.addLine("Statut : Mira t'a présenté à ce référent. Ses premières demandes peuvent être confiées même avant l'inscription complète.");
        }
        screen.addBackOption("Retour", "quest.client.back");

        MenuOptionItemData talkData = makeClientQuestNavigationItemData(
            clientName,
            "Contact",
            "Demander si ce contact a quelque chose à confier.",
            counts
        );
        talkData.actionType = "talk";
        talkData.status = !canAcceptLocalRequest
            ? "Inscription requise"
            : (storyReferentReferral ? "Référent présenté par Mira" : (isRecommendedClientName(clientName) ? "Contact recommandé - demandes limitées" : "Pourparler possible"));

        MenuOptionItemData overviewData = makeClientQuestNavigationItemData(
            clientName,
            "Contact",
            "Consulter les demandes connues de ce contact.",
            counts
        );
        overviewData.actionType = "inspect";
        overviewData.status = counts.total > 0 ? clientQuestStatusText(counts) : "Aucune demande connue";
        overviewData.important = counts.ready > 0;

        const int activePersonalServiceCount = countActivePersonalServiceQuestsForClient(player, clientName);

        MenuOptionItemData turnInData = makeClientQuestNavigationItemData(
            clientName,
            "Contact",
            "Valider une demande terminée auprès de ce contact.",
            counts
        );
        turnInData.actionType = "turn_in";
        turnInData.status = counts.ready > 0
            ? std::to_string(counts.ready) + " demande(s) prête(s)"
            : "Aucune demande prête";
        turnInData.important = counts.ready > 0;

        screen.addOption(
            1,
            "Parler",
            canAcceptLocalRequest ? "Demander si ce contact a quelque chose à confier." : "Inscription Aventurier requise pour accepter une nouvelle demande.",
            canAcceptLocalRequest,
            "quest.client.talk",
            talkData
        );
        screen.addOption(
            2,
            "Consulter les demandes de ce contact",
            counts.total > 0
                ? "Voir les demandes connues, avec inspection fiable pour la guilde ou estimation vague pour les PNJ."
                : "Aucune demande connue pour ce contact.",
            counts.total > 0,
            "quest.client.overview",
            overviewData
        );
        screen.addOption(
            3,
            "Rendre une demande terminée" + (counts.ready > 0 ? " [" + std::to_string(counts.ready) + "]" : ""),
            counts.ready > 0
                ? "Valider une demande prête auprès de ce contact."
                : "Aucune demande prête à rendre ici.",
            counts.ready > 0,
            "quest.client.turn_in",
            turnInData
        );

        MenuOptionItemData serviceData = makeClientQuestNavigationItemData(
            clientName,
            "Contact",
            "Traiter une demande de service/question directement avec ce PNJ.",
            counts
        );
        serviceData.actionType = "service";
        serviceData.status = activePersonalServiceCount > 0
            ? std::to_string(activePersonalServiceCount) + " service(s) à traiter"
            : "Aucun service actif";
        serviceData.important = activePersonalServiceCount > 0;

        screen.addOption(
            4,
            "Traiter une demande avec ce PNJ" + (activePersonalServiceCount > 0 ? " [" + std::to_string(activePersonalServiceCount) + "]" : ""),
            activePersonalServiceCount > 0
                ? "Résoudre une étape de service/QCM directement avec le contact."
                : "Aucune demande de service active avec ce contact.",
            activePersonalServiceCount > 0,
            "quest.client.service",
            serviceData
        );

        int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choix invalide.");
        Console::clear();

        if (choice == 0)
        {
            return;
        }

        if (choice == 2)
        {
            showClientQuestOverview(player, clientName);
            continue;
        }

        if (choice == 3)
        {
            completeQuestAtClient(player, clientName);
            continue;
        }

        if (choice == 4)
        {
            processPersonalServiceAtClient(player, clientName);
            continue;
        }

        if (handleStoryReferentMainQuestDialogue(player, clientName))
        {
            continue;
        }

        if (!canAcceptLocalRequest)
        {
            MessageScreen::show(
                "INSCRIPTION REQUISE",
                "quest.client.registration_required",
                {
                    clientName + " refuse de transformer la discussion en vraie demande tant que tu n'as pas de carte de guilde.",
                    "Va à la guilde et passe l'inscription Aventurier pour accéder aux demandes des vendeurs et contacts."
                },
                false
            );
            continue;
        }

        std::vector<std::string> introLines;
        std::vector<std::string> ambientLines = clientAmbientDialogueLines(player, clientName, counts);
        introLines.insert(introLines.end(), ambientLines.begin(), ambientLines.end());

        if (isRecommendedClientName(clientName))
        {
            int usedRequests = player.getQuestLog().getClientQuestCount(clientName);
            if (usedRequests >= 5)
            {
                MessageScreen::show(
                    "CONTACT ÉPUISÉ",
                    "quest.client.recommended.empty",
                    {
                        clientName + " n'a plus de nouvelles demandes à confier.",
                        "Son nom quitte naturellement la liste des contacts recommandés."
                    }
                );
                return;
            }

            introLines.push_back("Demandes confiées par ce contact : " + std::to_string(usedRequests) + "/5.");
        }

        Quest offeredQuest;
        Random questRandom;

        if (clientName == "Mira")
        {
            introLines.push_back("Mira ne transforme pas la ville en liste de courses. Elle te confie une priorité claire, proche des murs.");
            introLines.push_back("[Objectif de quête probable] Rapporter une preuve de terrain ou une ressource utile à la stabilité du quartier.");
            offeredQuest = QuestCatalog::createBiomeRequest(player.getLevel(), randomBiomeForClient(questRandom, clientName), clientName);
        }
        else if (clientName == "Orren")
        {
            introLines.push_back("Orren te montre une portion de route qui revient trop souvent dans les récits des gardes.");
            introLines.push_back("[Objectif de quête probable] Vérifier un repère, une borne ou une présence hostile sur une route courte.");
            offeredQuest = QuestCatalog::createBiomeRequest(player.getLevel(), "Route commerciale", clientName);
            if (player.hasStoryModeStarted() && player.getStoryChapter() >= 3
                && !questExistsInAnyState(player, "story_ch3_side_displaced_travelers"))
            {
                offeredQuest.id = "story_ch3_side_displaced_travelers";
                offeredQuest.title = "Les voyageurs revenus au mauvais endroit";
                offeredQuest.origin = "Quête secondaire d'histoire";
                offeredQuest.location = "Route commerciale / haltes contradictoires";
                offeredQuest.objective = "Retrouver des voyageurs revenus par une halte différente de celle qu'ils ont quittée, puis comparer leurs repères sans leur imposer une version des faits.";
                offeredQuest.objectiveType = "exploration";
                offeredQuest.targetFamily = "Voyageurs / route variable";
                offeredQuest.target = std::max(2, offeredQuest.target);
                offeredQuest.stageLabels = "Retrouver une première trace de retour|Comparer le récit avec une seconde halte";
                offeredQuest.hideFutureSteps = true;
            }
        }
        else if (clientName == "Lysa")
        {
            introLines.push_back("Lysa ne demande pas un miracle, seulement de quoi tenir jusqu'à la prochaine nuit.");
            introLines.push_back("[Objectif de quête probable] Trouver des plantes, signes de symptômes ou composants de soin simples.");
            offeredQuest = QuestCatalog::createBiomeRequest(player.getLevel(), randomBiomeForClient(questRandom, clientName), clientName);
            if (StoryCampaign::getChapterThreeRouteChoice(player) == "secours"
                && !questExistsInAnyState(player, "story_ch3_choice_rescue_triage"))
            {
                offeredQuest.id = "story_ch3_choice_rescue_triage";
                offeredQuest.title = "Ceux que la route rend sans blessure";
                offeredQuest.origin = "Conséquence du chapitre 3 — route de secours";
                offeredQuest.location = "Route commerciale / poste de soin de Lysa";
                offeredQuest.objective = "Retrouver des voyageurs revenus épuisés sans plaie visible, noter leurs symptômes puis rapporter des plantes propres pour un triage réel.";
                offeredQuest.objectiveType = "exploration / soins";
                offeredQuest.targetFamily = "Voyageurs altérés / plantes de soin";
                offeredQuest.target = std::max(3, offeredQuest.target);
                offeredQuest.stageLabels = "Identifier les symptômes communs|Rapporter des plantes intactes|Comparer les heures de retour";
                offeredQuest.hideFutureSteps = true;
            }
        }
        else if (clientName == "Bram")
        {
            introLines.push_back("Bram désigne les fissures de son enclume avant de parler des murs.");
            introLines.push_back("[Objectif de quête probable] Rapporter métal, cuir, outils ou pièces exploitables pour les réparations.");
            offeredQuest = QuestCatalog::createBiomeRequest(player.getLevel(), randomBiomeForClient(questRandom, clientName), clientName);
            if (StoryCampaign::getChapterThreeRouteChoice(player) == "commerce"
                && !questExistsInAnyState(player, "story_ch3_choice_commerce_reinforcement"))
            {
                offeredQuest.id = "story_ch3_choice_commerce_reinforcement";
                offeredQuest.title = "Le métal revenu trop neuf";
                offeredQuest.origin = "Conséquence du chapitre 3 — route commerciale";
                offeredQuest.location = "Route commerciale / forge de Bram";
                offeredQuest.objective = "Récupérer des pièces de renfort sur les convois admis, puis comparer leur usure avec les registres de départ avant de les intégrer aux murs.";
                offeredQuest.objectiveType = "récolte / vérification";
                offeredQuest.targetFamily = "Métal de convoi / renforts suspects";
                offeredQuest.target = std::max(3, offeredQuest.target);
                offeredQuest.stageLabels = "Récupérer un premier lot|Comparer les marques de forge|Valider les pièces réellement sûres";
                offeredQuest.hideFutureSteps = true;
            }
        }
        else if (clientName == "Soryn")
        {
            introLines.push_back("Soryn accepte de rouvrir une page, mais seulement si le terrain confirme que ce n'est pas une rumeur de plus.");
            introLines.push_back("[Objectif de quête probable] Vérifier une trace, une archive ou un indice avant d'en faire une légende.");
            offeredQuest = QuestCatalog::createBiomeRequest(player.getLevel(), randomBiomeForClient(questRandom, clientName), clientName);
            const std::string routeChoice = StoryCampaign::getChapterThreeRouteChoice(player);
            const std::string convoyDecision = StoryCampaign::getChapterThreeConvoyDecision(player);
            if ((routeChoice == "recherche" || convoyDecision == "preuves")
                && !questExistsInAnyState(player, "story_ch3_choice_research_contradictions"))
            {
                offeredQuest.id = "story_ch3_choice_research_contradictions";
                offeredQuest.title = "Les pages qui décrivent le lendemain";
                offeredQuest.origin = "Conséquence du chapitre 3 — recherche et preuves";
                offeredQuest.location = "Archives / route corrigée";
                offeredQuest.objective = "Comparer des notes du convoi avec des traces encore présentes sur la route, puis isoler les phrases écrites avant que les faits ne se produisent.";
                offeredQuest.objectiveType = "enquête / exploration";
                offeredQuest.targetFamily = "Archives contradictoires / traces temporelles";
                offeredQuest.target = std::max(3, offeredQuest.target);
                offeredQuest.stageLabels = "Identifier une première contradiction|Retrouver sa trace sur le terrain|Classer ce qui était écrit trop tôt";
                offeredQuest.hideFutureSteps = true;
            }
        }
        else if (clientName == "Nell la messagère")
        {
            introLines.push_back("Nell garde sa sacoche contre elle comme si les routes pouvaient encore essayer de la reprendre.");
            introLines.push_back("[Objectif de quête probable] Protéger une livraison courte, confirmer un passage ou escorter un message entre deux relais.");
            offeredQuest = QuestCatalog::createBiomeRequest(player.getLevel(), "Route commerciale", clientName);
            if (player.hasStoryModeStarted() && player.getStoryChapter() >= 3
                && !questExistsInAnyState(player, "story_ch3_side_returning_markers"))
            {
                offeredQuest.id = "story_ch3_side_returning_markers";
                offeredQuest.title = "Les balises qui reviennent seules";
                offeredQuest.origin = "Quête secondaire d'histoire";
                offeredQuest.location = "Route commerciale / bornes mobiles";
                offeredQuest.objective = "Récupérer deux balises revenues au relais sans leurs messagers et noter précisément la boue, l'heure et le sens de chaque retour.";
                offeredQuest.objectiveType = "exploration à étapes";
                offeredQuest.targetFamily = "Balises / route variable";
                offeredQuest.target = 2;
                offeredQuest.stageLabels = "Première balise revenue seule|Seconde balise et comparaison des traces";
                offeredQuest.hideFutureSteps = true;
            }
        }
        else if (clientName == "Eda")
        {
            introLines.push_back("Eda refuse les cartes jolies si aucun stock réel n'est revenu pour les confirmer.");
            introLines.push_back("[Objectif de quête probable] Vérifier un retour de route courte, aider un comptoir ou confirmer une livraison pendant les réparations.");
            offeredQuest = QuestCatalog::createTransportLogisticsQuestionRequest(player.getLevel());
            offeredQuest.client = clientName;
            if (player.hasStoryModeStarted() && player.getStoryChapter() >= 3
                && !questExistsInAnyState(player, "story_ch3_side_double_weight_convoy"))
            {
                offeredQuest.id = "story_ch3_side_double_weight_convoy";
                offeredQuest.title = "Le convoi qui pèse deux fois";
                offeredQuest.origin = "Quête secondaire d'histoire";
                offeredQuest.location = "Comptoir d'Eda / convoi revenu";
                offeredQuest.objective = "Comparer le poids déclaré, le poids réellement reçu et les caisses ajoutées pendant le trajet afin de repérer une cargaison qui n'appartient à aucun départ connu.";
                offeredQuest.objectiveType = "logistique / enquête";
                offeredQuest.targetFamily = "Convoi / poids contradictoire";
            }
            else if (StoryCampaign::getChapterThreeConvoyDecision(player) == "quarantaine"
                && !questExistsInAnyState(player, "story_ch3_choice_quarantine_inventory"))
            {
                offeredQuest.id = "story_ch3_choice_quarantine_inventory";
                offeredQuest.title = "L'inventaire derrière les barrières";
                offeredQuest.origin = "Conséquence du chapitre 3 — quarantaine";
                offeredQuest.location = "Zone de quarantaine / comptoir d'Eda";
                offeredQuest.objective = "Établir un inventaire séparé des caisses isolées, relever les changements de poids et signaler tout objet apparu sans ouverture visible.";
                offeredQuest.objectiveType = "logistique / observation";
                offeredQuest.targetFamily = "Cargaison isolée / anomalies de stock";
                offeredQuest.target = std::max(3, offeredQuest.target);
                offeredQuest.stageLabels = "Numéroter les caisses intactes|Relever les écarts de poids|Confirmer l'inventaire après une nuit";
                offeredQuest.hideFutureSteps = true;
            }
        }
        else if (clientName == "Hero Villager")
        {
            const int variant = questRandom.between(1, 8);
            introLines.push_back("Hmmm... Le marchand croise les bras. Son armure de diamant bleu ne produit aucun bruit... Huuuh.");
            introLines.push_back("Il ne propose jamais de petite course : seulement une condition de combat qu'il considère digne d'être observée.");

            offeredQuest.id = "hero_villager_challenge_" + std::to_string(player.getWorldDaysElapsed()) + "_" + std::to_string(variant);
            offeredQuest.rank = variant >= 7 ? "A" : (variant >= 3 ? "B" : "C");
            offeredQuest.origin = "Défi du Hero Villager";
            offeredQuest.client = "Hero Villager";
            offeredQuest.location = "Terrain de combat / apparition imprévisible";
            offeredQuest.objectiveType = "challenge";
            offeredQuest.targetFamily = "Défi héroïque";
            offeredQuest.target = 1;
            offeredQuest.rewardExperience = 45 + player.getLevel() * 4;
            offeredQuest.rewardGold = 2 + std::min(4, player.getLevel() / 5);
            offeredQuest.rewardMaterialId = "guild_challenge_mark";
            offeredQuest.rewardMaterialName = "Marque de défi";
            offeredQuest.rewardMaterialQuantity = variant >= 7 ? 3 : (variant >= 3 ? 2 : 1);
            offeredQuest.rewardNote = "Le Hero Villager valide lui-même l'exploit dès qu'il le voit accompli.";

            if (variant == 1)
            {
                offeredQuest.title = "Le boss sans sac de secours";
                offeredQuest.objective = "Vaincre un boss sans utiliser de consommable.";
                offeredQuest.challengeCondition = "boss_no_consumable";
            }
            else if (variant == 2)
            {
                offeredQuest.title = "Une élite, aucune fiole";
                offeredQuest.objective = "Vaincre une créature élite ou un mini-boss sans utiliser de consommable.";
                offeredQuest.challengeCondition = "elite_no_consumable";
            }
            else if (variant == 3)
            {
                offeredQuest.title = "Seulement toi et le boss";
                offeredQuest.objective = "Vaincre un boss sans consommable, compétence de classe ni technique d'arme.";
                offeredQuest.challengeCondition = "boss_no_consumable_skill";
            }
            else if (variant == 4)
            {
                offeredQuest.title = "La méthode la plus ancienne";
                offeredQuest.objective = "Remporter un combat en n'utilisant que des attaques simples. Défendre et attendre restent permis.";
                offeredQuest.challengeCondition = "basic_only_victory";
            }
            else if (variant == 5)
            {
                offeredQuest.title = "Personne derrière";
                offeredQuest.objective = "Remporter un combat de groupe avec tous les aventuriers encore debout à la fin.";
                offeredQuest.challengeCondition = "group_all_survive";
            }
            else if (variant == 6)
            {
                offeredQuest.title = "Même les invoqués comptent";
                offeredQuest.objective = "Remporter un combat où une invocation alliée agit et inflige réellement des dégâts.";
                offeredQuest.challengeCondition = "summon_support_victory";
            }
            else if (variant == 7)
            {
                offeredQuest.title = "Le quatrième problème";
                offeredQuest.objective = "Remporter un combat en portant au moins trois malédictions actives.";
                offeredQuest.challengeCondition = "triple_curse_victory";
            }
            else
            {
                offeredQuest.title = "Pas une égratignure";
                offeredQuest.objective = "Remporter un combat sans subir le moindre dégât.";
                offeredQuest.challengeCondition = "no_damage_victory";
            }
        }
        else if (clientName == "Bob et Maurice")
        {
            introLines.push_back("Bob : Hannnn... hummm... huuuhhhhh.");
            introLines.push_back("Maurice : « Mon collègue Bob a dit que plusieurs clients veulent récupérer leurs caisses, mais que la route est devenue franchement mauvaise. »");
            introLines.push_back("Maurice : Hammmm... hannn.");
            introLines.push_back("Bob : « Maurice demande si tu peux les protéger. Il précise qu'on se battra aussi. Enfin... on fera un dégât. Chacun. Peut-être. »");

            offeredQuest.id = "bob_maurice_protection_" + std::to_string(player.getWorldDaysElapsed());
            offeredQuest.rank = player.getLevel() >= 8 ? "C" : "D";
            offeredQuest.title = "Deux vendeurs à protéger coûte que coûte";
            offeredQuest.origin = "Demande de vendeurs temporaires";
            offeredQuest.client = "Bob et Maurice";
            offeredQuest.location = "Route commerciale / prochain combat PvE";
            offeredQuest.objective = "Remporter un combat de protection avec Bob et Maurice comme alliés. Ils infligent chacun 1 dégât, mais leurs objets peuvent produire un bon ou un mauvais effet.";
            offeredQuest.objectiveType = "combat";
            offeredQuest.targetFamily = "Générale";
            offeredQuest.target = 1;
            offeredQuest.rewardExperience = 24 + player.getLevel() * 3;
            offeredQuest.rewardGold = 2;
            offeredQuest.rewardMaterialId = "guild_challenge_mark";
            offeredQuest.rewardMaterialName = "Marque de défi";
            offeredQuest.rewardMaterialQuantity = 1;
            offeredQuest.rewardNote = "Récompense volontairement modeste : le duo considère déjà sa propre présence comme un avantage majeur.";
        }
        else if (clientName == "Prunigil le marchand")
        {
            const int merchantTrust = prunigilTrustScore(player);
            introLines.push_back("Prunigil ne te donne pas une fiche de guilde : il te fait travailler directement au comptoir.");
            introLines.push_back("Confiance de comptoir : " + prunigilTrustRankLabel(merchantTrust) + " (" + std::to_string(merchantTrust) + " point(s)).");
            introLines.push_back(prunigilNextMilestoneLine(merchantTrust));
            introLines.push_back("[Objectif de quête probable] Répondre à ses QCM de calcul/français quand tu reviens lui parler.");
            offeredQuest = QuestCatalog::createMerchantQuestionRequest(player.getLevel());
        }
        else if (clientName == "Archiviste Meron")
        {
            introLines.push_back("L'Archiviste Meron ouvre un classeur rempli de questions, de monstres et de notes à moitié vraies.");
            introLines.push_back("[Objectif de quête probable] Répondre à un QCM de connaissances directement à la bibliothèque.");
            offeredQuest = QuestCatalog::createLibrarianKnowledgeQuestionRequest(player.getLevel());
        }
        else if (clientName == "Scribe Ysolde")
        {
            introLines.push_back("Scribe Ysolde te montre une pile de fiches d'inscription et de litiges beaucoup trop haute.");
            introLines.push_back("[Objectif de quête probable] Traiter une étape de paperasse avec logique, calcul ou français.");
            offeredQuest = QuestCatalog::createAdministrativePaperworkRequest(player.getLevel());
        }
        else if (clientName == "Maëra l'alchimiste")
        {
            introLines.push_back("Maëra l'alchimiste te sourit en tenant deux fioles qui ne devraient probablement pas être proches.");
            introLines.push_back("[Objectif de quête probable] Vérifier dosages, étiquettes et sécurité au laboratoire.");
            offeredQuest = QuestCatalog::createAlchemistFormulaQuestionRequest(player.getLevel());
        }
        else if (clientName == "Noro le palefrenier")
        {
            introLines.push_back("Noro gratte la crinière d'un cheval qui semble mieux comprendre les routes que certains clients.");
            introLines.push_back("[Objectif de quête probable] Résoudre une demande de transport, pass, chargement ou caravane.");
            offeredQuest = QuestCatalog::createTransportLogisticsQuestionRequest(player.getLevel());
        }
        else if (clientName == "Tavia l'aubergiste")
        {
            introLines.push_back("Tavia l'aubergiste pose une addition, une plainte et trois objets oubliés sur le comptoir.");
            introLines.push_back("[Objectif de quête probable] Régler une épreuve d'auberge ou de service de ville.");
            offeredQuest = QuestCatalog::createInnkeeperServiceQuestionRequest(player.getLevel());
        }
        else if (clientName == "Forgeron")
        {
            introLines.push_back("Le forgeron essuie ses mains noircies et te jauge du regard.");
            offeredQuest = QuestCatalog::createForgemasterMaterialRequest(player.getLevel());
        }
        else if (clientName == "Alchimiste")
        {
            introLines.push_back("L'alchimiste sourit comme si son idée allait forcément exploser.");
            offeredQuest = QuestCatalog::createAlchemistIngredientRequest(player.getLevel());
        }
        else if (clientName == "Villageois nerveux")
        {
            introLines.push_back("Le villageois te rattrape presque en courant.");
            offeredQuest = QuestCatalog::createVillagerMonsterFearRequest(player.getLevel());
        }
        else if (clientName == "Marchand inquiet")
        {
            introLines.push_back("Le marchand tient une caisse vide et un sourire beaucoup trop forcé.");
            offeredQuest = QuestCatalog::createMerchantDeliveryRequest(player.getLevel());
        }
        else if (clientName == "Vendeur de composants")
        {
            introLines.push_back("Le vendeur aligne des bocaux pas vraiment rassurants.");
            offeredQuest = QuestCatalog::createMonsterMaterialVendorRequest(player.getLevel());
        }
        else if (clientName == "Vendeur de matériaux")
        {
            introLines.push_back("Le vendeur tapote une étagère presque vide.");
            offeredQuest = QuestCatalog::createMaterialVendorRequest(player.getLevel());
        }
        else if (clientName == "Herboriste")
        {
            introLines.push_back("L'herboriste trie des feuilles avec une précision maniaque.");
            offeredQuest = QuestCatalog::createHerbalistRequest(player.getLevel());
        }
        else if (clientName == "Armurier")
        {
            introLines.push_back("L'armurier soupire devant une pile de protections abîmées.");
            offeredQuest = QuestCatalog::createArmorerRequest(player.getLevel());
        }
        else if (clientName == "Vendeur d'armes")
        {
            introLines.push_back("Le vendeur d'armes vérifie ses lames une par une.");
            offeredQuest = QuestCatalog::createWeaponVendorRequest(player.getLevel());
        }
        else if (clientName == "Vendeur de consommables")
        {
            introLines.push_back("Le vendeur de consommables recompte ses flacons avec inquiétude.");
            offeredQuest = QuestCatalog::createConsumableVendorRequest(player.getLevel());
        }
        else if (questRandom.between(1, 100) <= 70)
        {
            std::string targetedBiome = randomBiomeForClient(questRandom, clientName);
            introLines.push_back(clientName + " n'a rien de totalement officiel à confier pour le moment.");
            introLines.push_back("Cette fois, il parle surtout d'une zone précise : " + targetedBiome + " [objectif de quête probable].");
            offeredQuest = QuestCatalog::createBiomeRequest(player.getLevel(), targetedBiome, clientName);
        }
        else if (isRecommendedClientName(clientName))
        {
            introLines.push_back(clientName + " t'accueille grâce à une recommandation griffonnée sur un billet.");
            introLines.push_back("Ce contact n'a pas encore pignon sur rue, mais il a déjà une demande précise.");
            offeredQuest = QuestCatalog::createBiomeRequest(player.getLevel(), randomBiomeForClient(questRandom, clientName), clientName);
        }
        else
        {
            introLines.push_back("La bibliothécaire te montre des notes incomplètes.");
            offeredQuest = QuestCatalog::createLibrarianRequest(player.getLevel());
        }

        if (!player.getQuestLog().canAcceptPersonalQuestForClient(offeredQuest.client))
        {
            MessageScreen::show(
                "DEMANDES EN ATTENTE",
                "quest.client.offer.blocked",
                {
                    offeredQuest.client + " a déjà deux demandes actives dans ton journal.",
                    "Il préfère attendre que tu lui rendes au moins une demande avant d'en confier une autre.",
                    "Conseil : consulte ce contact puis rends une demande terminée si elle est prête."
                }
            );
            continue;
        }

        int accept = askQuestOfferDecision("DEMANDE DE CLIENT", "quest.client.offer", player, offeredQuest, introLines);
        Console::clear();

        if (accept == 1)
        {
            Quest acceptedQuest = offeredQuest;
            prepareQuestForAcceptance(acceptedQuest, player.getWorldDaysElapsed());

            if (player.getQuestLog().addQuest(acceptedQuest))
            {
                player.getQuestLog().refreshMaterialDeliveryQuests(player.getInventory());
                std::vector<std::string> lines = {"Demande acceptée : " + acceptedQuest.title};
                std::vector<std::string> dialogue = clientQuestAcceptedDialogueLines(player, acceptedQuest);
                lines.insert(lines.end(), dialogue.begin(), dialogue.end());
                appendDeadlineLine(lines, acceptedQuest, player.getWorldDaysElapsed());
                lines.push_back("Journal : cette entrée reste une estimation de pourparler tant qu'elle ne vient pas de la guilde.");
                MessageScreen::show("DEMANDE ACCEPTÉE", "quest.client.offer.accepted", lines);
            }
            else
            {
                MessageScreen::show(
                    "DEMANDE NON AJOUTÉE",
                    "quest.client.offer.failed",
                    {"Cette demande est déjà active ou impossible à ajouter."}
                );
            }
        }
        else
        {
            MessageScreen::show(
                "DEMANDE REFUSÉE",
                "quest.client.offer.declined",
                {"Tu refuses la demande pour l'instant."}
            );
        }
    }
}

// EN: openReadyQuestTurnInMenu declares or implements a focused behavior used by this module.
// FR: openReadyQuestTurnInMenu déclare ou implémente un comportement précis utilisé par ce module.
void QuestMenu::openReadyQuestTurnInMenu(Player& player)
{
    expireOverdueQuestDeadlines(player, "quest.ready_turn_in");
    constexpr std::size_t clientsPerPage = 8;
    std::size_t pageIndex = 0;

    while (true)
    {
        const std::vector<ReadyQuestClientEntry> entries = collectReadyQuestClients(player);
        const std::size_t totalPages = PagedMenu::pageCount(entries.size(), clientsPerPage);
        if (pageIndex >= totalPages)
        {
            pageIndex = totalPages == 0 ? 0 : totalPages - 1;
        }

        const std::size_t first = PagedMenu::firstIndex(pageIndex, clientsPerPage);
        const std::size_t last = PagedMenu::lastIndexExclusive(entries.size(), pageIndex, clientsPerPage);

        MenuScreen screen("QUÊTES PRÊTES À RENDRE", "quest.ready_turn_in");
        screen.addLine("Choisis le contact concerné : la validation se fait auprès de la personne ou de l'organisme qui a confié la demande.");
        screen.addLine("Rappel : la guilde valide des contrats officiels ; les PNJ confirment surtout des pourparlers et services rendus.");
        screen.addLine("Affichage : " + PagedMenu::rangeText(first, last, entries.size()));
        screen.addBackOption("Retour", "quest.ready_turn_in.back");

        if (entries.empty())
        {
            screen.addLine("Aucune quête n'est prête à rendre pour le moment.");
            TerminalInterface::askMenuChoiceFromOptions(screen, "Entre 0 pour revenir.");
            Console::clear();
            return;
        }

        for (std::size_t i = first; i < last; ++i)
        {
            const ReadyQuestClientEntry& entry = entries[i];
            MenuOptionItemData itemData;
            itemData.structured = true;
            itemData.kind = entry.guildReadyCount > 0 && entry.personalReadyCount == 0 ? "quest" : "npc";
            itemData.section = "Quêtes prêtes";
            itemData.actionType = "turn_in";
            itemData.name = entry.clientName;
            itemData.detail = "Première entrée : " + entry.firstTitle;
            itemData.status = readyQuestClientStatusText(entry);
            itemData.reward = entry.firstReward;
            itemData.progress = "Contact " + std::to_string(i + 1) + "/" + std::to_string(entries.size());
            itemData.owner = entry.clientName;
            itemData.important = true;

            std::string label = entry.clientName + " | " + readyQuestClientStatusText(entry);
            if (!entry.firstTitle.empty())
            {
                label += " | Première : " + entry.firstTitle;
            }

            screen.addOption(
                static_cast<int>(10 + (i - first)),
                label,
                entry.guildReadyCount > 0 && entry.personalReadyCount == 0
                    ? "Rendre un contrat officiel auprès de ce contact."
                    : "Rendre une demande ou un service terminé auprès de ce contact.",
                true,
                "quest.ready_turn_in.client." + std::to_string(i + 1),
                itemData
            );
        }

        PagedMenu::addNavigationOptions(screen, pageIndex, totalPages);

        int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choix invalide.");
        Console::clear();

        if (choice == 0)
        {
            return;
        }

        if (choice == 98 && pageIndex > 0)
        {
            --pageIndex;
            continue;
        }

        if (choice == 99 && pageIndex + 1 < totalPages)
        {
            ++pageIndex;
            continue;
        }

        const int localClientIndex = choice - 10;
        if (localClientIndex >= 0 && first + static_cast<std::size_t>(localClientIndex) < last)
        {
            completeQuestAtClient(player, entries[first + static_cast<std::size_t>(localClientIndex)].clientName);
            continue;
        }

        MessageScreen::show(
            "CONTACT INVALIDE",
            "quest.ready_turn_in.invalid",
            {"Ce choix ne correspond à aucun contact ayant une quête prête à rendre."}
        );
    }
}

namespace
{
    int prunigilTrustScore(const Player& player)
    {
        int completed = 0;
        int failed = 0;
        for (const Quest& quest : player.getQuestLog().getQuests())
        {
            if (quest.client != "Prunigil le marchand") continue;
            if (quest.turnedIn) ++completed;
            if (quest.failed) ++failed;
        }
        return std::max(0, completed * 2 - failed * 2);
    }

    std::string prunigilTrustRankLabel(int score)
    {
        if (score >= 20) return "Mandataire du comptoir";
        if (score >= 12) return "Vérificateur de registre";
        if (score >= 6) return "Apprenti de Prunigil";
        if (score >= 2) return "Aide de comptoir";
        return "Client encore à l'essai";
    }

    std::string prunigilNextMilestoneLine(int score)
    {
        if (score < 2) return "Prochain palier : Aide de comptoir à 2 points de confiance.";
        if (score < 6) return "Prochain palier : Apprenti de Prunigil à 6 points, avec une première recommandation.";
        if (score < 12) return "Prochain palier : Vérificateur de registre à 12 points, avec de nouveaux contacts itinérants.";
        if (score < 20) return "Prochain palier : Mandataire du comptoir à 20 points, pour les recommandations les plus rares.";
        return "Palier maximal actuel : Prunigil peut engager ton nom auprès de tous ses contacts connus.";
    }

    void grantPrunigilProgressRewards(Player& player, std::vector<std::string>& lines)
    {
        const int score = prunigilTrustScore(player);

        auto grantMilestone = [&](int requiredScore, const std::string& title, const std::string& reason, bool recommendation)
        {
            if (score < requiredScore || !player.grantTitle(title)) return;
            lines.push_back("Titre obtenu : " + title + ".");
            lines.push_back("  > " + reason);
            if (recommendation)
            {
                player.getInventory().addMaterial(MaterialCatalog::createById("client_recommendation", 1));
                lines.push_back("Prunigil rédige aussi une recommandation de client. Certains vendeurs itinérants pourront désormais te reconnaître.");
            }
        };

        grantMilestone(2, "Aide de comptoir", "Prunigil te confie enfin autre chose que la pile la moins dangereuse.", false);
        grantMilestone(6, "Apprenti de Prunigil", "Trois services propres suffisent pour que ton nom reste dans son registre.", true);
        grantMilestone(12, "Vérificateur de registre", "Les comptes, contrats et documents sensibles passent désormais par toi sans surveillance constante.", true);
        grantMilestone(20, "Mandataire du comptoir", "Prunigil peut te recommander à des vendeurs qui ne restent jamais longtemps en ville.", true);
    }

    Quest buildMerchantFollowUpQuest(
        const Quest& sourceQuest,
        const std::string& suffix,
        const std::string& title,
        const std::string& client,
        const std::string& location,
        const std::string& objective,
        const std::string& objectiveType,
        const std::string& targetFamily,
        int target,
        int currentDay
    )
    {
        Quest followUp;
        followUp.id = "merchant_followup_" + suffix + "_" + sourceQuest.id;
        followUp.rank = sourceQuest.rank.empty() ? "E" : sourceQuest.rank;
        followUp.title = title;
        followUp.origin = "Suite de client";
        followUp.client = client;
        followUp.location = location;
        followUp.objective = objective;
        followUp.objectiveType = objectiveType;
        followUp.targetFamily = targetFamily;
        followUp.rewardExperience = std::max(24, sourceQuest.rewardExperience / 2 + 12);
        followUp.rewardGold = std::max(8, sourceQuest.rewardGold / 2 + 4);
        followUp.rewardNote = "Demande secondaire déclenchée par un document traité au comptoir de Prunigil.";
        followUp.target = std::max(1, target);
        followUp.availableFromDay = currentDay;
        followUp.expiresAtDay = currentDay + 8;
        followUp.accepted = true;
        return followUp;
    }

    bool historyMentions(const Quest& quest, const std::string& fragment)
    {
        return quest.serviceChallengeHistory.find(fragment) != std::string::npos;
    }

    bool maybeAddMerchantFollowUp(Player& player, const Quest& sourceQuest, std::vector<std::string>& lines)
    {
        if (sourceQuest.client != "Prunigil le marchand") return false;

        Quest followUp;
        std::string dialogue;
        const int day = player.getWorldDaysElapsed();

        if (historyMentions(sourceQuest, "mise_jour_de_client") || historyMentions(sourceQuest, "mise_a_jour_de_client"))
        {
            followUp = buildMerchantFollowUpQuest(
                sourceQuest, "updated_order", "La commande qui change encore", "Prunigil le marchand", "Comptoir de Prunigil",
                "Recevoir le client revenu au comptoir, vérifier les deux caisses à remplacer et mettre à jour la commande sans effacer les conditions déjà validées.",
                "service", "Marchand / client / modification de commande", 2, day
            );
            dialogue = "Le client n'a même pas attendu que l'encre sèche. Prunigil te tend le dossier : « Puisque tu as compris sa mise à jour, tu vas aussi la faire respecter. »";
        }
        else if (historyMentions(sourceQuest, "bidon_bleu"))
        {
            followUp = buildMerchantFollowUpQuest(
                sourceQuest, "blue_barrel", "Le bidon qui respire", "Alchimiste", "Atelier d'alchimie",
                "Apporter le rapport du bidon bleu à l'alchimiste, puis déterminer comment isoler son contenu sans respirer une deuxième fois dedans.",
                "service", "Alchimie / sécurité / substance inconnue", 2, day
            );
            dialogue = "Prunigil éloigne le papier du bout des doigts : « Va voir l'alchimiste. Et évite de sentir le bidon pour vérifier. »";
        }
        else if (historyMentions(sourceQuest, "coffre_maudit"))
        {
            followUp = buildMerchantFollowUpQuest(
                sourceQuest, "talking_chest", "Le coffre qui répond", "Prunigil le marchand", "Ruines effondrées",
                "Retrouver l'origine du coffre parlant et rapporter une preuve qu'il s'agit d'une malédiction, d'un mécanisme ou d'un très mauvais plaisantin.",
                "exploration", "Ruines effondrées / coffre maudit", 1, day
            );
            dialogue = "Prunigil ferme la réserve à clef : « Très bien. Puisqu'il parle, demande-lui d'où il vient. Mais ne lui promets rien. »";
        }
        else if (historyMentions(sourceQuest, "demande_urgente") || historyMentions(sourceQuest, "message_press"))
        {
            followUp = buildMerchantFollowUpQuest(
                sourceQuest, "overturned_cart", "Le chariot éventré", "Prunigil le marchand", "Route commerciale",
                "Rejoindre le chariot renversé, sécuriser les survivants et récupérer ce qui peut encore l'être avant l'arrivée des pillards.",
                "exploration", "Route commerciale / chariot renversé", 1, day
            );
            dialogue = "Prunigil cesse immédiatement de plaisanter : « Le papier est corrigé. Maintenant, il faut quelqu'un sur la route. »";
        }
        else if (historyMentions(sourceQuest, "achat_suspect") || historyMentions(sourceQuest, "facture_foir"))
        {
            followUp = buildMerchantFollowUpQuest(
                sourceQuest, "dubious_purchase", "L'objet qui n'existe peut-être pas", "Prunigil le marchand", "Marché sous les ponts",
                "Identifier le vendeur itinérant, comparer son histoire avec les registres de brocante et décider si l'objet est une invention, une arnaque ou une vraie curiosité mal nommée.",
                "service", "Marchand / brocante / provenance douteuse", 2, day
            );
            dialogue = "Prunigil relit le nom de l'objet : « Frigo-froid... soit c'est génial, soit on s'est encore fait avoir. Trouve-moi lequel. »";
        }
        else if (historyMentions(sourceQuest, "entr_e_comptable") || historyMentions(sourceQuest, "note_au_comptable"))
        {
            followUp = buildMerchantFollowUpQuest(
                sourceQuest, "missing_copper", "Les 212 cuivres manquants", "Prunigil le marchand", "Comptoir de Prunigil",
                "Comparer les reçus, les notes de caisse et les paiements différés afin de retrouver l'origine d'un écart de 212 cuivre.",
                "service", "Marchand / enquête / registre comptable", 2, day
            );
            dialogue = "Prunigil sort un second registre, beaucoup plus épais : « Puisque tu tiens à laisser des notes propres, retrouvons maintenant mes 212 cuivres. »";
        }
        else
        {
            int checksum = 0;
            for (unsigned char c : sourceQuest.id) checksum += c;
            if (checksum % 3 != 0) return false;

            followUp = buildMerchantFollowUpQuest(
                sourceQuest, "client_return", "Le client revient avec une autre demande", "Prunigil le marchand", "Comptoir de Prunigil",
                "Recevoir le client concerné, vérifier sa nouvelle demande et déterminer si elle prolonge réellement le premier dossier.",
                "service", "Marchand / client / mise à jour de quête", 1, day
            );
            dialogue = "À peine le dossier rangé, le client revient. Prunigil te lance le nouveau papier : « Bon. Apparemment, ce n'était que la première partie. »";
        }

        if (followUp.id.empty() || player.getQuestLog().hasQuest(followUp.id)) return false;
        if (!player.getQuestLog().addQuest(followUp))
        {
            lines.push_back("Prunigil garde une nouvelle demande de côté : ton journal contient déjà trop de services actifs pour ce contact.");
            return false;
        }

        lines.push_back("");
        lines.push_back("Mise à jour de quête : " + followUp.title + ".");
        lines.push_back(dialogue);
        lines.push_back("Nouvelle demande acceptée automatiquement : " + followUp.objective);
        lines.push_back("Délai : jusqu'au jour " + std::to_string(followUp.expiresAtDay + 1) + ".");
        return true;
    }

    bool maybeAddBobMauriceFollowUp(Player& player, const Quest& sourceQuest, std::vector<std::string>& lines)
    {
        if (sourceQuest.client != "Bob et Maurice") return false;

        Quest followUp;
        followUp.rank = "C";
        followUp.origin = "Suite de Bob et Maurice";
        followUp.client = "Bob et Maurice";
        followUp.location = "Route commerciale / prochain combat PvE";
        followUp.objectiveType = "combat";
        followUp.targetFamily = "Générale";
        followUp.target = 1;
        followUp.rewardExperience = 28 + player.getLevel() * 3;
        followUp.rewardGold = 1;
        followUp.rewardMaterialId = "guild_challenge_mark";
        followUp.rewardMaterialName = "Marque de défi";
        followUp.rewardMaterialQuantity = 1;
        followUp.availableFromDay = player.getWorldDaysElapsed();
        followUp.expiresAtDay = player.getWorldDaysElapsed() + 8;
        followUp.accepted = true;

        std::vector<std::string> dialogue;
        if (sourceQuest.id.rfind("bob_maurice_protection_", 0) == 0)
        {
            followUp.id = "bob_maurice_chain_missing_wheel";
            followUp.title = "Une affaire qui ne roule plus";
            followUp.objective = "Protéger Bob et Maurice pendant qu'ils récupèrent une roue de leur propre comptoir, vendue par erreur à un client armé.";
            followUp.rewardNote = "Première suite automatique du duo : la roue est devenue une affaire commerciale et diplomatique.";
            dialogue = {
                "Bob : Hannnn... hammmm... huuuh.",
                "Maurice : « Mon collègue Bob a dit que la protection était parfaite. Il a aussi vendu une roue de notre chariot pendant le combat. »",
                "Maurice : Hummm... hannn ?",
                "Bob : « Maurice demande si tu peux nous protéger une deuxième fois pendant qu'on la récupère. Il dit aussi que ce n'est pas entièrement sa faute. »"
            };
        }
        else if (sourceQuest.id == "bob_maurice_chain_missing_wheel")
        {
            followUp.id = "bob_maurice_chain_breathing_crate";
            followUp.title = "La caisse refuse d'être vendue";
            followUp.objective = "Escorter une caisse qui respire jusqu'à un lieu isolé et survivre à ce qu'elle attire avant que Bob tente de lui fixer un prix.";
            followUp.rewardNote = "La caisse est traitée comme marchandise jusqu'à preuve du contraire.";
            dialogue = {
                "Maurice : Hammmm... huuuhhhhh.",
                "Bob : « Maurice dit qu'on a retrouvé la roue. Il aimerait maintenant parler de la caisse qui respire derrière toi. »",
                "Bob : Hannnn... hummm.",
                "Maurice : « Mon collègue Bob affirme qu'elle vaut plus cher si elle est vivante. Je demande surtout qu'on l'éloigne du comptoir. »"
            };
        }
        else if (sourceQuest.id == "bob_maurice_chain_breathing_crate")
        {
            followUp.id = "bob_maurice_chain_wrong_customer";
            followUp.title = "Le client qui n'avait rien commandé";
            followUp.objective = "Défendre le duo contre les créatures attirées par un reçu établi au nom d'un client inexistant.";
            followUp.rewardNote = "Le reçu semble pourtant porter une signature récente.";
            dialogue = {
                "Bob : Huuuh... hannnn... hammmm.",
                "Maurice : « Mon collègue Bob dit que la caisse est maintenant calme. Le problème, c'est que son acheteur n'existe pas. »",
                "Maurice : Hannn... hummm ?",
                "Bob : « Maurice demande pourquoi des monstres suivent le reçu. Moi, je demande surtout s'ils comptent payer. »"
            };
        }
        else if (sourceQuest.id == "bob_maurice_chain_wrong_customer")
        {
            followUp.id = "bob_maurice_chain_final_inventory";
            followUp.title = "L'inventaire qui compte trois vendeurs";
            followUp.objective = "Remporter un dernier combat pendant que Bob et Maurice vérifient pourquoi leur registre insiste sur la présence d'un troisième vendeur invisible.";
            followUp.rewardMaterialQuantity = 2;
            followUp.rewardExperience += 18;
            followUp.rewardNote = "Fin de la première chaîne longue du duo et reconnaissance spéciale.";
            dialogue = {
                "Maurice : Hummm... hammmm... huuuh.",
                "Bob : « Maurice dit que le faux client a disparu du reçu. Mais notre inventaire compte toujours trois vendeurs. »",
                "Bob : Hannnn... huuuh ?",
                "Maurice : « Mon collègue Bob demande si tu peux rester pendant qu'on recompte. Personnellement, je préférerais que le troisième vendeur ne réponde pas. »"
            };
        }
        else if (sourceQuest.id == "bob_maurice_chain_final_inventory")
        {
            if (player.grantTitle("Le troisième avis n'était pas demandé"))
            {
                lines.push_back("Titre obtenu : Le troisième avis n'était pas demandé.");
            }
            lines.push_back("Bob : Hannnn... hummm... huuuhhhhh.");
            lines.push_back("Maurice : « Mon collègue Bob dit que le troisième vendeur a quitté le registre. Il refuse de préciser où il est allé. »");
            lines.push_back("La première longue affaire de Bob et Maurice est terminée. Ils pourront revenir avec d'autres demandes plus tard.");
            return true;
        }
        else
        {
            return false;
        }

        if (player.getQuestLog().hasQuest(followUp.id)) return false;
        if (!player.getQuestLog().addQuest(followUp))
        {
            lines.push_back("Bob et Maurice gardent leur nouvelle demande de côté : leur dossier existe déjà ou ton journal refuse le doublon.");
            return false;
        }

        lines.push_back("");
        lines.push_back("Mise à jour de quête : " + followUp.title + ".");
        lines.insert(lines.end(), dialogue.begin(), dialogue.end());
        lines.push_back("Nouvelle demande acceptée automatiquement : " + followUp.objective);
        lines.push_back("Délai : jusqu'au jour " + std::to_string(followUp.expiresAtDay + 1) + ".");
        return true;
    }

    void openChallengeMarkCounter(Player& player)
    {
        while (true)
        {
            const int marks = player.getInventory().countMaterialById("guild_challenge_mark");
            MenuScreen screen("COMPTOIR DES MARQUES", "quest.guild.challenge_marks");
            screen.addLine("Marques de défi possédées : " + std::to_string(marks) + ".");
            screen.addLine("Ces échanges restent modestes : les marques certifient surtout des exploits, elles ne remplacent pas l'économie normale.");
            screen.addBackOption("Retour aux défis", "quest.guild.challenge_marks.back");
            screen.addOption(1, "Nécessaire de terrain — 2 marques", "Feuille amère de soin x2 et résidu de slime x1.", marks >= 2, "quest.guild.challenge_marks.field_pack");
            screen.addOption(2, "Poussière d'atelier — 3 marques", "Poussière arcanique x1, utile aux recettes et manipulations runiques.", marks >= 3, "quest.guild.challenge_marks.arcane_dust");
            screen.addOption(
                3,
                "Nouveau tirage du panneau — 3 marques",
                player.isExplorationSceneOnCooldown("guild_challenge_paid_reroll")
                    ? "Déjà utilisé récemment : encore " + std::to_string(player.getExplorationSceneCooldownRemainingDays("guild_challenge_paid_reroll")) + " jour(s)."
                    : "Remplace les défis encore disponibles. Utilisable une fois tous les sept jours.",
                marks >= 3 && !player.isExplorationSceneOnCooldown("guild_challenge_paid_reroll"),
                "quest.guild.challenge_marks.reroll"
            );
            screen.addOption(
                4,
                "Reconnaissance de porte-marque — 6 marques",
                player.hasTitle("Porte-marque de la guilde") ? "Titre déjà obtenu." : "Débloque uniquement un titre et des réactions de PNJ.",
                marks >= 6 && !player.hasTitle("Porte-marque de la guilde"),
                "quest.guild.challenge_marks.title"
            );

            const int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choix invalide.");
            Console::clear();
            if (choice == 0) return;

            int cost = 0;
            if (choice == 1) cost = 2;
            else if (choice == 2 || choice == 3) cost = 3;
            else if (choice == 4) cost = 6;
            else continue;

            if (player.getInventory().countMaterialById("guild_challenge_mark") < cost)
            {
                MessageScreen::show("MARQUES INSUFFISANTES", "quest.guild.challenge_marks.missing", {"Il te manque des Marques de défi."}, false);
                continue;
            }
            if (!player.getInventory().removeMaterialQuantityByIdFlexible("guild_challenge_mark", cost))
            {
                MessageScreen::show("ÉCHANGE IMPOSSIBLE", "quest.guild.challenge_marks.remove_failed", {"Les marques n'ont pas pu être retirées proprement."}, false);
                continue;
            }

            std::vector<std::string> result;
            if (choice == 1)
            {
                player.getInventory().addMaterial(MaterialCatalog::createById("bitter_healing_leaf", 2));
                player.getInventory().addMaterial(MaterialCatalog::createById("slime_residue", 1));
                result = {"Feuille amère de soin x2.", "Résidu de slime x1.", "Aucun bonus de combat permanent n'est accordé."};
            }
            else if (choice == 2)
            {
                player.getInventory().addMaterial(MaterialCatalog::createById("arcane_dust", 1));
                result = {"Poussière arcanique x1.", "Échange volontairement limité pour ne pas remplacer les boutiques et la récolte."};
            }
            else if (choice == 3)
            {
                player.getQuestLog().forceRefreshGuildChallengeBoard(player.getLevel(), player.getWorldDaysElapsed());
                player.startExplorationSceneCooldown("guild_challenge_paid_reroll", 7);
                result = {"Les défis encore affichés ont été remplacés.", "Les défis déjà acceptés restent inchangés.", "Ce service reviendra dans sept jours."};
            }
            else
            {
                player.grantTitle("Porte-marque de la guilde");
                result = {"Titre obtenu : Porte-marque de la guilde.", "Ce titre n'accorde aucun bonus statistique, mais certains PNJ le reconnaîtront."};
            }

            MessageScreen::show("ÉCHANGE DE MARQUES", "quest.guild.challenge_marks.done", result, false);
        }
    }

    bool maybeTriggerLegendaryMerchantEncounter(
        Player& player,
        Random& random,
        const std::string& biomeName
    )
    {
        const int dayPart = player.getWorldDayProgressUnits();
        const bool civilizedOrRoad = biomeName.find("Route") != std::string::npos
            || biomeName.find("Plaine") != std::string::npos
            || biomeName.find("village") != std::string::npos
            || biomeName.find("Marché") != std::string::npos
            || biomeName.find("Quartier") != std::string::npos;

        // Le Hero Villager refuse les apparitions ordinaires et la nuit complète.
        if (dayPart >= 1 && dayPart <= 3
            && !player.isExplorationSceneOnCooldown("legendary_merchant_hero_villager")
            && random.between(1, 500) == 1)
        {
            const bool firstMeeting = !player.hasTitle("Témoin du marchand bleu");
            std::vector<std::string> lines;
            if (firstMeeting)
            {
                lines.push_back("Une silhouette se tient au milieu du passage sans qu'aucune trace n'annonce son arrivée.");
                lines.push_back("C'est un homme très musclé, vêtu d'un t-shirt bleu-vert et d'un pantalon violet.");
                lines.push_back("Une armure de diamant bleu couvre ses épaules, son torse et ses bras sans sembler ralentir le moindre de ses mouvements.");
                lines.push_back("Il porte plusieurs objets de vente comme si leur poids n'existait pas.");
            }
            else
            {
                lines.push_back("Le marchand à l'armure de diamant bleu est revenu sans bruit, exactement là où il n'était pas une seconde plus tôt.");
            }
            lines.push_back("Hero Villager : « Hmmm... Tu as survécu assez longtemps pour voir mon comptoir. Ne confonds pas cela avec une récompense... Huuuh. »");
            lines.push_back("Sa boutique restera accessible pendant cette journée et la suivante.");
            lines.push_back("Il peut aussi proposer des défis bien plus durs que les contrats ordinaires.");
            showExplorationNotice("UNE LÉGENDE AU BORD DE LA ROUTE", "exploration.legendary_merchant.hero", lines, false);

            player.grantTitle("Témoin du marchand bleu");
            BestiaryRuntimeProgress::recordEncounter(
                "Le marchand bleu qui juge les routes",
                "Légendes / contes",
                "Rumeur confirmée par une rencontre directe avec le Hero Villager."
            );
            player.startExplorationSceneCooldown("legendary_merchant_hero_villager", 35);
            player.recordExplorationEventKey("legendary_merchant_hero_villager");
            return true;
        }

        if (civilizedOrRoad
            && dayPart >= 0 && dayPart <= 3
            && !player.isExplorationSceneOnCooldown("legendary_merchant_bob_maurice")
            && random.between(1, 180) == 1)
        {
            const bool firstMeeting = !player.hasTitle("Les deux du même comptoir");
            std::vector<std::string> lines = {
                firstMeeting
                    ? "Deux vendeurs poussent le même comptoir ambulant, chacun persuadé que l'autre connaît la direction."
                    : "Le comptoir à deux voix réapparaît sur le chemin, toujours poussé par les mêmes vendeurs inséparables.",
                "Bob : Hannnn... hummm... hammmm...",
                "Maurice : « Mon collègue Bob a dit qu'il nous restait exactement le bon nombre de caisses. Il refuse de préciser ce que signifie le bon nombre. »",
                "Maurice : Huuuhhhhh... hannn...",
                "Bob : « Maurice demande si tu veux acheter quelque chose avant qu'il découvre ce que j'ai mis dans les caisses. »",
                "Leur boutique commune restera accessible pendant trois jours.",
                "Les voyageurs racontent qu'ils sont toujours ensemble, même lorsqu'ils essaient de partir dans deux directions opposées."
            };
            showExplorationNotice("BOB ET MAURICE", "exploration.legendary_merchant.duo", lines, false);

            player.grantTitle("Les deux du même comptoir");
            BestiaryRuntimeProgress::recordEncounter(
                "Le comptoir qui ne sait pas se séparer",
                "Légendes / contes",
                "Rumeur confirmée par la rencontre de Bob et Maurice, toujours ensemble."
            );
            player.startExplorationSceneCooldown("legendary_merchant_bob_maurice", 14);
            player.recordExplorationEventKey("legendary_merchant_bob_maurice");
            return true;
        }

        return false;
    }
}

// EN: completeQuestAtClient declares or implements a focused behavior used by this module.
// FR: completeQuestAtClient déclare ou implémente un comportement précis utilisé par ce module.
void QuestMenu::completeQuestAtClient(Player& player, const std::string& clientName)
{
    constexpr std::size_t questsPerPage = 5;
    std::size_t pageIndex = 0;

    while (true)
    {
        std::vector<Quest>& quests = player.getQuestLog().getQuests();
        std::vector<int> readyIndexes;

        for (int i = 0; i < static_cast<int>(quests.size()); ++i)
        {
            if (quests[i].client == clientName && !quests[i].turnedIn && !quests[i].failed && isReadyToTurnIn(player, quests[i]))
            {
                readyIndexes.push_back(i);
            }
        }

        if (readyIndexes.empty())
        {
            MessageScreen::show(
                "AUCUNE QUÊTE À RENDRE",
                "quest.turn_in.empty",
                {
                    "Aucune quête prête à être rendue à " + clientName + ".",
                    clientName == "Maître de guilde"
                        ? "Les contrats officiels doivent être terminés avant d'être tamponnés."
                        : "Pour les demandes PNJ, le journal peut estimer une avancée, mais le contact doit encore confirmer la fin."
                }
            );
            return;
        }

        const std::size_t totalPages = PagedMenu::pageCount(readyIndexes.size(), questsPerPage);
        if (pageIndex >= totalPages)
        {
            pageIndex = totalPages == 0 ? 0 : totalPages - 1;
        }

        const std::size_t first = PagedMenu::firstIndex(pageIndex, questsPerPage);
        const std::size_t last = PagedMenu::lastIndexExclusive(readyIndexes.size(), pageIndex, questsPerPage);

        MenuScreen screen("QUÊTES À RENDRE", "quest.turn_in.list");
        screen.addSubtitle(clientName);
        screen.addLine(clientName == "Maître de guilde"
            ? "Sélectionne le contrat officiel à tamponner auprès de la guilde."
            : "Sélectionne la demande à confirmer auprès de ce contact. Ce n'est pas un tampon officiel, plutôt une validation de parole donnée.");
        screen.addLine("Affichage : " + PagedMenu::rangeText(first, last, readyIndexes.size()));
        screen.addBackOption("Retour", "quest.turn_in.back");

        for (std::size_t i = first; i < last; ++i)
        {
            const Quest& quest = quests[readyIndexes[i]];
            const std::string label = questCardLabel(quest);

            MenuOptionItemData itemData;
            itemData.structured = true;
            itemData.kind = "quest";
            itemData.section = "Quêtes à rendre";
            itemData.actionType = "turn_in";
            itemData.name = quest.title;
            itemData.detail = "";
            itemData.status = quest.guildQuest ? "Prête à tamponner" : "Prête à confirmer";
            itemData.reward = quest.guildQuest ? questRewardText(quest) : approximateQuestRewardText(quest);
            itemData.progress = std::to_string(quest.progress) + "/" + std::to_string(quest.target);
            itemData.owner = quest.client;
            itemData.important = true;

            screen.addOption(
                static_cast<int>(10 + (i - first)),
                label,
                "",
                true,
                "quest.turn_in.select." + std::to_string(i + 1),
                itemData
            );
        }

        PagedMenu::addNavigationOptions(screen, pageIndex, totalPages);

        int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choix invalide.");
        Console::clear();

        if (choice == 0)
        {
            return;
        }

        if (choice == 98 && pageIndex > 0)
        {
            --pageIndex;
            continue;
        }

        if (choice == 99 && pageIndex + 1 < totalPages)
        {
            ++pageIndex;
            continue;
        }

        const int localQuestIndex = choice - 10;
        if (localQuestIndex < 0 || first + static_cast<std::size_t>(localQuestIndex) >= last)
        {
            MessageScreen::show(
                "QUÊTE INVALIDE",
                "quest.turn_in.invalid",
                {"Ce choix ne correspond à aucune quête prête sur cette page."}
            );
            continue;
        }

        Quest& quest = quests[readyIndexes[first + static_cast<std::size_t>(localQuestIndex)]];

        if (!askQuestTurnInConfirmation(player, quest, clientName))
        {
            continue;
        }

        const long long copperBefore = player.getInventory().getTotalCopper();
        const int experienceBefore = player.getExperience();
        const int levelBefore = player.getLevel();

        std::vector<std::string> resultLines;
        resultLines.push_back(quest.guildQuest
            ? "La guilde vérifie le contrat, puis appose son tampon."
            : quest.client + " confirme que le service rendu correspond bien à ce qui avait été demandé.");
        resultLines.push_back(quest.guildQuest
            ? "Nature : contrat officiel validé."
            : "Nature : demande informelle confirmée par le contact.");
        std::vector<std::string> titleContextLines = equippedTitleQuestContextLines(player, quest);
        if (!titleContextLines.empty())
        {
            resultLines.push_back("Influence des titres équipés :");
            resultLines.insert(resultLines.end(), titleContextLines.begin(), titleContextLines.end());
        }

        if (!quest.requiredMaterialId.empty() && quest.requiredMaterialQuantity > 0)
        {
            int owned = player.getInventory().countMaterialQualityPointsById(quest.requiredMaterialId) / 2;

            if (owned < quest.requiredMaterialQuantity)
            {
                MessageScreen::show(
                    "LIVRAISON INCOMPLÈTE",
                    "quest.turn_in.missing_materials",
                    {
                        "Il manque des matériaux pour rendre cette quête.",
                        quest.requiredMaterialName + " requis : " + std::to_string(quest.requiredMaterialQuantity)
                            + " (possédé : " + std::to_string(owned) + ")"
                    }
                );
                continue;
            }

            player.getInventory().removeMaterialQuantityByIdFlexible(quest.requiredMaterialId, quest.requiredMaterialQuantity);
            resultLines.push_back("Matériaux remis : " + quest.requiredMaterialName + " x" + std::to_string(quest.requiredMaterialQuantity));
        }

        quest.completed = true;
        quest.progress = quest.target;
        quest.turnedIn = true;
        player.recordPnjServed(quest.client.empty() ? std::string("Contact inconnu") : quest.client);
        player.recordQuestTypeCompleted(questKindText(quest));
        player.gainExperience(balancedQuestExperience(quest));
        player.getInventory().earnGold(balancedQuestGold(quest));
        applyQuestExtraReward(player, quest);
        if (quest.guildQuest && !quest.guildChallenge)
        {
            applyGuildStandingRewards(player, resultLines);
            applyQuestTitleRewards(player, quest, resultLines);
        }
        else if (quest.guildChallenge)
        {
            resultLines.push_back("La fiche reçoit la marque spéciale des défis, sans augmenter artificiellement le rang de guilde.");
        }

        appendQuestRewardResultLines(resultLines, quest);
        resultLines.push_back("Argent avant : " + Money::formatCurrencyOverviewFromCopper(copperBefore));
        resultLines.push_back("Argent actuel : " + Money::formatCurrencyOverviewFromCopper(player.getInventory().getTotalCopper()));
        resultLines.push_back("XP : " + std::to_string(experienceBefore) + " -> " + std::to_string(player.getExperience()));
        if (player.getLevel() != levelBefore)
        {
            resultLines.push_back("Niveau : " + std::to_string(levelBefore) + " -> " + std::to_string(player.getLevel()));
        }

        const Quest completedQuestSnapshot = quest;
        const std::string turnedInQuestId = quest.id;

        if (!quest.guildQuest && quest.client == "Prunigil le marchand")
        {
            grantPrunigilProgressRewards(player, resultLines);
            maybeAddMerchantFollowUp(player, completedQuestSnapshot, resultLines);
        }
        else if (!quest.guildQuest && quest.client == "Bob et Maurice")
        {
            maybeAddBobMauriceFollowUp(player, completedQuestSnapshot, resultLines);
        }

        MessageScreen::show(
            quest.guildChallenge ? "DÉFI VALIDÉ" : (quest.guildQuest ? "CONTRAT VALIDÉ" : "DEMANDE VALIDÉE"),
            quest.guildChallenge ? "quest.turn_in.challenge_completed" : (quest.guildQuest ? "quest.turn_in.guild_completed" : "quest.turn_in.personal_completed"),
            resultLines
        );

        if (turnedInQuestId == "story_ch1_orren_main"
            || turnedInQuestId == "story_ch1_lysa_main"
            || turnedInQuestId == "story_ch1_bram_main"
            || turnedInQuestId == "story_ch1_soryn_main"
            || turnedInQuestId == "story_ch1_mira_main")
        {
            // Le bilan de Mira doit toujours refléter les quêtes réellement rendues,
            // même si l'une d'elles a été terminée avant la rencontre des quatre référents.
            QuestMenu::syncMainStoryQuests(player);
        }

        bool hasMoreReadyForClient = false;
        const std::vector<Quest>& refreshedQuests = player.getQuestLog().getQuests();
        for (const Quest& remainingQuest : refreshedQuests)
        {
            if (remainingQuest.client == clientName && !remainingQuest.turnedIn && !remainingQuest.failed && isReadyToTurnIn(player, remainingQuest))
            {
                hasMoreReadyForClient = true;
                break;
            }
        }

        if (!hasMoreReadyForClient)
        {
            return;
        }
    }
}

// EN: maybeOfferRandomInterception declares or implements a focused behavior used by this module.
// FR: maybeOfferRandomInterception déclare ou implémente un comportement précis utilisé par ce module.
void QuestMenu::maybeOfferRandomInterception(Player& player, DifficultyMode difficulty, DeathRuleMode deathRule)
{
    Random random;

    if (random.between(1, 100) > 12)
    {
        return;
    }

    if (random.between(1, 100) <= 15)
    {
        simulateAfterCombatMiniBoss(player, random, difficulty, deathRule);
    }
    else
    {
        std::string intro;
        Quest offeredQuest = buildNpcQuestByRoll(player, random.between(1, 11), intro);
        displayQuestOffer(player, offeredQuest, intro);
    }

    Console::clear();
}


// EN: openExplorationMenu declares or implements a focused behavior used by this module.
// FR: openExplorationMenu déclare ou implémente un comportement précis utilisé par ce module.
void QuestMenu::openExplorationMenu(Player& player, DifficultyMode difficulty, DeathRuleMode deathRule)
{
    expireOverdueQuestDeadlines(player, "quest.exploration.menu", false);
    std::vector<ExplorationBiome> biomes = {
        {"Plaine sauvage", "biome ouvert, accessible aux débutants, mais jamais totalement sûr", "worn_leather_piece", "wolf_fang", 1, 10, "bêtes faibles, sangliers, loups isolés", "alphas jeunes, ours errants"},
        {"Route commerciale", "biome de passage accessible, avec voyageurs, bandits faibles et caisses perdues", "battle_torn_badge", "worn_leather_piece", 1, 14, "bandits, gobelins, humanoïdes opportunistes", "pilleurs vétérans, embuscades organisées"},
        {"Mares gélatineuses", "zone connue pour ses slimes : beaucoup de couleurs, peu de logique humaine, beaucoup de résidus", "slime_residue", "arcane_dust", 3, 18, "slimes verts, bleus, jaunes, rouges, ambrés et gris", "slimes chromatiques, dorés ou noirs anciens"},
        {"Forêt ancienne", "biome végétal plus sérieux, humide, propice aux plantes et aux bêtes discrètes", "bitter_healing_leaf", "mountain_blue_flower", 5, 20, "loups, racines, plantes hostiles", "alphas de mousse, gardiens de ronces"},
        {"Montagne froide", "biome rocheux intermédiaire, dur, avec minerais, fleurs rares et vents coupants", "rusted_metal_fragment", "mountain_blue_flower", 7, 24, "bêtes de givre, élémentaires, briseurs", "yétis, draconides froids, élites rocheuses"},
        {"Marais trouble", "biome dangereux, sale, collant et difficile d'accès en début de partie", "slime_residue", "arcane_dust", 12, 32, "slimes corrosifs, noyés, insectoïdes, prédateurs de boue", "slimes couronnés, mages putrides, noyés anciens"},
        {"Cimetière oublié", "biome sombre de niveau intermédiaire/avancé, lié aux morts-vivants, aux noms perdus et aux composants d'ombre", "cracked_bone", "shadow_thread", 10, 30, "squelettes, goules, corbeaux, lanternes d'âme", "oracles de tombe, ombres rares, ossuaires rampants"},
        {"Ruines effondrées", "biome ancien dangereux, instable, avec os, poussière arcanique et coffres suspects", "cracked_bone", "arcane_dust", 14, 36, "squelettes, goules, esprits, armures fissurées", "revenants, armures mortes, anomalies"},
        {"Bocage aux lanternes", "biome nocturne végétal, rempli de champignons-lampes, résines sonores et traces étranges", "mycelium_lantern", "echoing_resin", 8, 28, "spores, plantes lumineuses, bêtes attirées par la lumière", "rois-fonges, cerfs runiques, esprits clairs"},
        {"Désert d'argile rouge", "zone sèche de sel lunaire, argile cuite, fausses oasis et pilleurs poussiéreux", "sun_dried_clay", "moonlit_salt", 10, 34, "scorpions, chacals, slimes salins, totems fissurés", "colosses d'argile, sphinx perdus, bêtes de sel"},
        {"Quartier abandonné", "ancien morceau de ville visitable, avec maisons vides, caves, contrats sales et cartes brisées", "old_coin_bundle", "glass_map_fragment", 8, 32, "rats, voleurs, gobelins serruriers, automates de boutique", "collecteurs masqués, propriétaires sans visage, automates municipaux"},
        {"Mine sifflante", "ancienne mine visitable, pleine de rails, ressorts, fer froid et machines qui respirent mal", "cold_iron_nail", "tiny_gear_spring", 14, 40, "golems de rails, gobelins contremaîtres, slimes de charbon", "cœurs de machine, dragonnet de minerai, chefs de galerie"},
        {"Verger des lucioles de fer", "verger nocturne rempli d'insectes métalliques, de fruits trop brillants et de pièges doux au début", "firefly_iron_shell", "luminous_moth_wing", 6, 26, "lucioles de fer, mites lumineuses, renards voleurs de fruits", "essaims blindés, arbres-lampes, gardiens du verger"},
        {"Archives noyées", "ancienne bibliothèque inondée où les pages, les sceaux et les dettes murmurent encore", "tideworn_ink", "whispering_archive_page", 12, 38, "scribes noyés, slimes d'encre, rats de registre", "archives vivantes, greffiers fantômes, reliures carnivores"},
        {"Falaises des drakes gris", "corniches venteuses avec cordes, nids, pierres instables et petits drakes territoriaux", "salted_rope_knot", "grey_drake_scale", 18, 46, "chèvres de falaise, harpies grises, drakes jeunes", "matriarches des corniches, drakes gris adultes, esprits du vide"},
        {"Foire abandonnée", "ancienne fête foraine médiévale dont les stands vendent encore des tickets à des gens morts", "carnival_ticket_shred", "mirror_glass_bead", 10, 35, "pantins de stand, rats jongleurs, forains creux", "maîtres de piste masqués, manèges animés, miroirs menteurs"},
        {"Temple des cloches fendues", "ancien sanctuaire visitable où les cloches cassées répondent aux serments mal formulés", "cracked_bell_clapper", "sanctuary_wax_seal", 16, 42, "gardiens de nef, rats de sacristie, novices fantômes", "sonneurs creux, autels animés, chevaliers de vœu"},
        {"Canaux de brume bleue", "réseau de ponts bas, barques oubliées et brouillard froid qui cache les raccourcis", "blue_mist_reed", "mistglass_pearl", 9, 33, "anguilles de brume, voleurs de quai, slimes d'eau pâle", "passeurs sans visage, nixes anciennes, brumes conscientes"},
        {"Carrière des os blancs", "carrière pâle remplie de craie, de fossiles et de traces trop grandes", "white_bone_chalk", "buried_giant_chip", 20, 50, "scarabées d'os, golems de craie, mineurs pâles", "géants enfouis, sculpteurs d'os, colosses de poussière"},
        {"Marché sous les ponts", "marché illégal semi-visitable où chaque étal propose une bonne affaire et deux problèmes", "smuggler_token", "sealed_debt_slip", 12, 37, "contrebandiers, chiens de quai, gobelins prêteurs", "collecteurs masqués, arbitres de dette, ombres de pont"},
        {"Jardin des statues qui pleurent", "jardin noble abandonné, beau de loin, très mauvais de près", "weeping_stone_tear", "petrified_rose_petals", 14, 41, "statues fissurées, ronces blanches, oiseaux de pierre", "muses pétrifiées, jardiniers sans visage, rosiers de marbre"},
        {"Bois de la Corruption", "forêt noire où les racines boivent les mauvaises décisions et rendent la lumière sale", "shadow_thread", "unstable_core", 18, 44, "ronces sombres, loups corrompus, esprits collants", "cœurs noirs, dryades déformées, ombres à crocs"},
        {"Crypte du Sombre-Lien", "lieu souterrain de pactes anciens, entre corruption, morts-vivants et magie qui attache les noms", "cracked_bone", "shadow_thread", 20, 48, "squelettes liés, cultistes pâles, chaînes d'ombre", "prêtres sans regard, gardiens de serment noir, ossuaires liés"},
        {"Désert des Protecteurs", "désert antique couvert de statues de gardes, de sable blanc et de serments divins incomplets", "moonlit_salt", "progression_seal", 22, 52, "scarabées sacrés, chacals de sable, gardiens fissurés", "protecteurs éveillés, sphinx de serment, statues de divinité mineure"},
        {"Sanctuaire antique des Veilleurs", "ruines sacrées où les protecteurs jugent plus les intentions que les armes", "arcane_dust", "human_will_fragment", 24, 55, "sentinelles antiques, novices spectraux, golems de seuil", "veilleurs dorés, prêtresses de sable, juges de pierre"},
        {"Quartier des Lames Muettes", "zone urbaine d'assassins, de ruelles sans écho et de contrats qui disparaissent après lecture", "battle_torn_badge", "client_recommendation", 28, 62, "voleurs silencieux, éclaireurs masqués, chiens d'ombre", "assassins sans souffle, maîtres de poison, lames de guilde noire"},
        {"Toits des Assassins", "réseau de toitures, cordes, clochers et fenêtres ouvertes uniquement pour ceux qui savent fuir", "old_coin_bundle", "sealed_debt_slip", 30, 66, "archers de toit, coureurs masqués, corbeaux dressés", "duellistes de corniche, ombres de balcon, exécuteurs de contrat"},
        {"Nid draconique rouge", "territoire draconique de cendres chaudes, d'écailles rouges et de regards qui évaluent la nourriture", "draconic_scale_fragment", "elemental_fusion_core", 36, 75, "kobolds rouges, draconides jeunes, lézards de braise", "drakes rouges, mères de nid, gardiens de couvée"},
        {"Coulées de lave noire", "palier brûlant de rivières noires, basalte vivant et poches de feu trop calmes", "rusted_metal_fragment", "kitsune_ember", 38, 78, "slimes de lave, élémentaires de braise, golems de basalte", "seigneurs de magma, cœurs volcaniques, salamandres noires"},
        {"Glacier des Serments froids", "palier de glace séparé de la lave, où les promesses se conservent mieux que les corps", "mountain_blue_flower", "lunar_dream_fragment", 38, 78, "loups de givre, chevaliers gelés, slimes blancs", "drakes de glace, serments cristallisés, reines des congères"},
        {"Bosquet des Fées du Mana", "lieu lumineux, beau et dangereux, où les fées testent la politesse avant la puissance", "bitter_healing_leaf", "fitoria_feather", 45, 88, "fées joueuses, plantes de mana, lucioles bleues", "nobles fées, gardiens de pacte vert, esprits farceurs majeurs"},
        {"Sanctuaire kitsuné des Neuf Étincelles", "sanctuaire de renards-esprits, illusions et flammes fines qui ne brûlent pas toujours le corps", "kitsune_ember", "mirror_glass_bead", 46, 90, "kitsunés mineurs, renards de flamme, lanternes d'illusion", "prêtresses kitsuné, renards à neuf queues, miroirs de feu"},
        {"Confluence du Mana pur", "croisement de rivières magiques, instable mais magnifique, où chaque sort laisse une trace visible", "arcane_dust", "elemental_fusion_core", 50, 95, "élémentaires mineurs, slimes prismatiques, anomalies douces", "noyaux purs, archimages errants, tempêtes conscientes"},
        {"Bastion majeur scellé", "forteresse tardive liée à la fin de l'histoire, observable mais encore avare en réponses", "progression_seal", "absent_throne_fragment", 60, 115, "sentinelles majeures, chevaliers scellés, témoins muets", "gardiens de chapitre, serments royaux, fragments de trône"},
        {"Archipel des îles flottantes", "îles suspendues de tailles irrégulières, reliées par vents de mana, pierres volantes et ponts incomplets", "arcane_dust", "conscious_luck_shard", 70, 135, "harpies hautes, slimes de nuage, pierres éveillées", "baleines de ciel, chevaliers du vide, drakes d'altitude"},
        {"Ponts translucides de mana", "réseau fragile de ponts bleutés entre îles flottantes, plus solide quand personne ne panique", "blue_mist_reed", "elemental_fusion_core", 72, 140, "gardiens de pont, reflets de voyageur, élémentaires d'air", "architectes de mana, reflets parfaits, briseurs de passerelles"},
        {"Cieux des Légendes", "territoire céleste de récits vivants, où les exploits passés peuvent répondre par un combat", "lunar_dream_fragment", "lost_name_fragment", 90, 180, "échos héroïques, anges mineurs, constellations armées", "légendes éveillées, héros sans tombe, étoiles conscientes"},
        {"Parvis des Divinités", "hauteur presque divine, destinée aux mythes, aux cieux et aux entités qui ne devraient pas être farmées", "human_will_fragment", "absent_throne_fragment", 100, 200, "messagers célestes, statues vivantes, gardiens de seuil", "avatars mineurs, juges des cieux, fragments de divinité"}
    };

    std::vector<ExplorationIntensity> intensities = {
        {"Exploration courte", "sortie rapide : peu de temps dehors, moins de trouvailles folles", -16, -1, 70, 3, 1, 1, 0},
        {"Exploration normale", "équilibre actuel : un événement principal, faible chance d'un second", 0, 0, 100, 0, 1, 1, 18},
        {"Exploration longue", "grosse sortie : deux événements garantis, chance d'un troisième", 10, 1, 115, -2, 2, 2, 35}
    };

    const auto storyBiomeUnlocked = [&](const ExplorationBiome& biome)
    {
        if (!player.hasStoryModeStarted() || player.hasStorySkip())
        {
            return true;
        }

        if (biome.name == "Plaine sauvage" || biome.name == "Route commerciale" || biome.name == "Forêt ancienne")
        {
            return true;
        }

        if (player.getStoryChapter() >= 2 && biome.name == "Mares gélatineuses")
        {
            return true;
        }

        if (player.getStoryChapter() >= 2 && player.getStoryStep() >= 9
            && (biome.name == "Bocage aux lanternes" || biome.name == "Quartier abandonné"))
        {
            return true;
        }

        if (player.getStoryChapter() >= 2 && player.getStoryStep() >= 12 && biome.name == "Mine sifflante")
        {
            return true;
        }

        return false;
    };

    while (true)
    {
        MenuScreen screen("EXPLORATION", "exploration.biomes");
        screen.addLine("Choisis le style de biome à explorer.");
        screen.addLine("Temps actuel : " + player.formatWorldDateTimeLine());
        screen.addLine("Rythme : une journée = matin 1/5, midi 2/5, après-midi 3/5, soir 4/5, nuit 5/5.");
        if (player.getWorldDayProgressUnits() == 4)
        {
            screen.addLine("Avertissement : exploration lancée de nuit = danger plus élevé, sauf avec lanterne/kit nocturne/vision nocturne.");
        }
        screen.addLine("Exploration = fouille de terrain : plantes, matériaux, traces, trésors, coffres ou dangers imprévus.");
        screen.addLine("Tu pars chercher des traces, mais le terrain peut décider de te répondre avec des griffes.");
        screen.addLine("Économie : l'or direct d'exploration est pondéré par la difficulté ; les matériaux restent une grosse partie de la valeur.");
        const City* explorationOriginCity = City::findById(player.getCurrentCityId());
        if (explorationOriginCity != nullptr)
        {
            screen.addLine("Ville de départ : " + explorationOriginCity->getName() + " — les distances vers les biomes dépendront progressivement de cette ville.");
        }

        bool hasEvolvedBiome = false;
        int hiddenBiomeCount = 0;
        int storyLockedBiomeCount = 0;
        int unknownRumorCount = 0;
        std::vector<int> visibleBiomeIndexes;

        for (int i = 0; i < static_cast<int>(biomes.size()); ++i)
        {
            const ExplorationBiome& biomePreview = biomes[i];

            if (!storyBiomeUnlocked(biomePreview))
            {
                storyLockedBiomeCount++;
                continue;
            }

            if (!shouldShowBiomeToPlayer(player, biomePreview))
            {
                hiddenBiomeCount++;
                continue;
            }

            visibleBiomeIndexes.push_back(i);

            if (isBiomeUnknownToPlayer(player, biomePreview))
            {
                unknownRumorCount++;
                continue;
            }

            if (isBiomeEvolvedForPlayer(player, biomePreview))
            {
                if (!hasEvolvedBiome)
                {
                    screen.addLine("Zones déjà connues qui ont évolué avec ton niveau :");
                    hasEvolvedBiome = true;
                }

                screen.addLine("- " + biomePreview.name + " : "
                    + std::to_string(biomePreview.minLevel) + "-" + std::to_string(biomePreview.maxLevel)
                    + " devient " + std::to_string(evolvedBiomeMinLevel(player, biomePreview))
                    + "-" + std::to_string(evolvedBiomeMaxLevel(player, biomePreview))
                    + " autour de toi.");
            }
        }

        if (hasEvolvedBiome)
        {
            screen.addLine("Le monde ne t'attend pas immobile : les anciennes zones connues peuvent attirer des menaces adaptées.");
        }

        if (unknownRumorCount > 0)
        {
            screen.addLine("Certaines zones proches de ton niveau restent masquées : elles apparaissent en ????? jusqu'à la première vraie visite.");
        }

        if (hiddenBiomeCount > 0)
        {
            screen.addLine(std::to_string(hiddenBiomeCount) + " zones trop hautes restent invisibles pour préserver la découverte.");
        }
        if (storyLockedBiomeCount > 0)
        {
            screen.addLine("Mode histoire : " + std::to_string(storyLockedBiomeCount) + " zones pas encore accessibles restent cachées.");
        }

        screen.addOption(0, "Retour", "", true, "exploration.back");

        int visibleChoice = 1;
        int rumorIndex = 1;
        for (int biomeIndex : visibleBiomeIndexes)
        {
            const ExplorationBiome& biomePreview = biomes[biomeIndex];
            const bool unknownBiome = isBiomeUnknownToPlayer(player, biomePreview);

            if (unknownBiome)
            {
                screen.addOption(
                    visibleChoice,
                    unknownBiomeLabel(player, biomePreview, rumorIndex),
                    "Terrain : ??? | Rares : ??? | Le nom réel sera inscrit après exploration.",
                    true,
                    "exploration.biome.unknown." + std::to_string(rumorIndex),
                    makeUnknownExplorationBiomeItemData(rumorIndex, biomePreview)
                );
                rumorIndex++;
                visibleChoice++;
                continue;
            }

            std::string label = biomePreview.name
                + " (" + evolvedBiomeRangeText(player, biomePreview) + ") — "
                + biomePreview.style;

            const bool questLikely = hasPotentialQuestForBiome(player, biomePreview);

            if (questLikely)
            {
                label += " [Objectif de quête probable]";
            }

            screen.addOption(
                visibleChoice,
                label,
                "Terrain : " + biomePreview.commonMonsters + " | Rares : " + biomePreview.rareMonsters
                    + " | Distance : " + explorationDistanceLabel(explorationTravelUnitsForBiome(biomePreview))
                    + (explorationOriginCity != nullptr && explorationOriginCity->getDistanceToBiome(biomePreview.name) >= 0
                        ? " / " + std::to_string(explorationOriginCity->getDistanceToBiome(biomePreview.name)) + " km depuis " + explorationOriginCity->getName()
                        : "")
                    + " | Taille : " + explorationBiomeSizeLabel(explorationBiomeSizeUnits(biomePreview)),
                true,
                "exploration.biome." + std::to_string(visibleChoice),
                makeExplorationBiomeItemData(player, biomePreview, questLikely)
            );
            visibleChoice++;
        }

        int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choix invalide.");
        Console::clear();

        if (choice == 0)
        {
            return;
        }

        if (choice < 1 || choice > static_cast<int>(visibleBiomeIndexes.size()))
        {
            continue;
        }

        const ExplorationBiome& biome = biomes[visibleBiomeIndexes[choice - 1]];
        const bool wasUnknownBiome = isBiomeUnknownToPlayer(player, biome);
        const std::string selectedBiomeDisplayName = wasUnknownBiome ? "cette zone inconnue" : biome.name;
        int expeditionElapsedUnits = 0;
        int expeditionFoodUnitsSinceLastRation = 0;
        bool firstExplorationAtThisBiome = true;

        while (true)
        {
        MenuScreen intensityScreen("INTENSITÉ", "exploration.intensity");
        intensityScreen.addLine("Choisis comment tu veux explorer " + selectedBiomeDisplayName + ".");
        intensityScreen.addOption(0, "Retour aux biomes", "", true, "exploration.intensity.back");

        for (int i = 0; i < static_cast<int>(intensities.size()); ++i)
        {
            intensityScreen.addOption(
                i + 1,
                intensities[i].name + " — " + intensities[i].description,
                "Risque " + std::to_string(intensities[i].eventShift)
                    + "% | Pièces " + std::to_string(intensities[i].goldPercent)
                    + "% | Temps +" + std::to_string(intensities[i].durationUnits)
                    + " segment(s) | Événements " + std::to_string(intensities[i].guaranteedEvents)
                    + (intensities[i].extraEventChance > 0 ? " + chance bonus" : ""),
                true,
                "exploration.intensity." + std::to_string(i + 1),
                makeExplorationIntensityItemData(intensities[i])
            );
        }

        int intensityChoice = TerminalInterface::askMenuChoiceFromOptions(intensityScreen, "Choix invalide.");
        Console::clear();

        if (intensityChoice == 0)
        {
            break;
        }

        const ExplorationIntensity& intensity = intensities[intensityChoice - 1];
        if (wasUnknownBiome)
        {
            recordBiomeDiscoveryForPlayer(biome);
        }
        else if (!isBiomeDiscoveredForPlayer(biome))
        {
            recordBiomeDiscoveryForPlayer(biome);
        }
        Random random;
        std::vector<std::string> travelLines;
        const int rawTravelUnits = firstExplorationAtThisBiome ? explorationTravelUnitsForBiome(biome) : 0;
        const int biomeSizeUnits = explorationBiomeSizeUnits(biome);
        int travelUnits = 0;
        if (firstExplorationAtThisBiome)
        {
            travelUnits = reduceExplorationTravelWithPreparation(player, rawTravelUnits, travelLines);
        }
        else
        {
            travelLines.push_back("Continuité : tu es déjà sur place, retourner au même lieu ne coûte pas un nouveau trajet.");
            travelLines.push_back("Taille du biome : le terrain reste compté, parce que fouiller un grand lieu prend encore du temps même sans refaire la route.");
        }
        const int totalTimeUnits = std::max(1, intensity.durationUnits + travelUnits + biomeSizeUnits);
        const int dayBeforeExploration = player.getWorldDaysElapsed();
        const int unitBeforeExploration = player.getWorldDayProgressUnits();
        const bool touchesNight = explorationTouchesNight(unitBeforeExploration, totalTimeUnits, player.getWorldDayUnitsPerDay());
        player.advanceWorldDayUnits(totalTimeUnits);
        expeditionElapsedUnits += totalTimeUnits;
        expeditionFoodUnitsSinceLastRation += totalTimeUnits;
        expireOverdueQuestDeadlines(player, "exploration.run", true);
        const int hpBeforeExploration = player.getHp();
        const int goldBeforeExploration = player.getInventory().getGold();
        const int readyBeforeExploration = countQuestsForJournalFilter(player, QuestJournalFilter::ReadyToTurnIn);

        QuestSearchHint questHint = getQuestSearchHintForBiome(player, biome);
        int nightExtraFightChance = 0;
        std::vector<std::string> nightLines;
        const int nightRollShift = applyNightExplorationRisk(player, touchesNight, random, nightLines, nightExtraFightChance);
        const int temperatureRollShift = applyTemperatureExplorationRisk(player, biome, totalTimeUnits, nightLines);
        int racialRollShift = 0;
        if (playerHasExplorationPassive(player, "elven_fine_perception"))
        {
            racialRollShift -= 3;
            nightLines.push_back("Perception elfique : les traces utiles ressortent mieux avant le premier vrai événement.");
        }
        if (playerHasExplorationPassive(player, "dwarven_mine_sense")
            && (biome.name.find("Ruines") != std::string::npos
                || biome.name.find("Montagne") != std::string::npos
                || biome.name.find("Glacier") != std::string::npos
                || biome.name.find("Falaises") != std::string::npos))
        {
            racialRollShift -= 4;
            nightLines.push_back("Sens des galeries : pierres, pentes et ruines racontent un peu mieux leur danger.");
        }
        if (playerHasExplorationPassive(player, "dragon_weather_blood")
            && (biome.name.find("Coulées") != std::string::npos || biome.name.find("Nid draconique") != std::string::npos))
        {
            racialRollShift -= 3;
            nightLines.push_back("Sang draconique : la chaleur et les traces de grands reptiles sont moins perturbantes.");
        }
        if (playerHasExplorationPassive(player, "semi_wolf_tracking")
            && (biome.name.find("Forêt") != std::string::npos || biome.name.find("Route") != std::string::npos || biome.name.find("Plaine") != std::string::npos))
        {
            racialRollShift -= 3;
            nightLines.push_back("Flair de meute : les pistes de bêtes, de bandits ou de convoi ressortent mieux.");
        }
        if (playerHasExplorationPassive(player, "semi_dog_loyal_scent"))
        {
            racialRollShift -= 2;
            nightLines.push_back("Flair loyal : escortes, recherches et retours prudents sont un peu plus fiables.");
        }
        if (playerHasExplorationPassive(player, "semi_fox_cunning"))
        {
            racialRollShift -= 2;
            nightLines.push_back("Ruse de renard : un détour secondaire semble moins hasardeux que prévu.");
        }
        if (playerHasExplorationPassive(player, "semi_cat_reflexes") && touchesNight)
        {
            racialRollShift -= 2;
            nightLines.push_back("Réflexes félins : la nuit reste dangereuse, mais tes appuis corrigent plusieurs surprises.");
        }
        if (playerHasExplorationPassive(player, "semi_lizard_scales")
            && (biome.name.find("Désert") != std::string::npos || biome.name.find("Coulées") != std::string::npos))
        {
            racialRollShift -= 2;
            nightLines.push_back("Écailles tempérées : la chaleur sèche semble un peu moins brutale.");
        }
        if (playerHasExplorationPassive(player, "semi_lizard_scales")
            && (biome.name.find("Marais") != std::string::npos || biome.name.find("Mares") != std::string::npos || biome.name.find("Lagune") != std::string::npos))
        {
            racialRollShift -= 2;
            nightLines.push_back("Écailles de semi-lézard : l'humidité sale et les sols mous se lisent un peu mieux.");
        }
        if (playerHasExplorationPassive(player, "semi_bird_open_sky")
            && (biome.name.find("Falaises") != std::string::npos
                || biome.name.find("Cieux") != std::string::npos
                || biome.name.find("Archipel") != std::string::npos
                || biome.name.find("Route") != std::string::npos))
        {
            racialRollShift -= 2;
            nightLines.push_back("Sens des hauteurs : le vent, les corniches et les routes ouvertes se lisent un peu mieux.");
        }

        int curseRollShift = 0;
        const int travelCursePressure = player.getCursePressureForCategory("travel");
        const int luckCursePressure = player.getCursePressureForCategory("luck");
        if (travelCursePressure > 0)
        {
            curseRollShift += std::min(10, 2 + travelCursePressure * 2);
            if (player.getKnownCursePressureForCategory("travel") > 0)
            {
                nightLines.push_back("Malédiction diagnostiquée : la catégorie voyage rend la route moins sûre.");
            }
            else
            {
                nightLines.push_back("Route étrange : tu as l'impression d'être suivi ou mal orienté, sans certitude.");
            }
        }
        if (luckCursePressure > 0)
        {
            curseRollShift += std::min(7, 1 + luckCursePressure);
            if (player.getKnownCursePressureForCategory("luck") > 0)
            {
                nightLines.push_back("Malédiction diagnostiquée : la catégorie chance rend les petits hasards moins gentils.");
            }
            else
            {
                nightLines.push_back("Les petits signes de route tombent mal, comme si le hasard te regardait de travers.");
            }
        }

        const int spiritCursePressure = player.getCursePressureForCategory("spirit");
        const int corruptionCursePressure = player.getCursePressureForCategory("corruption");
        const int socialCursePressure = player.getCursePressureForCategory("social");
        if (spiritCursePressure > 0)
        {
            curseRollShift += std::min(6, 1 + spiritCursePressure);
            nightExtraFightChance += std::min(12, 2 + spiritCursePressure * 2);
            nightLines.push_back(player.getKnownCursePressureForCategory("spirit") > 0
                ? "Malédiction diagnostiquée : la catégorie esprit rend les présences de terrain plus insistantes."
                : "Présence mentale : tu as parfois l'impression de marcher avec une pensée qui n'est pas la tienne.");
        }
        if (corruptionCursePressure > 0)
        {
            curseRollShift += std::min(8, 2 + corruptionCursePressure);
            nightLines.push_back(player.getKnownCursePressureForCategory("corruption") > 0
                ? "Malédiction diagnostiquée : la catégorie corruption attire davantage les lieux sales ou instables."
                : "Quelque chose dans l'air accroche la peau, sans que tu saches si le lieu ou toi êtes en cause.");
        }
        if (socialCursePressure > 0)
        {
            nightLines.push_back(player.getKnownCursePressureForCategory("social") > 0
                ? "Malédiction diagnostiquée : la catégorie présence sociale peut rendre les rencontres moins naturelles."
                : "Quand une silhouette apparaît au loin, tu hésites une seconde de trop à l'aborder.");
        }

        int roll = adjustExplorationEventRoll(random.between(1, 100), intensity);
        roll = std::clamp(roll + nightRollShift + temperatureRollShift + racialRollShift + curseRollShift, 1, 100);
        roll = adjustExplorationRollForActiveQuests(roll, random, questHint);
        bool carefulRecovery = chooseCarefulRecovery(random, intensity);

        std::vector<std::string> entryLines = {
            "Style : " + biome.style + ".",
            "Niveaux locaux : " + std::to_string(biome.minLevel) + "-" + std::to_string(biome.maxLevel) + ".",
            "Monstres surtout présents : " + biome.commonMonsters + ".",
            "Rares / élites typiques : " + biome.rareMonsters + ".",
            "Approche : " + intensity.name + ".",
            "Distance : " + explorationDistanceLabel(rawTravelUnits) + " | coût final du déplacement : +" + std::to_string(travelUnits) + " segment(s).",
            "Taille/terrain du biome : " + explorationBiomeSizeLabel(biomeSizeUnits) + ".",
            "Temps écoulé : +" + std::to_string(totalTimeUnits) + " segment(s) de journée.",
            player.formatWorldTimeChange(dayBeforeExploration, unitBeforeExploration),
            "Rappel temps : une journée vaut maintenant 5 moments : matin, midi, après-midi, soir, nuit."
        };
        entryLines.insert(entryLines.end(), travelLines.begin(), travelLines.end());
        entryLines.insert(entryLines.end(), nightLines.begin(), nightLines.end());
        std::vector<std::string> timeReportLines = player.consumeWorldTimeReportLines();
        entryLines.insert(entryLines.end(), timeReportLines.begin(), timeReportLines.end());

        if (wasUnknownBiome)
        {
            entryLines.insert(entryLines.begin(), "Nouvelle zone découverte : " + biome.name + ". Elle restera maintenant affichée par son vrai nom.");
        }

        if (isBiomeEvolvedForPlayer(player, biome))
        {
            entryLines.push_back("Adaptation de zone : ton niveau attire maintenant des menaces plus fortes ici.");
            entryLines.push_back("Niveaux effectifs actuels : "
                + std::to_string(evolvedBiomeMinLevel(player, biome))
                + "-" + std::to_string(evolvedBiomeMaxLevel(player, biome)) + ".");
            entryLines.push_back("Les récompenses suivent mieux ce danger, car les rencontres générées montent aussi en niveau.");
        }

        if (questHint.hasAny)
        {
            entryLines.push_back("Ton journal réagit légèrement : cette zone peut aider une quête active, sans garantir la trouvaille.");
        }

        if (player.hasStoryModeStarted() && player.getStoryChapter() >= 3)
        {
            const std::vector<std::string> consequenceLines = StoryCampaign::buildChapterThreeConsequenceLines(player);
            if (!consequenceLines.empty())
            {
                entryLines.push_back("Conséquence d'histoire active : " + consequenceLines.front());
            }
        }

        showExplorationNotice("EXPLORATION — " + biome.name, "exploration.run.entry", entryLines, false);

        ExplorationRouteResult routeResult = runExplorationRouteChoice(player, biome, intensity, random);
        roll = std::clamp(roll + routeResult.rollShift, 1, 100);
        if (routeResult.carefulBoost)
        {
            carefulRecovery = true;
        }
        if (routeResult.questProgress > 0)
        {
            int updated = progressExplorationQuests(player, biome.name, routeResult.questProgress);
            if (updated > 0)
            {
                routeResult.lines.push_back("Des quêtes d'exploration progressent déjà grâce à ton choix de route.");
            }
        }
        showExplorationNotice("ROUTE CHOISIE", "exploration.route_choice.result", routeResult.lines);

        maybeTriggerLegendaryMerchantEncounter(player, random, biome.name);

        MicroChallengeResult microChallenge = runExplorationMicroChallenge(player, biome, intensity, random);
        if (microChallenge.success)
        {
            carefulRecovery = true;
            roll = std::max(1, roll - 8);
            int updated = progressExplorationQuests(player, biome.name, 1);
            if (updated > 0)
            {
                microChallenge.lines.push_back("Ton carnet progresse déjà grâce à cette préparation active.");
            }
        }
        else
        {
            roll = std::min(100, roll + 5);
        }
        showExplorationNotice(
            microChallenge.success ? "ÉPREUVE RÉUSSIE" : "ÉPREUVE RATÉE",
            microChallenge.success ? "exploration.micro_challenge.success" : "exploration.micro_challenge.failure",
            microChallenge.lines
        );

        std::vector<std::string> eventLabels;
        std::set<std::string> currentRunEventKeys;
        auto runExplorationEvent = [&](int eventRoll, int eventIndex) {
            eventRoll = std::clamp(eventRoll, 1, 100);
            const std::string eventKey = explorationEventKeyFromRoll(eventRoll);
            const std::string eventLabel = explorationEventLabelFromRoll(eventRoll);
            currentRunEventKeys.insert(eventKey);
            player.recordExplorationEventKey(eventKey);
            player.startExplorationSceneCooldown(eventKey, explorationEventCooldownDays(eventKey));
            eventLabels.push_back("Événement " + std::to_string(eventIndex) + " : " + eventLabel);

            if (eventIndex > 1)
            {
                showExplorationNotice(
                    "ÉVÉNEMENT SUPPLÉMENTAIRE",
                    "exploration.run.extra_event",
                    {
                        "La sortie continue : " + intensity.name + " permet de tomber sur plus d'une chose pendant la même exploration.",
                        "Événement supplémentaire : " + eventLabel + "."
                    },
                    false
                );
            }

            if (eventRoll <= 26)
            {
                std::vector<std::string> lines;
                int quantity = 1;
                bool favorQuality = carefulRecovery;
                if (eventKey == "main_gather_exposed")
                {
                    lines.push_back("Une ressource utile pousse à découvert, visible depuis le passage.");
                    quantity = random.between(1, 2);
                }
                else if (eventKey == "main_gather_hidden")
                {
                    lines.push_back("Des marques discrètes conduisent vers une petite poche de ressources cachée sous le terrain.");
                    quantity = random.between(1, 2);
                    if (random.between(1, 100) <= 24)
                    {
                        lines.push_back(addExplorationMaterial(player, biome.rareMaterialId, 1, chooseExplorationQuality(random, true)));
                    }
                }
                else
                {
                    lines.push_back("La ressource est fragile : une récolte brutale la rendrait presque inutile.");
                    quantity = 1;
                    favorQuality = true;
                }
                if (carefulRecovery)
                {
                    lines.push_back("Récolte propre : ta préparation évite de gaspiller la trouvaille.");
                }
                lines.push_back(addExplorationMaterial(player, biome.commonMaterialId, applyExplorationQuantityBonus(quantity, intensity), chooseExplorationQuality(random, favorQuality)));
                showExplorationNotice("RÉCOLTE", "exploration.run.gather." + eventKey, lines);
            }
            else if (eventRoll <= 40)
            {
                std::vector<std::string> lines;
                int progress = 1;
                if (eventKey == "main_trace_broken")
                {
                    lines.push_back("Une piste s'interrompt net, comme si ce qui laissait les traces avait changé de direction sans tourner.");
                    progress = random.between(1, 2);
                }
                else if (eventKey == "main_trace_passage")
                {
                    lines.push_back("Plusieurs marques de passage se superposent. Elles ne racontent pas la même heure ni le même groupe.");
                    recordBiomeFieldObservation(biome, "Marques superposées observées : plusieurs passages récents traversent " + biome.name + ".");
                    progress = 2;
                }
                else
                {
                    lines.push_back("Tu retrouves un objet perdu, trop abîmé pour être vendu mais assez précis pour indiquer d'où venait son propriétaire.");
                    progress = 1;
                    if (random.between(1, 100) <= 40)
                    {
                        int recoveredGold = applyExplorationGoldReward(random.between(2, 10 + player.getLevel()), player, intensity, difficulty, 0);
                        player.getInventory().earnGold(recoveredGold);
                        lines.push_back("Quelques pièces encore valables restent coincées dedans : " + Money::formatGoldWithRaw(recoveredGold) + ".");
                    }
                }
                int updated = progressExplorationQuests(player, biome.name, progress);
                if (updated > 0)
                {
                    lines.push_back("Des quêtes d'exploration progressent grâce à cette découverte.");
                }
                else
                {
                    lines.push_back("Tu conserves l'information dans tes notes, même si aucune quête actuelle ne l'exploite.");
                }
                showExplorationNotice("TRACE INTÉRESSANTE", "exploration.run.trace." + eventKey, lines);
            }
            else if (eventRoll <= 52)
            {
                const bool oldDeposit = eventKey == "main_treasure_deposit";
                int gold = applyExplorationGoldReward(
                    oldDeposit ? random.between(8, 28 + player.getLevel() * 2) : random.between(5, 20 + player.getLevel()),
                    player,
                    intensity,
                    difficulty,
                    1
                );
                player.getInventory().earnGold(gold);
                std::vector<std::string> lines = {
                    oldDeposit
                        ? "Tu découvres un petit dépôt ancien, protégé par une pierre plate et beaucoup de poussière."
                        : "Une bourse oubliée a glissé hors du passage principal.",
                    "Argent gagné : " + Money::formatGoldWithRaw(gold)
                };
                if (oldDeposit && random.between(1, 100) <= 55)
                {
                    lines.push_back(addExplorationMaterial(player, biome.commonMaterialId, 1, chooseExplorationQuality(random, true)));
                }
                showExplorationNotice("PETIT TRÉSOR", "exploration.run.gold." + eventKey, lines);
            }
            else if (eventRoll <= 59)
            {
                showExplorationNotice(
                    "FAUSSES PIÈCES",
                    "exploration.run.fake_gold",
                    {
                        "Tu trouves beaucoup de pièces d'or.",
                        "Pendant une seconde, tu te vois déjà riche.",
                        "Mais en les prenant dans ta main, les pièces fondent entre tes doigts.",
                        "De fausses pièces. Une arnaque magique ridicule.",
                        "Tu décides de laisser toute cette honte au sol."
                    }
                );
            }
            else if (eventRoll <= 70)
            {
                openExplorationChest(player, random, biome, intensity, difficulty, deathRule);
            }
            else if (eventRoll <= 82)
            {
                simulateUnexpectedExplorationFight(player, random, biome, intensity, difficulty, deathRule);
            }
            else if (eventRoll <= 84)
            {
                simulateExplorationMiniBoss(player, random, biome, intensity, difficulty, deathRule);
            }
            else if (eventRoll <= 91)
            {
                offerExplorationNpcQuest(player, random, biome);
            }
            else if (eventRoll <= 97)
            {
                triggerActiveExplorationEvent(player, random, biome, intensity, difficulty, deathRule);
            }
            else if (eventRoll <= 99)
            {
                openDangerousExplorationSite(player, random, biome, intensity, difficulty, deathRule);
            }
            else
            {
                triggerRareExplorationDiscovery(player, random, biome, intensity, difficulty, deathRule);
            }
        };

        int eventCount = std::max(1, intensity.guaranteedEvents);
        if (intensity.extraEventChance > 0 && random.between(1, 100) <= intensity.extraEventChance)
        {
            ++eventCount;
        }
        if (touchesNight && nightExtraFightChance > 0 && random.between(1, 100) <= nightExtraFightChance)
        {
            ++eventCount;
        }
        eventCount = std::min(3, eventCount);

        for (int eventIndex = 1; eventIndex <= eventCount; ++eventIndex)
        {
            int eventRoll = eventIndex == 1
                ? roll
                : adjustExplorationEventRoll(random.between(1, 100), intensity);

            const int repeatRerollShift = eventIndex > 1
                ? 4 + nightRollShift / 2 + std::max(0, temperatureRollShift / 2)
                : 0;
            if (eventIndex > 1)
            {
                eventRoll = std::clamp(eventRoll + repeatRerollShift, 1, 100);
            }

            eventRoll = applyChapterThreeExplorationChoiceBias(player, random, eventRoll);
            eventRoll = chooseVariedMainExplorationRoll(
                player,
                random,
                eventRoll,
                intensity,
                repeatRerollShift,
                currentRunEventKeys
            );
            runExplorationEvent(eventRoll, eventIndex);
        }

        std::string eventLabel = "aucun événement noté";
        if (!eventLabels.empty())
        {
            eventLabel.clear();
            for (std::size_t i = 0; i < eventLabels.size(); ++i)
            {
                if (i > 0) eventLabel += " | ";
                eventLabel += eventLabels[i];
            }
        }

        player.getQuestLog().refreshMaterialDeliveryQuests(player.getInventory());
        showExplorationRunSummary(
            player,
            biome,
            intensity,
            eventLabel,
            hpBeforeExploration,
            goldBeforeExploration,
            readyBeforeExploration,
            dayBeforeExploration,
            unitBeforeExploration,
            totalTimeUnits
        );

        std::vector<std::string> continuationLines = {
            "Zone actuelle : " + biome.name + ".",
            "Temps passé dehors depuis le départ : " + std::to_string(expeditionElapsedUnits)
                + " segment(s) (" + std::to_string(player.getWorldDayUnitsPerDay()) + " segment(s) = 1 journée complète).",
            "Autonomie depuis la dernière ration : " + std::to_string(expeditionFoodUnitsSinceLastRation)
                + "/" + std::to_string(player.getWorldDayUnitsPerDay()) + " segment(s).",
            "Continuer ici ne repaie pas le trajet : tu es déjà sur place.",
            "La taille du biome reste comptée à chaque nouvelle fouille : seul le trajet d'arrivée disparaît.",
            "Si une journée complète d'autonomie est utilisée dehors, il faut une Ration de survie pour continuer sans rentrer."
        };

        const int continuationChoice = askChoiceScreen(
            "APRÈS L'EXPLORATION",
            "exploration.after_run.choice",
            continuationLines,
            {
                {1, "Continuer l'exploration du même lieu"},
                {2, "Rentrer"}
            },
            1,
            2
        );
        Console::clear();

        if (continuationChoice != 1)
        {
            showExplorationNotice(
                "RETOUR",
                "exploration.after_run.return",
                {
                    "Tu rentres sans repayer le trajet retour dans cette version : le coût important était surtout l'aller et la préparation.",
                    "Le prochain départ vers une autre zone recalculera la distance normalement."
                },
                false
            );
            break;
        }

        if (expeditionFoodUnitsSinceLastRation >= player.getWorldDayUnitsPerDay())
        {
            if (player.getInventory().removeMaterialQuantityById("survival_ration", 1))
            {
                expeditionFoodUnitsSinceLastRation = 0;
                showExplorationNotice(
                    "RATION CONSOMMÉE",
                    "exploration.after_run.ration_used",
                    {
                        "Tu as utilisé une journée complète d'autonomie dehors depuis le départ ou la dernière ration.",
                        "Ration de survie consommée x1 : l'autonomie d'exploration est réinitialisée.",
                        "Tu ne dois donc pas spammer les rations à chaque clic : une ration couvre une nouvelle journée complète de sortie.",
                        "Tu choisiras à nouveau si la suite est courte, normale ou longue, puis l'approche de route prudente ou audacieuse."
                    },
                    false
                );
            }
            else
            {
                showExplorationNotice(
                    "RATION MANQUANTE",
                    "exploration.after_run.ration_missing",
                    {
                        "Tu as utilisé une journée complète d'autonomie dehors depuis le départ ou la dernière ration.",
                        "Impossible de continuer sans Ration de survie : le personnage doit éviter de crever de faim hors simulation détaillée.",
                        "Tu rentres donc en ville. Les auberges, relais et boutiques de consommables vendent des rations abordables, mais pas données gratuitement."
                    },
                    false
                );
                break;
            }
        }

        firstExplorationAtThisBiome = false;
        Console::clear();
        }
    }
}
