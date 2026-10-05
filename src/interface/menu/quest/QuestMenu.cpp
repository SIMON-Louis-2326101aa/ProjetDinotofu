// EN: QuestMenu.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: QuestMenu.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu.
// Description: Implements quest hub and read-only quest journal for Dinotofu.

#include "interface/menu/quest/QuestMenu.hpp"
#include "interface/menu/quest/QuestMenuInternalSupport.hpp"
#include "interface/menu/quest/QuestExplorationSupport.hpp"
#include "interface/menu/quest/QuestContractorMenu.hpp"
#include "interface/menu/quest/QuestTeamMenu.hpp"
#include "interface/menu/quest/QuestWorldMenuSupport.hpp"
#include "interface/menu/quest/QuestDeadlineSupport.hpp"
#include "interface/menu/quest/QuestClientNavigationSupport.hpp"
#include "interface/menu/quest/QuestStorySupport.hpp"
#include "interface/menu/quest/QuestPresentationSupport.hpp"
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

using QuestWorldMenuSupport::currentCityName;
using QuestWorldMenuSupport::guildRequestRankDUnlocked;
using QuestWorldMenuSupport::openCityTravelMenu;
using QuestWorldMenuSupport::openCityVault;
using QuestDeadlineSupport::expireOverdueQuestDeadlines;
using QuestClientNavigationSupport::ClientQuestCounts;
using QuestClientNavigationSupport::canCompleteMaterialDelivery;
using QuestClientNavigationSupport::collectRecommendedClients;
using QuestClientNavigationSupport::extractRecommendedClientName;
using QuestClientNavigationSupport::countQuestsForClient;
using QuestClientNavigationSupport::clientQuestHintText;
using QuestClientNavigationSupport::clientQuestStatusText;
using QuestClientNavigationSupport::hasRequiredQuestMaterial;
using QuestClientNavigationSupport::isMaterialDeliveryQuest;
using QuestClientNavigationSupport::isReadyToTurnIn;
using QuestClientNavigationSupport::isRecommendedClientName;
using QuestClientNavigationSupport::makeClientQuestNavigationItemData;
using QuestExplorationSupport::MicroChallengeResult;
using QuestExplorationSupport::randomBiomeForClient;
using QuestExplorationSupport::runGuildServiceMicroChallenge;
using QuestExplorationSupport::buildNpcQuestByRoll;
using QuestExplorationSupport::displayQuestOffer;
using QuestExplorationSupport::simulateAfterCombatMiniBoss;

namespace
{
    using namespace QuestStorySupport;
    using namespace QuestPresentationSupport;

    std::string questPlayableLocationHint(const Quest& quest);
    int prunigilTrustScore(const Player& player);
    std::string prunigilTrustRankLabel(int score);
    std::string prunigilNextMilestoneLine(int score);
    void openChallengeMarkCounter(Player& player);


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

    bool heroVillagerProgressGateIsOpen(const Player& player)
    {
        if (player.hasTitle("Témoin du marchand bleu"))
        {
            return true;
        }

        if (player.getWorldDaysElapsed() < 10 || player.getLevel() < 4)
        {
            return false;
        }

        int completedGuildContracts = 0;
        for (const Quest& quest : player.getQuestLog().getQuests())
        {
            if (quest.guildQuest && quest.turnedIn)
            {
                ++completedGuildContracts;
            }
        }

        if (completedGuildContracts >= 2)
        {
            return true;
        }

        if (player.hasStoryModeStarted())
        {
            if (player.getStoryChapter() >= 2)
            {
                return true;
            }

            if (player.getStoryChapter() == 1 && player.getStoryStep() >= 5)
            {
                return true;
            }
        }

        return player.getCanonicalJournalCategoryTotal("actions_tactiques_combat") >= 6
            || player.getCanonicalJournalCategoryTotal("observations_terrain") >= 3;
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
        if (Money::coinStacksValueInCopper(quest.rewardCoins) > 0)
        {
            // An explicitly authored physical payout is contractual: do not rebalance or
            // silently replace its denominations with the legacy PF reward field.
            return 0;
        }

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
        const long long exactCoinCopper = Money::coinStacksValueInCopper(quest.rewardCoins);
        const int displayedGold = balancedQuestGold(quest);

        if (exactCoinCopper > 0)
        {
            text += " | Pièces exactes +" + Money::formatCoinStacks(quest.rewardCoins, false);
        }
        else if (displayedGold > 0)
        {
            text += " | Argent +" + Money::formatEconomyUnits(displayedGold);
        }

        if (!quest.rewardMaterialId.empty() && quest.rewardMaterialQuantity > 0)
        {
            text += " | Objet : " + quest.rewardMaterialName + " x" + std::to_string(quest.rewardMaterialQuantity);
        }

        if (!quest.rewardNote.empty())
        {
            text += " | " + quest.rewardNote;
        }

        if (exactCoinCopper <= 0 && displayedGold <= 0 && quest.rewardMaterialId.empty() && quest.rewardNote.empty())
        {
            text += " | Pas de prime en pièces";
        }

        return text;
    }





