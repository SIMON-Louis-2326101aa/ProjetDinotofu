// EN: GameStory.cpp isolates the existing story-mode orchestration from the main Game runtime.
// FR: GameStory.cpp isole l’orchestration existante du mode histoire du runtime principal de Game.
// No story progression beyond the already-authorized chapter-3 introduction is added here.

#include "core/Game.hpp"
#include "diagnostic/RuntimeLog.hpp"
#include "core/Console.hpp"
#include "core/Random.hpp"
#include "core/VersionInfo.hpp"
#include "class_system/ClassCatalog.hpp"
#include "combat/Combat.hpp"
#include "combat/system/CombatClassSystem.hpp"
#include "combat/modes/pve/MonsterPveMode.hpp"
#include "boss/BossCatalog.hpp"
#include "entity/Monster.hpp"
#include "character/RaceCatalog.hpp"
#include "character/SpecialCharacterNativeBonus.hpp"
#include "save/SaveManager.hpp"
#include "save/menu/AccountMenu.hpp"
#include "save/menu/CharacterMenu.hpp"
#include "economy/shop/ShopRotationSystem.hpp"
#include "economy/shop/ShopTransactionSystem.hpp"
#include "interface/menu/shop/ShopMenu.hpp"
#include "interface/menu/progression/AttributeMenu.hpp"
#include "interface/menu/progression/StatisticsMenu.hpp"
#include "interface/menu/quest/QuestMenu.hpp"
#include "interface/menu/InventoryMenu.hpp"
#include "interface/menu/PostCombatMenu.hpp"
#include "interface/menu/training/TrainingGroundMenu.hpp"
#include "interface/TerminalInterface.hpp"
#include "interface/model/MenuScreen.hpp"
#include "interface/menu/common/MessageScreen.hpp"
#include "interface/menu/common/PagedMenu.hpp"
#include "cheat/CheatManager.hpp"
#include "progression/DifficultyRules.hpp"
#include "progression/DeathRuleRules.hpp"
#include "progression/death/DeathPenaltySystem.hpp"
#include "item/weapon/Weapon.hpp"
#include "item/armor/Armor.hpp"
#include "item/consumable/Consumable.hpp"
#include "item/material/Material.hpp"
#include "story/StoryCampaign.hpp"

#include <algorithm>
#include <cctype>
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <cstdlib>
#include <exception>
#include <set>
#include <array>


namespace
{
    Quest* findMutableStoryQuest(Player& player, const std::string& questId)
    {
        for (Quest& quest : player.getQuestLog().getQuests())
        {
            if (quest.id == questId)
            {
                return &quest;
            }
        }
        return nullptr;
    }

    bool questInState(const Player& player, const std::string& questId, const std::string& state)
    {
        for (const Quest& quest : player.getQuestLog().getQuests())
        {
            if (quest.id != questId || quest.failed)
            {
                continue;
            }

            if (state == "active") return quest.accepted && !quest.completed && !quest.turnedIn;
            if (state == "completed") return quest.completed && !quest.turnedIn;
            if (state == "turned_in") return quest.turnedIn;
        }
        return false;
    }

    bool storyQuestTurnedIn(const Player& player, const std::string& questId)
    {
        return questInState(player, questId, "turned_in");
    }

    bool storyQuestCompleted(const Player& player, const std::string& questId)
    {
        return questInState(player, questId, "completed");
    }

    bool completeStoryQuestSilently(Player& player, const std::string& questId)
    {
        for (Quest& quest : player.getQuestLog().getQuests())
        {
            if (quest.id == questId && !quest.turnedIn && !quest.failed)
            {
                quest.accepted = true;
                quest.progress = std::max(quest.target, 1);
                quest.completed = true;
                quest.turnedIn = true;
                quest.expiresAtDay = -1;
                player.getQuestLog().refreshLinkedQuestProgress();
                return true;
            }
        }
        return false;
    }

    int countTurnedInChapterOneReferentQuests(const Player& player)
    {
        const std::vector<std::string> ids = {
            "story_ch1_orren_main",
            "story_ch1_lysa_main",
            "story_ch1_bram_main",
            "story_ch1_soryn_main"
        };
        return static_cast<int>(std::count_if(ids.begin(), ids.end(), [&](const std::string& id) {
            return storyQuestTurnedIn(player, id);
        }));
    }

    bool progressStoryQuestById(Player& player, const std::string& questId, int amount)
    {
        if (amount <= 0)
        {
            return false;
        }
        return player.getQuestLog().progressQuest(questId, amount);
    }

    std::string storyQuestProgressText(const Player& player, const std::string& questId)
    {
        for (const Quest& quest : player.getQuestLog().getQuests())
        {
            if (quest.id != questId)
            {
                continue;
            }
            if (quest.failed) return "bloquée";
            if (quest.turnedIn) return "validée";
            if (quest.completed) return "prête à rendre";
            if (quest.accepted) return "en cours " + std::to_string(quest.progress) + "/" + std::to_string(quest.target);
            return "connue";
        }
        return "à débloquer";
    }

    int storyQuestProgressValue(const Player& player, const std::string& questId)
    {
        for (const Quest& quest : player.getQuestLog().getQuests())
        {
            if (quest.id == questId)
            {
                return std::max(0, quest.progress);
            }
        }
        return 0;
    }
}

void Game::launchStoryModePlaceholder()
{
    bool menuOpen = true;
    while (menuOpen)
    {
        const bool storyStarted = mainPlayer.hasStoryModeStarted();
        const int maxUnlockedChapter = StoryCampaign::maxUnlockedChapter(mainPlayer);

        MenuScreen screen("MODE HISTOIRE", "story.entry.menu");
        screen.addSubtitle("Route principale — page courte");
        screen.addLine("Progression : " + mainPlayer.getStoryProgressLabel());
        screen.addLine("Chapitre maximum débloqué : " + std::to_string(maxUnlockedChapter) + ".");
        screen.addBackOption();
        screen.addOption(
            1,
            "Continuer",
            storyStarted
                ? "Lancer la prochaine étape disponible de l'histoire."
                : "[◘ aucune histoire commencée pour ce personnage]",
            storyStarted,
            "story.continue"
        );
        screen.addOption(
            2,
            "Nouvelle histoire",
            "Repartir au vrai début : fumée blanche, niveau 1, aucun confort de bac à sable.",
            true,
            "story.new"
        );
        screen.addOption(
            3,
            "Sélectionner le chapitre",
            storyStarted
                ? "Choisir uniquement parmi les chapitres déjà débloqués. Les autres restent verrouillés."
                : "[◘ commence une nouvelle histoire avant de sélectionner un chapitre]",
            storyStarted,
            "story.chapter_select"
        );
        screen.addOption(4, "Infos : ville / PNJ / objectifs", "Lire les informations de contexte regroupées.", true, "story.info_group");
        screen.addOption(5, "Règles : histoire / bac à sable", "Reset, clone éphémère et bascule libre après fin d'histoire.", true, "story.rules_group");
        screen.addOption(6, "Bac à sable éphémère", "Créer un clone non sauvegardé pour jouer librement sans perturber l'histoire.", storyStarted, "story.ephemeral_sandbox");
        screen.addOption(7, "Accès rapides histoire", "Ouvrir quêtes, PNJ, lieux, inventaire et état de progression depuis une seule page.", true, "story.access");
        addOutOfCombatUtilityOptions(screen, true, true);

        const int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choisis une entrée du mode histoire.");
        Console::clear();

        if (handleOutOfCombatUtilityChoice(choice, true))
        {
            continue;
        }

        if (choice == 0)
        {
            return;
        }
        if (choice == 1)
        {
            continueStoryRoute();
            continue;
        }
        if (choice == 2)
        {
            startNewStoryFromMenu();
            continue;
        }
        if (choice == 3)
        {
            openStoryChapterSelectionMenu();
            continue;
        }
        if (choice == 4)
        {
            bool infoOpen = true;
            while (infoOpen)
            {
                MenuScreen infoScreen("INFOS HISTOIRE", "story.info_group.menu");
                infoScreen.addSubtitle("Contexte regroupé");
                infoScreen.addBackOption();
                infoScreen.addOption(1, "Lire la longue introduction", "Ton de départ et contexte général.", true, "story.long_intro");
                infoScreen.addOption(2, "Développement de la ville", "Voir ce qui est ouvert, limité ou fermé dans la route histoire.", true, "story.city_development");
                infoScreen.addOption(3, "Clients amis / référents", "Voir les PNJ qui peuvent aider ou orienter la quête principale.", true, "story.referents");
                infoScreen.addOption(4, "Intrigues suivies", "Voir les grands fils narratifs prévus sans révéler la fin.", true, "story.intrigues");
                infoScreen.addOption(5, "Prochains objectifs", "Voir missions principales et secondaires utiles au développement.", true, "story.next_objectives");

                const int infoChoice = TerminalInterface::askMenuChoiceFromOptions(infoScreen, "Choisis une information histoire.");
                Console::clear();

                if (infoChoice == 0)
                {
                    infoOpen = false;
                    continue;
                }
                if (infoChoice == 1)
                {
                    MessageScreen::show("INTRODUCTION", "story.long_intro", StoryCampaign::buildLongIntroductionLines(mainPlayer));
                    continue;
                }
                if (infoChoice == 2)
                {
                    MessageScreen::show("DÉVELOPPEMENT DE LA VILLE", "story.city_development", StoryCampaign::buildDevelopmentLines(mainPlayer));
                    continue;
                }
                if (infoChoice == 3)
                {
                    MessageScreen::show("CLIENTS AMIS / RÉFÉRENTS", "story.referents", StoryCampaign::buildReferentNpcLines(mainPlayer));
                    continue;
                }
                if (infoChoice == 4)
                {
                    MessageScreen::show("INTRIGUES", "story.intrigues", StoryCampaign::buildIntrigueLines(mainPlayer));
                    continue;
                }
                if (infoChoice == 5)
                {
                    MessageScreen::show("OBJECTIFS HISTOIRE", "story.next_objectives", StoryCampaign::buildNextObjectiveLines(mainPlayer));
                    continue;
                }
            }
            continue;
        }
        if (choice == 5)
        {
            bool rulesOpen = true;
            while (rulesOpen)
            {
                MenuScreen rulesScreen("RÈGLES HISTOIRE", "story.rules_group.menu");
                rulesScreen.addSubtitle("Règles regroupées");
                rulesScreen.addBackOption();
                rulesScreen.addOption(1, "Bac à sable / histoire", "Règles de reset, clone éphémère et progression séparée.", true, "story.sandbox_rules");
                rulesScreen.addOption(2, "Fin d'histoire → bac à sable", "Après la conclusion, le personnage continue automatiquement en mode libre.", true, "story.completion_sandbox");
                rulesScreen.addOption(3, "Ordre des chapitres", "Rappelle le rôle de Continuer et de la sélection de chapitre.", true, "story.chapter_order_rules");

                const int rulesChoice = TerminalInterface::askMenuChoiceFromOptions(rulesScreen, "Choisis une règle histoire.");
                Console::clear();

                if (rulesChoice == 0)
                {
                    rulesOpen = false;
                    continue;
                }
                if (rulesChoice == 1)
                {
                    MessageScreen::show("BAC À SABLE / HISTOIRE", "story.sandbox_rules", StoryCampaign::buildSandboxRulesLines(mainPlayer));
                    continue;
                }
                if (rulesChoice == 2)
                {
                    showStoryCompletionSandboxRule();
                    continue;
                }
                if (rulesChoice == 3)
                {
                    MessageScreen::show(
                        "ORDRE DES CHAPITRES",
                        "story.chapter_order_rules",
                        {
                            "Nouvelle histoire lance le vrai début : fumée blanche, forêt, aucun équipement.",
                            "Continuer est volontairement l'option principale : il reprend la prochaine étape disponible.",
                            "Sélectionner le chapitre sert seulement à revoir/continuer un chapitre déjà débloqué.",
                            "Les chapitres non débloqués restent affichés comme ???? et ne sont pas sélectionnables."
                        },
                        false
                    );
                    continue;
                }
            }
            continue;
        }
        if (choice == 6)
        {
            launchEphemeralSandboxCloneFromStory();
            continue;
        }
        if (choice == 7)
        {
            openStoryAccessMenu();
            continue;
        }
    }
}

void Game::startNewStoryFromMenu()
{
    MenuScreen confirm("NOUVELLE HISTOIRE", "story.new.confirm");
    confirm.addSubtitle("Départ commun : fumée blanche");
    confirm.addLine("Une nouvelle histoire remet le personnage au départ narratif commun.");
    confirm.addLine("Niveau 1, inventaire vidé avant le prologue, routes et confort du bac à sable perdus pour cette route histoire.");
    if (mainPlayer.hasStoryModeStarted())
    {
        confirm.addLine("Attention : ce personnage a déjà une histoire en cours. La recommencer effacera cette progression histoire.");
    }
    else if (shouldResetCharacterForStoryStart())
    {
        confirm.addLine("Attention : ce personnage vient du bac à sable ou a déjà progressé. L’histoire le remettra à zéro.");
    }
    confirm.addOption(0, "Annuler", "Retourner au menu histoire.", true, "story.new.cancel");
    confirm.addOption(1, "Commencer depuis la fumée blanche", "Réinitialiser puis jouer le prologue dans l’ordre.", true, "story.new.accept");
    const int confirmChoice = TerminalInterface::askMenuChoiceFromOptions(confirm, "Confirme la nouvelle histoire.");
    Console::clear();

    if (confirmChoice != 1)
    {
        MessageScreen::show("NOUVELLE HISTOIRE ANNULÉE", "story.new.cancelled", {"Le personnage reste dans son état actuel."}, false);
        return;
    }

    resetMainPlayerForStoryStart();
    saveCurrentProgress("Nouvelle histoire initialisée");
    playStoryWhiteFogPrologue();
}

void Game::continueStoryRoute()
{
    if (!mainPlayer.hasStoryModeStarted())
    {
        MessageScreen::show(
            "AUCUNE HISTOIRE",
            "story.continue.not_started",
            {
                "Aucune route histoire n’est commencée pour ce personnage.",
                "Utilise Nouvelle histoire pour démarrer depuis la fumée blanche."
            },
            false
        );
        return;
    }

    if (mainPlayer.getStoryStep() < 2 || mainPlayer.getInventory().getWeaponCount() <= 0)
    {
        playStoryWhiteFogPrologue();
        return;
    }

    if (StoryCampaign::canUnlockChapterTwo(mainPlayer) && mainPlayer.getStoryChapter() < 2)
    {
        mainPlayer.setStoryProgress(2, 1, std::max(2, mainPlayer.getStoryCityDevelopmentLevel()));
        saveCurrentProgress("Chapitre 2 débloqué par progression histoire");
        MessageScreen::show(
            "CHAPITRE 2 DÉBLOQUÉ",
            "story.chapter_2.unlocked",
            {
                "Les preuves, les monstres vaincus ou le premier boss ont assez fait bouger la ville.",
                "La route du relais silencieux peut commencer.",
                "Les chapitres restent joués dans l’ordre : le chapitre 2 est maintenant sélectionnable."
            },
            false
        );
    }

    if (StoryCampaign::canUnlockChapterThree(mainPlayer) && mainPlayer.getStoryChapter() < 3)
    {
        mainPlayer.setStoryProgress(3, 1, std::max(9, mainPlayer.getStoryCityDevelopmentLevel()));
        QuestMenu::syncMainStoryQuests(mainPlayer);
        saveCurrentProgress("Chapitre 3 débloqué par la route gardée");
        MessageScreen::show(
            "CHAPITRE 3 DÉBLOQUÉ",
            "story.chapter_3.unlocked",
            {
                "Le premier convoi gardé est revenu. Il est revenu seul, avec davantage de marchandises qu'au départ.",
                "Mira refuse qu'on décharge quoi que ce soit avant d'avoir compris quelle route l'a réellement ramené.",
                "Chapitre 3 débloqué : Les routes qui répondent mal."
            },
            false
        );
    }

    if (false && mainPlayer.getStoryChapter() >= 4)
    {
        playStoryChapterFour();
        return;
    }

    if (mainPlayer.getStoryChapter() >= 3)
    {
        playStoryChapterThree();
        return;
    }

    if (mainPlayer.getStoryChapter() >= 2)
    {
        playStoryChapterTwo();
        return;
    }

    playStoryChapterOne();
}

