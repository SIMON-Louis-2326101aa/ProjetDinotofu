// EN: QuestCatalog.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: QuestCatalog.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu.
// Description: Builds quest templates for guild and notable NPCs.

#include "quest/QuestCatalog.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <random>
#include <string>
#include <vector>

namespace
{
    struct RequestedMaterial
    {
        std::string id;
        std::string name;
        int baseQuantity;
        int minLevel;
    };

    Quest buildQuest(
        const std::string& id,
        const std::string& rank,
        const std::string& title,
        const std::string& origin,
        const std::string& client,
        const std::string& location,
        const std::string& objective,
        const std::string& objectiveType,
        const std::string& targetFamily,
        int experience,
        int gold,
        int target,
        bool guildQuest,
        const std::string& requiredMaterialId = "",
        const std::string& requiredMaterialName = "",
        int requiredMaterialQuantity = 0,
        const std::string& rewardMaterialId = "",
        const std::string& rewardMaterialName = "",
        int rewardMaterialQuantity = 0,
        const std::string& rewardNote = ""
    )
    {
        Quest quest;
        quest.id = id;
        quest.rank = rank;
        quest.title = title;
        quest.origin = origin;
        quest.client = client;
        quest.location = location;
        quest.objective = objective;
        quest.objectiveType = objectiveType;
        quest.targetFamily = targetFamily;
        quest.rewardExperience = experience;
        quest.rewardGold = gold;
        quest.rewardMaterialId = rewardMaterialId;
        quest.rewardMaterialName = rewardMaterialName;
        quest.rewardMaterialQuantity = rewardMaterialQuantity;
        quest.rewardNote = rewardNote;
        quest.requiredMaterialId = requiredMaterialId;
        quest.requiredMaterialName = requiredMaterialName;
        quest.requiredMaterialQuantity = requiredMaterialQuantity;
        quest.progress = 0;
        quest.target = target;
        quest.guildQuest = guildQuest;
        quest.accepted = true;
        quest.completed = false;
        quest.turnedIn = false;
        return quest;
    }

    std::mt19937& questGenerator()
    {
        static std::random_device device;
        static std::mt19937 generator(device());
        return generator;
    }

    // EN: randomBetween declares or implements a focused behavior used by this module.
    // FR: randomBetween déclare ou implémente un comportement précis utilisé par ce module.
    int randomBetween(int min, int max)
    {
        std::uniform_int_distribution<int> distribution(min, max);
        return distribution(questGenerator());
    }

    // EN: chooseMaterial declares or implements a focused behavior used by this module.
    // FR: chooseMaterial déclare ou implémente un comportement précis utilisé par ce module.
    RequestedMaterial chooseMaterial(const std::vector<RequestedMaterial>& materials, int playerLevel)
    {
        std::vector<RequestedMaterial> available;

        for (const RequestedMaterial& material : materials)
        {
            if (playerLevel >= material.minLevel)
            {
                available.push_back(material);
            }
        }

        if (available.empty())
        {
            return materials.front();
        }

        return available[randomBetween(0, static_cast<int>(available.size()) - 1)];
    }

    std::string questIdWithMaterial(const std::string& prefix, const RequestedMaterial& material, int playerLevel)
    {
        return prefix + "_" + material.id + "_lvl_" + std::to_string(playerLevel) + "_" + std::to_string(randomBetween(100, 999));
    }

    // EN: materialQuantity declares or implements a focused behavior used by this module.
    // FR: materialQuantity déclare ou implémente un comportement précis utilisé par ce module.
    int materialQuantity(const RequestedMaterial& material, int playerLevel)
    {
        int quantity = material.baseQuantity;

        if (playerLevel >= 5)
        {
            quantity++;
        }

        if (playerLevel >= 10 && material.baseQuantity <= 3)
        {
            quantity++;
        }

        return quantity;
    }

