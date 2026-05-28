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
#include "economy/shop/ShopTransactionSystem.hpp"
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
#include <set>
#include <utility>
#include <sstream>

namespace
{
    std::string questStateText(const Quest& quest)
    {
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

    std::string questRewardText(const Quest& quest)
    {
        std::string text = "XP +" + std::to_string(quest.rewardExperience);

        if (quest.rewardGold > 0)
        {
            text += " | Or +" + std::to_string(quest.rewardGold);
        }

        if (!quest.rewardMaterialId.empty() && quest.rewardMaterialQuantity > 0)
        {
            text += " | Objet : " + quest.rewardMaterialName + " x" + std::to_string(quest.rewardMaterialQuantity);
        }

        if (!quest.rewardNote.empty())
        {
            text += " | " + quest.rewardNote;
        }

        if (quest.rewardGold <= 0 && quest.rewardMaterialId.empty() && quest.rewardNote.empty())
        {
            text += " | Pas de prime en or";
        }

        return text;
    }


    std::vector<std::string> guildQuestAcceptedDialogueLines(const Quest& quest)
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

        if (!quest.location.empty())
        {
            lines.push_back("Zone annoncée : " + quest.location + ". La gérante précise que la carte donne une direction, pas une promesse de sécurité.");
        }

        if (!quest.targetFamily.empty())
        {
            lines.push_back("Famille ciblée : " + quest.targetFamily + ". Elle recommande de noter ce qui est observé avant de tout régler à coups de panique.");
        }

        if (quest.rank == "S" || quest.rank == "SS" || quest.rank == "SSS" || quest.rank == "Légende" || quest.rank == "Dieu")
        {
            lines.push_back("Avant de te laisser partir, elle ajoute que ce rang n'est pas une décoration : c'est une manière polie de prévenir les inconscients.");
        }

        return lines;
    }

    std::vector<std::string> clientQuestAcceptedDialogueLines(const Quest& quest)
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

        if (!quest.location.empty())
        {
            lines.push_back("Lieu évoqué : " + quest.location + ". Le client n'est pas certain de tout, mais il sait exactement où la peur a commencé.");
        }