void Game::openStoryChapterSelectionMenu()
{
    if (!mainPlayer.hasStoryModeStarted())
    {
        MessageScreen::show(
            "SÉLECTION IMPOSSIBLE",
            "story.chapter_select.not_started",
            {
                "Aucun chapitre n’est encore débloqué.",
                "Commence une nouvelle histoire pour ouvrir le prologue et le chapitre 1."
            },
            false
        );
        return;
    }

    bool selecting = true;
    while (selecting)
    {
        const int maxUnlocked = StoryCampaign::maxUnlockedChapter(mainPlayer);
        const bool chapterTwoUnlocked = StoryCampaign::isChapterUnlocked(mainPlayer, 2);
        const bool chapterThreeUnlocked = StoryCampaign::isChapterUnlocked(mainPlayer, 3);
        const bool chapterFourUnlocked = StoryCampaign::isChapterUnlocked(mainPlayer, 4);

        MenuScreen screen("SÉLECTIONNER LE CHAPITRE", "story.chapter_select.menu");
        screen.addSubtitle("Seuls les chapitres débloqués sont sélectionnables");
        screen.addLine("Progression : " + mainPlayer.getStoryProgressLabel());
        screen.addLine("Chapitre maximum débloqué : " + std::to_string(maxUnlocked) + ".");
        screen.addBackOption();
        screen.addOption(1, "Prologue : fumée blanche", "Revoir ou jouer le prologue si le kit de départ n’a pas encore été récupéré.", true, "story.chapter_select.prologue");
        screen.addOption(2, "Chapitre 1 : La ville qui tient à peine", "Chapitre débloqué par le départ histoire.", true, "story.chapter_select.chapter_1");
        screen.addOption(
            3,
            chapterTwoUnlocked ? "Chapitre 2 : Le relais silencieux" : "????",
            chapterTwoUnlocked ? "Chapitre débloqué : le relais peut être consulté/continué." : "[◘ chapitre verrouillé : avance dans l’histoire, sauve/débloque les preuves nécessaires]",
            chapterTwoUnlocked,
            "story.chapter_select.chapter_2"
        );
        screen.addOption(
            4,
            chapterThreeUnlocked ? "Chapitre 3 : Les routes qui répondent mal" : "????",
            chapterThreeUnlocked ? "Chapitre débloqué : enquêter sur le convoi revenu seul." : "[◘ chapitre verrouillé : stabilise d'abord la route gardée du chapitre 2]",
            chapterThreeUnlocked,
            "story.chapter_select.chapter_3"
        );
        screen.addOption(
            5,
            chapterFourUnlocked ? "Chapitre 4 : Le village à la mauvaise date" : "????",
            chapterFourUnlocked ? "Première phase débloquée : enquêter avant de choisir le boss majeur." : "[◘ chapitre verrouillé : classe d'abord le convoi et sa route corrigée]",
            chapterFourUnlocked,
            "story.chapter_select.chapter_4"
        );
        screen.addOption(6, "Règle d’ordre", "Rappelle que Continuer reprend la prochaine étape.", true, "story.chapter_select.rules");

        const int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choisis un chapitre débloqué.");
        Console::clear();

        if (choice == 0)
        {
            return;
        }
        if (choice == 1)
        {
            playStoryWhiteFogPrologue();
            continue;
        }
        if (choice == 2)
        {
            playStoryChapterOne();
            continue;
        }
        if (choice == 3)
        {
            if (!chapterTwoUnlocked)
            {
                MessageScreen::show("CHAPITRE VERROUILLÉ", "story.chapter_select.chapter_2.locked", {"Le chapitre 2 n’est pas encore débloqué pour ce personnage."}, false);
                continue;
            }
            if (mainPlayer.getStoryChapter() < 2)
            {
                mainPlayer.setStoryProgress(2, 1, std::max(2, mainPlayer.getStoryCityDevelopmentLevel()));
                saveCurrentProgress("Chapitre 2 sélectionné après déblocage");
            }
            playStoryChapterTwo();
            continue;
        }
        if (choice == 4)
        {
            if (!chapterThreeUnlocked)
            {
                MessageScreen::show("CHAPITRE VERROUILLÉ", "story.chapter_select.chapter_3.locked", {"Le chapitre 3 demande d'abord une route gardée réellement stabilisée."}, false);
                continue;
            }
            if (mainPlayer.getStoryChapter() < 3)
            {
                mainPlayer.setStoryProgress(3, 1, std::max(9, mainPlayer.getStoryCityDevelopmentLevel()));
                QuestMenu::syncMainStoryQuests(mainPlayer);
                saveCurrentProgress("Chapitre 3 sélectionné après déblocage");
            }
            playStoryChapterThree();
            continue;
        }
        if (choice == 5)
        {
            if (!chapterFourUnlocked)
            {
                MessageScreen::show("CHAPITRE VERROUILLÉ", "story.chapter_select.chapter_4.locked", {"Le chapitre 4 demande que le convoi du chapitre 3 soit entièrement classé et rendu."}, false);
                continue;
            }
            if (mainPlayer.getStoryChapter() < 4)
            {
                mainPlayer.setStoryProgress(4, 1, std::max(10, mainPlayer.getStoryCityDevelopmentLevel()));
                saveCurrentProgress("Chapitre 4 sélectionné après déblocage");
            }
            playStoryChapterFour();
            continue;
        }
        if (choice == 6)
        {
            MessageScreen::show(
                "ORDRE DES CHAPITRES",
                "story.chapter_select.rules",
                {
                    "Nouvelle histoire lance le vrai début : fumée blanche, forêt, aucun équipement.",
                    "Continuer ne demande pas de choisir un chapitre : il reprend la prochaine étape disponible.",
                    "Sélectionner le chapitre sert seulement à revoir/continuer un chapitre déjà débloqué.",
                    "Les chapitres non débloqués restent affichés comme ???? et ne sont pas sélectionnables."
                },
                false
            );
            continue;
        }
    }
}



void Game::openStoryAccessMenu()
{
    bool menuOpen = true;
    while (menuOpen)
    {
        QuestMenu::syncMainStoryQuests(mainPlayer);

        MenuScreen screen("ACCÈS HISTOIRE", "story.access.menu");
        screen.addSubtitle("Raccourcis utiles sans casser la route principale");
        screen.addLine("Progression : " + mainPlayer.getStoryProgressLabel());
        screen.addLine("Toutes les informations importantes restent écrites : les menus suivants ouvrent les vrais systèmes du jeu.");
        screen.addBackOption();
        screen.addOption(1, "Quête principale / journal", "Voir objectifs, quêtes prêtes à rendre et demandes liées à l'histoire.", true, "story.access.quests");
        screen.addOption(2, "PNJ notables", "Parler à Mira, Orren, Lysa, Bram, Soryn et autres contacts connus.", true, "story.access.npcs");
        screen.addOption(3, "Lieux, boutiques et services", "Ouvrir les endroits précis disponibles : ville, boutiques, forge, auberge, routes et services.", true, "story.access.locations");
        screen.addOption(4, "Inventaire", "Vérifier équipement, consommables et objets liés à la route histoire.", true, "story.access.inventory");
        screen.addOption(5, "État de progression", "Lire ce qui manque sans forcer de validation gratuite.", true, "story.access.progress");
        screen.addOption(6, "Resynchroniser le journal", "Réaffiche les quêtes principales connues si un menu n'avait pas encore été ouvert.", true, "story.access.resync");

        const int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choisis un accès histoire.");
        Console::clear();

        if (choice == 0)
        {
            return;
        }
        if (choice == 1)
        {
            QuestMenu::consultOnly(mainPlayer);
            saveCurrentProgress("Histoire : quêtes consultées");
            continue;
        }
        if (choice == 2)
        {
            QuestMenu::openNotableNpcMenu(mainPlayer);
            saveCurrentProgress("Histoire : PNJ notables consultés");
            continue;
        }
        if (choice == 3)
        {
            QuestMenu::openLocations(mainPlayer);
            saveCurrentProgress("Histoire : lieux et services consultés");
            continue;
        }
        if (choice == 4)
        {
            InventoryMenu::open(mainPlayer);
            saveCurrentProgress("Histoire : inventaire consulté");
            Console::clear();
            continue;
        }
        if (choice == 5)
        {
            std::vector<std::string> lines;
            lines.push_back("Progression actuelle : " + mainPlayer.getStoryProgressLabel());
            lines.push_back("Chapitre maximum débloqué : " + std::to_string(StoryCampaign::maxUnlockedChapter(mainPlayer)) + ".");
            lines.push_back("");
            if (!mainPlayer.hasStoryModeStarted())
            {
                lines.push_back("Aucune histoire commencée : utilise Nouvelle histoire.");
            }
            else if (mainPlayer.getStoryStep() < 2 || mainPlayer.getInventory().getWeaponCount() <= 0)
            {
                lines.push_back("Prologue incomplet : récupère le kit de départ dans la fumée blanche.");
            }
            else if (mainPlayer.getStoryChapter() <= 1)
            {
                const std::vector<std::string> chapterLines = StoryCampaign::buildChapterOneProgressLines(mainPlayer);
                lines.insert(lines.end(), chapterLines.begin(), chapterLines.end());
                lines.push_back("");
                lines.push_back("Raccourci conseillé : Accès histoire > PNJ notables pour parler aux référents, puis Quête principale / journal pour suivre les rendus.");
            }
            else if (mainPlayer.getStoryChapter() == 2)
            {
                const std::vector<std::string> chapterLines = StoryCampaign::buildChapterTwoProgressLines(mainPlayer);
                lines.insert(lines.end(), chapterLines.begin(), chapterLines.end());
                lines.push_back("");
                lines.push_back("Raccourci conseillé : Accès histoire > Lieux, boutiques et services si une étape demande une route ou un service déjà débloqué.");
            }
            else if (mainPlayer.getStoryChapter() == 3)
            {
                lines.push_back("Chapitre 3 actif : utilise Continuer pour l'étape suivante, ou PNJ notables si une quête est prête à rendre.");
            }
            else
            {
                lines.push_back("Chapitre 4 actif : utilise Continuer pour l'enquête, ou PNJ notables / Quête principale si un rendu est demandé.");
            }

            MessageScreen::show("ÉTAT HISTOIRE", "story.access.progress.detail", lines, false);
            continue;
        }
        if (choice == 6)
        {
            QuestMenu::syncMainStoryQuests(mainPlayer);
            saveCurrentProgress("Histoire : journal resynchronisé");
            MessageScreen::show(
                "JOURNAL RESYNCHRONISÉ",
                "story.access.resync.done",
                {
                    "Le journal histoire a été relu avec l'état réel du personnage.",
                    "Aucune étape n'a été validée gratuitement.",
                    "Les quêtes déjà terminées ou prêtes à rendre devraient maintenant apparaître dans les vrais menus concernés."
                },
                false
            );
            continue;
        }
    }
}


bool Game::shouldResetCharacterForStoryStart() const
{
    if (mainPlayer.hasStoryModeStarted())
    {
        return false;
    }

    return mainPlayer.getLevel() > 1
        || mainPlayer.getExperience() > 0
        || mainPlayer.getVictories() > 0
        || mainPlayer.getDefeats() > 0
        || mainPlayer.getEscapes() > 0
        || mainPlayer.getEnemiesKilled() > 0
        || mainPlayer.getBossesKilled() > 0
        || mainPlayer.getDeaths() > 0
        || mainPlayer.getInventory().getGold() > 0
        || mainPlayer.getInventory().getWeaponCount() > 0
        || mainPlayer.getInventory().getArmorCount() > 0
        || mainPlayer.getInventory().getConsumableCount() > 0
        || mainPlayer.getInventory().getMaterialCount() > 0;
}

void Game::resetMainPlayerForStoryStart()
{
    const std::string oldName = mainPlayer.getName().empty() ? playerName : mainPlayer.getName();
    const std::string oldClassName = mainPlayer.getType().empty() ? "Chevalier" : mainPlayer.getType();
    const CharacterRace oldRace = mainPlayer.getRace();
    const std::string hintFrequency = mainPlayer.getInterfaceHintFrequency();
    const std::string creatorAccount = mainPlayer.getCreatorAccountName();
    const std::string ownerAccount = mainPlayer.getCurrentOwnerAccountName();

    PlayerClass baseClass = ClassCatalog::createClassByName(oldClassName);
    mainPlayer = Player(oldName, baseClass);
    mainPlayer.setRace(oldRace);
    mainPlayer.getInventory().clearAll();
    mainPlayer.setInterfaceHintFrequency(hintFrequency);
    mainPlayer.setOwnershipMetadata(creatorAccount.empty() ? accountName : creatorAccount, ownerAccount.empty() ? accountName : ownerAccount);
    mainPlayer.grantTitle("Bienvenue dans Dinotofu");
    mainPlayer.grantTitle("Voix du terminal");
    mainPlayer.startStoryMode();
    mainPlayer.setStoryProgress(1, 1, 0);
    playerName = oldName;

    MessageScreen::show(
        "HISTOIRE — RESET",
        "story.start.reset.done",
        {
            "Le personnage est revenu au départ histoire.",
            "Niveau : 1.",
            "Inventaire : vidé avant le prologue.",
            "La fumée blanche a mangé les souvenirs utiles, les routes connues et le confort du bac à sable."
        },
        false
    );
}