    // EN: rankPower declares or implements a focused behavior used by this module.
    // FR: rankPower déclare ou implémente un comportement précis utilisé par ce module.
    int rankPower(const std::string& rank)
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
        if (rank.find("F") != std::string::npos) return 1;
        return 1;
    }

    int minimumLevelForRank(const std::string& rank)
    {
        if (rank.find("Dieu") != std::string::npos) return 90;
        if (rank.find("Légende") != std::string::npos || rank.find("Legende") != std::string::npos) return 70;
        if (rank.find("Héros mondial") != std::string::npos || rank.find("Heros mondial") != std::string::npos) return 55;
        if (rank.find("SSS") != std::string::npos) return 42;
        if (rank.find("SS") != std::string::npos) return 32;
        if (rank.find("S") != std::string::npos) return 24;
        if (rank.find("A") != std::string::npos) return 16;
        if (rank.find("B") != std::string::npos) return 11;
        if (rank.find("C") != std::string::npos) return 7;
        if (rank.find("D") != std::string::npos) return 4;
        if (rank.find("E") != std::string::npos) return 2;
        return 1;
    }

    std::string visibleRankVariant(const std::string& baseRank, int playerLevel)
    {
        if (baseRank == "F")
        {
            if (playerLevel >= 4 && randomBetween(1, 100) <= 35) return "F+";
            return randomBetween(1, 100) <= 30 ? "F-" : "F";
        }

        if (baseRank == "E")
        {
            if (playerLevel <= 2 && randomBetween(1, 100) <= 40) return "E-";
            if (playerLevel >= 5 && randomBetween(1, 100) <= 35) return "E+";
            return "E";
        }

        if (baseRank == "D")
        {
            if (playerLevel <= 4 && randomBetween(1, 100) <= 35) return "D-";
            if (playerLevel >= 8 && randomBetween(1, 100) <= 35) return "D+";
            return "D";
        }

        if (baseRank == "C" || baseRank == "B" || baseRank == "A")
        {
            int roll = randomBetween(1, 100);
            if (roll <= 25) return baseRank + "-";
            if (roll >= 76) return baseRank + "+";
        }

        return baseRank;
    }

    // EN: questExperience declares or implements a focused behavior used by this module.
    // FR: questExperience déclare ou implémente un comportement précis utilisé par ce module.
    int questExperience(const std::string& rank, int playerLevel, int target)
    {
        return 18 + playerLevel * 7 + target * 8 + rankPower(rank) * 14;
    }

    // EN: questGold declares or implements a focused behavior used by this module.
    // FR: questGold déclare ou implémente un comportement précis utilisé par ce module.
    int questGold(const std::string& rank, int playerLevel, int target)
    {
        int value = 8 + playerLevel * 3 + target * 5 + rankPower(rank) * 7;
        return std::max(0, value);
    }

    int adjustedQuestGold(const std::string& rank, int playerLevel, int target, const std::string& objectiveType, bool givesObjectReward)
    {
        int value = questGold(rank, playerLevel, target);

        if (objectiveType == "livraison") value = value * 70 / 100;
        if (objectiveType == "bestiaire") value = value * 55 / 100;
        if (objectiveType == "service") value = value * 45 / 100;
        if (givesObjectReward) value = value * 60 / 100;

        return std::max(0, value);
    }

    std::string materialRank(int playerLevel, int minRankPower)
    {
        if (playerLevel >= 12 && minRankPower >= 5) return "B";
        if (playerLevel >= 8 && minRankPower >= 4) return "C";
        if (playerLevel >= 5) return "D";
        return "E";
    }

    std::string chooseText(const std::vector<std::string>& values)
    {
        if (values.empty())
        {
            return "";
        }

        return values[randomBetween(0, static_cast<int>(values.size()) - 1)];
    }

    std::string questId(const std::string& prefix, int playerLevel)
    {
        return prefix + "_lvl_" + std::to_string(playerLevel) + "_" + std::to_string(randomBetween(1000, 9999));
    }

    struct GuildTemplate
    {
        std::string rank;
        std::string title;
        std::string objective;
        std::string type;
        std::string family;
        int target;
        int minLevel;
    };

    struct ExtraReward
    {
        std::string materialId;
        std::string materialName;
        int materialQuantity = 0;
        std::string note;
    };

    ExtraReward chooseExtraReward(const GuildTemplate& questTemplate, int playerLevel)
    {
        ExtraReward reward;
        int roll = randomBetween(1, 100);

        if (questTemplate.type == "service")
        {
            if (roll <= 45)
            {
                reward.materialId = "client_recommendation";
                reward.materialName = "Recommandation de client";
                reward.materialQuantity = 1;
                reward.note = chooseText({
                    "Client supplémentaire recommandé : Mirette la couturière",
                    "Client supplémentaire recommandé : Noro le palefrenier",
                    "Client supplémentaire recommandé : Éliane du vieux pont",
                    "Client supplémentaire recommandé : Caldor le porteur de caisses",
                    "Client supplémentaire recommandé : Bruma la réparatrice de selles"
                });
                return reward;
            }

            if (roll <= 75)
            {
                reward.materialId = "guild_favor_token";
                reward.materialName = "Jeton de faveur de guilde";
                reward.materialQuantity = 1;
                reward.note = "La guilde te remet un jeton de faveur au lieu d'une grosse prime.";
                return reward;
            }

            reward.materialId = "local_service_letter";
            reward.materialName = "Lettre de service local";
            reward.materialQuantity = 1;
            reward.note = "Le client laisse une lettre proprement signée pour confirmer le service rendu.";
            return reward;
        }

        if (questTemplate.type == "bestiaire" || roll <= 22)
        {
            reward.materialId = playerLevel >= 8 ? "advanced_monster_notes" : "common_goblin_notes";
            reward.materialName = playerLevel >= 8 ? "Notes avancées de monstre" : "Notes communes de gobelin";
            reward.materialQuantity = 1;
            reward.note = "La guilde ajoute une petite note exploitable au dossier de terrain.";
            return reward;
        }

        if (questTemplate.type == "exploration" && roll <= 55)
        {
            reward.materialId = playerLevel >= 10 ? "preservation_vials" : "bitter_healing_leaf";
            reward.materialName = playerLevel >= 10 ? "Fioles de conservation" : "Feuille amère de soin";
            reward.materialQuantity = playerLevel >= 10 ? 1 : 2;
            reward.note = "Récompense matérielle à la place d'une grosse bourse d'or.";
            return reward;
        }

        if (questTemplate.type == "combat" && roll <= 35)
        {
            reward.materialId = playerLevel >= 12 ? "medium_repair_kit" : "weak_repair_kit";
            reward.materialName = playerLevel >= 12 ? "Kit de réparation moyen" : "Kit de réparation faible";
            reward.materialQuantity = 1;
            reward.note = "Prime matérielle : la guilde préfère parfois fournir du matériel plutôt que des pièces.";
            return reward;
        }

        return reward;
    }


    std::string lowerQuestCatalogText(std::string value)
    {
        std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });
        return value;
    }

    bool questCatalogTextContains(const std::string& haystack, const std::string& needle)
    {
        return lowerQuestCatalogText(haystack).find(lowerQuestCatalogText(needle)) != std::string::npos;
    }

    std::string suggestedGuildQuestLocation(const GuildTemplate& questTemplate)
    {
        const std::string combined = questTemplate.type + " " + questTemplate.family + " " + questTemplate.objective + " " + questTemplate.title;

        if (questTemplate.type == "service")
        {
            return "Comptoir de guilde / ville";
        }

        if (questCatalogTextContains(combined, "ruine") || questCatalogTextContains(combined, "relais") || questCatalogTextContains(combined, "archive"))
        {
            return "Ruines effondrées";
        }

        if (questCatalogTextContains(combined, "mort") || questCatalogTextContains(combined, "ombre") || questCatalogTextContains(combined, "os"))
        {
            return "Cimetière oublié";
        }

        if (questCatalogTextContains(combined, "slime") || questCatalogTextContains(combined, "gélatine") || questCatalogTextContains(combined, "gelatine"))
        {
            return "Mares gélatineuses";
        }

        if (questCatalogTextContains(combined, "marais") || questCatalogTextContains(combined, "noy"))
        {
            return "Marais trouble";
        }

        if (questCatalogTextContains(combined, "forêt") || questCatalogTextContains(combined, "foret") || questCatalogTextContains(combined, "plante"))
        {
            return "Forêt ancienne";
        }

        if (questCatalogTextContains(combined, "montagne") || questCatalogTextContains(combined, "froid") || questCatalogTextContains(combined, "métal") || questCatalogTextContains(combined, "metal") || questCatalogTextContains(combined, "forge"))
        {
            return "Montagne froide / Ruines effondrées";
        }

        if (questCatalogTextContains(combined, "route") || questCatalogTextContains(combined, "livraison") || questCatalogTextContains(combined, "village") || questCatalogTextContains(combined, "client") || questCatalogTextContains(combined, "humano"))
        {
            return "Route commerciale";
        }

        if (questCatalogTextContains(combined, "mini-boss") || questCatalogTextContains(combined, "menace") || questCatalogTextContains(combined, "élite") || questCatalogTextContains(combined, "elite"))
        {
            return "Exploration audacieuse / zone adaptée au niveau";
        }

        return "Plaine sauvage";
    }

    // EN: chooseGuildTemplate declares or implements a focused behavior used by this module.
    // FR: chooseGuildTemplate déclare ou implémente un comportement précis utilisé par ce module.
    GuildTemplate chooseGuildTemplate(const std::vector<GuildTemplate>& templates, int playerLevel)
    {
        std::vector<GuildTemplate> available;

        for (const GuildTemplate& questTemplate : templates)
        {
            if (playerLevel >= questTemplate.minLevel && playerLevel >= minimumLevelForRank(questTemplate.rank))
            {
                available.push_back(questTemplate);
            }
        }

        if (available.empty())
        {
            return templates.front();
        }

        return available[randomBetween(0, static_cast<int>(available.size()) - 1)];
    }

    bool hasAvailableGuildTemplate(const std::vector<GuildTemplate>& templates, int playerLevel)
    {
        for (const GuildTemplate& questTemplate : templates)
        {
            if (playerLevel >= questTemplate.minLevel && playerLevel >= minimumLevelForRank(questTemplate.rank))
            {
                return true;
            }
        }

        return false;
    }

    // EN: buildGuildQuest declares or implements a focused behavior used by this module.
    // FR: buildGuildQuest déclare ou implémente un comportement précis utilisé par ce module.
    Quest buildGuildQuest(const std::string& idPrefix, int playerLevel, const GuildTemplate& questTemplate)
    {
        ExtraReward extraReward = chooseExtraReward(questTemplate, playerLevel);
        bool givesObjectReward = !extraReward.materialId.empty();
        std::string finalRank = visibleRankVariant(questTemplate.rank, playerLevel);

        return buildQuest(
            questId(idPrefix, playerLevel),
            finalRank, questTemplate.title, "Guilde", "Maître de guilde", suggestedGuildQuestLocation(questTemplate),
            questTemplate.objective, questTemplate.type, questTemplate.family,
            questExperience(finalRank, playerLevel, questTemplate.target),
            adjustedQuestGold(finalRank, playerLevel, questTemplate.target, questTemplate.type, givesObjectReward),
            questTemplate.target, true,
            "", "", 0,
            extraReward.materialId, extraReward.materialName, extraReward.materialQuantity, extraReward.note
        );
    }

    bool boardAlreadyHasTitle(const std::vector<Quest>& board, const std::string& title)
    {
        for (const Quest& quest : board)
        {
            if (quest.title == title)
            {
                return true;
            }
        }

        return false;
    }

    void addGuildQuestIfAvailable(std::vector<Quest>& board, const std::string& idPrefix, int playerLevel, const std::vector<GuildTemplate>& templates)
    {
        if (!hasAvailableGuildTemplate(templates, playerLevel))
        {
            return;
        }

        for (int attempt = 0; attempt < 8; ++attempt)
        {
            GuildTemplate selectedTemplate = chooseGuildTemplate(templates, playerLevel);
            if (!boardAlreadyHasTitle(board, selectedTemplate.title))
            {
                board.push_back(buildGuildQuest(idPrefix, playerLevel, selectedTemplate));
                return;
            }
        }

        board.push_back(buildGuildQuest(idPrefix, playerLevel, chooseGuildTemplate(templates, playerLevel)));
    }

    struct BiomeQuestProfile
    {
        std::string location;
        std::string family;
        std::vector<RequestedMaterial> materials;
        std::vector<std::string> explorationObjectives;
        std::vector<std::string> combatObjectives;
    };

    // EN: profileForBiome declares or implements a focused behavior used by this module.
    // FR: profileForBiome déclare ou implémente un comportement précis utilisé par ce module.
    BiomeQuestProfile profileForBiome(const std::string& biomeName)
    {
        if (biomeName == "Forêt ancienne")
        {
            return {
                biomeName, "Forêt ancienne / créatures naturelles",
                {{"bitter_healing_leaf", "Feuille amère de soin", 4, 1}, {"mountain_blue_flower", "Fleur bleue de montagne", 2, 1}, {"wolf_fang", "Croc de loup", 2, 2}, {"beast_hide", "Peau de bête robuste", 1, 3}},
                {"Relever des traces propres sans arracher la zone aux racines.", "Cartographier les plantes utiles et éviter les zones où la forêt devient silencieuse.", "Récupérer des indices naturels sans provoquer les gardiens de mousse."},
                {"Éloigner les bêtes qui rôdent autour des sentiers sans ravager leur territoire.", "Repousser une menace locale liée aux ronces et aux crocs."}
            };
        }

        if (biomeName == "Montagne froide")
        {
            return {
                biomeName, "Montagne froide / bêtes de givre",
                {{"rusted_metal_fragment", "Fragment de métal rouillé", 4, 1}, {"mountain_blue_flower", "Fleur bleue de montagne", 2, 1}, {"arcane_dust", "Poussière arcanique", 1, 4}, {"draconic_scale_fragment", "Fragment d'écaille draconique", 1, 7}},
                {"Suivre les marques laissées dans la neige avant qu'elles ne disparaissent.", "Explorer un ancien passage gelé sans déclencher tout l'éboulement.", "Trouver une ressource froide assez stable pour être rapportée."},
                {"Repousser une créature adaptée au froid avant qu'elle ne descende vers les routes.", "Survivre à une embuscade dans une pente glacée."}
            };
        }

        if (biomeName == "Marais trouble")
        {
            return {
                biomeName, "Marais trouble / slimes et noyés",
                {{"slime_residue", "Résidu de slime", 4, 1}, {"arcane_dust", "Poussière arcanique", 2, 2}, {"cracked_bone", "Os fissuré", 2, 2}, {"unstable_core", "Noyau instable", 1, 7}},
                {"Prélever ce qui flotte encore sans tomber dans ce qui respire dessous.", "Marquer les flaques qui bougent toutes seules pour les prochains aventuriers.", "Récupérer un échantillon collant sans le laisser manger le sac."},
                {"Nettoyer une poche de slimes avant qu'elle ne bloque un passage.", "Affronter une chose noyée qui attire les voyageurs hors du sentier."}
            };
        }

        if (biomeName == "Route commerciale")
        {
            return {
                biomeName, "Route commerciale / humanoïdes et embuscades",
                {{"battle_torn_badge", "Insigne abîmé d'aventurier", 2, 1}, {"worn_leather_piece", "Morceau de cuir abîmé", 3, 1}, {"goblin_ear", "Oreille de gobelin", 3, 1}, {"shadow_thread", "Fil d'ombre", 1, 5}},
                {"Retrouver une caisse perdue avant que les gobelins ne l'appellent investissement.", "Vérifier une portion de route où les marchands disparaissent toujours au même virage.", "Repérer les faux péages et les traces de pillards."},
                {"Dégager une petite bande d'embuscadeurs de la route.", "Faire comprendre à des voleurs que la taxe gobeline n'est pas reconnue par la guilde."}
            };
        }

        if (biomeName == "Ruines effondrées")
        {
            return {
                biomeName, "Ruines effondrées / morts-vivants et reliques",
                {{"cracked_bone", "Os fissuré", 3, 1}, {"arcane_dust", "Poussière arcanique", 2, 1}, {"battle_torn_badge", "Insigne abîmé d'aventurier", 1, 2}, {"shadow_thread", "Fil d'ombre", 1, 5}},
                {"Copier des symboles de ruines avant qu'ils ne se déplacent encore.", "Explorer une salle fissurée et revenir avec une preuve qu'elle existe vraiment.", "Récupérer une poussière ancienne sans réveiller tout le couloir."},
                {"Briser quelques gardiens fissurés qui bloquent l'accès aux fouilles.", "Nettoyer une alcôve où les os ont recommencé à s'organiser."}
            };
        }

        return {
            biomeName.empty() ? "Plaine sauvage" : biomeName, "Plaine sauvage / créatures locales",
            {{"worn_leather_piece", "Morceau de cuir abîmé", 3, 1}, {"wolf_fang", "Croc de loup", 2, 1}, {"beast_hide", "Peau de bête robuste", 1, 3}, {"bitter_healing_leaf", "Feuille amère de soin", 3, 1}},
            {"Suivre des traces fraîches dans l'herbe sans attirer toute la meute.", "Chercher un petit trésor de plaine avant qu'un animal curieux ne parte avec.", "Cartographier une zone calme qui ne le restera probablement pas."},
            {"Repousser une bête territoriale qui approche trop près des chemins.", "Faire fuir une meute locale sans transformer la plaine en cimetière."}
        };
    }
}

