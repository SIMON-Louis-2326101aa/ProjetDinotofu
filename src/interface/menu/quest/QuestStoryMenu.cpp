// EN: Main-story quest synchronization and active story menu extracted from QuestMenu.cpp.
// FR: Synchronisation et menu actif des quêtes principales extraits de QuestMenu.cpp.

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

using namespace QuestStorySupport;
using namespace QuestPresentationSupport;

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
        // Development cap: Chapter 3 is introduced, but its quest chain is intentionally not
        // injected into normal saves until the chapter receives its planned full rework.
        if (player.getStoryStep() < 1)
        {
            player.setStoryProgress(3, 1, std::max(9, player.getStoryCityDevelopmentLevel()));
        }
        player.getQuestLog().refreshLinkedQuestProgress();
        return;
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
            screen.addLine("Progression : introduction atteinte.");
            screen.addLine("");
            screen.addLine("[SUITE DE L'HISTOIRE INDISPONIBLE]");
            screen.addLine("La suite du chapitre 3 n'est pas encore accessible.");
            screen.addLine("Aucune étape incomplète n'est ajoutée au journal principal.");
            screen.addLine("Le monde libre, les contrats, l'exploration et les autres systèmes restent jouables.");
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