void Game::playStoryChapterOne()
{
    if (!mainPlayer.hasStoryModeStarted())
    {
        MessageScreen::show(
            "CHAPITRE 1 IMPOSSIBLE",
            "story.chapter_1.not_started",
            {
                "Le chapitre 1 appartient à la route histoire.",
                "Commence une Nouvelle histoire pour traverser d’abord la fumée blanche."
            },
            false
        );
        return;
    }

    if (mainPlayer.getStoryStep() < 2 || mainPlayer.getInventory().getWeaponCount() <= 0)
    {
        playStoryWhiteFogPrologue();
        return;
    }

    if (mainPlayer.getStoryStep() == 2)
    {
        MessageScreen::show("CHAPITRE 1 — ARRIVÉE", "story.chapter_1.arrival", StoryCampaign::buildChapterOneArrivalLines(mainPlayer), false);

        MenuScreen gate("PORTE DE LA VILLE", "story.chapter_1.gate_choice");
        gate.addSubtitle("Premier refuge, pas encore une maison");
        gate.addLine("Le garde attend une réaction. Derrière lui, la ville grince comme une charrette trop chargée.");
        gate.addOption(1, "Aller directement voir Mira", "Suivre l’indication du garde et chercher une référente utile.", true, "story.chapter_1.to_mira");
        gate.addOption(2, "Observer les murs et les portes", "Comprendre ce qui manque à la ville avant de parler.", true, "story.chapter_1.observe_walls");
        gate.addOption(3, "Demander où se trouve la guilde", "Chercher le registre des missions même s’il est presque vide.", true, "story.chapter_1.ask_guild");
        const int gateChoice = TerminalInterface::askMenuChoiceFromOptions(gate, "Choisis ton premier réflexe en ville.");
        Console::clear();

        std::vector<std::string> gateLines;
        if (gateChoice == 2)
        {
            gateLines.push_back("Tu prends le temps d’observer. Les murs ne sont pas faibles par paresse : ils ont été réparés trop souvent avec trop peu de matériaux.");
            gateLines.push_back("Une porte garde encore des marques de griffes. Une autre a été renforcée avec des plaques qui viennent probablement d’une vieille carriole.");
            gateLines.push_back("Tu comprends une chose simple : ici, chaque peau, chaque os, chaque planche rapportée peut réellement changer quelque chose.");
        }
        else if (gateChoice == 3)
        {
            gateLines.push_back("Le garde montre un bâtiment bas, coincé entre un puits et une forge à moitié fermée.");
            gateLines.push_back("« La guilde ? C’est ce registre humide là-bas. Pour les grandes promesses, il faudra repasser quand on aura assez de monde pour mentir correctement. »");
        }
        else
        {
            gateLines.push_back("Tu suis l’indication du garde. Dans une ville qui manque de tout, perdre du temps semble presque indécent.");
        }
        gateLines.push_back("Quel que soit ton détour, le chemin finit devant Mira.");
        MessageScreen::show("PREMIER REGARD", "story.chapter_1.gate_result", gateLines, false);

        MessageScreen::show("MIRA", "story.chapter_1.mira_intro", StoryCampaign::buildChapterOneMiraLines(mainPlayer), false);

        MenuScreen mira("MIRA — PREMIÈRE DEMANDE", "story.chapter_1.mira_choice");
        mira.addSubtitle("La ville ne cherche pas un héros. Elle cherche quelqu’un qui revient vivant.");
        mira.addLine("Mira pose trois marques sur le registre : portes, soins, matériaux.");
        mira.addOption(1, "Accepter d’aider la ville à tenir", "Entrer dans la première mission principale.", true, "story.chapter_1.accept_help");
        mira.addOption(2, "Demander d’abord des réponses", "Insister sur la fumée blanche et les souvenirs perdus.", true, "story.chapter_1.ask_answers");
        mira.addOption(3, "Rester prudent", "Ne pas promettre trop vite, mais écouter la mission.", true, "story.chapter_1.cautious");
        const int miraChoice = TerminalInterface::askMenuChoiceFromOptions(mira, "Réponds à Mira.");
        Console::clear();

        std::vector<std::string> reply;
        if (miraChoice == 2)
        {
            reply.push_back("Mira baisse les yeux vers le registre.");
            reply.push_back("Mira : « Les réponses sont au bout des routes. Et les routes sont fermées. Aide-nous à les rouvrir, et tu auras le droit de poser des questions qui m’effraient. »");
        }
        else if (miraChoice == 3)
        {
            reply.push_back("Mira hoche la tête, presque rassurée par ta prudence.");
            reply.push_back("Mira : « Bien. Les gens trop sûrs d’eux finissent souvent dehors, et dehors ne rend pas toujours les corps. »");
        }
        else
        {
            reply.push_back("Mira ne sourit pas vraiment, mais quelque chose dans ses épaules se détend.");
            reply.push_back("Mira : « Alors commence petit. Les petites choses sont les seules qui tiennent encore. »");
        }
        MessageScreen::show("RÉPONSE", "story.chapter_1.mira_reply", reply, false);

        mainPlayer.setStoryProgress(1, 3, std::max(1, mainPlayer.getStoryCityDevelopmentLevel()));
        saveCurrentProgress("Chapitre 1 : Mira rencontrée");
    }

    bool chapterMenuOpen = true;
    while (chapterMenuOpen)
    {
        QuestMenu::syncMainStoryQuests(mainPlayer);
        const bool chapterReady = StoryCampaign::canCompleteChapterOne(mainPlayer);
        MenuScreen chapterMenu("CHAPITRE 1 — LA VILLE QUI TIENT À PEINE", "story.chapter_1.action_menu");
        chapterMenu.addSubtitle("Mission principale : Faire respirer les murs");
        chapterMenu.addLine("Progression : " + mainPlayer.getStoryProgressLabel());
        chapterMenu.addLine(chapterReady
            ? "Mira pense que la ville peut ouvrir la route du relais."
            : "Mira garde la suite fermée tant que la chaîne principale n'est pas terminée.");
        chapterMenu.addBackOption();
        chapterMenu.addOption(1, "Voir la mission principale", "Relire les objectifs de Mira et le sens du chapitre 1.", true, "story.chapter_1.view_mission");
        chapterMenu.addOption(2, "Voir l'état de validation", "Voir précisément ce qui manque pour ouvrir le chapitre 2.", true, "story.chapter_1.progress");
        chapterMenu.addOption(3,
            "Demander à Mira ce qu'il reste à faire",
            "Mira récapitule la prochaine quête principale au lieu d'ouvrir directement les PNJ notables.",
            true,
            "story.chapter_1.ask_mira_next"
        );
        chapterMenu.addOption(4, "Retourner voir Mira", chapterReady ? "Notifier Mira que toutes les demandes principales sont terminées." : "Demander ce qui manque encore avant la suite.", true, "story.chapter_1.validate_mira");
        chapterMenu.addOption(5, "État de la ville", "Voir les boutiques, routes et systèmes encore limités par l'histoire.", true, "story.chapter_1.city_state");
        chapterMenu.addOption(6, "Quête principale / journal", "Accéder directement au vrai menu des quêtes liées à l'histoire.", true, "story.chapter_1.quests");
        chapterMenu.addOption(7, "PNJ notables", "Parler directement aux référents d'histoire sans repasser par le menu d'activité général.", true, "story.chapter_1.npcs");
        chapterMenu.addOption(8, "Accès rapides histoire", "Regrouper quêtes, PNJ, lieux, inventaire et diagnostic histoire.", true, "story.chapter_1.access");

        const int chapterChoice = TerminalInterface::askMenuChoiceFromOptions(chapterMenu, "Choisis une action du chapitre 1.");
        Console::clear();

        if (chapterChoice == 0)
        {
            return;
        }
        if (chapterChoice == 1)
        {
            std::vector<std::string> missionLines = StoryCampaign::buildChapterOneMissionLines(mainPlayer);
            missionLines.push_back("");
            missionLines.push_back("Dans les menus normaux, seuls les PNJ, quêtes, lieux et boutiques déjà présents ou rouverts sont affichés.");
            missionLines.push_back("Le chapitre 2 restera verrouillé tant que les quêtes principales de Mira et des quatre référents ne seront pas terminées puis notifiées.");
            MessageScreen::show("CHAPITRE 1 — MISSION PRINCIPALE", "story.chapter_1.main_mission", missionLines, false);
            continue;
        }
        if (chapterChoice == 2)
        {
            MessageScreen::show("VALIDATION — CHAPITRE 1", "story.chapter_1.progress", StoryCampaign::buildChapterOneProgressLines(mainPlayer), false);
            continue;
        }
        if (chapterChoice == 3)
        {
            QuestMenu::syncMainStoryQuests(mainPlayer);
            const bool fourMet = storyQuestCompleted(mainPlayer, "story_ch1_meet_referents")
                || storyQuestTurnedIn(mainPlayer, "story_ch1_meet_referents");
            const bool fourReported = storyQuestTurnedIn(mainPlayer, "story_ch1_meet_referents");

            if (!fourMet)
            {
                std::vector<std::string> lines = {
                    "Mira garde quatre lignes ouvertes dans son registre.",
                    "Parle séparément à Orren, Lysa, Bram et Soryn depuis PNJ notables > PNJ d'histoire.",
                    "Chacun te donnera immédiatement une quête principale indépendante. Tu peux commencer, terminer ou même rendre l'une d'elles avant d'avoir rencontré les trois autres.",
                    "Quand les quatre noms auront une vraie demande associée, reviens prévenir Mira."
                };
                const std::vector<std::string> progress = StoryCampaign::buildChapterOneProgressLines(mainPlayer);
                lines.insert(lines.end(), progress.begin(), progress.end());
                MessageScreen::show("MIRA — LES QUATRE RÉFÉRENTS", "story.chapter_1.mira_next.referents", lines, false);
                continue;
            }

            if (!fourReported)
            {
                MenuScreen notifyMira("NOTIFIER MIRA", "story.chapter_1.notify_mira_after_four");
                notifyMira.addSubtitle("Les quatre référents ont été rencontrés");
                notifyMira.addLine("Leurs quatre quêtes principales existent déjà. Certaines peuvent même être terminées ou rendues.");
                notifyMira.addOption(1, "Présenter le bilan à Mira", "Ouvrir la quête principale qui suit l'état réel des quatre demandes.", true, "story.chapter_1.notify_mira_after_four.confirm");
                notifyMira.addBackOption("Pas maintenant", "story.chapter_1.notify_mira_after_four.back");
                const int notifyChoice = TerminalInterface::askMenuChoiceFromOptions(notifyMira, "Choisis une réponse.");
                Console::clear();

                if (notifyChoice == 1)
                {
                    completeStoryQuestSilently(mainPlayer, "story_ch1_meet_referents");
                    mainPlayer.setStoryProgress(1, 4, std::max(1, mainPlayer.getStoryCityDevelopmentLevel()));
                    QuestMenu::syncMainStoryQuests(mainPlayer);
                    const int alreadyDone = countTurnedInChapterOneReferentQuests(mainPlayer);
                    MessageScreen::show(
                        "MIRA — FAIRE RESPIRER LES MURS",
                        "story.chapter_1.mira_bundle_opened",
                        {
                            "Mira coche les quatre noms, puis ouvre une nouvelle page du registre.",
                            "Mira : « Maintenant, termine ce qu'ils t'ont confié. Pas pour remplir une colonne. Pour que les murs aient réellement quelque chose derrière eux. »",
                            "Nouvelle quête principale : terminer puis rendre les quatre quêtes d'Orren, Lysa, Bram et Soryn.",
                            "Progression déjà reconnue : " + std::to_string(alreadyDone) + "/4.",
                            "Les quêtes validées avant ce retour sont comptées automatiquement."
                        },
                        false
                    );
                    saveCurrentProgress("Chapitre 1 : bilan des quatre référents ouvert");
                }
                continue;
            }

            std::vector<std::string> lines = {
                "Mira relit le bilan des quatre référents.",
                chapterReady
                    ? "Mira : « Les quatre voix sont là. Il ne reste plus qu'à fermer proprement cette page. »"
                    : "Mira : « Le registre se met à jour tout seul quand une demande est réellement rendue. Il en manque encore. »",
                "Quêtes de référents rendues : " + std::to_string(countTurnedInChapterOneReferentQuests(mainPlayer)) + "/4."
            };
            const std::vector<std::string> progress = StoryCampaign::buildChapterOneProgressLines(mainPlayer);
            lines.insert(lines.end(), progress.begin(), progress.end());
            MessageScreen::show("MIRA — CE QU'IL RESTE À FAIRE", "story.chapter_1.mira_next_status", lines, false);
            continue;
        }
        if (chapterChoice == 4)
        {
            if (!chapterReady)
            {
                std::vector<std::string> lines = {
                    "Mira lit le registre, puis secoue la tête.",
                    "Mira : « Pas encore. Ce n'est pas que je ne te crois pas, " + mainPlayer.getName() + ". C'est que les portes ne s'ouvrent pas avec de bonnes intentions. »",
                    "Mira : « Commence par parler aux quatre référents. Chacun te confiera sa propre demande. Reviens ensuite me prévenir, puis termine ce qu'ils t'ont donné. Le registre comptera aussi ce que tu as déjà rendu. »",
                    ""
                };
                const std::vector<std::string> progress = StoryCampaign::buildChapterOneProgressLines(mainPlayer);
                lines.insert(lines.end(), progress.begin(), progress.end());
                MessageScreen::show("MIRA — PAS ENCORE", "story.chapter_1.mira_not_ready", lines, false);
                continue;
            }

            mainPlayer.setStoryProgress(2, 1, std::max(2, mainPlayer.getStoryCityDevelopmentLevel()));
            saveCurrentProgress("Chapitre 1 terminé : relais silencieux débloqué");
            MessageScreen::show(
                "CHAPITRE 2 DÉBLOQUÉ",
                "story.chapter_1.completed",
                {
                    "Mira ferme le registre avec lenteur, comme si le bruit pouvait réveiller les murs.",
                    "Mira : « D'accord. Ce n'est pas assez pour sauver la ville, mais c'est assez pour arrêter de faire semblant qu'on peut rester ici les yeux fermés. »",
                    "Mira trace un cercle autour d'un nom presque effacé : le relais silencieux.",
                    "Mira : « Si Orren ou les autres sont encore vivants, ils savent pourquoi les routes mentent. Si personne ne répond, alors la route du nord est déjà en train de disparaître. »",
                    "Chapitre 2 débloqué : Le relais silencieux."
                },
                false
            );
            return;
        }
        if (chapterChoice == 5)
        {
            MessageScreen::show("ÉTAT DE LA VILLE", "story.chapter_1.city_state", StoryCampaign::buildDevelopmentLines(mainPlayer), false);
            continue;
        }
        if (chapterChoice == 6)
        {
            QuestMenu::consultOnly(mainPlayer);
            saveCurrentProgress("Chapitre 1 : quêtes consultées");
            continue;
        }
        if (chapterChoice == 7)
        {
            QuestMenu::openNotableNpcMenu(mainPlayer);
            saveCurrentProgress("Chapitre 1 : PNJ notables consultés");
            continue;
        }
        if (chapterChoice == 8)
        {
            openStoryAccessMenu();
            continue;
        }
    }
}