std::vector<Quest> QuestCatalog::createGuildBoard(int playerLevel)
{
    std::vector<Quest> board;
    std::vector<GuildTemplate> fillerTemplates;
    auto registerTemplates = [&fillerTemplates](const std::vector<GuildTemplate>& templates) {
        fillerTemplates.insert(fillerTemplates.end(), templates.begin(), templates.end());
    };

    const std::vector<GuildTemplate> fTemplates = {
        {"F", "Livrer des provisions à un camp proche", "Terminer une mission courte pour sécuriser une livraison locale.", "exploration", "Route / livraison", 1, 1},
        {"F", "Repérer une trace facile", "Sortir brièvement et confirmer une trace signalée par un éclaireur débutant.", "exploration", "Plaine sauvage / traces", 1, 1},
        {"F", "Aider un garde qui débute", "Écarter une petite menace sans transformer la mission en duel héroïque inutile.", "combat", "Créatures faibles", 1, 1},
        {"F", "Retrouver le seau de Madame Brune", "Rendre un petit service local. Pas glorieux, mais la guilde paie parfois en contacts plutôt qu'en or.", "service", "Service local", 1, 1},
        {"F", "Compter les caisses du dépôt", "Vérifier un stock de base sans se faire enfermer par erreur dans la réserve.", "service", "Guilde / inventaire", 1, 1},
        {"F", "Porter une lettre pas urgente", "Livrer un message de village sans prétendre que c'est une mission de héros.", "service", "Village / comptoir", 1, 1}
    };
    registerTemplates(fTemplates);

    const std::vector<GuildTemplate> eTemplates = {
        {"E", "Nettoyer une petite route infestée", "Écarter quelques monstres faibles d'une route locale.", "combat", "Créatures faibles", 2, 2},
        {"E", "Cartographier un détour douteux", "Explorer une zone simple et revenir avec assez de détails pour corriger la carte de la guilde.", "exploration", "Route / exploration", 2, 2},
        {"E", "Surveiller une caisse suspecte", "Vérifier une livraison abandonnée et survivre à ce qui pourrait se cacher dedans.", "exploration", "Route commerciale", 2, 2},
        {"E", "Retrouver un client qui se cache", "Identifier un client paniqué qui doit de l'argent à trois personnes différentes.", "service", "Clientèle locale", 1, 2},
        {"E", "Récupérer des outils oubliés", "Rapporter du petit matériel abandonné près d'une zone encore raisonnable.", "exploration", "Route commerciale / matériel", 2, 2}
    };
    registerTemplates(eTemplates);

    const std::vector<GuildTemplate> dTemplates = {
        {"D", "Récupérer des matériaux près d'une zone instable", "Revenir avec des matériaux exploitables après plusieurs affrontements ou fouilles.", "exploration", "Matériaux / fouille", 3, 4},
        {"D", "Traquer une meute locale", "Réduire la pression d'un groupe de créatures qui rôde trop près des voyageurs.", "combat", "Créatures locales", 3, 4},
        {"D", "Observer un monstre évolué", "Confirmer les signes d'évolution d'une créature sans mourir pour la science.", "bestiaire", "Créature évoluée", 3, 4},
        {"D", "Récupérer une dette minable", "Faire comprendre à un client que payer en chaussettes trouées ne compte pas comme une récompense.", "service", "Clientèle locale", 2, 4},
        {"D", "Patrouille des chemins secondaires", "Faire une sortie utile sur un chemin que personne ne veut surveiller parce qu'il sent mauvais.", "exploration", "Route / surveillance", 3, 5}
    };
    registerTemplates(dTemplates);

    const std::vector<GuildTemplate> cTemplates = {
        {"C", "Escorter un apprenti marchand nerveux", "Protéger un civil assez longtemps pour qu'il arrête de trembler.", "combat", "Humanoïdes / embuscades", 3, 7},
        {"C", "Fouille de ruines encadrée", "Explorer une ruine instable et revenir avec des notes utilisables par la guilde.", "exploration", "Ruines effondrées", 3, 7},
        {"C", "Contrat anti-embuscade", "Affronter plusieurs menaces intelligentes qui testent les routes commerciales.", "combat", "Humanoïdes / embuscades", 4, 7},
        {"C", "Inspection d'un ancien relais", "Vérifier un relais de guilde abandonné et noter ce qui manque avant de toucher aux trucs maudits.", "bestiaire", "Ruines / relais", 3, 8},
        {"C", "Livraison avec témoins gênants", "Aider un client officiel sans laisser les témoins empirer la situation.", "service", "Clientèle officielle", 2, 7}
    };
    registerTemplates(cTemplates);

    const std::vector<GuildTemplate> bTemplates = {
        {"B", "Traquer une menace signalée par la guilde", "Identifier puis vaincre une menace plus sérieuse.", "combat", "Élite / menace", 4, 11},
        {"B", "Mini-boss régional", "Forcer une menace locale à se montrer et survivre au rapport de mission.", "combat", "Mini-boss / menace évoluée", 4, 11},
        {"B", "Zone dangereuse sous surveillance", "Revenir d'un lieu dangereux avec assez d'informations pour éviter un massacre de novices.", "exploration", "Menace avancée", 4, 11},
        {"B", "Prime silencieuse", "Régler une affaire que la guilde refuse d'écrire trop clairement sur le panneau public.", "combat", "Menace avancée", 4, 13},
        {"B", "Dossier de terrain incomplet", "Compléter des informations dangereuses sans offrir ton cadavre comme source supplémentaire.", "bestiaire", "Bestiaire avancé", 4, 12}
    };
    registerTemplates(bTemplates);

    addGuildQuestIfAvailable(board, "guild_f_dynamic", playerLevel, fTemplates);
    addGuildQuestIfAvailable(board, "guild_e_dynamic", playerLevel, eTemplates);
    addGuildQuestIfAvailable(board, "guild_d_dynamic", playerLevel, dTemplates);
    addGuildQuestIfAvailable(board, "guild_c_dynamic", playerLevel, cTemplates);
    addGuildQuestIfAvailable(board, "guild_b_dynamic", playerLevel, bTemplates);

    if (playerLevel >= 16)
    {
        const std::vector<GuildTemplate> aTemplates = {
            {"A", "Contrat dangereux de la guilde", "Enchaîner plusieurs sorties contre des adversaires solides sans abandonner le contrat.", "combat", "Menace avancée", 5, 16},
            {"A", "Anomalie régionale mineure", "Explorer et stabiliser une variation anormale avant qu'elle n'attire un vrai boss.", "exploration", "Variation d'énergie", 5, 16},
            {"A", "Chasse d'élite", "Affronter une menace évoluée qui a déjà survécu à plusieurs groupes.", "combat", "Mini-boss / menace évoluée", 5, 18},
            {"A", "Client trop important pour paniquer", "Résoudre une affaire de client influent sans ruiner la réputation de la guilde.", "service", "Clientèle noble", 3, 17}
        };
        registerTemplates(aTemplates);

        addGuildQuestIfAvailable(board, "guild_a_dynamic", playerLevel, aTemplates);
    }

    if (playerLevel >= 24)
    {
        const std::vector<GuildTemplate> sTemplates = {
            {"S", "Mission classée S", "Un contrat que la guilde ne donne pas aux aventuriers qui tiennent à leurs dents.", "combat", "Menace majeure", 6, 24},
            {"S", "Registre interdit", "Vérifier une entrée qui ne devrait pas apparaître dans un registre public.", "bestiaire", "Boss potentiel / variation majeure", 6, 24},
            {"S", "Route condamnée", "Nettoyer une zone où la guilde a déjà perdu trop de noms.", "combat", "Menace majeure", 7, 26},
            {"SS", "Ordre scellé de la guilde", "Un contrat au-dessus du rang S classique, réservé aux noms que la guilde n'enterre pas à la légère.", "combat", "Menace catastrophique", 8, 32},
            {"SS", "Caravane sous sceau noir", "Escorter un convoi que même les vétérans refusent de regarder trop longtemps.", "exploration", "Menace catastrophique", 7, 32}
        };
        registerTemplates(sTemplates);

        addGuildQuestIfAvailable(board, "guild_s_dynamic", playerLevel, sTemplates);
    }

    if (playerLevel >= 42)
    {
        const std::vector<GuildTemplate> sssTemplates = {
            {"SSS", "Éradication d'une zone morte", "Entrer dans une zone que la guilde a déjà rayée de ses cartes et revenir avec une preuve de nettoyage.", "combat", "Zone morte", 9, 42},
            {"SSS", "Archive qui respire encore", "Récupérer un registre vivant sans le laisser écrire ton nom à l'intérieur.", "bestiaire", "Archive vivante", 8, 42}
        };
        registerTemplates(sssTemplates);

        addGuildQuestIfAvailable(board, "guild_sss_dynamic", playerLevel, sssTemplates);
    }

    if (playerLevel >= 55)
    {
        const std::vector<GuildTemplate> heroTemplates = {
            {"Héros mondial", "Contrat de héros mondial", "Répondre à une menace dont l'échec serait raconté dans plusieurs royaumes.", "combat", "Menace mondiale", 10, 55},
            {"Héros mondial", "Serment sous plusieurs bannières", "Porter une mission signée par plusieurs autorités sans laisser la politique tuer les civils.", "service", "Clientèle royale", 4, 55}
        };
        registerTemplates(heroTemplates);

        addGuildQuestIfAvailable(board, "guild_world_hero_dynamic", playerLevel, heroTemplates);
    }

    if (playerLevel >= 70)
    {
        const std::vector<GuildTemplate> legendTemplates = {
            {"Légende", "Contrat réservé aux légendes", "S'occuper d'une menace que la guilde ne décrit plus aux aventuriers normaux.", "combat", "Menace légendaire", 11, 70},
            {"Légende", "Cartographie d'un lieu impossible", "Revenir d'un endroit qui change de forme dès que quelqu'un affirme l'avoir compris.", "exploration", "Lieu impossible", 9, 70}
        };
        registerTemplates(legendTemplates);

        addGuildQuestIfAvailable(board, "guild_legend_dynamic", playerLevel, legendTemplates);
    }

    if (playerLevel >= 90)
    {
        const std::vector<GuildTemplate> godTemplates = {
            {"Dieu", "Demande que personne ne devrait accepter", "Approcher une anomalie de rang divin sans confondre courage et suicide.", "bestiaire", "Anomalie divine", 12, 90},
            {"Dieu", "Dernière ligne d'un registre brûlé", "Traiter une menace que les maîtres de guilde ne prononcent qu'une fois la porte fermée.", "combat", "Menace divine", 12, 95}
        };
        registerTemplates(godTemplates);

        addGuildQuestIfAvailable(board, "guild_god_dynamic", playerLevel, godTemplates);
    }

    const int desiredBoardSize = randomBetween(3, 8);
    int fillAttempts = 0;

    while (static_cast<int>(board.size()) < desiredBoardSize && fillAttempts < 24)
    {
        fillAttempts++;
        addGuildQuestIfAvailable(board, "guild_extra_dynamic", playerLevel, fillerTemplates);
    }

    std::shuffle(board.begin(), board.end(), questGenerator());

    if (!board.empty())
    {
        bool onlyServices = true;
        for (const Quest& quest : board)
        {
            if (quest.objectiveType != "service")
            {
                onlyServices = false;
                break;
            }
        }

        if (onlyServices)
        {
            for (const GuildTemplate& candidate : fillerTemplates)
            {
                if (candidate.type == "service" || !hasAvailableGuildTemplate(std::vector<GuildTemplate>{candidate}, playerLevel))
                {
                    continue;
                }

                if (!boardAlreadyHasTitle(board, candidate.title))
                {
                    board[0] = buildGuildQuest("guild_forced_playable", playerLevel, candidate);
                    break;
                }
            }
        }
    }

    if (static_cast<int>(board.size()) > desiredBoardSize)
    {
        board.resize(desiredBoardSize);
    }

    return board;
}

