// EN: QuestTeamMenu.cpp implements recruit, clan and group-management quest menus.
// FR: QuestTeamMenu.cpp implémente les menus de recrues, clan et gestion de groupe.

#include "interface/menu/quest/QuestMenu.hpp"
#include "interface/menu/quest/QuestTeamMenu.hpp"
#include "interface/menu/quest/QuestContractorMenu.hpp"
#include "world/City.hpp"
#include "core/Console.hpp"
#include "core/Random.hpp"
#include "economy/EconomyBalance.hpp"
#include "economy/Money.hpp"
#include "entity/Player.hpp"
#include "interface/TerminalInterface.hpp"
#include "interface/menu/common/MessageScreen.hpp"
#include "interface/model/MenuScreen.hpp"
#include "quest/Quest.hpp"
#include "quest/QuestLog.hpp"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstddef>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

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

    void recordRecentAction(Player& player, const std::string& baseKey, const std::string& label)
    {
        const std::string key = baseKey + ":day" + std::to_string(player.getWorldDaysElapsed())
            + ":unit" + std::to_string(player.getWorldDayProgressUnits())
            + ":" + std::to_string(player.getCanonicalJournalRecords().size());
        player.recordCanonicalEvent("dernieres_actions", key, label);
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
        int completed = 0;
        for (const Quest& quest : player.getQuestLog().getQuests()) if (quest.guildQuest && quest.turnedIn) ++completed;
        struct Threshold { int contracts; int level; const char* rank; };
        const Threshold thresholds[] = {{130,90,"Dieu"},{100,70,"Légende"},{75,55,"Héros mondial"},{55,42,"SSS"},{40,35,"SS"},{28,24,"S"},{20,18,"A"},{14,12,"B"},{9,8,"C"},{5,5,"D"},{2,2,"E"}};
        for (const Threshold& x : thresholds) if (completed >= x.contracts && player.getLevel() >= x.level) return x.rank;
        return "F";
    }

    bool guildRequestRankDUnlocked(const Player& player)
    {
        return guildRankPowerForRequests(guildRankForRequestGate(player)) >= guildRankPowerForRequests("D");
    }

    std::vector<std::string> guildRequestRankGateLines(const Player& player)
    {
        int completed = 0;
        for (const Quest& quest : player.getQuestLog().getQuests()) if (quest.guildQuest && quest.turnedIn) ++completed;
        return {
            "Accès officiel refusé : ces services demandent au moins le rang D de guilde.",
            "Rang actuel : " + guildRankForRequestGate(player) + ".",
            "Progression actuelle : " + std::to_string(completed) + "/5 contrats officiels validés et niveau " + std::to_string(player.getLevel()) + "/5 requis pour le rang D."
        };
    }

    std::string questProgressMethodText(const Quest& quest)
    {
        if (quest.objectiveType == "service") return quest.guildQuest ? "Traite ce service depuis la guilde." : "Retourne parler au PNJ concerné.";
        if (quest.objectiveType == "combat") return "Affronte la cible ou la famille indiquée jusqu'à remplir l'objectif.";
        if (quest.objectiveType == "exploration" || quest.objectiveType == "bestiaire") return "Passe par Exploration et suis la zone conseillée.";
        if (quest.objectiveType == "livraison") return quest.requiredMaterialId.empty() ? "Explore la zone conseillée puis reviens au contact." : "Récupère le matériau demandé puis rends la quête.";
        return "Suis la cible et le lieu conseillés, puis reviens voir le contact.";
    }
}


    std::string normalizedClassForObservation(const Player& player)
    {
        std::string text = player.getType();
        std::transform(text.begin(), text.end(), text.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        return text;
    }

    bool hasFieldObservationSense(const Player& player)
    {
        const std::string className = normalizedClassForObservation(player);
        return player.hasActiveSkill("combat_observation")
            || player.hasPassiveSkill("field_observer")
            || player.hasPassiveSkill("bestiary_family_reader")
            || player.hasPassiveSkill("guild_route_memory")
            || player.hasPassiveSkill("semi_wolf_tracking")
            || player.hasPassiveSkill("semi_fox_cunning")
            || player.hasPassiveSkill("semi_dog_loyal_scent")
            || player.hasPassiveSkill("semi_cat_reflexes")
            || player.hasPassiveSkill("elven_fine_perception")
            || player.hasPassiveSkill("night_vision")
            || className.find("voleur") != std::string::npos
            || className.find("roublard") != std::string::npos
            || className.find("brigand") != std::string::npos
            || className.find("assassin") != std::string::npos
            || className.find("rôdeur") != std::string::npos
            || className.find("rodeur") != std::string::npos;
    }

    std::vector<Quest> collectActiveReadableQuests(const Player& player)
    {
        std::vector<Quest> active;
        for (const Quest& quest : player.getQuestLog().getQuests())
        {
            if (quest.accepted && !quest.turnedIn && !quest.failed)
            {
                active.push_back(quest);
            }
        }
        return active;
    }

    std::string questLocationForObservation(const Quest& quest)
    {
        if (!quest.location.empty()) return quest.location;
        if (!quest.client.empty()) return "Contact : " + quest.client;
        return "Lieu non précisé";
    }

    std::map<std::string, int> countActiveQuestLocations(const std::vector<Quest>& quests)
    {
        std::map<std::string, int> counts;
        for (const Quest& quest : quests)
        {
            ++counts[questLocationForObservation(quest)];
        }
        return counts;
    }

    void maybeUnlockFieldObservation(Player& player, std::vector<std::string>& lines)
    {
        const int observations = player.getCanonicalJournalCategoryTotal("observations_terrain");
        if (observations >= 3 && !player.hasPassiveSkill("field_observer"))
        {
            player.unlockPassiveSkill("field_observer", "Lecture de terrain");
            lines.push_back("Passif débloqué : Lecture de terrain. Tes observations hors combat renforcent aussi Observation tactique en combat.");
        }
        else
        {
            lines.push_back("Progression Lecture de terrain : " + std::to_string(observations) + "/3 observations utiles.");
        }
    }

    void maybeUnlockContractReading(Player& player, std::vector<std::string>& lines)
    {
        const int consultations = player.getCanonicalJournalCategoryTotal("consultations_comptoir_mercenaire");
        if (consultations >= 2 && !player.hasPassiveSkill("contract_reader"))
        {
            player.unlockPassiveSkill("contract_reader", "Lecture de contrat");
            lines.push_back("Passif débloqué : Lecture de contrat. Les mandats et profils mercenaires deviennent plus faciles à comparer avant de payer.");
        }
        else
        {
            lines.push_back("Progression Lecture de contrat : " + std::to_string(consultations) + "/2 consultations du comptoir mercenaire.");
        }
    }

    void openFieldObservationDesk(Player& player)
    {
        while (true)
        {
            const std::vector<Quest> activeQuests = collectActiveReadableQuests(player);
            const std::map<std::string, int> locationCounts = countActiveQuestLocations(activeQuests);
            int groupedLocationCount = 0;
            for (const auto& pair : locationCounts)
            {
                if (pair.second >= 2) ++groupedLocationCount;
            }

            const bool trained = hasFieldObservationSense(player);
            MenuScreen screen("POSTE D'OBSERVATION", "quest.field_observation");
            screen.addLine("Ici, tu lis les contrats, les lieux et les sorties possibles avant de partir. Ce n'est pas une téléportation ni un objectif automatique.");
            screen.addLine("Ville actuelle : " + currentCityName(player) + ".");
            screen.addLine("Lecture actuelle : " + std::string(trained ? "entraînée / intuitive" : "basique") + ".");
            screen.addLine("Quêtes actives lisibles : " + std::to_string(activeQuests.size()) + ". Lieux avec plusieurs objectifs : " + std::to_string(groupedLocationCount) + ".");
            screen.addBackOption("Retour", "quest.field_observation.back");
            screen.addOption(1, "Lire les quêtes actives", "Résumé clair des objectifs, lieux et méthodes probables.", !activeQuests.empty(), "quest.field_observation.active");
            screen.addOption(2, "Repérer les sorties groupées", "Trouver les lieux où plusieurs quêtes peuvent être avancées ensemble.", !activeQuests.empty(), "quest.field_observation.grouped");
            screen.addOption(3, "Faire une ronde d'observation", "Prend 1 segment. Sert à développer Lecture de terrain et à noter les indices de la ville.", true, "quest.field_observation.round");
            screen.addOption(4, "Conseil d'observation", "Explique comment utiliser Voleur/Rôdeur/Observation sans transformer le jeu en GPS gratuit.", true, "quest.field_observation.help");

            const int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choisis une action d'observation.");
            Console::clear();
            if (choice == 0) return;

            if (choice == 1)
            {
                std::vector<std::string> lines;
                lines.push_back("Lecture : " + std::string(trained ? "assez fine pour donner des méthodes probables" : "simple, sans bonus caché"));
                for (const Quest& quest : activeQuests)
                {
                    lines.push_back("- [" + (quest.rank.empty() ? std::string("?") : quest.rank) + "] " + quest.title + " | " + questLocationForObservation(quest) + ".");
                    if (trained)
                    {
                        lines.push_back("  Objectif : " + quest.objective);
                        lines.push_back("  Comment faire : " + questProgressMethodText(quest));
                    }
                    else
                    {
                        lines.push_back("  Indice : entraîne Observation tactique, Voleur/Rôdeur, ou quelques passifs de perception pour lire plus de détails.");
                    }
                }
                MessageScreen::show("LECTURE DES QUÊTES", "quest.field_observation.active.view", lines, false);
                continue;
            }

            if (choice == 2)
            {
                std::vector<std::string> lines;
                lines.push_back("But : montrer quand spammer le même lieu est bénéfique, sans forcer le joueur.");
                bool anyGrouped = false;
                for (const auto& pair : locationCounts)
                {
                    if (pair.second >= 2)
                    {
                        anyGrouped = true;
                        lines.push_back("- " + pair.first + " : " + std::to_string(pair.second) + " objectif(s) actifs. Même sortie possible.");
                    }
                }
                if (!anyGrouped)
                {
                    lines.push_back("Aucun gros regroupement actuel. Prendre une future quête du même lieu peut devenir intéressant si la récompense ou le risque convient.");
                }
                if (trained)
                {
                    lines.push_back("Lecture entraînée : priorise les lieux où tu peux rendre, récolter ou combattre dans la même sortie.");
                }
                MessageScreen::show("SORTIES GROUPÉES", "quest.field_observation.grouped.view", lines, false);
                continue;
            }

            if (choice == 3)
            {
                player.advanceWorldDayUnits(1);
                player.recordCanonicalEvent("observations_terrain", player.getCurrentCityId(), "Observation de terrain à " + currentCityName(player));
                recordRecentAction(player, "field_observation", "Observation de terrain à " + currentCityName(player));
                std::vector<std::string> lines;
                lines.push_back("Tu prends le temps de lire les traces, les affiches, les routes et les allées entre les comptoirs.");
                lines.push_back("Temps passé : 1 segment.");
                lines.push_back(player.formatWorldDateTimeLine());
                if (!activeQuests.empty())
                {
                    lines.push_back("Tu notes " + std::to_string(activeQuests.size()) + " quête(s) active(s) sur lesquelles cette lecture peut aider.");
                }
                maybeUnlockFieldObservation(player, lines);
                MessageScreen::show("RONDE D'OBSERVATION", "quest.field_observation.round.done", lines, false);
                continue;
            }

            if (choice == 4)
            {
                MessageScreen::show(
                    "CONSEIL D'OBSERVATION",
                    "quest.field_observation.help.view",
                    {
                        "Observation ne doit pas remplacer le JDR : elle donne des indices de lieu, de risque et de méthode, pas la solution complète.",
                        "Voleur/Roublard/Brigand : utile pour lire les contrats suspects, les coffres, les sorties discrètes et les mauvais coups.",
                        "Rôdeur/Assassin/perception raciale : utile pour routes, traces, doublons de lieu et ennemis avant combat.",
                        "En combat, Observation tactique consomme un vrai tour et une récupération : elle prépare une faille au lieu de juste augmenter les dégâts.",
                        "Après 3 rondes utiles, Lecture de terrain améliore légèrement l'observation en combat et les lectures hors combat. GG, c'est du vrai apprentissage, pas un cheat code."
                    },
                    false
                );
                continue;
            }
        }
    }

    void openMercenaryCounter(Player& player)
    {
        player.recordCanonicalEvent("consultations_comptoir_mercenaire", player.getCurrentCityId(), "Consultation du comptoir mercenaire à " + currentCityName(player));
        std::vector<std::string> lines;
        lines.push_back("Le comptoir mercenaire regroupe les missions déléguées : groupes connus, demandes légales, risques illégaux et quêtes publiées.");
        lines.push_back("Ce n'est pas une seconde guilde gratuite : les coûts, rangs, suspensions et refus restent ceux du système existant.");
        lines.push_back("Ville actuelle : " + currentCityName(player) + ".");
        maybeUnlockContractReading(player, lines);
        MessageScreen::show("COMPTOIR MERCENAIRE", "quest.mercenary_counter.intro", lines, false);
        QuestContractorMenu::openDelegatedMissionBoard(player);
    }

    bool activeTitleEquals(const Player& player, const std::string& titleName)
    {
        const std::vector<std::string>& activeTitles = player.getActiveTitles();
        return std::find(activeTitles.begin(), activeTitles.end(), titleName) != activeTitles.end();
    }

    void unequipActiveTitleIfEquipped(Player& player, const std::string& titleName)
    {
        const std::vector<std::string> activeTitles = player.getActiveTitles();
        for (std::size_t i = 0; i < activeTitles.size(); ++i)
        {
            if (activeTitles[i] == titleName)
            {
                player.unequipActiveTitleSlot(static_cast<int>(i));
                return;
            }
        }
    }

    int extractRecruitIntField(const std::string& label, const std::string& fieldName, int fallback);
    std::string extractRecruitStringField(const std::string& label, const std::string& fieldName, const std::string& fallback);

    bool recruitIsDismissed(const Player& player, const std::string& recruitName)
    {
        return canonicalRecordCount(player, "recrues_renvoyees", recruitName) > 0;
    }

    std::string canonicalRecordLabel(const Player& player, const std::string& category, const std::string& key, const std::string& fallback = "")
    {
        for (const PlayerJournalRecord& record : player.getCanonicalJournalRecords())
        {
            if (record.category == category && record.key == key)
            {
                return record.label.empty() ? fallback : record.label;
            }
        }
        return fallback;
    }

    bool recruitIsInInfirmary(const Player& player, const std::string& recruitName)
    {
        if (recruitName.empty())
        {
            return false;
        }
        return canonicalRecordCount(player, "recrues_infirmerie_sejours", recruitName)
            > canonicalRecordCount(player, "recrues_infirmerie_recuperees", recruitName);
    }

    bool recruitAwaitingInfirmaryTransfer(const Player& player, const std::string& recruitName)
    {
        if (recruitName.empty())
        {
            return false;
        }
        return canonicalRecordCount(player, "recrues_a_evacuer", recruitName)
            > canonicalRecordCount(player, "recrues_evacuees_infirmerie", recruitName);
    }

    std::string recruitAwaitingTransferLabel(const Player& player, const std::string& recruitName)
    {
        return canonicalRecordLabel(player, "recrues_a_evacuer", recruitName, "gravite=surveillance | repos_base=1 | sortie_pct=50");
    }

    int recruitInfirmaryReadyDay(const Player& player, const std::string& recruitName)
    {
        const std::string label = canonicalRecordLabel(player, "recrues_infirmerie_sejours", recruitName);
        return extractRecruitIntField(label, "pret_jour", player.getWorldDaysElapsed());
    }

    std::string recruitInfirmarySeverity(const Player& player, const std::string& recruitName)
    {
        const std::string label = canonicalRecordLabel(player, "recrues_infirmerie_sejours", recruitName, "gravite=surveillance");
        return extractRecruitStringField(label, "gravite", "surveillance");
    }

    bool recruitIsEquipped(const Player& player, const std::string& recruitName)
    {
        if (recruitIsDismissed(player, recruitName) || recruitIsInInfirmary(player, recruitName) || recruitAwaitingInfirmaryTransfer(player, recruitName))
        {
            return false;
        }
        return (canonicalRecordCount(player, "recrues_equipees_toggle", recruitName) % 2) == 1;
    }

    bool guildGroupTestPassed(const Player& player)
    {
        return player.getCanonicalJournalCategoryTotal("tests_groupe_guilde_reussis") > 0;
    }

    int retainedRecruitCandidateCount(const Player& player)
    {
        int total = 0;
        for (const PlayerJournalRecord& record : player.getCanonicalJournalRecords())
        {
            if (record.category == "pnj_recrutables_retenus" && record.count > 0 && !recruitIsDismissed(player, record.key))
            {
                ++total;
            }
        }
        return total;
    }

    void maybeShowTorvaldRankDIntro(Player& player)
    {
        if (!guildRequestRankDUnlocked(player))
        {
            return;
        }
        if (canonicalRecordCount(player, "rencontres_torvald", "intro_rang_d") > 0)
        {
            return;
        }

        player.recordCanonicalEvent("rencontres_torvald", "intro_rang_d", "Torvald se présente au rang D");
        MessageScreen::show(
            "NOUVEAU CONTACT DE GUILDE",
            "quest.guild.torvald.rank_d_intro",
            {
                "Un homme large d'épaules attend près du panneau de guilde, bras croisés, sourire tranquille.",
                "« Rang D, hein ? À partir de maintenant, tu peux embaucher légalement au lieu de tout faire comme un héros épuisé. »",
                "« Je m'appelle Torvald. Entraîneur de guilde. Je peux t'expliquer les équipes, les recrues, et te proposer un 1v1 amical. »",
                "Torvald rejoint les PNJ notables. Tu peux le retrouver dans les lieux notables ou la liste des PNJ."
            },
            false
        );
    }

    struct RecruitCandidatePreview
    {
        std::string name;
        std::string race;
        std::string job;
        int level;
        int rewardSharePercent;
        int weeklySalaryCopper;
        bool personalQuestRequired;
        std::string trait;
        std::string originRank;
        int originRankIndex = 0;
        int currentRankIndex = 0;
        int loyaltyDiscountPercent = 0;
    };

    struct RecruitTeamSharePreview
    {
        std::string name;
        std::string label;
        std::string race = "Inconnue";
        std::string job = "profil libre";
        std::string trait = "trait non relu";
        int retainedTraceCount = 1;
        int recruitedLevel = 1;
        int estimatedLevel = 1;
        int originRankIndex = 0;
        int currentRankIndex = 0;
        int requestedShare = 10;
        int normalizedShare = 0;
        int weeklySalaryCopper = 20;
        int loyaltyDiscountPercent = 0;
        bool personalQuestRequired = true;
        bool personalQuestCompleted = false;
        int personalQuestProgress = 0;
        bool activeEquipped = false;
        bool inInfirmary = false;
        bool awaitingInfirmaryTransfer = false;
        bool infirmaryReady = false;
        int infirmaryReadyDay = 0;
        int infirmaryExitHealthPercent = 50;
        int pendingRecoveryDays = 1;
        int currentHp = 1;
        int maxHp = 1;
        int potionCharges = 0;
        int weaponQuality = 1;
        int armorQuality = 1;
        int supportKitQuality = 0;
        std::string infirmarySeverity;
    };

    std::string recruitHpLine(const RecruitTeamSharePreview& recruit);

    std::string recruitRankNameFromIndex(int index)
    {
        static const std::vector<std::string> ranks = {"F", "E", "D", "C", "B", "A", "S"};
        index = std::clamp(index, 0, static_cast<int>(ranks.size()) - 1);
        return ranks[static_cast<std::size_t>(index)];
    }

    int recruitRankIndexFromLevel(int level)
    {
        if (level >= 70) return 6;
        if (level >= 52) return 5;
        if (level >= 38) return 4;
        if (level >= 26) return 3;
        if (level >= 15) return 2;
        if (level >= 8) return 1;
        return 0;
    }

    int recruitMaxShareCapForGroupSize(int groupSizeIncludingPlayer)
    {
        if (groupSizeIncludingPlayer >= 5) return 22;
        if (groupSizeIncludingPlayer == 4) return 25;
        if (groupSizeIncludingPlayer == 3) return 30;
        if (groupSizeIncludingPlayer == 2) return 35;
        return 0;
    }

    int recruitPlayerMinimumShareForGroupSize(int groupSizeIncludingPlayer)
    {
        if (groupSizeIncludingPlayer <= 1) return 100;
        const int averageShare = 100 / std::max(1, groupSizeIncludingPlayer);
        return std::clamp(averageShare + 10, 30, 65);
    }

    int extractRecruitIntField(const std::string& label, const std::string& fieldName, int fallback)
    {
        const std::string marker = fieldName + "=";
        const std::size_t pos = label.find(marker);
        if (pos == std::string::npos)
        {
            return fallback;
        }
        std::size_t start = pos + marker.size();
        std::size_t end = start;
        if (end < label.size() && label[end] == '-')
        {
            ++end;
        }
        while (end < label.size() && std::isdigit(static_cast<unsigned char>(label[end])))
        {
            ++end;
        }
        if (end <= start)
        {
            return fallback;
        }
        try
        {
            return std::stoi(label.substr(start, end - start));
        }
        catch (...)
        {
            return fallback;
        }
    }

    bool extractRecruitBoolField(const std::string& label, const std::string& fieldName, bool fallback)
    {
        const int value = extractRecruitIntField(label, fieldName, fallback ? 1 : 0);
        return value != 0;
    }

    std::string extractRecruitStringField(const std::string& label, const std::string& fieldName, const std::string& fallback)
    {
        const std::string marker = fieldName + "=";
        const std::size_t pos = label.find(marker);
        if (pos == std::string::npos)
        {
            return fallback;
        }
        const std::size_t start = pos + marker.size();
        std::size_t end = label.find(" | ", start);
        if (end == std::string::npos)
        {
            end = label.size();
        }
        if (end <= start)
        {
            return fallback;
        }
        return label.substr(start, end - start);
    }

    int playerGuildRankIndexForTeam(const Player& player)
    {
        const std::string rank = guildRankForRequestGate(player);
        if (rank.find("S") != std::string::npos) return 6;
        if (rank.find("A") != std::string::npos) return 5;
        if (rank.find("B") != std::string::npos) return 4;
        if (rank.find("C") != std::string::npos) return 3;
        if (rank.find("D") != std::string::npos) return 2;
        if (rank.find("E") != std::string::npos) return 1;
        return 0;
    }

    int requiredLevelForRecruitRankIndex(int rankIndex)
    {
        static const std::vector<int> requiredLevels = {1, 8, 15, 26, 38, 52, 70};
        rankIndex = std::clamp(rankIndex, 0, static_cast<int>(requiredLevels.size()) - 1);
        return requiredLevels[static_cast<std::size_t>(rankIndex)];
    }

    int stableRecruitRoll(const std::string& seed, int minValue, int maxValue)
    {
        if (maxValue < minValue)
        {
            return minValue;
        }
        unsigned int hash = 2166136261u;
        for (unsigned char c : seed)
        {
            hash ^= c;
            hash *= 16777619u;
        }
        const int span = maxValue - minValue + 1;
        return minValue + static_cast<int>(hash % static_cast<unsigned int>(span));
    }

    int estimateRecruitMaxHp(const std::string& profile, int level, int rankIndex)
    {
        int maxHp = std::max(18, 34 + std::max(1, level) * 3 + std::clamp(rankIndex, 0, 6) * 15);
        std::string lower = profile;
        std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        if (lower.find("gardien") != std::string::npos || lower.find("tank") != std::string::npos)
        {
            maxHp = maxHp * 120 / 100;
        }
        if (lower.find("soigneur") != std::string::npos || lower.find("mage d'appui") != std::string::npos)
        {
            maxHp = maxHp * 95 / 100;
        }
        return std::max(1, maxHp);
    }

    std::string makeRecruitRecordLabel(const RecruitCandidatePreview& candidate)
    {
        std::ostringstream oss;
        oss << candidate.name
            << " | race=" << candidate.race
            << " | profil=" << candidate.job
            << " | trait=" << candidate.trait
            << " | niveau_recrutement=" << candidate.level
            << " | rang_origine=" << candidate.originRank
            << " | rang_origine_index=" << candidate.originRankIndex
            << " | rang_actuel_index=" << candidate.currentRankIndex
            << " | part_base=" << candidate.rewardSharePercent
            << " | salaire=" << candidate.weeklySalaryCopper
            << " | quete_perso=" << (candidate.personalQuestRequired ? 1 : 0)
            << " | remise_fidelite=" << candidate.loyaltyDiscountPercent;
        return oss.str();
    }

    bool recruitPersonalQuestCompleted(const Player& player, const std::string& recruitName)
    {
        return canonicalRecordCount(player, "quetes_personnelles_recrues_terminees", recruitName) > 0;
    }

    int recruitPersonalQuestProgress(const Player& player, const std::string& recruitName)
    {
        return canonicalRecordCount(player, "progres_quetes_personnelles_recrues", recruitName);
    }

    std::string recruitPersonalQuestTheme(const RecruitTeamSharePreview& recruit)
    {
        std::string text = recruit.job + " " + recruit.trait;
        std::transform(text.begin(), text.end(), text.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        if (text.find("soigneur") != std::string::npos || text.find("mage d'appui") != std::string::npos)
        {
            return "Aider " + recruit.name + " à sécuriser des plantes, bandages et fioles propres pour son kit de soutien.";
        }
        if (text.find("gardien") != std::string::npos || text.find("tank") != std::string::npos)
        {
            return "Accompagner " + recruit.name + " pendant une ronde dangereuse pour prouver que la ligne peut tenir sans sacrifier personne.";
        }
        if (text.find("roublard") != std::string::npos || text.find("assassin") != std::string::npos || text.find("brigand") != std::string::npos)
        {
            return "Régler discrètement une ancienne dette de rue de " + recruit.name + " sans transformer la mission en bain de sang.";
        }
        if (text.find("artisan") != std::string::npos)
        {
            return "Trouver les pièces manquantes pour que " + recruit.name + " répare son équipement personnel.";
        }
        if (text.find("archer") != std::string::npos || text.find("éclaireur") != std::string::npos || text.find("eclaireur") != std::string::npos)
        {
            return "Aider " + recruit.name + " à cartographier une route sûre et à confirmer un vieux repère de tir.";
        }
        return "Faire une mission personnelle courte avec " + recruit.name + " pour comprendre ses limites, sa loyauté et ce qu'il attend du clan.";
    }

    RecruitTeamSharePreview recruitTeamPreviewFromRecord(const Player& player, const PlayerJournalRecord& record)
    {
        RecruitTeamSharePreview preview;
        preview.name = record.key.empty() ? record.label : record.key;
        preview.label = record.label.empty() ? preview.name : record.label;
        preview.race = extractRecruitStringField(preview.label, "race", "Inconnue");
        preview.job = extractRecruitStringField(preview.label, "profil", "profil libre");
        preview.trait = extractRecruitStringField(preview.label, "trait", "trait non relu");
        preview.retainedTraceCount = std::max(1, record.count);
        preview.recruitedLevel = std::max(1, extractRecruitIntField(preview.label, "niveau_recrutement", std::max(1, player.getLevel() - 2)));
        const int gainedSinceRetention = std::min(18, std::max(0, record.count - 1) * 2 + stableRecruitRoll(preview.name + ":growth", 0, 4));
        preview.estimatedLevel = std::max(preview.recruitedLevel, std::min(100, preview.recruitedLevel + gainedSinceRetention));
        preview.originRankIndex = std::clamp(extractRecruitIntField(preview.label, "rang_origine_index", recruitRankIndexFromLevel(preview.recruitedLevel)), 0, 6);
        int promotionCount = 0;
        const std::string promotionPrefix = preview.name + ":";
        for (const PlayerJournalRecord& promotionRecord : player.getCanonicalJournalRecords())
        {
            if (promotionRecord.category == "rangs_recrues_obtenus" &&
                promotionRecord.key.rfind(promotionPrefix, 0) == 0 &&
                promotionRecord.count > 0)
            {
                promotionCount += promotionRecord.count;
            }
        }
        preview.currentRankIndex = std::max(preview.originRankIndex, recruitRankIndexFromLevel(preview.estimatedLevel));
        preview.currentRankIndex = std::clamp(preview.currentRankIndex + promotionCount, preview.originRankIndex, 6);
        preview.requestedShare = std::clamp(extractRecruitIntField(preview.label, "part_base", 12 + preview.currentRankIndex * 4), 6, 40);
        preview.weeklySalaryCopper = std::max(20, extractRecruitIntField(preview.label, "salaire", 30 + preview.estimatedLevel * 10));
        preview.personalQuestRequired = extractRecruitBoolField(preview.label, "quete_perso", true);
        preview.personalQuestCompleted = recruitPersonalQuestCompleted(player, preview.name);
        preview.personalQuestProgress = recruitPersonalQuestProgress(player, preview.name);
        const int earlyInvestmentDiscount = std::max(0, preview.currentRankIndex - preview.originRankIndex) * 2;
        const int storedDiscount = extractRecruitIntField(preview.label, "remise_fidelite", std::max(0, 7 - preview.originRankIndex));
        const int personalQuestDiscount = preview.personalQuestCompleted ? 3 : 0;
        preview.loyaltyDiscountPercent = std::clamp(storedDiscount + earlyInvestmentDiscount + personalQuestDiscount, 0, 17);
        preview.requestedShare = std::clamp(preview.requestedShare - preview.loyaltyDiscountPercent, 6, 40);
        if (preview.personalQuestCompleted)
        {
            preview.weeklySalaryCopper = std::max(15, preview.weeklySalaryCopper * 95 / 100);
        }
        preview.maxHp = estimateRecruitMaxHp(preview.job + " " + preview.trait, preview.estimatedLevel, preview.currentRankIndex);
        const int initialPercent = stableRecruitRoll(preview.name + ":initial_hp_percent", 70, 100);
        const std::string vitalsLabel = canonicalRecordLabel(player, "pv_recrues_persistants", preview.name, "");
        preview.currentHp = std::clamp(extractRecruitIntField(vitalsLabel, "hp", std::max(1, preview.maxHp * initialPercent / 100)), 0, preview.maxHp);
        preview.potionCharges = std::clamp(extractRecruitIntField(vitalsLabel, "potions", 1 + preview.currentRankIndex / 2), 0, 5);
        const std::string equipmentLabel = canonicalRecordLabel(player, "equipement_recrues", preview.name, "");
        const int naturalQuality = std::clamp(1 + preview.currentRankIndex + preview.estimatedLevel / 28, 1, 9);
        preview.weaponQuality = std::clamp(extractRecruitIntField(equipmentLabel, "arme", naturalQuality), 1, 10);
        preview.armorQuality = std::clamp(extractRecruitIntField(equipmentLabel, "armure", std::max(1, naturalQuality - 1)), 1, 10);
        preview.supportKitQuality = std::clamp(extractRecruitIntField(equipmentLabel, "kit", preview.job.find("soigneur") != std::string::npos ? 2 : 1), 0, 10);
        return preview;
    }

    int recruitOrderPriority(const Player& player, const std::string& recruitName)
    {
        return canonicalRecordCount(player, "ordre_priorite_recrues", recruitName);
    }

    std::string recruitManualOrderRecordLabel(const Player& player, const std::string& key, const std::string& fallback = "")
    {
        for (const PlayerJournalRecord& record : player.getCanonicalJournalRecords())
        {
            if (record.category == "ordre_manuel_recrues" && record.key == key)
            {
                return record.label.empty() ? fallback : record.label;
            }
        }
        return fallback;
    }

    bool recruitManualOrderEnabled(const Player& player)
    {
        const std::string mode = recruitManualOrderRecordLabel(player, "mode", "auto");
        return mode.find("manuel") != std::string::npos || mode.find("manual") != std::string::npos;
    }

    int recruitManualOrderIndex(const Player& player, const std::string& recruitName)
    {
        if (!recruitManualOrderEnabled(player))
        {
            return 10000;
        }
        for (int slot = 1; slot <= 12; ++slot)
        {
            const std::string name = recruitManualOrderRecordLabel(player, "slot_" + std::to_string(slot));
            if (name == recruitName)
            {
                return slot;
            }
        }
        return 10000;
    }

    std::vector<RecruitTeamSharePreview> collectRecruitTeamPreviews(const Player& player)
    {
        std::vector<RecruitTeamSharePreview> previews;
        for (const PlayerJournalRecord& record : player.getCanonicalJournalRecords())
        {
            if (record.category == "pnj_recrutables_retenus" && record.count > 0 && !recruitIsDismissed(player, record.key))
            {
                RecruitTeamSharePreview preview = recruitTeamPreviewFromRecord(player, record);
                preview.inInfirmary = recruitIsInInfirmary(player, preview.name);
                preview.awaitingInfirmaryTransfer = recruitAwaitingInfirmaryTransfer(player, preview.name);
                const std::string pendingLabel = recruitAwaitingTransferLabel(player, preview.name);
                preview.infirmaryReadyDay = recruitInfirmaryReadyDay(player, preview.name);
                preview.infirmaryReady = preview.inInfirmary && player.getWorldDaysElapsed() >= preview.infirmaryReadyDay;
                preview.infirmarySeverity = preview.awaitingInfirmaryTransfer
                    ? extractRecruitStringField(pendingLabel, "gravite", "surveillance")
                    : recruitInfirmarySeverity(player, preview.name);
                preview.pendingRecoveryDays = extractRecruitIntField(pendingLabel, "repos_base", preview.infirmarySeverity == "grave" ? 3 : 1);
                preview.infirmaryExitHealthPercent = extractRecruitIntField(
                    preview.awaitingInfirmaryTransfer ? pendingLabel : canonicalRecordLabel(player, "recrues_infirmerie_sejours", preview.name, "sortie_pct=50"),
                    "sortie_pct",
                    50
                );
                preview.activeEquipped = recruitIsEquipped(player, preview.name);
                previews.push_back(preview);
            }
        }
        std::sort(previews.begin(), previews.end(), [&](const RecruitTeamSharePreview& a, const RecruitTeamSharePreview& b) {
            if (a.activeEquipped != b.activeEquipped) return a.activeEquipped > b.activeEquipped;
            if (a.inInfirmary != b.inInfirmary) return a.inInfirmary < b.inInfirmary;
            const int manualA = recruitManualOrderIndex(player, a.name);
            const int manualB = recruitManualOrderIndex(player, b.name);
            if (manualA != manualB) return manualA < manualB;

            // FR: fallback automatique seulement quand le joueur n'a pas posé d'ordre manuel.
            const int orderA = recruitOrderPriority(player, a.name);
            const int orderB = recruitOrderPriority(player, b.name);
            if (orderA != orderB) return orderA > orderB;
            if (a.currentRankIndex != b.currentRankIndex) return a.currentRankIndex > b.currentRankIndex;
            if (a.estimatedLevel != b.estimatedLevel) return a.estimatedLevel > b.estimatedLevel;
            return a.name < b.name;
        });
        return previews;
    }

    int normalizeRecruitRewardShares(std::vector<RecruitTeamSharePreview>& recruits)
    {
        const int groupSize = static_cast<int>(recruits.size()) + 1;
        int playerShare = recruitPlayerMinimumShareForGroupSize(groupSize);
        if (recruits.empty())
        {
            return 100;
        }

        const int perRecruitCap = std::min(recruitMaxShareCapForGroupSize(groupSize), std::max(6, playerShare - 1));
        const int available = std::max(0, 100 - playerShare);
        int totalWeight = 0;
        std::vector<int> weights;
        for (const RecruitTeamSharePreview& recruit : recruits)
        {
            const int powerWeight = recruit.currentRankIndex * 3 + recruit.estimatedLevel / 8;
            const int weight = std::clamp(recruit.requestedShare + powerWeight, 5, perRecruitCap * 2);
            weights.push_back(weight);
            totalWeight += weight;
        }

        int assigned = 0;
        for (std::size_t i = 0; i < recruits.size(); ++i)
        {
            int share = totalWeight <= 0
                ? available / static_cast<int>(recruits.size())
                : (weights[i] * available) / totalWeight;
            share = std::clamp(share, recruits.size() <= 1 ? 0 : 5, perRecruitCap);
            recruits[i].normalizedShare = share;
            assigned += share;
        }

        // FR: redistribution douce du reste sans dépasser le plafond, pour éviter des sommes bizarres.
        int safety = 0;
        while (assigned < available && safety < 200)
        {
            bool changed = false;
            for (RecruitTeamSharePreview& recruit : recruits)
            {
                if (assigned >= available) break;
                if (recruit.normalizedShare < perRecruitCap)
                {
                    ++recruit.normalizedShare;
                    ++assigned;
                    changed = true;
                }
            }
            if (!changed) break;
            ++safety;
        }

        playerShare = 100 - assigned;
        const int biggestRecruit = std::max_element(recruits.begin(), recruits.end(), [](const RecruitTeamSharePreview& a, const RecruitTeamSharePreview& b) {
            return a.normalizedShare < b.normalizedShare;
        })->normalizedShare;
        if (playerShare <= biggestRecruit)
        {
            int remainingNeeded = biggestRecruit + 1 - playerShare;
            for (RecruitTeamSharePreview& recruit : recruits)
            {
                if (remainingNeeded <= 0) break;
                if (recruit.normalizedShare > 5)
                {
                    const int taken = std::min(remainingNeeded, recruit.normalizedShare - 5);
                    recruit.normalizedShare -= taken;
                    playerShare += taken;
                    remainingNeeded -= taken;
                }
            }
        }
        return std::clamp(playerShare, 0, 100);
    }

    double computeTeamAverageRankIndex(const Player& player, const std::vector<RecruitTeamSharePreview>& recruits)
    {
        double total = static_cast<double>(playerGuildRankIndexForTeam(player));
        for (const RecruitTeamSharePreview& recruit : recruits)
        {
            total += static_cast<double>(recruit.currentRankIndex);
        }
        return total / static_cast<double>(recruits.size() + 1);
    }

    int computeTeamRankIndex(const Player& player, const std::vector<RecruitTeamSharePreview>& recruits)
    {
        const double average = computeTeamAverageRankIndex(player, recruits);
        return std::clamp(static_cast<int>(average + 0.5), 0, 6);
    }

    std::string formatRankAverage(double value)
    {
        std::ostringstream oss;
        oss.setf(std::ios::fixed);
        oss.precision(2);
        oss << value;
        return oss.str();
    }

    std::vector<std::string> buildTeamRankLines(const Player& player, const std::vector<RecruitTeamSharePreview>& recruits)
    {
        const int playerRankIndex = playerGuildRankIndexForTeam(player);
        const double average = computeTeamAverageRankIndex(player, recruits);
        const int teamRank = computeTeamRankIndex(player, recruits);
        std::vector<std::string> lines;
        lines.push_back("Rang du chef : " + guildRankForRequestGate(player) + " / " + recruitRankNameFromIndex(playerRankIndex) + ".");
        lines.push_back("Rang moyen du groupe : " + formatRankAverage(average) + " → rang de groupe " + recruitRankNameFromIndex(teamRank) + ".");
        lines.push_back("Lecture : le rang de groupe vient de la moyenne joueur + recrues retenues. Il servira à filtrer les quêtes de groupe et l'ambition du clan.");
        return lines;
    }

    std::vector<std::string> buildTeamRewardShareLines(const Player& player)
    {
        std::vector<RecruitTeamSharePreview> recruits = collectRecruitTeamPreviews(player);
        std::vector<std::string> lines;
        const int groupSize = static_cast<int>(recruits.size()) + 1;
        const int playerShare = normalizeRecruitRewardShares(recruits);
        lines.push_back("Groupe compté : joueur + " + std::to_string(recruits.size()) + " recrue(s) retenue(s), soit " + std::to_string(groupSize) + " personne(s).");
        const std::vector<std::string> rankLines = buildTeamRankLines(player, recruits);
        lines.insert(lines.end(), rankLines.begin(), rankLines.end());
        lines.push_back("Part du chef : " + std::to_string(playerShare) + "% minimum effectif. Le joueur reste toujours au-dessus de chaque recrue.");
        lines.push_back("Plafond recrue actuel : " + std::to_string(recruitMaxShareCapForGroupSize(groupSize)) + "% avant normalisation, et jamais plus de 40% en demande brute.");
        if (recruits.empty())
        {
            lines.push_back("Aucune recrue retenue pour le moment.");
            return lines;
        }
        for (const RecruitTeamSharePreview& recruit : recruits)
        {
            lines.push_back("- " + recruit.name + " : " + std::to_string(recruit.normalizedShare) + "% | " + std::string(recruit.activeEquipped ? "équipé" : "réserve") + " | rang estimé " + recruitRankNameFromIndex(recruit.currentRankIndex) + " | niveau estimé " + std::to_string(recruit.estimatedLevel) + " | remise investissement " + std::to_string(recruit.loyaltyDiscountPercent) + "%.");
        }
        lines.push_back("Lecture : une recrue prise tôt garde une demande plus basse quand elle monte, donc l'investissement reste rentable sans devenir abusé.");
        return lines;
    }

    std::vector<std::string> buildRecruitDetailedLines(const Player& player, const RecruitTeamSharePreview& recruit)
    {
        std::vector<std::string> lines;
        lines.push_back("Nom : " + recruit.name + ".");
        lines.push_back("Race : " + recruit.race + " | profil : " + recruit.job + ".");
        lines.push_back("Trait lu : " + recruit.trait + ".");
        lines.push_back("Niveau recrutement : " + std::to_string(recruit.recruitedLevel) + " | niveau estimé actuel : " + std::to_string(recruit.estimatedLevel) + ".");
        lines.push_back("Rang origine : " + recruitRankNameFromIndex(recruit.originRankIndex) + " | rang estimé actuel : " + recruitRankNameFromIndex(recruit.currentRankIndex) + ".");
        lines.push_back("Statut actif : " + std::string(recruit.activeEquipped ? "équipé dans l'équipe active" : "en réserve / pas équipé") + ".");
        lines.push_back("Demande actuelle après fidélité : " + std::to_string(recruit.requestedShare) + "% | remise investissement : " + std::to_string(recruit.loyaltyDiscountPercent) + "%.");
        lines.push_back("Salaire hebdomadaire de base : " + Money::formatCopper(recruit.weeklySalaryCopper) + ".");
        lines.push_back("PV sauvegardés : " + recruitHpLine(recruit) + " | potions personnelles : " + std::to_string(recruit.potionCharges) + ".");
        lines.push_back("Équipement personnel : arme qualité " + std::to_string(recruit.weaponQuality) + ", armure qualité " + std::to_string(recruit.armorQuality) + ", kit soutien qualité " + std::to_string(recruit.supportKitQuality) + ".");
        lines.push_back(recruit.personalQuestCompleted
            ? "Quête personnelle : terminée. La confiance baisse légèrement sa demande et son salaire."
            : (recruit.personalQuestRequired
                ? "Quête personnelle : en attente. Sujet : " + recruitPersonalQuestTheme(recruit)
                : "Quête personnelle : non obligatoire pour ce profil, cas plus rare."));
        if (!recruit.personalQuestCompleted && recruit.personalQuestProgress > 0)
        {
            lines.push_back("Progression quête personnelle : " + std::to_string(recruit.personalQuestProgress) + "/2 trace(s) utile(s).");
        }
        const int simulatedParticipation = canonicalRecordCount(player, "participation_recrues", recruit.name);
        const int equippedToggles = canonicalRecordCount(player, "recrues_equipees_toggle", recruit.name);
        lines.push_back("Traces d'activité : " + std::to_string(simulatedParticipation) + " participation(s), " + std::to_string(equippedToggles) + " changement(s) d'équipement actif.");
        lines.push_back("Important : la réduction de salaire ne dépend pas juste de la participation. Elle dépend du fait d'être équipé ET d'avoir touché assez de parts de récompense sur la semaine.");
        return lines;
    }

    int equippedRecruitCount(const Player& player)
    {
        int total = 0;
        for (const RecruitTeamSharePreview& recruit : collectRecruitTeamPreviews(player))
        {
            if (recruit.activeEquipped)
            {
                ++total;
            }
        }
        return total;
    }

    std::string clanNameFromJournal(const Player& player)
    {
        for (const PlayerJournalRecord& record : player.getCanonicalJournalRecords())
        {
            if (record.category == "clan_nom" && record.key == "nom_actuel" && !record.label.empty())
            {
                return record.label;
            }
        }
        return "Groupe sans nom";
    }

    std::string cleanClanName(const std::string& raw)
    {
        std::string cleaned;
        for (char c : raw)
        {
            const unsigned char uc = static_cast<unsigned char>(c);
            if (std::isalnum(uc) || c == ' ' || c == '-' || c == '_' || c == '\'' )
            {
                cleaned.push_back(c);
            }
        }
        while (!cleaned.empty() && std::isspace(static_cast<unsigned char>(cleaned.front())))
        {
            cleaned.erase(cleaned.begin());
        }
        while (!cleaned.empty() && std::isspace(static_cast<unsigned char>(cleaned.back())))
        {
            cleaned.pop_back();
        }
        if (cleaned.empty())
        {
            cleaned = "Clan sans bannière";
        }
        if (cleaned.size() > 40)
        {
            cleaned.resize(40);
        }
        return cleaned;
    }

    std::string recruitWeekKey(const RecruitTeamSharePreview& recruit, int weekIndex)
    {
        return recruit.name + ":semaine_" + std::to_string(weekIndex);
    }

    std::string recruitEquippedWeekKey(const RecruitTeamSharePreview& recruit, int weekIndex)
    {
        return recruit.name + ":semaine_" + std::to_string(weekIndex);
    }

    bool recruitWasEquippedDuringWeek(const Player& player, const RecruitTeamSharePreview& recruit, int weekIndex)
    {
        return canonicalRecordCount(player, "recrues_equipees_semaines", recruitEquippedWeekKey(recruit, weekIndex)) > 0;
    }

    int recruitWeeklyRewardCopper(const Player& player, const RecruitTeamSharePreview& recruit, int weekIndex)
    {
        return canonicalRecordCount(player, "parts_or_recrues", recruitWeekKey(recruit, weekIndex));
    }

    int computeRecruitSalaryAfterWeeklyRewards(const Player& player, const RecruitTeamSharePreview& recruit, int weekIndex)
    {
        const int rewardCopper = recruitWeeklyRewardCopper(player, recruit, weekIndex);
        const bool equippedThisWeek = recruitWasEquippedDuringWeek(player, recruit, weekIndex);
        if (!equippedThisWeek || rewardCopper < recruit.weeklySalaryCopper)
        {
            return recruit.weeklySalaryCopper;
        }
        const int coveredCopper = rewardCopper - recruit.weeklySalaryCopper;
        const int maxDiscount = recruit.weeklySalaryCopper / 2;
        const int discount = std::clamp(coveredCopper / 2, 5, std::max(5, maxDiscount));
        return std::max(5, recruit.weeklySalaryCopper - discount);
    }

    int hpTargetNinetyPercent(int maxHp)
    {
        return std::max(1, std::max(1, maxHp) * 90 / 100);
    }

    int computePlayerInfirmaryHealCostCopper(const Player& player)
    {
        const int targetHp = hpTargetNinetyPercent(player.getMaxHp());
        const int missing = std::max(0, targetHp - player.getHp());
        if (missing <= 0)
        {
            return 0;
        }
        return std::max(20, 35 + missing * (2 + std::max(0, player.getLevel()) / 12));
    }

    int computeRecruitInfirmaryHealCostCopper(const RecruitTeamSharePreview& recruit)
    {
        const int targetHp = hpTargetNinetyPercent(recruit.maxHp);
        const int missing = std::max(0, targetHp - recruit.currentHp);
        if (missing <= 0)
        {
            return 0;
        }
        return std::max(18, 25 + missing * (2 + std::clamp(recruit.currentRankIndex, 0, 6) / 2));
    }

    std::string recruitHpLine(const RecruitTeamSharePreview& recruit)
    {
        return std::to_string(std::clamp(recruit.currentHp, 0, std::max(1, recruit.maxHp)))
            + "/" + std::to_string(std::max(1, recruit.maxHp)) + " PV";
    }

    std::string makeRecruitEquipmentLabelForDay(const RecruitTeamSharePreview& recruit, int weaponQuality, int armorQuality, int supportKitQuality, int day)
    {
        std::ostringstream oss;
        oss << recruit.name
            << " | arme=" << std::clamp(weaponQuality, 1, 10)
            << " | armure=" << std::clamp(armorQuality, 1, 10)
            << " | kit=" << std::clamp(supportKitQuality, 0, 10)
            << " | jour=" << day;
        return oss.str();
    }

    int totalInfirmaryDebtCopper(const Player& player, const std::string& debtCategory)
    {
        int total = 0;
        for (const PlayerJournalRecord& record : player.getCanonicalJournalRecords())
        {
            if (record.category == debtCategory)
            {
                total += std::max(0, record.count);
            }
        }
        const std::string paymentKey = debtCategory == "dettes_infirmerie_groupe" ? "groupe" : "joueur";
        total -= canonicalRecordCount(player, "paiements_dettes_infirmerie", paymentKey);
        return std::max(0, total);
    }

    int totalInfirmaryDebtCopper(const Player& player)
    {
        return totalInfirmaryDebtCopper(player, "dettes_infirmerie_joueur")
            + totalInfirmaryDebtCopper(player, "dettes_infirmerie_groupe");
    }

    int infirmaryRevivalDebtThresholdCopper()
    {
        return static_cast<int>(Money::copperFromGold(100));
    }

    bool infirmaryDebtBlocksHealing(const Player& player)
    {
        return totalInfirmaryDebtCopper(player) > 0;
    }

    bool infirmaryDebtBlocksRecruitRevival(const Player& player)
    {
        return totalInfirmaryDebtCopper(player) > infirmaryRevivalDebtThresholdCopper();
    }

    std::vector<std::string> buildInfirmaryDebtBlockLines(const Player& player, bool revivalBlock)
    {
        const int totalDebt = totalInfirmaryDebtCopper(player);
        std::vector<std::string> lines;
        lines.push_back("Dette d'infirmerie restante : " + Money::formatCopper(totalDebt) + ".");
        lines.push_back("Règle : Lysa refuse tout nouveau soin tant qu'une dette d'infirmerie existe.");
        if (revivalBlock)
        {
            lines.push_back("Dette critique : au-dessus de " + Money::formatCopper(infirmaryRevivalDebtThresholdCopper()) + ", l'infirmerie bloque aussi les réanimations/récupérations d'équipiers.");
        }
        lines.push_back("Va dans Dettes d'infirmerie pour payer ce que tu peux avant de demander un soin.");
        return lines;
    }

    bool payInfirmaryDebtCategory(Player& player, const std::string& debtCategory, const std::string& paymentKey, std::vector<std::string>& lines)
    {
        const int debt = totalInfirmaryDebtCopper(player, debtCategory);
        if (debt <= 0)
        {
            lines.push_back("Aucune dette à régler pour cette catégorie.");
            return false;
        }
        const int wallet = player.getInventory().getTotalCopper();
        const int payment = std::min(debt, std::max(0, wallet));
        if (payment <= 0 || !player.getInventory().spendCopper(payment))
        {
            lines.push_back("Impossible de payer maintenant : bourse insuffisante.");
            return false;
        }
        player.recordCanonicalEvent("paiements_dettes_infirmerie", paymentKey, "Paiement dette infirmerie " + paymentKey, payment);
        lines.push_back("Paiement effectué : " + Money::formatCopper(payment) + ". Reste estimé : " + Money::formatCopper(std::max(0, debt - payment)) + ".");
        return true;
    }

    void openInfirmaryDebtMenu(Player& player)
    {
        while (true)
        {
            const int personalDebt = totalInfirmaryDebtCopper(player, "dettes_infirmerie_joueur");
            const int groupDebt = totalInfirmaryDebtCopper(player, "dettes_infirmerie_groupe");
            MenuScreen screen("DETTES D'INFIRMERIE", "team.infirmary.debts.menu");
            screen.addLine("Dette personnelle : " + Money::formatCopper(personalDebt) + ".");
            screen.addLine("Dette de groupe : " + Money::formatCopper(groupDebt) + ".");
            screen.addLine("Bourse actuelle : " + player.getInventory().getWalletLine() + ".");
            screen.addBackOption("Retour", "team.infirmary.debts.back");
            screen.addOption(1, "Payer dette personnelle", "Règle autant que possible avec ta bourse actuelle.", personalDebt > 0, "team.infirmary.debts.pay_player");
            screen.addOption(2, "Payer dette de groupe", "Règle autant que possible la dette liée au sauvetage par l'équipe.", groupDebt > 0, "team.infirmary.debts.pay_group");
            screen.addOption(3, "Voir le registre", "Afficher un résumé des dettes/frais enregistrés.", true, "team.infirmary.debts.log");
            const int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choisis une action de dette.");
            Console::clear();
            if (choice == 0) return;
            std::vector<std::string> lines;
            if (choice == 1)
            {
                payInfirmaryDebtCategory(player, "dettes_infirmerie_joueur", "joueur", lines);
                MessageScreen::show("DETTE PERSONNELLE", "team.infirmary.debts.player", lines, false);
            }
            else if (choice == 2)
            {
                payInfirmaryDebtCategory(player, "dettes_infirmerie_groupe", "groupe", lines);
                MessageScreen::show("DETTE DE GROUPE", "team.infirmary.debts.group", lines, false);
            }
            else if (choice == 3)
            {
                lines.push_back("Résumé des dettes/frais d'infirmerie :");
                lines.push_back("- Dette personnelle restante : " + Money::formatCopper(personalDebt) + ".");
                lines.push_back("- Dette de groupe restante : " + Money::formatCopper(groupDebt) + ".");
                lines.push_back("- Paiements personnels enregistrés : " + Money::formatCopper(canonicalRecordCount(player, "paiements_dettes_infirmerie", "joueur")) + ".");
                lines.push_back("- Paiements groupe enregistrés : " + Money::formatCopper(canonicalRecordCount(player, "paiements_dettes_infirmerie", "groupe")) + ".");
                MessageScreen::show("REGISTRE INFIRMERIE", "team.infirmary.debts.log", lines, false);
            }
        }
    }

    void healPlayerAtInfirmary(Player& player)
    {
        if (infirmaryDebtBlocksHealing(player))
        {
            MessageScreen::show(
                "SOINS BLOQUÉS",
                "team.infirmary.player.debt_block",
                buildInfirmaryDebtBlockLines(player, false),
                false
            );
            return;
        }

        const int targetHp = hpTargetNinetyPercent(player.getMaxHp());
        if (player.getHp() >= targetHp)
        {
            MessageScreen::show(
                "INFIRMERIE",
                "team.infirmary.player.no_need",
                {
                    "Tu es déjà au seuil de soin payant de l'infirmerie.",
                    "PV actuels : " + std::to_string(player.getHp()) + "/" + std::to_string(player.getMaxHp()) + ".",
                    "L'infirmerie payante soigne jusqu'à 90% des PV, pas forcément jusqu'au maximum."
                },
                false
            );
            return;
        }

        const int cost = computePlayerInfirmaryHealCostCopper(player);
        const int healed = targetHp - player.getHp();
        MenuScreen screen("SE SOIGNER", "team.infirmary.player.heal");
        screen.addLine("L'infirmerie peut te remettre jusqu'à 90% de tes PV si tu paies les soins.");
        screen.addLine("PV : " + std::to_string(player.getHp()) + "/" + std::to_string(player.getMaxHp()) + " -> " + std::to_string(targetHp) + "/" + std::to_string(player.getMaxHp()) + ".");
        screen.addLine("Coût : " + Money::formatCopper(cost) + " | Bourse : " + player.getInventory().getWalletLine() + ".");
        screen.addBackOption("Retour", "team.infirmary.player.back");
        screen.addOption(1, "Payer les soins", "Récupère " + std::to_string(healed) + " PV, sans dépasser 90%.", true, "team.infirmary.player.pay");
        const int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choisis une action.");
        Console::clear();
        if (choice != 1)
        {
            return;
        }
        if (!player.getInventory().spendCopper(cost))
        {
            MessageScreen::show("SOINS IMPOSSIBLES", "team.infirmary.player.no_money", {"Il te manque de l'argent pour payer l'infirmerie.", "Coût : " + Money::formatCopper(cost) + ".", "Bourse actuelle : " + player.getInventory().getWalletLine() + "."}, false);
            return;
        }
        player.heal(healed);
        player.recordCanonicalEvent("soins_infirmerie_joueur", player.getCurrentCityId(), "Soin payant jusqu'à 90% PV", cost);
        MessageScreen::show("SOINS TERMINÉS", "team.infirmary.player.done", {"L'infirmerie te soigne jusqu'au seuil de 90%.", "PV actuels : " + std::to_string(player.getHp()) + "/" + std::to_string(player.getMaxHp()) + ".", "Payé : " + Money::formatCopper(cost) + "."}, false);
    }

    void healRecruitAtInfirmary(Player& player)
    {
        if (infirmaryDebtBlocksHealing(player))
        {
            MessageScreen::show(
                "SOINS D'ÉQUIPE BLOQUÉS",
                "team.infirmary.recruit.debt_block",
                buildInfirmaryDebtBlockLines(player, false),
                false
            );
            return;
        }

        std::vector<RecruitTeamSharePreview> recruits = collectRecruitTeamPreviews(player);
        std::vector<RecruitTeamSharePreview> healable;
        for (const RecruitTeamSharePreview& recruit : recruits)
        {
            if (recruit.inInfirmary || recruit.awaitingInfirmaryTransfer)
            {
                continue;
            }
            if (recruit.currentHp < hpTargetNinetyPercent(recruit.maxHp))
            {
                healable.push_back(recruit);
            }
        }

        if (healable.empty())
        {
            MessageScreen::show(
                "SOIGNER UN MEMBRE",
                "team.infirmary.recruit.no_need",
                {
                    "Aucune recrue disponible n'a besoin d'un soin payant jusqu'à 90%.",
                    "Les recrues KO doivent d'abord être amenées à l'infirmerie dans le menu des évacuations."
                },
                false
            );
            return;
        }

        MenuScreen screen("SOIGNER UN MEMBRE", "team.infirmary.recruit.heal.menu");
        screen.addLine("Choisis une recrue disponible à soigner. L'infirmerie remonte au maximum à 90% des PV si tu paies.");
        screen.addBackOption("Retour", "team.infirmary.recruit.heal.back");
        for (std::size_t i = 0; i < healable.size(); ++i)
        {
            const RecruitTeamSharePreview& recruit = healable[i];
            const int targetHp = hpTargetNinetyPercent(recruit.maxHp);
            const int cost = computeRecruitInfirmaryHealCostCopper(recruit);
            screen.addOption(
                static_cast<int>(i + 1),
                "Soigner " + recruit.name,
                recruitHpLine(recruit) + " -> " + std::to_string(targetHp) + "/" + std::to_string(recruit.maxHp) + " | coût " + Money::formatCopper(cost),
                true,
                "team.infirmary.recruit.heal.select"
            );
        }

        const int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choisis une recrue à soigner.");
        Console::clear();
        if (choice <= 0 || choice > static_cast<int>(healable.size()))
        {
            return;
        }

        const RecruitTeamSharePreview recruit = healable[static_cast<std::size_t>(choice - 1)];
        const int targetHp = hpTargetNinetyPercent(recruit.maxHp);
        const int healed = std::max(0, targetHp - recruit.currentHp);
        const int cost = computeRecruitInfirmaryHealCostCopper(recruit);
        if (healed <= 0 || cost <= 0)
        {
            MessageScreen::show("SOINS INUTILES", "team.infirmary.recruit.heal.skip", {recruit.name + " n'a pas besoin de soin payant pour atteindre 90%."}, false);
            return;
        }
        if (!player.getInventory().spendCopper(cost))
        {
            MessageScreen::show("SOINS IMPOSSIBLES", "team.infirmary.recruit.heal.no_money", {"Tu n'as pas assez d'argent pour soigner " + recruit.name + ".", "Coût : " + Money::formatCopper(cost) + ".", "Bourse actuelle : " + player.getInventory().getWalletLine() + "."}, false);
            return;
        }

        player.recordCanonicalEvent("pv_recrues_persistants", recruit.name, recruit.name + " | hp=" + std::to_string(targetHp) + " | max=" + std::to_string(recruit.maxHp) + " | potions=" + std::to_string(recruit.potionCharges) + " | statut=soin_paye_90 | jour=" + std::to_string(player.getWorldDaysElapsed()), 1);
        player.recordCanonicalEvent("soins_infirmerie_recrues", recruit.name, recruit.name + " soigné jusqu'à 90% PV", cost);
        MessageScreen::show("SOINS TERMINÉS", "team.infirmary.recruit.heal.done", {recruit.name + " est soigné jusqu'au seuil de 90%.", "PV : " + std::to_string(targetHp) + "/" + std::to_string(recruit.maxHp) + " (gain " + std::to_string(healed) + ").", "Payé : " + Money::formatCopper(cost) + "."}, false);
    }

    void showTeamRewardSharePreview(Player& player)
    {
        MessageScreen::show("PARTS DE GROUPE", "team.reward_shares.preview", buildTeamRewardShareLines(player), false);
    }

    struct RecruitRankAttemptReadiness
    {
        bool eligible = false;
        int chance = 0;
        std::string nextRank;
        std::string reason;
    };

    int recruitJobPromotionModifier(const RecruitTeamSharePreview& recruit)
    {
        const std::string job = recruit.job;
        if (job.find("éclaireur") != std::string::npos || job.find("roublard") != std::string::npos || job.find("archer") != std::string::npos)
        {
            return 8;
        }
        if (job.find("gardien") != std::string::npos || job.find("soigneur") != std::string::npos || job.find("mage") != std::string::npos)
        {
            return 5;
        }
        if (job.find("artisan") != std::string::npos)
        {
            return 2;
        }
        return 0;
    }

    int recruitTraitPromotionModifier(const RecruitTeamSharePreview& recruit)
    {
        const std::string trait = recruit.trait;
        if (trait.find("loyal") != std::string::npos || trait.find("contrats") != std::string::npos)
        {
            return 7;
        }
        if (trait.find("prudent") != std::string::npos || trait.find("peur") != std::string::npos)
        {
            return -5;
        }
        if (trait.find("exploration") != std::string::npos || trait.find("potions") != std::string::npos)
        {
            return 3;
        }
        return 0;
    }

    RecruitRankAttemptReadiness recruitRankAttemptReadiness(const RecruitTeamSharePreview& recruit, const Player& player)
    {
        RecruitRankAttemptReadiness readiness;
        const int nextRank = recruit.currentRankIndex + 1;
        readiness.nextRank = recruitRankNameFromIndex(std::min(nextRank, 6));
        if (nextRank > 6)
        {
            readiness.reason = "déjà au rang maximum visible";
            return readiness;
        }

        const int requiredLevel = requiredLevelForRecruitRankIndex(nextRank);
        if (recruit.estimatedLevel < requiredLevel)
        {
            readiness.reason = "niveau insuffisant pour viser le rang " + readiness.nextRank + " (niveau " + std::to_string(requiredLevel) + " conseillé)";
            return readiness;
        }

        const int participation = canonicalRecordCount(player, "participation_recrues", recruit.name);
        const int autonomousActions = canonicalRecordCount(player, "achats_recrues", recruit.name + ":potions")
            + canonicalRecordCount(player, "achats_recrues", recruit.name + ":equipement");
        const bool classMakesSense = recruitJobPromotionModifier(recruit) >= 2;
        const bool enoughActivity = participation > 0 || autonomousActions > 0 || recruit.retainedTraceCount >= 2;
        if (!enoughActivity && !classMakesSense)
        {
            readiness.reason = "pas assez d'activité utile récente pour justifier une demande de rang";
            return readiness;
        }

        const int levelPressure = recruit.estimatedLevel - requiredLevel;
        const int baseChance = 22
            + levelPressure * 3
            + participation * 9
            + autonomousActions * 3
            + recruitJobPromotionModifier(recruit)
            + recruitTraitPromotionModifier(recruit)
            + std::max(0, recruit.currentRankIndex - recruit.originRankIndex);
        readiness.eligible = true;
        readiness.chance = std::clamp(baseChance, 12, 82);
        readiness.reason = "conditions remplies : niveau, activité et profil cohérents";
        return readiness;
    }

    void runRecruitWeeklyManagement(Player& player)
    {
        std::vector<RecruitTeamSharePreview> recruits = collectRecruitTeamPreviews(player);
        if (recruits.empty())
        {
            MessageScreen::show("BILAN DE GROUPE", "team.weekly.empty", {"Aucune recrue retenue pour le moment."}, false);
            return;
        }

        const int weekIndex = std::max(0, player.getWorldDaysElapsed() / 7);
        const std::string weekKey = "semaine_" + std::to_string(weekIndex);
        if (canonicalRecordCount(player, "bilan_hebdo_recrues", weekKey) > 0)
        {
            MessageScreen::show(
                "BILAN DÉJÀ FAIT",
                "team.weekly.already_done",
                {
                    "Les recrues ont déjà fait leur bilan de semaine.",
                    "Semaine actuelle : " + std::to_string(weekIndex) + ".",
                    "Le prochain bilan pourra se faire après le passage à une nouvelle semaine."
                },
                false
            );
            return;
        }

        Random random;
        std::vector<std::string> lines;
        lines.push_back("Semaine " + std::to_string(weekIndex) + " : Torvald rassemble les notes de groupe.");
        for (const RecruitTeamSharePreview& recruit : recruits)
        {
            const int weeklyReward = recruitWeeklyRewardCopper(player, recruit, weekIndex);
            const int salary = computeRecruitSalaryAfterWeeklyRewards(player, recruit, weekIndex);
            std::string salaryLine = "- " + recruit.name + " : salaire demandé " + Money::formatCopper(salary) + ".";
            const bool equippedThisWeek = recruitWasEquippedDuringWeek(player, recruit, weekIndex);
            if (equippedThisWeek && weeklyReward >= recruit.weeklySalaryCopper)
            {
                salaryLine += " Réduit car la recrue a été équipée cette semaine et ses parts reçues " + Money::formatCopper(weeklyReward) + " couvrent le salaire de base " + Money::formatCopper(recruit.weeklySalaryCopper) + ".";
                player.recordCanonicalEvent("salaires_recrues_reduits", recruit.name, recruit.name + " réduit son salaire grâce aux parts hebdo", 1);
            }
            else if (equippedThisWeek)
            {
                salaryLine += " Pas de réduction : elle a été équipée, mais ses parts reçues " + Money::formatCopper(weeklyReward) + " restent sous le salaire de base " + Money::formatCopper(recruit.weeklySalaryCopper) + ".";
            }
            else
            {
                salaryLine += " Pas de réduction : aucune trace d'équipement actif cette semaine.";
            }
            lines.push_back(salaryLine);

            const bool lowOnHealing = recruit.potionCharges <= 1 || random.between(1, 100) <= 45;
            if (lowOnHealing)
            {
                const int potionBudget = std::max(8, 8 + recruit.currentRankIndex * 7 + random.between(0, 14));
                const int newPotions = std::clamp(recruit.potionCharges + 1 + random.between(0, 1), 0, 5);
                player.recordCanonicalEvent("achats_recrues", recruit.name + ":potions", recruit.name + " rachète des potions", potionBudget);
                player.recordCanonicalEvent("pv_recrues_persistants", recruit.name, recruit.name + " | hp=" + std::to_string(recruit.currentHp) + " | max=" + std::to_string(recruit.maxHp) + " | potions=" + std::to_string(newPotions) + " | statut=stock_potions_autonome | jour=" + std::to_string(player.getWorldDaysElapsed()), 1);
                lines.push_back("  Achat autonome : refait son stock de potions " + std::to_string(recruit.potionCharges) + " -> " + std::to_string(newPotions) + " (budget " + Money::formatCopper(potionBudget) + ").");
            }
            else
            {
                const int gearBudget = std::max(10, 12 + recruit.currentRankIndex * 10 + random.between(0, 20));
                int newWeapon = recruit.weaponQuality;
                int newArmor = recruit.armorQuality;
                int newKit = recruit.supportKitQuality;
                if (recruit.job.find("soigneur") != std::string::npos || recruit.job.find("mage") != std::string::npos)
                {
                    newKit = std::min(10, newKit + 1);
                }
                else if (recruit.job.find("gardien") != std::string::npos || recruit.job.find("tank") != std::string::npos)
                {
                    newArmor = std::min(10, newArmor + 1);
                }
                else
                {
                    newWeapon = std::min(10, newWeapon + 1);
                }
                player.recordCanonicalEvent("achats_recrues", recruit.name + ":equipement", recruit.name + " améliore son équipement personnel", gearBudget);
                player.recordCanonicalEvent("equipement_recrues", recruit.name, makeRecruitEquipmentLabelForDay(recruit, newWeapon, newArmor, newKit, player.getWorldDaysElapsed()), 1);
                lines.push_back("  Achat autonome : équipement personnel amélioré (arme " + std::to_string(newWeapon) + ", armure " + std::to_string(newArmor) + ", kit " + std::to_string(newKit) + ", budget " + Money::formatCopper(gearBudget) + ").");
            }

            const RecruitRankAttemptReadiness readiness = recruitRankAttemptReadiness(recruit, player);
            if (readiness.eligible)
            {
                const int roll = random.between(1, 100);
                player.recordCanonicalEvent("tentatives_rang_recrues", recruit.name, recruit.name + " tente une montée de rang", 1);
                if (roll <= readiness.chance)
                {
                    player.recordCanonicalEvent("rangs_recrues_obtenus", recruit.name + ":" + readiness.nextRank, recruit.name + " obtient le rang " + readiness.nextRank, 1);
                    lines.push_back("  Rang : tentative cohérente réussie vers le rang " + readiness.nextRank + " (" + std::to_string(readiness.chance) + "%).");
                }
                else
                {
                    lines.push_back("  Rang : tentative cohérente échouée vers le rang " + readiness.nextRank + " (" + std::to_string(readiness.chance) + "%).");
                }
            }
            else
            {
                lines.push_back("  Rang : aucune tentative cette semaine — " + readiness.reason + ".");
            }
        }
        player.recordCanonicalEvent("bilan_hebdo_recrues", weekKey, "Bilan hebdomadaire des recrues", 1);
        MessageScreen::show("BILAN HEBDOMADAIRE DES RECRUES", "team.weekly.report", lines, false);
    }

    void inspectTeamRoster(Player& player)
    {
        std::vector<RecruitTeamSharePreview> recruits = collectRecruitTeamPreviews(player);
        if (recruits.empty())
        {
            MessageScreen::show("ÉQUIPE", "team.roster.empty", {"Aucune recrue retenue pour le moment.", "Va voir Torvald au rang D pour inspecter des candidats."}, false);
            return;
        }

        while (true)
        {
            MenuScreen screen("INSPECTION D'ÉQUIPE", "team.roster.inspect");
            screen.addLine("Inspection rapide : nom, rang, niveau estimé, salaire et demande de part.");
            screen.addBackOption("Retour", "team.roster.back");
            for (std::size_t i = 0; i < recruits.size(); ++i)
            {
                const RecruitTeamSharePreview& recruit = recruits[i];
                screen.addOption(
                    static_cast<int>(i + 1),
                    recruit.name,
                    "Rang " + recruitRankNameFromIndex(recruit.currentRankIndex) + " | niv. " + std::to_string(recruit.estimatedLevel) + " | salaire " + Money::formatCopper(recruit.weeklySalaryCopper),
                    true,
                    "team.roster.member"
                );
            }
            const int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choisis une recrue à inspecter.");
            Console::clear();
            if (choice == 0) return;
            if (choice >= 1 && choice <= static_cast<int>(recruits.size()))
            {
                MessageScreen::show("FICHE DE RECRUE", "team.roster.member.detail", buildRecruitDetailedLines(player, recruits[static_cast<std::size_t>(choice - 1)]), false);
            }
        }
    }

    void recordManualRecruitOrder(Player& player, const std::vector<RecruitTeamSharePreview>& orderedRecruits)
    {
        player.recordCanonicalEvent("ordre_manuel_recrues", "mode", "manuel", 1);
        int slot = 1;
        std::set<std::string> usedNames;
        for (const RecruitTeamSharePreview& recruit : orderedRecruits)
        {
            if (recruit.name.empty() || usedNames.count(recruit.name) > 0 || recruitIsDismissed(player, recruit.name))
            {
                continue;
            }
            player.recordCanonicalEvent("ordre_manuel_recrues", "slot_" + std::to_string(slot), recruit.name, 1);
            usedNames.insert(recruit.name);
            ++slot;
        }
    }

    std::vector<RecruitTeamSharePreview> reorderActiveRecruitChain(
        const std::vector<RecruitTeamSharePreview>& recruits,
        const std::string& recruitName,
        int movement
    )
    {
        std::vector<RecruitTeamSharePreview> active;
        std::vector<RecruitTeamSharePreview> reserve;
        for (const RecruitTeamSharePreview& recruit : recruits)
        {
            if (recruit.activeEquipped)
            {
                active.push_back(recruit);
            }
            else
            {
                reserve.push_back(recruit);
            }
        }

        auto it = std::find_if(active.begin(), active.end(), [&](const RecruitTeamSharePreview& recruit) {
            return recruit.name == recruitName;
        });
        if (it == active.end())
        {
            return recruits;
        }
        const std::size_t currentIndex = static_cast<std::size_t>(std::distance(active.begin(), it));
        std::size_t targetIndex = currentIndex;
        if (movement == -100)
        {
            targetIndex = 0;
        }
        else if (movement == 100)
        {
            targetIndex = active.empty() ? 0 : active.size() - 1;
        }
        else if (movement < 0 && currentIndex > 0)
        {
            targetIndex = currentIndex - 1;
        }
        else if (movement > 0 && currentIndex + 1 < active.size())
        {
            targetIndex = currentIndex + 1;
        }

        if (targetIndex != currentIndex)
        {
            RecruitTeamSharePreview moved = active[currentIndex];
            active.erase(active.begin() + static_cast<std::ptrdiff_t>(currentIndex));
            active.insert(active.begin() + static_cast<std::ptrdiff_t>(targetIndex), moved);
        }

        active.insert(active.end(), reserve.begin(), reserve.end());
        return active;
    }

    void openRecruitOrderMoveMenu(Player& player, const RecruitTeamSharePreview& selectedRecruit)
    {
        while (true)
        {
            std::vector<RecruitTeamSharePreview> recruits = collectRecruitTeamPreviews(player);
            std::vector<RecruitTeamSharePreview> active;
            for (const RecruitTeamSharePreview& recruit : recruits)
            {
                if (recruit.activeEquipped)
                {
                    active.push_back(recruit);
                }
            }

            int position = -1;
            for (std::size_t i = 0; i < active.size(); ++i)
            {
                if (active[i].name == selectedRecruit.name)
                {
                    position = static_cast<int>(i) + 1;
                    break;
                }
            }
            if (position < 0)
            {
                MessageScreen::show("ORDRE D'ÉQUIPE", "team.order.member.not_active", {selectedRecruit.name + " n'est plus dans l'équipe active."}, false);
                return;
            }

            MenuScreen screen("PLACER UNE RECRUE", "team.order.move");
            screen.addLine("Recrue : " + selectedRecruit.name + " | position active actuelle : " + std::to_string(position) + "/" + std::to_string(active.size()) + ".");
            screen.addLine("Le mode manuel garde la chaîne choisie. Si une recrue est renvoyée, les suivantes se décalent naturellement.");
            screen.addBackOption("Retour", "team.order.move.back");
            screen.addOption(1, "Monter d'une place", "Passe juste avant la recrue précédente.", position > 1, "team.order.move.up");
            screen.addOption(2, "Descendre d'une place", "Passe juste après la recrue suivante.", position < static_cast<int>(active.size()), "team.order.move.down");
            screen.addOption(3, "Mettre en premier", "Place cette recrue juste après le joueur.", position > 1, "team.order.move.top");
            screen.addOption(4, "Mettre en dernier", "Place cette recrue après les autres recrues actives.", position < static_cast<int>(active.size()), "team.order.move.bottom");
            const int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choisis le déplacement.");
            Console::clear();
            if (choice == 0)
            {
                return;
            }

            int movement = 0;
            if (choice == 1) movement = -1;
            else if (choice == 2) movement = 1;
            else if (choice == 3) movement = -100;
            else if (choice == 4) movement = 100;
            else continue;

            const std::vector<RecruitTeamSharePreview> ordered = reorderActiveRecruitChain(recruits, selectedRecruit.name, movement);
            recordManualRecruitOrder(player, ordered);
            MessageScreen::show(
                "ORDRE MANUEL ENREGISTRÉ",
                "team.order.manual.saved",
                {
                    selectedRecruit.name + " change de position dans l'équipe active.",
                    "Le tri automatique rang/niveau reste seulement le fallback quand aucun ordre manuel n'est actif.",
                    "Si une recrue de la chaîne est renvoyée, les autres gardent leur ordre relatif."
                },
                false
            );
            return;
        }
    }

    void showTeamOrderMenu(Player& player)
    {
        while (true)
        {
            std::vector<RecruitTeamSharePreview> recruits = collectRecruitTeamPreviews(player);
            std::vector<RecruitTeamSharePreview> active;
            std::vector<RecruitTeamSharePreview> reserve;
            for (const RecruitTeamSharePreview& recruit : recruits)
            {
                if (recruit.activeEquipped)
                {
                    active.push_back(recruit);
                }
                else
                {
                    reserve.push_back(recruit);
                }
            }

            MenuScreen screen("ORDRE D'ÉQUIPE", "team.order.menu");
            screen.addLine("Le chef joue toujours en premier. L'ordre manuel concerne les recrues actives après lui.");
            screen.addLine("Mode actuel : " + std::string(recruitManualOrderEnabled(player) ? "manuel" : "automatique") + ". Auto = priorité/rang/niveau seulement par défaut.");
            screen.addLine("Si une recrue est renvoyée, la chaîne ne casse pas : les suivantes se décalent.");
            screen.addBackOption("Retour", "team.order.back");
            screen.addLine("Chef : " + player.getName() + " — toujours premier.");

            int option = 1;
            for (const RecruitTeamSharePreview& recruit : active)
            {
                screen.addOption(
                    option,
                    "Déplacer " + recruit.name,
                    "Actif | position " + std::to_string(option) + " après le chef | rang " + recruitRankNameFromIndex(recruit.currentRankIndex) + " | niv. " + std::to_string(recruit.estimatedLevel),
                    active.size() > 1,
                    "team.order.member.move"
                );
                ++option;
            }
            for (const RecruitTeamSharePreview& recruit : reserve)
            {
                screen.addLine("- " + recruit.name + " — réserve, garde sa place relative si elle rejoint plus tard.");
            }
            if (active.empty())
            {
                screen.addLine("Aucune recrue équipée : équipe au moins une recrue avant de fixer un ordre utile.");
            }
            screen.addOption(90, "Fixer l'ordre actuel en manuel", "Enregistre la chaîne affichée comme ordre choisi par le joueur.", !active.empty(), "team.order.lock_current");
            screen.addOption(91, "Revenir à l'ordre automatique", "Réutilise priorité/rang/niveau comme comportement par défaut.", recruitManualOrderEnabled(player), "team.order.auto");

            const int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choisis une recrue à déplacer ou une action.");
            Console::clear();
            if (choice == 0) return;
            if (choice == 90)
            {
                recordManualRecruitOrder(player, recruits);
                MessageScreen::show("ORDRE MANUEL ENREGISTRÉ", "team.order.locked", {"L'ordre affiché devient l'ordre manuel.", "Le tri automatique rang/niveau ne reprendra que si tu repasses en automatique."}, false);
                continue;
            }
            if (choice == 91)
            {
                player.recordCanonicalEvent("ordre_manuel_recrues", "mode", "auto", 1);
                MessageScreen::show("ORDRE AUTOMATIQUE", "team.order.auto", {"L'ordre manuel est désactivé.", "Les recrues actives seront de nouveau classées par priorité/rang/niveau, donc seulement le fallback par défaut."}, false);
                continue;
            }
            if (choice >= 1 && choice <= static_cast<int>(active.size()))
            {
                openRecruitOrderMoveMenu(player, active[static_cast<std::size_t>(choice - 1)]);
            }
        }
    }

    struct GroupQuestOffer
    {
        std::string id;
        std::string name;
        std::string description;
        int minimumRankIndex = 0;
        int baseRewardCopper = 80;
        int baseExperience = 8;
        int risk = 20;
    };

    std::vector<GroupQuestOffer> buildGroupQuestOffers()
    {
        return {
            {"patrol", "Patrouille coordonnée", "Sécuriser une route proche avec un binôme actif.", 0, 110, 10, 18},
            {"escort", "Escorte sous contrat", "Protéger un marchand ou un messager sur une courte distance.", 1, 170, 15, 28},
            {"den", "Nettoyage de tanière", "Entrer, tenir la ligne et ressortir avec tout le monde debout.", 2, 260, 24, 42},
            {"relay", "Relais de crise", "Aider une guilde locale à gérer plusieurs fronts en même temps.", 3, 390, 34, 55},
            {"elite", "Contrat d'équipe élite", "Mission risquée où la cohésion du clan compte autant que la force brute.", 4, 580, 48, 70},
            {"s_rank", "Mandat de réputation majeure", "Mission très visible : si le groupe assure, son nom commence à circuler.", 5, 820, 68, 82}
        };
    }

    int recruitContributionWeight(const RecruitTeamSharePreview& recruit, Random& random)
    {
        int weight = 20 + recruit.estimatedLevel / 2 + recruit.currentRankIndex * 8 + random.between(0, 35);
        if (recruit.trait.find("loyal") != std::string::npos || recruit.trait.find("contrats") != std::string::npos)
        {
            weight += 8;
        }
        if (recruit.trait.find("peur") != std::string::npos || recruit.trait.find("prudent") != std::string::npos)
        {
            weight -= 5;
        }
        return std::max(8, weight);
    }

    void runGroupQuestBoard(Player& player)
    {
        std::vector<RecruitTeamSharePreview> recruits = collectRecruitTeamPreviews(player);
        if (!guildGroupTestPassed(player))
        {
            MessageScreen::show("QUÊTES DE GROUPE", "team.group_quest.blocked_test", {"La guilde ne propose pas encore de vraies quêtes de groupe.", "Valide d'abord le test de groupe et choisis un nom de clan."}, false);
            return;
        }

        std::vector<RecruitTeamSharePreview> active;
        for (const RecruitTeamSharePreview& recruit : recruits)
        {
            if (recruit.activeEquipped)
            {
                active.push_back(recruit);
            }
        }
        if (active.empty())
        {
            MessageScreen::show("QUÊTES DE GROUPE", "team.group_quest.no_active", {"Aucune recrue équipée.", "La guilde veut au moins un allié actif avant de classer une mission comme quête de groupe."}, false);
            return;
        }

        const int teamRank = computeTeamRankIndex(player, recruits);
        const std::vector<GroupQuestOffer> offers = buildGroupQuestOffers();
        MenuScreen screen("QUÊTES DE GROUPE", "team.group_quest.board");
        screen.addLine("Clan : " + clanNameFromJournal(player) + " | rang de groupe : " + recruitRankNameFromIndex(teamRank) + " | alliés actifs : " + std::to_string(active.size()) + "/2.");
        screen.addLine("Ces missions simulent déjà un partage or/XP par contribution avant le vrai combat allié complet.");
        screen.addBackOption("Retour", "team.group_quest.back");
        for (std::size_t i = 0; i < offers.size(); ++i)
        {
            const GroupQuestOffer& offer = offers[i];
            const bool available = teamRank >= offer.minimumRankIndex;
            screen.addOption(
                static_cast<int>(i + 1),
                offer.name,
                "Rang requis " + recruitRankNameFromIndex(offer.minimumRankIndex) + " | " + offer.description,
                available,
                "team.group_quest.offer"
            );
        }

        const int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choisis une quête de groupe.");
        Console::clear();
        if (choice <= 0 || choice > static_cast<int>(offers.size()))
        {
            return;
        }
        const GroupQuestOffer offer = offers[static_cast<std::size_t>(choice - 1)];
        if (teamRank < offer.minimumRankIndex)
        {
            MessageScreen::show("RANG DE GROUPE INSUFFISANT", "team.group_quest.locked", {"Rang actuel : " + recruitRankNameFromIndex(teamRank) + ".", "Rang requis : " + recruitRankNameFromIndex(offer.minimumRankIndex) + "."}, false);
            return;
        }

        const int weekIndex = std::max(0, player.getWorldDaysElapsed() / 7);
        Random random;
        const int cohesion = 35 + teamRank * 8 + static_cast<int>(active.size()) * 10 + random.between(0, 45);
        const bool success = cohesion >= offer.risk;
        const int rewardCopper = success ? offer.baseRewardCopper + teamRank * 45 + random.between(0, 120) : std::max(20, offer.baseRewardCopper / 4);
        const int rewardXp = success ? offer.baseExperience + teamRank * 4 + random.between(0, 12) : std::max(3, offer.baseExperience / 3);

        const int playerShare = normalizeRecruitRewardShares(recruits);
        int playerContribution = 35 + player.getLevel() / 2 + playerGuildRankIndexForTeam(player) * 9 + random.between(0, 35);
        std::vector<int> recruitContributions;
        int totalContribution = playerContribution;
        for (const RecruitTeamSharePreview& recruit : active)
        {
            const int contribution = recruitContributionWeight(recruit, random);
            recruitContributions.push_back(contribution);
            totalContribution += contribution;
        }
        totalContribution = std::max(1, totalContribution);

        const int playerCopper = std::max(1, (rewardCopper * playerShare) / 100);
        const int playerXp = std::max(1, (rewardXp * playerContribution) / totalContribution);
        player.getInventory().earnCopper(playerCopper);
        player.gainExperience(playerXp);
        player.recordCanonicalEvent("quetes_groupe_terminees", offer.id, offer.name, success ? 1 : 0);
        player.recordCanonicalEvent("or_quetes_groupe_chef", offer.id, offer.name, playerCopper);
        player.recordCanonicalEvent(
            "reputation_clan",
            clanNameFromJournal(player),
            offer.name + (success ? " réussie" : " sécurisée partiellement"),
            success ? 2 : 1
        );

        std::vector<std::string> lines;
        lines.push_back(offer.name + " — " + (success ? "réussite" : "réussite partielle / retrait prudent") + ".");
        lines.push_back("Cohésion : " + std::to_string(cohesion) + " | risque : " + std::to_string(offer.risk) + ".");
        lines.push_back("Récompense totale simulée : " + Money::formatCopper(rewardCopper) + " et " + std::to_string(rewardXp) + " XP d'activité.");
        lines.push_back("Chef : " + Money::formatCopper(playerCopper) + " (" + std::to_string(playerShare) + "%) et " + std::to_string(playerXp) + " XP selon contribution.");

        for (std::size_t i = 0; i < active.size(); ++i)
        {
            const RecruitTeamSharePreview& activeRecruit = active[i];
            int share = activeRecruit.normalizedShare;
            for (const RecruitTeamSharePreview& recruit : recruits)
            {
                if (recruit.name == activeRecruit.name)
                {
                    share = recruit.normalizedShare;
                    break;
                }
            }
            const int copper = std::max(1, (rewardCopper * share) / 100);
            const int xp = std::max(1, (rewardXp * recruitContributions[i]) / totalContribution);
            player.recordCanonicalEvent("participation_recrues", activeRecruit.name, activeRecruit.name + " participe à une quête de groupe", 1);
            player.recordCanonicalEvent("recrues_equipees_semaines", recruitEquippedWeekKey(activeRecruit, weekIndex), activeRecruit.name + " a été équipé cette semaine", 1);
            player.recordCanonicalEvent("parts_or_recrues", recruitWeekKey(activeRecruit, weekIndex), activeRecruit.name + " reçoit une part de quête de groupe", copper);
            player.recordCanonicalEvent("xp_recrues", activeRecruit.name, activeRecruit.name + " gagne de l'expérience de quête de groupe", xp);
            lines.push_back("- " + activeRecruit.name + " : " + Money::formatCopper(copper) + " (" + std::to_string(share) + "%) et " + std::to_string(xp) + " XP contribution.");
        }
        lines.push_back("Note : l'or suit les parts économiques plafonnées, l'XP suit davantage ce que chacun a fait pendant la mission.");
        lines.push_back("Les parts enregistrées compteront au bilan hebdomadaire si elles couvrent le salaire de base.");
        MessageScreen::show("RÉSULTAT DE QUÊTE DE GROUPE", "team.group_quest.result", lines, false);
    }

    void showClanReputationSummary(Player& player)
    {
        const std::string clanName = clanNameFromJournal(player);
        const int reputation = canonicalRecordCount(player, "reputation_clan", clanName);
        const int groupQuests = player.getCanonicalJournalCategoryTotal("quetes_groupe_terminees");
        const int personalQuests = player.getCanonicalJournalCategoryTotal("quetes_personnelles_recrues_terminees");

        std::string tier = "Nom discret";
        std::string meaning = "La guilde connaît surtout le chef, pas encore le clan.";
        if (reputation >= 20)
        {
            tier = "Clan reconnu";
            meaning = "Le nom commence à circuler sérieusement : contrats visibles, attentes plus fortes, meilleurs candidats à terme.";
        }
        else if (reputation >= 12)
        {
            tier = "Groupe respecté";
            meaning = "Les comptoirs et Torvald commencent à parler du groupe comme d'une vraie équipe.";
        }
        else if (reputation >= 6)
        {
            tier = "Nom qui circule";
            meaning = "Quelques aventuriers ont déjà entendu le nom. Les quêtes de groupe deviennent plus crédibles.";
        }
        else if (reputation >= 3)
        {
            tier = "Réputation locale faible";
            meaning = "La guilde sait que le groupe existe, mais elle vérifie encore sa régularité.";
        }

        std::vector<std::string> lines;
        lines.push_back("Clan : " + clanName + ".");
        lines.push_back("Réputation enregistrée : " + std::to_string(reputation) + " trace(s).");
        lines.push_back("Palier : " + tier + ".");
        lines.push_back(meaning);
        lines.push_back("Quêtes de groupe suivies : " + std::to_string(groupQuests) + ".");
        lines.push_back("Quêtes personnelles terminées : " + std::to_string(personalQuests) + ".");
        lines.push_back("Ce système reste progressif : il ne donne pas encore de bonus énorme, mais il prépare la reconnaissance du nom de clan, les candidats recrutables et les contrats visibles.");
        MessageScreen::show("RÉPUTATION DU CLAN", "team.clan_reputation.summary", lines, false);
    }

    void runRecruitPersonalQuestMenu(Player& player)
    {
        std::vector<RecruitTeamSharePreview> recruits = collectRecruitTeamPreviews(player);
        if (recruits.empty())
        {
            MessageScreen::show("QUÊTES PERSONNELLES", "team.personal_quests.empty", {"Aucune recrue retenue pour le moment."}, false);
            return;
        }

        MenuScreen screen("QUÊTES PERSONNELLES", "team.personal_quests.menu");
        screen.addLine("Ces quêtes servent à rendre les recrues plus humaines : loyauté, histoire personnelle, petite remise et meilleure cohésion.");
        screen.addLine("Une recrue à l'infirmerie ou à évacuer ne peut pas avancer sa quête personnelle.");
        screen.addBackOption("Retour", "team.personal_quests.back");
        for (std::size_t i = 0; i < recruits.size(); ++i)
        {
            const RecruitTeamSharePreview& recruit = recruits[i];
            const bool available = !recruit.personalQuestCompleted && !recruit.inInfirmary && !recruit.awaitingInfirmaryTransfer;
            std::string state = recruit.personalQuestCompleted ? "terminée" : (recruit.personalQuestRequired ? "à faire" : "facultative");
            if (recruit.inInfirmary) state = "infirmerie";
            if (recruit.awaitingInfirmaryTransfer) state = "à évacuer";
            screen.addOption(
                static_cast<int>(i + 1),
                recruit.name,
                state + " | rang " + recruitRankNameFromIndex(recruit.currentRankIndex) + " | " + recruitPersonalQuestTheme(recruit),
                available,
                "team.personal_quests.recruit"
            );
        }

        const int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choisis une recrue.");
        Console::clear();
        if (choice <= 0 || choice > static_cast<int>(recruits.size()))
        {
            return;
        }

        RecruitTeamSharePreview recruit = recruits[static_cast<std::size_t>(choice - 1)];
        if (recruit.personalQuestCompleted)
        {
            MessageScreen::show("QUÊTE DÉJÀ TERMINÉE", "team.personal_quests.done_already", {recruit.name + " a déjà réglé sa quête personnelle avec le clan."}, false);
            return;
        }
        if (recruit.inInfirmary || recruit.awaitingInfirmaryTransfer)
        {
            MessageScreen::show("RECRUE INDISPONIBLE", "team.personal_quests.unavailable", {recruit.name + " n'est pas disponible pour une quête personnelle."}, false);
            return;
        }

        Random random;
        const int teamRank = computeTeamRankIndex(player, recruits);
        const int progress = recruitPersonalQuestProgress(player, recruit.name);
        const int preparation = player.getLevel() * 5
            + playerGuildRankIndexForTeam(player) * 10
            + teamRank * 7
            + (recruit.activeEquipped ? 10 : 0)
            + progress * 15
            + (player.hasPassiveSkill("contract_reader") ? 8 : 0)
            + (player.hasPassiveSkill("field_observer") ? 8 : 0)
            + random.between(0, 55);
        const int required = 58 + recruit.currentRankIndex * 8 + random.between(0, 28);
        const bool success = preparation >= required || progress >= 2;

        std::vector<std::string> lines;
        lines.push_back("Sujet : " + recruitPersonalQuestTheme(recruit));
        lines.push_back("Préparation : " + std::to_string(preparation) + " | seuil : " + std::to_string(required) + ".");
        if (!success)
        {
            player.recordCanonicalEvent("progres_quetes_personnelles_recrues", recruit.name, recruit.name + " avance sa quête personnelle", 1);
            player.recordCanonicalEvent("participation_recrues", recruit.name, recruit.name + " travaille sa quête personnelle", 1);
            lines.push_back("Résultat : pas encore terminé, mais une trace de progression est enregistrée.");
            lines.push_back("Après assez de traces, la prochaine tentative peut réussir même si le jet est moyen.");
            MessageScreen::show("QUÊTE PERSONNELLE — PROGRESSION", "team.personal_quests.progress", lines, false);
            return;
        }

        const int xpGain = std::max(2, 3 + teamRank + recruit.currentRankIndex + player.getLevel() / 10);
        player.recordCanonicalEvent("quetes_personnelles_recrues_terminees", recruit.name, recruit.name + " termine sa quête personnelle avec le clan", 1);
        player.recordCanonicalEvent("xp_recrues", recruit.name, recruit.name + " gagne de l'expérience grâce à sa quête personnelle", xpGain);
        player.recordCanonicalEvent("participation_recrues", recruit.name, recruit.name + " renforce sa loyauté envers le clan", 1);
        player.recordCanonicalEvent("reputation_clan", clanNameFromJournal(player), "Quête personnelle terminée pour " + recruit.name, 1);
        lines.push_back("Résultat : quête personnelle terminée.");
        lines.push_back(recruit.name + " fait davantage confiance au clan : sa demande et son salaire baissent légèrement via la loyauté.");
        lines.push_back("XP recrue : +" + std::to_string(xpGain) + ". Réputation de clan : +1 trace.");
        MessageScreen::show("QUÊTE PERSONNELLE TERMINÉE", "team.personal_quests.success", lines, false);
    }

    void showTeamOnlineFeasibility()
    {
        MessageScreen::show(
            "MULTI EN LIGNE — FAISABILITÉ",
            "team.online.feasibility",
            {
                "Oui, c'est possible, mais ce chantier demande une vraie structure réseau, pas une couture posée à la hâte.",
                "Approche conseillée : host/client d'abord. Le joueur hôte garde la sauvegarde officielle, les autres envoient leurs choix de tour.",
                "Phase 1 réaliste : combats coop en ligne + menus partagés simples + sauvegarde côté hôte.",
                "À prévoir : synchronisation des tours, reconnexion, latence, validation anti-fichiers modifiés, conflits de sauvegarde, et affichage terminal/IG cohérent.",
                "À éviter au début : MMO/serveur permanent. Trop lourd pour l'état actuel du projet."
            },
            false
        );
    }

    RecruitCandidatePreview createRecruitCandidatePreview(Player& player, Random& random)
    {
        const std::vector<std::string> names = {
            "Nelia Marchevite", "Brann des Halles", "Yori Fend-brume", "Malo Têtefroide", "Iris des Bornes",
            "Garruk Demi-sourire", "Sela Queue-de-renard", "Owen Sac-sec", "Kaelis Rivecendre", "Mina des Lanternes"
        };
        const std::vector<std::string> races = {
            "Humain", "Elfe", "Nain", "Kitsune", "Semi-dragon", "Orc", "Tieffelin", "Fée"
        };
        const std::vector<std::string> jobs = {
            "éclaireur", "gardien", "soigneur débutant", "roublard", "archer", "artisan de terrain", "mage d'appui"
        };
        const std::vector<std::string> traits = {
            "prudent, demande toujours un plan de sortie",
            "efficace mais négocie chaque prime avec sérieux",
            "aide bien en exploration, moins stable en duel frontal",
            "bon réflexe de potion, mais peur des boss trop bruyants",
            "lit bien les contrats suspects, déteste les ordres flous",
            "loyal si sa quête personnelle est respectée"
        };

        RecruitCandidatePreview preview;
        preview.name = names[static_cast<std::size_t>(random.between(0, static_cast<int>(names.size()) - 1))];
        preview.race = races[static_cast<std::size_t>(random.between(0, static_cast<int>(races.size()) - 1))];
        preview.job = jobs[static_cast<std::size_t>(random.between(0, static_cast<int>(jobs.size()) - 1))];
        preview.trait = traits[static_cast<std::size_t>(random.between(0, static_cast<int>(traits.size()) - 1))];
        preview.level = std::max(1, player.getLevel() + random.between(-4, 3));
        preview.originRankIndex = recruitRankIndexFromLevel(preview.level);
        preview.currentRankIndex = preview.originRankIndex;
        preview.originRank = recruitRankNameFromIndex(preview.originRankIndex);

        const int earlyRecruitDiscount = std::max(0, 7 - preview.originRankIndex);
        const int powerDemand = 9 + preview.originRankIndex * 4 + preview.level / 6 + random.between(0, 7);
        preview.loyaltyDiscountPercent = std::clamp(earlyRecruitDiscount + random.between(0, 3), 0, 12);
        preview.rewardSharePercent = std::clamp(powerDemand - preview.loyaltyDiscountPercent, 8, 40);
        preview.weeklySalaryCopper = std::max(20, 30 + preview.level * 10 + preview.originRankIndex * 20 + random.between(0, 24));
        preview.personalQuestRequired = random.between(1, 100) <= 85;
        return preview;
    }

    void displayRecruitmentRules(Player& player)
    {
        const int retained = retainedRecruitCandidateCount(player);
        std::vector<std::string> lines;
        lines.push_back("Règle de base : le joueur reste chef de groupe/clan et compte toujours dans la limite.");
        lines.push_back("Lecture de groupe : la guilde juge le niveau du meneur et des recrues retenues avant d'ouvrir les contrats.");
        lines.push_back("Taille pensée : joueur + 2 équipés en combat, avec réserve de groupe jusqu'à 5 personnes joueur compris.");
        lines.push_back("PNJ retenus actuellement dans cette base : " + std::to_string(retained) + "/4 recrues possibles avant de former un groupe complet de 5 avec le joueur.");
        lines.push_back("85% des recrues demanderont une quête personnelle avant recrutement réel.");
        lines.push_back("Part de récompense : demande brute plafonnée à 40%, puis normalisée selon la taille du groupe pour que le joueur garde la plus grosse part.");
        lines.push_back("Chef de groupe : le joueur vise au moins 10 points au-dessus de la moyenne théorique, puis récupère le surplus si les plafonds de recrues bloquent.");
        lines.push_back("Investissement : une recrue prise bas rang garde une remise de fidélité quand elle progresse, donc la monter toi-même reste rentable.");
        lines.push_back("Salaire : payé en fin de semaine. Réduction seulement si le PNJ était équipé ET si ses parts de récompense de la semaine couvrent/dépassent son salaire de base.");
        lines.push_back("Autonomie : une recrue pourra refaire son stock de potions ou acheter une arme avec ses gains, surtout si elle tombe à court de soin.");
        lines.push_back("Rang recrue : pas de tentative automatique idiote. La tentative existe seulement si niveau, activité, classe et personnalité la rendent crédible.");
        lines.push_back("Titre On recrute ! : augmente les occasions de rencontrer des PNJ recrutables tant que le groupe n'est pas plein.");
        lines.push_back("Titre Anti clan : si équipé, annule les rencontres de recrutement et indique clairement une équipe fermée.");
        MessageScreen::show("RÈGLES DE RECRUTEMENT", "quest.guild.torvald.recruitment_rules", lines, false);
    }

    void inspectRecruitCandidate(Player& player, Random& random)
    {
        const bool antiClanEquipped = activeTitleEquals(player, "Anti clan");
        if (antiClanEquipped)
        {
            MessageScreen::show(
                "ÉQUIPE FERMÉE",
                "quest.guild.torvald.recruit.blocked_anticlan",
                {
                    "Le titre Anti clan est équipé.",
                    "Torvald comprend le message : aucune rencontre de recrutement n'est proposée tant que ce titre reste équipé.",
                    "Tu peux le retirer depuis le menu Titres si tu veux rouvrir l'équipe."
                },
                false
            );
            return;
        }

        int retained = retainedRecruitCandidateCount(player);
        if (retained >= 4)
        {
            if (activeTitleEquals(player, "On recrute !"))
            {
                unequipActiveTitleIfEquipped(player, "On recrute !");
                MessageScreen::show(
                    "GROUPE COMPLET",
                    "quest.guild.torvald.recruit.full_auto_unequip",
                    {
                        "Le groupe atteint la limite de 5 personnes joueur compris.",
                        "Le titre On recrute ! est déséquipé automatiquement : tu n'as plus besoin d'attirer autant de candidatures.",
                        "Tu peux toujours croiser des recrues plus rarement, mais le bonus actif s'arrête ici."
                    },
                    false
                );
            }
            else
            {
                MessageScreen::show(
                    "GROUPE COMPLET",
                    "quest.guild.torvald.recruit.full",
                    {"La base de groupe est pleine pour le moment : joueur + 4 recrues retenues."},
                    false
                );
            }
            return;
        }

        const bool recruitTitleEquipped = activeTitleEquals(player, "On recrute !");
        const int encounterChance = recruitTitleEquipped ? 80 : 28;
        if (random.between(1, 100) > encounterChance)
        {
            MessageScreen::show(
                "AUCUN CANDIDAT SÉRIEUX",
                "quest.guild.torvald.recruit.none",
                {
                    recruitTitleEquipped
                        ? "Même avec On recrute ! équipé, personne de fiable ne se présente dans cette fenêtre."
                        : "Sans signal clair de recrutement, les candidats sérieux restent rares.",
                    "Sources futures : sauvetage, collaboration de quête, balade à la guilde, ville ou exploration urbaine."
                },
                false
            );
            return;
        }

        RecruitCandidatePreview candidate = createRecruitCandidatePreview(player, random);
        const std::string key = candidate.name + ":jour" + std::to_string(player.getWorldDaysElapsed()) + ":" + std::to_string(retained + 1);
        player.recordCanonicalEvent("candidats_recrutement_inspectes", key, candidate.name + " inspecté par Torvald");

        std::vector<std::string> lines;
        lines.push_back("CV rapide / trait d'identité :");
        lines.push_back("Nom : " + candidate.name + ".");
        lines.push_back("Race : " + candidate.race + " | classe/profil : " + candidate.job + " | niveau estimé : " + std::to_string(candidate.level) + " | rang origine : " + candidate.originRank + ".");
        lines.push_back("Trait : " + candidate.trait + ".");
        lines.push_back("Part demandée brute : " + std::to_string(candidate.rewardSharePercent) + "% avant normalisation de groupe.");
        lines.push_back("Remise investissement précoce : " + std::to_string(candidate.loyaltyDiscountPercent) + "% déjà prise en compte dans sa demande.");
        lines.push_back("Salaire de base hebdomadaire : " + Money::formatCopper(candidate.weeklySalaryCopper) + ".");
        lines.push_back(candidate.personalQuestRequired
            ? "Condition : quête personnelle requise avant recrutement réel."
            : "Condition : peut accepter sans quête personnelle, cas plus rare.");
        lines.push_back("Cette passe crée la base de lecture/retenue. L'équipement réel en équipe sera branché ensuite dans le menu rapide Équipe.");
        MessageScreen::show("CANDIDAT RECRUTABLE", "quest.guild.torvald.recruit.preview", lines, false);

        MenuScreen keepScreen("RETENIR LE PROFIL ?", "quest.guild.torvald.recruit.keep");
        keepScreen.addLine("Retenir le profil marque l'intérêt dans le registre et garde ses bases économiques pour les futurs calculs d'équipe.");
        keepScreen.addBackOption("Ne pas retenir", "quest.guild.torvald.recruit.keep.no");
        keepScreen.addOption(1, "Retenir ce profil", "Ajoute ce candidat aux profils retenus pour la future équipe.", true, "quest.guild.torvald.recruit.keep.yes");
        int keepChoice = TerminalInterface::askMenuChoiceFromOptions(keepScreen, "Choix invalide.");
        Console::clear();
        if (keepChoice == 1)
        {
            player.recordCanonicalEvent("pnj_recrutables_retenus", candidate.name, makeRecruitRecordLabel(candidate));
            retained = retainedRecruitCandidateCount(player);
            std::vector<std::string> keptLines;
            keptLines.push_back(candidate.name + " est retenu dans le registre de recrutement.");
            keptLines.push_back("Profils retenus : " + std::to_string(retained) + "/4 recrues possibles avant groupe complet de 5 avec le joueur.");
            const std::vector<std::string> shareLines = buildTeamRewardShareLines(player);
            keptLines.insert(keptLines.end(), shareLines.begin(), shareLines.end());
            if (retained >= 4 && activeTitleEquals(player, "On recrute !"))
            {
                unequipActiveTitleIfEquipped(player, "On recrute !");
                keptLines.push_back("Groupe complet : On recrute ! est déséquipé automatiquement.");
            }
            MessageScreen::show("PROFIL RETENU", "quest.guild.torvald.recruit.kept", keptLines, false);
        }
    }

    bool runTorvaldFriendlyTrial(Player& player, Random& random)
    {
        const int attempts = player.getCanonicalJournalCategoryTotal("duels_torvald_tentes") + 1;
        player.recordCanonicalEvent("duels_torvald_tentes", "torvald", "Duel amical contre Torvald");

        const int preparation = player.getLevel() * 8
            + player.getUnspentAttributePoints()
            + (player.hasActiveSkill("combat_observation") ? 12 : 0)
            + (player.hasPassiveSkill("field_observer") ? 8 : 0)
            + (player.hasPassiveSkill("contract_reader") ? 4 : 0)
            + random.between(1, 70);
        const int torvaldGuard = 54 + attempts * 2 + random.between(0, 45);
        const bool victory = preparation >= torvaldGuard;

        std::vector<std::string> lines;
        lines.push_back("Torvald garde le combat amical : pas de mort, pas de casse d'équipement, pas de loot.");
        lines.push_back("Essai : " + std::to_string(attempts) + ".");
        lines.push_back("Lecture du joueur : " + std::to_string(preparation) + " | garde de Torvald : " + std::to_string(torvaldGuard) + ".");
        if (victory)
        {
            player.recordCanonicalEvent("duels_torvald_gagnes", "torvald", "Torvald battu en duel amical");
            bool anyTitle = false;
            anyTitle = player.grantTitle("Un bon entraînement") || anyTitle;
            anyTitle = player.grantTitle("Anti clan") || anyTitle;
            anyTitle = player.grantTitle("On recrute !") || anyTitle;
            lines.push_back("Résultat : victoire amicale.");
            lines.push_back("Torvald rit : « Voilà. Tu sais tenir debout, regarder, et écouter. Tu peux commencer à former une équipe. »");
            lines.push_back(anyTitle
                ? "Titres obtenus : Un bon entraînement, Anti clan, On recrute !."
                : "Titres déjà possédés : Un bon entraînement, Anti clan, On recrute !.");
            lines.push_back("On recrute ! augmente les occasions de trouver des PNJ recrutables tant que le groupe n'est pas plein.");
            lines.push_back("Anti clan sert à fermer volontairement l'équipe si tu l'équipes.");
        }
        else
        {
            lines.push_back("Résultat : Torvald gagne proprement.");
            lines.push_back("« Rien de grave. Tu peux retenter quand tu veux. Le but, c'est d'apprendre sans perdre un bras. »");
        }

        MessageScreen::show(
            victory ? "DUEL AMICAL RÉUSSI" : "DUEL AMICAL PERDU",
            victory ? "quest.guild.torvald.duel.victory" : "quest.guild.torvald.duel.defeat",
            lines,
            false
        );
        return victory;
    }

    void openTorvaldGuildTrainer(Player& player)
    {
        if (!guildRequestRankDUnlocked(player))
        {
            MessageScreen::show(
                "TORVALD ABSENT",
                "quest.guild.torvald.locked",
                guildRequestRankGateLines(player),
                false
            );
            return;
        }

        Random random;
        player.recordCanonicalEvent("rencontres_torvald", "notable", "Torvald rejoint les PNJ notables");
        while (true)
        {
            const int retained = retainedRecruitCandidateCount(player);
            MenuScreen screen("TORVALD — ENTRAÎNEUR DE GUILDE", "quest.guild.torvald");
            screen.addLine("Torvald s'occupe des bases d'équipe : embauche légale, discipline de groupe, tests amicaux et premiers candidats.");
            screen.addLine("Ville actuelle : " + currentCityName(player) + ". Rang actuel : " + guildRankForRequestGate(player) + ".");
            const std::vector<RecruitTeamSharePreview> torvaldRecruits = collectRecruitTeamPreviews(player);
            screen.addLine("Profils retenus : " + std::to_string(retained) + "/4 recrues possibles avant groupe complet joueur compris.");
            screen.addLine("Rang moyen de groupe : " + recruitRankNameFromIndex(computeTeamRankIndex(player, torvaldRecruits)) + " (moyenne " + formatRankAverage(computeTeamAverageRankIndex(player, torvaldRecruits)) + ").");
            screen.addLine("Titres équipés : " + player.getActiveTitleSummary() + ".");
            screen.addBackOption("Retour", "quest.guild.torvald.back");
            screen.addOption(1, "Écouter l'explication de Torvald", "Embauche, quêtes personnelles, part de récompense, salaire et groupe de 5.", true, "quest.guild.torvald.rules");
            screen.addOption(2, "1v1 amical contre Torvald", "Test sans mort ni casse. Si tu gagnes, Torvald valide le départ d'équipe.", true, "quest.guild.torvald.duel");
            screen.addOption(3, "Inspecter un candidat recrutable", "Peut afficher un CV/trait d'identité. Bonus si On recrute ! est équipé, blocage si Anti clan est équipé.", true, "quest.guild.torvald.recruit");
            screen.addOption(4, "Prévoir les parts du groupe", "Montre les plafonds et parts normalisées avec le joueur en plus grosse part.", true, "quest.guild.torvald.reward_shares");
            screen.addOption(5, "Bilan hebdomadaire des recrues", "Achats autonomes, salaire et tentative de montée de rang.", true, "quest.guild.torvald.weekly");
            screen.addOption(6, "Ouvrir le menu Équipe", "Accès rapide : inspection, parts, ordre, bilan, quêtes de groupe, multi en ligne.", true, "quest.guild.torvald.team_menu");
            screen.addOption(7, "Quêtes de groupe", "Premières missions de clan filtrées par rang moyen.", true, "quest.guild.torvald.group_quests");
            screen.addOption(8, "Résumé système équipe", "Rappelle ce qui existe maintenant et ce qui sera branché ensuite.", true, "quest.guild.torvald.team_summary");

            const int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choisis une action auprès de Torvald.");
            Console::clear();
            if (choice == 0) return;
            if (choice == 1)
            {
                displayRecruitmentRules(player);
            }
            else if (choice == 2)
            {
                runTorvaldFriendlyTrial(player, random);
            }
            else if (choice == 3)
            {
                inspectRecruitCandidate(player, random);
            }
            else if (choice == 4)
            {
                showTeamRewardSharePreview(player);
            }
            else if (choice == 5)
            {
                runRecruitWeeklyManagement(player);
            }
            else if (choice == 6)
            {
                QuestMenu::openTeamMenu(player);
            }
            else if (choice == 7)
            {
                runGroupQuestBoard(player);
            }
            else if (choice == 8)
            {
                MessageScreen::show(
                    "SYSTÈME ÉQUIPE — BASE",
                    "quest.guild.torvald.team_summary",
                    {
                        "Présent maintenant : arrivée de Torvald au rang D, duel amical, titres de recrutement, blocage Anti clan, CV de candidats, registre de profils retenus et menu Équipe hors combat.",
                        "Présent maintenant : prévisualisation des parts, plafonds par taille de groupe, remise de fidélité pour recrue prise tôt, bilan hebdomadaire, achats autonomes et tentatives de rang.",
                        "Présent maintenant : test de groupe à la guilde, nom de clan, équipement/déséquipement de 2 recrues actives, mission courte, renvoi confirmé et économie hebdo corrigée.",
                        "Présent maintenant : premières quêtes de groupe selon rang moyen, ordre d'équipe priorisable, part or plafonnée et XP selon contribution simulée.",
                        "À brancher ensuite : combat réel joueur + 2 alliés, quêtes personnelles poussées et contribution réelle issue des tours de combat."
                    },
                    false
                );
            }
        }
    }

// EN: openTeamMenu declares or implements a focused behavior used by this module.
// FR: openTeamMenu déclare ou implémente un comportement précis utilisé par ce module.
void runGuildGroupTest(Player& player)
{
    std::vector<RecruitTeamSharePreview> recruits = collectRecruitTeamPreviews(player);
    if (recruits.empty())
    {
        MessageScreen::show("TEST DE GROUPE", "team.guild_test.empty", {"Il faut au moins une recrue retenue avant de passer le test de groupe."}, false);
        return;
    }
    if (guildGroupTestPassed(player))
    {
        MessageScreen::show("TEST DE GROUPE", "team.guild_test.already", {"La guilde a déjà validé tes bases de jeu de groupe.", "Clan/groupe actuel : " + clanNameFromJournal(player) + "."}, false);
        return;
    }

    Random random;
    const int retained = static_cast<int>(recruits.size());
    const int preparation = player.getLevel() * 7
        + playerGuildRankIndexForTeam(player) * 11
        + computeTeamRankIndex(player, recruits) * 8
        + retained * 5
        + (player.hasPassiveSkill("contract_reader") ? 7 : 0)
        + (player.hasPassiveSkill("field_observer") ? 7 : 0)
        + random.between(1, 65);
    const int required = 46 + retained * 5 + random.between(0, 35);
    if (preparation < required)
    {
        MessageScreen::show(
            "TEST DE GROUPE ÉCHOUÉ",
            "team.guild_test.failed",
            {
                "La guilde te pose des questions simples : placement, partage, ordre de passage, responsabilités, soins, retraite.",
                "Préparation : " + std::to_string(preparation) + " | seuil : " + std::to_string(required) + ".",
                "Résultat : pas encore validé. Tu peux retenter après avoir observé, progressé, ou mieux préparé ton équipe.",
                "Tant que ce test n'est pas réussi, les recrues restent dans le registre mais ne peuvent pas être équipées dans l'équipe active."
            },
            false
        );
        return;
    }

    player.recordCanonicalEvent("tests_groupe_guilde_reussis", "base", "Test de groupe validé par la guilde", 1);
    std::string clanName = MessageScreen::askText(
        "NOM DU GROUPE / CLAN",
        "team.guild_test.clan_name",
        {
            "Test validé : la guilde accepte que tu équipes des recrues dans ton équipe active.",
            "Choisis maintenant un nom de groupe/clan. Il pourra être reconnu progressivement comme ton nom de joueur."
        },
        "Nom du clan",
        "Entre 3 et 40 caractères. Lettres, chiffres, espaces, tirets et apostrophes.",
        true,
        0,
        40
    );
    clanName = cleanClanName(clanName);
    player.recordCanonicalEvent("clan_nom", "nom_actuel", clanName, 1);
    MessageScreen::show(
        "CHEF DE GROUPE VALIDÉ",
        "team.guild_test.success",
        {
            "Préparation : " + std::to_string(preparation) + " | seuil : " + std::to_string(required) + ".",
            "Nom enregistré : " + clanName + ".",
            "Tu peux maintenant équiper jusqu'à 2 recrues actives avec toi. Le reste du groupe reste en réserve.",
            "Le rang moyen du groupe servira plus tard aux quêtes de groupe."
        },
        false
    );
}

void manageActiveRecruitSlots(Player& player)
{
    std::vector<RecruitTeamSharePreview> recruits = collectRecruitTeamPreviews(player);
    if (recruits.empty())
    {
        MessageScreen::show("ÉQUIPE ACTIVE", "team.active.empty", {"Aucune recrue retenue pour le moment."}, false);
        return;
    }
    if (!guildGroupTestPassed(player))
    {
        MessageScreen::show(
            "ÉQUIPE ACTIVE BLOQUÉE",
            "team.active.blocked_test",
            {
                "La guilde n'a pas encore validé ton test de groupe.",
                "Tu peux garder des profils dans le registre, mais pas encore les équiper en équipe active.",
                "Passe le test de groupe depuis le menu Équipe."
            },
            false
        );
        return;
    }

    while (true)
    {
        recruits = collectRecruitTeamPreviews(player);
        const int equipped = equippedRecruitCount(player);
        MenuScreen screen("ÉQUIPE ACTIVE", "team.active.manage");
        screen.addLine("Clan/groupe : " + clanNameFromJournal(player) + ".");
        screen.addLine("Actifs : joueur + " + std::to_string(equipped) + "/2 recrue(s) équipée(s). Les autres restent en réserve.");
        screen.addBackOption("Retour", "team.active.back");
        for (std::size_t i = 0; i < recruits.size(); ++i)
        {
            const RecruitTeamSharePreview& recruit = recruits[i];
            const bool unavailableMedical = recruit.inInfirmary || recruit.awaitingInfirmaryTransfer;
            const bool canToggle = !unavailableMedical && (recruit.activeEquipped || equipped < 2);
            const std::string availability = recruit.awaitingInfirmaryTransfer
                ? "à évacuer vers l'infirmerie"
                : (recruit.inInfirmary
                    ? ("infirmerie, prêt jour " + std::to_string(recruit.infirmaryReadyDay))
                    : (recruit.activeEquipped ? "actif" : "réserve"));
            screen.addOption(
                static_cast<int>(i + 1),
                std::string(recruit.activeEquipped ? "Déséquiper " : "Équiper ") + recruit.name,
                "Rang " + recruitRankNameFromIndex(recruit.currentRankIndex) + " | niv. " + std::to_string(recruit.estimatedLevel) + " | " + availability,
                canToggle,
                "team.active.toggle"
            );
        }
        const int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choisis une recrue à équiper/déséquiper.");
        Console::clear();
        if (choice == 0) return;
        if (choice >= 1 && choice <= static_cast<int>(recruits.size()))
        {
            const RecruitTeamSharePreview recruit = recruits[static_cast<std::size_t>(choice - 1)];
            if (recruit.awaitingInfirmaryTransfer)
            {
                MessageScreen::show("RECRUE À ÉVACUER", "team.active.awaiting_infirmary", {recruit.name + " est tombé à 0 PV.", "Va dans Infirmerie des recrues pour l'amener en ville avant de pouvoir la rééquiper."}, false);
                continue;
            }
            if (recruit.inInfirmary)
            {
                MessageScreen::show("RECRUE INDISPONIBLE", "team.active.infirmary", {recruit.name + " est à l'infirmerie.", "Va la récupérer dans le menu Infirmerie des recrues quand Lysa l'autorise."}, false);
                continue;
            }
            if (!recruit.activeEquipped && equipped >= 2)
            {
                MessageScreen::show("ÉQUIPE ACTIVE PLEINE", "team.active.full", {"Tu peux avoir seulement 2 recrues équipées avec le joueur pour le moment.", "Déséquipe quelqu'un avant d'en équiper une autre."}, false);
                continue;
            }
            player.recordCanonicalEvent("recrues_equipees_toggle", recruit.name, recruit.name + (recruit.activeEquipped ? " déséquipé" : " équipé"), 1);
            MessageScreen::show("ÉQUIPE ACTIVE", "team.active.toggled", {recruit.name + (recruit.activeEquipped ? " passe en réserve." : " rejoint l'équipe active."), "Rappel : équiper/déséquiper ne renvoie pas la recrue et ne change pas la taille totale du groupe."}, false);
        }
    }
}

void runSmallGroupAssignment(Player& player)
{
    std::vector<RecruitTeamSharePreview> recruits = collectRecruitTeamPreviews(player);
    if (!guildGroupTestPassed(player))
    {
        MessageScreen::show("MISSION DE GROUPE", "team.assignment.blocked_test", {"La guilde refuse d'envoyer ton groupe tant que le test de groupe n'est pas validé."}, false);
        return;
    }
    std::vector<RecruitTeamSharePreview> active;
    for (const RecruitTeamSharePreview& recruit : recruits)
    {
        if (recruit.activeEquipped)
        {
            active.push_back(recruit);
        }
    }
    if (active.empty())
    {
        MessageScreen::show("MISSION DE GROUPE", "team.assignment.no_active", {"Aucune recrue équipée. Cette mission courte sert justement à enregistrer des parts de récompense pour les alliés actifs."}, false);
        return;
    }

    // FR: on normalise sur toutes les recrues retenues pour garder une cohérence économique globale.
    const int playerShare = normalizeRecruitRewardShares(recruits);
    const int weekIndex = std::max(0, player.getWorldDaysElapsed() / 7);
    Random random;
    const int baseReward = 80 + player.getLevel() * 12 + computeTeamRankIndex(player, recruits) * 35 + random.between(0, 80);
    std::vector<std::string> lines;
    lines.push_back("Mission courte encadrée par la guilde : escorte, livraison ou soutien local. Ce n'est pas encore le vrai combat joueur + 2 alliés, mais ça prépare l'économie.");
    lines.push_back("Récompense or de groupe : " + Money::formatCopper(baseReward) + " | part chef : " + std::to_string(playerShare) + "%.");
    for (const RecruitTeamSharePreview& activeRecruit : active)
    {
        int share = activeRecruit.normalizedShare;
        for (const RecruitTeamSharePreview& recruit : recruits)
        {
            if (recruit.name == activeRecruit.name)
            {
                share = recruit.normalizedShare;
                break;
            }
        }
        const int copper = std::max(1, (baseReward * share) / 100);
        player.recordCanonicalEvent("participation_recrues", activeRecruit.name, activeRecruit.name + " participe à une mission courte", 1);
        player.recordCanonicalEvent("recrues_equipees_semaines", recruitEquippedWeekKey(activeRecruit, weekIndex), activeRecruit.name + " a été équipé cette semaine", 1);
        player.recordCanonicalEvent("parts_or_recrues", recruitWeekKey(activeRecruit, weekIndex), activeRecruit.name + " reçoit une part de récompense", copper);
        player.recordCanonicalEvent("xp_recrues", activeRecruit.name, activeRecruit.name + " gagne de l'expérience de groupe", 1 + computeTeamRankIndex(player, recruits));
        lines.push_back("- " + activeRecruit.name + " reçoit " + Money::formatCopper(copper) + " (" + std::to_string(share) + "%).");
    }
    lines.push_back("Ces parts serviront au bilan hebdomadaire : un salaire ne baisse que si la recrue était équipée ET a reçu assez de parts cette semaine.");
    MessageScreen::show("MISSION DE GROUPE COURTE", "team.assignment.done", lines, false);
}

void openRecruitInfirmaryMenu(Player& player)
{
    std::vector<RecruitTeamSharePreview> recruits = collectRecruitTeamPreviews(player);
    std::vector<RecruitTeamSharePreview> injured;
    for (const RecruitTeamSharePreview& recruit : recruits)
    {
        if (recruit.inInfirmary || recruit.awaitingInfirmaryTransfer)
        {
            injured.push_back(recruit);
        }
    }

    if (injured.empty())
    {
        MessageScreen::show(
            "INFIRMERIE DES RECRUES",
            "team.infirmary.empty",
            {
                "Aucune recrue n'est actuellement immobilisée à l'infirmerie.",
                "Si une recrue tombe au combat, elle devra être amenée en ville avant d'être de nouveau opérationnelle."
            },
            false
        );
        return;
    }

    while (true)
    {
        recruits = collectRecruitTeamPreviews(player);
        injured.clear();
        for (const RecruitTeamSharePreview& recruit : recruits)
        {
            if (recruit.inInfirmary || recruit.awaitingInfirmaryTransfer)
            {
                injured.push_back(recruit);
            }
        }
        if (injured.empty())
        {
            MessageScreen::show("INFIRMERIE DES RECRUES", "team.infirmary.cleared", {"Toutes les recrues soignées ont été récupérées."}, false);
            return;
        }

        const bool revivalBlockedByDebt = infirmaryDebtBlocksRecruitRevival(player);
        MenuScreen screen("INFIRMERIE DES RECRUES", "team.infirmary.menu");
        screen.addLine("Une recrue tombée au combat n'est pas opérationnelle tant que tu ne viens pas la récupérer.");
        screen.addLine("Si les dégâts sont graves, Lysa peut la garder plusieurs jours.");
        if (revivalBlockedByDebt)
        {
            screen.addLine("Dette critique : récupération/réanimation bloquée tant que la dette dépasse " + Money::formatCopper(infirmaryRevivalDebtThresholdCopper()) + ".");
        }
        screen.addBackOption("Retour", "team.infirmary.back");
        for (std::size_t i = 0; i < injured.size(); ++i)
        {
            const RecruitTeamSharePreview& recruit = injured[i];
            const std::string actionLabel = recruit.awaitingInfirmaryTransfer
                ? ("Amener à l'infirmerie " + recruit.name)
                : (std::string(recruit.infirmaryReady ? (revivalBlockedByDebt ? "Récupération bloquée : " : "Récupérer ") : "Voir dossier de ") + recruit.name);
            const std::string detailLabel = recruit.awaitingInfirmaryTransfer
                ? ("KO à évacuer | gravité " + recruit.infirmarySeverity + " | repos estimé " + std::to_string(recruit.pendingRecoveryDays) + " jour(s)")
                : ("Gravité " + recruit.infirmarySeverity + " | prêt jour " + std::to_string(recruit.infirmaryReadyDay) + " | jour actuel " + std::to_string(player.getWorldDaysElapsed()));
            screen.addOption(
                static_cast<int>(i + 1),
                actionLabel,
                detailLabel,
                true,
                recruit.awaitingInfirmaryTransfer ? "team.infirmary.transport" : (recruit.infirmaryReady ? "team.infirmary.recover" : "team.infirmary.inspect")
            );
        }

        const int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choisis une recrue à consulter.");
        Console::clear();
        if (choice == 0) return;
        if (choice < 1 || choice > static_cast<int>(injured.size()))
        {
            continue;
        }

        const RecruitTeamSharePreview recruit = injured[static_cast<std::size_t>(choice - 1)];
        if (recruit.awaitingInfirmaryTransfer)
        {
            const int readyDay = player.getWorldDaysElapsed() + std::max(1, recruit.pendingRecoveryDays);
            std::ostringstream label;
            label << recruit.name
                  << " | pret_jour=" << readyDay
                  << " | gravite=" << recruit.infirmarySeverity
                  << " | ville=" << player.getCurrentCityId()
                  << " | sortie_pct=" << std::clamp(recruit.infirmaryExitHealthPercent, 1, 100)
                  << " | origine=transport_manuel";
            player.recordCanonicalEvent("recrues_infirmerie_sejours", recruit.name, label.str(), 1);
            player.recordCanonicalEvent("recrues_evacuees_infirmerie", recruit.name, recruit.name + " transporté manuellement à l'infirmerie", 1);
            player.recordCanonicalEvent("pv_recrues_persistants", recruit.name, recruit.name + " | hp=0 | max=" + std::to_string(recruit.maxHp) + " | potions=" + std::to_string(recruit.potionCharges) + " | statut=en_soins | jour=" + std::to_string(player.getWorldDaysElapsed()), 1);
            MessageScreen::show(
                "TRANSPORT VERS L'INFIRMERIE",
                "team.infirmary.transport",
                {
                    recruit.name + " est amené à l'infirmerie de " + player.getCurrentCityId() + ".",
                    "Repos estimé : " + std::to_string(recruit.pendingRecoveryDays) + " jour(s).",
                    "Retour possible à partir du jour " + std::to_string(readyDay) + "."
                },
                false
            );
            continue;
        }
        if (!recruit.infirmaryReady)
        {
            MessageScreen::show(
                "SOINS EN COURS",
                "team.infirmary.wait",
                {
                    recruit.name + " est encore gardé par l'infirmerie.",
                    "Gravité : " + recruit.infirmarySeverity + ".",
                    "Retour possible à partir du jour " + std::to_string(recruit.infirmaryReadyDay) + "."
                },
                false
            );
            continue;
        }

        if (infirmaryDebtBlocksRecruitRevival(player))
        {
            std::vector<std::string> lines = buildInfirmaryDebtBlockLines(player, true);
            lines.push_back(recruit.name + " est prêt administrativement, mais Lysa refuse la réanimation/récupération tant que la dette critique n'est pas descendue sous le seuil.");
            MessageScreen::show("RÉANIMATION BLOQUÉE", "team.infirmary.recover.debt_block", lines, false);
            continue;
        }

        const int restoredHp = std::max(1, recruit.maxHp * std::clamp(recruit.infirmaryExitHealthPercent, 1, 100) / 100);
        player.recordCanonicalEvent("recrues_infirmerie_recuperees", recruit.name, recruit.name + " récupéré à l'infirmerie", 1);
        player.recordCanonicalEvent("recrues_retour_soins", recruit.name, recruit.name + " redevient opérationnel", 1);
        player.recordCanonicalEvent("pv_recrues_persistants", recruit.name, recruit.name + " | hp=" + std::to_string(restoredHp) + " | max=" + std::to_string(recruit.maxHp) + " | potions=" + std::to_string(recruit.potionCharges) + " | statut=sortie_infirmerie | jour=" + std::to_string(player.getWorldDaysElapsed()), 1);
        MessageScreen::show(
            "RECRUE RÉCUPÉRÉE",
            "team.infirmary.recovered",
            {
                recruit.name + " quitte l'infirmerie et redevient opérationnel.",
                "PV de sortie : " + std::to_string(restoredHp) + "/" + std::to_string(recruit.maxHp) + " selon le seuil de difficulté enregistré au moment de la chute.",
                "Si cette recrue était équipée avant de tomber, elle pourra de nouveau compter dans l'équipe active."
            },
            false
        );
    }
}

void openInfirmaryServiceMenu(Player& player)
{
    while (true)
    {
        const int playerTarget = hpTargetNinetyPercent(player.getMaxHp());
        std::vector<RecruitTeamSharePreview> recruits = collectRecruitTeamPreviews(player);
        int woundedAvailable = 0;
        int immobilized = 0;
        int awaitingTransfer = 0;
        for (const RecruitTeamSharePreview& recruit : recruits)
        {
            if (recruit.awaitingInfirmaryTransfer)
            {
                ++awaitingTransfer;
                ++immobilized;
            }
            else if (recruit.inInfirmary)
            {
                ++immobilized;
            }
            else if (recruit.currentHp < hpTargetNinetyPercent(recruit.maxHp))
            {
                ++woundedAvailable;
            }
        }

        const int totalDebt = totalInfirmaryDebtCopper(player);
        const bool debtBlocksHealing = totalDebt > 0;
        const bool debtBlocksRevival = totalDebt > infirmaryRevivalDebtThresholdCopper();

        MenuScreen screen("INFIRMERIE DE LYSA", "team.infirmary.service.menu");
        screen.addLine("Lysa ne transforme pas une nuit de sommeil en miracle : les soins sérieux se paient.");
        screen.addLine("Soin payant : jusqu'à 90% des PV, jamais plus, sauf futurs traitements spéciaux.");
        screen.addLine("Tes PV : " + std::to_string(player.getHp()) + "/" + std::to_string(player.getMaxHp()) + " | seuil infirmerie : " + std::to_string(playerTarget) + ".");
        screen.addLine("Membres blessés disponibles : " + std::to_string(woundedAvailable) + " | immobilisés/à évacuer : " + std::to_string(immobilized) + " dont " + std::to_string(awaitingTransfer) + " à amener.");
        if (debtBlocksHealing)
        {
            screen.addLine("Dette actuelle : " + Money::formatCopper(totalDebt) + " -> soins bloqués tant qu'elle n'est pas réglée.");
        }
        if (debtBlocksRevival)
        {
            screen.addLine("Dette critique > " + Money::formatCopper(infirmaryRevivalDebtThresholdCopper()) + " -> récupération/réanimation d'équipier bloquée.");
        }
        screen.addBackOption("Retour", "team.infirmary.service.back");
        screen.addOption(1, "Se soigner", debtBlocksHealing ? "Bloqué : dette d'infirmerie à régler d'abord." : "Payer Lysa pour remonter tes PV jusqu'à 90% maximum.", !debtBlocksHealing, "team.infirmary.service.player");
        screen.addOption(2, "Soigner un membre d'équipe", debtBlocksHealing ? "Bloqué : dette d'infirmerie à régler d'abord." : "Choisir une recrue disponible et payer ses soins jusqu'à 90% maximum.", !debtBlocksHealing, "team.infirmary.service.recruit");
        screen.addOption(3, "Mettre / récupérer quelqu'un", debtBlocksRevival ? "Tu peux amener un KO, mais la récupération est bloquée par la dette critique." : "Amener une recrue KO à l'infirmerie ou récupérer une recrue prête.", true, "team.infirmary.service.manage");
        screen.addOption(4, "Dettes d'infirmerie", "Voir ou payer les dettes personnelles/groupe après sauvetage.", totalDebt > 0, "team.infirmary.service.debts");

        const int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choisis un service d'infirmerie.");
        Console::clear();
        if (choice == 0)
        {
            return;
        }
        if (choice == 1)
        {
            healPlayerAtInfirmary(player);
        }
        else if (choice == 2)
        {
            healRecruitAtInfirmary(player);
        }
        else if (choice == 3)
        {
            openRecruitInfirmaryMenu(player);
        }
        else if (choice == 4)
        {
            openInfirmaryDebtMenu(player);
        }
    }
}

void dismissRecruitMenu(Player& player)
{
    std::vector<RecruitTeamSharePreview> recruits = collectRecruitTeamPreviews(player);
    if (recruits.empty())
    {
        MessageScreen::show("RENVOYER UNE RECRUE", "team.dismiss.empty", {"Aucune recrue active dans le registre."}, false);
        return;
    }
    MenuScreen screen("RENVOYER UNE RECRUE", "team.dismiss.menu");
    screen.addLine("Renvoyer retire le profil du groupe actif/recrutement. Le jeu garde une trace historique, mais le profil ne compte plus dans la limite.");
    screen.addBackOption("Retour", "team.dismiss.back");
    for (std::size_t i = 0; i < recruits.size(); ++i)
    {
        const RecruitTeamSharePreview& recruit = recruits[i];
        screen.addOption(static_cast<int>(i + 1), "Renvoyer " + recruit.name, "Rang " + recruitRankNameFromIndex(recruit.currentRankIndex) + " | " + (recruit.activeEquipped ? "équipé" : "réserve"), true, "team.dismiss.select");
    }
    const int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choisis une recrue à renvoyer.");
    Console::clear();
    if (choice <= 0 || choice > static_cast<int>(recruits.size()))
    {
        return;
    }
    const RecruitTeamSharePreview recruit = recruits[static_cast<std::size_t>(choice - 1)];
    const bool confirmed = MessageScreen::askKeywordConfirmation(
        "CONFIRMER LE RENVOI",
        "team.dismiss.confirm",
        {
            "Tu vas renvoyer " + recruit.name + ".",
            "Cela libère une place dans le groupe, mais ne réactive pas automatiquement le titre On recrute !.",
            "Tape RENVOYER pour confirmer."
        },
        "RENVOYER"
    );
    if (!confirmed)
    {
        MessageScreen::show("RENVOI ANNULÉ", "team.dismiss.cancelled", {"Aucun changement."}, false);
        return;
    }
    player.recordCanonicalEvent("recrues_renvoyees", recruit.name, recruit.name + " renvoyé du groupe", 1);
    MessageScreen::show("RECRUE RENVOYÉE", "team.dismiss.done", {recruit.name + " quitte le groupe.", "Le registre historique garde une trace, mais cette recrue ne compte plus dans les places actives."}, false);
}

// EN: openTeamMenu declares or implements a focused behavior used by this module.
// FR: openTeamMenu déclare ou implémente un comportement précis utilisé par ce module.
void QuestMenu::openTeamMenu(Player& player)
{
    while (true)
    {
        std::vector<RecruitTeamSharePreview> recruits = collectRecruitTeamPreviews(player);
        MenuScreen screen("ÉQUIPE", "team.quick.menu");
        screen.addSubtitle("Gestion de groupe / clan — base recrutements");
        screen.addLine("Chef : " + player.getName() + " | clan : " + clanNameFromJournal(player) + " | profils actifs : " + std::to_string(recruits.size()) + "/4.");
        screen.addLine("Rang de groupe : " + recruitRankNameFromIndex(computeTeamRankIndex(player, recruits)) + " | moyenne " + formatRankAverage(computeTeamAverageRankIndex(player, recruits)) + " | test guilde : " + std::string(guildGroupTestPassed(player) ? "validé" : "à faire") + ".");
        screen.addLine("Équipe active : joueur + " + std::to_string(equippedRecruitCount(player)) + "/2 recrue(s) équipée(s).");
        screen.addBackOption("Retour", "team.quick.back");
        screen.addOption(1, "Inspection rapide de l'équipe", "Nom, rang, niveau estimé, part, salaire et quête personnelle.", true, "team.quick.inspect");
        screen.addOption(2, "Parts de récompense", "Prévisualise les plafonds : le joueur garde la plus grosse part.", true, "team.quick.reward_shares");
        screen.addOption(3, "Test de groupe à la guilde", "Obligatoire avant d'équiper des recrues ; permet de choisir un nom de clan.", !guildGroupTestPassed(player), "team.quick.guild_test");
        screen.addOption(4, "Équiper / déséquiper une recrue", "Joueur + 2 alliés actifs maximum. Nécessite le test de groupe.", true, "team.quick.active_slots");
        screen.addOption(5, "Mission de groupe courte", "Enregistre des parts de récompense pour les recrues équipées.", true, "team.quick.assignment");
        screen.addOption(6, "Bilan hebdomadaire des recrues", "Achats autonomes réels : potions, équipement personnel, salaire selon parts reçues, rang si logique.", true, "team.quick.weekly");
        screen.addOption(7, "Rang moyen du groupe", "Moyenne des rangs du chef et des recrues, puis rang de groupe déduit.", true, "team.quick.group_rank");
        screen.addOption(8, "Ordre d'équipe", "Modifier la priorité de passage des recrues actives.", true, "team.quick.order");
        screen.addOption(9, "Quêtes de groupe", "Premières missions filtrées par rang moyen de groupe, avec partage or/XP.", true, "team.quick.group_quests");
        screen.addOption(10, "Renvoyer une recrue", "Avec confirmation. Ne rééquipe pas On recrute ! automatiquement.", true, "team.quick.dismiss");
        screen.addOption(11, "Infirmerie / soins", "Se soigner, soigner une recrue ou gérer les évacuations/récupérations.", true, "team.quick.infirmary");
        screen.addOption(12, "Quêtes personnelles", "Aider une recrue à régler son histoire/problème personnel pour renforcer la loyauté.", true, "team.quick.personal_quests");
        screen.addOption(13, "Réputation du clan", "Voir le nom de groupe, ses traces de réputation et le palier actuel.", true, "team.quick.clan_reputation");
        screen.addOption(14, "Faisabilité multi en ligne", "Résumé technique pour savoir comment rendre le jeu jouable en ligne plus tard.", true, "team.quick.online");

        const int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choisis une option d'équipe.");
        Console::clear();
        if (choice == 0) return;
        if (choice == 1)
        {
            inspectTeamRoster(player);
        }
        else if (choice == 2)
        {
            showTeamRewardSharePreview(player);
        }
        else if (choice == 3)
        {
            runGuildGroupTest(player);
        }
        else if (choice == 4)
        {
            manageActiveRecruitSlots(player);
        }
        else if (choice == 5)
        {
            runSmallGroupAssignment(player);
        }
        else if (choice == 6)
        {
            runRecruitWeeklyManagement(player);
        }
        else if (choice == 7)
        {
            const std::vector<std::string> rankLines = buildTeamRankLines(player, recruits);
            MessageScreen::show("RANG MOYEN DU GROUPE", "team.quick.group_rank", rankLines, false);
        }
        else if (choice == 8)
        {
            showTeamOrderMenu(player);
        }
        else if (choice == 9)
        {
            runGroupQuestBoard(player);
        }
        else if (choice == 10)
        {
            dismissRecruitMenu(player);
        }
        else if (choice == 11)
        {
            openInfirmaryServiceMenu(player);
        }
        else if (choice == 12)
        {
            runRecruitPersonalQuestMenu(player);
        }
        else if (choice == 13)
        {
            showClanReputationSummary(player);
        }
        else if (choice == 14)
        {
            showTeamOnlineFeasibility();
        }
    }
}

// EN: openExploration declares or implements a focused behavior used by this module.
// FR: openExploration déclare ou implémente un comportement précis utilisé par ce module.