void Game::playStoryChapterTwo()
{
    if (!mainPlayer.hasStoryModeStarted() || mainPlayer.getStoryChapter() < 2)
    {
        MessageScreen::show(
            "CHAPITRE 2 IMPOSSIBLE",
            "story.chapter_2.not_unlocked",
            {
                "Le relais silencieux appartient à la suite de l'histoire.",
                "Termine d'abord les demandes principales du chapitre 1, puis notifie Mira."
            },
            false
        );
        return;
    }

    bool chapterMenuOpen = true;
    while (chapterMenuOpen)
    {
        QuestMenu::syncMainStoryQuests(mainPlayer);

        MenuScreen chapterMenu("CHAPITRE 2 — LE RELAIS SILENCIEUX", "story.chapter_2.action_menu");
        chapterMenu.addSubtitle("Mission principale : comprendre pourquoi la route ment");
        chapterMenu.addLine("Progression : " + mainPlayer.getStoryProgressLabel());
        if (mainPlayer.getStoryStep() < 2)
        {
            chapterMenu.addLine("Mira et Orren doivent d'abord poser le cadre avant toute sortie.");
        }
        else if (mainPlayer.getStoryStep() < 3)
        {
            chapterMenu.addLine("Priorité : explorer la Route commerciale et rendre la reconnaissance à Orren.");
        }
        else if (mainPlayer.getStoryStep() < 4)
        {
            chapterMenu.addLine("Priorité : obtenir une preuve assez nette pour Soryn, pas seulement une rumeur.");
        }
        else if (mainPlayer.getStoryStep() < 5)
        {
            chapterMenu.addLine("Priorité : affronter les guetteurs sans feu. Cette menace est imposée par l'histoire du relais.");
        }
        else if (mainPlayer.getStoryStep() < 6)
        {
            chapterMenu.addLine("Priorité : faire répondre le relais. Une route dégagée mais muette reste dangereuse.");
        }
        else if (mainPlayer.getStoryStep() < 7)
        {
            chapterMenu.addLine("Priorité : suivre le premier signal vivant du relais et sauver Nell la messagère.");
        }
        else if (mainPlayer.getStoryStep() < 8)
        {
            chapterMenu.addLine("Priorité : exploiter la sacoche de Nell pour transformer le sauvetage en piste exploitable.");
        }
        else if (mainPlayer.getStoryStep() < 9)
        {
            chapterMenu.addLine("Priorité : distribuer les informations de Nell aux comptoirs pour que la ville réagisse concrètement.");
        }
        else if (mainPlayer.getStoryStep() < 10)
        {
            chapterMenu.addLine("Priorité : retourner sur la Route commerciale et suivre l'encre froide jusqu'à une vraie piste.");
        }
        else if (mainPlayer.getStoryStep() < 11)
        {
            chapterMenu.addLine("Priorité : identifier ce qui réécrit les routes avec Soryn et Nell.");
        }
        else if (mainPlayer.getStoryStep() < 12)
        {
            chapterMenu.addLine("Priorité : installer le contre-registre des routes courtes pour protéger stocks et comptoirs.");
        }
        else if (mainPlayer.getStoryStep() < 13)
        {
            chapterMenu.addLine("Priorité : reconnaître le nœud noir, l'endroit où les routes corrigées semblent revenir.");
        }
        else if (mainPlayer.getStoryStep() < 14)
        {
            chapterMenu.addLine("Priorité : tenir pendant les travaux. La ville prépare portes, soins et retours avant la sortie dangereuse.");
        }
        else if (mainPlayer.getStoryStep() < 15)
        {
            chapterMenu.addLine("Priorité : confirmer ce qui garde la borne noire sans écrire son vrai nom trop tôt.");
        }
        else if (mainPlayer.getStoryStep() < 16)
        {
            chapterMenu.addLine("Priorité : briser le premier verrou de la borne noire et revenir avec une preuve.");
        }
        else if (mainPlayer.getStoryStep() < 17)
        {
            chapterMenu.addLine("Priorité : lire les cicatrices du verrou avec Soryn avant de croire la route sauvée.");
        }
        else if (mainPlayer.getStoryStep() < 18)
        {
            chapterMenu.addLine("Priorité : organiser une route gardée pour les premiers retours confirmés.");
        }
        else
        {
            chapterMenu.addLine("La route courte tient sous garde. Le vrai nom derrière la borne noire reste encore à découvrir.");
        }
        chapterMenu.addBackOption();
        chapterMenu.addOption(1, "Voir la mission principale", "Relire le plan du relais silencieux.", true, "story.chapter_2.view_mission");
        chapterMenu.addOption(2, "Voir l'état de validation", "Voir les étapes principales déjà rendues.", true, "story.chapter_2.progress");
        chapterMenu.addOption(3, "Demander à Mira et Orren ce qu'il reste à faire", "Obtenir la prochaine consigne de route.", true, "story.chapter_2.ask_next");
        const int chapterTwoStep = mainPlayer.getStoryStep();
        std::string chapterTwoActionLabel = "Lire le bilan de route";
        std::string chapterTwoActionDetail = "Lire les conséquences validées.";
        if (chapterTwoStep < 2)
        {
            chapterTwoActionLabel = "Faire le briefing du relais";
            chapterTwoActionDetail = "Réunir Mira et Orren autour de la carte.";
        }
        else if (chapterTwoStep < 4)
        {
            chapterTwoActionLabel = "Enquêter sur la Route commerciale";
            chapterTwoActionDetail = "Scènes ciblées : bornes, traces, ornières et relais silencieux.";
        }
        else if (chapterTwoStep < 5)
        {
            chapterTwoActionLabel = "Affronter la menace du relais";
            chapterTwoActionDetail = "Combat imposé par l'histoire, puis rapport à Orren.";
        }
        else if (chapterTwoStep < 6)
        {
            chapterTwoActionLabel = "Faire répondre le relais";
            chapterTwoActionDetail = "Service histoire : cloche, marque et registre du relais.";
        }
        else if (chapterTwoStep < 7)
        {
            chapterTwoActionLabel = "Sauver la voix du relais";
            chapterTwoActionDetail = "Premier sauvetage de route : convoi brisé et Nell la messagère.";
        }
        else if (chapterTwoStep < 8)
        {
            chapterTwoActionLabel = "Exploiter la sacoche de Nell";
            chapterTwoActionDetail = "Trier cartes, bons, haltes et encre froide dans la sacoche.";
        }
        else if (chapterTwoStep < 9)
        {
            chapterTwoActionLabel = "Distribuer les infos aux comptoirs";
            chapterTwoActionDetail = "Faire réagir forge, herboristerie, guilde et relais aux informations de Nell.";
        }
        else if (chapterTwoStep < 10)
        {
            chapterTwoActionLabel = "Suivre l'encre froide";
            chapterTwoActionDetail = "Deux scènes de Route commerciale liées à la sacoche de Nell.";
        }
        else if (chapterTwoStep < 11)
        {
            chapterTwoActionLabel = "Identifier la route réécrite";
            chapterTwoActionDetail = "Analyse d'archives : prouver que la route est corrigée après passage.";
        }
        else if (chapterTwoStep < 12)
        {
            chapterTwoActionLabel = "Installer le contre-registre";
            chapterTwoActionDetail = "Ville/économie : rendre les stocks moins dépendants des cartes corrompues.";
        }
        else if (chapterTwoStep < 13)
        {
            chapterTwoActionLabel = "Reconnaître le nœud noir";
            chapterTwoActionDetail = "Reconnaissance risquée avant la prochaine vraie crise.";
        }
        else if (chapterTwoStep < 14)
        {
            chapterTwoActionLabel = "Tenir pendant les travaux";
            chapterTwoActionDetail = "Patrouilles, services et quêtes utiles pendant que la ville prépare la suite.";
        }
        else if (chapterTwoStep < 15)
        {
            chapterTwoActionLabel = "Confirmer la présence";
            chapterTwoActionDetail = "Piste de menace : identifier la chose qui garde la borne sans révéler son vrai nom.";
        }
        else if (chapterTwoStep < 16)
        {
            chapterTwoActionLabel = "Briser le verrou";
            chapterTwoActionDetail = "Boss d'étape non nommé lié à la borne noire.";
        }
        else if (chapterTwoStep < 17)
        {
            chapterTwoActionLabel = "Lire les cicatrices du verrou";
            chapterTwoActionDetail = "Classer les preuves laissées par le verrou avec Soryn.";
        }
        else if (chapterTwoStep < 18)
        {
            chapterTwoActionLabel = "Organiser la route gardée";
            chapterTwoActionDetail = "Conséquence ville : retours courts, stocks surveillés et relais secondaires.";
        }
        chapterMenu.addOption(4, chapterTwoActionLabel, chapterTwoActionDetail, true, "story.chapter_2.action");
        chapterMenu.addOption(5, "Ouvrir le menu Quêtes", "Accéder à Quête principale et au journal.", true, "story.chapter_2.main_quests");
        chapterMenu.addOption(6, "PNJ notables", "Parler à Mira, Orren, Soryn ou aux autres référents actuellement présents.", true, "story.chapter_2.npcs");
        chapterMenu.addOption(7, "Accès rapides histoire", "Regrouper quêtes, PNJ, lieux, inventaire et diagnostic histoire.", true, "story.chapter_2.access");

        const int chapterChoice = TerminalInterface::askMenuChoiceFromOptions(chapterMenu, "Choisis une action du chapitre 2.");
        Console::clear();

        if (chapterChoice == 0)
        {
            return;
        }
        if (chapterChoice == 1)
        {
            MessageScreen::show("CHAPITRE 2 — MISSION PRINCIPALE", "story.chapter_2.main_mission", StoryCampaign::buildChapterTwoMissionLines(mainPlayer), false);
            continue;
        }
        if (chapterChoice == 2)
        {
            MessageScreen::show("VALIDATION — CHAPITRE 2", "story.chapter_2.progress", StoryCampaign::buildChapterTwoProgressLines(mainPlayer), false);
            continue;
        }
        if (chapterChoice == 3)
        {
            std::vector<std::string> lines;
            if (mainPlayer.getStoryStep() < 2)
            {
                lines = {
                    "Mira garde le doigt sur le nom du relais silencieux.",
                    "Mira : « Avant de courir, tu écoutes Orren. Une route qui ment tue surtout les gens pressés. »",
                    "Prochaine étape : faire le briefing du relais."
                };
            }
            else if (mainPlayer.getStoryStep() < 3)
            {
                lines = {
                    "Orren replie une carte déjà trop raturée.",
                    "Orren : « Va sur la Route commerciale. Deux sorties courtes valent mieux qu'une grande déclaration héroïque. Puis reviens me rendre la reconnaissance. »",
                    "Outil : Exploration > Route commerciale, puis PNJ notables > Orren pour rendre la demande quand elle est prête."
                };
            }
            else if (mainPlayer.getStoryStep() < 4)
            {
                lines = {
                    "Soryn refuse d'écrire le mot malédiction sans preuve.",
                    "Soryn : « Une borne retournée, une trace impossible, un témoin cohérent. Un seul élément propre vaut mieux que dix paniques. »",
                    "Outil : Quêtes > Quête principale, exploration de la Route commerciale, puis rendu auprès de Soryn."
                };
            }
            else if (mainPlayer.getStoryStep() < 5)
            {
                lines = {
                    "Mira, Orren et Soryn ont maintenant assez de preuves pour admettre que le relais ne se contente pas d'être silencieux.",
                    "Suite : les guetteurs sans feu doivent être affrontés depuis le menu histoire. Ce n'est pas un contrat libre à contourner.",
                    "Après le combat, rends la quête principale à Orren pour stabiliser le relais."
                };
            }
            else if (mainPlayer.getStoryStep() < 6)
            {
                lines = {
                    "Les guetteurs sans feu sont repoussés, mais Mira refuse de cocher le relais comme vraiment fiable.",
                    "Mira : « Une route vide ne suffit pas. Il faut qu'elle réponde aux gens qui vont revenir après toi. »",
                    "Suite : faire répondre le relais depuis le menu histoire, puis rendre la quête principale à Mira."
                };
            }
            else if (mainPlayer.getStoryStep() < 7)
            {
                lines = {
                    "La cloche du relais répond. Pour la première fois, elle ne renvoie pas seulement un signal : elle renvoie une urgence.",
                    "Orren : « Trois coups depuis le nord. Ça veut dire vivant, coincé, bientôt trop tard. »",
                    "Suite : sauver Nell la messagère depuis le menu histoire, puis rendre la quête principale auprès d'elle."
                };
            }
            else if (mainPlayer.getStoryStep() < 8)
            {
                lines = {
                    "Nell est revenue avec une sacoche de routes et assez de souffle pour parler.",
                    "Nell : « Si vous lisez seulement la carte du dessus, vous suivrez la mauvaise route. Les papiers du fond disent autre chose. »",
                    "Suite : exploiter la sacoche de Nell depuis le menu histoire, puis rendre l'analyse auprès d'elle."
                };
            }
            else if (mainPlayer.getStoryStep() < 9)
            {
                lines = {
                    "La sacoche a donné assez d'informations pour aider la ville, pas seulement l'histoire principale.",
                    "Mira : « Une route qui revient doit nourrir la forge, l'herboristerie, la guilde et le relais. Sinon elle ne sert qu'à raconter une belle frayeur. »",
                    "Suite : distribuer les informations aux comptoirs, puis notifier Mira."
                };
            }
            else if (mainPlayer.getStoryStep() < 10)
            {
                lines = {
                    "Les comptoirs respirent un peu, mais Soryn ne lâche pas la marque d'encre froide.",
                    "Soryn : « Les stocks prouvent que Nell dit vrai. L'encre, elle, prouve que quelqu'un a réécrit la route. »",
                    "Suite : suivre deux traces scénarisées sur la Route commerciale, puis rendre la preuve à Soryn."
                };
            }
            else if (mainPlayer.getStoryStep() < 11)
            {
                lines = {
                    "Soryn et Nell gardent la carte entre eux comme une bête malade.",
                    "Soryn : « Maintenant qu'on a l'encre, il faut prouver le procédé. Une route qui se corrige après coup n'est pas une route perdue : c'est une route tenue. »",
                    "Suite : identifier ce qui réécrit la route, puis rendre la preuve auprès de Soryn."
                };
            }
            else if (mainPlayer.getStoryStep() < 12)
            {
                lines = {
                    "Mira refuse de laisser les comptoirs dépendre de cartes que l'encre froide sait manipuler.",
                    "Mira : « Si la carte ment, nos stocks doivent apprendre à vérifier autrement. »",
                    "Suite : installer le contre-registre des routes courtes, puis notifier Mira."
                };
            }
            else if (mainPlayer.getStoryStep() < 13)
            {
                lines = {
                    "Orren n'aime pas la forme dessinée par les routes corrigées.",
                    "Orren : « Toutes les erreurs reviennent vers une ancienne borne noire. On n'y envoie pas un convoi. On y envoie une preuve. »",
                    "Suite : reconnaître le nœud noir, survivre à l'approche, puis rendre l'alerte auprès d'Orren."
                };
            }
            else if (mainPlayer.getStoryStep() < 14)
            {
                lines = {
                    "Le nœud noir est repéré, mais la ville ne peut pas tout réparer pendant que tu regardes le registre.",
                    "Mira : « Bram renforce, Lysa prépare, Eda vérifie, Orren balise. Toi, tu t'occupes utilement et tu reviens avec assez de gestes sûrs pour tenir la suite. »",
                    "Suite : faire avancer Tenir pendant les travaux avec patrouilles, services, quêtes secondaires ou exploration proche."
                };
            }
            else if (mainPlayer.getStoryStep() < 15)
            {
                lines = {
                    "Les réparations tiennent assez pour regarder la borne noire sans sacrifier la ville derrière toi.",
                    "Soryn : « On ne dira pas son vrai nom tant qu'on ne l'a pas proprement identifié. Mais quelque chose garde la borne. »",
                    "Suite : recouper les témoignages et préparer l'idée d'une menace majeure sans donner son vrai nom trop tôt."
                };
            }
            else if (mainPlayer.getStoryStep() < 16)
            {
                lines = {
                    "Nell est revenue, la ville a réagi, l'encre froide a été classée, le nœud noir est repéré et les réparations ont tenu.",
                    "Mira : « Maintenant on sait assez pour préparer l'affrontement. Pas assez pour donner son vrai nom. »",
                    "Suite : briser le verrou de la borne noire, puis rendre la preuve auprès d'Orren."
                };
            }
            else if (mainPlayer.getStoryStep() < 17)
            {
                lines = {
                    "Le premier verrou de la borne noire a cédé, mais Soryn refuse de le classer comme une victoire simple.",
                    "Soryn : « Ce qui cède laisse toujours une marque. On lit la marque avant de donner un nom. »",
                    "Suite : lire les cicatrices du verrou, puis rendre la preuve auprès de Soryn."
                };
            }
            else if (mainPlayer.getStoryStep() < 18)
            {
                lines = {
                    "Les cicatrices du verrou sont classées.",
                    "Mira : « Une route blessée peut encore tuer. Maintenant on organise les premiers retours, un par un. »",
                    "Suite : organiser une route gardée avec Mira, Nell, Eda, Bram et Lysa."
                };
            }
            else
            {
                lines = {
                    "La route courte tient sous garde.",
                    "Orren : « On a gagné du passage, pas la paix. C'est déjà beaucoup. »",
                    "Suite possible : préparer plus tard l'identification complète de la menace derrière les routes réécrites."
                };
            }
            MessageScreen::show("CE QU'IL RESTE À FAIRE", "story.chapter_2.next", lines, false);
            continue;
        }
        if (chapterChoice == 4)
        {
            if (mainPlayer.getStoryStep() < 2)
            {
                MessageScreen::show("BRIEFING DU RELAIS", "story.chapter_2.briefing", StoryCampaign::buildChapterTwoBriefingLines(mainPlayer), false);
                mainPlayer.setStoryProgress(2, 2, std::max(2, mainPlayer.getStoryCityDevelopmentLevel()));
                QuestMenu::syncMainStoryQuests(mainPlayer);
                saveCurrentProgress("Chapitre 2 : briefing du relais terminé");
                continue;
            }

            if (mainPlayer.getStoryStep() < 4)
            {
                QuestMenu::syncMainStoryQuests(mainPlayer);
                MenuScreen routeMenu("ROUTE COMMERCIALE — RELAIS", "story.chapter_2.route_events");
                routeMenu.addSubtitle("Enquête ciblée : bornes, ornières et silence du relais");
                routeMenu.addLine("Cette option ne remplace pas le bac à sable : elle ajoute des scènes de Route commerciale liées au relais silencieux.");
                routeMenu.addLine("Reconnaissance Orren : " + storyQuestProgressText(mainPlayer, "story_ch2_north_road_scout") + ".");
                routeMenu.addLine("Preuve Soryn : " + storyQuestProgressText(mainPlayer, "story_ch2_turned_marker") + ".");
                routeMenu.addBackOption("Retour", "story.chapter_2.route.back");
                routeMenu.addOption(1, "Marquer les bornes qui changent de côté", "Avance la reconnaissance d'Orren sur la Route commerciale.", true, "story.chapter_2.route.marker");
                routeMenu.addOption(2, "Suivre les ornières muettes", "Scène plus risquée : cherche une preuve utilisable pour Soryn.", true, "story.chapter_2.route.tracks");
                routeMenu.addOption(3, "Ouvrir l'exploration complète", "Laisser le joueur explorer normalement les biomes, dont la Route commerciale.", true, "story.chapter_2.route.full_exploration");

                const int routeChoice = TerminalInterface::askMenuChoiceFromOptions(routeMenu, "Choisis une scène de route.");
                Console::clear();

                if (routeChoice == 0)
                {
                    continue;
                }
                if (routeChoice == 1)
                {
                    std::vector<std::string> lines = {
                        "Tu plantes un bout de charbon dans la terre, puis tu avances jusqu'à la borne suivante.",
                        "Elle porte le même éclat sur le côté opposé. Pas une erreur de lecture : quelqu'un ou quelque chose retourne les repères après le passage des patrouilles.",
                        "Orren pourra exploiter cette mesure, mais il faudra lui rendre la reconnaissance auprès des PNJ notables."
                    };
                    if (progressStoryQuestById(mainPlayer, "story_ch2_north_road_scout", 1))
                    {
                        lines.push_back("Quête principale mise à jour : La route qui s'allonge — " + storyQuestProgressText(mainPlayer, "story_ch2_north_road_scout") + ".");
                    }
                    else
                    {
                        lines.push_back("Aucune progression directe : la quête est peut-être déjà prête à rendre ou validée.");
                    }
                    MessageScreen::show("BORNES QUI MENTENT", "story.chapter_2.route.marker_result", lines, false);
                    saveCurrentProgress("Chapitre 2 : bornes du relais inspectées");
                    continue;
                }
                if (routeChoice == 2)
                {
                    std::vector<std::string> lines = {
                        "Les ornières d'une charrette s'arrêtent dans la boue sans trace de demi-tour.",
                        "Plus loin, le même dessin réapparaît comme si la route avait recollé deux morceaux de trajet qui ne devraient pas se toucher.",
                        "Tu récupères une preuve propre : pas une panique, pas un témoignage, une contradiction physique."
                    };

                    bool updated = false;
                    updated = progressStoryQuestById(mainPlayer, "story_ch2_north_road_scout", 1) || updated;
                    if (storyQuestTurnedIn(mainPlayer, "story_ch2_north_road_scout") || mainPlayer.getStoryStep() >= 3)
                    {
                        updated = progressStoryQuestById(mainPlayer, "story_ch2_turned_marker", 1) || updated;
                    }

                    lines.push_back("Reconnaissance Orren : " + storyQuestProgressText(mainPlayer, "story_ch2_north_road_scout") + ".");
                    lines.push_back("Preuve Soryn : " + storyQuestProgressText(mainPlayer, "story_ch2_turned_marker") + ".");
                    if (!updated)
                    {
                        lines.push_back("Aucune progression directe : pense à rendre les étapes prêtes auprès d'Orren ou de Soryn.");
                    }
                    MessageScreen::show("ORNIÈRES MUETTES", "story.chapter_2.route.tracks_result", lines, false);
                    saveCurrentProgress("Chapitre 2 : preuve de route relevée");
                    continue;
                }

                QuestMenu::openExploration(mainPlayer, selectedDifficulty, selectedDeathRule);
                QuestMenu::syncMainStoryQuests(mainPlayer);
                saveCurrentProgress("Chapitre 2 : exploration libre depuis la route du relais");
                continue;
            }

            QuestMenu::syncMainStoryQuests(mainPlayer);
            if (mainPlayer.getStoryStep() < 6 && storyQuestTurnedIn(mainPlayer, "story_ch2_relay_signal"))
            {
                mainPlayer.setStoryProgress(2, 6, std::max(3, mainPlayer.getStoryCityDevelopmentLevel()));
                QuestMenu::syncMainStoryQuests(mainPlayer);
            }
            if (mainPlayer.getStoryStep() < 7 && storyQuestTurnedIn(mainPlayer, "story_ch2_first_rescue"))
            {
                mainPlayer.setStoryProgress(2, 7, std::max(4, mainPlayer.getStoryCityDevelopmentLevel()));
                QuestMenu::syncMainStoryQuests(mainPlayer);
            }
            if (mainPlayer.getStoryStep() < 8 && storyQuestTurnedIn(mainPlayer, "story_ch2_route_sack"))
            {
                mainPlayer.setStoryProgress(2, 8, std::max(4, mainPlayer.getStoryCityDevelopmentLevel()));
                QuestMenu::syncMainStoryQuests(mainPlayer);
            }
            if (mainPlayer.getStoryStep() < 9 && storyQuestTurnedIn(mainPlayer, "story_ch2_city_recovery"))
            {
                mainPlayer.setStoryProgress(2, 9, std::max(5, mainPlayer.getStoryCityDevelopmentLevel()));
                QuestMenu::syncMainStoryQuests(mainPlayer);
            }
            if (mainPlayer.getStoryStep() < 10 && storyQuestTurnedIn(mainPlayer, "story_ch2_cold_ink_trail"))
            {
                mainPlayer.setStoryProgress(2, 10, std::max(5, mainPlayer.getStoryCityDevelopmentLevel()));
                QuestMenu::syncMainStoryQuests(mainPlayer);
            }
            if (mainPlayer.getStoryStep() < 11 && storyQuestTurnedIn(mainPlayer, "story_ch2_route_rewrite"))
            {
                mainPlayer.setStoryProgress(2, 11, std::max(5, mainPlayer.getStoryCityDevelopmentLevel()));
                QuestMenu::syncMainStoryQuests(mainPlayer);
            }
            if (mainPlayer.getStoryStep() < 12 && storyQuestTurnedIn(mainPlayer, "story_ch2_short_route_counter"))
            {
                mainPlayer.setStoryProgress(2, 12, std::max(6, mainPlayer.getStoryCityDevelopmentLevel()));
                QuestMenu::syncMainStoryQuests(mainPlayer);
            }
            if (mainPlayer.getStoryStep() < 13 && storyQuestTurnedIn(mainPlayer, "story_ch2_black_knot_warning"))
            {
                mainPlayer.setStoryProgress(2, 13, std::max(6, mainPlayer.getStoryCityDevelopmentLevel()));
                QuestMenu::syncMainStoryQuests(mainPlayer);
            }
            if (mainPlayer.getStoryStep() < 14 && storyQuestTurnedIn(mainPlayer, "story_ch2_repair_downtime"))
            {
                mainPlayer.setStoryProgress(2, 14, std::max(6, mainPlayer.getStoryCityDevelopmentLevel()));
                QuestMenu::syncMainStoryQuests(mainPlayer);
            }
            if (mainPlayer.getStoryStep() < 15 && storyQuestTurnedIn(mainPlayer, "story_ch2_hidden_guardian_hint"))
            {
                mainPlayer.setStoryProgress(2, 15, std::max(7, mainPlayer.getStoryCityDevelopmentLevel()));
                QuestMenu::syncMainStoryQuests(mainPlayer);
            }
            if (mainPlayer.getStoryStep() < 16 && storyQuestTurnedIn(mainPlayer, "story_ch2_black_knot_seal"))
            {
                mainPlayer.setStoryProgress(2, 16, std::max(8, mainPlayer.getStoryCityDevelopmentLevel()));
                QuestMenu::syncMainStoryQuests(mainPlayer);
            }
            if (mainPlayer.getStoryStep() < 17 && storyQuestTurnedIn(mainPlayer, "story_ch2_black_knot_scars"))
            {
                mainPlayer.setStoryProgress(2, 17, std::max(8, mainPlayer.getStoryCityDevelopmentLevel()));
                QuestMenu::syncMainStoryQuests(mainPlayer);
            }
            if (mainPlayer.getStoryStep() < 18 && storyQuestTurnedIn(mainPlayer, "story_ch2_guarded_route"))
            {
                mainPlayer.setStoryProgress(2, 18, std::max(9, mainPlayer.getStoryCityDevelopmentLevel()));
                QuestMenu::syncMainStoryQuests(mainPlayer);
            }

            if (mainPlayer.getStoryStep() >= 18 || storyQuestTurnedIn(mainPlayer, "story_ch2_guarded_route"))
            {
                mainPlayer.setStoryProgress(2, 18, std::max(9, mainPlayer.getStoryCityDevelopmentLevel()));
                MessageScreen::show(
                    "ROUTE COURTE SOUS GARDE",
                    "story.chapter_2.guarded_route.done",
                    {
                        "La ville ne crie pas victoire. Elle compte les retours.",
                        "Le verrou de la borne noire a laissé des cicatrices, la route accepte de rendre quelques vivants, et les comptoirs savent enfin distinguer une carte rassurante d'une preuve fiable.",
                        "Mira classe la crise comme contenue, pas résolue. Le vrai nom de ce qui tord les routes reste absent du registre."
                    },
                    false
                );
                saveCurrentProgress("Chapitre 2 : route gardée stabilisée");
                continue;
            }

            if (mainPlayer.getStoryStep() >= 17)
            {
                QuestMenu::syncMainStoryQuests(mainPlayer);
                if (storyQuestCompleted(mainPlayer, "story_ch2_guarded_route"))
                {
                    MessageScreen::show(
                        "ROUTE À NOTIFIER",
                        "story.chapter_2.guarded_route.ready",
                        {
                            "Les premiers retours gardés sont organisés.",
                            "Il reste à rendre la quête principale auprès de Mira dans PNJ notables ou Quêtes > Rendre une quête prête.",
                            "Après ce rapport, la ville pourra respirer un peu mieux sans croire que le problème est terminé."
                        },
                        false
                    );
                    continue;
                }

                std::vector<std::string> lines = StoryCampaign::buildChapterTwoGuardedRouteLines(mainPlayer);
                const bool progressed = progressStoryQuestById(mainPlayer, "story_ch2_guarded_route", 1);
                lines.push_back(progressed
                    ? "Quête principale prête : Une route à garder ouverte."
                    : "La quête était peut-être déjà prête à rendre auprès de Mira.");
                MessageScreen::show("UNE ROUTE À GARDER OUVERTE", "story.chapter_2.guarded_route", lines, false);
                saveCurrentProgress("Chapitre 2 : route gardée organisée");
                continue;
            }

            if (mainPlayer.getStoryStep() >= 16)
            {
                QuestMenu::syncMainStoryQuests(mainPlayer);
                if (storyQuestCompleted(mainPlayer, "story_ch2_black_knot_scars"))
                {
                    MessageScreen::show(
                        "CICATRICES À CLASSER",
                        "story.chapter_2.black_knot_scars.ready",
                        {
                            "Les marques du verrou sont assez nettes pour être classées.",
                            "Il reste à rendre la quête principale auprès de Soryn dans PNJ notables ou Quêtes > Rendre une quête prête.",
                            "Après ce rapport, la ville pourra organiser des retours gardés au lieu de juste raconter un combat gagné."
                        },
                        false
                    );
                    continue;
                }

                std::vector<std::string> lines = StoryCampaign::buildChapterTwoBlackKnotScarsLines(mainPlayer);
                const bool progressed = progressStoryQuestById(mainPlayer, "story_ch2_black_knot_scars", 1);
                lines.push_back(progressed
                    ? "Quête principale prête : Les cicatrices du verrou."
                    : "La quête était peut-être déjà prête à rendre auprès de Soryn.");
                MessageScreen::show("LES CICATRICES DU VERROU", "story.chapter_2.black_knot_scars", lines, false);
                saveCurrentProgress("Chapitre 2 : cicatrices du verrou relevées");
                continue;
            }

            if (mainPlayer.getStoryStep() >= 15)
            {
                QuestMenu::syncMainStoryQuests(mainPlayer);
                if (storyQuestCompleted(mainPlayer, "story_ch2_black_knot_seal"))
                {
                    MessageScreen::show(
                        "VERROU À RENDRE",
                        "story.chapter_2.black_knot_seal.ready",
                        {
                            "Le verrou de la borne a cédé pendant l'affrontement.",
                            "Il reste à rendre la quête principale auprès d'Orren dans PNJ notables ou Quêtes > Rendre une quête prête.",
                            "Après ce rapport, la ville saura que la menace n'est plus seulement évitée : elle a déjà reculé une fois."
                        },
                        false
                    );
                    continue;
                }

                MessageScreen::show(
                    "LE VERROU DE LA BORNE",
                    "story.chapter_2.black_knot_seal.intro",
                    StoryCampaign::buildChapterTwoBlackKnotSealLines(mainPlayer),
                    false
                );

                Combat combat;
                mainPlayer.recordCombatStarted();
                ShopTransactionSystem::clearBuybackAfterCombat();
                Console::useCombatTheme();
                combat.launchMonsterPve(mainPlayer, selectedDifficulty, selectedDeathRule);
                Console::useNormalTheme();
                ShopRotationSystem::markShopsDirtyAfterCombat();

                if (!mainPlayer.isDead())
                {
                    const bool progressed = progressStoryQuestById(mainPlayer, "story_ch2_black_knot_seal", 1);
                    MessageScreen::show(
                        "PREMIER VERROU REPOUSSÉ",
                        "story.chapter_2.black_knot_seal.complete",
                        {
                            "La borne noire ne livre pas son vrai nom. Elle livre une réaction : la route s'est contractée autour de toi, puis a lâché d'un coup sec.",
                            "Orren devra recevoir cette preuve avant que la ville décide jusqu'où poursuivre l'enquête.",
                            progressed ? "Quête principale prête : Le verrou de la borne." : "La quête était peut-être déjà prête à rendre auprès d'Orren."
                        },
                        false
                    );
                    saveCurrentProgress("Chapitre 2 : verrou de la borne affronté");
                }
                else
                {
                    saveCurrentProgress("Chapitre 2 : échec face au verrou de la borne");
                }
                continue;
            }

            if (mainPlayer.getStoryStep() >= 14)
            {
                QuestMenu::syncMainStoryQuests(mainPlayer);
                if (storyQuestCompleted(mainPlayer, "story_ch2_hidden_guardian_hint"))
                {
                    MessageScreen::show(
                        "PISTE À RENDRE",
                        "story.chapter_2.hidden_guardian.ready",
                        {
                            "Les témoignages et retours de stock indiquent assez clairement qu'une présence garde la borne noire.",
                            "Il reste à rendre la quête principale auprès de Soryn dans PNJ notables ou Quêtes > Rendre une quête prête.",
                            "Après ce rapport, la ville saura préparer un affrontement sérieux sans nommer trop vite ce qui garde la borne."
                        },
                        false
                    );
                    continue;
                }

                std::vector<std::string> lines = StoryCampaign::buildChapterTwoHiddenGuardianHintLines(mainPlayer);
                const bool progressed = progressStoryQuestById(mainPlayer, "story_ch2_hidden_guardian_hint", 1);
                lines.push_back(progressed
                    ? "Quête principale mise à jour : La chose qui garde la borne — " + storyQuestProgressText(mainPlayer, "story_ch2_hidden_guardian_hint") + "."
                    : "Aucune progression directe : la quête est peut-être déjà prête à rendre auprès de Soryn.");
                lines.push_back("Retourne voir Soryn pour classer la menace sans écrire son vrai nom trop tôt.");
                MessageScreen::show("LA CHOSE QUI GARDE LA BORNE", "story.chapter_2.hidden_guardian", lines, false);
                saveCurrentProgress("Chapitre 2 : menace de la borne recoupée");
                continue;
            }

            if (mainPlayer.getStoryStep() >= 13)
            {
                QuestMenu::syncMainStoryQuests(mainPlayer);
                if (storyQuestCompleted(mainPlayer, "story_ch2_repair_downtime"))
                {
                    MessageScreen::show(
                        "TRAVAUX À NOTIFIER",
                        "story.chapter_2.repair_downtime.ready",
                        {
                            "Les trois actions utiles pendant les réparations sont terminées.",
                            "Il reste à rendre la quête principale auprès d'Eda dans PNJ notables ou Quêtes > Rendre une quête prête.",
                            "Après ce rapport, la ville acceptera de regarder ce qui garde vraiment la borne noire."
                        },
                        false
                    );
                    continue;
                }

                MenuScreen repairMenu("PENDANT LES RÉPARATIONS", "story.chapter_2.repair_downtime_menu");
                repairMenu.addSubtitle("Quête principale : Tenir pendant les travaux");
                repairMenu.addLine("Progression : " + storyQuestProgressText(mainPlayer, "story_ch2_repair_downtime") + ".");
                repairMenu.addLine("Mira ne demande pas d'attendre par confort : Bram, Lysa, Orren, Nell et Eda ont besoin que les portes, les trousses et les registres tiennent avant la prochaine sortie.");
                repairMenu.addBackOption("Retour", "story.chapter_2.repair.back");
                repairMenu.addOption(1, "Faire une patrouille courte", "Combat ou nettoyage proche pendant que les réparations avancent.", true, "story.chapter_2.repair.patrol");
                repairMenu.addOption(2, "Aider les comptoirs", "Service de ville : stocks, registre, herboristerie ou forge.", true, "story.chapter_2.repair.counter");
                repairMenu.addOption(3, "Ouvrir les quêtes secondaires", "Aller au menu Quêtes pour prendre ou rendre des activités utiles.", true, "story.chapter_2.repair.side_quests");

                const int repairChoice = TerminalInterface::askMenuChoiceFromOptions(repairMenu, "Choisis comment t'occuper pendant les réparations.");
                Console::clear();

                if (repairChoice == 0)
                {
                    continue;
                }
                if (repairChoice == 3)
                {
                    QuestMenu::openQuestHub(mainPlayer);
                    QuestMenu::syncMainStoryQuests(mainPlayer);
                    saveCurrentProgress("Chapitre 2 : passage par les quêtes secondaires pendant les réparations");
                    continue;
                }

                if (repairChoice == 1)
                {
                    MessageScreen::show(
                        "PATROUILLE DE RÉPARATION",
                        "story.chapter_2.repair.patrol_intro",
                        StoryCampaign::buildChapterTwoRepairDowntimeLines(mainPlayer),
                        false
                    );

                    Combat combat;
                    mainPlayer.recordCombatStarted();
                    ShopTransactionSystem::clearBuybackAfterCombat();
                    Console::useCombatTheme();
                    combat.launchMonsterPve(mainPlayer, selectedDifficulty, selectedDeathRule);
                    Console::useNormalTheme();
                    ShopRotationSystem::markShopsDirtyAfterCombat();

                    if (!mainPlayer.isDead())
                    {
                        const bool progressed = progressStoryQuestById(mainPlayer, "story_ch2_repair_downtime", 1);
                        MessageScreen::show(
                            "PATROUILLE UTILE",
                            "story.chapter_2.repair.patrol_done",
                            {
                                "La patrouille n'a pas réglé le nœud noir, mais elle a laissé Bram, Lysa et Eda finir une partie de leurs préparatifs.",
                                progressed ? "Quête principale mise à jour : Tenir pendant les travaux — " + storyQuestProgressText(mainPlayer, "story_ch2_repair_downtime") + "." : "Aucune progression directe : la quête est peut-être déjà prête à rendre auprès d'Eda."
                            },
                            false
                        );
                    }
                    saveCurrentProgress("Chapitre 2 : patrouille pendant les réparations");
                    continue;
                }

                if (repairChoice == 2)
                {
                    std::vector<std::string> lines = StoryCampaign::buildChapterTwoRepairDowntimeLines(mainPlayer);
                    const bool progressed = progressStoryQuestById(mainPlayer, "story_ch2_repair_downtime", 1);
                    lines.push_back("Tu aides Eda à comparer les stocks revenus, Bram à trier les ferrures et Lysa à préparer les trousses courtes.");
                    lines.push_back(progressed
                        ? "Quête principale mise à jour : Tenir pendant les travaux — " + storyQuestProgressText(mainPlayer, "story_ch2_repair_downtime") + "."
                        : "Aucune progression directe : la quête est peut-être déjà prête à rendre auprès d'Eda.");
                    MessageScreen::show("SERVICE DE COMPTOIR", "story.chapter_2.repair.counter_done", lines, false);
                    saveCurrentProgress("Chapitre 2 : service de comptoir pendant les réparations");
                    continue;
                }
            }

            if (mainPlayer.getStoryStep() >= 12)
            {
                QuestMenu::syncMainStoryQuests(mainPlayer);
                if (storyQuestCompleted(mainPlayer, "story_ch2_black_knot_warning"))
                {
                    MessageScreen::show(
                        "ALERTE À RENDRE",
                        "story.chapter_2.black_knot.ready",
                        {
                            "La reconnaissance du nœud noir est faite.",
                            "Il reste à rendre la quête principale auprès d'Orren dans PNJ notables ou Quêtes > Rendre une quête prête.",
                            "Après ce rapport, le chapitre 2 aura une vraie crise suivante à ouvrir."
                        },
                        false
                    );
                    continue;
                }

                MessageScreen::show(
                    "LE NŒUD NOIR AU BOUT DU RELAIS",
                    "story.chapter_2.black_knot_intro",
                    StoryCampaign::buildChapterTwoBlackKnotWarningLines(mainPlayer),
                    false
                );

                {
                    Combat combat;
                    mainPlayer.recordCombatStarted();
                    ShopTransactionSystem::clearBuybackAfterCombat();
                    Console::useCombatTheme();
                    combat.launchMonsterPve(mainPlayer, selectedDifficulty, selectedDeathRule);
                    Console::useNormalTheme();
                    ShopRotationSystem::markShopsDirtyAfterCombat();
                }

                if (!mainPlayer.isDead())
                {
                    mainPlayer.getQuestLog().completeQuest("story_ch2_black_knot_warning");
                    MessageScreen::show(
                        "BORNE NOIRE LOCALISÉE",
                        "story.chapter_2.black_knot.complete",
                        {
                            "Tu ne détruis pas encore le nœud noir. Tu prouves qu'il existe, qu'il surveille l'approche, et qu'une vraie crise peut partir de là.",
                            "Orren devra recevoir ce rapport avant que la ville décide quoi risquer.",
                            "Quête principale prête : Le nœud noir au bout du relais."
                        },
                        false
                    );
                    saveCurrentProgress("Chapitre 2 : nœud noir reconnu");
                }
                else
                {
                    saveCurrentProgress("Chapitre 2 : échec de reconnaissance du nœud noir");
                }
                continue;
            }

            if (mainPlayer.getStoryStep() >= 11)
            {
                QuestMenu::syncMainStoryQuests(mainPlayer);
                if (storyQuestCompleted(mainPlayer, "story_ch2_short_route_counter"))
                {
                    MessageScreen::show(
                        "CONTRE-REGISTRE À RENDRE",
                        "story.chapter_2.counter.ready",
                        {
                            "Le contre-registre des routes courtes est installé.",
                            "Il reste à rendre la quête principale auprès de Mira dans PNJ notables ou Quêtes > Rendre une quête prête.",
                            "Après ce rapport, les boutiques auront une justification plus solide pour leurs stocks de route."
                        },
                        false
                    );
                    continue;
                }

                std::vector<std::string> lines = StoryCampaign::buildChapterTwoShortRouteCounterLines(mainPlayer);
                const bool progressed = progressStoryQuestById(mainPlayer, "story_ch2_short_route_counter", 1);
                lines.push_back(progressed
                    ? "Quête principale mise à jour : Le contre-registre des routes courtes — " + storyQuestProgressText(mainPlayer, "story_ch2_short_route_counter") + "."
                    : "Aucune progression directe : la quête est peut-être déjà prête à rendre auprès de Mira.");
                lines.push_back("Effet visible : la ville distingue mieux les stocks réellement revenus des promesses écrites sur cartes corrompues.");
                MessageScreen::show("CONTRE-REGISTRE", "story.chapter_2.short_route_counter", lines, false);
                saveCurrentProgress("Chapitre 2 : contre-registre des routes courtes installé");
                continue;
            }

            if (mainPlayer.getStoryStep() >= 10)
            {
                QuestMenu::syncMainStoryQuests(mainPlayer);
                if (storyQuestCompleted(mainPlayer, "story_ch2_route_rewrite"))
                {
                    MessageScreen::show(
                        "CARTE À RENDRE",
                        "story.chapter_2.route_rewrite.ready",
                        {
                            "Soryn et Nell ont assez d'éléments pour classer la route réécrite.",
                            "Il reste à rendre la quête principale auprès de Soryn dans PNJ notables ou Quêtes > Rendre une quête prête.",
                            "Après ce rapport, Mira pourra protéger les comptoirs contre les cartes corrompues."
                        },
                        false
                    );
                    continue;
                }

                std::vector<std::string> lines = StoryCampaign::buildChapterTwoRouteRewriteLines(mainPlayer);
                const bool progressed = progressStoryQuestById(mainPlayer, "story_ch2_route_rewrite", 1);
                lines.push_back(progressed
                    ? "Quête principale mise à jour : La carte qui se réécrit — " + storyQuestProgressText(mainPlayer, "story_ch2_route_rewrite") + "."
                    : "Aucune progression directe : la quête est peut-être déjà prête à rendre auprès de Soryn.");
                lines.push_back("Retourne voir Soryn pour classer officiellement la preuve de route réécrite.");
                MessageScreen::show("LA CARTE QUI SE RÉÉCRIT", "story.chapter_2.route_rewrite", lines, false);
                saveCurrentProgress("Chapitre 2 : route réécrite identifiée");
                continue;
            }

            if (mainPlayer.getStoryStep() >= 9)
            {
                QuestMenu::syncMainStoryQuests(mainPlayer);
                if (storyQuestCompleted(mainPlayer, "story_ch2_cold_ink_trail"))
                {
                    MessageScreen::show(
                        "PREUVE À RENDRE",
                        "story.chapter_2.cold_ink.ready",
                        {
                            "Les deux traces de l'encre froide sont relevées.",
                            "Il reste à rendre la quête principale auprès de Soryn dans PNJ notables ou Quêtes > Rendre une quête prête.",
                            "Après ce rapport, le chapitre 2 aura une vraie piste pour la crise suivante."
                        },
                        false
                    );
                    continue;
                }

                MenuScreen coldInkMenu("ROUTE COMMERCIALE — ENCRE FROIDE", "story.chapter_2.cold_ink_menu");
                coldInkMenu.addSubtitle("Quête principale : L'encre froide de la route");
                coldInkMenu.addLine("La sacoche de Nell a indiqué deux traces à vérifier avant de choisir la prochaine crise.");
                coldInkMenu.addLine("Progression : " + storyQuestProgressText(mainPlayer, "story_ch2_cold_ink_trail") + ".");
                coldInkMenu.addBackOption("Retour", "story.chapter_2.cold_ink.back");
                coldInkMenu.addOption(1, "Suivre la halte rayée", "Scène de Route commerciale : le nom effacé revient sur le terrain.", true, "story.chapter_2.cold_ink.halt");
                coldInkMenu.addOption(2, "Mesurer la boucle du pont court", "Scène de Route commerciale : la route revient vers le relais au lieu de traverser.", true, "story.chapter_2.cold_ink.loop");
                coldInkMenu.addOption(3, "Ouvrir l'exploration complète", "Explorer librement sans perdre la piste principale.", true, "story.chapter_2.cold_ink.full_exploration");
                const int coldInkChoice = TerminalInterface::askMenuChoiceFromOptions(coldInkMenu, "Choisis une trace de route.");
                Console::clear();

                if (coldInkChoice == 0)
                {
                    continue;
                }
                if (coldInkChoice == 3)
                {
                    QuestMenu::openExploration(mainPlayer, selectedDifficulty, selectedDeathRule);
                    QuestMenu::syncMainStoryQuests(mainPlayer);
                    saveCurrentProgress("Chapitre 2 : exploration libre depuis l'encre froide");
                    continue;
                }

                std::vector<std::string> lines = StoryCampaign::buildChapterTwoColdInkTrailLines(mainPlayer);
                if (coldInkChoice == 1)
                {
                    lines.push_back("Halte rayée : le panneau existe encore, mais son nom a été gratté avec une précision presque administrative.");
                    lines.push_back("Nell reconnaît un bon de convoi : cette halte devait recevoir des plantes, du cuir et deux caisses de clous de forge.");
                }
                else
                {
                    lines.push_back("Boucle du pont court : Orren compte les pas deux fois. Le trajet revient vers le relais sans avoir tourné.");
                    lines.push_back("Soryn note que la route ne ment pas comme une illusion : elle ment comme un registre corrigé après coup.");
                }
                const bool progressed = progressStoryQuestById(mainPlayer, "story_ch2_cold_ink_trail", 1);
                lines.push_back(progressed
                    ? "Quête principale mise à jour : L'encre froide de la route — " + storyQuestProgressText(mainPlayer, "story_ch2_cold_ink_trail") + "."
                    : "Aucune progression directe : la quête est peut-être déjà prête à rendre auprès de Soryn.");
                if (storyQuestCompleted(mainPlayer, "story_ch2_cold_ink_trail"))
                {
                    lines.push_back("Les deux traces sont suffisantes. Retourne voir Soryn pour classer la preuve.");
                }
                MessageScreen::show("L'ENCRE FROIDE", "story.chapter_2.cold_ink.result", lines, false);
                saveCurrentProgress("Chapitre 2 : trace d'encre froide suivie");
                continue;
            }

            if (mainPlayer.getStoryStep() >= 8)
            {
                QuestMenu::syncMainStoryQuests(mainPlayer);
                if (storyQuestCompleted(mainPlayer, "story_ch2_city_recovery"))
                {
                    MessageScreen::show(
                        "VILLE À NOTIFIER",
                        "story.chapter_2.city_recovery.ready",
                        {
                            "Les comptoirs ont reçu les informations de Nell.",
                            "Il reste à rendre la quête principale auprès de Mira dans PNJ notables ou Quêtes > Rendre une quête prête.",
                            "Après ce rapport, la ville considérera les premiers stocks de route comme justifiés."
                        },
                        false
                    );
                    continue;
                }

                std::vector<std::string> lines = StoryCampaign::buildChapterTwoCityRecoveryLines(mainPlayer);
                const bool progressed = progressStoryQuestById(mainPlayer, "story_ch2_city_recovery", 1);
                lines.push_back(progressed
                    ? "Quête principale mise à jour : Les comptoirs rouvrent un œil — " + storyQuestProgressText(mainPlayer, "story_ch2_city_recovery") + "."
                    : "Aucune progression directe : la quête est peut-être déjà prête à rendre auprès de Mira.");
                lines.push_back("Effet visible : les menus de boutique et PNJ peuvent maintenant afficher un palier de ville plus crédible autour des stocks, routes courtes et demandes locales.");
                MessageScreen::show("LA VILLE RÉAGIT", "story.chapter_2.city_recovery", lines, false);
                saveCurrentProgress("Chapitre 2 : informations de Nell distribuées aux comptoirs");
                continue;
            }

            if (mainPlayer.getStoryStep() >= 7)
            {
                QuestMenu::syncMainStoryQuests(mainPlayer);
                if (storyQuestCompleted(mainPlayer, "story_ch2_route_sack"))
                {
                    MessageScreen::show(
                        "SACOCHE À RENDRE",
                        "story.chapter_2.route_sack.ready",
                        {
                            "La sacoche de Nell a été triée : cartes, bons de convoi, halte rayée et marque d'encre froide.",
                            "Il reste à rendre la quête principale auprès de Nell dans PNJ notables ou Quêtes > Rendre une quête prête.",
                            "Après ce rapport, Mira pourra utiliser ces informations pour faire réagir la ville."
                        },
                        false
                    );
                    continue;
                }

                std::vector<std::string> lines = StoryCampaign::buildChapterTwoRouteSackLines(mainPlayer);
                const bool progressed = progressStoryQuestById(mainPlayer, "story_ch2_route_sack", 1);
                lines.push_back(progressed
                    ? "Quête principale mise à jour : La sacoche qui parle — " + storyQuestProgressText(mainPlayer, "story_ch2_route_sack") + "."
                    : "Aucune progression directe : la quête est peut-être déjà prête à rendre auprès de Nell.");
                lines.push_back("Retourne voir Nell pour confirmer ce que la sacoche raconte vraiment.");
                MessageScreen::show("LA SACOCHE QUI PARLE", "story.chapter_2.route_sack", lines, false);
                saveCurrentProgress("Chapitre 2 : sacoche de routes exploitée");
                continue;
            }

            if (mainPlayer.getStoryStep() >= 6)
            {
                QuestMenu::syncMainStoryQuests(mainPlayer);

                if (storyQuestCompleted(mainPlayer, "story_ch2_first_rescue"))
                {
                    MessageScreen::show(
                        "NELL À RAMENER AU REGISTRE",
                        "story.chapter_2.first_rescue.ready",
                        {
                            "Nell la messagère a été sortie du convoi brisé.",
                            "Il reste à rendre la quête principale auprès d'elle dans PNJ notables ou Quêtes > Rendre une quête prête.",
                            "Après ce rapport, la route ne sera plus seulement stabilisée : elle aura ramené une voix vivante."
                        },
                        false
                    );
                    continue;
                }

                MenuScreen rescueMenu("PREMIER APPEL DU RELAIS", "story.chapter_2.first_rescue_menu");
                rescueMenu.addSubtitle("Quête principale : La voix derrière les caisses");
                rescueMenu.addLine("Le signal du relais a répondu par trois coups venus du nord : vivant, coincé, bientôt trop tard.");
                rescueMenu.addLine("Cette scène ouvre la première route plus longue sans quitter le chapitre 2.");
                rescueMenu.addBackOption("Retour", "story.chapter_2.first_rescue.back");
                rescueMenu.addOption(1, "Suivre le signal jusqu'au convoi brisé", "Déclencher le sauvetage histoire de Nell la messagère.", true, "story.chapter_2.first_rescue.start");
                const int rescueChoice = TerminalInterface::askMenuChoiceFromOptions(rescueMenu, "Choisis l'action du relais.");
                Console::clear();

                if (rescueChoice != 1)
                {
                    continue;
                }

                MessageScreen::show(
                    "LA VOIX DERRIÈRE LES CAISSES",
                    "story.chapter_2.first_rescue_intro",
                    StoryCampaign::buildChapterTwoFirstRescueLines(mainPlayer),
                    false
                );

                {
                    Combat combat;
                    mainPlayer.recordCombatStarted();
                    ShopTransactionSystem::clearBuybackAfterCombat();
                    Console::useCombatTheme();
                    combat.launchMonsterPve(mainPlayer, selectedDifficulty, selectedDeathRule);
                    Console::useNormalTheme();
                    ShopRotationSystem::markShopsDirtyAfterCombat();
                }

                if (!mainPlayer.isDead())
                {
                    mainPlayer.getQuestLog().completeQuest("story_ch2_first_rescue");
                    MessageScreen::show(
                        "NELL RESPIRE ENCORE",
                        "story.chapter_2.first_rescue_complete",
                        {
                            "Le convoi ne repartira pas aujourd'hui, mais Nell sort de derrière les caisses avec assez de souffle pour serrer sa sacoche contre elle.",
                            "Nell : « Le relais a sonné... alors j'ai répondu. Je pensais que personne ne comprendrait. »",
                            "Quête principale prête : La voix derrière les caisses.",
                            "Retourne voir Nell dans PNJ notables pour rendre le rapport et ouvrir la suite."
                        },
                        false
                    );
                    saveCurrentProgress("Chapitre 2 : Nell la messagère sauvée");
                }
                else
                {
                    saveCurrentProgress("Chapitre 2 : échec du sauvetage de Nell");
                }
                continue;
            }

            if (mainPlayer.getStoryStep() >= 5 || storyQuestTurnedIn(mainPlayer, "story_ch2_relay_threat"))
            {
                if (mainPlayer.getStoryStep() < 5)
                {
                    mainPlayer.setStoryProgress(2, 5, std::max(3, mainPlayer.getStoryCityDevelopmentLevel()));
                    QuestMenu::syncMainStoryQuests(mainPlayer);
                }

                if (storyQuestCompleted(mainPlayer, "story_ch2_relay_signal"))
                {
                    MessageScreen::show(
                        "SIGNAL À RENDRE",
                        "story.chapter_2.signal.ready",
                        {
                            "La cloche basse répond. Le registre porte une marque propre. La route possède enfin un signe que les prochaines patrouilles pourront comprendre.",
                            "Il reste à rendre la quête principale auprès de Mira dans PNJ notables ou Quêtes > Rendre une quête prête.",
                            "Après ce rapport, le relais sera considéré comme stabilisé pour cette étape."
                        },
                        false
                    );
                    continue;
                }

                std::vector<std::string> signalLines = StoryCampaign::buildChapterTwoRelaySignalLines(mainPlayer);
                const bool progressed = progressStoryQuestById(mainPlayer, "story_ch2_relay_signal", 1);
                signalLines.push_back(progressed
                    ? "Quête principale mise à jour : Le relais doit répondre — " + storyQuestProgressText(mainPlayer, "story_ch2_relay_signal") + "."
                    : "Aucune progression directe : la quête est peut-être déjà prête à rendre auprès de Mira.");
                signalLines.push_back("Retourne notifier Mira pour que le relais soit marqué comme vraiment stabilisé.");
                MessageScreen::show("LE RELAIS RÉPOND", "story.chapter_2.relay_signal", signalLines, false);
                saveCurrentProgress("Chapitre 2 : signal du relais réactivé");
                continue;
            }

            if (storyQuestCompleted(mainPlayer, "story_ch2_relay_threat"))
            {
                MessageScreen::show(
                    "MENACE À RAPPORTER",
                    "story.chapter_2.threat.ready",
                    {
                        "Les guetteurs sans feu ont été repoussés assez nettement.",
                        "Il reste à rendre la quête principale auprès d'Orren dans PNJ notables ou Quêtes > Rendre une quête prête.",
                        "Après le rapport, le relais pourra être marqué comme stabilisé."
                    },
                    false
                );
                continue;
            }

            MenuScreen threatMenu("MENACE DU RELAIS", "story.chapter_2.threat_menu");
            threatMenu.addSubtitle("Combat imposé : Les guetteurs sans feu");
            threatMenu.addLine("La route ne ment plus seulement par ses bornes. Quelqu'un garde le silence autour du relais.");
            threatMenu.addLine("Cette menace fait partie de l'histoire : elle n'est pas proposée comme contrat libre.");
            threatMenu.addBackOption("Retour", "story.chapter_2.threat.back");
            threatMenu.addOption(1, "Entrer dans l'embuscade volontairement", "Déclencher le combat imposé du relais.", true, "story.chapter_2.threat.fight");
            const int threatChoice = TerminalInterface::askMenuChoiceFromOptions(threatMenu, "Choisis l'action du relais.");
            Console::clear();

            if (threatChoice != 1)
            {
                continue;
            }

            {
                std::vector<std::string> threatIntro = {
                    "Orren t'arrête avant la sortie : « Ceux-là ne patrouillent pas. Ils attendent que les autres oublient qu'une route devrait répondre. »",
                    "Mira ajoute seulement : « Tu n'as pas besoin de comprendre toute la fumée blanche aujourd'hui. Juste de ramener assez de silence brisé pour que la ville respire. »",
                    "Tu avances vers le relais sans torche haute. Pour une fois, l'embuscade n'est pas une surprise : c'est le rendez-vous."
                };
                const std::vector<std::string> specialLines = StoryCampaign::buildChapterTwoSpecialThreatLines(mainPlayer);
                threatIntro.insert(threatIntro.end(), specialLines.begin(), specialLines.end());
                MessageScreen::show(
                    "LES GUETTEURS SANS FEU",
                    "story.chapter_2.threat_intro",
                    threatIntro,
                    false
                );
            }

            {
                Combat combat;
                mainPlayer.recordCombatStarted();
                ShopTransactionSystem::clearBuybackAfterCombat();
                Console::useCombatTheme();
                combat.launchMonsterPve(mainPlayer, selectedDifficulty, selectedDeathRule);
                Console::useNormalTheme();
                ShopRotationSystem::markShopsDirtyAfterCombat();
            }

            if (!mainPlayer.isDead())
            {
                mainPlayer.getQuestLog().completeQuest("story_ch2_relay_threat");
                MessageScreen::show(
                    "GUETTEURS REPOUSSÉS",
                    "story.chapter_2.threat_done",
                    {
                        "Le combat ne donne pas toutes les réponses, mais il donne une certitude : le relais était gardé.",
                        "Quête principale prête : Les guetteurs sans feu.",
                        "Retourne voir Orren pour rendre le rapport et stabiliser le relais."
                    },
                    false
                );
                saveCurrentProgress("Chapitre 2 : menace du relais repoussée");
            }
            else
            {
                saveCurrentProgress("Chapitre 2 : échec contre les guetteurs sans feu");
            }
            continue;
        }
        if (chapterChoice == 5)
        {
            QuestMenu::openQuestHub(mainPlayer);
            continue;
        }
        if (chapterChoice == 6)
        {
            QuestMenu::openNotableNpcMenu(mainPlayer);
            QuestMenu::syncMainStoryQuests(mainPlayer);
            saveCurrentProgress("Chapitre 2 : discussion avec les PNJ notables");
            continue;
        }
        if (chapterChoice == 7)
        {
            openStoryAccessMenu();
            continue;
        }
    }
}


