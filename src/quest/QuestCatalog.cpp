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

    int balancedQuestExperienceForType(
        const std::string& rank,
        const std::string& objectiveType,
        int originalExperience,
        int target
    );

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
        quest.rewardExperience = balancedQuestExperienceForType(rank, objectiveType, experience, target);
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
        return 10 + playerLevel * 5 + target * 6 + rankPower(rank) * 10;
    }

    int balancedQuestExperienceForType(
        const std::string& rank,
        const std::string& objectiveType,
        int originalExperience,
        int target
    )
    {
        if (originalExperience <= 0)
        {
            return 0;
        }

        const int power = rankPower(rank);
        int balanced = originalExperience;

        // FR: Les petites quêtes de service doivent rester utiles, pas devenir le meilleur farm XP.
        // EN: Small service quests should be useful, not the best XP farm.
        if (objectiveType == "service")
        {
            balanced = std::min(originalExperience, 2 + target + power * 2);
        }
        else if (objectiveType == "livraison")
        {
            balanced = originalExperience * 42 / 100;
        }
        else if (objectiveType == "bestiaire")
        {
            balanced = originalExperience * 40 / 100;
        }
        else if (objectiveType == "exploration")
        {
            balanced = originalExperience * 52 / 100;
        }
        else if (objectiveType == "material")
        {
            balanced = originalExperience * 48 / 100;
        }

        return std::max(1, balanced);
    }

    // EN: questGold declares or implements a focused behavior used by this module.
    // FR: questGold déclare ou implémente un comportement précis utilisé par ce module.
    int questGold(const std::string& rank, int playerLevel, int target)
    {
        // FR: base volontairement prudente : l'or doit suivre les prix de boutique sans exploser après quelques événements chanceux.
        // EN: intentionally careful base: gold faucets should follow shop prices without exploding after a few lucky events.
        int value = 4 + playerLevel * 2 + target * 3 + rankPower(rank) * 4;
        return std::max(0, value);
    }

    int adjustedQuestGold(const std::string& rank, int playerLevel, int target, const std::string& objectiveType, bool givesObjectReward)
    {
        int value = questGold(rank, playerLevel, target);

        if (objectiveType == "livraison") value = value * 52 / 100;
        if (objectiveType == "bestiaire") value = value * 45 / 100;
        if (objectiveType == "service") value = value * 22 / 100;
        if (givesObjectReward) value = value * 60 / 100;

        int cap = 45 + playerLevel * 7 + rankPower(rank) * 18 + target * 2;
        if (objectiveType == "service") cap = cap * 42 / 100;
        if (objectiveType == "livraison") cap = cap * 65 / 100;
        if (objectiveType == "bestiaire") cap = cap * 58 / 100;
        if (objectiveType == "material") cap = cap * 70 / 100;
        if (objectiveType == "exploration") cap = cap * 85 / 100;
        if (givesObjectReward) cap = cap * 72 / 100;

        if (value > cap)
        {
            value = cap + (value - cap) / 5;
        }

        int hardCap = 38 + playerLevel * 5 + rankPower(rank) * 12 + target * 3;
        if (objectiveType == "service") hardCap = 10 + playerLevel * 2 + rankPower(rank) * 4 + target;
        if (objectiveType == "livraison") hardCap = 24 + playerLevel * 3 + rankPower(rank) * 6 + target * 2;
        if (objectiveType == "bestiaire") hardCap = 22 + playerLevel * 3 + rankPower(rank) * 6 + target * 2;
        if (objectiveType == "material") hardCap = 30 + playerLevel * 4 + rankPower(rank) * 8 + target * 2;
        if (objectiveType == "exploration") hardCap = 42 + playerLevel * 5 + rankPower(rank) * 10 + target * 3;
        if (givesObjectReward) hardCap = hardCap * 80 / 100;

        value = std::min(value, hardCap);
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

    int countedHuntTargetForRank(const std::string& rank, int playerLevel, int currentTarget)
    {
        int target = currentTarget;
        const int power = rankPower(rank);

        if (power <= 1) target = std::max(target, 2);
        else if (power == 2) target = std::max(target, 3);
        else if (power == 3) target = std::max(target, 4);
        else if (power == 4) target = std::max(target, 5);
        else if (power == 5) target = std::max(target, 6);
        else if (power <= 7) target = std::max(target, 7);
        else if (power <= 10) target = std::max(target, 8);
        else if (power <= 14) target = std::max(target, 10);
        else if (power <= 18) target = std::max(target, 12);
        else if (power <= 22) target = std::max(target, 14);
        else if (power <= 28) target = std::max(target, 16);
        else target = std::max(target, 20);

        if (rank.find('+') != std::string::npos)
        {
            target += 1;
        }
        else if (rank.find('-') != std::string::npos && target > 2)
        {
            target -= 1;
        }

        if (playerLevel >= 12 && power <= 5)
        {
            target += 1;
        }

        return std::max(1, target);
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

    bool questCatalogTextContains(const std::string& haystack, const std::string& needle);
    bool isCountedHuntTemplate(const GuildTemplate& questTemplate);

    ExtraReward chooseHuntTrophyReward(const GuildTemplate& questTemplate, int playerLevel)
    {
        ExtraReward reward;
        const std::string combined = questTemplate.title + " " + questTemplate.objective + " " + questTemplate.family;

        auto setReward = [&reward](const std::string& id, const std::string& name, int quantity, const std::string& note) {
            reward.materialId = id;
            reward.materialName = name;
            reward.materialQuantity = quantity;
            reward.note = note;
        };

        if (questCatalogTextContains(combined, "slime"))
        {
            setReward("slime_residue", "Résidu de slime", playerLevel >= 8 ? 3 : 2,
                "Prime de chasse : la guilde garde quelques résidus collants pour l'alchimie et les réparations de fortune.");
        }
        else if (questCatalogTextContains(combined, "gobelin"))
        {
            setReward("goblin_ear", "Oreille de gobelin", 2,
                "Prime de chasse : preuve simple que la cible n'était pas juste une rumeur de route.");
        }
        else if (questCatalogTextContains(combined, "loup") || questCatalogTextContains(combined, "ours") || questCatalogTextContains(combined, "bête") || questCatalogTextContains(combined, "bete"))
        {
            setReward(playerLevel >= 10 ? "beast_hide" : "wolf_fang", playerLevel >= 10 ? "Peau de bête robuste" : "Croc de loup", playerLevel >= 10 ? 1 : 2,
                "Prime de chasse : trophée de bête utile à la forge, au cuir ou aux petites améliorations.");
        }
        else if (questCatalogTextContains(combined, "squelette") || questCatalogTextContains(combined, "goule") || questCatalogTextContains(combined, "revenant") || questCatalogTextContains(combined, "mort"))
        {
            setReward("cracked_bone", "Os fissuré", playerLevel >= 12 ? 3 : 2,
                "Prime de chasse : les os récupérés sont mis sous scellé avant usage en craft sombre ou recherche.");
        }
        else if (questCatalogTextContains(combined, "automate") || questCatalogTextContains(combined, "golem") || questCatalogTextContains(combined, "armure") || questCatalogTextContains(combined, "construction") || questCatalogTextContains(combined, "sentinelle"))
        {
            setReward(playerLevel >= 12 ? "rusted_gear_core" : "tiny_gear_spring", playerLevel >= 12 ? "Noyau d'engrenage rouillé" : "Petit ressort d'engrenage", 1,
                "Prime de chasse : les créatures artificielles laissent surtout des pièces mécaniques, pas de viande ni de cuir.");
        }
        else if (questCatalogTextContains(combined, "drake") || questCatalogTextContains(combined, "draconide") || questCatalogTextContains(combined, "dragon"))
        {
            setReward("draconic_scale_fragment", "Fragment d'écaille draconique", 1,
                "Prime de chasse : fragment draconique rare, gardé sous contrôle pour éviter les crafts absurdes trop tôt.");
        }
        else if (questCatalogTextContains(combined, "démon") || questCatalogTextContains(combined, "demon"))
        {
            setReward("arcane_dust", "Poussière arcanique", 2,
                "Prime de chasse : l'église et la guilde préfèrent transformer les restes instables en poussière contrôlée.");
        }
        else if (questCatalogTextContains(combined, "araignée") || questCatalogTextContains(combined, "araignee") || questCatalogTextContains(combined, "insect"))
        {
            setReward("venom_arrows", "Flèches enduites de venin", 1,
                "Prime de chasse : la guilde transforme le venin récupéré en munitions contrôlées plutôt que de le laisser dans une cave.");
        }

        return reward;
    }

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
            const std::string combinedExploration = questTemplate.title + " " + questTemplate.objective + " " + questTemplate.family;
            if (questCatalogTextContains(combinedExploration, "route") || questCatalogTextContains(combinedExploration, "carte") || questCatalogTextContains(combinedExploration, "repère") || questCatalogTextContains(combinedExploration, "repere"))
            {
                reward.materialId = "field_route_mark";
                reward.materialName = "Marque de route de terrain";
                reward.materialQuantity = 1;
                reward.note = "Récompense de terrain : la guilde ajoute une marque de route utile aux dossiers d'exploration.";
                return reward;
            }
            reward.materialId = playerLevel >= 10 ? "preservation_vials" : "bitter_healing_leaf";
            reward.materialName = playerLevel >= 10 ? "Fioles de conservation" : "Feuille amère de soin";
            reward.materialQuantity = playerLevel >= 10 ? 1 : 2;
            reward.note = "Récompense matérielle à la place d'une grosse bourse d'or.";
            return reward;
        }

        if (questTemplate.type == "combat" && isCountedHuntTemplate(questTemplate) && roll <= 50)
        {
            ExtraReward trophy = chooseHuntTrophyReward(questTemplate, playerLevel);
            if (!trophy.materialId.empty())
            {
                return trophy;
            }
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

    bool isCountedHuntTemplate(const GuildTemplate& questTemplate)
    {
        if (questTemplate.type != "combat")
        {
            return false;
        }

        const std::string combined = questTemplate.title + " " + questTemplate.objective + " " + questTemplate.family;
        return questCatalogTextContains(combined, "chasse")
            || questCatalogTextContains(combined, "chasser")
            || questCatalogTextContains(combined, "traquer")
            || questCatalogTextContains(combined, "nettoyer");
    }

    std::string objectiveWithHuntCount(const GuildTemplate& questTemplate, int target)
    {
        if (!isCountedHuntTemplate(questTemplate))
        {
            return questTemplate.objective;
        }

        return questTemplate.objective
            + " Objectif chiffré : vaincre " + std::to_string(target)
            + " cible" + (target > 1 ? "s" : "")
            + " de cette famille/catégorie. Le nombre demandé suit le rang réel du contrat.";
    }

    std::string objectiveWithServiceTrial(const GuildTemplate& questTemplate)
    {
        if (questTemplate.type != "service")
        {
            return questTemplate.objective;
        }

        const std::string combined = questTemplate.title + " " + questTemplate.objective + " " + questTemplate.family;

        if (questCatalogTextContains(combined, "tri de sac") || questCatalogTextContains(combined, "sac trop plein") || questCatalogTextContains(combined, "inventaire trop"))
        {
            return questTemplate.objective
                + " Épreuve : choisir quoi garder, déposer, vendre ou signaler selon poids, valeur, fragilité et utilité de quête. La réussite dépend du tri logique, pas d'un combat.";
        }

        if (questCatalogTextContains(combined, "armure mal ajustée") || questCatalogTextContains(combined, "armure mal ajustee") || questCatalogTextContains(combined, "sangles") || questCatalogTextContains(combined, "morphologie"))
        {
            return questTemplate.objective
                + " Épreuve : identifier la gêne morphologique puis proposer l'ajustement cohérent selon la race ou sous-race : ailes, queue, cornes, écailles, fourrure, taille ou masse.";
        }

        if (questCatalogTextContains(combined, "caisse de réparation") || questCatalogTextContains(combined, "reparation") || questCatalogTextContains(combined, "réparation"))
        {
            return questTemplate.objective
                + " Épreuve : classer les pièces réparables, les déchets et les composants à garder avant de livrer la caisse au bon artisan.";
        }

        return questTemplate.objective;
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

        if (questCatalogTextContains(combined, "automate") || questCatalogTextContains(combined, "golem") || questCatalogTextContains(combined, "armure") || questCatalogTextContains(combined, "sentinelle") || questCatalogTextContains(combined, "mannequin") || questCatalogTextContains(combined, "pantin"))
        {
            return "Atelier abandonné / Ruines effondrées";
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
        const int finalTarget = isCountedHuntTemplate(questTemplate)
            ? countedHuntTargetForRank(finalRank, playerLevel, questTemplate.target)
            : questTemplate.target;
        const std::string finalObjective = objectiveWithHuntCount(
            GuildTemplate{questTemplate.rank, questTemplate.title, objectiveWithServiceTrial(questTemplate), questTemplate.type, questTemplate.family, questTemplate.target, questTemplate.minLevel},
            finalTarget
        );

        return buildQuest(
            questId(idPrefix, playerLevel),
            finalRank, questTemplate.title, "Guilde", "Maître de guilde", suggestedGuildQuestLocation(questTemplate),
            finalObjective, questTemplate.type, questTemplate.family,
            questExperience(finalRank, playerLevel, finalTarget),
            adjustedQuestGold(finalRank, playerLevel, finalTarget, questTemplate.type, givesObjectReward),
            finalTarget, true,
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

    bool isActionQuestType(const std::string& objectiveType)
    {
        return objectiveType == "combat" || objectiveType == "exploration";
    }

    int actionQuestCount(const std::vector<Quest>& board)
    {
        int count = 0;
        for (const Quest& quest : board)
        {
            if (isActionQuestType(quest.objectiveType))
            {
                count++;
            }
        }
        return count;
    }

    bool isAvailableActionTemplate(const GuildTemplate& questTemplate, int playerLevel)
    {
        return isActionQuestType(questTemplate.type)
            && playerLevel >= questTemplate.minLevel
            && playerLevel >= minimumLevelForRank(questTemplate.rank);
    }

    void enforceActionQuestPresence(std::vector<Quest>& board, int playerLevel, const std::vector<GuildTemplate>& templates, int desiredMinimum)
    {
        int current = actionQuestCount(board);
        if (current >= desiredMinimum)
        {
            return;
        }

        std::vector<GuildTemplate> candidates;
        for (const GuildTemplate& questTemplate : templates)
        {
            if (isAvailableActionTemplate(questTemplate, playerLevel) && !boardAlreadyHasTitle(board, questTemplate.title))
            {
                candidates.push_back(questTemplate);
            }
        }

        while (current < desiredMinimum && !candidates.empty())
        {
            const int index = randomBetween(0, static_cast<int>(candidates.size()) - 1);
            Quest replacement = buildGuildQuest("guild_forced_action", playerLevel, candidates[index]);
            candidates.erase(candidates.begin() + index);

            bool replaced = false;
            for (Quest& quest : board)
            {
                if (!isActionQuestType(quest.objectiveType))
                {
                    quest = replacement;
                    replaced = true;
                    break;
                }
            }

            if (!replaced)
            {
                board.push_back(replacement);
            }

            current = actionQuestCount(board);
        }
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

        if (biomeName == "Bocage aux lanternes")
        {
            return {
                biomeName, "Bocage aux lanternes / plantes lumineuses",
                {{"mycelium_lantern", "Lanterne de mycélium", 3, 1}, {"echoing_resin", "Résine d'écho", 2, 3}, {"living_vine_fiber", "Fibre de vigne vivante", 2, 2}, {"mountain_blue_flower", "Fleur bleue de montagne", 1, 1}},
                {"Relever les lanternes de mycélium qui s'éteignent quand on approche trop vite.", "Cartographier une clairière lumineuse sans faire hurler les spores.", "Récupérer une trace de résine d'écho encore sonore."},
                {"Éloigner les bêtes attirées par les champignons-lampes.", "Neutraliser une plante lumineuse devenue agressive près d'un sentier."}
            };
        }

        if (biomeName == "Désert d'argile rouge")
        {
            return {
                biomeName, "Désert d'argile rouge / sel et constructions sèches",
                {{"sun_dried_clay", "Argile rouge séchée", 4, 1}, {"moonlit_salt", "Sel lunaire", 1, 4}, {"glass_map_fragment", "Fragment de carte de verre", 1, 6}, {"old_coin_bundle", "Lot de vieilles pièces", 2, 2}},
                {"Vérifier une oasis sèche avant que les traces disparaissent dans le sable rouge.", "Prélever du sel lunaire sans briser toute la plaque d'argile.", "Copier les marques laissées par les statues fendues."},
                {"Repousser des pilleurs de dunes autour d'une fausse oasis.", "Affronter une construction d'argile qui s'est réveillée trop près d'une route."}
            };
        }

        if (biomeName == "Quartier abandonné")
        {
            return {
                biomeName, "Quartier abandonné / ruelles, contrats et automates",
                {{"old_coin_bundle", "Lot de vieilles pièces", 3, 1}, {"glass_map_fragment", "Fragment de carte de verre", 1, 5}, {"inked_contract_scrap", "Morceau de contrat encré", 3, 2}, {"forgotten_camp_tag", "Plaque de camp oubliée", 2, 1}},
                {"Explorer une maison scellée sans déplacer tout ce qui devrait rester comme preuve.", "Retrouver une cache de vieilles pièces avant les gobelins serruriers.", "Relever les plans d'un quartier que personne ne revendique."},
                {"Chasser une bande de voleurs installée dans les caves.", "Désactiver un automate de boutique qui attaque les visiteurs."}
            };
        }

        if (biomeName == "Mine sifflante")
        {
            return {
                biomeName, "Mine sifflante / fer froid et machines",
                {{"cold_iron_nail", "Clou de fer froid", 3, 2}, {"tiny_gear_spring", "Petit ressort d'engrenage", 3, 2}, {"rusted_metal_fragment", "Fragment de métal rouillé", 3, 1}, {"runic_iron_shard", "Éclat de fer runique", 1, 5}},
                {"Repérer d'où vient le sifflement sans réveiller tout l'ascenseur.", "Récupérer des pièces mécaniques encore utilisables sur une vieille ligne de rails.", "Cartographier une galerie où les clous vibrent quand on parle."},
                {"Repousser une construction minière qui protège un passage.", "Stopper un contremaître gobelin avant qu'il ne transforme la galerie en piège."}
            };
        }

        if (biomeName == "Verger des lucioles de fer")
        {
            return {
                biomeName, "Verger des lucioles de fer / insectes lumineux",
                {{"firefly_iron_shell", "Carapace de luciole de fer", 3, 2}, {"luminous_moth_wing", "Aile de mite lumineuse", 2, 3}, {"cliff_basil_leaf", "Feuille de basilic des falaises", 1, 4}, {"echoing_resin", "Résine d'écho", 1, 5}},
                {"Suivre les clignotements du verger sans attirer tout l'essaim.", "Prélever des carapaces intactes avant que la lumière ne s'éteigne.", "Retrouver un arbre-lampe qui change de place entre deux rapports."},
                {"Repousser des lucioles blindées autour d'une récolte nocturne.", "Neutraliser un gardien de verger qui confond visiteur et voleur."}
            };
        }

        if (biomeName == "Archives noyées")
        {
            return {
                biomeName, "Archives noyées / encre, pages et morts-vivants",
                {{"tideworn_ink", "Encre rongée par la marée", 3, 2}, {"whispering_archive_page", "Page d'archive murmurante", 2, 4}, {"inked_contract_scrap", "Morceau de contrat encré", 2, 2}, {"old_coin_bundle", "Lot de vieilles pièces", 1, 3}},
                {"Classer des pages humides sans laisser le registre écrire ton nom.", "Récupérer une preuve administrative dans une salle qui n'a plus de sol sec.", "Relever les cotes d'archives avant que l'encre ne change de ligne."},
                {"Éloigner des scribes noyés qui protègent encore les rayons.", "Forcer une reliure carnivore à rendre les documents de la guilde."}
            };
        }

        if (biomeName == "Falaises des drakes gris")
        {
            return {
                biomeName, "Falaises des drakes gris / cordes et draconides",
                {{"salted_rope_knot", "Nœud de corde salée", 4, 2}, {"grey_drake_scale", "Écaille de drake gris", 1, 5}, {"draconic_scale_fragment", "Fragment d'écaille draconique", 1, 6}, {"mountain_blue_flower", "Fleur bleue de montagne", 1, 3}},
                {"Vérifier une ligne d'ancrage avant que les drakes ne l'utilisent comme jouet.", "Cartographier une corniche où le vent efface les traces trop vite.", "Rapporter des nœuds de corde sans finir en exemple de prudence ratée."},
                {"Repousser un jeune drake trop proche des passages d'escalade.", "Écarter des harpies qui coupent les cordes des voyageurs."}
            };
        }

        if (biomeName == "Foire abandonnée")
        {
            return {
                biomeName, "Foire abandonnée / illusions et stands oubliés",
                {{"carnival_ticket_shred", "Morceau de ticket de foire", 4, 1}, {"mirror_glass_bead", "Perle de verre miroir", 2, 3}, {"glass_map_fragment", "Fragment de carte de verre", 1, 5}, {"old_coin_bundle", "Lot de vieilles pièces", 2, 2}},
                {"Vérifier les stands dont les tickets se vendent encore sans vendeur.", "Récupérer des perles miroir avant qu'elles ne reflètent un mauvais chemin.", "Cartographier le chapiteau qui semble plus grand quand on veut sortir."},
                {"Démonter un pantin de stand devenu trop enthousiaste.", "Faire taire un maître de piste qui attire les curieux vers les miroirs."}
            };
        }

        if (biomeName == "Temple des cloches fendues")
        {
            return {
                biomeName, "Temple des cloches fendues / sanctuaire et serments",
                {{"cracked_bell_clapper", "Battant de cloche fissuré", 2, 4}, {"sanctuary_wax_seal", "Sceau de cire sanctuaire", 1, 5}, {"white_bone_chalk", "Craie d'os blanc", 2, 3}, {"arcane_dust", "Poussière arcanique", 1, 3}},
                {"Vérifier une nef dont les cloches répondent aux mensonges administratifs.", "Recopier un serment ancien sans inverser deux noms de familles.", "Récupérer un sceau de sanctuaire sans réveiller le sonneur creux."},
                {"Éloigner des gardiens de nef qui frappent avant de poser des questions.", "Faire taire une cloche animée avant qu'elle n'appelle tout le temple."}
            };
        }

        if (biomeName == "Canaux de brume bleue")
        {
            return {
                biomeName, "Canaux de brume bleue / ponts, brume et barques",
                {{"blue_mist_reed", "Roseau de brume bleue", 3, 2}, {"mistglass_pearl", "Perle de verre-brume", 1, 5}, {"tideworn_ink", "Encre rongée par la marée", 1, 4}, {"old_coin_bundle", "Lot de vieilles pièces", 2, 2}},
                {"Suivre un canal sans perdre les ponts de vue dans la brume.", "Récupérer des roseaux bleus avant que les passeurs ne les coupent tous.", "Retrouver une barque revenue sans rameur, mais avec un paquet de guilde."},
                {"Repousser des voleurs de quai embusqués dans le brouillard.", "Dissiper une brume consciente qui déplace les panneaux de route."}
            };
        }

        if (biomeName == "Carrière des os blancs")
        {
            return {
                biomeName, "Carrière des os blancs / craie, fossiles et géants",
                {{"white_bone_chalk", "Craie d'os blanc", 4, 2}, {"buried_giant_chip", "Éclat de géant enfoui", 1, 6}, {"cracked_bone", "Os fissuré", 2, 2}, {"runic_iron_shard", "Éclat de fer runique", 1, 5}},
                {"Mesurer des empreintes trop grandes sans inventer une histoire encore pire.", "Prélever de la craie rituelle dans une galerie pâle.", "Cartographier une veine blanche qui disparaît quand on parle trop fort."},
                {"Repousser des scarabées d'os autour d'un dépôt de craie.", "Briser un golem pâle qui défend une vieille trace de géant."}
            };
        }

        if (biomeName == "Marché sous les ponts")
        {
            return {
                biomeName, "Marché sous les ponts / contrebande, reçus et dettes",
                {{"smuggler_token", "Jeton de contrebandier", 3, 2}, {"sealed_debt_slip", "Billet de dette scellé", 1, 5}, {"inked_contract_scrap", "Morceau de contrat encré", 2, 2}, {"old_coin_bundle", "Lot de vieilles pièces", 2, 2}},
                {"Suivre une dette sans signer trois dettes de plus.", "Identifier un étal disparu entre deux arches.", "Récupérer un reçu avant qu'il ne soit vendu à son propre débiteur."},
                {"Chasser un collecteur masqué qui menace les petits vendeurs.", "Faire reculer des gobelins prêteurs avant que la guilde ne devienne caution."}
            };
        }

        if (biomeName == "Jardin des statues qui pleurent")
        {
            return {
                biomeName, "Jardin des statues qui pleurent / noblesse, pierre et roses",
                {{"weeping_stone_tear", "Larme de pierre pleureuse", 3, 3}, {"petrified_rose_petals", "Pétales de rose pétrifiée", 1, 5}, {"living_vine_fiber", "Fibre de vigne vivante", 2, 2}, {"glass_map_fragment", "Fragment de carte de verre", 1, 4}},
                {"Observer quelles statues changent de regard entre deux passages.", "Récolter des pétales pétrifiés sans abîmer le parterre noble.", "Relever le plan d'un jardin qui refuse d'avoir la même sortie deux fois."},
                {"Repousser des oiseaux de pierre attirés par les intrus.", "Briser une ronce blanche avant qu'elle ne scelle l'allée."}
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
        {"F", "Porter une lettre pas urgente", "Livrer un message de village sans prétendre que c'est une mission de héros.", "service", "Village / comptoir", 1, 1},
        {"F", "Retrouver une poule agressive", "Capturer une poule qui attaque les bottes des clients. Oui, la guilde note vraiment ça.", "service", "Service local / animaux", 1, 1},
        {"F", "Ramasser des herbes au bord du chemin", "Prélever quelques plantes simples sans partir jouer au héros dans les bois profonds.", "exploration", "Plaine sauvage / plantes", 1, 1},
        {"F", "Éloigner trois rats de cave", "Nettoyer une cave basique avant que le propriétaire dramatise encore plus.", "combat", "Créatures faibles", 1, 1},
        {"F", "Chercher un chat trop confiant", "Retrouver un animal parti inspecter des bottes d'aventuriers plus grandes que lui.", "service", "Service local / animaux", 1, 1},
        {"F", "Ranger les fioles vides", "Aider l'alchimiste à trier les fioles non explosives. Normalement non explosives.", "service", "Guilde / alchimie", 1, 1},
        {"F", "Vérifier le puits du hameau", "Descendre une corde, écouter les bruits et remonter avant de jouer au héros.", "exploration", "Village / puits", 1, 1},
        {"F", "Faire fuir des corbeaux voleurs", "Récupérer quelques objets brillants avant que les corbeaux ne fondent une guilde concurrente.", "combat", "Bêtes faibles", 1, 1},
        {"F", "Relire trois formulaires simples", "Corriger une faute évidente avant que la gérante ne perde encore dix minutes.", "service", "Guilde / paperasse", 1, 1},
        {"F", "Porter un sac de tickets usés", "Déplacer des tickets de foire récupérés sans les recompter trente fois.", "service", "Foire abandonnée / tri", 1, 1},
        {"F", "Allumer deux lanternes de verger", "Vérifier des lucioles de fer faibles sans entrer trop loin dans le verger.", "exploration", "Verger des lucioles de fer", 1, 1},
        {"F", "Porter un reçu sous les ponts", "Remettre un papier simple au bon comptoir sans signer une dette par accident.", "service", "Marché sous les ponts / reçu", 1, 1},
        {"F", "Balayer une marche de temple", "Vérifier les marches d'un sanctuaire sans toucher aux cloches importantes.", "service", "Temple des cloches fendues / entretien", 1, 1},
        {"F", "Compter trois roseaux bleus", "Aider un passeur à distinguer les roseaux utiles des mauvaises herbes mouillées.", "exploration", "Canaux de brume bleue", 1, 1},
        {"F", "Monnaie rendue de travers", "Vérifier une petite erreur de cuivre avant que le client et le marchand ne s'inventent une guerre.", "service", "Économie locale / monnaie", 1, 1},
        {"F", "Message de rumeur au comptoir", "Porter une rumeur de guilde sans la transformer en annonce officielle.", "service", "Ville / guilde / rumeurs", 1, 1},
        {"F", "Étiqueter trois matériaux", "Noter nom, qualité et usage probable sans vendre par erreur le seul bon composant.", "service", "Matériaux / journal", 1, 1},
        {"F", "Chasse aux rats de cave", "Éliminer une petite nuisance de cave sans transformer la cave en champ de bataille.", "combat", "Rat / Bêtes faibles", 2, 1},
        {"F", "Chasse aux gobelins trouillards", "Repousser quelques gobelins faibles qui testent les voyageurs débutants.", "combat", "Gobelin / Créatures faibles", 2, 1},
        {"F", "Tri de sac après expédition", "Aider un débutant à trier son inventaire avant qu'il ne vende par erreur son meilleur composant.", "service", "Inventaire / matériaux / débutant", 1, 1},
        {"F", "Titre trop fier pour un novice", "Expliquer à un nouvel inscrit que les titres impressionnent surtout quand les actes suivent.", "service", "Guilde / titres / réputation", 1, 1},
        {"F", "Petit délai de comptoir", "Classer trois demandes simples en urgent, normal ou reportable sans inventer une prime héroïque.", "service", "Guilde / délais / organisation", 1, 1},
        {"F", "Sac et armure avant départ", "Vérifier sac, sangles et objet de quête avant une sortie courte de débutant.", "service", "Inventaire / équipement / préparation", 1, 1},
        {"F", "Patrouille du chemin court", "Explorer une portion de route simple, noter les traces fraîches et revenir avant la nuit.", "exploration", "Route / première patrouille", 2, 1},
        {"F", "Rats sous les planches", "Descendre dans une cave basse et vaincre assez de rats pour que le propriétaire arrête de crier au monstre légendaire.", "combat", "Rat / cave / débutant", 2, 1},
        {"F", "Repères du premier sentier", "Explorer une boucle courte, marquer trois repères simples et revenir avant que la guilde lance une recherche pour toi.", "exploration", "Route / repères débutants", 2, 1},
        {"F", "Nid sous la remise", "Vaincre une petite famille de nuisibles qui grignote les réserves avant que le client n'accuse un dragon.", "combat", "Rat / insectes / réserve", 2, 1},
        {"F", "Trace dans le fossé", "Explorer un fossé proche, relever une empreinte simple et rentrer avant de suivre une piste trop grande pour ton rang.", "exploration", "Route / trace simple", 2, 1},
        {"F", "Nuisibles du petit pont", "Repousser quelques bêtes qui bloquent un pont minuscule mais très utilisé par les habitants.", "combat", "Bêtes faibles / pont", 2, 1},
        {"F", "Trace fraîche derrière l'auberge", "Suivre une piste très courte, confirmer si elle vient d'une bête ou d'un client maladroit, puis revenir sans partir trop loin.", "exploration", "Village / trace débutante", 2, 1},
        {"F", "Deux crocs sous la grange", "Vaincre une petite menace cachée sous la grange avant que les habitants ne dramatisent jusqu'au dragon.", "combat", "Bêtes faibles / grange", 2, 1}
    };
    registerTemplates(fTemplates);

    const std::vector<GuildTemplate> eTemplates = {
        {"E", "Nettoyer une petite route infestée", "Écarter quelques monstres faibles d'une route locale.", "combat", "Créatures faibles", 2, 2},
        {"E", "Cartographier un détour douteux", "Explorer une zone simple et revenir avec assez de détails pour corriger la carte de la guilde.", "exploration", "Route / exploration", 2, 2},
        {"E", "Surveiller une caisse suspecte", "Vérifier une livraison abandonnée et survivre à ce qui pourrait se cacher dedans.", "exploration", "Route commerciale", 2, 2},
        {"E", "Retrouver un client qui se cache", "Identifier un client paniqué qui doit de l'argent à trois personnes différentes.", "service", "Clientèle locale", 1, 2},
        {"E", "Récupérer des outils oubliés", "Rapporter du petit matériel abandonné près d'une zone encore raisonnable.", "exploration", "Route commerciale / matériel", 2, 2},
        {"E", "Tester un nouveau panneau de route", "Vérifier qu'un panneau placé par la guilde ne dirige pas les débutants vers un marais.", "exploration", "Route / signalisation", 2, 2},
        {"E", "Chasser les slimes du lavoir", "Nettoyer des résidus collants avant que les villageois ne perdent tout leur linge.", "combat", "Slimes faibles", 2, 2},
        {"E", "Inventaire de petite réserve", "Compter des ressources de secours et signaler les objets manquants.", "service", "Guilde / inventaire", 1, 2},
        {"E", "Escorter un apprenti livreur", "Accompagner un livreur qui court moins vite que sa peur.", "exploration", "Route / livraison", 2, 2},
        {"E", "Nettoyer un vieux grenier", "Écarter rats, chauves-souris et poussière suspecte d'une maison trop bruyante.", "combat", "Créatures faibles", 2, 2},
        {"E", "Copier des pancartes effacées", "Relever les directions d'un vieux chemin avant que quelqu'un ne tourne vers les marais.", "exploration", "Route / signalisation", 2, 2},
        {"E", "Retrouver une boîte de pansements", "Récupérer du matériel médical laissé au mauvais relais.", "service", "Guilde / médecine", 1, 2},
        {"E", "Trier les vieux tickets de foire", "Séparer les faux tickets, les vrais tickets et ceux qui changent de numéro.", "service", "Foire abandonnée / paperasse", 1, 2},
        {"E", "Prélever des ailes lumineuses", "Ramasser des composants fragiles sans les transformer en poussière triste.", "exploration", "Verger des lucioles de fer", 2, 3},
        {"E", "Inventaire des cordes de secours", "Compter et vérifier les cordes avant une sortie aux falaises.", "service", "Guilde / matériel", 1, 3},
        {"E", "Reçu taché de brume", "Retrouver à qui appartient un reçu humide avant que l'encre ne devienne abstraite.", "service", "Canaux de brume bleue / paperasse", 1, 3},
        {"E", "Petite ronde au jardin noble", "Vérifier un jardin abandonné sans accuser chaque statue d'être coupable trop vite.", "exploration", "Jardin des statues qui pleurent", 2, 3},
        {"E", "Craie pour le tableau de guilde", "Rapporter de la craie blanche assez propre pour noter les missions sans poussière maudite.", "exploration", "Carrière des os blancs", 2, 3},
        {"E", "Panneau des prix hésitant", "Comparer quelques prix de marché et signaler les écarts sans accuser tout le village d'arnaque.", "service", "Économie locale / prix", 1, 3},
        {"E", "Stock de rations trop optimiste", "Vérifier une réserve annoncée comme pleine alors que les sacs sonnent beaucoup trop creux.", "service", "Ville / stock / rations", 1, 3},
        {"E", "Qualité de cuir mélangée", "Séparer les morceaux faibles, normaux et solides avant qu'un artisan ne fabrique une armure bancale.", "service", "Matériaux / qualité", 1, 3},
        {"E", "Chasse aux slimes du lavoir", "Nettoyer les gelées faibles qui collent aux bassines et bloquent le travail du village.", "combat", "Slime / Slimes faibles", 3, 2},
        {"E", "Chasse aux loups trop proches", "Éloigner une petite meute avant qu'elle ne confonde la route et son territoire.", "combat", "Loup / Bêtes faibles", 3, 2},
        {"E", "Armure mal ajustée", "Vérifier plusieurs armures légères avant qu'une sangle mal placée ne blesse un semi-humain.", "service", "Équipement / morphologie / semi-humains", 1, 2},
        {"E", "Petite caisse de réparation", "Réunir les outils de base et noter quel équipement mérite une réparation avant le prochain départ.", "service", "Équipement / durabilité / réparation", 1, 2},
        {"E", "Clairière qui grogne", "Explorer une clairière signalée par trois témoins et revenir avec une preuve claire de ce qui rôde dedans.", "exploration", "Forêt ancienne / clairière", 2, 3},
        {"E", "Patrouille de nuit des barrières", "Tenir une courte patrouille contre des bêtes qui testent les clôtures après le coucher du soleil.", "combat", "Bêtes faibles / village", 3, 3},
        {"E", "Balises de clairière", "Explorer une clairière qui change de son entre midi et soir, puis poser des repères vérifiables.", "exploration", "Forêt / repères", 3, 3},
        {"E", "Poursuite de traces fraîches", "Suivre une piste courte et affronter ce qui laisse les marques avant que les traces se mélangent aux routes civiles.", "combat", "Bêtes / piste fraîche", 3, 3},
        {"E", "Sentier des empreintes fraîches", "Explorer un sentier où plusieurs empreintes se croisent et revenir avec la piste la plus probable, pas la plus impressionnante.", "exploration", "Route / traces croisées", 3, 3},
        {"E", "Chasse aux gobelins de talus", "Repousser plusieurs gobelins installés dans un talus qui surveille trop bien les sacs des voyageurs.", "combat", "Gobelin / talus", 3, 3}
    };
    registerTemplates(eTemplates);

    const std::vector<GuildTemplate> dTemplates = {
        {"D", "Récupérer des matériaux près d'une zone instable", "Revenir avec des matériaux exploitables après plusieurs affrontements ou fouilles.", "exploration", "Matériaux / fouille", 3, 4},
        {"D", "Traquer une meute locale", "Réduire la pression d'un groupe de créatures qui rôde trop près des voyageurs.", "combat", "Créatures locales", 3, 4},
        {"D", "Observer un monstre évolué", "Confirmer les signes d'évolution d'une créature sans mourir pour la science.", "bestiaire", "Créature évoluée", 3, 4},
        {"D", "Récupérer une dette minable", "Faire comprendre à un client que payer en chaussettes trouées ne compte pas comme une récompense.", "service", "Clientèle locale", 2, 4},
        {"D", "Patrouille des chemins secondaires", "Faire une sortie utile sur un chemin que personne ne veut surveiller parce qu'il sent mauvais.", "exploration", "Route / surveillance", 3, 5},
        {"D", "Protéger un ramasseur de plantes", "Accompagner un récolteur qui a juré que les ronces lui parlent mal.", "combat", "Forêt ancienne / plantes", 3, 4},
        {"D", "Noter les traces de mutation", "Observer des marques d'évolution sans provoquer la créature qui les a laissées.", "bestiaire", "Créature évoluée", 3, 5},
        {"D", "Réparer un relais de guilde", "Rapporter les pièces manquantes et sécuriser le relais le temps des travaux.", "exploration", "Ruines / relais", 3, 5},
        {"D", "Rapport sur un slime inhabituel", "Observer une gelée de couleur douteuse sans la laisser manger le carnet.", "bestiaire", "Slimes / observation", 3, 4},
        {"D", "Affiche de guilde qui change de ligne", "Recopier une mission locale dont le titre se décale tout seul sans accuser trop vite l'encre.", "service", "Guilde / affichage instable", 2, 5},
        {"D", "Petite escorte de forgeron", "Protéger un artisan venu récupérer des fragments trop lourds pour ses apprentis.", "combat", "Forge / route", 3, 5},
        {"D", "Débloquer une cave scellée", "Ouvrir une cave envahie et confirmer que le bruit n'est pas juste la plomberie.", "exploration", "Village / cave", 3, 4},
        {"D", "Nid de ronces au moulin", "Nettoyer des plantes agressives avant que le meunier ne perde patience.", "combat", "Plantes / village", 3, 5},
        {"D", "Page qui murmure dans le sac", "Récupérer une page d'archive avant qu'elle ne donne de mauvais conseils aux apprentis.", "exploration", "Archives noyées", 3, 5},
        {"D", "Stand de foire trop vivant", "Désactiver un pantin de stand qui vend des tickets aux voyageurs perdus.", "combat", "Foire abandonnée", 3, 5},
        {"D", "Rapport de corde effilochée", "Vérifier une ligne d'ancrage et noter précisément ce qui doit être remplacé.", "service", "Falaises / maintenance", 2, 5},
        {"D", "Cloche qui dénonce les retards", "Récupérer un battant fissuré sans faire sonner toute la nef.", "exploration", "Temple des cloches fendues", 3, 5},
        {"D", "Journal de récolte incomplet", "Reconstituer quels composants ont été abîmés par le feu, les flèches ou les coups lourds.", "bestiaire", "Matériaux / dégradation", 3, 5},
        {"D", "Dette vendue deux fois", "Comprendre un billet de dette avant que deux marchands ne réclament la même récompense.", "service", "Marché sous les ponts / dette", 2, 5},
        {"D", "Statue déplacée d'un pas", "Vérifier pourquoi une statue du jardin a changé de place pendant la nuit.", "bestiaire", "Jardin des statues qui pleurent", 3, 5},
        {"D", "Ronde de marché tendu", "Faire une ronde quand les prix montent trop vite et noter si la tension vient du stock, des routes ou des rumeurs.", "service", "Ville / marché / tension", 2, 5},
        {"D", "Contrat pour pisteur semi-humain", "Suivre une trace que les humains lisent mal sans transformer le passif racial en solution magique.", "exploration", "Route / traces / semi-humains", 3, 5},
        {"D", "Chasse aux squelettes fissurés", "Nettoyer les os qui se relèvent près des vieilles pierres avant qu'ils ne forment une vraie patrouille.", "combat", "Squelette / Morts-vivants et reliques", 4, 4},
        {"D", "Chasse aux araignées de cave", "Réduire un nid d'insectoïdes agressifs sans brûler toute la réserve autour.", "combat", "Araignée / Insectoïdes", 4, 4},
        {"D", "Chasse aux pantins animés", "Désactiver des mannequins ou pantins de foire avant qu'ils ne piègent des débutants.", "combat", "Automate / golem / armure animée", 4, 5},
        {"D", "Atelier d'entretien de terrain", "Comparer armes usées, armures fatiguées et matériaux de réparation sans promettre une durabilité magique.", "service", "Équipement / durabilité / atelier", 2, 5},
        {"D", "Bestiaire des matériaux communs", "Relier trois composants à leur famille de monstre sans inventer une faiblesse gratuite.", "bestiaire", "Bestiaire / matériaux / statistiques", 3, 5},
        {"D", "Sentier aux traces croisées", "Explorer un chemin où deux familles de monstres se disputent le même territoire sans suivre la mauvaise piste.", "exploration", "Route / traces croisées", 3, 5},
        {"D", "Ronde contre les pillards nerveux", "Affronter un petit groupe organisé avant qu'il ne comprenne que la route est moins protégée que prévu.", "combat", "Bandit / route", 4, 5},
        {"D", "Carte tachée de boue", "Explorer une ancienne portion de route dont les repères ont disparu sous pluie, boue et mensonges de voyageurs.", "exploration", "Route / pluie / repères", 4, 5},
        {"D", "Chasse avant la foire", "Nettoyer une menace mobile avant l'arrivée d'une petite foire locale et rapporter assez de preuves pour rassurer les gardes.", "combat", "Bêtes / village / foire", 4, 5},
        {"D", "Bosquet aux hurlements courts", "Explorer un bosquet où les cris s'arrêtent dès qu'une lanterne approche, puis marquer les sorties sûres.", "exploration", "Forêt / cris courts", 4, 5},
        {"D", "Chasse au vieux moulin", "Nettoyer les abords d'un moulin où les monstres utilisent le bruit des pales pour cacher leurs déplacements.", "combat", "Créatures locales / moulin", 4, 5}
    };
    registerTemplates(dTemplates);

    const std::vector<GuildTemplate> cTemplates = {
        {"C", "Escorter un apprenti marchand nerveux", "Protéger un civil assez longtemps pour qu'il arrête de trembler.", "combat", "Humanoïdes / embuscades", 3, 7},
        {"C", "Fouille de ruines encadrée", "Explorer une ruine instable et revenir avec des notes utilisables par la guilde.", "exploration", "Ruines effondrées", 3, 7},
        {"C", "Contrat anti-embuscade", "Affronter plusieurs menaces intelligentes qui testent les routes commerciales.", "combat", "Humanoïdes / embuscades", 4, 7},
        {"C", "Inspection d'un ancien relais", "Vérifier un relais de guilde abandonné et noter ce qui manque avant de toucher aux trucs maudits.", "bestiaire", "Ruines / relais", 3, 8},
        {"C", "Livraison avec témoins gênants", "Aider un client officiel sans laisser les témoins empirer la situation.", "service", "Clientèle officielle", 2, 7},
        {"C", "Échantillon de slime suspect", "Récupérer une matière collante qui semble digérer l'étiquette de mission.", "exploration", "Marais trouble / slimes", 3, 7},
        {"C", "Arbitrage de marchands armés", "Empêcher deux groupes de s'entretuer pour une cargaison probablement banale.", "service", "Route commerciale / conflit", 2, 8},
        {"C", "Chasse dans les vieilles pierres", "Nettoyer une alcôve où quelque chose se réveille dès qu'on prononce le mot trésor.", "combat", "Ruines effondrées", 4, 8},
        {"C", "Convoi de remèdes fragiles", "Faire passer des fioles utiles par une route que les bandits trouvent très intéressante.", "exploration", "Route commerciale / convoi", 3, 7},
        {"C", "Bestiaire des insectes agressifs", "Confirmer les espèces toxiques qui rôdent près des points d'eau.", "bestiaire", "Insectoïdes / poison", 3, 8},
        {"C", "Duel de faux péagers", "Démanteler un poste de péage inventé par des gens trop créatifs.", "combat", "Humanoïdes / route", 4, 8},
        {"C", "Caveau qui répond", "Explorer un caveau dont les échos n'ont pas le même nombre de voix que le groupe.", "exploration", "Cimetière oublié", 3, 8},
        {"C", "Greffier noyé en retard", "Récupérer un registre dans une archive où les morts-vivants respectent encore les horaires.", "combat", "Archives noyées", 4, 8},
        {"C", "Essaim dans les pommiers-lampes", "Éloigner des lucioles blindées avant qu'elles ne mangent les lanternes de la route.", "combat", "Verger des lucioles de fer", 4, 8},
        {"C", "Miroir qui corrige les témoins", "Étudier un reflet qui répond avant les personnes interrogées.", "bestiaire", "Foire abandonnée / illusions", 3, 8},
        {"C", "Panneau de guilde brouillé", "Vérifier plusieurs contrats dont les cibles se remplacent entre deux lectures.", "bestiaire", "Anomalie mineure / interface", 3, 8},
        {"C", "Passeur disparu dans la brume", "Suivre une piste de barque sans laisser la brume choisir la destination.", "exploration", "Canaux de brume bleue", 3, 8},
        {"C", "Autel aux reçus scellés", "Classer des sceaux de sanctuaire avant qu'un ancien serment ne réclame des intérêts.", "service", "Temple des cloches fendues / archives", 2, 8},
        {"C", "Golem de craie réveillé", "Neutraliser une forme pâle qui s'anime dès qu'on trace un cercle complet.", "combat", "Carrière des os blancs", 4, 8},
        {"C", "Audit des primes de guilde", "Vérifier que les récompenses proposées suivent le danger réel au lieu de suivre la panique du client.", "service", "Guilde / économie / primes", 2, 8},
        {"C", "Dialogue de sous-race mal parti", "Calmer une dispute née d'une remarque stupide sur un semi-humain avant que la guilde ne doive payer les dégâts.", "service", "Semi-humains / ville", 2, 8},
        {"C", "Chasse aux goules affamées", "Vider une zone de goules assez nombreuses pour inquiéter les fossoyeurs.", "combat", "Goule / Morts-vivants et reliques", 5, 7},
        {"C", "Chasse aux bandits de route", "Réduire une bande d'humanoïdes qui confond péage et agression organisée.", "combat", "Bandit / Humanoïdes / embuscades", 5, 7},
        {"C", "Chasse aux armures vivantes", "Briser plusieurs armures animées qui gardent encore des couloirs sans maître.", "combat", "Automate / golem / armure animée", 5, 8},
        {"C", "Chasse propre pour composants", "Vaincre assez de créatures sans broyer tous les matériaux utiles que la guilde espère récupérer.", "combat", "Matériaux / chasse contrôlée", 5, 8},
        {"C", "Titres affichés au mauvais moment", "Calmer un client vexé parce qu'un aventurier a choisi un titre classe mais pas du tout diplomate.", "service", "Titres / réputation / dialogues", 2, 8},
        {"C", "Piste sous pluie mauvaise", "Explorer une piste presque effacée par l'orage et revenir avec assez d'indices pour éviter une chasse au hasard.", "exploration", "Route / pluie / pistage", 4, 8},
        {"C", "Meute au vieux relais", "Nettoyer un relais où une meute utilise les portes mieux que certains aventuriers.", "combat", "Bêtes / relais", 5, 8},
        {"C", "Reconnaissance de tanière", "Explorer les abords d'une tanière active, confirmer les sorties et éviter de confondre courage avec entrée principale.", "exploration", "Tanière / repérage", 5, 8},
        {"C", "Chasse de la route coupée", "Ouvrir une route bloquée par une famille de monstres qui revient dès que les gardes relâchent la surveillance.", "combat", "Créatures locales / route", 5, 8},
        {"C", "Relais humide sans gardien", "Explorer un relais trempé dont les traces prouvent que quelqu'un monte encore la garde sans être vivant.", "exploration", "Relais / pluie / morts-vivants", 5, 8},
        {"C", "Chasse aux briseurs de barrière", "Vaincre une famille de monstres qui teste les palissades la nuit et force la ville à doubler les rondes.", "combat", "Créatures locales / barrière", 5, 8}
    };
    registerTemplates(cTemplates);

    const std::vector<GuildTemplate> bTemplates = {
        {"B", "Traquer une menace signalée par la guilde", "Identifier puis vaincre une menace plus sérieuse.", "combat", "Élite / menace", 4, 11},
        {"B", "Mini-boss régional", "Forcer une menace locale à se montrer et survivre au rapport de mission.", "combat", "Mini-boss / menace évoluée", 4, 11},
        {"B", "Zone dangereuse sous surveillance", "Revenir d'un lieu dangereux avec assez d'informations pour éviter un massacre de novices.", "exploration", "Menace avancée", 4, 11},
        {"B", "Prime silencieuse", "Régler une affaire que la guilde refuse d'écrire trop clairement sur le panneau public.", "combat", "Menace avancée", 4, 13},
        {"B", "Dossier de terrain incomplet", "Compléter des informations dangereuses sans offrir ton cadavre comme source supplémentaire.", "bestiaire", "Bestiaire avancé", 4, 12},
        {"B", "Forge attaquée par une chose lourde", "Identifier puis repousser une armure animée ou un golem attiré par les métaux rares.", "combat", "Automate / golem / armure animée", 4, 12},
        {"B", "Récit de survivant incohérent", "Vérifier une histoire impossible sans accuser trop vite le témoin d'être nul.", "bestiaire", "Anomalie mineure", 4, 13},
        {"B", "Témoin qui voit trop de cibles", "Interroger un groupe persuadé d'avoir combattu une meute alors qu'une seule trace existe.", "bestiaire", "Hallucinations / anomalie", 4, 13},
        {"B", "Nettoyage de camp abandonné", "Reprendre un vieux camp que les monstres utilisent mieux que la guilde.", "combat", "Camp abandonné", 5, 12},
        {"B", "Ruine qui change de plan", "Vérifier une ruine dont les couloirs refusent la carte de la veille.", "exploration", "Ruines / anomalie", 4, 12},
        {"B", "Prime sur un soigneur ennemi", "Identifier un soutien hostile avant qu'il ne transforme une bande faible en cauchemar.", "bestiaire", "Support ennemi", 4, 12},
        {"B", "Bête lourde de colline", "Repousser une créature qui casse les clôtures et les aventuriers trop sûrs d'eux.", "combat", "Bêtes lourdes", 5, 13},
        {"B", "Atelier pillé par des mains mortes", "Récupérer des outils dans un atelier où les morts-vivants rangent mieux que les vivants.", "exploration", "Atelier / morts-vivants", 4, 13},
        {"B", "Archive vivante mineure", "Observer une archive qui respire sans la laisser ajouter une annexe sur ta mort.", "bestiaire", "Archives noyées", 4, 13},
        {"B", "Corniche des drakes gris", "Explorer une falaise où les jeunes drakes considèrent les cordes comme des jouets.", "exploration", "Falaises des drakes gris", 4, 13},
        {"B", "Maître de stand masqué", "Démanteler une attraction qui attire les clients vers un miroir beaucoup trop profond.", "combat", "Foire abandonnée", 5, 13},
        {"B", "Arbitre de dette masqué", "Mettre fin à une dette sous les ponts avant qu'elle ne devienne un contrat de sang.", "combat", "Marché sous les ponts", 5, 13},
        {"B", "Jardin qui change de sortie", "Explorer un jardin noble où les allées refusent la même carte deux fois.", "exploration", "Jardin des statues qui pleurent", 4, 13},
        {"B", "Sonneur creux du vieux temple", "Affronter une présence de nef qui transforme les mensonges en bruit dangereux.", "combat", "Temple des cloches fendues", 5, 13},
        {"B", "Troc noir sous surveillance", "Observer un échange au marché noir sans confondre commerce louche et menace immédiate.", "bestiaire", "Marché sous les ponts / troc", 4, 13},
        {"B", "Biome mal conseillé", "Corriger une mission envoyée dans le mauvais biome avant que les novices ne partent avec la mauvaise préparation.", "service", "Guilde / biomes / préparation", 2, 13},
        {"B", "Chasse aux ours de colline", "Repousser plusieurs bêtes lourdes avant qu'elles ne transforment les clôtures en décor.", "combat", "Ours / Bêtes lourdes", 6, 11},
        {"B", "Chasse aux automates cabossés", "Neutraliser des armures animées, golems et mannequins actifs qui usent les armes plus vite que prévu.", "combat", "Automate / golem / armure animée", 6, 12},
        {"B", "Ravitaillement de crise de guilde", "Répartir rations, réparations et stocks quand la ville veut tout acheter en même temps.", "service", "Ville / guilde / économie de crise", 2, 13},
        {"B", "Sous-races en terrain hostile", "Étudier comment plusieurs sous-races réagissent à un biome difficile sans réduire ça à un bonus gratuit.", "bestiaire", "Semi-humains / sous-races / biomes", 4, 13},
        {"B", "Chaîne de contrats locale", "Terminer une suite de demandes liées : stock, route, témoin et retour au comptoir sans mélanger les preuves.", "service", "Quêtes de guilde / chaîne / ville", 3, 13},
        {"B", "Réparation avant chasse longue", "Préparer arme, armure, kits et matériaux avant une chasse où l'usure risque de coûter plus que la prime.", "service", "Équipement / durabilité / chasse", 3, 13},
        {"B", "Cartographie d'une tanière active", "Explorer une tanière encore habitée, marquer les issues et ressortir avant que la carte ne devienne un testament.", "exploration", "Tanière / menace avancée", 5, 13},
        {"B", "Chasse de rupture de ligne", "Briser une ligne de monstres qui coupe les ravitaillements sans transformer la mission en massacre inutile.", "combat", "Menace avancée / route", 6, 13},
        {"B", "Exploration de ravitaillement perdu", "Explorer la dernière position d'un chariot disparu et revenir avec itinéraire, traces et preuve exploitable.", "exploration", "Route / ravitaillement / traces", 6, 13},
        {"B", "Chasse aux gardiens cabossés", "Affronter des créatures artificielles qui usent les armes des patrouilles et bloquent un atelier secondaire.", "combat", "Automate / golem / atelier", 6, 13},
        {"B", "Exploration des torches noires", "Relever une ligne de torches éteintes qui dessine un chemin différent selon l'heure du retour.", "exploration", "Route / torches / anomalie mineure", 6, 13},
        {"B", "Chasse au verrou vivant", "Briser une créature qui garde une porte en se refermant sur les équipes trop lentes.", "combat", "Créature artificielle / verrou", 6, 13},
        {"B", "Patrouille des bornes fendues", "Suivre une vieille route de bornes cassées et repousser ce qui attaque les équipes qui comptent mal les repères.", "exploration", "Route / bornes / embuscades", 6, 14},
        {"B", "Chasse aux coureurs de ravin", "Traquer des bêtes rapides qui isolent les porteurs de preuves avant le retour au comptoir.", "combat", "Ravin / bêtes rapides", 7, 14},
        {"B", "Reconnaissance de ruine mouillée", "Explorer une ruine humide avant que la corrosion ne rende les armes et les portes aussi peu fiables l'une que l'autre.", "exploration", "Ruine humide / corrosion", 6, 14}
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
            {"A", "Salle qui affiche le mauvais combat", "Stabiliser une zone où l'interface des aventuriers annonce parfois un PvE sans ennemi réel.", "exploration", "Anomalie d'interface", 5, 18},
            {"A", "Chasse d'élite", "Affronter une menace évoluée qui a déjà survécu à plusieurs groupes.", "combat", "Mini-boss / menace évoluée", 5, 18},
            {"A", "Client trop important pour paniquer", "Résoudre une affaire de client influent sans ruiner la réputation de la guilde.", "service", "Clientèle noble", 3, 17},
            {"A", "Bête qui refuse de mourir", "Traquer une créature déjà déclarée morte par deux rapports différents.", "combat", "Menace évoluée", 5, 17},
            {"A", "Ancien atelier sous scellés", "Explorer un atelier interdit et revenir avec moins de pièces manquantes que prévu.", "exploration", "Ruines / atelier", 5, 18},
            {"A", "Fuite d'une archive vivante", "Rattraper un registre capable de modifier ses propres pages.", "bestiaire", "Archive vivante", 5, 18},
            {"A", "Chasse au drake territorial", "Repousser un draconide assez fort pour faire payer chaque erreur d'équipement.", "combat", "Draconide / montagne", 5, 19},
            {"A", "Serment dans le cimetière", "Tenir une promesse de guilde devant des morts qui écoutent encore.", "exploration", "Cimetière oublié", 5, 18},
            {"A", "Conflit de trois clients nobles", "Résoudre un contrat politique où tout le monde ment mais veut quand même être payé.", "service", "Clientèle noble", 3, 18},
            {"A", "Chasse dans les falaises grises", "Affronter une menace draconique sur une corniche qui ne pardonne pas les erreurs simples.", "combat", "Falaises des drakes gris", 5, 19},
            {"A", "Grand registre sous scellé", "Explorer une archive noyée dont les pages classent les vivants comme du matériel.", "exploration", "Archives noyées", 5, 18},
            {"A", "Éclat de géant enfoui", "Descendre dans une carrière blanche et revenir avant que quelque chose de trop grand ne respire.", "exploration", "Carrière des os blancs", 5, 18},
            {"A", "Serment de pierre noble", "Comprendre pourquoi les statues pleurent sans transformer le jardin en procès familial.", "bestiaire", "Jardin des statues qui pleurent", 5, 18},
            {"A", "Dette sous pont interdit", "Traiter une dette assez sale pour intéresser la guilde, le marché noir et personne d'honnête.", "service", "Marché sous les ponts", 3, 18},
            {"A", "Semaine de crise mal chiffrée", "Reconstituer pourquoi une ville en réparations annonce des prix, des taxes et des stocks incompatibles.", "service", "Ville / économie de crise", 3, 18},
            {"A", "Frontière de biome contestée", "Explorer une limite où deux biomes se mélangent assez pour tromper les préparations de voyage.", "exploration", "Biomes / frontière instable", 5, 18},
            {"A", "Registre de matériaux de boss", "Comparer des fragments rares sans prétendre savoir leur usage final avant assez d'observations.", "bestiaire", "Matériaux rares / boss", 5, 18},
            {"A", "Chasse aux draconides gris", "Repousser assez de draconides pour rouvrir une route de corniche sans annoncer que le danger est fini.", "combat", "Draconide / Falaises des drakes gris", 7, 18},
            {"A", "Chasse aux revenants silencieux", "Nettoyer une présence morte-vivante persistante avant qu'elle ne devienne un dossier de boss.", "combat", "Revenant / Morts-vivants et reliques", 7, 18},
        {"A", "Chasse aux sentinelles anciennes", "Neutraliser plusieurs gardiens artificiels sans confondre ruine habitée et simple bâtiment.", "combat", "Automate / golem / armure animée", 7, 18},
            {"A", "Exploration sous ciel faux", "Traverser une zone qui affiche le mauvais horizon et revenir avec des repères vérifiables.", "exploration", "Anomalie / repères instables", 6, 19},
            {"A", "Chasse au chef de tanière", "Forcer une créature dominante à sortir sans laisser ses petits transformer la zone en nid permanent.", "combat", "Mini-boss / tanière", 7, 19},
            {"A", "Expédition de corniche instable", "Explorer une route haute où la météo, les cordes et les draconides punissent l'équipement mal préparé.", "exploration", "Falaises / corde / draconide", 7, 19},
            {"A", "Chasse au prédateur qui observe", "Traquer une menace qui attend que les aventuriers soient fatigués avant de quitter sa cache.", "combat", "Prédateur / embuscade / fatigue", 7, 19},
            {"A", "Poste avancé sans réponse", "Explorer un relais qui ne répond plus et distinguer accident, embuscade ou fuite organisée.", "exploration", "Relais / enquête de terrain", 6, 20},
            {"A", "Chasse au noyau de ruine", "Neutraliser une menace artificielle qui réactive les petits gardiens quand on frappe sans réfléchir.", "combat", "Automate / ruine active", 8, 20},
            {"A", "Exploration de route avalée", "Suivre une route dont un tronçon disparaît des cartes et revenir avec des repères physiques vérifiables.", "exploration", "Route instable / carte", 7, 21},
            {"A", "Chasse au faux éclaireur", "Vaincre une créature qui imite les signaux de guilde pour attirer les patrouilles hors du chemin.", "combat", "Imitation / patrouille", 7, 21},
            {"A", "Exploration des marques effacées", "Retrouver une route dont les marques de guilde disparaissent quand une équipe approche de la bonne sortie.", "exploration", "Route instable / marques", 7, 22},
            {"A", "Chasse au chef de brèche", "Abattre une menace qui ouvre les lignes de défense aux plus petites créatures derrière elle.", "combat", "Chef de brèche / menace avancée", 8, 22},
            {"A", "Reconnaissance d'atelier vivant", "Cartographier un atelier qui réorganise ses outils dès qu'un aventurier parle de réparation facile.", "exploration", "Atelier vivant / équipement", 7, 22}
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
            {"SS", "Caravane sous sceau noir", "Escorter un convoi que même les vétérans refusent de regarder trop longtemps.", "exploration", "Menace catastrophique", 7, 32},
            {"S", "Bestiaire impossible à classer", "Étudier une cible dont la famille change entre deux lignes de rapport.", "bestiaire", "Variation majeure", 6, 25},
            {"SS", "Garde d'une porte sans salle", "Tenir une position où l'intérieur semble parfois plus grand que le bâtiment.", "combat", "Anomalie spatiale", 8, 33},
            {"S", "Contrat aux caractères morts", "Récupérer un rapport dont les lettres provoquent les lecteurs avant de s'effacer.", "bestiaire", "Anomalie / interface maudite", 6, 26},
            {"S", "Dragonnet qui a trop grandi", "Traiter une menace draconique avant qu'elle ne devienne le problème de trois villes.", "combat", "Dragon / territoire", 7, 27},
            {"S", "Nécropole sous contrat", "Récupérer une preuve dans un lieu où chaque nom inscrit attire quelque chose.", "exploration", "Cimetière oublié", 6, 26},
            {"SS", "Équipement avalé par la ruine", "Entrer dans un atelier qui use les armes plus vite que les monstres.", "exploration", "Ruines / atelier vivant", 7, 34},
            {"SS", "Bestiaire d'une chose composite", "Étudier une menace qui semble assemblée avec plusieurs familles de monstres.", "bestiaire", "Aberration composite", 7, 34},
            {"S", "Comptoir de guilde sous influence", "Identifier pourquoi un comptoir entier rédige parfois des contrats avec les mauvais noms de race, de biome et de cible.", "bestiaire", "Guilde / anomalie sociale", 6, 28},
            {"S", "Inventaire qui ment sur la rareté", "Contrôler un lot de composants dont la qualité affichée change selon la personne qui le lit.", "bestiaire", "Matériaux / interface maudite", 6, 28},
            {"S", "Chasse aux dragons mineurs", "Réduire une menace draconique assez nombreuse pour que la guilde arrête de parler de simple observation.", "combat", "Dragon / territoire", 8, 27},
            {"SS", "Chasse aux démons mineurs", "Écraser plusieurs présences infernales avant qu'elles ne s'organisent en vraie invocation de masse.", "combat", "Démon / Menace catastrophique", 10, 32},
            {"S", "Chasse aux golems de seuil", "Briser des gardiens artificiels trop nombreux pour qu'on parle encore d'une simple protection de porte.", "combat", "Automate / golem / armure animée", 8, 28},
            {"S", "Coordination de groupe risquée", "Répartir rôles, récompenses et preuves de participation pour une mission que la guilde ne veut pas voir farmée en groupe.", "service", "Coop / multi / récompenses", 4, 28},
            {"S", "Bestiaire de crise vivante", "Comparer traces, matériaux, morts et témoignages quand la ville mélange panique économique et vrai danger.", "bestiaire", "Bestiaire / ville / économie / crise", 6, 28},
            {"S", "Expédition dans la zone qui avale les cartes", "Explorer une région où les cartes reviennent blanches si personne ne garde un repère physique.", "exploration", "Zone instable / cartes", 7, 29},
            {"S", "Chasse au prédateur de convoi", "Traquer une créature qui choisit toujours le dernier wagon et disparaît avant les renforts.", "combat", "Prédateur / convoi", 8, 29},
            {"S", "Reconnaissance avant zone morte", "Explorer une bordure de zone interdite, relever les anomalies de terrain et revenir avant que la carte ne te retire.", "exploration", "Zone instable / bordure", 8, 29},
            {"S", "Chasse de meute supérieure", "Démanteler une meute assez organisée pour contourner les patrouilles et tester les faiblesses des aventuriers.", "combat", "Meute supérieure / tactique", 9, 29},
            {"S", "Exploration de balises éteintes", "Retrouver une ligne de balises disparues et noter si elles ont été détruites, déplacées ou simplement mensongères.", "exploration", "Balises / route instable", 8, 30},
            {"S", "Chasse au gardien de passage", "Affronter une créature qui ne protège pas un trésor, mais la sortie elle-même.", "combat", "Gardien / passage dangereux", 9, 30},
            {"S", "Exploration des bornes inversées", "Explorer une zone où les bornes de route pointent vers l'endroit d'où tu viens dès que tu doutes du chemin.", "exploration", "Zone instable / bornes", 8, 31},
            {"S", "Chasse au porteur de carte vide", "Affronter une menace qui efface les cartes des équipes qu'elle laisse repartir vivantes.", "combat", "Cartes / menace intelligente", 9, 31},
            {"S", "Exploration de la route qui recule", "Avancer sur une route qui replace les repères derrière l'équipe si personne ne garde une preuve physique.", "exploration", "Route impossible / repères", 8, 31},
            {"S", "Chasse au veilleur sans camp", "Traquer une menace qui surveille les bivouacs sans jamais laisser de feu, seulement des traces autour.", "combat", "Prédateur / campement", 9, 31},
            {"SS", "Percée dans la ligne noire", "Ouvrir un passage dans une ligne de monstres assez organisée pour comprendre les horaires de patrouille.", "combat", "Ligne noire / menace catastrophique", 10, 34}
        };
        registerTemplates(sTemplates);

        addGuildQuestIfAvailable(board, "guild_s_dynamic", playerLevel, sTemplates);
    }

    if (playerLevel >= 42)
    {
        const std::vector<GuildTemplate> sssTemplates = {
            {"SSS", "Éradication d'une zone morte", "Entrer dans une zone que la guilde a déjà rayée de ses cartes et revenir avec une preuve de nettoyage.", "combat", "Zone morte", 9, 42},
            {"SSS", "Archive qui respire encore", "Récupérer un registre vivant sans le laisser écrire ton nom à l'intérieur.", "bestiaire", "Archive vivante", 8, 42},
            {"SSS", "Marche sur une frontière morte", "Explorer une limite de carte où les monstres semblent attendre qu'on les nomme.", "exploration", "Zone morte", 9, 44},
            {"SSS", "Suppression d'un nid impossible", "Détruire une source de monstres qui repousse dès qu'on ment dans le rapport.", "combat", "Nid impossible", 9, 45},
            {"SSS", "Convoi d'artefact muet", "Protéger une relique qui ne parle que quand tout va déjà très mal.", "exploration", "Artefact / convoi", 8, 46},
            {"SSS", "Chasse sous ciel mort", "Vaincre une menace qui revient tant que le rapport ne nomme pas correctement sa famille.", "combat", "Zone morte / famille inconnue", 10, 46},
            {"SSS", "Exploration du bord de carte", "Avancer jusqu'à une limite où la guilde ne garantit plus que le retour mène au même endroit.", "exploration", "Frontière morte / carte", 9, 47},
            {"SSS", "Chasse sans deuxième rapport", "Vaincre une menace dont les précédentes équipes n'ont laissé qu'une carte pliée et une arme fendue.", "combat", "Menace majeure / rapport perdu", 11, 48},
            {"SSS", "Exploration du camp qui revient", "Explorer un ancien camp de guilde qui réapparaît parfois avec les mêmes cendres encore chaudes.", "exploration", "Camp impossible / route morte", 10, 49},
            {"SSS", "Chasse au seuil qui respire", "Affronter une présence de bord de zone avant qu'elle ne devienne un vrai dossier de boss.", "combat", "Seuil vivant / zone morte", 11, 50},
            {"SSS", "Exploration de la carte retournée", "Revenir d'une zone où le nord, le sud et le journal de guilde ne sont pas d'accord.", "exploration", "Carte inversée / frontière morte", 10, 50}
        };
        registerTemplates(sssTemplates);

        addGuildQuestIfAvailable(board, "guild_sss_dynamic", playerLevel, sssTemplates);
    }

    if (playerLevel >= 55)
    {
        const std::vector<GuildTemplate> heroTemplates = {
            {"Héros mondial", "Contrat de héros mondial", "Répondre à une menace dont l'échec serait raconté dans plusieurs royaumes.", "combat", "Menace mondiale", 10, 55},
            {"Héros mondial", "Serment sous plusieurs bannières", "Porter une mission signée par plusieurs autorités sans laisser la politique tuer les civils.", "service", "Clientèle royale", 4, 55},
            {"Héros mondial", "Frontière que les cartes refusent", "Stabiliser une région que plusieurs royaumes dessinent différemment.", "exploration", "Frontière mondiale", 9, 56},
            {"Héros mondial", "Nom connu dans trois capitales", "Traquer une menace que les politiques accusent déjà les uns sur les autres.", "bestiaire", "Menace mondiale", 9, 58}
        };
        registerTemplates(heroTemplates);

        addGuildQuestIfAvailable(board, "guild_world_hero_dynamic", playerLevel, heroTemplates);
    }

    if (playerLevel >= 70)
    {
        const std::vector<GuildTemplate> legendTemplates = {
            {"Légende", "Contrat réservé aux légendes", "S'occuper d'une menace que la guilde ne décrit plus aux aventuriers normaux.", "combat", "Menace légendaire", 11, 70},
            {"Légende", "Cartographie d'un lieu impossible", "Revenir d'un endroit qui change de forme dès que quelqu'un affirme l'avoir compris.", "exploration", "Lieu impossible", 9, 70},
            {"Légende", "Nom rayé dans trois royaumes", "Poursuivre une menace que plusieurs registres ont volontairement oubliée.", "bestiaire", "Menace légendaire", 10, 72},
            {"Légende", "Expédition au lieu qui ment", "Explorer une zone dont la sortie raconte toujours une version différente de l'entrée.", "exploration", "Lieu impossible", 10, 73},
            {"Légende", "Combat que la guilde ne promet pas de payer", "Accepter un contrat si dangereux que la prime devient presque symbolique.", "combat", "Menace légendaire", 12, 74},
            {"Légende", "Piste sous trois soleils", "Explorer une route où les repères semblent appartenir à trois journées différentes.", "exploration", "Lieu impossible / route haute", 10, 75},
            {"Légende", "Chasse au nom interdit", "Affronter une menace que les registres remplacent par un blanc dès qu'un témoin parle trop précisément.", "combat", "Menace légendaire / nom interdit", 12, 76},
            {"Légende", "Exploration du seuil sans retour", "Approcher un seuil qui ne promet pas de ramener le personnage dans la même version de la route.", "exploration", "Seuil impossible / route haute", 11, 78}
        };
        registerTemplates(legendTemplates);

        addGuildQuestIfAvailable(board, "guild_legend_dynamic", playerLevel, legendTemplates);
    }

    if (playerLevel >= 90)
    {
        const std::vector<GuildTemplate> godTemplates = {
            {"Dieu", "Demande que personne ne devrait accepter", "Approcher une anomalie de rang divin sans confondre courage et suicide.", "bestiaire", "Anomalie divine", 12, 90},
            {"Dieu", "Dernière ligne d'un registre brûlé", "Traiter une menace que les maîtres de guilde ne prononcent qu'une fois la porte fermée.", "combat", "Menace divine", 12, 95},
            {"Dieu", "Archive qui corrige le joueur", "Approcher une information si haute qu'elle tente de réécrire le dossier de mission.", "bestiaire", "Archive divine", 12, 96},
            {"Dieu", "Chasse au silence divin", "Répondre à une présence qui ne rugit pas : elle retire simplement les bruits autour de toi.", "combat", "Menace divine / silence", 13, 98},
            {"Dieu", "Exploration de la marge du monde", "Avancer sur une marge où les cartes, les titres et les rapports semblent hésiter à admettre que tu existes.", "exploration", "Marge du monde / pré-histoire", 12, 100}
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

    // FR: Le panneau doit proposer davantage de vrai terrain maintenant : combat/exploration.
    // EN: The board should now lean more toward real field work: combat/exploration.
    const int desiredActionMinimum = playerLevel >= 7 ? 3 : 2;
    enforceActionQuestPresence(board, playerLevel, fillerTemplates, std::min(desiredActionMinimum, desiredBoardSize));

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
        {"draconic_scale_fragment", "Fragment d'écaille draconique", 1, 6},
        {"firefly_iron_shell", "Carapace de luciole de fer", 2, 4},
        {"grey_drake_scale", "Écaille de drake gris", 1, 8},
        {"rusted_gear_core", "Noyau d'engrenage rouillé", 1, 5},
        {"mirror_glass_bead", "Perle de verre miroir", 2, 5}
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

// EN: createMerchantQuestionRequest creates a direct Prunigil service quest using QCM-style paperwork tests.
// FR: createMerchantQuestionRequest crée une demande directe de Prunigil avec épreuves de comptoir en QCM.
Quest QuestCatalog::createMerchantQuestionRequest(int playerLevel)
{
    std::string rank = playerLevel >= 8 ? "C" : (playerLevel >= 4 ? "D" : (playerLevel >= 2 ? "E" : "F"));
    int target = 1 + (playerLevel >= 4 ? 1 : 0) + (playerLevel >= 9 ? 1 : 0);
    const int roll = randomBetween(1, 10);

    std::string title;
    std::string objective;
    std::string family;
    std::string requiredMaterialId;
    std::string requiredMaterialName;
    int requiredMaterialQuantity = 0;
    std::string rewardMaterialId;
    std::string rewardMaterialName;
    int rewardMaterialQuantity = 0;

    switch (roll)
    {
        case 1:
            title = "Registre de Prunigil";
            objective = "Aider Prunigil à corriger des lignes de registre avant que les clients ne signent n'importe quoi.";
            family = "Marchand / registre / français";
            break;
        case 2:
            title = "Factures et pièces mélangées";
            objective = "Recalculer quelques factures en cuivre, fer, électrum, or et platine avec Prunigil au comptoir.";
            family = "Marchand / calcul / monnaies";
            break;
        case 3:
            title = "Messages de livraison illisibles";
            objective = "Transformer des notes de livraison pleines de fautes en consignes compréhensibles.";
            family = "Marchand / français drôle";
            break;
        case 4:
            title = "Réduction de gros client";
            objective = "Vérifier remises, taxes et rendus de monnaie sans vexer le client important.";
            family = "Marchand / calcul de comptoir";
            break;
        case 5:
            title = "Caravane à budgéter";
            objective = "Aider Prunigil à savoir si payer les gardes de caravane vaut vraiment le coût.";
            family = "Marchand / caravane / comptabilité";
            requiredMaterialId = "route_toll_receipt";
            requiredMaterialName = "Reçu de péage de route";
            requiredMaterialQuantity = 1;
            rewardMaterialId = "travel_pass_note";
            rewardMaterialName = "Note de pass de voyage";
            rewardMaterialQuantity = 1;
            break;
        case 6:
            title = "Étal de brocante à trier";
            objective = "Séparer bonnes affaires, objets inutiles et vieux lots douteux avant l'ouverture du marché.";
            family = "Marchand / brocante / troc";
            rewardMaterialId = "local_reputation_note";
            rewardMaterialName = "Note de réputation locale";
            rewardMaterialQuantity = 1;
            break;
        case 7:
            title = "Objet de luxe à estimer";
            objective = "Comparer rareté, finition, provenance et risque d'arnaque sur un objet de luxe proposé à un noble client.";
            family = "Marchand / luxe / estimation / provenance";
            requiredMaterialId = "city_service_stamp";
            requiredMaterialName = "Tampon de service municipal";
            requiredMaterialQuantity = 1;
            rewardMaterialId = "client_recommendation";
            rewardMaterialName = "Recommandation de client";
            rewardMaterialQuantity = 1;
            break;
        case 8:
            title = "Reçu de péage mal classé";
            objective = "Retrouver quel reçu de route correspond à quelle caisse avant que le convoi ne paie deux fois le même pont.";
            family = "Marchand / reçu / péage / classement";
            requiredMaterialId = "route_toll_receipt";
            requiredMaterialName = "Reçu de péage de route";
            requiredMaterialQuantity = 1;
            rewardMaterialId = "local_service_letter";
            rewardMaterialName = "Lettre de service local";
            rewardMaterialQuantity = 1;
            break;
        case 9:
            title = "Bons d'auberge dans les comptes";
            objective = "Séparer bons de repas, bons de lit et vraies pièces dans une addition de voyageurs trop pressés.";
            family = "Marchand / auberge / bons / comptabilité";
            requiredMaterialId = "warm_meal_voucher";
            requiredMaterialName = "Bon de repas chaud";
            requiredMaterialQuantity = 1;
            rewardMaterialId = "local_reputation_note";
            rewardMaterialName = "Note de réputation locale";
            rewardMaterialQuantity = 1;
            break;
        default:
            title = "Contrat écrit n'importe comment";
            objective = "Relire un contrat de Prunigil assez mal écrit pour devenir dangereux juridiquement.";
            family = "Marchand / contrat / QCM";
            requiredMaterialId = "municipal_proof_letter";
            requiredMaterialName = "Attestation municipale";
            requiredMaterialQuantity = 1;
            break;
    }

    const bool givesObjectReward = !rewardMaterialId.empty() && rewardMaterialQuantity > 0;
    return buildQuest(
        "npc_prunigil_questions_" + std::to_string(playerLevel) + "_" + std::to_string(randomBetween(100, 999)),
        rank, title, "PNJ notable", "Prunigil le marchand", "Comptoir de Prunigil",
        objective,
        "service", family,
        questExperience(rank, playerLevel, target + 1), adjustedQuestGold(rank, playerLevel, target + 1, "service", givesObjectReward), target, false,
        requiredMaterialId, requiredMaterialName, requiredMaterialQuantity,
        rewardMaterialId, rewardMaterialName, rewardMaterialQuantity,
        "Objectif de quête probable : parler à Prunigil et réussir ses QCM de comptoir. Certaines variantes peuvent demander une preuve achetée ou obtenue en ville."
    );
}


// EN: createLibrarianKnowledgeQuestionRequest creates a direct library knowledge QCM quest from guild knowledge sheets.
// FR: createLibrarianKnowledgeQuestionRequest crée une demande directe de bibliothèque avec QCM de connaissances issus des fiches.
Quest QuestCatalog::createLibrarianKnowledgeQuestionRequest(int playerLevel)
{
    std::string rank = playerLevel >= 10 ? "C" : (playerLevel >= 5 ? "D" : "E");
    int target = 2 + (playerLevel >= 6 ? 1 : 0) + (playerLevel >= 12 ? 1 : 0);
    const int roll = randomBetween(1, 5);

    std::string title;
    std::string objective;
    std::string family;

    switch (roll)
    {
        case 1:
            title = "Oral de bestiaire";
            objective = "Répondre aux questions de l'Archiviste Meron sur spectres, trolls, marais et traces dangereuses.";
            family = "Bibliothèque / bestiaire / QCM";
            break;
        case 2:
            title = "Classement des écoles magiques";
            objective = "Aider l'Archiviste Meron à corriger des fiches sur écoles magiques, sorts de zone et sorts canalisés.";
            family = "Bibliothèque / magie / QCM";
            break;
        case 3:
            title = "Plantes, animaux et sommeil";
            objective = "Vérifier des notes d'herboristerie et de créatures utiles sans inventer une vérité dangereuse.";
            family = "Bibliothèque / plantes / animaux / QCM";
            break;
        case 4:
            title = "Cartes anciennes incomplètes";
            objective = "Relire des cartes anciennes et choisir la transition d'information la moins trompeuse pour le journal.";
            family = "Bibliothèque / cartes / logique";
            break;
        default:
            title = "Résumé pour aventurier pressé";
            objective = "Transformer des notes savantes en conseils compréhensibles pour quelqu'un qui ne lit pas forcément bien.";
            family = "Bibliothèque / vulgarisation / français";
            break;
    }

    return buildQuest(
        questId("npc_librarian_qcm", playerLevel), rank, title, "PNJ notable", "Archiviste Meron", "Bibliothèque des cartes et savoirs",
        objective,
        "service", family,
        questExperience(rank, playerLevel, target + 1), adjustedQuestGold(rank, playerLevel, target + 1, "service", true), target, false,
        "", "", 0,
        "common_goblin_notes", "Notes de connaissance corrigées", 1,
        "Objectif de quête probable : parler à l'Archiviste Meron et réussir un QCM de connaissances."
    );
}

// EN: createAdministrativePaperworkRequest creates direct paperwork and inscription service quests.
// FR: createAdministrativePaperworkRequest crée des demandes directes de paperasse et d'inscription.
Quest QuestCatalog::createAdministrativePaperworkRequest(int playerLevel)
{
    std::string rank = playerLevel >= 9 ? "C" : (playerLevel >= 4 ? "D" : "E");
    int target = 2 + (playerLevel >= 7 ? 1 : 0);
    const int roll = randomBetween(1, 8);

    std::string title;
    std::string objective;
    std::string family;
    std::string requiredMaterialId;
    std::string requiredMaterialName;
    int requiredMaterialQuantity = 0;
    std::string rewardMaterialId = "local_service_letter";
    std::string rewardMaterialName = "Lettre de service administratif";
    int rewardMaterialQuantity = 1;

    switch (roll)
    {
        case 1:
            title = "Fiches d'inscription à corriger";
            objective = "Aider Scribe Ysolde à repérer noms, classes, rangs et signatures illisibles avant enregistrement magique.";
            family = "Administration / inscription / français";
            break;
        case 2:
            title = "Pastilles de fiabilité";
            objective = "Classer des dossiers verts, oranges, rouges ou noirs sans confondre une erreur bête avec une trahison.";
            family = "Administration / pastilles / logique";
            requiredMaterialId = "city_service_stamp";
            requiredMaterialName = "Tampon de service municipal";
            requiredMaterialQuantity = 1;
            break;
        case 3:
            title = "Abonnements de guilde";
            objective = "Comparer des avantages d'abonnement et vérifier que le reçu ne promet pas des droits impossibles.";
            family = "Administration / abonnement / calcul";
            break;
        case 4:
            title = "Réhabilitation douteuse";
            objective = "Relire une demande de réhabilitation écrite à moitié correctement et décider quelles preuves demander.";
            family = "Administration / sanction / QCM / preuve";
            requiredMaterialId = "local_service_letter";
            requiredMaterialName = "Lettre de service local";
            requiredMaterialQuantity = 1;
            rewardMaterialId = "municipal_proof_letter";
            rewardMaterialName = "Attestation municipale";
            break;
        case 5:
            title = "Preuves de service local";
            objective = "Distinguer lettre de service, recommandation de client et simple rumeur avant de modifier un dossier.";
            family = "Administration / service local / preuve";
            requiredMaterialId = "local_reputation_note";
            requiredMaterialName = "Note de réputation locale";
            requiredMaterialQuantity = 1;
            break;
        case 6:
            title = "Guichet de preuve municipale";
            objective = "Vérifier qu'une attestation, un tampon et une signature racontent la même histoire avant d'ouvrir un droit local.";
            family = "Administration / guichet / preuve municipale";
            requiredMaterialId = "municipal_proof_letter";
            requiredMaterialName = "Attestation municipale";
            requiredMaterialQuantity = 1;
            rewardMaterialId = "local_reputation_note";
            rewardMaterialName = "Note de réputation locale";
            break;
        case 7:
            title = "Dossier de bonne conduite";
            objective = "Assembler notes locales et services propres pour montrer qu'un petit incident ne définit pas tout un dossier.";
            family = "Administration / bonne conduite / réputation";
            requiredMaterialId = "local_reputation_note";
            requiredMaterialName = "Note de réputation locale";
            requiredMaterialQuantity = 1;
            rewardMaterialId = "client_recommendation";
            rewardMaterialName = "Recommandation de client";
            break;
        default:
            title = "Registre des litiges";
            objective = "Résumer un litige entre client et aventurier avec une phrase claire, pas un roman incompréhensible.";
            family = "Administration / litige / rédaction";
            break;
    }

    return buildQuest(
        questId("npc_scribe_paperwork", playerLevel), rank, title, "PNJ notable", "Scribe Ysolde", "Bureau des inscriptions",
        objective,
        "service", family,
        questExperience(rank, playerLevel, target + 1), adjustedQuestGold(rank, playerLevel, target + 1, "service", true), target, false,
        requiredMaterialId, requiredMaterialName, requiredMaterialQuantity,
        rewardMaterialId, rewardMaterialName, rewardMaterialQuantity,
        "Objectif de quête probable : parler à Scribe Ysolde et traiter la paperasse avec un QCM. Certaines validations demandent une vraie preuve de ville."
    );
}

// EN: createAlchemistFormulaQuestionRequest creates non-combat alchemy QCM quests.
// FR: createAlchemistFormulaQuestionRequest crée des demandes d'alchimie non-combat en QCM.
Quest QuestCatalog::createAlchemistFormulaQuestionRequest(int playerLevel)
{
    std::string rank = playerLevel >= 11 ? "C" : (playerLevel >= 5 ? "D" : "E");
    int target = 2 + (playerLevel >= 6 ? 1 : 0) + (playerLevel >= 12 ? 1 : 0);
    const int roll = randomBetween(1, 5);

    std::string title;
    std::string objective;
    std::string family;

    switch (roll)
    {
        case 1:
            title = "Dosage de potions";
            objective = "Vérifier des dosages de soin, de mana et d'élixir avant que le laboratoire ne devienne une soupe lumineuse.";
            family = "Alchimie / dosage / calcul";
            break;
        case 2:
            title = "Étiquettes dangereuses";
            objective = "Corriger des étiquettes de potions pour éviter qu'un client avale trois flacons par curiosité.";
            family = "Alchimie / étiquette / français";
            break;
        case 3:
            title = "Réactifs incompatibles";
            objective = "Choisir quel ingrédient ne doit pas être mélangé avec un stabilisant instable.";
            family = "Alchimie / logique / sécurité";
            break;
        case 4:
            title = "Inventaire du laboratoire";
            objective = "Compter fioles, réactifs et pertes sans mentir au registre de la guilde des alchimistes.";
            family = "Alchimie / inventaire / calcul";
            break;
        default:
            title = "Notice d'effet secondaire";
            objective = "Rendre lisible une notice de potion sans supprimer les informations qui peuvent sauver des dents.";
            family = "Alchimie / notice / QCM";
            break;
    }

    return buildQuest(
        questId("npc_alchemist_qcm", playerLevel), rank, title, "PNJ notable", "Maëra l'alchimiste", "Laboratoire de Maëra",
        objective,
        "service", family,
        questExperience(rank, playerLevel, target + 1), adjustedQuestGold(rank, playerLevel, target + 1, "service", true), target, false,
        "", "", 0,
        "preservation_vials", "Fioles de conservation", playerLevel >= 8 ? 2 : 1,
        "Objectif de quête probable : parler à Maëra et résoudre une vérification d'alchimie."
    );
}

// EN: createTransportLogisticsQuestionRequest creates logistics and route service QCM quests.
// FR: createTransportLogisticsQuestionRequest crée des demandes de logistique et routes en QCM.
Quest QuestCatalog::createTransportLogisticsQuestionRequest(int playerLevel)
{
    std::string rank = playerLevel >= 12 ? "B" : (playerLevel >= 7 ? "C" : (playerLevel >= 3 ? "D" : "E"));
    int target = 2 + (playerLevel >= 7 ? 1 : 0) + (playerLevel >= 13 ? 1 : 0);
    const int roll = randomBetween(1, 14);

    std::string title;
    std::string objective;
    std::string family;
    std::string requiredMaterialId;
    std::string requiredMaterialName;
    int requiredMaterialQuantity = 0;
    std::string rewardMaterialId = "travel_pass_note";
    std::string rewardMaterialName = "Note de passage tamponnée";
    int rewardMaterialQuantity = 1;

    switch (roll)
    {
        case 1:
            title = "Route de caravane";
            objective = "Comparer une route sûre et une route rapide en tenant compte des gardes, du coût et des risques.";
            family = "Transport / caravane / logique";
            requiredMaterialId = "caravan_seat_ticket";
            requiredMaterialName = "Place de caravane";
            requiredMaterialQuantity = 1;
            break;
        case 2:
            title = "Pass de voyage";
            objective = "Vérifier quel pass autorise un passage sans envoyer le client devant un portail impossible.";
            family = "Transport / pass / QCM";
            requiredMaterialId = "travel_pass_note";
            requiredMaterialName = "Note de pass de voyage";
            requiredMaterialQuantity = 1;
            rewardMaterialId = "route_toll_receipt";
            rewardMaterialName = "Reçu de péage de route";
            break;
        case 3:
            title = "Chargement de diligence";
            objective = "Répartir caisses, voyageurs et bagages pour éviter le trajet ridicule qui finit dans un fossé.";
            family = "Transport / charge / calcul";
            requiredMaterialId = "stable_stall_ticket";
            requiredMaterialName = "Ticket d'écurie";
            requiredMaterialQuantity = 1;
            break;
        case 4:
            title = "Portail ou détour";
            objective = "Décider si un portail coûte vraiment moins cher qu'une escorte longue et dangereuse.";
            family = "Transport / portail / calcul";
            break;
        case 5:
            title = "Liaison entre paliers";
            objective = "Vérifier pass, escorte et durée probable pour un voyage entre deux paliers sans promettre l'impossible.";
            family = "Transport / inter-paliers / pass";
            requiredMaterialId = playerLevel >= 7 ? "guarded_transport_pass" : "caravan_seat_ticket";
            requiredMaterialName = playerLevel >= 7 ? "Pass de transport gardé" : "Place de caravane";
            requiredMaterialQuantity = 1;
            rewardMaterialId = "local_reputation_note";
            rewardMaterialName = "Note de réputation locale";
            break;
        case 6:
            title = "Péage et relais";
            objective = "Calculer péages, relais et repas de route pour éviter que le voyage coûte plus cher que la cargaison.";
            family = "Transport / péage / relais";
            requiredMaterialId = "route_toll_receipt";
            requiredMaterialName = "Reçu de péage de route";
            requiredMaterialQuantity = 1;
            break;
        case 7:
            title = "Nuit au relais";
            objective = "Prévoir lit, écurie et repas avant un départ de caravane qui ne pardonne pas les oublis.";
            family = "Transport / relais / auberge / préparation";
            requiredMaterialId = "lodging_bed_token";
            requiredMaterialName = "Bon de lit d'auberge";
            requiredMaterialQuantity = 1;
            rewardMaterialId = "route_toll_receipt";
            rewardMaterialName = "Reçu de péage de route";
            break;
        case 8:
            title = "Contrôle de pont";
            objective = "Présenter les bons papiers au garde du pont et repérer quel reçu évite de repayer le passage.";
            family = "Transport / pont / contrôle / reçu";
            requiredMaterialId = "municipal_proof_letter";
            requiredMaterialName = "Attestation municipale";
            requiredMaterialQuantity = 1;
            rewardMaterialId = "route_toll_receipt";
            rewardMaterialName = "Reçu de péage de route";
            break;
        case 9:
            title = "Carnet d'écurie du relais";
            objective = "Vérifier boxes, montures, sacoches et départs pour éviter qu'un cheval parte avec la mauvaise cargaison.";
            family = "Transport / écurie / relais / registre";
            requiredMaterialId = "stable_stall_ticket";
            requiredMaterialName = "Ticket d'écurie";
            requiredMaterialQuantity = 1;
            rewardMaterialId = "travel_pass_note";
            rewardMaterialName = "Note de pass de voyage";
            break;
        case 10:
            title = "Bagages sous scellé";
            objective = "Classer les bagages d'un convoi gardé sans ouvrir ce qui porte déjà trop de sceaux officiels.";
            family = "Transport / bagages / convoi gardé / preuves";
            requiredMaterialId = playerLevel >= 7 ? "guarded_transport_pass" : "route_toll_receipt";
            requiredMaterialName = playerLevel >= 7 ? "Pass de transport gardé" : "Reçu de péage de route";
            requiredMaterialQuantity = 1;
            rewardMaterialId = "local_reputation_note";
            rewardMaterialName = "Note de réputation locale";
            break;
        case 11:
            title = "Monture pour long trajet";
            objective = "Vérifier si une monture louée suffit ou s'il faut vraiment passer par caravane gardée.";
            family = "Transport / monture / distance";
            requiredMaterialId = "rental_mount_voucher";
            requiredMaterialName = "Bon de monture de location";
            requiredMaterialQuantity = 1;
            rewardMaterialId = "relay_route_badge";
            rewardMaterialName = "Badge de route du relais";
            break;
        case 12:
            title = "Box sécurisé et cargaison";
            objective = "Choisir quelle charge reste au box, laquelle part en selle, et laquelle doit attendre le prochain convoi.";
            family = "Transport / écurie / stockage";
            requiredMaterialId = "stable_box_reservation";
            requiredMaterialName = "Réservation de box sécurisé";
            requiredMaterialQuantity = 1;
            rewardMaterialId = "local_service_letter";
            rewardMaterialName = "Lettre de service local";
            break;
        case 13:
            title = "Selle chargée avant l'aube";
            objective = "Recompter les sacs d'une selle de bât pour éviter un départ lent et une route de nuit inutile.";
            family = "Transport / sacoches / temps";
            requiredMaterialId = "loaded_pack_saddle";
            requiredMaterialName = "Selle de bât chargée";
            requiredMaterialQuantity = 1;
            rewardMaterialId = "travel_distance_mark";
            rewardMaterialName = "Marque de distance de trajet";
            rewardMaterialQuantity = 2;
            break;
        default:
            title = "Bon de livraison inter-ville";
            objective = "Corriger un bon de livraison avant qu'un colis parte vers la mauvaise ville.";
            family = "Transport / livraison / français";
            break;
    }

    return buildQuest(
        questId("npc_transport_qcm", playerLevel), rank, title, "PNJ notable", "Noro le palefrenier", "Relais des routes",
        objective,
        "service", family,
        questExperience(rank, playerLevel, target + 1), adjustedQuestGold(rank, playerLevel, target + 1, "service", true), target, false,
        requiredMaterialId, requiredMaterialName, requiredMaterialQuantity,
        rewardMaterialId, rewardMaterialName, rewardMaterialQuantity,
        "Objectif de quête probable : parler à Noro et traiter une épreuve de route ou de transport. Certains trajets demandent maintenant un ticket, reçu ou pass réel."
    );
}

// EN: createInnkeeperServiceQuestionRequest creates lodging and city-service QCM quests.
// FR: createInnkeeperServiceQuestionRequest crée des demandes d'auberge et services urbains en QCM.
Quest QuestCatalog::createInnkeeperServiceQuestionRequest(int playerLevel)
{
    std::string rank = playerLevel >= 8 ? "C" : (playerLevel >= 4 ? "D" : "E");
    int target = 2 + (playerLevel >= 6 ? 1 : 0);
    const int roll = randomBetween(1, 15);

    std::string title;
    std::string objective;
    std::string family;
    std::string requiredMaterialId;
    std::string requiredMaterialName;
    int requiredMaterialQuantity = 0;
    std::string rewardMaterialId = "warm_meal_voucher";
    std::string rewardMaterialName = "Bon pour un repas chaud";
    int rewardMaterialQuantity = 1;

    switch (roll)
    {
        case 1:
            title = "Chambres à répartir";
            objective = "Répartir voyageurs, chambres et lits sans faire dormir un cheval dans une suite.";
            family = "Auberge / hébergement / calcul";
            break;
        case 2:
            title = "Plainte de client";
            objective = "Réécrire une plainte d'auberge pour qu'elle soit lisible sans perdre le côté dramatique.";
            family = "Auberge / français fun";
            requiredMaterialId = "city_service_stamp";
            requiredMaterialName = "Tampon de service municipal";
            requiredMaterialQuantity = 1;
            rewardMaterialId = "local_service_letter";
            rewardMaterialName = "Lettre de service local";
            break;
        case 3:
            title = "Addition de taverne";
            objective = "Vérifier repas, nuitées et réduction de groupe avant que la table ne lance les chopes.";
            family = "Auberge / addition / calcul";
            break;
        case 4:
            title = "Objets oubliés";
            objective = "Classer les objets oubliés par chambre et retrouver celui qui n'appartient à personne.";
            family = "Auberge / logique / inventaire";
            requiredMaterialId = "lodging_bed_token";
            requiredMaterialName = "Bon de lit d'auberge";
            requiredMaterialQuantity = 1;
            rewardMaterialId = "local_reputation_note";
            rewardMaterialName = "Note de réputation locale";
            break;
        case 5:
            title = "Contrat de chambre longue durée";
            objective = "Vérifier caution, durée et règles de logement avant de donner une chambre pour plusieurs semaines.";
            family = "Auberge / logement long terme / contrat";
            requiredMaterialId = "lodging_bed_token";
            requiredMaterialName = "Bon de lit d'auberge";
            requiredMaterialQuantity = 1;
            rewardMaterialId = "municipal_proof_letter";
            rewardMaterialName = "Attestation municipale";
            break;
        case 6:
            title = "Services de ville à recommander";
            objective = "Orienter un voyageur vers bain, blanchisserie, messager ou garde sans vendre tout comme une nuit d'auberge.";
            family = "Auberge / services ville / orientation";
            requiredMaterialId = "city_service_stamp";
            requiredMaterialName = "Tampon de service municipal";
            requiredMaterialQuantity = 1;
            rewardMaterialId = "local_reputation_note";
            rewardMaterialName = "Note de réputation locale";
            break;
        case 7:
            title = "Repas de route à préparer";
            objective = "Compter repas chauds, rations et bons de table pour un groupe qui repart avant l'aube.";
            family = "Auberge / repas / voyage";
            requiredMaterialId = "warm_meal_voucher";
            requiredMaterialName = "Bon de repas chaud";
            requiredMaterialQuantity = 1;
            rewardMaterialId = "travel_pass_note";
            rewardMaterialName = "Note de pass de voyage";
            break;
        case 8:
            title = "Réservation d'écurie";
            objective = "Associer montures, boxes et départ de relais sans vendre deux fois la même place.";
            family = "Auberge / écurie / relais / réservation";
            requiredMaterialId = "stable_stall_ticket";
            requiredMaterialName = "Ticket d'écurie";
            requiredMaterialQuantity = 1;
            rewardMaterialId = "route_toll_receipt";
            rewardMaterialName = "Reçu de péage de route";
            break;
        case 9:
            title = "Acompte de groupe";
            objective = "Calculer l'acompte d'une table nombreuse qui veut repas, lits et départ avant l'aube.";
            family = "Auberge / groupe / acompte / repas";
            requiredMaterialId = "warm_meal_voucher";
            requiredMaterialName = "Bon de repas chaud";
            requiredMaterialQuantity = 1;
            rewardMaterialId = "client_recommendation";
            rewardMaterialName = "Recommandation de client";
            break;
        case 10:
            title = "Écurie trop pleine";
            objective = "Réorganiser boxes, selles, sacs et chevaux nerveux avant que le relais ne transforme le départ en théâtre.";
            family = "Auberge / écurie / stockage / logique";
            requiredMaterialId = "stable_stall_ticket";
            requiredMaterialName = "Ticket d'écurie";
            requiredMaterialQuantity = 1;
            rewardMaterialId = "travel_pass_note";
            rewardMaterialName = "Note de pass de voyage";
            break;
        case 11:
            title = "Nuit réparatrice à préparer";
            objective = "Choisir repas, chambre et heure de réveil pour un aventurier qui doit récupérer sans rater son départ.";
            family = "Auberge / repos / délai / préparation";
            requiredMaterialId = "lodging_bed_token";
            requiredMaterialName = "Bon de lit d'auberge";
            requiredMaterialQuantity = 1;
            rewardMaterialId = "local_service_letter";
            rewardMaterialName = "Lettre de service local";
            break;
        case 12:
            title = "Monture louée au mauvais nom";
            objective = "Retrouver quelle monture correspond à quel client avant que deux groupes partent avec la mauvaise bête.";
            family = "Auberge / écurie / monture";
            requiredMaterialId = "rental_mount_voucher";
            requiredMaterialName = "Bon de monture de location";
            requiredMaterialQuantity = 1;
            rewardMaterialId = "stable_box_reservation";
            rewardMaterialName = "Réservation de box sécurisé";
            break;
        case 13:
            title = "Box sécurisé contesté";
            objective = "Lire deux reçus presque identiques et décider qui a vraiment réservé le box avant la nuit.";
            family = "Auberge / box / reçu";
            requiredMaterialId = "stable_box_reservation";
            requiredMaterialName = "Réservation de box sécurisé";
            requiredMaterialQuantity = 1;
            rewardMaterialId = "local_reputation_note";
            rewardMaterialName = "Note de réputation locale";
            break;
        case 14:
            title = "Selle de bât trop chargée";
            objective = "Retirer les objets inutiles d'une selle chargée pour gagner du temps sans perdre la cargaison importante.";
            family = "Auberge / sacoches / logique";
            requiredMaterialId = "loaded_pack_saddle";
            requiredMaterialName = "Selle de bât chargée";
            requiredMaterialQuantity = 1;
            rewardMaterialId = "travel_pass_note";
            rewardMaterialName = "Note de pass de voyage";
            break;
        default:
            title = "Règlement affiché au mur";
            objective = "Corriger un règlement d'auberge assez mal écrit pour donner envie de le violer exprès.";
            family = "Auberge / règlement / QCM";
            break;
    }

    return buildQuest(
        questId("npc_innkeeper_qcm", playerLevel), rank, title, "PNJ notable", "Tavia l'aubergiste", "Auberge du Repos Bruyant",
        objective,
        "service", family,
        questExperience(rank, playerLevel, target + 1), adjustedQuestGold(rank, playerLevel, target + 1, "service", true), target, false,
        requiredMaterialId, requiredMaterialName, requiredMaterialQuantity,
        rewardMaterialId, rewardMaterialName, rewardMaterialQuantity,
        "Objectif de quête probable : parler à Tavia et régler une épreuve d'auberge. Les bons de repas/lit/écurie servent maintenant à valider certaines demandes."
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
        "Éclaireur de guilde", "Villageois nerveux", "Marchand inquiet", "Herboriste", "Bibliothécaire", "Vendeur de composants",
        "Sœur Cléria", "Batia des barques", "Tarek le carrier", "Niko sous le pont", "Rosalie des statues"
    }) : preferredClient;

    if (client == "Forgeron" || client == "Armurier" || client == "Vendeur d'armes" || client == "Bram")
    {
        materialQuest = true;
    }

    if (client == "Lysa")
    {
        materialQuest = true;
        combatQuest = false;
    }

    if (client == "Orren" || client == "Soryn")
    {
        materialQuest = false;
        combatQuest = false;
    }

    if (client == "Mira")
    {
        combatQuest = true;
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
