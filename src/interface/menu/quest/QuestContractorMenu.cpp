// EN: QuestMenu.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: QuestMenu.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu.
// Description: Implements quest hub and read-only quest journal for Dinotofu.

#include "interface/menu/quest/QuestContractorMenu.hpp"
#include "interface/menu/quest/QuestMenu.hpp"
#include "adventure/flavor/ExplorationBiomeFlavor.hpp"
#include "adventure/flavor/ExplorationLanguageTrace.hpp"
#include "adventure/content/BiomeLivingContentCatalog.hpp"
#include "adventure/content/BiomeAmbientEventSystem.hpp"
#include "interface/menu/training/TrainingGroundMenu.hpp"
#include "interface/menu/LocalReputationRepairMenu.hpp"

#include "core/Console.hpp"
#include "core/Random.hpp"
#include "quest/QuestCatalog.hpp"
#include "quest/language/QuestLanguageSystem.hpp"
#include "item/material/MaterialCatalog.hpp"
#include "entity/MonsterCatalog.hpp"
#include "combat/modes/pve/MonsterPveMode.hpp"
#include "combat/system/ElementalAffinitySystem.hpp"
#include "character/RaceCatalog.hpp"
#include "economy/EconomyBalance.hpp"
#include "economy/shop/ShopTransactionSystem.hpp"
#include "economy/Money.hpp"
#include "interface/menu/InventoryMenu.hpp"
#include "interface/menu/shop/ShopMenu.hpp"
#include "interface/menu/common/PagedMenu.hpp"
#include "interface/menu/common/MessageScreen.hpp"
#include "interface/TerminalInterface.hpp"
#include "interface/model/MenuScreen.hpp"
#include "progression/bestiary/BestiaryRuntimeProgress.hpp"
#include "story/StoryCampaign.hpp"
#include "world/City.hpp"
#include "world/CityTravelRules.hpp"
#include "world/npc/NpcKnowledgeSystem.hpp"
#include "world/npc/NpcInformationPropagationSystem.hpp"

#include <iostream>
#include <vector>
#include <algorithm>
#include <cstddef>
#include <cctype>
#include <set>
#include <utility>
#include <sstream>
#include <map>

namespace
{
    std::string currentCityName(const Player& player)
    {
        const City* city = City::findById(player.getCurrentCityId());
        return city == nullptr ? "Ville inconnue" : city->getName();
    }

    int canonicalRecordCount(const Player& player, const std::string& category, const std::string& key)
    {
        for (const PlayerJournalRecord& record : player.getCanonicalJournalRecords())
        {
            if (record.category == category && record.key == key) return record.count;
        }
        return 0;
    }

    std::string recentActionKey(const Player& player, const std::string& baseKey)
    {
        return baseKey + ":day" + std::to_string(player.getWorldDaysElapsed())
            + ":unit" + std::to_string(player.getWorldDayProgressUnits())
            + ":" + std::to_string(player.getCanonicalJournalRecords().size());
    }

    void recordRecentAction(Player& player, const std::string& baseKey, const std::string& label)
    {
        player.recordCanonicalEvent("dernieres_actions", recentActionKey(player, baseKey), label);
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

    std::vector<std::string> splitMissionKey(const std::string& key)
    {
        std::vector<std::string> parts;
        std::stringstream ss(key);
        std::string part;
        while (std::getline(ss, part, '|')) parts.push_back(part);
        return parts;
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
        if (!player.hasTitle("Aventurier")) return "Non inscrit";
        int completedGuildContracts = 0;
        for (const Quest& quest : player.getQuestLog().getQuests())
            if (quest.guildQuest && quest.turnedIn) ++completedGuildContracts;
        struct Threshold { int requiredContracts; int requiredLevel; std::string rank; };
        const std::vector<Threshold> thresholds = {
            {130, 90, "Dieu"}, {100, 70, "Légende"}, {75, 55, "Héros mondial"},
            {55, 42, "SSS"}, {40, 35, "SS"}, {28, 24, "S"}, {20, 18, "A"},
            {14, 12, "B"}, {9, 8, "C"}, {5, 5, "D"}, {2, 2, "E"}
        };
        for (const Threshold& threshold : thresholds)
            if (completedGuildContracts >= threshold.requiredContracts && player.getLevel() >= threshold.requiredLevel) return threshold.rank;
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
            if (quest.guildQuest && quest.turnedIn) ++completedGuildContracts;
        return {
            "Accès officiel refusé : les demandes, mandats directs, contacts de groupes et quêtes publiées demandent au moins le rang D de guilde.",
            "Rang actuel : " + guildRankForRequestGate(player) + ".",
            "Progression actuelle : " + std::to_string(completedGuildContracts) + "/5 contrats officiels validés et niveau " + std::to_string(player.getLevel()) + "/5 requis pour le rang D.",
            "Raison d'équilibrage : avant le rang D, payer des PNJ pour travailler à ta place serait trop fort et casserait la progression.",
            "Exception risquée : certains contacts douteux peuvent accepter illégalement, avec prix gonflé, vol possible et suspension de guilde si ça se sait.",
            "Tu peux encore faire les quêtes de guilde normales, découvrir le monde, rendre des services et revenir quand ton dossier sera assez solide."
        };
    }
}

namespace QuestContractorMenu
{
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


}