void Game::playStoryChapterThree()
{
    if (!mainPlayer.hasStoryModeStarted() || mainPlayer.getStoryChapter() < 3)
    {
        MessageScreen::show(
            "CHAPITRE 3 IMPOSSIBLE",
            "story.chapter_3.not_unlocked",
            {
                "Les routes qui répondent mal appartiennent à la suite de l'histoire.",
                "Stabilise d'abord la route gardée du chapitre 2."
            },
            false
        );
        return;
    }

    MessageScreen::show(
        "CHAPITRE 3 — LES ROUTES QUI RÉPONDENT MAL",
        "story.chapter_3.introduction",
        StoryCampaign::buildChapterThreeLines(mainPlayer),
        false
    );
    MessageScreen::show(
        "FIN TEMPORAIRE DU DÉVELOPPEMENT HISTOIRE",
        "story.development_limit",
        StoryCampaign::buildDevelopmentLimitLines(mainPlayer),
        false
    );
    return;

    const std::array<std::string, 8> questIds = {
        "story_ch3_lonely_convoy",
        "story_ch3_three_routes",
        "story_ch3_signatures",
        "story_ch3_escort_withdrawal",
        "story_ch3_margin_village",
        "story_ch3_corrected_route",
        "story_ch3_map_guardian",
        "story_ch3_convoy_return"
    };

    const std::array<std::string, 8> questTitles = {
        "Le convoi qui revient seul",
        "Trois routes pour une même borne",
        "Les signatures sans voyageurs",
        "Une escorte qui sait renoncer",
        "Le village écrit dans la marge",
        "La route corrigée",
        "Le Gardien de la Carte Juste [mini-boss]",
        "Ce que le convoi a rapporté"
    };

    bool chapterMenuOpen = true;
    while (chapterMenuOpen)
    {
        QuestMenu::syncMainStoryQuests(mainPlayer);
        const int step = mainPlayer.getStoryChapter() > 3 ? 9 : std::clamp(mainPlayer.getStoryStep(), 1, 9);

        MenuScreen chapterMenu("CHAPITRE 3 — LES ROUTES QUI RÉPONDENT MAL", "story.chapter_3.action_menu");
        chapterMenu.addSubtitle("Enquête : un convoi est revenu seul avec une cargaison impossible");
        chapterMenu.addLine("Progression : " + mainPlayer.getStoryProgressLabel());
        for (const std::string& consequence : StoryCampaign::buildChapterThreeConsequenceLines(mainPlayer))
        {
            chapterMenu.addLine(consequence);
        }
        if (step <= 8)
        {
            chapterMenu.addLine("Objectif actuel : " + questTitles[static_cast<std::size_t>(step - 1)] + ".");
            chapterMenu.addLine("État : " + storyQuestProgressText(mainPlayer, questIds[static_cast<std::size_t>(step - 1)]) + ".");
        }
        else
        {
            chapterMenu.addLine("[fait] Le convoi est classé et la route du village absent des cartes est connue.");
        }
        chapterMenu.addBackOption();
        chapterMenu.addOption(1, "Voir la mission principale", "Afficher uniquement les étapes terminées et l'étape actuelle.", true, "story.chapter_3.view_mission");
        chapterMenu.addOption(2, "Voir l'état de validation", "Consulter les quêtes principales rendues et les étapes masquées.", true, "story.chapter_3.progress");
        chapterMenu.addOption(3, "Demander ce qu'il reste à faire", "Relire la consigne actuelle sans révéler la suite.", true, "story.chapter_3.ask_next");
        chapterMenu.addOption(4, step >= 9 ? "Lire le bilan du chapitre" : "Effectuer l'étape actuelle", step >= 9 ? "Relire les conséquences déjà validées." : "Faire progresser uniquement l'objectif actuellement disponible.", true, "story.chapter_3.current_action");
        chapterMenu.addOption(5, "Ouvrir les quêtes", "Inspecter ou rendre les quêtes principales prêtes.", true, "story.chapter_3.quests");
        chapterMenu.addOption(6, "Parler aux PNJ notables", "Retrouver les personnes déjà présentes dans le monde.", true, "story.chapter_3.npcs");
        chapterMenu.addOption(7, "Accès rapides histoire", "Regrouper quêtes, PNJ, lieux, inventaire et diagnostic histoire.", true, "story.chapter_3.access");

        const int choice = TerminalInterface::askMenuChoiceFromOptions(chapterMenu, "Choisis une action du chapitre 3.");
        Console::clear();

        if (choice == 0)
        {
            return;
        }
        if (choice == 1)
        {
            MessageScreen::show("MISSION PRINCIPALE", "story.chapter_3.mission", StoryCampaign::buildChapterThreeMissionLines(mainPlayer), false);
            continue;
        }
        if (choice == 2)
        {
            MessageScreen::show("VALIDATION DU CHAPITRE 3", "story.chapter_3.progress", StoryCampaign::buildChapterThreeProgressLines(mainPlayer), false);
            continue;
        }
        if (choice == 3)
        {
            MessageScreen::show("CE QU'IL RESTE À FAIRE", "story.chapter_3.next", StoryCampaign::buildChapterThreeActionLines(mainPlayer), false);
            continue;
        }
        if (choice == 5)
        {
            QuestMenu::openQuestHub(mainPlayer);
            QuestMenu::syncMainStoryQuests(mainPlayer);
            saveCurrentProgress("Chapitre 3 : journal de quêtes consulté");
            continue;
        }
        if (choice == 6)
        {
            QuestMenu::openNotableNpcMenu(mainPlayer);
            QuestMenu::syncMainStoryQuests(mainPlayer);
            saveCurrentProgress("Chapitre 3 : PNJ notables consultés");
            continue;
        }
        if (choice == 7)
        {
            openStoryAccessMenu();
            continue;
        }
        if (choice != 4)
        {
            continue;
        }

        if (step >= 9)
        {
            MessageScreen::show(
                "CHAPITRE 3 TERMINÉ",
                "story.chapter_3.completed",
                {
                    "[fait] Le convoi revenu seul a été inspecté, classé et partiellement accepté en ville.",
                    "[fait] Le Gardien de la Carte Juste, mini-boss unique, a été repoussé.",
                    "Le contre-registre conserve désormais une route choisie, sans effacer les preuves des autres.",
                    "Une piste de boss secondaire peut rester ouverte dans le registre, mais elle n'est pas obligatoire pour poursuivre l'histoire.",
                    "La prochaine piste mène vers un village absent des cartes et présent à la mauvaise date."
                },
                false
            );
            continue;
        }

        const std::string currentQuestId = questIds[static_cast<std::size_t>(step - 1)];
        if (storyQuestCompleted(mainPlayer, currentQuestId))
        {
            MessageScreen::show(
                "OBJECTIF À NOTIFIER",
                "story.chapter_3.ready_to_turn_in",
                {
                    "[fait - à notifier] " + questTitles[static_cast<std::size_t>(step - 1)] + ".",
                    "L'action est déjà accomplie. Rends maintenant la quête depuis Quêtes ou auprès du PNJ concerné.",
                    "La prochaine étape restera masquée jusqu'à ce rendu."
                },
                false
            );
            continue;
        }

        if (step == 1)
        {
            MessageScreen::show(
                "LE CONVOI QUI REVIENT SEUL",
                "story.chapter_3.lonely_convoy",
                {
                    "Mira interdit de déplacer les caisses. Les roues portent deux boues différentes et le registre pèse plus lourd qu'au départ.",
                    "Aucun corps, aucune trace de fuite, mais quatre signatures de personnes qui n'ont jamais quitté la ville.",
                    "Tu relèves les essieux, les scellés et la quantité exacte avant le premier déchargement."
                },
                false
            );
            progressStoryQuestById(mainPlayer, currentQuestId, 1);
        }
        else if (step == 2)
        {
            const int progress = storyQuestProgressValue(mainPlayer, currentQuestId);
            const std::array<std::string, 3> measurements = {
                "À l'aube, la borne annonce une route plus courte que la veille.",
                "Au milieu du jour, la même borne ajoute presque une heure de marche.",
                "La nuit, la distance revient à sa première valeur, mais la direction change."
            };
            MessageScreen::show("MESURE DE ROUTE", "story.chapter_3.three_routes.measure", {measurements[static_cast<std::size_t>(std::clamp(progress, 0, 2))]}, false);
            progressStoryQuestById(mainPlayer, currentQuestId, 1);
        }
        else if (step == 3)
        {
            const int progress = storyQuestProgressValue(mainPlayer, currentQuestId);
            const std::array<std::string, 3> witnesses = {
                "Soryn confirme que deux signatures utilisent une encre d'archive qui n'a jamais quitté sa salle.",
                "Eda démontre que le poids déclaré ne correspond à aucun chargement parti de la ville.",
                "Nell reconnaît un sceau de convoi utilisé par un village qui n'existe sur aucune carte actuelle."
            };
            MessageScreen::show("SIGNATURE IMPOSSIBLE", "story.chapter_3.signatures.check", {witnesses[static_cast<std::size_t>(std::clamp(progress, 0, 2))]}, false);
            progressStoryQuestById(mainPlayer, currentQuestId, 1);
        }
        else if (step == 4)
        {
            const int progress = storyQuestProgressValue(mainPlayer, currentQuestId);
            if (progress <= 0)
            {
                MessageScreen::show("ESCORTE COURTE", "story.chapter_3.escort.start", {"Le petit convoi part avec peu de marchandises et une règle claire : personne ne meurt pour sauver une caisse."}, false);
            }
            else
            {
                MessageScreen::show("DEMI-TOUR SÉCURISÉ", "story.chapter_3.escort.withdraw", {"La route change sous les roues. Tu fais demi-tour avant l'embuscade. Orren classe ce renoncement comme une réussite, pas comme une fuite."}, false);
            }
            progressStoryQuestById(mainPlayer, currentQuestId, 1);
        }
        else if (step == 5)
        {
            MessageScreen::show(
                "LE VILLAGE ÉCRIT DANS LA MARGE",
                "story.chapter_3.margin_village",
                {
                    "Nell déplie une doublure cachée dans la sacoche du convoi.",
                    "Un village est écrit dans la marge, avec des horaires de passage mais aucune position fixe.",
                    "La preuve est assez nette pour préparer une route, pas assez pour y envoyer des habitants."
                },
                false
            );
            progressStoryQuestById(mainPlayer, currentQuestId, 1);
        }
        else if (step == 6)
        {
            MenuScreen routeChoice("LA ROUTE CORRIGÉE", "story.chapter_3.corrected_route.choice");
            routeChoice.addSubtitle("Choisis ce que le contre-registre protégera en priorité");
            routeChoice.addLine("Ce choix modifie le récit et les futurs stocks, sans supprimer les preuves des autres routes.");
            routeChoice.addBackOption();
            routeChoice.addOption(1, "Route du commerce", "Prioriser les marchandises et la reconstruction.", true, "story.chapter_3.corrected_route.commerce");
            routeChoice.addOption(2, "Route des secours", "Prioriser les blessés, messagers et retours vivants.", true, "story.chapter_3.corrected_route.rescue");
            routeChoice.addOption(3, "Route de recherche", "Prioriser les preuves, cartes et relevés.", true, "story.chapter_3.corrected_route.research");
            const int route = TerminalInterface::askMenuChoiceFromOptions(routeChoice, "Choisis une version de route.");
            Console::clear();
            if (route == 0) continue;
            Quest* quest = findMutableStoryQuest(mainPlayer, currentQuestId);
            if (quest != nullptr)
            {
                quest->rewardNote = route == 1 ? "Choix durable : route du commerce." : route == 2 ? "Choix durable : route des secours." : "Choix durable : route de recherche.";
            }
            progressStoryQuestById(mainPlayer, currentQuestId, 1);
            MessageScreen::show("CONTRE-REGISTRE CORRIGÉ", "story.chapter_3.corrected_route.done", {quest != nullptr ? quest->rewardNote : "La route choisie a été enregistrée."}, false);
        }
        else if (step == 7)
        {
            MessageScreen::show(
                "LE GARDIEN DE LA CARTE JUSTE",
                "story.chapter_3.map_guardian.intro",
                {
                    "La présence prononce son propre titre avant d'avancer : « Je garde la carte juste. Pas vos vies. »",
                    "Classification : mini-boss unique — plus dangereux qu'un élite ou une créature évoluée, mais inférieur à un vrai boss du registre.",
                    "La route qu'il protège est cohérente, mais elle condamne le convoi au trajet le plus dangereux."
                },
                false
            );

            const int miniBossLevel = std::max(6, mainPlayer.getLevel() + 1);
            Monster mapGuardian(
                "Le Gardien de la Carte Juste",
                "Mini-boss unique / sentinelle cartographique",
                Race::AnomalieArcanique,
                miniBossLevel,
                260 + miniBossLevel * 48,
                14 + miniBossLevel * 3,
                28 + miniBossLevel * 5,
                42 + miniBossLevel * 7,
                1,
                2,
                false,
                true,
                false,
                true
            );

            mainPlayer.recordCombatStarted();
            ShopTransactionSystem::clearBuybackAfterCombat();
            Console::useCombatTheme();
            Random miniBossRandom;
            const bool miniBossDefeated = MonsterPveMode::runExplorationWave(
                mainPlayer,
                miniBossRandom,
                selectedDifficulty,
                selectedDeathRule,
                {mapGuardian},
                "Mini-boss unique : le Gardien de la Carte Juste verrouille la route corrigée."
            );
            Console::useNormalTheme();
            ShopRotationSystem::markShopsDirtyAfterCombat();

            if (miniBossDefeated && !mainPlayer.isDead())
            {
                progressStoryQuestById(mainPlayer, currentQuestId, 1);
                const bool optionalBossUnlocked = mainPlayer.unlockBoss(9);
                saveCurrentProgress("Chapitre 3 : mini-boss Gardien de la Carte Juste vaincu");

                if (optionalBossUnlocked)
                {
                    MessageScreen::show(
                        "PISTE DE BOSS SECONDAIRE",
                        "story.chapter_3.optional_boss.unlocked",
                        {
                            "Le mini-boss laisse une réflexion qui ne correspond à aucune route réelle.",
                            "Une nouvelle variation de boss est stabilisée dans le registre : identité encore inconnue.",
                            "Danger estimé par le registre : niveau conseillé " + std::to_string(BossCatalog::getRecommendedLevel(9)) + ".",
                            "Cette confrontation est secondaire et volontaire : elle n'est pas requise pour terminer le chapitre."
                        },
                        false
                    );
                }
            }
            else
            {
                saveCurrentProgress("Chapitre 3 : échec face au mini-boss Gardien de la Carte Juste");
                continue;
            }
        }
        else if (step == 8)
        {
            MenuScreen conclusion("CE QUE LE CONVOI A RAPPORTÉ", "story.chapter_3.convoy_return.choice");
            conclusion.addSubtitle("Mira demande ce qui peut franchir les portes");
            conclusion.addBackOption();
            conclusion.addOption(1, "Faire entrer les marchandises contrôlées", "Aider rapidement les stocks, sous surveillance.", true, "story.chapter_3.convoy_return.goods");
            conclusion.addOption(2, "Faire entrer seulement les preuves", "Refuser les marchandises jusqu'à comprendre leur provenance.", true, "story.chapter_3.convoy_return.proofs");
            conclusion.addOption(3, "Mettre tout le convoi en quarantaine", "Choisir la sécurité maximale et retarder les bénéfices.", true, "story.chapter_3.convoy_return.quarantine");
            const int conclusionChoice = TerminalInterface::askMenuChoiceFromOptions(conclusion, "Décide ce qui entre en ville.");
            Console::clear();
            if (conclusionChoice == 0) continue;
            Quest* quest = findMutableStoryQuest(mainPlayer, currentQuestId);
            if (quest != nullptr)
            {
                quest->rewardNote = conclusionChoice == 1 ? "Décision : marchandises contrôlées admises." : conclusionChoice == 2 ? "Décision : seules les preuves sont admises." : "Décision : convoi placé en quarantaine.";
            }
            progressStoryQuestById(mainPlayer, currentQuestId, 1);
            MessageScreen::show("DÉCISION ENREGISTRÉE", "story.chapter_3.convoy_return.done", {quest != nullptr ? quest->rewardNote : "La décision de Mira est enregistrée."}, false);
        }

        QuestMenu::syncMainStoryQuests(mainPlayer);
        saveCurrentProgress("Chapitre 3 : progression de l'étape actuelle");
        MessageScreen::show(
            "QUÊTE MISE À JOUR",
            "story.chapter_3.quest_updated",
            {
                questTitles[static_cast<std::size_t>(step - 1)] + " — " + storyQuestProgressText(mainPlayer, currentQuestId) + ".",
                storyQuestCompleted(mainPlayer, currentQuestId)
                    ? "[fait - à notifier] Rends la quête pour dévoiler l'étape suivante."
                    : "La prochaine sous-étape est maintenant disponible."
            },
            false
        );
    }
}