// EN: createForgemasterMaterialRequest declares or implements a focused behavior used by this module.
// FR: createForgemasterMaterialRequest déclare ou implémente un comportement précis utilisé par ce module.
Quest QuestCatalog::createForgemasterMaterialRequest(int playerLevel)
{
    std::vector<RequestedMaterial> materials = {
        {"rusted_metal_fragment", "Fragment de métal rouillé", 3, 1},
        {"worn_leather_piece", "Morceau de cuir abîmé", 3, 1},
        {"battle_torn_badge", "Insigne abîmé d'aventurier", 2, 2},
        {"beast_hide", "Peau de bête robuste", 2, 3},
        {"draconic_scale_fragment", "Fragment d'écaille draconique", 1, 6}
    };

    RequestedMaterial material = chooseMaterial(materials, playerLevel);
    int quantity = materialQuantity(material, playerLevel);
    std::string rank = materialRank(playerLevel, 4);

    return buildQuest(
        questIdWithMaterial("npc_forge_materials", material, playerLevel), rank, "Commande du forgeron", "PNJ client", "Forgeron", "Forge",
        "Récupérer des matériaux utiles à la forge, puis revenir les vendre au forgeron.",
        "livraison", "Métal / cuir / forge", questExperience(rank, playerLevel, quantity), questGold(rank, playerLevel, quantity), quantity, false,
        material.id, material.name, quantity
    );
}

