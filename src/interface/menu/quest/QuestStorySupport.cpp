// EN: Story-specific quest helpers extracted from QuestMenu.cpp.
// FR: Helpers de quêtes histoire extraits de QuestMenu.cpp.
// No story progression is added here.

#include "interface/menu/quest/QuestMenu.hpp"
#include "interface/menu/quest/QuestContractorMenu.hpp"
#include "interface/menu/quest/QuestTeamMenu.hpp"
#include "interface/menu/quest/QuestWorldMenuSupport.hpp"
#include "interface/menu/quest/QuestDeadlineSupport.hpp"
#include "interface/menu/quest/QuestClientNavigationSupport.hpp"
#include "adventure/flavor/ExplorationBiomeFlavor.hpp"
#include "adventure/flavor/ExplorationLanguageTrace.hpp"
#include "adventure/content/BiomeLivingContentCatalog.hpp"
#include "adventure/content/BiomeAmbientEventSystem.hpp"
#include "adventure/content/BiomeNonCombatInteractionSystem.hpp"
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
#include "interface/menu/quest/QuestStorySupport.hpp"
#include "interface/menu/quest/QuestPresentationSupport.hpp"

using namespace QuestPresentationSupport;

namespace QuestStorySupport
{    bool isStoryReferentClientName(const std::string& clientName)
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

    [[maybe_unused]] Quest createChapterThreeLonelyConvoyQuest()
    {
        return buildChapterOneStoryQuest(
            "story_ch3_lonely_convoy", "Le convoi qui revient seul", "Mira",
            "Relais silencieux / convoi revenu", "Inspecter les roues, le registre et les marchandises supplémentaires sans déplacer le convoi avant le relevé.",
            "enquête", "Convoi sans équipage", 1, 105, 30
        );
    }

    [[maybe_unused]] Quest createChapterThreeThreeRoutesQuest()
    {
        Quest quest = buildChapterOneStoryQuest(
            "story_ch3_three_routes", "Trois routes pour une même borne", "Orren",
            "Route commerciale", "Mesurer la même borne à l'aube, au milieu du jour puis la nuit afin de prouver que la distance dépend du moment.",
            "exploration à étapes", "Route commerciale / moments de la journée", 3, 128, 36
        );
        quest.stageLabels = "Mesure à l'aube|Mesure au milieu du jour|Mesure de nuit";
        return quest;
    }

    [[maybe_unused]] Quest createChapterThreeSignaturesQuest()
    {
        Quest quest = buildChapterOneStoryQuest(
            "story_ch3_signatures", "Les signatures sans voyageurs", "Soryn",
            "Archives / comptoirs / relais", "Faire identifier les sceaux impossibles par Soryn, Eda et Nell. Chaque avis déjà obtenu reste compté.",
            "enquête à étapes", "Sceaux / registres / témoins", 3, 118, 34
        );
        quest.stageLabels = "Analyse de Soryn|Vérification des poids par Eda|Reconnaissance des sceaux par Nell";
        return quest;
    }

    [[maybe_unused]] Quest createChapterThreeEscortWithdrawalQuest()
    {
        Quest quest = buildChapterOneStoryQuest(
            "story_ch3_escort_withdrawal", "Une escorte qui sait renoncer", "Orren",
            "Route commerciale / convoi court", "Escorter un petit convoi, puis accepter un demi-tour propre si la route change au lieu de sacrifier les voyageurs.",
            "escorte à étapes", "Convoi / retour sécurisé", 2, 142, 42
        );
        quest.stageLabels = "Escorte engagée|Demi-tour sécurisé";
        return quest;
    }

    [[maybe_unused]] Quest createChapterThreeMarginVillageQuest()
    {
        return buildChapterOneStoryQuest(
            "story_ch3_margin_village", "Le village écrit dans la marge", "Nell",
            "Relais / carte corrigée", "Isoler la première preuve lisible d'un village absent des cartes actuelles sans encore y envoyer de groupe.",
            "découverte", "Village absent des cartes", 1, 122, 35
        );
    }

    [[maybe_unused]] Quest createChapterThreeCorrectedRouteQuest()
    {
        return buildChapterOneStoryQuest(
            "story_ch3_corrected_route", "La route corrigée", "Mira",
            "Contre-registre de la ville", "Choisir quelle version du trajet conserver : commerce, secours ou preuve de recherche. Le choix modifie le récit sans bloquer la suite.",
            "choix narratif", "Contre-registre / conséquence durable", 1, 136, 40
        );
    }

    [[maybe_unused]] Quest createChapterThreeMapGuardianQuest()
    {
        return buildChapterOneStoryQuest(
            "story_ch3_map_guardian", "Le Gardien de la Carte Juste", "Soryn",
            "Route corrigée / borne de cohérence", "Affronter le Gardien de la Carte Juste, mini-boss unique qui défend la route considérée correcte plutôt que la route la plus sûre.",
            "combat mini-boss unique", "Mini-boss unique / cohérence de la carte", 1, 190, 58
        );
    }

    [[maybe_unused]] Quest createChapterThreeConvoyReturnQuest()
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

}