void Game::playStoryChapterFour()
{
    if (!mainPlayer.hasStoryModeStarted() || mainPlayer.getStoryChapter() < 4)
    {
        MessageScreen::show(
            "CHAPITRE 4 IMPOSSIBLE",
            "story.chapter_4.not_unlocked",
            {
                "Le village à la mauvaise date ne peut pas être atteint tant que le convoi du chapitre 3 n'est pas classé.",
                "Termine et rends la dernière étape du chapitre précédent."
            },
            false
        );
        return;
    }

    bool chapterMenuOpen = true;
    while (chapterMenuOpen)
    {
        const int step = std::clamp(mainPlayer.getStoryStep(), 1, 9);
        MenuScreen chapterMenu("CHAPITRE 4 — LE VILLAGE À LA MAUVAISE DATE", "story.chapter_4.action_menu");
        chapterMenu.addSubtitle(step <= 5 ? "Première phase : comprendre le village" : "Deuxième phase : conserver les preuves et identifier la menace");
        chapterMenu.addLine("Progression : " + mainPlayer.getStoryProgressLabel());
        chapterMenu.addLine(step >= 9
            ? "[fait] La deuxième phase est terminée. La menace majeure est identifiée, mais le combat reste à préparer."
            : "Objectif actuel : " + StoryCampaign::buildChapterFourActionLines(mainPlayer)[1]);
        chapterMenu.addBackOption();
        chapterMenu.addOption(1, "Voir la mission principale", "Afficher les étapes connues du chapitre.", true, "story.chapter_4.view_mission");
        chapterMenu.addOption(2, "Voir l'état de l'enquête", "Vérifier ce qui est établi et ce qui reste volontairement non nommé.", true, "story.chapter_4.progress");
        chapterMenu.addOption(3, "Demander ce qu'il reste à faire", "Relire uniquement l'action actuelle.", true, "story.chapter_4.ask_next");
        chapterMenu.addOption(4, step >= 9 ? "Relire le bilan de la deuxième phase" : "Effectuer l'étape actuelle", step >= 9 ? "Revoir les preuves et la menace identifiée." : "Faire avancer l'enquête d'une scène.", true, "story.chapter_4.current_action");
        chapterMenu.addOption(5, "Ouvrir les quêtes", "Consulter les quêtes et conséquences déjà actives.", true, "story.chapter_4.quests");
        chapterMenu.addOption(6, "Parler aux PNJ notables", "Observer les réactions durables du chapitre 3 avant de repartir.", true, "story.chapter_4.npcs");
        chapterMenu.addOption(7, "Accès rapides histoire", "Regrouper quêtes, PNJ, lieux, inventaire et diagnostic histoire.", true, "story.chapter_4.access");

        const int choice = TerminalInterface::askMenuChoiceFromOptions(chapterMenu, "Choisis une action du chapitre 4.");
        Console::clear();
        if (choice == 0) return;
        if (choice == 1)
        {
            MessageScreen::show("MISSION PRINCIPALE", "story.chapter_4.mission", StoryCampaign::buildChapterFourMissionLines(mainPlayer), false);
            continue;
        }
        if (choice == 2)
        {
            MessageScreen::show("ÉTAT DE L'ENQUÊTE", "story.chapter_4.progress", StoryCampaign::buildChapterFourProgressLines(mainPlayer), false);
            continue;
        }
        if (choice == 3)
        {
            MessageScreen::show("PROCHAINE ACTION", "story.chapter_4.next", StoryCampaign::buildChapterFourActionLines(mainPlayer), false);
            continue;
        }
        if (choice == 5)
        {
            QuestMenu::openQuestHub(mainPlayer);
            saveCurrentProgress("Chapitre 4 : journal de quêtes consulté");
            continue;
        }
        if (choice == 6)
        {
            QuestMenu::openNotableNpcMenu(mainPlayer);
            saveCurrentProgress("Chapitre 4 : PNJ notables consultés");
            continue;
        }
        if (choice == 7)
        {
            openStoryAccessMenu();
            continue;
        }
        if (choice != 4) continue;

        if (step >= 9)
        {
            MessageScreen::show(
                "DEUXIÈME PHASE TERMINÉE",
                "story.chapter_4.phase_two_complete",
                {
                    "[fait] Le contre-registre résiste désormais à une sonnerie complète.",
                    "[fait] Le Sonneur sans heure est identifié comme gardien du cycle, pas comme son maître.",
                    "[fait] Le témoin effacé a confirmé l'existence du registre intérieur.",
                    "[fait] L'Intendant de la Date Vide est maintenant identifié par une preuve directe comme menace majeure.",
                    "La phase suivante préparera la salle des dates, ses règles et le véritable affrontement."
                },
                false
            );
            continue;
        }

        if (step == 1)
        {
            std::vector<std::string> lines = {
                "Les trois mesures de la borne s'accordent pendant moins d'une heure. La route cesse enfin de bouger sous les pas.",
                "Le village apparaît derrière une rangée d'arbres trop jeunes pour les maisons qu'ils protègent.",
                "À l'entrée, une pancarte porte le même nom que la marge du convoi, mais aucune distance pour repartir."
            };
            const std::vector<std::string> consequences = StoryCampaign::buildChapterThreeConsequenceLines(mainPlayer);
            if (!consequences.empty()) lines.push_back("Ta préparation suit le choix précédent : " + consequences.front());
            MessageScreen::show("LE VILLAGE AU BOUT DE LA BONNE HEURE", "story.chapter_4.arrival", lines, false);
        }
        else if (step == 2)
        {
            MessageScreen::show(
                "TROIS DATES POUR UN MÊME LIEU",
                "story.chapter_4.dates",
                {
                    "Le registre de la mairie date la semaine de demain.",
                    "Les tombes les plus récentes portent une année terminée depuis longtemps.",
                    "La cloche, elle, a été réparée ce matin selon le forgeron — mais la soudure est couverte de plusieurs hivers de rouille.",
                    "Tu copies chaque date séparément au lieu d'en choisir une qui arrangerait l'enquête."
                },
                false
            );
        }
        else if (step == 3)
        {
            MessageScreen::show(
                "CEUX QUI CONNAISSENT DÉJÀ LE CONVOI",
                "story.chapter_4.witnesses",
                {
                    "Une aubergiste décrit les quatre conducteurs disparus avec leurs vrais noms.",
                    "Un enfant affirme que les caisses sont reparties hier, alors qu'elles sont encore sous surveillance dans ta ville.",
                    "Le doyen demande pourquoi le convoi a mis autant de temps à revenir, comme si l'aller avait eu lieu depuis des mois.",
                    "Personne ne semble mentir de la même manière. C'est précisément ce qui rend leurs réponses crédibles."
                },
                false
            );
        }
        else if (step == 4)
        {
            MenuScreen nightChoice("LA PREMIÈRE NUIT SOUS LA CLOCHE", "story.chapter_4.first_night.choice");
            nightChoice.addSubtitle("La cloche sonne avant que le soleil disparaisse");
            nightChoice.addLine("Les habitants ferment les volets, mais la rue reste pleine de pas sans silhouettes.");
            nightChoice.addBackOption();
            nightChoice.addOption(1, "Rester près de la cloche", "Observer la source du phénomène au risque de perdre la sortie.", true, "story.chapter_4.first_night.bell");
            nightChoice.addOption(2, "Garder la route visible", "Préserver le retour et observer le village depuis sa limite.", true, "story.chapter_4.first_night.road");
            nightChoice.addOption(3, "Suivre les pas sans silhouettes", "Chercher ce que la cloche retire au lieu de regarder ce qu'elle laisse.", true, "story.chapter_4.first_night.steps");
            const int night = TerminalInterface::askMenuChoiceFromOptions(nightChoice, "Choisis ton poste d'observation.");
            Console::clear();
            if (night == 0) continue;

            std::vector<std::string> lines;
            if (night == 1)
            {
                lines = {
                    "La cloche vibre sans être frappée. À chaque son, un nom disparaît d'une plaque puis revient sur une autre.",
                    "Tu comprends que le village ne se déplace pas seulement dans l'espace : il redistribue les preuves de son existence."
                };
            }
            else if (night == 2)
            {
                lines = {
                    "La route pâlit à chaque son, mais les repères du contre-registre tiennent assez longtemps pour ne pas être effacés.",
                    "Le village perd plusieurs maisons dans la brume, tandis que leurs habitants continuent de parler derrière des murs absents."
                };
            }
            else
            {
                lines = {
                    "Les pas te conduisent vers une place qui n'existait pas au coucher du soleil.",
                    "Au centre, des silhouettes rejouent le déchargement du convoi sans caisses, comme le souvenir d'un événement qui n'a pas encore choisi sa date."
                };
            }
            lines.push_back("Tu refuses d'affronter une identité encore inconnue. Cette nuit sert à établir la menace, pas à inventer son nom.");
            MessageScreen::show("PREMIÈRE NUIT OBSERVÉE", "story.chapter_4.first_night.result", lines, false);
        }

        else if (step == 5)
        {
            MessageScreen::show(
                "LE CONTRE-REGISTRE",
                "story.chapter_4.counter_register",
                {
                    "Soryn, Nell et Eda ont préparé trois copies qui ne séjournent jamais ensemble dans le village.",
                    "Une page reste sur la route, une autre sous la cloche et la dernière dans une boîte scellée hors des bornes.",
                    "Lorsque la cloche sonne, deux versions changent. La troisième conserve un nom rayé : le Sonneur sans heure.",
                    "Pour la première fois, le phénomène laisse une contradiction qu'il ne parvient pas à répartir ailleurs."
                },
                false
            );
        }
        else if (step == 6)
        {
            MenuScreen sonneurChoice("LE SONNEUR SANS HEURE", "story.chapter_4.bell_keeper.choice");
            sonneurChoice.addSubtitle("Premier gardien actif du cycle");
            sonneurChoice.addLine("Une silhouette tire sur une corde qui ne rejoint aucune cloche visible.");
            sonneurChoice.addBackOption();
            sonneurChoice.addOption(1, "Couper son rythme", "Interrompre les intervalles entre les sonneries sans détruire la corde.", true, "story.chapter_4.bell_keeper.rhythm");
            sonneurChoice.addOption(2, "Protéger le contre-registre", "Laisser le gardien se dévoiler en tentant d'effacer la seule page stable.", true, "story.chapter_4.bell_keeper.register");
            sonneurChoice.addOption(3, "Suivre la corde", "Remonter jusqu'à l'endroit où elle traverse une date absente.", true, "story.chapter_4.bell_keeper.rope");
            const int sonneur = TerminalInterface::askMenuChoiceFromOptions(sonneurChoice, "Choisis comment repousser le gardien.");
            Console::clear();
            if (sonneur == 0) continue;

            std::vector<std::string> lines;
            if (sonneur == 1) lines = {"Tu imposes un rythme irrégulier. Le Sonneur manque une mesure et son corps se dédouble entre deux âges.", "Il recule lorsque la cloche répond avant lui, comme si une autorité supérieure venait de corriger son geste."};
            else if (sonneur == 2) lines = {"Le gardien frappe directement la page stable. Le contre-registre absorbe trois dates et révèle le sceau d'un supérieur.", "Le Sonneur fuit dès que son propre nom apparaît dans une colonne réservée aux serviteurs."};
            else lines = {"La corde traverse une porte condamnée puis ressort du puits communal plusieurs décennies plus tôt.", "En la suivant, tu forces le Sonneur à abandonner sa position pour protéger l'accès au registre intérieur."};
            lines.push_back("Le gardien est repoussé, pas détruit. Son rôle est maintenant clair : il entretient le cycle au nom de quelqu'un d'autre.");
            MessageScreen::show("GARDIEN REPOUSSÉ", "story.chapter_4.bell_keeper.result", lines, false);
        }
        else if (step == 7)
        {
            MessageScreen::show(
                "LE TÉMOIN EFFACÉ",
                "story.chapter_4.erased_witness",
                {
                    "Le même prénom apparaît dans le registre de l'auberge, sur une tombe vide et au dos d'une facture du convoi.",
                    "En réunissant les trois écritures, une personne réapparaît quelques minutes dans une maison sans porte.",
                    "Elle affirme avoir travaillé pour un intendant qui classe les événements refusés par les calendriers ordinaires.",
                    "Avant de disparaître, elle indique que le registre intérieur ne s'ouvre qu'avec une date qui n'a jamais eu lieu."
                },
                false
            );
        }
        else if (step == 8)
        {
            MessageScreen::show(
                "L'INTENDANT DE LA DATE VIDE",
                "story.chapter_4.major_threat_revealed",
                {
                    "Le contre-registre, le témoignage effacé et le sceau du Sonneur décrivent enfin la même autorité.",
                    "Nom de fonction : l'Intendant de la Date Vide.",
                    "Il ne crée pas les paradoxes : il récupère les événements sans date reconnue et les range dans le village jusqu'à ce qu'ils puissent remplacer une réalité existante.",
                    "Le village protège son registre intérieur. Le Sonneur maintient le cycle. L'Intendant décide ce qui sera réécrit.",
                    "Cette identité devient le boss majeur cohérent du chapitre 4, mais son combat exige encore la construction de la salle des dates et de ses règles."
                },
                false
            );
        }

        mainPlayer.setStoryProgress(4, std::min(9, step + 1), std::max(10, mainPlayer.getStoryCityDevelopmentLevel()));
        saveCurrentProgress("Chapitre 4 : enquête avancée");
        MessageScreen::show(
            "ENQUÊTE MISE À JOUR",
            "story.chapter_4.updated",
            StoryCampaign::buildChapterFourActionLines(mainPlayer),
            false
        );
    }
}