// EN: createAlchemistIngredientRequest declares or implements a focused behavior used by this module.
// FR: createAlchemistIngredientRequest déclare ou implémente un comportement précis utilisé par ce module.
Quest QuestCatalog::createAlchemistIngredientRequest(int playerLevel)
{
    std::vector<RequestedMaterial> materials = {
        {"bitter_healing_leaf", "Feuille amère de soin", 3, 1},
        {"mountain_blue_flower", "Fleur bleue de montagne", 2, 1},
        {"slime_residue", "Résidu de slime", 3, 1},
        {"arcane_dust", "Poussière arcanique", 2, 2},
        {"kitsune_ember", "Braise kitsune", 1, 5},
        {"unstable_core", "Noyau instable", 1, 7}
    };

    RequestedMaterial material = chooseMaterial(materials, playerLevel);
    int quantity = materialQuantity(material, playerLevel);
    std::string rank = materialRank(playerLevel, 3);

    return buildQuest(
        questIdWithMaterial("npc_alchemist_ingredients", material, playerLevel), rank, "Ingrédients instables", "PNJ client", "Alchimiste", "Herboristerie",
        "Trouver des plantes, résidus ou catalyseurs utiles pour une préparation dangereusement intéressante.",
        "livraison", "Plantes / alchimie", questExperience(rank, playerLevel, quantity), questGold(rank, playerLevel, quantity), quantity, false,
        material.id, material.name, quantity
    );
}

