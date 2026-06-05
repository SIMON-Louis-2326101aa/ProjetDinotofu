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
#include "economy/shop/ShopTransactionSystem.hpp"
#include "economy/Money.hpp"
#include "interface/menu/InventoryMenu.hpp"
#include "interface/menu/common/PagedMenu.hpp"
#include "interface/menu/common/MessageScreen.hpp"
#include "interface/TerminalInterface.hpp"
#include "interface/model/MenuScreen.hpp"
#include "progression/bestiary/BestiaryRuntimeProgress.hpp"

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
            && player.getStoryStep() >= 5;
    }

    std::vector<std::string> chapterOneReferentClients()
    {
        return {"Orren", "Lysa", "Bram", "Soryn"};
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
        return buildChapterOneStoryQuest(
            "story_ch1_meet_referents",
            "Faire le tour de la ville",
            "Mira",
            "Quartier de départ",
            "Rencontrer Orren, Lysa, Bram et Soryn après la présentation de Mira, puis revenir la prévenir.",
            "dialogue",
            "Référents de ville",
            1,
            0,
            0
        );
    }

    Quest createChapterOneMiraMainQuest()
    {
        return buildChapterOneStoryQuest(
            "story_ch1_mira_main",
            "Faire respirer les murs",
            "Mira",
            "Abords de la ville",
            "Repousser des menaces ou confirmer des traces hostiles autour des murs pour que Mira puisse défendre une sortie contrôlée.",
            "combat",
            "Créatures locales",
            3,
            65,
            22
        );
    }

    Quest createChapterOneAskHelpQuest(const std::string& clientName)
    {
        return buildChapterOneStoryQuest(
            storyAskHelpQuestId(clientName),
            "Demander l'aide de " + clientName,
            clientName,
            clientName == "Orren" ? "Poste d'Orren" : (clientName == "Lysa" ? "Infirmerie de Lysa" : (clientName == "Bram" ? "Forge de Bram" : "Archives de Soryn")),
            "Retourner parler à " + clientName + " de la part de Mira, demander comment l'aider, puis recevoir sa vraie demande principale.",
            "dialogue",
            "Référents de ville",
            1,
            0,
            0
        );
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
        return buildChapterOneStoryQuest(
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
        return buildChapterOneStoryQuest(
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
        return buildChapterOneStoryQuest(
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

    bool allChapterOneMainRequestsTurnedIn(const Player& player)
    {
        return countTurnedInChapterOneMainRequests(player) >= 5;
    }

    bool handleStoryReferentMainQuestDialogue(Player& player, const std::string& clientName)
    {
        if (!isStoryReferentClientName(clientName)
            || !player.hasStoryModeStarted()
            || player.getStoryChapter() != 1
            || player.getStoryStep() < 5)
        {
            return false;
        }

        QuestMenu::syncMainStoryQuests(player);

        const std::string mainQuestId = storyMainQuestIdForClient(clientName);
        const std::string askQuestId = storyAskHelpQuestId(clientName);

        if (clientName != "Mira" && questIsActiveInLog(player, askQuestId))
        {
            completeAndTurnInQuestSilently(player, askQuestId);
            const bool added = addNonRefusableQuestIfMissing(player, createChapterOneReferentMainQuest(clientName));
            std::vector<std::string> lines = {
                clientName + " (" + storyReferentProfession(clientName) + ") écoute quand tu dis que tu viens de la part de Mira.",
                storyReferentRoleLine(clientName),
                clientName + " ne te propose pas une faveur à refuser : il ajoute sa demande principale au journal.",
                added ? "Quête principale ajoutée : " + storyQuestStatusForId(player, mainQuestId) : "La quête principale était déjà connue dans le journal."
            };
            MessageScreen::show("DE LA PART DE MIRA", "quest.story.referent.main_added", lines, false);
            return true;
        }

        if (questExistsInAnyState(player, mainQuestId))
        {
            std::vector<std::string> lines = {
                clientName + " — " + storyReferentProfession(clientName) + ".",
                storyReferentRoleLine(clientName),
                "Demande principale : " + storyQuestStatusForId(player, mainQuestId) + ".",
                questIsCompletedInLog(player, mainQuestId)
                    ? "Cette demande est prête : utilise l'option de rendu auprès de ce contact."
                    : (questIsTurnedInInLog(player, mainQuestId)
                        ? "Cette demande est déjà validée. Mira attendra le bilan final quand tout le monde aura été aidé."
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

        if (clientName != "Mira")
        {
            MessageScreen::show(
                "PAS ENCORE",
                "quest.story.referent.wait_mira",
                {
                    clientName + " te reconnaît, mais attend encore que Mira te donne officiellement la suite.",
                    "Va d'abord notifier Mira que le tour de la ville est terminé."
                },
                false
            );
            return true;
        }

        return false;
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
            player.getInventory().addMaterial(MaterialCatalog::createById(quest.rewardMaterialId, quest.rewardMaterialQuantity));
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

    std::string questCardLabel(const Quest& quest)
    {
        std::ostringstream label;

        if (quest.guildQuest)
        {
            label << "[Contrat de guilde - Rang " << quest.rank << "] " << quest.title
                  << " | À rendre : Maître de guilde"
                  << " | Zone/action : " << questPlayableLocationHint(quest)
                  << " | Objectif : " << quest.objective
                  << " | Progression : " << quest.progress << "/" << quest.target
                  << " | État : " << questStateText(quest)
                  << " | Récompenses : " << questRewardText(quest);
        }
        else
        {
            label << "[Demande PNJ - Rang estimé " << quest.rank << "] " << quest.title
                  << " | Contact : " << quest.client
                  << " | Zone/action : " << questPlayableLocationHint(quest)
                  << " | Objectif résumé : " << quest.objective
                  << " | Avancée : " << quest.progress << "/" << quest.target
                  << " | État : " << questStateText(quest)
                  << " | " << approximateQuestRewardText(quest);
        }

        if (!quest.requiredMaterialId.empty() && quest.requiredMaterialQuantity > 0)
        {
            label << " | Livraison : " << quest.requiredMaterialName << " x" << quest.requiredMaterialQuantity;
        }

        return label.str();
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
        lines.push_back("Progression : " + std::to_string(quest.progress) + "/" + std::to_string(quest.target));
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
        lines.push_back("Avancée notée : " + std::to_string(quest.progress) + "/" + std::to_string(quest.target));
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
            case QuestJournalFilter::TurnedIn:
                return "Archives des quêtes déjà rendues.";
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
            case QuestJournalFilter::TurnedIn:
                return quest.turnedIn || quest.failed;
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

    std::string questJournalInspectHint(const Quest& quest)
    {
        if (quest.guildQuest)
        {
            return "Inspecter le contrat officiel et ses clauses principales.";
        }

        return "Noter seulement des suppositions : cette demande PNJ n'est pas un contrat officiel.";
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
                    std::string questLabel = questCardLabel(quest);
                    const std::string deadlineStatus = activeQuestDeadlineStatusText(quest, player.getWorldDaysElapsed());
                    if (!deadlineStatus.empty())
                    {
                        questLabel += " | " + deadlineStatus;
                    }
                    MenuOptionItemData itemData;
                    itemData.structured = true;
                    itemData.kind = "quest";
                    itemData.section = clientName;
                    itemData.actionType = quest.guildQuest ? "inspect_contract" : "estimate_request";
                    itemData.name = quest.title;
                    itemData.detail = quest.guildQuest ? quest.objective : "Pourparler : " + quest.objective;
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
                        questJournalInspectHint(quest),
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
                showQuestDetail(player, *relatedQuests[first + static_cast<std::size_t>(localQuestIndex)]);
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
        MenuScreen screen(title, screenId);
        screen.addSubtitle(quest.guildQuest ? "Contrat officiel" : "Pourparler / demande informelle");

        for (const std::string& line : introLines)
        {
            screen.addLine(line);
        }

        screen.addLine("Nature : " + questKindText(quest));
        screen.addLine((quest.guildQuest ? "Contrat proposé : [Rang " : "Demande proposée : [Rang estimé ") + quest.rank + "] " + quest.title);
        screen.addLine((quest.guildQuest ? "Client officiel : " : "Contact : ") + quest.client);
        screen.addLine((quest.guildQuest ? "Zone/action jouable : " : "Zone/action probable : ") + questPlayableLocationHint(quest));
        screen.addLine((quest.guildQuest ? "Objectif : " : "Objectif raconté : ") + quest.objective);
        for (const std::string& trialLine : guildServiceTrialLines(quest))
        {
            screen.addLine(trialLine);
        }
        screen.addLine("Comment faire : " + questProgressMethodText(quest));
        screen.addLine((quest.guildQuest ? "Récompenses : " : "Estimation : ") + (quest.guildQuest ? questRewardText(quest) : approximateQuestRewardText(quest)));
        const std::string deadlineLine = offeredQuestDeadlineLine(quest, player.getWorldDaysElapsed());
        if (!deadlineLine.empty())
        {
            screen.addLine(deadlineLine);
        }

        if (!quest.guildQuest)
        {
            screen.addLine("Note : ce PNJ parle de vive voix. Le journal pourra seulement estimer certaines informations.");
        }

        const std::string materialLine = questRequiredMaterialStatusLine(player, quest);
        if (!materialLine.empty())
        {
            screen.addLine(materialLine);
        }

        MenuOptionItemData acceptData;
        acceptData.structured = true;
        acceptData.kind = "quest";
        acceptData.section = quest.guildQuest ? "Contrat officiel" : "Demande informelle";
        acceptData.actionType = "accept";
        acceptData.name = quest.title;
        acceptData.detail = quest.objective;
        acceptData.status = quest.guildQuest ? "Disponible" : "Disponible - informations estimées";
        acceptData.reward = quest.guildQuest ? questRewardText(quest) : approximateQuestRewardText(quest);
        acceptData.progress = quest.guildQuest ? "Rang " + quest.rank : "Rang estimé " + quest.rank;
        acceptData.owner = quest.client;
        acceptData.important = true;

        screen.addOption(1, quest.guildQuest ? "Accepter le contrat" : "Accepter la demande", "Ajouter cette entrée au journal.", true, screenId + ".accept", acceptData);
        screen.addOption(0, "Refuser", quest.guildQuest ? "Laisser ce contrat sur le panneau." : "Laisser ce pourparler pour le moment.", true, screenId + ".decline");

        return TerminalInterface::askMenuChoiceFromOptions(screen, "Choix invalide.");
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
                quest.guildQuest ? "Relire le contrat" : "Relire les estimations",
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

    ExplorationBossUnlockResult tryUnlockExplorationBossVariation(Player& player, Random& random, bool dangerousSite)
    {
        const std::size_t unlockedCount = player.getUnlockedBossIds().size();
        const int bossKills = player.getBossesKilled();
        const int level = player.getLevel();

        if (level < 8)
        {
            return {false, "Trace trop faible : le registre ne stabilise encore aucun dossier de boss fiable."};
        }

        if (unlockedCount >= 28)
        {
            return {false, "Trace verrouillée : les entités presque finales ne laissent presque que des noms. Les témoins sont morts, brisés ou réinitialisés."};
        }

        if (unlockedCount >= 23 && (bossKills < 6 || level < 24))
        {
            return {false, "Trace trop haute : les survivants manquent, et les récits restants semblent réinitialisés ou incomplets."};
        }

        const int chance = unlockedCount >= 23 ? (dangerousSite ? 16 : 6) : (dangerousSite ? 28 : 12);
        if (random.between(1, 100) > chance)
        {
            return {false, "Trace instable : le registre note une silhouette, mais refuse encore de stabiliser une nouvelle faille."};
        }

        const bool unlocked = player.unlockNextBossVariation();
        if (unlocked)
        {
            return {true, "Nouvelle variation de boss détectée : le registre se stabilise d'un cran, sans livrer ses secrets."};
        }

        return {false, "Trace finale bloquée : cette présence ne peut être obtenue qu'après avoir vaincu tous les autres boss concernés."};
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
        std::vector<std::string> lines;
    };

    MicroChallengeResult runExplorationMicroChallenge(const ExplorationBiome& biome, const ExplorationIntensity& intensity, Random& random)
    {
        (void)intensity;

        struct Challenge
        {
            std::string title;
            std::string question;
            std::vector<std::pair<int, std::string>> options;
            int correctChoice = 1;
            std::string successLine;
            std::string failureLine;
        };

        std::vector<Challenge> genericChallenges = {
            {
                "ÉPREUVE D'ORIENTATION",
                "Tu dois choisir rapidement une méthode avant de t'enfoncer plus loin.",
                {{1, "Marquer un repère discret et écouter la zone"}, {2, "Courir vers le premier bruit"}, {3, "Jeter une pierre très loin pour voir"}},
                1,
                "Bonne approche : tu avances avec un vrai repère, pas juste au feeling.",
                "Mauvaise approche : tu avances quand même, mais ton repère est moins fiable."
            },
            {
                "ÉPREUVE DE CALCUL",
                "Tu dois partager 18 mètres de corde en 3 segments égaux. Combien mesure chaque segment ?",
                {{1, "5 mètres"}, {2, "6 mètres"}, {3, "9 mètres"}},
                2,
                "Calcul propre : la corde est préparée sans perte de temps.",
                "Erreur de calcul : tu corriges au dernier moment, mais tu perds l'avantage propre."
            },
            {
                "ÉPREUVE DE FRANÇAIS",
                "Quelle phrase est correcte pour ton carnet de terrain ?",
                {{1, "Les traces sont récentes."}, {2, "Les trace sont récente."}, {3, "Les traces est récentes."}},
                1,
                "Note claire : ton carnet reste lisible et exploitable pour la guilde.",
                "Note maladroite : tu comprends l'idée, mais le rapport sera moins utile."
            }
        };

        Challenge challenge = genericChallenges[random.between(0, static_cast<int>(genericChallenges.size()) - 1)];

        if (biome.name == "Bocage aux lanternes")
        {
            challenge = {
                "ÉPREUVE DE RÉCOLTE",
                "Une lanterne de mycélium pulse doucement. Que fais-tu pour ne pas l'abîmer ?",
                {{1, "Couper la base d'un coup sec"}, {2, "Attendre que la lumière baisse puis détacher la terre autour"}, {3, "Souffler dessus pour l'éteindre"}},
                2,
                "Récolte intelligente : la lanterne reste presque intacte.",
                "Geste approximatif : la lanterne survit, mais une partie de sa lumière se perd."
            };
        }
        else if (biome.name == "Désert d'argile rouge")
        {
            challenge = {
                "ÉPREUVE DE CALCUL SEC",
                "Tu notes 7 traces, puis 6 groupes identiques plus loin. Combien de traces cela fait au total ?",
                {{1, "36"}, {2, "42"}, {3, "48"}},
                2,
                "Comptage propre : tu identifies la vraie piste et évites une fausse oasis.",
                "Comptage faux : tu repères l'erreur, mais un peu trop tard."
            };
        }
        else if (biome.name == "Quartier abandonné")
        {
            challenge = {
                "ÉPREUVE DE PAPERASSE",
                "Sur un formulaire de guilde, quelle formulation est la plus propre ?",
                {{1, "Les documents ont été remis."}, {2, "Les document on été remit."}, {3, "Les documents a été remis."}},
                1,
                "Formulaire propre : la guilde pourra vraiment exploiter le document.",
                "Formulaire sale : la guilde acceptera peut-être, mais pas sans soupirer."
            };
        }
        else if (biome.name == "Mine sifflante")
        {
            challenge = {
                "ÉPREUVE DE LOGIQUE",
                "Un rail vibre toutes les 4 secondes. Tu comptes 5 vibrations. Combien de secondes se sont écoulées entre la première et la cinquième ?",
                {{1, "16 secondes"}, {2, "20 secondes"}, {3, "24 secondes"}},
                1,
                "Logique nickel : tu comprends le rythme de la mine avant de marcher dessus.",
                "Logique bancale : tu comprends trop tard que la mine donne déjà le tempo."
            };
        }
        else if (biome.name == "Verger des lucioles de fer")
        {
            challenge = {
                "ÉPREUVE DE LUMIÈRE",
                "Trois lucioles clignotent 2, 4 puis 6 fois. Quel rythme semble logique ensuite ?",
                {{1, "7 clignotements"}, {2, "8 clignotements"}, {3, "12 clignotements"}},
                2,
                "Suite propre : tu synchronises ta marche avec les lucioles sans déclencher l'essaim.",
                "Suite ratée : les lucioles se dispersent et l'essaim devient beaucoup moins poli."
            };
        }
        else if (biome.name == "Archives noyées")
        {
            challenge = {
                "ÉPREUVE DE CLASSEMENT",
                "Une archive porte les cotes A-12, A-13 et A-15. Quelle cote manque probablement ?",
                {{1, "A-14"}, {2, "B-12"}, {3, "A-16"}},
                1,
                "Classement net : la page murmurante accepte d'être lue sans crier.",
                "Classement faux : l'archive se referme et te laisse une odeur d'encre froide."
            };
        }
        else if (biome.name == "Falaises des drakes gris")
        {
            challenge = {
                "ÉPREUVE DE CORDE",
                "Tu as 24 mètres de corde et 4 points d'ancrage égaux. Combien de mètres par point ?",
                {{1, "5 mètres"}, {2, "6 mètres"}, {3, "8 mètres"}},
                2,
                "Ancrage propre : la falaise te respecte presque.",
                "Mauvais partage : la corde tient, mais ton cœur descend avant tes pieds."
            };
        }
        else if (biome.name == "Foire abandonnée")
        {
            challenge = {
                "ÉPREUVE DE FRANÇAIS FORAIN",
                "Quel panneau est écrit correctement ?",
                {{1, "Les tickets sont valables."}, {2, "Les ticket sont valable."}, {3, "Les tickets est valables."}},
                1,
                "Panneau propre : même la vieille caisse semble moins te juger.",
                "Panneau faux : la foire applaudit, mais probablement pour se moquer."
            };
        }
        else if (biome.name == "Temple des cloches fendues")
        {
            challenge = {
                "ÉPREUVE DE SERMENT",
                "Une cloche sonne 3 fois, puis 6, puis 9. Combien devrait-elle sonner ensuite si le rythme continue ?",
                {{1, "10 fois"}, {2, "12 fois"}, {3, "18 fois"}},
                2,
                "Rythme compris : la cloche cesse de vibrer juste assez longtemps pour passer.",
                "Rythme raté : la cloche sonne faux et attire des regards invisibles."
            };
        }
        else if (biome.name == "Canaux de brume bleue")
        {
            challenge = {
                "ÉPREUVE DE PASSAGE",
                "Deux ponts sont brisés, un bac peut porter 3 caisses par traversée. Il faut déplacer 9 caisses. Combien de traversées chargées ?",
                {{1, "2"}, {2, "3"}, {3, "4"}},
                2,
                "Calcul net : tu ne perds pas la moitié de la matinée à revenir chercher des caisses.",
                "Calcul faux : tu comprends pourquoi les passeurs facturent au trajet."
            };
        }
        else if (biome.name == "Carrière des os blancs")
        {
            challenge = {
                "ÉPREUVE DE MESURE",
                "Une trace mesure 40 cm. Une autre est deux fois plus grande. Combien mesure la deuxième ?",
                {{1, "60 cm"}, {2, "80 cm"}, {3, "120 cm"}},
                2,
                "Mesure propre : tu sais tout de suite quand une empreinte est trop grande pour être rassurante.",
                "Mesure bancale : la carrière paraît soudain beaucoup moins vide."
            };
        }
        else if (biome.name == "Marché sous les ponts")
        {
            challenge = {
                "ÉPREUVE DE CONTRAT",
                "Quelle phrase évite le mieux une arnaque dans un reçu ?",
                {{1, "Payé après livraison vérifiée."}, {2, "Payer quand le vendeur dit que c'est bon."}, {3, "Payé peut-être demain hier."}},
                1,
                "Reçu propre : même le vendeur douteux respecte presque ton sérieux.",
                "Reçu faible : tu viens probablement d'acheter une explication, pas un objet."
            };
        }
        else if (biome.name == "Jardin des statues qui pleurent")
        {
            challenge = {
                "ÉPREUVE D'OBSERVATION",
                "Trois statues regardent la fontaine, sauf une qui regarde la sortie. Laquelle surveiller ?",
                {{1, "Celle qui regarde la sortie"}, {2, "La plus jolie"}, {3, "Aucune, courir au hasard"}},
                1,
                "Observation utile : tu repères la statue qui connaît déjà ton chemin de fuite.",
                "Observation ratée : le jardin semble changer quand tu clignes des yeux."
            };
        }

        int choice = askChoiceScreen(
            challenge.title,
            "exploration.micro_challenge",
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

    MicroChallengeResult runGuildServiceMicroChallenge(const Quest& quest, Random& random)
    {
        const std::string questText = toLowerChoiceText(quest.title + " " + quest.objective + " " + quest.location + " " + quest.targetFamily + " " + quest.client);

        struct Challenge
        {
            std::string title;
            std::string question;
            std::vector<std::pair<int, std::string>> options;
            int correctChoice = 1;
            std::string successLine;
            std::string failureLine;
        };

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
            const std::vector<Challenge> merchantChallenges = {
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
                    {{1, "3 fer"}, {2, "7 fer"}, {3, "1 électrum"}},
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
                    "'Vente : 7 potion. Montant total : 84 ?? cuivre.' Quel mot corrige le mieux l'unité ?",
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
                    {{1, "Par la présente, je soussigné le marchand Prunigil promets à… à qui déjà ?"}, {2, "Par la présente je ssoussigné le marchan Prunigil promet a qui déjà ???"}, {3, "Par la présente, je sous-signé le marchan Prunigil promet à quelqu'un."}},
                    1,
                    "Contrat un peu moins chaotique : il manque encore le client, détail mineur évidemment.",
                    "Contrat raté : juridiquement, même un gobelin refuserait de signer."
                }
            };

            challenges.insert(challenges.end(), merchantChallenges.begin(), merchantChallenges.end());
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

        const Challenge& challenge = challenges[random.between(0, static_cast<int>(challenges.size()) - 1)];
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
        result.success = choice == challenge.correctChoice;
        result.lines.push_back(result.success ? challenge.successLine : challenge.failureLine);
        result.lines.push_back(result.success
            ? "Le service peut progresser."
            : "Le service ne progresse pas cette fois. Tu pourras réessayer plus tard.");
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
        if (roll <= 26) return "récolte de terrain";
        if (roll <= 40) return "trace intéressante";
        if (roll <= 52) return "petit trésor";
        if (roll <= 59) return "fausses pièces";
        if (roll <= 70) return "coffre suspect";
        if (roll <= 82) return "rencontre imprévue";
        if (roll <= 84) return "mini-boss d'exploration";
        if (roll <= 91) return "demande de PNJ";
        if (roll <= 97) return "événement actif de biome";
        if (roll <= 99) return "lieu dangereux";
        return "découverte rare";
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

            ExplorationBossUnlockResult bossTrace = tryUnlockExplorationBossVariation(player, random, false);
            std::vector<std::string> registryLines = {
                bossTrace.line,
                "Nom : ???",
                "Statut : repérée par exploration rare."
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
        bool evolved = random.between(1, 100) <= 45;
        std::string miniBossName = miniBossNameForBiome(biome, evolved);
        std::string questFamily = miniBossQuestFamilyForBiome(biome, evolved);

        Monster miniBoss = createExplorationEliteForBiome(player, random, biome, intensity);

        std::vector<std::string> introLines = {
            "L'air se tasse autour de toi.",
            "Mini-boss d'exploration : " + miniBossName + ".",
            "Forme rencontrée : " + miniBoss.getName() + " [niveau " + std::to_string(miniBoss.getLevel()) + "].",
            "Zone : " + biome.name + " | Approche : " + intensity.name + "."
        };

        if (evolved)
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
            addExplorationMaterial(player, evolved ? biome.rareMaterialId : biome.commonMaterialId, applyExplorationQuantityBonus(1, intensity), chooseExplorationQuality(random, evolved))
        };

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

        ExplorationBossUnlockResult bossTrace = tryUnlockExplorationBossVariation(player, random, true);
        bossLines.push_back(bossTrace.line);
        if (bossTrace.unlocked)
        {
            bossLines.push_back("Nom : ???");
            bossLines.push_back("Statut : éveillé par exploration dangereuse.");
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
        int eventRoll = random.between(1, 100);

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
    if (!player.hasStoryModeStarted())
    {
        return;
    }

    if (player.getStoryChapter() == 1)
    {
        if (player.getStoryStep() >= 3)
        {
            addNonRefusableQuestIfMissing(player, createChapterOneMeetReferentsQuest());
        }

        if (player.getStoryStep() >= 4)
        {
            completeAndTurnInQuestSilently(player, "story_ch1_meet_referents");
        }

        if (player.getStoryStep() >= 5)
        {
            addNonRefusableQuestIfMissing(player, createChapterOneMiraMainQuest());
            for (const std::string& clientName : chapterOneReferentClients())
            {
                const std::string mainQuestId = storyMainQuestIdForClient(clientName);
                if (!questExistsInAnyState(player, mainQuestId))
                {
                    addNonRefusableQuestIfMissing(player, createChapterOneAskHelpQuest(clientName));
                }
            }
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
            const int mainDone = countTurnedInChapterOneMainRequests(player);
            screen.addLine("Chapitre actuel : 1 — La ville qui tient à peine.");
            screen.addLine("Progression : " + player.getStoryProgressLabel());
            screen.addLine("1. Faire connaissance avec Mira : " + std::string(player.getStoryStep() >= 3 ? "validée" : "à faire dans le menu histoire") + ".");
            screen.addLine("2. Faire le tour de la ville : " + storyQuestStatusForId(player, "story_ch1_meet_referents") + ".");
            screen.addLine("3. Notifier Mira après le tour : " + std::string(player.getStoryStep() >= 5 ? "validée" : (player.getStoryStep() >= 4 ? "à faire auprès de Mira" : "verrouillée")) + ".");
            screen.addLine("4. Mira — Faire respirer les murs : " + storyQuestStatusForId(player, "story_ch1_mira_main") + ".");
            screen.addLine("5. Orren — " + storyQuestStatusForId(player, "story_ch1_ask_help_orren") + " / demande : " + storyQuestStatusForId(player, "story_ch1_orren_main") + ".");
            screen.addLine("6. Lysa — " + storyQuestStatusForId(player, "story_ch1_ask_help_lysa") + " / demande : " + storyQuestStatusForId(player, "story_ch1_lysa_main") + ".");
            screen.addLine("7. Bram — " + storyQuestStatusForId(player, "story_ch1_ask_help_bram") + " / demande : " + storyQuestStatusForId(player, "story_ch1_bram_main") + ".");
            screen.addLine("8. Soryn — " + storyQuestStatusForId(player, "story_ch1_ask_help_soryn") + " / demande : " + storyQuestStatusForId(player, "story_ch1_soryn_main") + ".");
            screen.addLine("Bilan des demandes principales rendues : " + std::to_string(mainDone) + "/5.");
            screen.addLine(allChapterOneMainRequestsTurnedIn(player)
                ? "Dernière étape : retourne notifier Mira dans le menu histoire."
                : "Suite : va dans PNJ notables, parle aux référents indiqués, puis rends leurs demandes quand elles sont terminées.");
        }
        else if (player.getStoryChapter() == 2)
        {
            screen.addLine("Chapitre actuel : 2 — Le relais silencieux.");
            screen.addLine("Progression : " + player.getStoryProgressLabel());
            screen.addLine("1. Mira — Le nom du relais silencieux : " + storyQuestStatusForId(player, "story_ch2_relay_briefing") + ".");
            screen.addLine("2. Orren — La route qui s'allonge : " + storyQuestStatusForId(player, "story_ch2_north_road_scout") + ".");
            screen.addLine("3. Soryn — La borne retournée : " + storyQuestStatusForId(player, "story_ch2_turned_marker") + ".");
            screen.addLine("4. Orren — Les guetteurs sans feu : " + storyQuestStatusForId(player, "story_ch2_relay_threat") + ".");
            screen.addLine("5. Mira — Le relais doit répondre : " + storyQuestStatusForId(player, "story_ch2_relay_signal") + ".");
            screen.addLine("6. Nell la messagère — La voix derrière les caisses : " + storyQuestStatusForId(player, "story_ch2_first_rescue") + ".");
            screen.addLine("7. Nell la messagère — La sacoche qui parle : " + storyQuestStatusForId(player, "story_ch2_route_sack") + ".");
            screen.addLine("8. Mira — Les comptoirs rouvrent un œil : " + storyQuestStatusForId(player, "story_ch2_city_recovery") + ".");
            screen.addLine("9. Soryn — L'encre froide de la route : " + storyQuestStatusForId(player, "story_ch2_cold_ink_trail") + ".");
            screen.addLine("10. Soryn — La carte qui se réécrit : " + storyQuestStatusForId(player, "story_ch2_route_rewrite") + ".");
            screen.addLine("11. Mira — Le contre-registre des routes courtes : " + storyQuestStatusForId(player, "story_ch2_short_route_counter") + ".");
            screen.addLine("12. Orren — Le nœud noir au bout du relais : " + storyQuestStatusForId(player, "story_ch2_black_knot_warning") + ".");
            screen.addLine("13. Eda — Tenir pendant les travaux : " + storyQuestStatusForId(player, "story_ch2_repair_downtime") + ".");
            screen.addLine("14. Soryn — La chose qui garde la borne : " + storyQuestStatusForId(player, "story_ch2_hidden_guardian_hint") + ".");
            screen.addLine("15. Orren — Le verrou de la borne : " + storyQuestStatusForId(player, "story_ch2_black_knot_seal") + ".");
            screen.addLine("16. Soryn — Les cicatrices du verrou : " + storyQuestStatusForId(player, "story_ch2_black_knot_scars") + ".");
            screen.addLine("17. Mira — Une route à garder ouverte : " + storyQuestStatusForId(player, "story_ch2_guarded_route") + ".");
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

        MenuOptionItemData journalData = makeQuestNavigationItemData(
            "quest",
            "Hub",
            "inspect",
            "Journal de quêtes",
            "Consulter les contrats officiels et les demandes informelles."
        );
        journalData.status = "Consultation";

        MenuOptionItemData readyData = makeQuestNavigationItemData(
            "quest",
            "Hub",
            "turn_in",
            "Demandes prêtes à rendre",
            readyCount > 0 ? "Choisir le bon contact pour valider une quête terminée." : "Aucune quête prête à rendre."
        );
        readyData.status = readyCount > 0 ? std::to_string(readyCount) + " prête(s)" : "Indisponible";
        readyData.important = readyCount > 0;

        MenuOptionItemData guildData = makeQuestNavigationItemData(
            "npc",
            "Hub",
            "quest",
            "Guilde",
            "Panneau officiel, contrats et remise auprès du maître de guilde.",
            "Maître de guilde"
        );
        guildData.status = "Contrats officiels";

        screen.addOption(1, "Quête principale", "Voir ce que l'histoire demande réellement, sans acceptation/refus.", true, "quest.hub.main_story", mainQuestData);
        screen.addOption(2, "Consulter le journal de quêtes", "Voir les quêtes et estimations connues.", true, "quest.hub.journal", journalData);
        screen.addOption(3, "Rendre une quête prête" + (readyCount > 0 ? " [" + std::to_string(readyCount) + "]" : ""),
            readyCount > 0 ? "Choisir un contact et valider une quête terminée." : "Aucune quête prête à rendre.",
            readyCount > 0,
            "quest.hub.ready_turn_in",
            readyData
        );
        screen.addOption(4, "Aller à la guilde", "Consulter le panneau officiel ou rendre un contrat de guilde.", true, "quest.hub.guild", guildData);

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
            displayQuestJournal(player);
        }
        else if (choice == 3)
        {
            openReadyQuestTurnInMenu(player);
        }
        else if (choice == 4)
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
                std::string questLabel = questCardLabel(quest);
                const std::string deadlineStatus = activeQuestDeadlineStatusText(quest, player.getWorldDaysElapsed());
                if (!deadlineStatus.empty())
                {
                    questLabel += " | " + deadlineStatus;
                }
                const int localNumber = static_cast<int>(10 + (i - first));
                MenuOptionItemData itemData;
                itemData.structured = true;
                itemData.kind = "quest";
                itemData.section = questJournalFilterTitle(activeFilter);
                itemData.actionType = quest.guildQuest ? "inspect_contract" : "estimate_request";
                itemData.name = quest.title;
                itemData.detail = quest.guildQuest ? quest.objective : "Demande informelle : " + quest.objective;
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
                    questJournalInspectHint(quest),
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
        screen.addOption(8, "Filtre : rendues", "Archives des quêtes déjà validées.", true, "quest.journal.filter.turned_in");
        PagedMenu::addNavigationOptions(screen, pageIndex, totalPages);

        int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choix invalide.");
        Console::clear();

        if (choice == 0)
        {
            return;
        }

        if (choice >= 1 && choice <= 8)
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
                case 8: activeFilter = QuestJournalFilter::TurnedIn; break;
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
            showQuestDetail(player, *displayedQuests[first + static_cast<std::size_t>(localQuestIndex)]);
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
    if (player.hasTitle("Aventurier"))
    {
        std::vector<std::string> lines = {
            "Tu possèdes déjà le titre Aventurier.",
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
        screen.addLine("Statut d'inscription : " + std::string(isAdventurer ? "Aventurier inscrit" : "Non inscrit"));
        if (isAdventurer)
        {
            std::vector<std::string> standingLines = guildStandingSummaryLines(player);
            for (const std::string& line : standingLines)
            {
                screen.addLine(line);
            }
        }
        screen.addLine("Économie locale : " + Money::coinScaleText());
        screen.addLine("Tu peux avoir jusqu'à 3 quêtes de guilde actives.");
        screen.addLine("Contrats de guilde : " + clientQuestStatusText(guildCounts));
        screen.addLine("Services de guilde à traiter au comptoir : " + std::to_string(activeServiceCount) + ".");
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

        screen.addOption(1, "Voir le panneau de quêtes", guildOpen ? (isAdventurer ? "Consulter les contrats officiels disponibles." : "Inscription Aventurier requise avant d'accepter un contrat officiel.") : "La guilde ne tamponne pas de contrat la nuit.", guildOpen && isAdventurer, "quest.guild.board", boardData);
        screen.addOption(2, "Rendre une quête de guilde terminée" + (guildCounts.ready > 0 ? " [" + std::to_string(guildCounts.ready) + "]" : ""),
            guildCounts.ready > 0 ? "Valider un contrat terminé." : "Aucun contrat de guilde prêt à rendre.",
            guildOpen && guildCounts.ready > 0,
            "quest.guild.turn_in",
            turnInData
        );
        screen.addOption(3, "Consulter le journal", "Lire le journal complet des quêtes.", true, "quest.guild.journal");
        screen.addOption(4, "Traiter un service de guilde" + (activeServiceCount > 0 ? " [" + std::to_string(activeServiceCount) + "]" : ""),
            activeServiceCount > 0 ? "Avancer un contrat de service local." : "Aucun service actif à traiter.",
            guildOpen && isAdventurer && activeServiceCount > 0,
            "quest.guild.service",
            serviceData
        );
        screen.addOption(5, isAdventurer ? "Voir l'inscription aventurier" : "S'inscrire comme aventurier",
            isAdventurer ? "Relire le statut d'inscription." : "Passer le test QCM de la guilde.",
            guildOpen,
            "quest.guild.registration",
            registrationData
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
            itemData.detail = quest.objective;
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
            applySoftServiceFailureCost(player, quest, random, lines);
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
                itemData.detail = quest.objective;
                itemData.status = "À traiter avec " + clientName;
                itemData.reward = approximateQuestRewardText(quest);
                itemData.progress = std::to_string(quest.progress) + "/" + std::to_string(quest.target);
                itemData.owner = clientName;
                itemData.important = true;

                screen.addOption(
                    static_cast<int>(10 + (i - first)),
                    "[Demande - Rang estimé " + quest.rank + "] " + quest.title
                        + " | Progression : " + std::to_string(quest.progress) + "/" + std::to_string(quest.target),
                    "Traiter une étape directement avec le PNJ notable.",
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
                applySoftServiceFailureCost(player, quest, random, lines);
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
            std::string label = questCardLabel(board[i]);
            if (board[i].expiresAtDay >= 0)
            {
                const int remainingDays = std::max(0, board[i].expiresAtDay - player.getWorldDaysElapsed());
                label += " | Expire dans " + std::to_string(remainingDays)
                    + " jour" + (remainingDays > 1 ? "s" : "");
            }
            const bool alreadyTaken = questLog.hasQuest(board[i].id);
            const bool rankAllowed = isGuildQuestRankAllowedForStanding(board[i], standing);
            if (alreadyTaken)
            {
                label += " | Statut : déjà prise";
            }
            if (!rankAllowed)
            {
                label += " | Accès bloqué par la carte";
            }

            MenuOptionItemData itemData;
            itemData.structured = true;
            itemData.kind = "quest";
            itemData.section = "Panneau de guilde";
            itemData.actionType = "quest";
            itemData.name = board[i].title;
            itemData.detail = board[i].objective;
            itemData.status = alreadyTaken ? "Déjà prise" : (rankAllowed ? "Disponible" : "Rang/pastille insuffisant");
            itemData.reward = questRewardText(board[i]);
            itemData.progress = "Rang " + board[i].rank;
            itemData.owner = "Guilde";
            itemData.important = !alreadyTaken && rankAllowed;

            screen.addOption(
                static_cast<int>(i) + 1,
                label,
                rankAllowed ? "Accepter cette quête de guilde si une place est libre." : "La carte magique refuse ce rang pour l'instant.",
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
    struct LocationEntry
    {
        int choice;
        std::string label;
        std::string detail;
        std::string client;
        bool guild = false;
        bool inventory = false;
    };

    const std::vector<LocationEntry> entries = {
        {1, "Guilde", "Contrats officiels, panneau et journal.", "Maître de guilde", true, false},
        {2, "Intendance de Mira", "Référente histoire : priorités de ville, murs, matériaux et validation.", "Mira", false, false},
        {3, "Poste d'Orren", "Référent de route : chemins, ponts, bornes et disparitions.", "Orren", false, false},
        {4, "Infirmerie de Lysa", "Soigneuse de fortune : blessés, remèdes simples et symptômes étranges.", "Lysa", false, false},
        {5, "Forge de Bram", "Forgeron fatigué : réparations, outils et matériaux de survie.", "Bram", false, false},
        {6, "Archives de Soryn", "Archiviste : traces, légendes et indices à vérifier.", "Soryn", false, false},
        {7, "Forge", "Commandes et demandes du forgeron.", "Forgeron", false, false},
        {3, "Herboristerie", "Demandes liées aux plantes et ingrédients.", "Alchimiste", false, false},
        {4, "Place du village", "Rumeurs, habitants et petites demandes.", "Villageois nerveux", false, false},
        {5, "Route commerciale", "Demandes de marchands et risques de voyage.", "Marchand inquiet", false, false},
        {6, "Comptoir de Prunigil", "PNJ notable : QCM de marchand, calcul, français, factures et registres [objectif de quête probable].", "Prunigil le marchand", false, false},
        {7, "Boutique de monstres", "Composants de créatures et revente spécialisée.", "Vendeur de composants", false, false},
        {8, "Boutique de matériaux", "Matériaux, stocks et approvisionnement.", "Vendeur de matériaux", false, false},
        {9, "Armurerie défensive", "Protections, pièces d'armure et commandes.", "Armurier", false, false},
        {10, "Forge d'armes", "Armes, réparation et approvisionnement.", "Vendeur d'armes", false, false},
        {11, "Boutique de consommables", "Potions, consommables et réserves.", "Vendeur de consommables", false, false},
        {12, "Bibliothèque", "Notes, savoirs et pistes de recherche.", "Bibliothécaire", false, false},
        {13, "Bocage aux lanternes", "Lieu visitable nocturne : champignons-lampes, résine d'écho, spores calmes et bêtes attirées par la lumière.", "Mila des lanternes", false, false},
        {14, "Désert d'argile rouge", "Lieu visitable sec : argile rouge, sel lunaire, fausses oasis, pilleurs et constructions fissurées.", "Safa la pisteuse", false, false},
        {15, "Quartier abandonné", "Lieu visitable urbain : caves, maisons vides, contrats douteux, vieilles pièces et automates oubliés.", "Rika des clés", false, false},
        {16, "Mine sifflante", "Lieu visitable souterrain : rails vibrants, fer froid, ressorts, vieux mécanismes et golems de mine.", "Bram le foreur", false, false},
        {17, "Bibliothèque des cartes", "PNJ notable : Archiviste Meron, QCM de connaissances, bestiaire, magie, plantes et transitions [objectif de quête probable].", "Archiviste Meron", false, false},
        {18, "Bureau des inscriptions", "PNJ notable : Scribe Ysolde, paperasse, pastilles, abonnements et litiges [objectif de quête probable].", "Scribe Ysolde", false, false},
        {19, "Laboratoire de Maëra", "PNJ notable : alchimie, dosages, étiquettes de potions et sécurité [objectif de quête probable].", "Maëra l'alchimiste", false, false},
        {20, "Relais des routes", "PNJ notable : Noro le palefrenier, transports, pass, caravanes et bons de livraison [objectif de quête probable].", "Noro le palefrenier", false, false},
        {21, "Auberge du Repos Bruyant", "PNJ notable : Tavia l'aubergiste, hébergement, additions et services de ville [objectif de quête probable].", "Tavia l'aubergiste", false, false},
        {22, "Ouvrir l'inventaire", "Consulter objets et connaissances avant de repartir.", "", false, true}
    };

    constexpr std::size_t locationsPerPage = 8;
    std::size_t pageIndex = 0;

    while (true)
    {
        const std::size_t totalPages = PagedMenu::pageCount(entries.size(), locationsPerPage);
        if (pageIndex >= totalPages)
        {
            pageIndex = totalPages == 0 ? 0 : totalPages - 1;
        }

        const std::size_t first = PagedMenu::firstIndex(pageIndex, locationsPerPage);
        const std::size_t last = PagedMenu::lastIndexExclusive(entries.size(), pageIndex, locationsPerPage);

        MenuScreen screen("LIEUX VISITABLES", "quest.locations");
        screen.setPagination(pageIndex, totalPages);
        screen.addLine("Chaque lieu peut servir à parler, rendre une demande ou vérifier un contact.");
        screen.addLine("Les demandes PNJ restent des pourparlers : seules les quêtes de guilde sont des contrats officiels.");
        screen.addLine("Affichage : " + PagedMenu::rangeText(first, last, entries.size()));
        screen.addBackOption("Retour", "quest.locations.back");

        for (std::size_t i = first; i < last; ++i)
        {
            const LocationEntry& entry = entries[i];
            if (entry.inventory)
            {
                screen.addOption(
                    static_cast<int>(i - first + 1),
                    entry.label,
                    entry.detail,
                    true,
                    "quest.locations.inventory",
                    makeQuestNavigationItemData("location", "Lieux visitables", "open", "Inventaire", entry.detail)
                );
                continue;
            }

            const ClientQuestCounts counts = countQuestsForClient(player, entry.client);
            std::string label = entry.label + " — " + entry.client;
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
                "Lieux visitables",
                entry.detail,
                counts
            );
            itemData.kind = "location";
            itemData.actionType = entry.guild ? "quest" : itemData.actionType;
            itemData.name = entry.label;
            itemData.owner = entry.client;
            itemData.progress = "Lieu " + std::to_string(i + 1) + "/" + std::to_string(entries.size());

            screen.addOption(
                static_cast<int>(i - first + 1),
                label,
                entry.detail + " " + clientQuestHintText(counts),
                true,
                "quest.locations.select." + std::to_string(entry.choice),
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

        if (choice < 1 || static_cast<std::size_t>(choice) > (last - first))
        {
            MessageScreen::show(
                "LIEU INDISPONIBLE",
                "quest.locations.invalid_choice",
                {
                    "Cette entrée n'existe pas sur la page actuelle.",
                    "Utilise les choix affichés ou les boutons de pagination."
                }
            );
            continue;
        }

        const LocationEntry& selected = entries[first + static_cast<std::size_t>(choice - 1)];

        if (selected.inventory)
        {
            InventoryMenu::open(player);
            Console::clear();
            continue;
        }

        if (selected.guild)
        {
            openGuild(player);
        }
        else
        {
            talkToClient(player, selected.client);
        }
    }
}

// EN: openNotableNpcMenu declares or implements a focused behavior used by this module.
// FR: openNotableNpcMenu déclare ou implémente un comportement précis utilisé par ce module.
void QuestMenu::openNotableNpcMenu(Player& player)
{
    expireOverdueQuestDeadlines(player, "quest.notable_npcs");
    constexpr std::size_t clientsPerPage = 8;
    std::size_t pageIndex = 0;

    while (true)
    {
        std::vector<std::pair<std::string, std::string>> entries = {
            {"Maître de guilde", "PNJ important / contrats officiels"},
            {"Mira", "intendante de quartier / référente histoire"},
            {"Orren", "vieux garde / référent de route"},
            {"Lysa", "soigneuse de fortune"},
            {"Bram", "forgeron fatigué"},
            {"Soryn", "archiviste"},
            {"Nell la messagère", "messagère de relais / première survivante de route"},
            {"Eda", "comptable des routes courtes / stocks réels"},
            {"Forgeron", "PNJ client"},
            {"Alchimiste", "PNJ client"},
            {"Villageois nerveux", "PNJ client / événement"},
            {"Marchand inquiet", "PNJ client"},
            {"Prunigil le marchand", "PNJ notable / QCM de comptoir"},
            {"Vendeur de composants", "PNJ client"},
            {"Vendeur de matériaux", "PNJ client"},
            {"Herboriste", "PNJ client"},
            {"Armurier", "PNJ client"},
            {"Vendeur d'armes", "PNJ client"},
            {"Vendeur de consommables", "PNJ client"},
            {"Bibliothécaire", "PNJ client"},
            {"Archiviste Meron", "PNJ notable / QCM de connaissances"},
            {"Scribe Ysolde", "PNJ notable / paperasse et guilde"},
            {"Maëra l'alchimiste", "PNJ notable / alchimie QCM"},
            {"Noro le palefrenier", "PNJ notable / transport et routes"},
            {"Tavia l'aubergiste", "PNJ notable / hébergement et services"}
        };

        const std::vector<std::string> recommendedClients = collectRecommendedClients(player);
        for (const std::string& clientName : recommendedClients)
        {
            entries.push_back({clientName, "Recommandé par un habitant"});
        }

        const std::size_t totalPages = PagedMenu::pageCount(entries.size(), clientsPerPage);
        if (pageIndex >= totalPages)
        {
            pageIndex = totalPages == 0 ? 0 : totalPages - 1;
        }

        const std::size_t first = PagedMenu::firstIndex(pageIndex, clientsPerPage);
        const std::size_t last = PagedMenu::lastIndexExclusive(entries.size(), pageIndex, clientsPerPage);

        MenuScreen screen("PNJ NOTABLES", "quest.notable_npc");
        screen.setPagination(pageIndex, totalPages);
        screen.addLine("Sélectionne un contact pour parler, consulter ses demandes ou rendre ce qui est terminé.");
        screen.addLine("Les PNJ donnent des demandes de vive voix : le journal les estime, il ne les certifie pas comme la guilde.");
        screen.addLine("Affichage : " + PagedMenu::rangeText(first, last, entries.size()));
        screen.addBackOption("Retour", "quest.notable_npc.back");

        if (entries.empty())
        {
            screen.addLine("Aucun PNJ notable n'est disponible pour l'instant.");
            TerminalInterface::askMenuChoiceFromOptions(screen, "Entre 0 pour revenir.");
            Console::clear();
            return;
        }

        bool printedRecommendedHeader = false;
        for (std::size_t i = first; i < last; ++i)
        {
            if (!printedRecommendedHeader && entries[i].second == "Recommandé par un habitant")
            {
                printedRecommendedHeader = true;
                screen.addLine("--- Recommandés par un habitant ---");
            }

            const ClientQuestCounts counts = countQuestsForClient(player, entries[i].first);
            std::string label = entries[i].first + " (" + entries[i].second + ")";

            if (counts.ready > 0)
            {
                label += " [" + std::to_string(counts.ready) + " à rendre]";
            }
            else if (counts.active > 0)
            {
                label += " [" + std::to_string(counts.active) + " en cours]";
            }

            MenuOptionItemData itemData = makeClientQuestNavigationItemData(
                entries[i].first,
                "PNJ notables",
                entries[i].second,
                counts
            );
            itemData.status = entries[i].first == "Maître de guilde"
                ? "Contrats officiels / panneau de guilde"
                : clientQuestStatusText(counts);
            itemData.actionType = entries[i].first == "Maître de guilde" ? "quest" : itemData.actionType;
            itemData.progress = "Contact " + std::to_string(i + 1) + "/" + std::to_string(entries.size());

            screen.addOption(
                static_cast<int>(i - first + 1),
                label,
                entries[i].first == "Maître de guilde"
                    ? "Ouvrir le panneau officiel de guilde."
                    : "Parler, consulter ou rendre une demande auprès de ce contact.",
                true,
                "quest.notable_npc.select." + std::to_string(i + 1),
                itemData
            );
        }

        if (recommendedClients.empty())
        {
            screen.addFooterLine("Recommandés par un habitant : aucun nom pour l'instant.");
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

        if (choice < 1 || static_cast<std::size_t>(choice) > (last - first))
        {
            MessageScreen::show(
                "CONTACT INDISPONIBLE",
                "quest.notable_npc.invalid_choice",
                {
                    "Cette entrée n'existe pas sur la page actuelle.",
                    "Utilise les choix affichés ou les boutons de pagination."
                }
            );
            continue;
        }

        const std::string selectedClient = entries[first + static_cast<std::size_t>(choice - 1)].first;
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
        }
        else if (clientName == "Lysa")
        {
            introLines.push_back("Lysa ne demande pas un miracle, seulement de quoi tenir jusqu'à la prochaine nuit.");
            introLines.push_back("[Objectif de quête probable] Trouver des plantes, signes de symptômes ou composants de soin simples.");
            offeredQuest = QuestCatalog::createBiomeRequest(player.getLevel(), randomBiomeForClient(questRandom, clientName), clientName);
        }
        else if (clientName == "Bram")
        {
            introLines.push_back("Bram désigne les fissures de son enclume avant de parler des murs.");
            introLines.push_back("[Objectif de quête probable] Rapporter métal, cuir, outils ou pièces exploitables pour les réparations.");
            offeredQuest = QuestCatalog::createBiomeRequest(player.getLevel(), randomBiomeForClient(questRandom, clientName), clientName);
        }
        else if (clientName == "Soryn")
        {
            introLines.push_back("Soryn accepte de rouvrir une page, mais seulement si le terrain confirme que ce n'est pas une rumeur de plus.");
            introLines.push_back("[Objectif de quête probable] Vérifier une trace, une archive ou un indice avant d'en faire une légende.");
            offeredQuest = QuestCatalog::createBiomeRequest(player.getLevel(), randomBiomeForClient(questRandom, clientName), clientName);
        }
        else if (clientName == "Nell la messagère")
        {
            introLines.push_back("Nell garde sa sacoche contre elle comme si les routes pouvaient encore essayer de la reprendre.");
            introLines.push_back("[Objectif de quête probable] Protéger une livraison courte, confirmer un passage ou escorter un message entre deux relais.");
            offeredQuest = QuestCatalog::createBiomeRequest(player.getLevel(), "Route commerciale", clientName);
        }
        else if (clientName == "Eda")
        {
            introLines.push_back("Eda refuse les cartes jolies si aucun stock réel n'est revenu pour les confirmer.");
            introLines.push_back("[Objectif de quête probable] Vérifier un retour de route courte, aider un comptoir ou confirmer une livraison pendant les réparations.");
            offeredQuest = QuestCatalog::createTransportLogisticsQuestionRequest(player.getLevel());
            offeredQuest.client = clientName;
        }
        else if (clientName == "Prunigil le marchand")
        {
            introLines.push_back("Prunigil ne te donne pas une fiche de guilde : il te fait travailler directement au comptoir.");
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
            std::string label = quest.guildQuest
                ? "[Contrat officiel - Rang " + quest.rank + "] " + quest.title + " | " + questRewardText(quest)
                : "[Pourparler PNJ - Rang estimé " + quest.rank + "] " + quest.title + " | " + approximateQuestRewardText(quest);

            if (isMaterialDeliveryQuest(quest))
            {
                label += " | " + quest.requiredMaterialName
                    + " " + std::to_string(player.getInventory().countMaterialById(quest.requiredMaterialId))
                    + " (équiv. " + std::to_string(player.getInventory().countMaterialQualityPointsById(quest.requiredMaterialId) / 2)
                    + ")/" + std::to_string(quest.requiredMaterialQuantity);
            }

            MenuOptionItemData itemData;
            itemData.structured = true;
            itemData.kind = "quest";
            itemData.section = "Quêtes à rendre";
            itemData.actionType = "turn_in";
            itemData.name = quest.title;
            itemData.detail = quest.objective;
            itemData.status = quest.guildQuest ? "Prête à tamponner" : "Prête à confirmer";
            itemData.reward = quest.guildQuest ? questRewardText(quest) : approximateQuestRewardText(quest);
            itemData.progress = std::to_string(quest.progress) + "/" + std::to_string(quest.target);
            itemData.owner = quest.client;
            itemData.important = true;

            screen.addOption(
                static_cast<int>(10 + (i - first)),
                label,
                quest.guildQuest
                    ? "Tamponner ce contrat officiel et recevoir les récompenses."
                    : "Confirmer cette demande informelle auprès du PNJ.",
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

        const int goldBefore = player.getInventory().getGold();
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
        player.gainExperience(balancedQuestExperience(quest));
        player.getInventory().earnGold(balancedQuestGold(quest));
        applyQuestExtraReward(player, quest);
        if (quest.guildQuest)
        {
            applyGuildStandingRewards(player, resultLines);
            applyQuestTitleRewards(player, quest, resultLines);
        }

        appendQuestRewardResultLines(resultLines, quest);
        resultLines.push_back("Argent : " + Money::formatGold(goldBefore) + " -> " + Money::formatGold(player.getInventory().getGold()));
        resultLines.push_back("XP : " + std::to_string(experienceBefore) + " -> " + std::to_string(player.getExperience()));
        if (player.getLevel() != levelBefore)
        {
            resultLines.push_back("Niveau : " + std::to_string(levelBefore) + " -> " + std::to_string(player.getLevel()));
        }

        MessageScreen::show(
            quest.guildQuest ? "CONTRAT VALIDÉ" : "DEMANDE VALIDÉE",
            quest.guildQuest ? "quest.turn_in.guild_completed" : "quest.turn_in.personal_completed",
            resultLines
        );

        bool hasMoreReadyForClient = false;
        for (const Quest& remainingQuest : quests)
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

        bool hasEvolvedBiome = false;
        int hiddenBiomeCount = 0;
        int unknownRumorCount = 0;
        std::vector<int> visibleBiomeIndexes;

        for (int i = 0; i < static_cast<int>(biomes.size()); ++i)
        {
            const ExplorationBiome& biomePreview = biomes[i];

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

        MicroChallengeResult microChallenge = runExplorationMicroChallenge(biome, intensity, random);
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
        auto runExplorationEvent = [&](int eventRoll, int eventIndex) {
            eventRoll = std::clamp(eventRoll, 1, 100);
            const std::string eventLabel = explorationEventLabelFromRoll(eventRoll);
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
                std::vector<std::string> lines = {"Tu fouilles calmement la zone."};
                if (carefulRecovery)
                {
                    lines.push_back("Récolte propre : tu récupères la ressource de la meilleure façon possible.");
                }
                lines.push_back(addExplorationMaterial(player, biome.commonMaterialId, applyExplorationQuantityBonus(random.between(1, 2), intensity), chooseExplorationQuality(random, carefulRecovery)));
                showExplorationNotice("RÉCOLTE", "exploration.run.gather", lines);
            }
            else if (eventRoll <= 40)
            {
                std::vector<std::string> lines = {"Une trace intéressante attire ton attention."};
                int updated = progressExplorationQuests(player, biome.name, random.between(1, 2));
                if (updated > 0)
                {
                    lines.push_back("Des quêtes d'exploration progressent grâce à cette découverte.");
                }
                else
                {
                    lines.push_back("Tu gardes mentalement l'endroit en tête, même si aucune quête actuelle ne l'exploite.");
                }
                showExplorationNotice("TRACE INTÉRESSANTE", "exploration.run.trace", lines);
            }
            else if (eventRoll <= 52)
            {
                int gold = applyExplorationGoldReward(random.between(5, 22 + player.getLevel() * 2), player, intensity, difficulty, 1);
                player.getInventory().earnGold(gold);
                showExplorationNotice(
                    "PETIT TRÉSOR",
                    "exploration.run.gold",
                    {"Tu trouves un petit trésor au sol.", "Argent gagné : " + Money::formatGoldWithRaw(gold)}
                );
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

            if (eventIndex > 1)
            {
                eventRoll = std::clamp(eventRoll + 4 + nightRollShift / 2 + std::max(0, temperatureRollShift / 2), 1, 100);
            }

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