void Game::playStoryWhiteFogPrologue()
{
    if (!mainPlayer.hasStoryModeStarted())
    {
        mainPlayer.startStoryMode();
    }

    if (mainPlayer.getStoryStep() >= 2 && mainPlayer.getInventory().getWeaponCount() > 0)
    {
        MessageScreen::show(
            "PROLOGUE DÉJÀ FRANCHI",
            "story.white_fog.already_done",
            {
                "La fumée blanche a déjà reculé pour ce personnage.",
                "Tu peux relire l’introduction, mais le kit de départ a déjà été trouvé.",
                "Progression : " + mainPlayer.getStoryProgressLabel()
            },
            false
        );
        return;
    }

    mainPlayer.getInventory().clearAll();
    mainPlayer.setStoryProgress(1, 1, std::max(0, mainPlayer.getStoryCityDevelopmentLevel()));

    MessageScreen::show("PROLOGUE — MISSION ORDINAIRE", "story.white_fog.intro", StoryCampaign::buildWhiteFogPrologueLines(mainPlayer));
    MessageScreen::show("LA FUMÉE MANGE LES NOMS", "story.white_fog.memory_loss", StoryCampaign::buildWhiteFogMemoryLossLines(mainPlayer), false);

    MenuScreen firstChoice("FORÊT BLANCHE", "story.white_fog.choice_1");
    firstChoice.addSubtitle("Aucun équipement. Aucun souvenir fiable.");
    firstChoice.addLine("La brume mange les troncs. Elle ne cache pas la forêt : elle la remplace.");
    firstChoice.addLine("Tes mains sont vides. Même ton nom paraît venir de quelqu’un d’autre.");
    firstChoice.addOption(1, "Avancer lentement vers les arbres moins blancs", "Garder une direction sans courir dans la brume.", true, "story.white_fog.slow");
    firstChoice.addOption(2, "Courir droit devant", "Chercher une sortie immédiate, au risque de perdre le peu de repères restants.", true, "story.white_fog.run");
    firstChoice.addOption(3, "Appeler à l’aide", "Tester si quelqu’un répond dans la fumée.", true, "story.white_fog.call");
    int first = TerminalInterface::askMenuChoiceFromOptions(firstChoice, "Choisis ta première réaction.");
    Console::clear();

    std::vector<std::string> report;
    if (first == 1)
    {
        report.push_back("Tu avances lentement. La fumée tire sur tes souvenirs, mais elle ne parvient pas à te faire tourner en rond tout de suite.");
    }
    else if (first == 2)
    {
        report.push_back("Tu cours. La forêt te laisse faire trois secondes, puis remet un arbre devant toi. Tu comprends que la panique nourrit la brume.");
    }
    else
    {
        report.push_back("Tu appelles. Une voix répond avec ton intonation exacte. Tu décides de ne pas lui confier ta direction.");
    }

    {
        const std::vector<std::string> specialLines = StoryCampaign::buildWhiteFogFirstReactionLines(mainPlayer, first);
        report.insert(report.end(), specialLines.begin(), specialLines.end());
    }
    report.push_back("Un corps bas traverse la brume. Pas assez net pour être nommé. Assez proche pour attaquer.");
    MessageScreen::show("RENCONTRE", "story.white_fog.encounter_1", report, false);

    MenuScreen encounter("RENCONTRE SANS ARME", "story.white_fog.encounter_choice");
    encounter.addSubtitle("Combat impossible : seulement parade ou fuite.");
    encounter.addLine("Tu n’as ni arme, ni armure, ni potion fiable.");
    encounter.addLine("L’objectif n’est pas de gagner. L’objectif est de sortir vivant.");
    encounter.addOption(1, "Parer avec les avant-bras et reculer", "Réduire le choc et chercher une ouverture.", true, "story.white_fog.block");
    encounter.addOption(2, "Fuir vers les arbres moins serrés", "Tenter de casser la ligne d’attaque.", true, "story.white_fog.escape");
    int second = TerminalInterface::askMenuChoiceFromOptions(encounter, "Choisis : parer ou fuir.");
    Console::clear();

    if (second == 1)
    {
        std::vector<std::string> blockLines = {
            "Le choc remonte jusqu’aux épaules.",
            "Tu ne gagnes pas le combat, mais tu refuses de tomber là.",
            "La créature hésite juste assez pour ouvrir un passage."
        };
        const std::vector<std::string> specialLines = StoryCampaign::buildWhiteFogEncounterReactionLines(mainPlayer, second);
        blockLines.insert(blockLines.end(), specialLines.begin(), specialLines.end());
        MessageScreen::show("PARADE", "story.white_fog.block_result", blockLines, false);
    }
    else
    {
        std::vector<std::string> escapeLines = {
            "Tu fuis sans regarder derrière.",
            "La brume essaie de remettre tes pas au même endroit, mais le sol descend enfin.",
            "Quelque chose grogne derrière toi, puis abandonne."
        };
        const std::vector<std::string> specialLines = StoryCampaign::buildWhiteFogEncounterReactionLines(mainPlayer, second);
        escapeLines.insert(escapeLines.end(), specialLines.begin(), specialLines.end());
        MessageScreen::show("FUITE", "story.white_fog.escape_result", escapeLines, false);
    }

    MenuScreen exitChoice("SORTIE DE FORÊT", "story.white_fog.choice_2");
    exitChoice.addSubtitle("Une silhouette au sol");
    exitChoice.addLine("La fumée s’ouvre sur un fossé humide. Un cadavre repose contre une racine, déjà trop froid pour raconter son histoire.");
    exitChoice.addLine("À côté de lui : un paquet simple, abîmé, mais utilisable.");
    exitChoice.addOption(1, "Prendre le paquet et fermer les yeux du mort", "Récupérer le kit de départ sans traiter le corps comme un coffre gratuit.", true, "story.white_fog.take_respect");
    exitChoice.addOption(2, "Prendre seulement ce qui peut sauver ta vie", "Rester pratique : la forêt n’attend pas.", true, "story.white_fog.take_practical");
    exitChoice.addOption(3, "Fouiller vite et partir", "Sortie nerveuse : survivre d’abord, réfléchir ensuite.", true, "story.white_fog.take_fast");
    int third = TerminalInterface::askMenuChoiceFromOptions(exitChoice, "Choisis comment récupérer le premier équipement.");
    Console::clear();

    mainPlayer.initializeStarterInventory(selectedDifficulty);
    mainPlayer.setStoryProgress(1, 2, std::max(1, mainPlayer.getStoryCityDevelopmentLevel()));
    saveCurrentProgress("Prologue fumée blanche terminé");

    std::vector<std::string> endLines;
    if (third == 1)
    {
        endLines.push_back("Tu prends le paquet, puis tu fermes les yeux du mort. Ce n’est pas grand-chose, mais la forêt semble moins blanche pendant une seconde.");
    }
    else if (third == 2)
    {
        endLines.push_back("Tu prends seulement le nécessaire. La honte viendra peut-être plus tard. Pour l’instant, respirer suffit.");
    }
    else
    {
        endLines.push_back("Tu fouilles vite. La peur te rend maladroit, mais vivant. La brume n’aura pas tout aujourd’hui.");
    }
    {
        const std::vector<std::string> specialLines = StoryCampaign::buildWhiteFogKitReactionLines(mainPlayer, third);
        endLines.insert(endLines.end(), specialLines.begin(), specialLines.end());
    }
    endLines.push_back("Kit de départ récupéré. La vraie histoire peut commencer.");
    endLines.push_back("Au loin, une ville tient à peine debout. Ce sera ton premier refuge, pas encore ta maison.");
    MessageScreen::show("PROLOGUE TERMINÉ", "story.white_fog.completed", endLines, false);
}