// EN: createVillagerMonsterFearRequest declares or implements a focused behavior used by this module.
// FR: createVillagerMonsterFearRequest déclare ou implémente un comportement précis utilisé par ce module.
Quest QuestCatalog::createVillagerMonsterFearRequest(int playerLevel)
{
    std::string rank = playerLevel >= 6 ? "D" : "F";
    int target = playerLevel >= 6 ? 3 : 2;

    return buildQuest(
        "npc_villager_fear_" + std::to_string(playerLevel) + "_" + std::to_string(randomBetween(100, 999)),
        rank, "Peur de villageois", "Événement personnel", "Villageois nerveux", "Place du village",
        "Éliminer ou éloigner des monstres qui terrorisent un habitant. Ses raisons peuvent être valables... ou pas très glorieuses.",
        "combat", "Créatures locales", questExperience(rank, playerLevel, target), questGold(rank, playerLevel, target), target, false
    );
}

// EN: createMerchantDeliveryRequest declares or implements a focused behavior used by this module.
// FR: createMerchantDeliveryRequest déclare ou implémente un comportement précis utilisé par ce module.
Quest QuestCatalog::createMerchantDeliveryRequest(int playerLevel)
{
    std::string rank = playerLevel >= 5 ? "D" : "E";
    int target = playerLevel >= 7 ? 2 : 1;

    return buildQuest(
        "npc_merchant_delivery_" + std::to_string(playerLevel) + "_" + std::to_string(randomBetween(100, 999)),
        rank, "Marchandise en retard", "PNJ client", "Marchand inquiet", "Route commerciale",
        "Récupérer une caisse perdue avant que quelqu'un de moins honnête ne la trouve.",
        "exploration", "Route commerciale", questExperience(rank, playerLevel, target), questGold(rank, playerLevel, target), target, false
    );
}