    std::string countedHuntQuestLine(const Quest& quest)
    {
        return "Chasse chiffrée : " + std::to_string(quest.progress) + "/" + std::to_string(quest.target)
            + " cible" + (quest.target > 1 ? "s" : "")
            + " validée" + (quest.target > 1 ? "s" : "")
            + " pour ce rang.";
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






    // EN: isMaterialDeliveryQuest declares or implements a focused behavior used by this module.
    // FR: isMaterialDeliveryQuest déclare ou implémente un comportement précis utilisé par ce module.

    // EN: canCompleteMaterialDelivery declares or implements a focused behavior used by this module.
    // FR: canCompleteMaterialDelivery déclare ou implémente un comportement précis utilisé par ce module.

    // EN: isReadyToTurnIn declares or implements a focused behavior used by this module.
    // FR: isReadyToTurnIn déclare ou implémente un comportement précis utilisé par ce module.


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
                lines.push_back("Prime ajustée : " + Money::formatEconomyUnits(oldReward) + " -> " + Money::formatEconomyUnits(quest.rewardGold) + ".");
            }
            else
            {
                lines.push_back("Conséquence douce : pas de perte directe, mais le contact note que le service a dû être repris.");
            }
        }
        else
        {
            lines.push_back("Conséquence douce : pas de perte d'argent ni d'objet, mais le contact demandera une réponse plus propre au prochain essai.");
        }

        const int expired = player.getQuestLog().expireOverdueQuests(player.getWorldDaysElapsed());
        QuestDeadlineSupport::synchronizeQuestConsequences(player);
        if (expired > 0)
        {
            lines.push_back(std::to_string(expired) + " quête" + (expired > 1 ? "s" : "")
                + " vient d'être archivée pour délai dépassé pendant ce retard.");
        }
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

    bool hasActiveGuildQuestAtSamePlayableLocation(const QuestLog& questLog, const Quest& offeredQuest)
    {
        const std::string offeredLocation = questPlayableLocationHint(offeredQuest);
        if (offeredLocation.empty())
        {
            return false;
        }

        for (const Quest& quest : questLog.getQuests())
        {
            if (!quest.guildQuest || !quest.accepted || quest.turnedIn || quest.failed || quest.id == offeredQuest.id)
            {
                continue;
            }

            if (questPlayableLocationHint(quest) == offeredLocation)
            {
                return true;
            }
        }

        return false;
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
        label << quest.title << marker;
        if (!quest.rank.empty())
        {
            label << " | Rang : " << quest.rank;
        }
        label << " | Lieu cible : " << questPlayableLocationHint(quest)
              << " | Récompenses potentielles : " << questPotentialRewardText(quest)
              << " | Avancement : " << quest.progress << "/" << quest.target
              << " (" << questStateText(quest) << ")";
        return label.str();
    }

    std::string questBoardOfferLabel(const Quest& quest, bool samePlayableLocation)
    {
        std::ostringstream label;
        label << quest.title;
        if (!quest.rank.empty())
        {
            label << " | Rang : " << quest.rank;
        }
        label << " | Lieu cible : " << questPlayableLocationHint(quest)
              << " | Récompenses potentielles : " << questPotentialRewardText(quest)
              << " | Objectif : 0/" << std::max(1, quest.target)
              << " (Proposée";
        if (samePlayableLocation)
        {
            label << " - quête du même lieu qu'une autre quête active";
        }
        label << ")";
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

    std::vector<std::string> guildQuestDetailLines(const Player& player, const Quest& quest, int currentDay)
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
        lines.push_back(QuestLanguageSystem::requirementLine(player, quest));
        lines.push_back("Objectif : " + QuestLanguageSystem::readableObjective(player, quest));
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

    std::vector<std::string> personalQuestEstimateLines(const Player& player, const Quest& quest, int currentDay)
    {
        std::vector<std::string> lines;
        lines.push_back("Nature : demande informelle de PNJ");
        lines.push_back("Ce n'est pas un contrat officiel : le journal ne peut pas tout certifier.");
        lines.push_back("Contact : " + quest.client);
        lines.push_back("Rang supposé : " + quest.rank);
        lines.push_back("Zone/action probable : " + questPlayableLocationHint(quest));
        lines.push_back("Type supposé : " + (quest.objectiveType.empty() ? std::string("service général") : quest.objectiveType));
        lines.push_back(QuestLanguageSystem::requirementLine(player, quest));
        lines.push_back("Objectif rapporté : " + QuestLanguageSystem::readableObjective(player, quest));
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
            MessageScreen::show("INSPECTION DU CONTRAT", "quest.detail.guild", guildQuestDetailLines(player, quest, player.getWorldDaysElapsed()), true);
            return;
        }

        MessageScreen::show("ESTIMATION DE DEMANDE", "quest.detail.personal_estimate", personalQuestEstimateLines(player, quest, player.getWorldDaysElapsed()), true);
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

        const long long exactCoinCopper = Money::coinStacksValueInCopper(quest.rewardCoins);
        const int displayedGold = balancedQuestGold(quest);
        if (exactCoinCopper > 0)
        {
            lines.push_back("Pièces reçues exactement comme annoncées : " + Money::formatCoinStacks(quest.rewardCoins, false));
        }
        else if (displayedGold > 0)
        {
            lines.push_back("Argent gagné : " + Money::formatEconomyUnits(displayedGold));
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



}

namespace QuestMenuInternalSupport
{
    std::vector<std::string> clientQuestAcceptedDialogueLines(const Player& player, const Quest& quest)
    {
        return ::clientQuestAcceptedDialogueLines(player, quest);
    }

    std::string toLowerChoiceText(std::string text)
    {
        return ::toLowerChoiceText(std::move(text));
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
        return ::runTrackedExplorationWave(player, random, difficulty, deathRule, monsters, context);
    }

    int askChoiceScreen(
        const std::string& title,
        const std::string& screenId,
        const std::vector<std::string>& lines,
        const std::vector<std::pair<int, std::string>>& options,
        int minChoice,
        int maxChoice,
        const std::string& invalidMessage
    )
    {
        return ::askChoiceScreen(title, screenId, lines, options, minChoice, maxChoice, invalidMessage);
    }

    void showExplorationNotice(
        const std::string& title,
        const std::string& screenId,
        const std::vector<std::string>& lines,
        bool waitAndClear
    )
    {
        ::showExplorationNotice(title, screenId, lines, waitAndClear);
    }

    void prepareQuestForAcceptance(Quest& quest, int currentDay)
    {
        ::prepareQuestForAcceptance(quest, currentDay);
    }

    void appendDeadlineLine(std::vector<std::string>& lines, const Quest& quest, int currentDay)
    {
        ::appendDeadlineLine(lines, quest, currentDay);
    }

    int askQuestOfferDecision(
        const std::string& title,
        const std::string& screenId,
        const Player& player,
        const Quest& quest,
        const std::vector<std::string>& introLines
    )
    {
        return ::askQuestOfferDecision(title, screenId, player, quest, introLines);
    }
}

// EN: openQuestHub declares or implements a focused behavior used by this module.
// FR: openQuestHub déclare ou implémente un comportement précis utilisé par ce module.
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
                "Tu peux maintenant accéder au panneau de quêtes officiel et accepter les demandes de vendeurs liées à la guilde."
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

static void openGuildCurrencyExchange(Player& player)
{
    auto coinLabel = [](CoinType type) {
        return Money::coinName(type) + " (" + Money::coinAbbreviation(type) + ")";
    };
    const std::vector<CoinType> allTypes = {
        CoinType::Copper,
        CoinType::Iron,
        CoinType::Electrum,
        CoinType::Gold,
        CoinType::Platinum
    };

    auto conversionTerms = [](CoinType source, CoinType target) {
        const long long sourceValue = Money::coinValueInCopper(source);
        const long long targetValue = Money::coinValueInCopper(target);
        const long long sourcePerLot = sourceValue < targetValue ? targetValue / sourceValue : 1;
        const long long targetPerLot = sourceValue > targetValue ? sourceValue / targetValue : 1;
        return std::pair<long long, long long>{sourcePerLot, targetPerLot};
    };

    while (true)
    {
        MenuScreen screen("BUREAU DE CHANGE DE LA GUILDE", "quest.guild.currency_exchange");
        screen.addSubtitle("Organiser volontairement les cinq piles physiques de ta bourse");
        screen.addLine("Bourse actuelle : " + player.getInventory().getWalletLine() + ".");
        screen.addLine("Valeur totale exacte : " + player.getInventory().getWalletTotalLine() + ".");
        screen.addLine(Money::coinScaleText() + ".");
        screen.addLine("Le change ne crée ni ne détruit d'argent. Il modifie seulement les pièces physiques que tu portes.");
        screen.addLine("La forme de la bourse peut aussi changer l'impression sociale qu'elle donne : beaucoup de cuivre et une pièce d'or ne racontent pas la même chose.");
        screen.addBackOption("Retour à la guilde", "quest.guild.currency_exchange.back");
        screen.addOption(1, "Conversion personnalisée", "Choisir la pièce de départ, la pièce d'arrivée, puis le nombre de conversions.", true, "quest.guild.currency_exchange.custom");
        screen.addOption(2, "Tout vers les pièces les plus élevées", "Créer une bourse compacte : PP d'abord, puis PO, PE, PF et le reste exact en PC.", player.getInventory().getTotalCopper() > 0, "quest.guild.currency_exchange.compact");
        screen.addOption(3, "Tout vers la pièce la plus faible", "Transformer toute la valeur en PC physiques. Très précis, très encombrant, et socialement assez modeste.", player.getInventory().getTotalCopper() > 0, "quest.guild.currency_exchange.copper_all");

        const int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choisis une opération de change.");
        Console::clear();
        if (choice == 0) return;

        if (choice == 2 || choice == 3)
        {
            const long long before = player.getInventory().getTotalCopper();
            const std::string oldWallet = player.getInventory().getWalletLine();
            const bool success = choice == 2
                ? player.getInventory().compactCoinsToHighest()
                : player.getInventory().flattenCoinsToCopper();
            if (success)
            {
                player.refreshCurrencyTitles();
            }
            const long long after = player.getInventory().getTotalCopper();

            MessageScreen::show(
                success && before == after ? "CHANGE EFFECTUÉ" : "ERREUR DE CHANGE",
                choice == 2 ? "quest.guild.currency_exchange.compact.done" : "quest.guild.currency_exchange.copper_all.done",
                {
                    choice == 2
                        ? "La guilde regroupe chaque tranche possible vers la pièce la plus élevée sans perdre le moindre PC."
                        : "La guilde casse toute la bourse jusqu'au cuivre : chaque PC devient une pièce physique.",
                    "Avant : " + oldWallet + ".",
                    "Après : " + player.getInventory().getWalletLine() + ".",
                    "Valeur exacte : " + player.getInventory().getWalletTotalLine() + ".",
                    "Valeur conservée : " + std::string(before == after ? "oui" : "ERREUR") + "."
                },
                false
            );
            Console::clear();
            continue;
        }

        if (choice != 1)
        {
            continue;
        }

        MenuScreen sourceScreen("CONVERSION PERSONNALISÉE — SOURCE", "quest.guild.currency_exchange.custom.source");
        sourceScreen.addLine("Choisis la pile physique que tu veux donner au guichet.");
        sourceScreen.addBackOption("Retour", "quest.guild.currency_exchange.custom.source.back");
        for (std::size_t i = 0; i < allTypes.size(); ++i)
        {
            const CoinType type = allTypes[i];
            const long long count = player.getInventory().getCoinCount(type);
            sourceScreen.addOption(
                static_cast<int>(i + 1),
                coinLabel(type),
                "En bourse : " + std::to_string(count) + " " + Money::coinAbbreviation(type) + ".",
                count > 0,
                "quest.guild.currency_exchange.custom.source." + Money::coinAbbreviation(type)
            );
        }

        const int sourceChoice = TerminalInterface::askMenuChoiceFromOptions(sourceScreen, "Choisis une pile présente dans ta bourse.");
        Console::clear();
        if (sourceChoice <= 0 || static_cast<std::size_t>(sourceChoice) > allTypes.size()) continue;
        const CoinType sourceType = allTypes[static_cast<std::size_t>(sourceChoice - 1)];
        const long long sourceCount = player.getInventory().getCoinCount(sourceType);

        MenuScreen targetScreen("CONVERSION PERSONNALISÉE — DESTINATION", "quest.guild.currency_exchange.custom.target");
        targetScreen.addLine("Source : " + coinLabel(sourceType) + " | disponible : " + std::to_string(sourceCount) + " " + Money::coinAbbreviation(sourceType) + ".");
        targetScreen.addLine("Choisis maintenant la pièce physique que tu veux recevoir.");
        targetScreen.addBackOption("Retour", "quest.guild.currency_exchange.custom.target.back");

        int targetOption = 1;
        std::vector<CoinType> targetTypes;
        for (CoinType targetType : allTypes)
        {
            if (targetType == sourceType)
            {
                continue;
            }

            const auto [sourcePerLot, targetPerLot] = conversionTerms(sourceType, targetType);
            const long long maxLots = sourcePerLot > 0 ? sourceCount / sourcePerLot : 0;
            targetTypes.push_back(targetType);
            targetScreen.addOption(
                targetOption++,
                coinLabel(sourceType) + " -> " + coinLabel(targetType),
                "1 conversion : " + std::to_string(sourcePerLot) + " " + Money::coinAbbreviation(sourceType)
                    + " -> " + std::to_string(targetPerLot) + " " + Money::coinAbbreviation(targetType)
                    + " | maximum possible : " + std::to_string(maxLots) + ".",
                maxLots > 0,
                "quest.guild.currency_exchange.custom.target." + Money::coinAbbreviation(targetType)
            );
        }

        const int targetChoice = TerminalInterface::askMenuChoiceFromOptions(targetScreen, "Choisis une destination possible.");
        Console::clear();
        if (targetChoice <= 0 || static_cast<std::size_t>(targetChoice) > targetTypes.size()) continue;
        const CoinType targetType = targetTypes[static_cast<std::size_t>(targetChoice - 1)];
        const auto [sourcePerLot, targetPerLot] = conversionTerms(sourceType, targetType);
        const long long maxLots = sourceCount / sourcePerLot;
        const int maxQuantity = static_cast<int>(std::min(maxLots, 1000000LL));
        if (maxQuantity <= 0) continue;

        const int lotCount = MessageScreen::askQuantity(
            "NOMBRE DE CONVERSIONS",
            "quest.guild.currency_exchange.custom.quantity",
            {
                "Conversion choisie : " + std::to_string(sourcePerLot) + " " + Money::coinAbbreviation(sourceType)
                    + " -> " + std::to_string(targetPerLot) + " " + Money::coinAbbreviation(targetType) + ".",
                "Pièces source disponibles : " + std::to_string(sourceCount) + " " + Money::coinAbbreviation(sourceType) + ".",
                "Maximum possible : " + std::to_string(maxLots) + " conversion(s).",
                "Pour le maximum, le guichet utiliserait " + std::to_string(sourcePerLot * maxLots) + " " + Money::coinAbbreviation(sourceType)
                    + " et remettrait " + std::to_string(targetPerLot * maxLots) + " " + Money::coinAbbreviation(targetType) + "."
            },
            1,
            maxQuantity,
            "Le nombre doit rester entre 1 et le maximum convertible affiché."
        );
        Console::clear();

        const long long before = player.getInventory().getTotalCopper();
        if (player.getInventory().convertCoinLots(sourceType, targetType, lotCount))
        {
            player.refreshCurrencyTitles();
            const long long after = player.getInventory().getTotalCopper();
            MessageScreen::show(
                "CHANGE EFFECTUÉ",
                "quest.guild.currency_exchange.custom.done",
                {
                    std::to_string(sourcePerLot * static_cast<long long>(lotCount)) + " " + Money::coinAbbreviation(sourceType)
                        + " échangé(s) contre " + std::to_string(targetPerLot * static_cast<long long>(lotCount)) + " " + Money::coinAbbreviation(targetType) + ".",
                    "Bourse : " + player.getInventory().getWalletLine() + ".",
                    "Valeur exacte : " + player.getInventory().getWalletTotalLine() + ".",
                    "Valeur conservée : " + std::string(before == after ? "oui" : "ERREUR") + "."
                },
                false
            );
            Console::clear();
        }
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
        screen.addLine("Contrats officiels et défis utilisent deux limites séparées.");
        screen.addLine("Défis actifs : " + std::to_string(player.getQuestLog().getActiveGuildChallengeCount()) + "/3.");
        screen.addLine("Contrats de guilde : " + clientQuestStatusText(guildCounts));
        screen.addLine("Services de guilde à traiter au comptoir : " + std::to_string(activeServiceCount) + ".");
        if (!player.hasTitle("Témoin du marchand bleu")
            && heroVillagerProgressGateIsOpen(player)
            && player.getWorldDaysElapsed() % 5 == 0)
        {
            screen.addLine("Rumeur tardive : un marchand très musclé en t-shirt bleu-vert, pantalon violet et armure de diamant bleu apparaîtrait sur certaines routes avant de disparaître sans laisser de traces.");
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
        screen.addOption(7, "Bureau de change",
            !guildOpen ? guildClosedReason : "Répartir librement les mêmes fonds entre PC, PF, PE, PO et PP, sans frais.",
            guildOpen,
            "quest.guild.currency_exchange"
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
        else if (choice == 7)
        {
            openGuildCurrencyExchange(player);
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
        screen.addLine("Récompense spéciale : Marques de défi, avec un peu d'expérience et d'argent seulement.");
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
                "Le registre validera cette réussite quand le combat ou l'action concernée aura vraiment eu lieu."
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
    if (const City* currentGuildCity = City::findById(player.getCurrentCityId()))
    {
        questLog.prioritizeGuildBoardForCity(*currentGuildCity);
    }

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

        screen.addLine("Note : une fiche marquée même lieu n'est pas en cours, elle partage seulement une zone avec une quête active.");

        PagedMenu::addNavigationOptions(screen, pageIndex, totalPages);

        for (std::size_t i = first; i < last; ++i)
        {
            const bool alreadyTaken = questLog.hasQuest(board[i].id);
            const bool rankAllowed = isGuildQuestRankAllowedForStanding(board[i], standing);
            const bool samePlayableLocation = !alreadyTaken && hasActiveGuildQuestAtSamePlayableLocation(questLog, board[i]);
            const std::string label = questBoardOfferLabel(board[i], samePlayableLocation);

            MenuOptionItemData itemData;
            itemData.structured = true;
            itemData.kind = "quest";
            itemData.section = "Panneau de guilde";
            itemData.actionType = "quest";
            itemData.name = board[i].title;
            itemData.detail = samePlayableLocation
                ? "Quête du même lieu qu'une autre quête active : ce n'est pas un contrat déjà en cours."
                : "Contrat proposé par la guilde.";
            itemData.status = alreadyTaken
                ? "Déjà prise"
                : (!rankAllowed ? "Rang/pastille insuffisant" : (samePlayableLocation ? "Quête du même lieu qu'une autre quête active" : "Disponible"));
            itemData.reward = questRewardText(board[i]);
            itemData.progress = "Rang " + (board[i].rank.empty() ? std::string("?") : board[i].rank) + " | Offre proposée";
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

        if (choice < 1 || choice > static_cast<int>(board.size()))
        {
            MessageScreen::show(
                "CHOIX INVALIDE",
                "quest.guild.board.invalid_choice",
                {"Choisis une fiche de quête affichée ou utilise les options de navigation."},
                false
            );
            continue;
        }

        const std::size_t selectedIndex = static_cast<std::size_t>(choice - 1);
        if (selectedIndex < first || selectedIndex >= last)
        {
            MessageScreen::show(
                "FICHE NON AFFICHÉE",
                "quest.guild.board.not_on_page",
                {"Pour éviter les erreurs, choisis une quête visible sur la page actuelle."},
                false
            );
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

    if (!QuestLanguageSystem::canRead(player, selectedQuest))
    {
        MessageScreen::show(
            "ANNEXE NON COMPRISE",
            "quest.guild.board.language_blocked",
            {
                QuestLanguageSystem::requirementLine(player, selectedQuest),
                "Extrait : " + QuestLanguageSystem::readableObjective(player, selectedQuest),
                "La guilde refuse de te faire signer un contrat dont tu ne peux pas vérifier l'annexe.",
                "Consulte la bibliothèque, apprends la langue, puis reviens : la fiche restera disponible tant qu'elle n'expire pas."
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
        const NpcPropagationResult localPropagation = NpcInformationPropagationSystem::propagateOneLocalFact(player, clientName);
        const NpcPropagationResult intercityPropagation = localPropagation.transferred
            ? NpcPropagationResult{}
            : NpcInformationPropagationSystem::propagateOneIntercityFact(player, clientName);
        MenuScreen screen(clientName, "quest.client");
        addClientQuestSummaryLines(screen, player, clientName);
        for (const std::string& introLine : NpcKnowledgeSystem::spontaneousIntroLines(player, clientName))
        {
            screen.addLine(introLine);
        }
        if (localPropagation.transferred)
        {
            screen.addLine("Rumeur locale reçue : " + localPropagation.sourceNpc + " a réellement transmis une information à ce contact.");
        }
        else if (intercityPropagation.transferred)
        {
            screen.addLine("Information arrivée d'une autre ville : " + intercityPropagation.sourceNpc
                + " -> " + intercityPropagation.relayChannel + ". Le trajet réel explique pourquoi elle n'était pas disponible plus tôt.");
        }
        const std::vector<std::string> npcMemoryLines = NpcKnowledgeSystem::conversationMemoryLines(player, clientName, 2);
        for (const std::string& memoryLine : npcMemoryLines)
        {
            screen.addLine(memoryLine);
        }
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

        const bool hasShareableFact = NpcKnowledgeSystem::hasShareableRecentFact(player);
        screen.addOption(
            5,
            "Partager un fait récent",
            hasShareableFact
                ? "Raconter au PNJ le fait récent le plus pertinent. Il sera mémorisé comme ton témoignage, pas comme une vérité omnisciente."
                : "Aucun fait récent pertinent à transmettre pour le moment.",
            hasShareableFact,
            "quest.client.share_fact"
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

        if (choice == 5)
        {
            MessageScreen::show(
                "INFORMATION TRANSMISE",
                "quest.client.share_fact.result",
                NpcKnowledgeSystem::shareMostRecentFact(player, clientName),
                false
            );
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
                player.rememberNpcFact(
                    clientName,
                    "quest_accepted",
                    acceptedQuest.id,
                    "Demande acceptée : " + acceptedQuest.title,
                    "interaction_directe",
                    player.getName(),
                    100,
                    3
                );
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
            player.rememberNpcFact(
                clientName,
                "quest_declined",
                offeredQuest.id,
                "Demande déclinée : " + offeredQuest.title,
                "interaction_directe",
                player.getName(),
                100,
                3
            );
            MessageScreen::show(
                "DEMANDE REFUSÉE",
                "quest.client.offer.declined",
                {"Tu refuses la demande pour l'instant.", "Le contact retiendra surtout que tu n'as pas pris l'engagement, pas que tu l'as rompu."}
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
            && heroVillagerProgressGateIsOpen(player)
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

namespace QuestMenuInternalSupport
{
    bool maybeTriggerLegendaryMerchantEncounter(
        Player& player,
        Random& random,
        const std::string& biomeName
    )
    {
        return ::maybeTriggerLegendaryMerchantEncounter(player, random, biomeName);
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
        player.rememberNpcFact(
            clientName,
            "quest_completed",
            quest.id,
            "Service confirmé : " + quest.title,
            "interaction_directe",
            player.getName(),
            100,
            3
        );
        player.recordPnjServed(quest.client.empty() ? std::string("Contact inconnu") : quest.client);
        player.recordQuestTypeCompleted(questKindText(quest));
        player.recordCanonicalEvent("quetes_reussies", quest.id, quest.title);
        player.gainExperience(balancedQuestExperience(quest));
        if (Money::coinStacksValueInCopper(quest.rewardCoins) > 0)
        {
            player.getInventory().earnCoinStacks(quest.rewardCoins);
            player.refreshCurrencyTitles();
        }
        else
        {
            player.getInventory().earnEconomyUnits(balancedQuestGold(quest));
            player.refreshCurrencyTitles();
        }
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
        resultLines.push_back("Argent actuel : " + player.getInventory().getWalletLine());
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