void Game::launchEphemeralSandboxCloneFromStory()
{
    if (!mainPlayer.hasStoryModeStarted())
    {
        MessageScreen::show(
            "CLONE IMPOSSIBLE",
            "story.ephemeral.no_story",
            {
                "Le clone éphémère sert à quitter temporairement une route histoire déjà engagée.",
                "Commence d’abord l’histoire, puis reviens ici si tu veux une session libre sans sauvegarde."
            },
            false
        );
        return;
    }

    MenuScreen choice("BAC À SABLE ÉPHÉMÈRE", "story.ephemeral.choice");
    choice.addSubtitle("Abandonner le monde à sa destinée, ou cloner juste aujourd’hui ?");
    choice.addLine("Passer réellement en bac à sable abandonnerait la route histoire de ce personnage.");
    choice.addLine("Le clone éphémère permet de jouer librement pendant la session sans sauvegarder ni perturber l’histoire.");
    choice.addOption(0, "Retour", "Ne rien changer.", true, "story.ephemeral.back");
    choice.addOption(1, "Abandonner le monde à sa destinée", "Quitter mentalement la route histoire. Pour l’instant, cette option reste une décision forte à confirmer plus tard.", true, "story.ephemeral.abandon");
    choice.addOption(2, "Créer un clone pour aujourd’hui", "Session non sauvegardée, supprimée à la fin ou au prochain nettoyage si elle existe encore.", true, "story.ephemeral.clone");
    const int selected = TerminalInterface::askMenuChoiceFromOptions(choice, "Choisis la sortie temporaire ou le retour.");
    Console::clear();

    if (selected == 0)
    {
        return;
    }

    if (selected == 1)
    {
        MessageScreen::show(
            "DESTIN REFUSÉ",
            "story.ephemeral.abandon_scaffold",
            {
                "Cette option représentera plus tard un vrai abandon du fil histoire.",
                "Pour éviter une erreur définitive pendant le développement, elle reste informative pour l’instant.",
                "Utilise plutôt le clone éphémère si tu veux te régaler sans casser la campagne."
            },
            false
        );
        return;
    }

    Player savedStoryPlayer = mainPlayer;
    const GameMode savedMode = selectedMode;
    const bool previousEphemeral = ephemeralSandboxSession;

    mainPlayer.setClone(true);
    ephemeralSandboxSession = true;

    MessageScreen::show(
        "CLONE ÉPHÉMÈRE CRÉÉ",
        "story.ephemeral.clone_created",
        {
            "Un clone de session est créé pour aujourd’hui.",
            "Il peut jouer dans le bac à sable, mais aucune sauvegarde réelle ne sera écrite.",
            "À la fin de la session libre, le personnage histoire original sera restauré."
        },
        false
    );

    chooseGameMode();
    if (selectedMode == GameMode::Story)
    {
        MessageScreen::show(
            "CLONE REFUSÉ PAR L’HISTOIRE",
            "story.ephemeral.story_blocked",
            {
                "Le clone éphémère ne sert pas à écrire l’histoire à la place du vrai personnage.",
                "Retour au menu histoire."
            },
            false
        );
    }
    else
    {
        displaySelectedMode();
        launchSelectedMode();
    }

    mainPlayer = savedStoryPlayer;
    selectedMode = savedMode;
    ephemeralSandboxSession = previousEphemeral;
    saveCurrentProgress("Retour du clone éphémère vers la route histoire");

    MessageScreen::show(
        "ROUTE HISTOIRE RESTAURÉE",
        "story.ephemeral.restored",
        {
            "Le clone éphémère se dissipe.",
            "La sauvegarde histoire reprend exactement avec le personnage original.",
            "Aucune ressource gagnée en clone n’a été transférée."
        },
        false
    );
}

void Game::showStoryCompletionSandboxRule() const
{
    MessageScreen::show(
        "APRÈS LA FIN",
        "story.completion.sandbox_rule",
        StoryCampaign::buildStoryCompletionLines(mainPlayer),
        false
    );
}

// EN: launchChallengeBoard declares or implements a focused behavior used by this module.
// FR: launchChallengeBoard déclare ou implémente un comportement précis utilisé par ce module.