// EN: createMonsterMaterialVendorRequest declares or implements a focused behavior used by this module.
// FR: createMonsterMaterialVendorRequest déclare ou implémente un comportement précis utilisé par ce module.
Quest QuestCatalog::createMonsterMaterialVendorRequest(int playerLevel)
{
    std::vector<RequestedMaterial> materials = {
        {"goblin_ear", "Oreille de gobelin", 3, 1},
        {"wolf_fang", "Croc de loup", 2, 1},
        {"cracked_bone", "Os fissuré", 2, 2},
        {"slime_residue", "Résidu de slime", 3, 2},
        {"shadow_thread", "Fil d'ombre", 1, 5}
    };

    RequestedMaterial material = chooseMaterial(materials, playerLevel);
    int quantity = materialQuantity(material, playerLevel);
    std::string rank = materialRank(playerLevel, 3);

    return buildQuest(
        questIdWithMaterial("npc_monster_vendor", material, playerLevel), rank, "Commande de composants de monstres", "PNJ client", "Vendeur de composants", "Boutique de monstres",
        "Rapporter des composants récupérés sur des créatures pour alimenter les contrats et les recettes étranges.",
        "livraison", "Composants de monstres", questExperience(rank, playerLevel, quantity), questGold(rank, playerLevel, quantity), quantity, false,
        material.id, material.name, quantity
    );
}

// EN: createMaterialVendorRequest declares or implements a focused behavior used by this module.
// FR: createMaterialVendorRequest déclare ou implémente un comportement précis utilisé par ce module.
Quest QuestCatalog::createMaterialVendorRequest(int playerLevel)
{
    std::vector<RequestedMaterial> materials = {
        {"rusted_metal_fragment", "Fragment de métal rouillé", 4, 1},
        {"worn_leather_piece", "Morceau de cuir abîmé", 3, 1},
        {"beast_hide", "Peau de bête robuste", 2, 3},
        {"arcane_dust", "Poussière arcanique", 2, 4}
    };

    RequestedMaterial material = chooseMaterial(materials, playerLevel);
    int quantity = materialQuantity(material, playerLevel);
    std::string rank = materialRank(playerLevel, 3);

    return buildQuest(
        questIdWithMaterial("npc_material_vendor", material, playerLevel), rank, "Réassort de matériaux", "PNJ client", "Vendeur de matériaux", "Boutique de matériaux",
        "Aider le vendeur à refaire son stock avec des matériaux exploitables.",
        "livraison", "Matériaux généraux", questExperience(rank, playerLevel, quantity), questGold(rank, playerLevel, quantity), quantity, false,
        material.id, material.name, quantity
    );
}

// EN: createHerbalistRequest declares or implements a focused behavior used by this module.
// FR: createHerbalistRequest déclare ou implémente un comportement précis utilisé par ce module.
Quest QuestCatalog::createHerbalistRequest(int playerLevel)
{
    std::vector<RequestedMaterial> materials = {
        {"bitter_healing_leaf", "Feuille amère de soin", 4, 1},
        {"mountain_blue_flower", "Fleur bleue de montagne", 2, 1},
        {"kitsune_ember", "Braise kitsune", 1, 6}
    };

    RequestedMaterial material = chooseMaterial(materials, playerLevel);
    int quantity = materialQuantity(material, playerLevel);
    std::string rank = materialRank(playerLevel, 3);

    return buildQuest(
        questIdWithMaterial("npc_herbalist", material, playerLevel), rank, "Commande de l'herboriste", "PNJ client", "Herboriste", "Herboristerie",
        "Trouver des plantes ou composants naturels pour préparer des remèdes.",
        "livraison", "Plantes", questExperience(rank, playerLevel, quantity), questGold(rank, playerLevel, quantity), quantity, false,
        material.id, material.name, quantity
    );
}

// EN: createArmorerRequest declares or implements a focused behavior used by this module.
// FR: createArmorerRequest déclare ou implémente un comportement précis utilisé par ce module.
Quest QuestCatalog::createArmorerRequest(int playerLevel)
{
    std::vector<RequestedMaterial> materials = {
        {"worn_leather_piece", "Morceau de cuir abîmé", 4, 1},
        {"beast_hide", "Peau de bête robuste", 2, 3},
        {"draconic_scale_fragment", "Fragment d'écaille draconique", 1, 7},
        {"atlas_broken_plate", "Plaque brisée d'Atlas", 1, 10}
    };

    RequestedMaterial material = chooseMaterial(materials, playerLevel);
    int quantity = materialQuantity(material, playerLevel);
    std::string rank = materialRank(playerLevel, 4);

    return buildQuest(
        questIdWithMaterial("npc_armorer", material, playerLevel), rank, "Pièces pour armures", "PNJ client", "Armurier", "Armurerie défensive",
        "Ramener des matériaux défensifs pour réparer ou renforcer des armures.",
        "livraison", "Armures / défense", questExperience(rank, playerLevel, quantity), questGold(rank, playerLevel, quantity), quantity, false,
        material.id, material.name, quantity
    );
}