        if (!quest.targetFamily.empty())
        {
            lines.push_back("Indice donné : la demande semble liée à " + quest.targetFamily + ", sans garantie officielle tant que la guilde n'a rien vérifié.");
        }

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
        const std::vector<Monster>& monsters,
        const std::string& context
    )
    {
        player.recordCombatStarted();
        ShopTransactionSystem::clearBuybackAfterCombat();
        return MonsterPveMode::runExplorationWave(player, random, difficulty, monsters, context);
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
                screenId + ".choice." + std::to_string(option.first)
            );
        }

        return TerminalInterface::askMenuChoice(screen, minChoice, maxChoice, invalidMessage);
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
    bool isReadyToTurnIn(const Player& player, const Quest& quest)
    {
        return quest.completed || canCompleteMaterialDelivery(player, quest);
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

    std::string approximateQuestRewardText(const Quest& quest)
    {
        if (quest.guildQuest)
        {
            return questRewardText(quest);
        }

        std::string text = "Récompense probable : ";

        if (quest.rewardGold > 0)
        {
            if (quest.rewardGold < 40) text += "petite compensation";
            else if (quest.rewardGold < 120) text += "paiement correct";
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

        if (quest.rewardExperience > 0)
        {
            text += " | Expérience estimée : ";
            if (quest.rewardExperience < 80) text += "faible";
            else if (quest.rewardExperience < 220) text += "moyenne";
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
                  << " | Client : " << quest.client
                  << " | Lieu : " << quest.location
                  << " | Objectif : " << quest.objective
                  << " | Progression : " << quest.progress << "/" << quest.target
                  << " | État : " << questStateText(quest)
                  << " | Récompenses : " << questRewardText(quest);
        }
        else
        {
            label << "[Demande PNJ - Rang estimé " << quest.rank << "] " << quest.title
                  << " | Contact : " << quest.client
                  << " | Zone supposée : " << quest.location
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

    std::vector<std::string> guildQuestDetailLines(const Quest& quest)
    {
        std::vector<std::string> lines;
        lines.push_back("Nature : contrat officiel de guilde");
        lines.push_back("Titre : " + quest.title);
        lines.push_back("Origine : " + quest.origin);
        lines.push_back("Client : " + quest.client);
        lines.push_back("Rang : " + quest.rank);
        lines.push_back("Lieu : " + quest.location);
        lines.push_back("Type : " + (quest.objectiveType.empty() ? std::string("général") : quest.objectiveType));
        lines.push_back("Cible : " + (quest.targetFamily.empty() ? std::string("générale") : quest.targetFamily));
        lines.push_back("Objectif : " + quest.objective);
        lines.push_back("Progression : " + std::to_string(quest.progress) + "/" + std::to_string(quest.target));
        lines.push_back("État : " + questStateText(quest));
        lines.push_back("Récompenses : " + questRewardText(quest));

        if (!quest.requiredMaterialId.empty() && quest.requiredMaterialQuantity > 0)
        {
            lines.push_back("Livraison demandée : " + quest.requiredMaterialName + " x" + std::to_string(quest.requiredMaterialQuantity));
        }

        lines.push_back("Lecture : la guilde a assez cadré ce contrat pour que les informations soient fiables.");
        return lines;
    }

    std::vector<std::string> personalQuestEstimateLines(const Quest& quest)
    {
        std::vector<std::string> lines;
        lines.push_back("Nature : demande informelle de PNJ");
        lines.push_back("Ce n'est pas un contrat officiel : le journal ne peut pas tout certifier.");
        lines.push_back("Contact : " + quest.client);
        lines.push_back("Rang supposé : " + quest.rank);
        lines.push_back("Zone probable : " + (quest.location.empty() ? std::string("à confirmer sur le terrain") : quest.location));
        lines.push_back("Type supposé : " + (quest.objectiveType.empty() ? std::string("service général") : quest.objectiveType));
        lines.push_back("Objectif rapporté : " + quest.objective);
        lines.push_back("Avancée notée : " + std::to_string(quest.progress) + "/" + std::to_string(quest.target));
        lines.push_back("État : " + questStateText(quest));
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

    void showQuestDetail(const Quest& quest)
    {
        if (quest.guildQuest)
        {
            MessageScreen::show("INSPECTION DU CONTRAT", "quest.detail.guild", guildQuestDetailLines(quest), true);
            return;
        }

        MessageScreen::show("ESTIMATION DE DEMANDE", "quest.detail.personal_estimate", personalQuestEstimateLines(quest), true);
    }

    std::string questRequiredMaterialStatusLine(const Player& player, const Quest& quest)
    {
        if (quest.requiredMaterialId.empty() || quest.requiredMaterialQuantity <= 0)
        {
            return "";
        }

        return "Matériaux à rapporter : " + quest.requiredMaterialName
            + " x" + std::to_string(quest.requiredMaterialQuantity)
            + " (possédé : " + std::to_string(player.getInventory().countMaterialById(quest.requiredMaterialId))
            + ", équiv. normale : " + std::to_string(player.getInventory().countMaterialQualityPointsById(quest.requiredMaterialId) / 2)
            + ")";
    }

    void appendQuestRewardResultLines(std::vector<std::string>& lines, const Quest& quest)
    {
        lines.push_back("Quête validée : " + quest.title);
        lines.push_back("XP gagnée : " + std::to_string(quest.rewardExperience));

        if (quest.rewardGold > 0)
        {
            lines.push_back("Or gagné : " + std::to_string(quest.rewardGold) + " pièces");
        }
        else
        {
            lines.push_back("Prime en or : aucune");
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
                return "Demandes qui réclament des matériaux ou objets précis.";
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
                return !quest.turnedIn;
            case QuestJournalFilter::ReadyToTurnIn:
                return !quest.turnedIn && isReadyToTurnIn(player, quest);
            case QuestJournalFilter::Guild:
                return !quest.turnedIn && quest.guildQuest;
            case QuestJournalFilter::Personal:
                return !quest.turnedIn && isPersonalNpcQuest(quest);
            case QuestJournalFilter::Combat:
                return !quest.turnedIn && quest.objectiveType == "combat";
            case QuestJournalFilter::Exploration:
                return !quest.turnedIn && (quest.objectiveType == "exploration" || quest.objectiveType == "bestiaire");
            case QuestJournalFilter::Delivery:
                return !quest.turnedIn && isMaterialDeliveryQuest(quest);
            case QuestJournalFilter::TurnedIn:
                return quest.turnedIn;
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

            if (quest.turnedIn)
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
            if (quest.turnedIn || !isReadyToTurnIn(player, quest))
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
                const bool leftReady = !left->turnedIn && isReadyToTurnIn(player, *left);
                const bool rightReady = !right->turnedIn && isReadyToTurnIn(player, *right);

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
                    itemData.reward = quest.guildQuest ? questRewardText(quest) : approximateQuestRewardText(quest);
                    itemData.progress = std::to_string(quest.progress) + "/" + std::to_string(quest.target);
                    itemData.owner = quest.client;
                    itemData.important = !quest.turnedIn && isReadyToTurnIn(player, quest);

                    screen.addOption(
                        static_cast<int>(10 + (i - first)),
                        questCardLabel(quest),
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
                showQuestDetail(*relatedQuests[first + static_cast<std::size_t>(localQuestIndex)]);
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
        screen.addLine((quest.guildQuest ? "Lieu : " : "Zone probable : ") + quest.location);
        screen.addLine((quest.guildQuest ? "Objectif : " : "Objectif raconté : ") + quest.objective);
        screen.addLine((quest.guildQuest ? "Récompenses : " : "Estimation : ") + (quest.guildQuest ? questRewardText(quest) : approximateQuestRewardText(quest)));

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

        return TerminalInterface::askMenuChoice(screen, 0, 1, "Choix invalide.");
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
            screen.addLine(quest.guildQuest ? "Lieu : " + quest.location : "Zone supposée : " + quest.location);
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

            int choice = TerminalInterface::askMenuChoice(screen, 0, 2, "Choix invalide.");
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
                showQuestDetail(quest);
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
    };

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

    bool questTextMentionsBiome(const Quest& quest, const std::string& biomeName)
    {
        return textContainsInsensitive(quest.location, biomeName)
            || textContainsInsensitive(quest.targetFamily, biomeName)
            || textContainsInsensitive(quest.objective, biomeName);
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
        if (quest.turnedIn || quest.completed || !quest.accepted)
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
        if (biome.name == "Cimetière oublié") return evolved ? "Ombre de nom perdu" : "Veilleur sans sépulture";
        if (biome.name == "Plaine sauvage") return evolved ? "Alpha aux crocs longs" : "Bête territoriale";
        return evolved ? "Créature évoluée locale" : "Menace locale isolée";
    }

    std::string miniBossQuestFamilyForBiome(const ExplorationBiome& biome, bool evolved)
    {
        if (evolved) return "Mini-boss / menace évoluée";
        if (biome.name == "Route commerciale") return "Humanoïdes / embuscades";
        if (biome.name == "Cimetière oublié") return "Morts-vivants / ombres";
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

        std::vector<std::string> biomes = {"Forêt ancienne", "Mares gélatineuses", "Montagne froide", "Marais trouble", "Route commerciale", "Ruines effondrées", "Plaine sauvage"};
        return biomes[random.between(0, static_cast<int>(biomes.size()) - 1)];
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
        itemData.status = "Danger : " + std::to_string(intensity.eventShift) + "%";
        itemData.reward = "Or direct : " + std::to_string(intensity.goldPercent) + "%";
        itemData.progress = "Bonus trouvailles : " + std::to_string(intensity.quantityBonus)
            + " | Prudence : " + std::to_string(intensity.carefulBonus);
        itemData.important = intensity.eventShift > 0 || intensity.quantityBonus > 0 || intensity.goldPercent > 100;
        return itemData;
    }

    // EN: progressExplorationQuests declares or implements a focused behavior used by this module.
    // FR: progressExplorationQuests déclare ou implémente un comportement précis utilisé par ce module.
    int progressExplorationQuests(Player& player, const std::string& biomeName, int amount)
    {
        int updated = 0;
        std::string biome = lowerCopy(biomeName);

        for (Quest& quest : player.getQuestLog().getQuests())
        {
            if (!quest.accepted || quest.completed || quest.turnedIn)
            {
                continue;
            }

            if (quest.objectiveType != "exploration" && quest.objectiveType != "bestiaire")
            {
                continue;
            }

            std::string target = lowerCopy(quest.targetFamily);
            std::string location = lowerCopy(quest.location);

            if (!target.empty() && target.find(biome) == std::string::npos && biome.find(target) == std::string::npos
                && !location.empty() && location.find(biome) == std::string::npos)
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

    std::string explorationEventLabelFromRoll(int roll)
    {
        if (roll <= 24) return "récolte de terrain";
        if (roll <= 37) return "trace intéressante";
        if (roll <= 48) return "petit trésor";
        if (roll <= 54) return "fausses pièces";
        if (roll <= 66) return "coffre suspect";
        if (roll <= 78) return "rencontre imprévue";
        if (roll <= 87) return "mini-boss d'exploration";
        if (roll <= 91) return "demande de PNJ";
        if (roll <= 96) return "événement actif de biome";
        if (roll <= 98) return "lieu dangereux";
        return "découverte rare";
    }

    void showExplorationRunSummary(
        const Player& player,
        const ExplorationBiome& biome,
        const ExplorationIntensity& intensity,
        const std::string& eventLabel,
        int hpBefore,
        int goldBefore,
        int readyBefore
    )
    {
        const int hpAfter = player.getHp();
        const int goldAfter = player.getInventory().getGold();
        const int readyAfter = countQuestsForJournalFilter(player, QuestJournalFilter::ReadyToTurnIn);

        std::vector<std::string> lines = {
            "Zone : " + biome.name + ".",
            "Approche : " + intensity.name + ".",
            "Événement principal : " + eventLabel + ".",
            "PV : " + std::to_string(hpBefore) + " -> " + std::to_string(hpAfter)
                + " / " + std::to_string(player.getMaxHp()) + ".",
            "Or : " + std::to_string(goldBefore) + " -> " + std::to_string(goldAfter)
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
    void simulateUnexpectedExplorationFight(Player& player, Random& random, const ExplorationBiome& biome, const ExplorationIntensity& intensity, DifficultyMode difficulty)
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
    void openExplorationChest(Player& player, Random& random, const ExplorationBiome& biome, const ExplorationIntensity& intensity, DifficultyMode difficulty)
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
                showExplorationNotice(
                    "PIÈGE",
                    "exploration.chest.trap",
                    {
                        "Un mécanisme claque.",
                        "Tu prends " + std::to_string(damage) + " dégâts, mais tu restes debout."
                    }
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
            simulateUnexpectedExplorationFight(player, random, biome, intensity, difficulty);
            int gold = applyExplorationGoldReward(random.between(8, 24 + player.getLevel() * 2), player, intensity, difficulty, 1);
            player.getInventory().earnGold(gold);
            showExplorationNotice(
                "RESTES DU MIMIC",
                "exploration.chest.mimic.reward",
                {"Dans les restes visqueux, tu récupères " + std::to_string(gold) + " pièces."}
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
                "Or gagné : " + std::to_string(gold),
                addExplorationMaterial(player, biome.commonMaterialId, applyExplorationQuantityBonus(1, intensity), chooseExplorationQuality(random, true))
            };
            showExplorationNotice("COFFRE MODESTE", "exploration.chest.modest", rewardLines);
        }
        else
        {
            int gold = applyExplorationGoldReward(random.between(35 + player.getLevel() * 3, 90 + player.getLevel() * 8), player, intensity, difficulty, 2);
            player.getInventory().earnGold(gold);
            std::vector<std::string> rewardLines = {
                "Le coffre est réel, et pour une fois il n'a pas décidé de te mordre.",
                "Or gagné : " + std::to_string(gold),
                addExplorationMaterial(player, biome.rareMaterialId, applyExplorationQuantityBonus(1, intensity), chooseExplorationQuality(random, true))
            };
            showExplorationNotice("COFFRE INTACT", "exploration.chest.good", rewardLines);
        }
    }

    // EN: triggerRareExplorationDiscovery declares or implements a focused behavior used by this module.
    // FR: triggerRareExplorationDiscovery déclare ou implémente un comportement précis utilisé par ce module.
    void triggerRareExplorationDiscovery(Player& player, Random& random, const ExplorationBiome& biome, const ExplorationIntensity& intensity, DifficultyMode difficulty)
    {
        int roll = random.between(1, 100);

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
                "Or gagné : " + std::to_string(gold),
                addExplorationMaterial(player, biome.rareMaterialId, applyExplorationQuantityBonus(1, intensity), chooseExplorationQuality(random, true))
            };
            appendExplorationQuestProgressLine(player, biome, 1, lines, "Le journal d'exploration progresse grâce à cette cache");
            showExplorationNotice("CACHE ANCIENNE", "exploration.rare.discovery.cache", lines);
            return;
        }

        if (roll <= 58)
        {
            std::vector<std::string> lines = {
                "Tu trouves des traces parfaitement conservées.",
                "Elles ne donnent pas un objet immédiat, mais elles valent beaucoup pour les quêtes et le registre."
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
                    "Le registre ne grave aucun nom complet, mais ses pages tremblent comme devant une présence éveillée.",
                    "Trace perçue : " + bossTraceForBiome(biome) + "."
                }
            );

            bool newEntityDetected = player.unlockNextBossVariation();
            std::vector<std::string> registryLines;
            if (newEntityDetected)
            {
                registryLines = {
                    "Une nouvelle entité a été détectée dans le registre des variations d'énergie anormale.",
                    "Nom : ???",
                    "Statut : repérée par exploration rare."
                };
            }
            else
            {
                registryLines = {"Le registre garde la trace, mais aucune nouvelle entrée ne se stabilise."};
            }
            appendExplorationQuestProgressLine(
                player,
                biome,
                2,
                registryLines,
                "La trace de boss fait progresser les notes d'exploration"
            );
            showExplorationNotice(
                "REGISTRE DES BOSS",
                newEntityDetected ? "exploration.rare.discovery.boss_trace.new" : "exploration.rare.discovery.boss_trace.old",
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
                "Villageois nerveux", "Marchand inquiet", "Forgeron", "Alchimiste", "Vendeur de composants",
                "Vendeur de matériaux", "Herboriste", "Armurier", "Vendeur d'armes", "Vendeur de consommables", "Bibliothécaire"
            };

            std::string client = clients[std::clamp(roll, 1, 11) - 1];
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
            if (player.getQuestLog().addQuest(offeredQuest))
            {
                player.getQuestLog().refreshMaterialDeliveryQuests(player.getInventory());
                std::vector<std::string> lines = {"Demande ajoutée au journal : " + offeredQuest.title};
                std::vector<std::string> dialogue = clientQuestAcceptedDialogueLines(offeredQuest);
                lines.insert(lines.end(), dialogue.begin(), dialogue.end());
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
        Quest offeredQuest = buildNpcQuestByRoll(player, random.between(1, 11), intro, biome.name);
        displayQuestOffer(player, offeredQuest, intro);
    }

    // EN: simulateExplorationMiniBoss declares or implements a focused behavior used by this module.
    // FR: simulateExplorationMiniBoss déclare ou implémente un comportement précis utilisé par ce module.
    void simulateExplorationMiniBoss(Player& player, Random& random, const ExplorationBiome& biome, const ExplorationIntensity& intensity, DifficultyMode difficulty)
    {
        bool evolved = random.between(1, 100) <= 45;
        std::string miniBossName = miniBossNameForBiome(biome, evolved);
        std::string questFamily = miniBossQuestFamilyForBiome(biome, evolved);

        Monster miniBoss = createExplorationMonsterForBiome(player, random, biome, intensity);

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
    void simulateAfterCombatMiniBoss(Player& player, Random& random, DifficultyMode difficulty)
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
    void openDangerousExplorationSite(Player& player, Random& random, const ExplorationBiome& biome, const ExplorationIntensity& intensity, DifficultyMode difficulty)
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
            "Le registre des Boss grave maintenant son sceau. Reviens par cette voie si tu veux vraiment l'affronter."
        };

        bool newEntityDetected = player.unlockNextBossVariation();
        if (newEntityDetected)
        {
            bossLines.push_back("Une nouvelle entité a été détectée dans le registre des variations d'énergie anormale.");
            bossLines.push_back("Nom : ???");
            bossLines.push_back("Statut : éveillé par exploration dangereuse.");
        }
        else
        {
            bossLines.push_back("Le registre tremble, mais aucune nouvelle entrée ne se stabilise pour l'instant.");
        }

        int updated = progressExplorationQuests(player, biome.name, 2);
        if (updated > 0)
        {
            bossLines.push_back("Des notes d'exploration progressent grâce à cette découverte dangereuse.");
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

    void triggerActiveExplorationEvent(Player& player, Random& random, const ExplorationBiome& biome, const ExplorationIntensity& intensity, DifficultyMode difficulty)
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
                "Tu récupères dans le camp : " + std::to_string(gold) + " pièces.",
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
                {{1, "Étudier le signe"}, {2, "Tenter de prélever un fragment"}, {0, "Ne pas toucher"}},
                0,
                2
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
                showExplorationNotice("SIGNE ÉTUDIÉ", "exploration.event.ancient_sign.study", lines);
                return;
            }

            showExplorationNotice("FRAGMENT INSTABLE", "exploration.event.ancient_sign.fragment", {"Le fragment refuse d'être prélevé gratuitement."});
            bool victory = runTrackedExplorationWave(
                player,
                random,
                difficulty,
                createExplorationGroup(player, random, biome, intensity, 1, 2, true),
                "Autel instable : réaction de " + biome.name
            );

            if (victory)
            {
                std::vector<std::string> rewardLines = {
                    addExplorationMaterial(player, "variation_residue", applyExplorationQuantityBonus(1, intensity), chooseExplorationQuality(random, true)),
                    addExplorationMaterial(player, biome.rareMaterialId, 1, chooseExplorationQuality(random, true))
                };
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
        std::vector<std::string> rewardLines = {"Récompense improvisée : " + std::to_string(gold) + " pièces."};
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
void QuestMenu::openQuestHub(Player& player)
{
    while (true)
    {
        const int readyCount = countQuestsForJournalFilter(player, QuestJournalFilter::ReadyToTurnIn);
        MenuScreen screen("QUÊTES", "quest.hub");
        screen.addLine("Les quêtes progressent en combattant, explorant, récupérant des ressources ou battant les bonnes cibles.");
        screen.addLine("Quêtes de guilde actives : " + std::to_string(player.getQuestLog().getActiveGuildQuestCount()) + "/3.");
        screen.addLine("Demandes prêtes à rendre : " + std::to_string(readyCount) + ".");
        screen.addBackOption("Retour", "quest.hub.back");

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

        screen.addOption(1, "Consulter le journal de quêtes", "Voir les quêtes et estimations connues.", true, "quest.hub.journal", journalData);
        screen.addOption(2, "Rendre une quête prête" + (readyCount > 0 ? " [" + std::to_string(readyCount) + "]" : ""),
            readyCount > 0 ? "Choisir un contact et valider une quête terminée." : "Aucune quête prête à rendre.",
            readyCount > 0,
            "quest.hub.ready_turn_in",
            readyData
        );
        screen.addOption(3, "Aller à la guilde", "Consulter le panneau officiel ou rendre un contrat de guilde.", true, "quest.hub.guild", guildData);

        int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choix invalide.");
        Console::clear();

        if (choice == 0)
        {
            return;
        }

        if (choice == 1)
        {
            displayQuestJournal(player);
        }
        else if (choice == 2)
        {
            openReadyQuestTurnInMenu(player);
        }
        else if (choice == 3)
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
        screen.addLine("Quêtes de guilde actives : " + std::to_string(player.getQuestLog().getActiveGuildQuestCount()) + "/3");
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
                itemData.reward = quest.guildQuest ? questRewardText(quest) : approximateQuestRewardText(quest);
                itemData.progress = std::to_string(quest.progress) + "/" + std::to_string(quest.target);
                itemData.owner = quest.client;
                itemData.important = isReadyToTurnIn(player, quest) || !quest.guildQuest;

                screen.addOption(
                    localNumber,
                    questCardLabel(quest),
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
            showQuestDetail(*displayedQuests[first + static_cast<std::size_t>(localQuestIndex)]);
            continue;
        }

        MessageScreen::show(
            "ACTION INDISPONIBLE",
            "quest.journal.invalid",
            {"Ce choix ne correspond à aucune action du journal."}
        );
    }
}

// EN: openGuild declares or implements a focused behavior used by this module.
// FR: openGuild déclare ou implémente un comportement précis utilisé par ce module.
void QuestMenu::openGuild(Player& player)
{
    while (true)
    {
        const ClientQuestCounts guildCounts = countQuestsForClient(player, "Maître de guilde");
        MenuScreen screen("GUILDE", "quest.guild");
        screen.addLine("La guilde centralise les quêtes officielles.");
        screen.addLine("Tu peux avoir jusqu'à 3 quêtes de guilde actives.");
        screen.addLine("Contrats de guilde : " + clientQuestStatusText(guildCounts));
        screen.addBackOption("Retour", "quest.guild.back");

        MenuOptionItemData boardData = makeQuestNavigationItemData(
            "quest",
            "Guilde",
            "quest",
            "Panneau de quêtes",
            "Voir les contrats officiels disponibles.",
            "Maître de guilde"
        );
        boardData.status = "Officiel";

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

        screen.addOption(1, "Voir le panneau de quêtes", "Consulter les contrats officiels disponibles.", true, "quest.guild.board", boardData);
        screen.addOption(2, "Rendre une quête de guilde terminée" + (guildCounts.ready > 0 ? " [" + std::to_string(guildCounts.ready) + "]" : ""),
            guildCounts.ready > 0 ? "Valider un contrat terminé." : "Aucun contrat de guilde prêt à rendre.",
            guildCounts.ready > 0,
            "quest.guild.turn_in",
            turnInData
        );
        screen.addOption(3, "Consulter le journal", "Lire le journal complet des quêtes.", true, "quest.guild.journal");

        int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choix invalide.");
        Console::clear();

        if (choice == 0)
        {
            return;
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
    }
}

// EN: acceptGuildQuest declares or implements a focused behavior used by this module.
// FR: acceptGuildQuest déclare ou implémente un comportement précis utilisé par ce module.
void QuestMenu::acceptGuildQuest(Player& player)
{
    QuestLog& questLog = player.getQuestLog();
    questLog.ensureGuildBoardReady(player.getLevel(), player.getCombatsStarted());

    const std::vector<Quest>& board = questLog.getGuildBoardOffers();
    MenuScreen screen("PANNEAU DE GUILDE", "quest.guild.board");
    screen.addLine("Quêtes actives : " + std::to_string(questLog.getActiveGuildQuestCount()) + "/3");
    screen.addLine("Offres visibles : " + std::to_string(board.size()) + "/" + std::to_string(questLog.getGuildBoardTargetSize()));

    int remainingBeforeRefresh = questLog.getGuildBoardCombatsBeforeRefresh(player.getCombatsStarted());
    if (remainingBeforeRefresh <= 0)
    {
        screen.addLine("Le panneau sera réécrit au prochain passage.");
    }
    else
    {
        screen.addLine("Le panneau actuel reste affiché encore " + std::to_string(remainingBeforeRefresh)
            + " combat" + (remainingBeforeRefresh > 1 ? "s" : "") + ".");
    }

    if (questLog.getGuildBoardPendingReplacements() > 0)
    {
        screen.addLine("Des places prises seront remplacées après le prochain combat.");
    }

    if (board.empty())
    {
        screen.addLine("Le panneau est vide pour l'instant. Repasse après un combat.");
    }

    screen.addBackOption("Retour", "quest.guild.board.back");

    for (int i = 0; i < static_cast<int>(board.size()); ++i)
    {
        std::string label = questCardLabel(board[i]);
        if (questLog.hasQuest(board[i].id))
        {
            label += " | Statut : déjà prise";
        }

        MenuOptionItemData itemData;
        itemData.structured = true;
        itemData.kind = "quest";
        itemData.section = "Panneau de guilde";
        itemData.actionType = "quest";
        itemData.name = board[i].title;
        itemData.detail = board[i].objective;
        itemData.status = questLog.hasQuest(board[i].id) ? "Déjà prise" : "Disponible";
        itemData.reward = questRewardText(board[i]);
        itemData.progress = "Rang " + board[i].rank;
        itemData.owner = "Guilde";
        itemData.important = !questLog.hasQuest(board[i].id);

        screen.addOption(
            i + 1,
            label,
            "Accepter cette quête de guilde si une place est libre.",
            true,
            "quest.guild.board.accept." + std::to_string(i + 1),
            itemData
        );
    }

    int choice = TerminalInterface::askMenuChoice(screen, 0, static_cast<int>(board.size()), "Choix invalide.");
    Console::clear();

    if (choice == 0)
    {
        return;
    }

    Quest selectedQuest = board[choice - 1];

    if (!questLog.canAcceptGuildQuest())
    {
        MessageScreen::show(
            "PANNEAU SATURÉ",
            "quest.guild.board.full",
            {
                "Tu as déjà 3 quêtes de guilde actives.",
                "Termine ou rends-en une avant d'en accepter une autre."
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

    if (questLog.addQuest(selectedQuest))
    {
        questLog.removeGuildBoardOfferAt(choice - 1, player.getCombatsStarted());

        std::vector<std::string> lines = {
            "Quête acceptée : " + selectedQuest.title
        };
        std::vector<std::string> dialogue = guildQuestAcceptedDialogueLines(selectedQuest);
        lines.insert(lines.end(), dialogue.begin(), dialogue.end());
        lines.push_back("Objectif : " + selectedQuest.objective);
        lines.push_back("Zone probable : " + selectedQuest.location);
        lines.push_back("Récompenses : " + questRewardText(selectedQuest));
        lines.push_back("Une nouvelle place sera préparée après ton prochain combat.");

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
void QuestMenu::openExploration(Player& player, DifficultyMode difficulty)
{
    openExplorationMenu(player, difficulty);
}

// EN: openLocations declares or implements a focused behavior used by this module.
// FR: openLocations déclare ou implémente un comportement précis utilisé par ce module.
void QuestMenu::openLocations(Player& player)
{
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
        {2, "Forge", "Commandes et demandes du forgeron.", "Forgeron", false, false},
        {3, "Herboristerie", "Demandes liées aux plantes et ingrédients.", "Alchimiste", false, false},
        {4, "Place du village", "Rumeurs, habitants et petites demandes.", "Villageois nerveux", false, false},
        {5, "Route commerciale", "Demandes de marchands et risques de voyage.", "Marchand inquiet", false, false},
        {6, "Boutique de monstres", "Composants de créatures et revente spécialisée.", "Vendeur de composants", false, false},
        {7, "Boutique de matériaux", "Matériaux, stocks et approvisionnement.", "Vendeur de matériaux", false, false},
        {8, "Armurerie défensive", "Protections, pièces d'armure et commandes.", "Armurier", false, false},
        {9, "Forge d'armes", "Armes, réparation et approvisionnement.", "Vendeur d'armes", false, false},
        {10, "Boutique de consommables", "Potions, consommables et réserves.", "Vendeur de consommables", false, false},
        {11, "Bibliothèque", "Notes, savoirs et pistes de recherche.", "Bibliothécaire", false, false},
        {12, "Ouvrir l'inventaire", "Consulter objets et connaissances avant de repartir.", "", false, true}
    };

    while (true)
    {
        MenuScreen screen("LIEUX VISITABLES", "quest.locations");
        screen.addLine("Chaque lieu peut servir à parler, rendre une demande ou vérifier un contact.");
        screen.addLine("Les demandes PNJ restent des pourparlers : seules les quêtes de guilde sont des contrats officiels.");
        screen.addBackOption("Retour", "quest.locations.back");

        for (const LocationEntry& entry : entries)
        {
            if (entry.inventory)
            {
                screen.addOption(
                    entry.choice,
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

            screen.addOption(
                entry.choice,
                label,
                entry.detail + " " + clientQuestHintText(counts),
                true,
                "quest.locations.select." + std::to_string(entry.choice),
                itemData
            );
        }

        int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choix invalide.");
        Console::clear();

        if (choice == 0)
        {
            return;
        }

        if (choice == 12)
        {
            InventoryMenu::open(player);
            Console::clear();
            continue;
        }

        for (const LocationEntry& entry : entries)
        {
            if (choice != entry.choice)
            {
                continue;
            }

            if (entry.guild)
            {
                openGuild(player);
            }
            else
            {
                talkToClient(player, entry.client);
            }
            break;
        }
    }
}

// EN: openNotableNpcMenu declares or implements a focused behavior used by this module.
// FR: openNotableNpcMenu déclare ou implémente un comportement précis utilisé par ce module.
void QuestMenu::openNotableNpcMenu(Player& player)
{
    while (true)
    {
        std::vector<std::pair<std::string, std::string>> entries = {
            {"Maître de guilde", "PNJ important / contrats officiels"},
            {"Forgeron", "PNJ client"},
            {"Alchimiste", "PNJ client"},
            {"Villageois nerveux", "PNJ client / événement"},
            {"Marchand inquiet", "PNJ client"},
            {"Vendeur de composants", "PNJ client"},
            {"Vendeur de matériaux", "PNJ client"},
            {"Herboriste", "PNJ client"},
            {"Armurier", "PNJ client"},
            {"Vendeur d'armes", "PNJ client"},
            {"Vendeur de consommables", "PNJ client"},
            {"Bibliothécaire", "PNJ client"}
        };

        const std::vector<std::string> recommendedClients = collectRecommendedClients(player);
        for (const std::string& clientName : recommendedClients)
        {
            entries.push_back({clientName, "Recommandé par un habitant"});
        }

        MenuScreen screen("PNJ NOTABLES", "quest.notable_npc");
        screen.addLine("Sélectionne un contact pour parler, consulter ses demandes ou rendre ce qui est terminé.");
        screen.addLine("Les PNJ donnent des demandes de vive voix : le journal les estime, il ne les certifie pas comme la guilde.");
        screen.addBackOption("Retour", "quest.notable_npc.back");

        bool printedRecommendedHeader = false;
        for (int i = 0; i < static_cast<int>(entries.size()); ++i)
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

            screen.addOption(
                i + 1,
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
            screen.addLine("--- Recommandés par un habitant ---");
            screen.addLine("Aucun nom recommandé pour l'instant.");
        }

        int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choix invalide.");
        Console::clear();

        if (choice == 0)
        {
            return;
        }

        const std::string selectedClient = entries[choice - 1].first;
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
        const ClientQuestCounts counts = countQuestsForClient(player, clientName);
        MenuScreen screen(clientName, "quest.client");
        addClientQuestSummaryLines(screen, player, clientName);
        screen.addBackOption("Retour", "quest.client.back");

        MenuOptionItemData talkData = makeClientQuestNavigationItemData(
            clientName,
            "Contact",
            "Demander si ce contact a quelque chose à confier.",
            counts
        );
        talkData.actionType = "talk";
        talkData.status = isRecommendedClientName(clientName)
            ? "Contact recommandé - demandes limitées"
            : "Pourparler possible";

        MenuOptionItemData overviewData = makeClientQuestNavigationItemData(
            clientName,
            "Contact",
            "Consulter les demandes connues de ce contact.",
            counts
        );
        overviewData.actionType = "inspect";
        overviewData.status = counts.total > 0 ? clientQuestStatusText(counts) : "Aucune demande connue";
        overviewData.important = counts.ready > 0;

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
            "Demander si ce contact a quelque chose à confier.",
            true,
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

        std::vector<std::string> introLines;
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

        if (questRandom.between(1, 100) <= 70)
        {
            std::string targetedBiome = randomBiomeForClient(questRandom, clientName);
            introLines.push_back(clientName + " n'a rien de totalement officiel à confier pour le moment.");
            introLines.push_back("Cette fois, il parle surtout d'une zone précise : " + targetedBiome + ".");
            offeredQuest = QuestCatalog::createBiomeRequest(player.getLevel(), targetedBiome, clientName);
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
            if (player.getQuestLog().addQuest(offeredQuest))
            {
                player.getQuestLog().refreshMaterialDeliveryQuests(player.getInventory());
                std::vector<std::string> lines = {"Demande acceptée : " + offeredQuest.title};
                std::vector<std::string> dialogue = clientQuestAcceptedDialogueLines(offeredQuest);
                lines.insert(lines.end(), dialogue.begin(), dialogue.end());
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
    while (true)
    {
        const std::vector<ReadyQuestClientEntry> entries = collectReadyQuestClients(player);
        MenuScreen screen("QUÊTES PRÊTES À RENDRE", "quest.ready_turn_in");
        screen.addLine("Choisis le contact concerné : la validation se fait auprès de la personne ou de l'organisme qui a confié la demande.");
        screen.addLine("Rappel : la guilde valide des contrats officiels ; les PNJ confirment surtout des pourparlers et services rendus.");
        screen.addBackOption("Retour", "quest.ready_turn_in.back");

        if (entries.empty())
        {
            screen.addLine("Aucune quête n'est prête à rendre pour le moment.");
            TerminalInterface::askMenuChoiceFromOptions(screen, "Entre 0 pour revenir.");
            Console::clear();
            return;
        }

        for (int i = 0; i < static_cast<int>(entries.size()); ++i)
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
            itemData.owner = entry.clientName;
            itemData.important = true;

            std::string label = entry.clientName + " | " + readyQuestClientStatusText(entry);
            if (!entry.firstTitle.empty())
            {
                label += " | Première : " + entry.firstTitle;
            }

            screen.addOption(
                i + 1,
                label,
                entry.guildReadyCount > 0 && entry.personalReadyCount == 0
                    ? "Rendre un contrat officiel auprès de ce contact."
                    : "Rendre une demande ou un service terminé auprès de ce contact.",
                true,
                "quest.ready_turn_in.client." + std::to_string(i + 1),
                itemData
            );
        }

        int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choix invalide.");
        Console::clear();

        if (choice == 0)
        {
            return;
        }

        if (choice >= 1 && choice <= static_cast<int>(entries.size()))
        {
            completeQuestAtClient(player, entries[choice - 1].clientName);
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
    std::vector<Quest>& quests = player.getQuestLog().getQuests();
    std::vector<int> readyIndexes;

    for (int i = 0; i < static_cast<int>(quests.size()); ++i)
    {
        if (quests[i].client == clientName && !quests[i].turnedIn && isReadyToTurnIn(player, quests[i]))
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

    MenuScreen screen("QUÊTES À RENDRE", "quest.turn_in.list");
    screen.addSubtitle(clientName);
    screen.addLine(clientName == "Maître de guilde"
        ? "Sélectionne le contrat officiel à tamponner auprès de la guilde."
        : "Sélectionne la demande à confirmer auprès de ce contact. Ce n'est pas un tampon officiel, plutôt une validation de parole donnée.");
    screen.addBackOption("Retour", "quest.turn_in.back");

    for (int i = 0; i < static_cast<int>(readyIndexes.size()); ++i)
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
        itemData.status = "Prête à rendre";
        itemData.reward = quest.guildQuest ? questRewardText(quest) : approximateQuestRewardText(quest);
        itemData.progress = std::to_string(quest.progress) + "/" + std::to_string(quest.target);
        itemData.owner = quest.client;
        itemData.important = true;

        screen.addOption(
            i + 1,
            label,
            quest.guildQuest
                ? "Tamponner ce contrat officiel et recevoir les récompenses."
                : "Confirmer cette demande informelle auprès du PNJ.",
            true,
            "quest.turn_in.select." + std::to_string(i + 1),
            itemData
        );
    }

    int choice = TerminalInterface::askMenuChoice(screen, 0, static_cast<int>(readyIndexes.size()), "Choix invalide.");
    Console::clear();

    if (choice == 0)
    {
        return;
    }

    Quest& quest = quests[readyIndexes[choice - 1]];

    if (!askQuestTurnInConfirmation(player, quest, clientName))
    {
        return;
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
            return;
        }

        player.getInventory().removeMaterialQuantityByIdFlexible(quest.requiredMaterialId, quest.requiredMaterialQuantity);
        resultLines.push_back("Matériaux remis : " + quest.requiredMaterialName + " x" + std::to_string(quest.requiredMaterialQuantity));
    }

    quest.completed = true;
    quest.progress = quest.target;
    quest.turnedIn = true;
    player.gainExperience(quest.rewardExperience);
    player.getInventory().earnGold(quest.rewardGold);
    applyQuestExtraReward(player, quest);

    appendQuestRewardResultLines(resultLines, quest);
    resultLines.push_back("Or : " + std::to_string(goldBefore) + " -> " + std::to_string(player.getInventory().getGold()));
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
}

// EN: maybeOfferRandomInterception declares or implements a focused behavior used by this module.
// FR: maybeOfferRandomInterception déclare ou implémente un comportement précis utilisé par ce module.
void QuestMenu::maybeOfferRandomInterception(Player& player, DifficultyMode difficulty)
{
    Random random;

    if (random.between(1, 100) > 22)
    {
        return;
    }

    if (random.between(1, 100) <= 25)
    {
        simulateAfterCombatMiniBoss(player, random, difficulty);
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
void QuestMenu::openExplorationMenu(Player& player, DifficultyMode difficulty)
{
    std::vector<ExplorationBiome> biomes = {
        {"Plaine sauvage", "biome ouvert, accessible aux débutants, mais jamais totalement sûr", "worn_leather_piece", "wolf_fang", 1, 10, "bêtes faibles, sangliers, loups isolés", "alphas jeunes, ours errants"},
        {"Route commerciale", "biome de passage accessible, avec voyageurs, bandits faibles et caisses perdues", "battle_torn_badge", "worn_leather_piece", 1, 14, "bandits, gobelins, humanoïdes opportunistes", "pilleurs vétérans, embuscades organisées"},
        {"Mares gélatineuses", "zone connue pour ses slimes : beaucoup de couleurs, peu de logique humaine, beaucoup de résidus", "slime_residue", "arcane_dust", 3, 18, "slimes verts, bleus, jaunes, rouges, ambrés et gris", "slimes chromatiques, dorés ou noirs anciens"},
        {"Forêt ancienne", "biome végétal plus sérieux, humide, propice aux plantes et aux bêtes discrètes", "bitter_healing_leaf", "mountain_blue_flower", 5, 20, "loups, racines, plantes hostiles", "alphas de mousse, gardiens de ronces"},
        {"Montagne froide", "biome rocheux intermédiaire, dur, avec minerais, fleurs rares et vents coupants", "rusted_metal_fragment", "mountain_blue_flower", 7, 24, "bêtes de givre, élémentaires, briseurs", "yétis, draconides froids, élites rocheuses"},
        {"Marais trouble", "biome dangereux, sale, collant et difficile d'accès en début de partie", "slime_residue", "arcane_dust", 12, 32, "slimes corrosifs, noyés, insectoïdes, prédateurs de boue", "slimes couronnés, mages putrides, noyés anciens"},
        {"Cimetière oublié", "biome sombre de niveau intermédiaire/avancé, lié aux morts-vivants, aux noms perdus et aux composants d'ombre", "cracked_bone", "shadow_thread", 10, 30, "squelettes, goules, corbeaux, lanternes d'âme", "oracles de tombe, ombres rares, ossuaires rampants"},
        {"Ruines effondrées", "biome ancien dangereux, instable, avec os, poussière arcanique et coffres suspects", "cracked_bone", "arcane_dust", 14, 36, "squelettes, goules, esprits, armures fissurées", "revenants, armures mortes, anomalies"}
    };

    std::vector<ExplorationIntensity> intensities = {
        {"Sortie prudente", "moins de danger, mais moins de trouvailles importantes", -10, 0, 80, 3},
        {"Sortie normale", "équilibre entre découverte, gain et risque", 0, 0, 100, 0},
        {"Sortie audacieuse", "plus de danger, plus de chances de coffres, mini-boss, lieux rares et meilleurs gains", 12, 1, 125, -2}
    };

    while (true)
    {
        MenuScreen screen("EXPLORATION", "exploration.biomes");
        screen.addLine("Choisis le style de biome à explorer.");
        screen.addLine("Exploration = fouille de terrain : plantes, matériaux, traces, trésors, coffres ou dangers imprévus.");
        screen.addLine("Tu pars chercher des traces, mais le terrain peut décider de te répondre avec des griffes.");
        screen.addLine("Économie : l'or direct d'exploration est pondéré par la difficulté ; les matériaux restent une grosse partie de la valeur.");

        bool hasEvolvedBiome = false;
        for (const ExplorationBiome& biomePreview : biomes)
        {
            if (isBiomeEvolvedForPlayer(player, biomePreview))
            {
                if (!hasEvolvedBiome)
                {
                    screen.addLine("Zones qui ont évolué avec ton niveau :");
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
            screen.addLine("Le monde ne t'attend pas immobile : les anciennes zones peuvent attirer des menaces adaptées.");
        }

        screen.addOption(0, "Retour", "", true, "exploration.back");

        for (int i = 0; i < static_cast<int>(biomes.size()); ++i)
        {
            std::string label = biomes[i].name
                + " (" + evolvedBiomeRangeText(player, biomes[i]) + ") — "
                + biomes[i].style;

            const bool questLikely = hasPotentialQuestForBiome(player, biomes[i]);

            if (questLikely)
            {
                label += " [Objectif de quête probable]";
            }

            screen.addOption(
                i + 1,
                label,
                "Terrain : " + biomes[i].commonMonsters + " | Rares : " + biomes[i].rareMonsters,
                true,
                "exploration.biome." + std::to_string(i + 1),
                makeExplorationBiomeItemData(player, biomes[i], questLikely)
            );
        }

        int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choix invalide.");
        Console::clear();

        if (choice == 0)
        {
            return;
        }

        const ExplorationBiome& biome = biomes[choice - 1];

        MenuScreen intensityScreen("INTENSITÉ", "exploration.intensity");
        intensityScreen.addLine("Choisis comment tu veux explorer " + biome.name + ".");
        intensityScreen.addOption(0, "Retour aux biomes", "", true, "exploration.intensity.back");

        for (int i = 0; i < static_cast<int>(intensities.size()); ++i)
        {
            intensityScreen.addOption(
                i + 1,
                intensities[i].name + " — " + intensities[i].description,
                "Risque " + std::to_string(intensities[i].eventShift)
                    + "% | Or " + std::to_string(intensities[i].goldPercent)
                    + "% | Trouvailles +" + std::to_string(intensities[i].quantityBonus),
                true,
                "exploration.intensity." + std::to_string(i + 1),
                makeExplorationIntensityItemData(intensities[i])
            );
        }

        int intensityChoice = TerminalInterface::askMenuChoiceFromOptions(intensityScreen, "Choix invalide.");
        Console::clear();

        if (intensityChoice == 0)
        {
            continue;
        }

        const ExplorationIntensity& intensity = intensities[intensityChoice - 1];
        const int hpBeforeExploration = player.getHp();
        const int goldBeforeExploration = player.getInventory().getGold();
        const int readyBeforeExploration = countQuestsForJournalFilter(player, QuestJournalFilter::ReadyToTurnIn);

        Random random;
        QuestSearchHint questHint = getQuestSearchHintForBiome(player, biome);
        int roll = adjustExplorationEventRoll(random.between(1, 100), intensity);
        roll = adjustExplorationRollForActiveQuests(roll, random, questHint);
        const std::string eventLabel = explorationEventLabelFromRoll(roll);
        bool carefulRecovery = chooseCarefulRecovery(random, intensity);

        std::vector<std::string> entryLines = {
            "Style : " + biome.style + ".",
            "Niveaux locaux : " + std::to_string(biome.minLevel) + "-" + std::to_string(biome.maxLevel) + ".",
            "Monstres surtout présents : " + biome.commonMonsters + ".",
            "Rares / élites typiques : " + biome.rareMonsters + ".",
            "Approche : " + intensity.name + "."
        };

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

        if (roll <= 24)
        {
            std::vector<std::string> lines = {"Tu fouilles calmement la zone."};
            if (carefulRecovery)
            {
                lines.push_back("Récolte propre : tu récupères la ressource de la meilleure façon possible.");
            }
            lines.push_back(addExplorationMaterial(player, biome.commonMaterialId, applyExplorationQuantityBonus(random.between(1, 2), intensity), chooseExplorationQuality(random, carefulRecovery)));
            showExplorationNotice("RÉCOLTE", "exploration.run.gather", lines);
        }
        else if (roll <= 37)
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
        else if (roll <= 48)
        {
            int gold = applyExplorationGoldReward(random.between(5, 22 + player.getLevel() * 2), player, intensity, difficulty, 1);
            player.getInventory().earnGold(gold);
            showExplorationNotice(
                "PETIT TRÉSOR",
                "exploration.run.gold",
                {"Tu trouves un petit trésor au sol.", "Or gagné : " + std::to_string(gold)}
            );
        }
        else if (roll <= 54)
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
        else if (roll <= 66)
        {
            openExplorationChest(player, random, biome, intensity, difficulty);
        }
        else if (roll <= 78)
        {
            simulateUnexpectedExplorationFight(player, random, biome, intensity, difficulty);
        }
        else if (roll <= 87)
        {
            simulateExplorationMiniBoss(player, random, biome, intensity, difficulty);
        }
        else if (roll <= 91)
        {
            offerExplorationNpcQuest(player, random, biome);
        }
        else if (roll <= 96)
        {
            triggerActiveExplorationEvent(player, random, biome, intensity, difficulty);
        }
        else if (roll <= 98)
        {
            openDangerousExplorationSite(player, random, biome, intensity, difficulty);
        }
        else
        {
            triggerRareExplorationDiscovery(player, random, biome, intensity, difficulty);
        }

        player.getQuestLog().refreshMaterialDeliveryQuests(player.getInventory());
        showExplorationRunSummary(
            player,
            biome,
            intensity,
            eventLabel,
            hpBeforeExploration,
            goldBeforeExploration,
            readyBeforeExploration
        );
        Console::clear();
    }
}