// EN: createWeaponVendorRequest declares or implements a focused behavior used by this module.
// FR: createWeaponVendorRequest déclare ou implémente un comportement précis utilisé par ce module.
Quest QuestCatalog::createWeaponVendorRequest(int playerLevel)
{
    std::vector<RequestedMaterial> materials = {
        {"rusted_metal_fragment", "Fragment de métal rouillé", 4, 1},
        {"wolf_fang", "Croc de loup", 2, 2},
        {"arcane_dust", "Poussière arcanique", 2, 4},
        {"zelef_demon_blood", "Sang démoniaque de Zelef", 1, 10}
    };

    RequestedMaterial material = chooseMaterial(materials, playerLevel);
    int quantity = materialQuantity(material, playerLevel);
    std::string rank = materialRank(playerLevel, 4);

    return buildQuest(
        questIdWithMaterial("npc_weapon_vendor", material, playerLevel), rank, "Approvisionnement de forge d'armes", "PNJ client", "Vendeur d'armes", "Forge d'armes",
        "Fournir de quoi entretenir ou améliorer les armes vendues aux aventuriers.",
        "livraison", "Armes", questExperience(rank, playerLevel, quantity), questGold(rank, playerLevel, quantity), quantity, false,
        material.id, material.name, quantity
    );
}

// EN: createConsumableVendorRequest declares or implements a focused behavior used by this module.
// FR: createConsumableVendorRequest déclare ou implémente un comportement précis utilisé par ce module.
Quest QuestCatalog::createConsumableVendorRequest(int playerLevel)
{
    std::vector<RequestedMaterial> materials = {
        {"bitter_healing_leaf", "Feuille amère de soin", 4, 1},
        {"slime_residue", "Résidu de slime", 3, 1},
        {"arcane_dust", "Poussière arcanique", 2, 3},
        {"unstable_core", "Noyau instable", 1, 6}
    };

    RequestedMaterial material = chooseMaterial(materials, playerLevel);
    int quantity = materialQuantity(material, playerLevel);
    std::string rank = materialRank(playerLevel, 3);

    return buildQuest(
        questIdWithMaterial("npc_consumable_vendor", material, playerLevel), rank, "Stocks de consommables", "PNJ client", "Vendeur de consommables", "Boutique de consommables",
        "Rapporter des ingrédients pour que les consommables ne coûtent pas un rein à tout le monde.",
        "livraison", "Consommables", questExperience(rank, playerLevel, quantity), questGold(rank, playerLevel, quantity), quantity, false,
        material.id, material.name, quantity
    );
}

// EN: createLibrarianRequest declares or implements a focused behavior used by this module.
// FR: createLibrarianRequest déclare ou implémente un comportement précis utilisé par ce module.
Quest QuestCatalog::createLibrarianRequest(int playerLevel)
{
    std::string rank = playerLevel >= 8 ? "C" : (playerLevel >= 4 ? "D" : "E");
    int target = 2 + (playerLevel >= 5 ? 1 : 0) + (playerLevel >= 10 ? 1 : 0);

    return buildQuest(
        "npc_librarian_notes_" + std::to_string(playerLevel) + "_" + std::to_string(randomBetween(100, 999)),
        rank, "Notes à vérifier", "PNJ client", "Bibliothécaire", "Bibliothèque",
        "Sortir sur le terrain pour confirmer des informations de bestiaire que personne n'a pris le temps de vérifier proprement.",
        "bestiaire", "Bestiaire / observation", questExperience(rank, playerLevel, target), questGold(rank, playerLevel, target), target, false
    );
}


// EN: createBiomeRequest declares or implements a focused behavior used by this module.
// FR: createBiomeRequest déclare ou implémente un comportement précis utilisé par ce module.
Quest QuestCatalog::createBiomeRequest(int playerLevel, const std::string& biomeName, const std::string& preferredClient)
{
    BiomeQuestProfile profile = profileForBiome(biomeName);
    bool materialQuest = randomBetween(1, 100) <= 45;
    bool combatQuest = randomBetween(1, 100) <= 50;

    std::string client = preferredClient.empty() ? chooseText({
        "Éclaireur de guilde", "Villageois nerveux", "Marchand inquiet", "Herboriste", "Bibliothécaire", "Vendeur de composants"
    }) : preferredClient;

    if (client == "Forgeron" || client == "Armurier" || client == "Vendeur d'armes")
    {
        materialQuest = true;
    }

    if (client == "Bibliothécaire")
    {
        materialQuest = false;
        combatQuest = false;
    }

    std::string rank = playerLevel >= 12 ? "B" : (playerLevel >= 8 ? "C" : (playerLevel >= 5 ? "D" : "E"));
    int target = 2 + (playerLevel >= 6 ? 1 : 0) + (playerLevel >= 12 ? 1 : 0);

    if (materialQuest)
    {
        RequestedMaterial material = chooseMaterial(profile.materials, playerLevel);
        int quantity = materialQuantity(material, playerLevel);
        std::string title = chooseText({
            "Demande locale : " + profile.location,
            "Ressource recherchée — " + profile.location,
            "Commande ciblée du biome",
            "Prélèvement utile en zone connue"
        });

        return buildQuest(
            questIdWithMaterial("npc_biome_material", material, playerLevel), rank, title,
            "Demande de biome", client, profile.location,
            "Rapporter " + material.name + " depuis " + profile.location + ". La demande est liée au terrain, pas à une liste générique.",
            "livraison", profile.family,
            questExperience(rank, playerLevel, quantity + 1), questGold(rank, playerLevel, quantity + 1), quantity, false,
            material.id, material.name, quantity
        );
    }

    if (combatQuest)
    {
        std::string title = chooseText({
            "Menace locale — " + profile.location,
            "Contrat de terrain ciblé",
            "Présence hostile confirmée",
            "Nettoyage prudent du biome"
        });

        return buildQuest(
            questId("npc_biome_combat", playerLevel), rank, title,
            "Demande de biome", client, profile.location,
            chooseText(profile.combatObjectives),
            "combat", profile.family,
            questExperience(rank, playerLevel, target), questGold(rank, playerLevel, target), target, false
        );
    }

    std::string title = chooseText({
        "Notes de terrain — " + profile.location,
        "Observation ciblée du biome",
        "Carte incomplète",
        "Hypothèse à vérifier"
    });

    return buildQuest(
        questId("npc_biome_exploration", playerLevel), rank, title,
        "Demande de biome", client, profile.location,
        chooseText(profile.explorationObjectives),
        client == "Bibliothécaire" ? "bestiaire" : "exploration", profile.family,
        questExperience(rank, playerLevel, target), questGold(rank, playerLevel, target), target, false
    );
}
