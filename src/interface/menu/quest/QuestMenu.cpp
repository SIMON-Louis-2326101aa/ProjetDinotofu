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
    void displayQuestLine(const Quest& quest, int index)
    {
        std::cout << index << " : [Rang " << quest.rank << "] " << quest.title << std::endl;
        std::cout << "    Origine : " << quest.origin << " | Client : " << quest.client << " | Lieu : " << quest.location << std::endl;
        if (!quest.objectiveType.empty() || !quest.targetFamily.empty())
        {
            std::cout << "    Type : " << (quest.objectiveType.empty() ? "général" : quest.objectiveType)
                      << " | Cible : " << (quest.targetFamily.empty() ? "générale" : quest.targetFamily)
                      << std::endl;
        }
        std::cout << "    Objectif : " << quest.objective << std::endl;
        std::cout << "    Progression : " << quest.progress << "/" << quest.target << " | État : " << questStateText(quest) << std::endl;
        std::cout << "    Récompenses : " << questRewardText(quest) << std::endl;

        if (!quest.requiredMaterialId.empty() && quest.requiredMaterialQuantity > 0)
        {
            std::cout << "    Livraison demandée : " << quest.requiredMaterialName
                      << " x" << quest.requiredMaterialQuantity << std::endl;
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
    void addExplorationMaterial(Player& player, const std::string& id, int quantity, const std::string& quality)
    {
        player.getInventory().addMaterial(MaterialCatalog::createById(id, quantity, quality));
        Material preview = MaterialCatalog::createById(id, quantity, quality);
        std::cout << "Récupéré : " << preview.getName();
        if (preview.hasSpecialQuality())
        {
            std::cout << " [" << preview.getQualityLabel() << "]";
        }
        std::cout << " x" << quantity << std::endl;
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

        if (random.between(1, 100) <= 70)
        {
            addExplorationMaterial(player, biome.commonMaterialId, applyExplorationQuantityBonus(1, intensity), chooseExplorationQuality(random, false));
        }

        int updated = player.getQuestLog().progressCombatQuestsByFamily(1, "Créatures locales");
        if (updated > 0)
        {
            showExplorationNotice(
                "QUÊTE MISE À JOUR",
                "exploration.unexpected_fight.quest_progress",
                {"Une quête de combat progresse grâce à cette menace imprévue."}
            );
        }
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
            showExplorationNotice(
                "COFFRE MODESTE",
                "exploration.chest.modest",
                {
                    "Le coffre est réel, mais son contenu reste modeste.",
                    "Or gagné : " + std::to_string(gold)
                }
            );
            addExplorationMaterial(player, biome.commonMaterialId, applyExplorationQuantityBonus(1, intensity), chooseExplorationQuality(random, true));
        }
        else
        {
            int gold = applyExplorationGoldReward(random.between(35 + player.getLevel() * 3, 90 + player.getLevel() * 8), player, intensity, difficulty, 2);
            player.getInventory().earnGold(gold);
            showExplorationNotice(
                "COFFRE INTACT",
                "exploration.chest.good",
                {
                    "Le coffre est réel, et pour une fois il n'a pas décidé de te mordre.",
                    "Or gagné : " + std::to_string(gold)
                }
            );
            addExplorationMaterial(player, biome.rareMaterialId, applyExplorationQuantityBonus(1, intensity), chooseExplorationQuality(random, true));
        }
    }

    // EN: triggerRareExplorationDiscovery declares or implements a focused behavior used by this module.
    // FR: triggerRareExplorationDiscovery déclare ou implémente un comportement précis utilisé par ce module.
    void triggerRareExplorationDiscovery(Player& player, Random& random, const ExplorationBiome& biome, const ExplorationIntensity& intensity, DifficultyMode difficulty)
    {
        int roll = random.between(1, 100);

        if (roll <= 22)
        {
            showExplorationNotice(
                "DÉCOUVERTE RARE",
                "exploration.rare.discovery.resource",
                {
                    "Un filon / bouquet intact a survécu aux passages précédents.",
                    "Tu prends le temps de récupérer proprement ce qui peut l'être."
                }
            );
            addExplorationMaterial(player, biome.rareMaterialId, applyExplorationQuantityBonus(random.between(1, 2), intensity), chooseExplorationQuality(random, true));
            addExplorationMaterial(player, biome.commonMaterialId, applyExplorationQuantityBonus(1, intensity), chooseExplorationQuality(random, true));
            progressExplorationQuests(player, biome.name, 2);
            return;
        }

        if (roll <= 40)
        {
            int gold = applyExplorationGoldReward(random.between(45 + player.getLevel() * 4, 120 + player.getLevel() * 9), player, intensity, difficulty, 3);
            player.getInventory().earnGold(gold);
            showExplorationNotice(
                "CACHE ANCIENNE",
                "exploration.rare.discovery.cache",
                {
                    "Une cache ancienne est dissimulée sous des marques presque effacées.",
                    "Ce n'est pas un trésor de roi, mais ce n'est clairement pas une trouvaille normale.",
                    "Or gagné : " + std::to_string(gold)
                }
            );
            addExplorationMaterial(player, biome.rareMaterialId, applyExplorationQuantityBonus(1, intensity), chooseExplorationQuality(random, true));
            progressExplorationQuests(player, biome.name, 1);
            return;
        }

        if (roll <= 58)
        {
            showExplorationNotice(
                "TRACES CONSERVÉES",
                "exploration.rare.discovery.traces",
                {
                    "Tu trouves des traces parfaitement conservées.",
                    "Elles ne donnent pas un objet immédiat, mais elles valent beaucoup pour les quêtes et le registre."
                }
            );
            recordBiomeFieldObservation(biome, "Trace rare conservée : " + biome.name + " révèle des présences locales plus anciennes que les rencontres normales.");
            int updated = progressExplorationQuests(player, biome.name, 3);
            if (updated > 0)
            {
                showExplorationNotice("QUÊTES MISES À JOUR", "exploration.rare.discovery.traces.quest", {"Plusieurs notes de quête progressent grâce à ces traces."});
            }
            else
            {
                showExplorationNotice("TRACE NOTÉE", "exploration.rare.discovery.traces.note", {"Tu notes mentalement le lieu : ce genre de trace intéresserait clairement une guilde ou un client."});
            }
            return;
        }

        if (roll <= 76)
        {
            showExplorationNotice(
                "ANOMALIE DE MATÉRIAUX",
                "exploration.rare.discovery.anomaly",
                {
                    "Une petite anomalie de matériaux pulse au sol.",
                    "Tu n'en comprends pas tout, mais tu arrives à détacher un résidu stable."
                }
            );
            addExplorationMaterial(player, "variation_residue", applyExplorationQuantityBonus(1, intensity), chooseExplorationQuality(random, true));
            progressExplorationQuests(player, biome.name, 2);
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
            if (newEntityDetected)
            {
                showExplorationNotice(
                    "REGISTRE DES BOSS",
                    "exploration.rare.discovery.boss_trace.new",
                    {
                        "Une nouvelle entité a été détectée dans le registre des variations d'énergie anormale.",
                        "Nom : ???",
                        "Statut : repérée par exploration rare."
                    }
                );
            }
            else
            {
                showExplorationNotice("REGISTRE DES BOSS", "exploration.rare.discovery.boss_trace.old", {"Le registre garde la trace, mais aucune nouvelle entrée ne se stabilise."});
            }
            progressExplorationQuests(player, biome.name, 2);
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
            addExplorationMaterial(player, biome.rareMaterialId, applyExplorationQuantityBonus(1, intensity), "exceptional");
            progressExplorationQuests(player, biome.name, 3);
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
            intro = client + " te confie une demande liée à " + biomeName + ". La demande porte une marque locale et un sceau récent de la guilde.";
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
        std::cout << "========== ÉVÉNEMENT DE QUÊTE ==========" << std::endl;
        std::cout << intro << std::endl;
        std::cout << std::endl;

        if (!player.getQuestLog().canAcceptPersonalQuestForClient(offeredQuest.client))
        {
            std::cout << offeredQuest.client << " a déjà deux demandes actives dans ton journal." << std::endl;
            std::cout << "Tant qu'au moins une de ses demandes n'est pas rendue, ce PNJ évite de t'en confier une autre." << std::endl;
            return;
        }

        std::cout << "Demande proposée : [Rang " << offeredQuest.rank << "] " << offeredQuest.title << std::endl;
        std::cout << offeredQuest.objective << std::endl;
        std::cout << "Client : " << offeredQuest.client << " | Lieu : " << offeredQuest.location << std::endl;
        std::cout << "Récompenses : " << questRewardText(offeredQuest) << std::endl;

        if (!offeredQuest.requiredMaterialId.empty() && offeredQuest.requiredMaterialQuantity > 0)
        {
            std::cout << "Matériaux à rapporter : " << offeredQuest.requiredMaterialName
                      << " x" << offeredQuest.requiredMaterialQuantity
                      << " (équiv. normale possédée : "
                      << player.getInventory().countMaterialQualityPointsById(offeredQuest.requiredMaterialId) / 2
                      << ")" << std::endl;
        }

        std::cout << std::endl;
        std::cout << "1 : Accepter la demande" << std::endl;
        std::cout << "0 : Refuser poliment" << std::endl;
        std::cout << "========================================" << std::endl;
        std::cout << "> ";

        int choice = Console::askNumberBetween(0, 1, "Choix invalide.");
        Console::clear();

        if (choice == 1)
        {
            if (player.getQuestLog().addQuest(offeredQuest))
            {
                player.getQuestLog().refreshMaterialDeliveryQuests(player.getInventory());
                std::cout << "Demande ajoutée au journal : " << offeredQuest.title << std::endl;
            }
            else
            {
                std::cout << "Impossible d'ajouter cette demande au journal." << std::endl;
            }
        }
        else
        {
            std::cout << "Tu refuses la demande pour l'instant." << std::endl;
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

        std::cout << "L'air se tasse autour de toi." << std::endl;
        std::cout << "Mini-boss d'exploration : " << miniBossName << "." << std::endl;
        std::cout << "Forme rencontrée : " << miniBoss.getName()
                  << " [niveau " << miniBoss.getLevel() << "]." << std::endl;
        if (evolved)
        {
            std::cout << "Cette chose ressemble à une version évoluée d'un monstre local." << std::endl;
        }
        std::cout << std::endl;

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

        addExplorationMaterial(player, evolved ? biome.rareMaterialId : biome.commonMaterialId, applyExplorationQuantityBonus(1, intensity), chooseExplorationQuality(random, evolved));

        int updated = player.getQuestLog().progressCombatQuestsByFamily(evolved ? 2 : 1, questFamily);
        if (updated > 0)
        {
            std::cout << "Des quêtes de combat progressent grâce à cette rencontre." << std::endl;
        }
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

        std::cout << "========== ÉVÉNEMENT APRÈS-COMBAT ==========" << std::endl;
        std::cout << "Tu pensais pouvoir souffler, mais quelque chose a suivi le bruit du combat." << std::endl;
        std::cout << "Mini-boss détecté : " << miniBossName << "." << std::endl;
        std::cout << "La menace est trop proche pour être ignorée : il va falloir survivre." << std::endl;
        std::cout << std::endl;
        std::cout << "1 : Affronter la menace" << std::endl;
        std::cout << "0 : Tenter de l'éviter avant contact" << std::endl;
        std::cout << "> ";

        int choice = Console::askNumberBetween(0, 1, "Choix invalide.");
        Console::clear();

        if (choice == 0)
        {
            int escapeChance = evolved ? 45 : 65;
            if (random.between(1, 100) <= escapeChance)
            {
                std::cout << "Tu t'éloignes avant que la menace ne verrouille vraiment ta position." << std::endl;
                std::cout << "L'événement est évité, mais aucune récompense supplémentaire n'est obtenue." << std::endl;
                return;
            }

            std::cout << "Trop tard. La menace a déjà senti ta fatigue." << std::endl;
            std::cout << std::endl;
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

        int updated = player.getQuestLog().progressCombatQuestsByFamily(evolved ? 2 : 1, questFamily);
        if (updated > 0)
        {
            std::cout << "Le sang versé après l'embuscade fait avancer les contrats de chasse." << std::endl;
        }
    }

    // EN: openDangerousExplorationSite declares or implements a focused behavior used by this module.
    // FR: openDangerousExplorationSite déclare ou implémente un comportement précis utilisé par ce module.
    void openDangerousExplorationSite(Player& player, Random& random, const ExplorationBiome& biome, const ExplorationIntensity& intensity, DifficultyMode difficulty)
    {
        std::cout << "Tu remarques un passage récent vers un lieu qui n'a clairement pas envie d'être visité." << std::endl;
        std::cout << "Lieu repéré : " << dangerousSiteNameForBiome(biome) << "." << std::endl;
        std::cout << dangerousSiteWarningForBiome(biome) << std::endl;
        std::cout << "L'air est trop lourd, les traces trop profondes, et ton instinct te conseille poliment de rentrer." << std::endl;
        std::cout << std::endl;
        std::cout << "1 : Visiter quand même ce lieu dangereux" << std::endl;
        std::cout << "0 : Ignorer l'endroit" << std::endl;
        std::cout << "> ";

        int visitChoice = Console::askNumberBetween(0, 1, "Choix invalide.");
        Console::clear();

        if (visitChoice == 0)
        {
            std::cout << "Tu décides de ne pas offrir ton nom au premier trou suspect venu." << std::endl;
            return;
        }

        bool bossEntrance = random.between(1, 100) <= 35;

        if (!bossEntrance)
        {
            std::cout << "Le lieu abrite une vague de gros monstres." << std::endl;
            std::cout << "Ce n'est pas un simple détour : c'est une embuscade naturelle." << std::endl;
            std::cout << "Tu reconnais assez la zone pour comprendre que ce danger appartient à " << biome.name << "." << std::endl;
            std::cout << std::endl;
            std::cout << "1 : Tenter l'affrontement" << std::endl;
            std::cout << "0 : Reculer maintenant" << std::endl;
            std::cout << "> ";

            int fightChoice = Console::askNumberBetween(0, 1, "Choix invalide.");
            Console::clear();

            if (fightChoice == 0)
            {
                std::cout << "Tu recules avant que la zone ne se referme sur toi." << std::endl;
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
                addExplorationMaterial(player, biome.rareMaterialId, applyExplorationQuantityBonus(1, intensity), chooseExplorationQuality(random, true));
                player.getQuestLog().progressCombatQuestsByFamily(3, "Menace avancée");
            }
            return;
        }

        std::cout << "Ce n'est pas une simple tanière." << std::endl;
        std::cout << "C'est une entrée de boss." << std::endl;
        std::cout << std::endl;
        std::cout << "Description rapide : " << bossTraceForBiome(biome) << "," << std::endl;
        std::cout << "mais trop brouillée pour que le registre accepte son nom." << std::endl;
        std::cout << "Le sol vibre comme si une variation d'énergie anormale venait de respirer." << std::endl;
        std::cout << std::endl;
        std::cout << "1 : Tenter l'affrontement malgré l'avertissement" << std::endl;
        std::cout << "0 : Reculer et mémoriser l'entrée" << std::endl;
        std::cout << "> ";

        int bossChoice = Console::askNumberBetween(0, 1, "Choix invalide.");
        Console::clear();

        if (bossChoice == 0)
        {
            std::cout << "Tu recules. Le registre note seulement : Boss potentiel — nom inconnu." << std::endl;
            progressExplorationQuests(player, biome.name, 1);
            return;
        }

        std::cout << "Tu franchis la limite... puis ton instinct te ramène brutalement en arrière." << std::endl;
        std::cout << "Le registre des Boss grave maintenant son sceau. Reviens par cette voie si tu veux vraiment l'affronter." << std::endl;

        bool newEntityDetected = player.unlockNextBossVariation();
        if (newEntityDetected)
        {
            std::cout << std::endl;
            std::cout << "Une nouvelle entité a été détectée dans le registre des variations d'énergie anormale." << std::endl;
            std::cout << "Nom : ???" << std::endl;
            std::cout << "Statut : éveillé par exploration dangereuse." << std::endl;
        }
        else
        {
            std::cout << "Le registre tremble, mais aucune nouvelle entrée ne se stabilise pour l'instant." << std::endl;
        }

        progressExplorationQuests(player, biome.name, 2);
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
                showExplorationNotice("EMBUSCADE ÉVITÉE", "exploration.event.abandoned_camp.avoid", {"Tu lis correctement les traces et évites l'embuscade avant qu'elle ne se referme."});
                progressExplorationQuests(player, biome.name, 1);
            }

            int gold = applyExplorationGoldReward(random.between(12, 38 + player.getLevel() * 2), player, intensity, difficulty, 1);
            player.getInventory().earnGold(gold);
            showExplorationNotice("CAMP FOUILLÉ", "exploration.event.abandoned_camp.reward", {"Tu récupères dans le camp : " + std::to_string(gold) + " pièces."});
            addExplorationMaterial(player, biome.commonMaterialId, applyExplorationQuantityBonus(1, intensity), chooseExplorationQuality(random, true));
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
                showExplorationNotice("REPAIRE IGNORÉ", "exploration.event.local_den.leave", {"Tu marques mentalement le lieu, mais tu ne vas pas mourir pour trois bouts de cuir."});
                progressExplorationQuests(player, biome.name, 1);
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
                addExplorationMaterial(player, biome.commonMaterialId, applyExplorationQuantityBonus(random.between(1, 2), intensity), chooseExplorationQuality(random, true));
                if (random.between(1, 100) <= 45)
                {
                    addExplorationMaterial(player, biome.rareMaterialId, applyExplorationQuantityBonus(1, intensity), chooseExplorationQuality(random, true));
                }
                player.getQuestLog().progressCombatQuestsByFamily(2, "Créatures locales");
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
                showExplorationNotice("TRACE IGNORÉE", "exploration.event.tracks.leave", {"Tu notes mentalement le lieu, sans jouer au héros inutilement."});
                progressExplorationQuests(player, biome.name, 1);
                return;
            }

            std::string clue = "Trace étudiée : " + biome.name + " favorise " + biome.commonMonsters
                + ". Présences rares possibles : " + biome.rareMonsters + ".";
            recordBiomeFieldObservation(biome, clue);
            progressExplorationQuests(player, biome.name, choice == 1 ? 2 : 1);
            showExplorationNotice("BESTIAIRE", "exploration.event.tracks.bestiary", {"Le bestiaire ajoute une observation de terrain sur " + biome.name + "."});

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
                    addExplorationMaterial(player, biome.commonMaterialId, applyExplorationQuantityBonus(1, intensity), chooseExplorationQuality(random, true));
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
                showExplorationNotice("OBSERVATION", "exploration.event.hazard.observe", {environmentalObservationForBiome(biome)});
                recordBiomeFieldObservation(biome, environmentalObservationForBiome(biome));
                progressExplorationQuests(player, biome.name, 2);
                return;
            }

            if (choice == 2)
            {
                int successChance = 62 + intensity.carefulBonus * 4;
                if (random.between(1, 100) <= successChance)
                {
                    showExplorationNotice("RÉCUPÉRATION RÉUSSIE", "exploration.event.hazard.collect_success", {"Tu récupères sans réveiller toute la zone."});
                    addExplorationMaterial(player, biome.commonMaterialId, applyExplorationQuantityBonus(1, intensity), chooseExplorationQuality(random, true));
                    if (random.between(1, 100) <= 28)
                    {
                        addExplorationMaterial(player, biome.rareMaterialId, 1, chooseExplorationQuality(random, true));
                    }
                    progressExplorationQuests(player, biome.name, 2);
                    return;
                }

                int damage = std::min(random.between(4, 12 + player.getLevel()), std::max(0, player.getHp() - 1));
                showExplorationNotice("RÉCUPÉRATION RISQUÉE", "exploration.event.hazard.collect_fail", {"La zone répond mal à ta récupération."});
                if (damage > 0)
                {
                    player.takeDamage(damage);
                    showExplorationNotice("DÉGÂTS", "exploration.event.hazard.damage", {"Tu subis " + std::to_string(damage) + " dégâts, mais tu gardes le contrôle."});
                }
                recordBiomeFieldObservation(biome, environmentalObservationForBiome(biome));
                progressExplorationQuests(player, biome.name, 1);
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
                addExplorationMaterial(player, biome.rareMaterialId, applyExplorationQuantityBonus(1, intensity), chooseExplorationQuality(random, true));
                progressExplorationQuests(player, biome.name, 2);
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
            addExplorationMaterial(player, found, random.between(1, 3), chooseExplorationQuality(random, true));
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
                addExplorationMaterial(player, biome.commonMaterialId, applyExplorationQuantityBonus(1, intensity), chooseExplorationQuality(random, false));
                progressExplorationQuests(player, biome.name, 2);
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
                showExplorationNotice("SIGNE RESPECTÉ", "exploration.event.ancient_sign.leave", {"Tu respectes l'endroit. Le registre note quand même la position."});
                progressExplorationQuests(player, biome.name, 1);
                return;
            }

            if (choice == 1)
            {
                showExplorationNotice("SIGNE ÉTUDIÉ", "exploration.event.ancient_sign.study", {"Tu prends des notes. Le bestiaire garde maintenant cette observation de terrain."});
                recordBiomeFieldObservation(
                    biome,
                    "Signe ancien étudié : le biome " + biome.name + " semble lié à des variations locales et à des présences plus rares."
                );
                progressExplorationQuests(player, biome.name, 3);
                if (random.between(1, 100) <= 35)
                {
                    addExplorationMaterial(player, "variation_residue", 1, chooseExplorationQuality(random, true));
                }
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
                addExplorationMaterial(player, "variation_residue", applyExplorationQuantityBonus(1, intensity), chooseExplorationQuality(random, true));
                addExplorationMaterial(player, biome.rareMaterialId, 1, chooseExplorationQuality(random, true));
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
        showExplorationNotice("RÉCOMPENSE IMPROVISÉE", "exploration.event.distress_call.reward", {"Récompense improvisée : " + std::to_string(gold) + " pièces."});
        offerExplorationNpcQuest(player, random, biome);
    }

}

// EN: openQuestHub declares or implements a focused behavior used by this module.
// FR: openQuestHub déclare ou implémente un comportement précis utilisé par ce module.
void QuestMenu::openQuestHub(Player& player)
{
    while (true)
    {
        MenuScreen screen("QUÊTES", "quest.hub");
        screen.addLine("Les quêtes progressent en combattant, explorant, récupérant des ressources ou battant les bonnes cibles.");
        screen.addOption(0, "Retour", "", true, "quest.hub.back");
        screen.addOption(1, "Consulter le journal de quêtes", "", true, "quest.hub.journal");
        screen.addOption(2, "Aller à la guilde", "", true, "quest.hub.guild");
        int choice = TerminalInterface::askMenuChoice(screen, 0, 2, "Choix invalide.");
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
            openGuild(player);
        }
    }
}

// EN: consultOnly declares or implements a focused behavior used by this module.
// FR: consultOnly déclare ou implémente un comportement précis utilisé par ce module.
void QuestMenu::consultOnly(const Player& player)
{
    std::cout << "Note : depuis ce menu, tu peux seulement consulter." << std::endl;
    std::cout << "Pour accepter ou valider une quête, retourne voir la guilde ou le client." << std::endl;
    std::cout << std::endl;
    displayQuestJournal(player);
}

// EN: displayQuestJournal declares or implements a focused behavior used by this module.
// FR: displayQuestJournal déclare ou implémente un comportement précis utilisé par ce module.
void QuestMenu::displayQuestJournal(const Player& player)
{
    const std::vector<Quest>& quests = player.getQuestLog().getQuests();
    constexpr std::size_t questsPerPage = 3;
    std::size_t activePage = 0;
    std::size_t completedPage = 0;
    bool showingCompleted = false;

    while (true)
    {
        std::vector<const Quest*> activeGuildQuests;
        std::vector<const Quest*> activePersonalQuests;
        std::vector<const Quest*> completedQuests;

        for (const Quest& quest : quests)
        {
            if (quest.turnedIn)
            {
                completedQuests.push_back(&quest);
            }
            else if (quest.guildQuest)
            {
                activeGuildQuests.push_back(&quest);
            }
            else
            {
                activePersonalQuests.push_back(&quest);
            }
        }

        std::vector<const Quest*> activeQuests = activeGuildQuests;
        activeQuests.insert(activeQuests.end(), activePersonalQuests.begin(), activePersonalQuests.end());

        std::vector<const Quest*>& displayedQuests = showingCompleted ? completedQuests : activeQuests;
        std::size_t& pageIndex = showingCompleted ? completedPage : activePage;
        const std::size_t totalPages = PagedMenu::pageCount(displayedQuests.size(), questsPerPage);

        if (pageIndex >= totalPages)
        {
            pageIndex = totalPages == 0 ? 0 : totalPages - 1;
        }

        const std::size_t first = PagedMenu::firstIndex(pageIndex, questsPerPage);
        const std::size_t last = PagedMenu::lastIndexExclusive(displayedQuests.size(), pageIndex, questsPerPage);

        std::cout << "========== JOURNAL DE QUÊTES ==========" << std::endl;

        if (quests.empty())
        {
            std::cout << "Aucune quête acceptée pour l'instant." << std::endl;
            std::cout << "La guilde propose des contrats, et certains PNJ peuvent aussi te demander un service." << std::endl;
            std::cout << std::endl;
            std::cout << "0 : Retour" << std::endl;
            std::cout << "=======================================" << std::endl;
            std::cout << "> ";
            Console::askNumberBetween(0, 0, "Choix invalide.");
            Console::clear();
            return;
        }

        std::cout << "Quêtes de guilde actives : " << player.getQuestLog().getActiveGuildQuestCount() << "/3" << std::endl;
        std::cout << "Vue : " << (showingCompleted ? "quêtes terminées / rendues" : "quêtes actives") << std::endl;
        PagedMenu::printPageInfo(pageIndex, totalPages, displayedQuests.size());
        std::cout << "Affichage : " << PagedMenu::rangeText(first, last, displayedQuests.size()) << std::endl;
        std::cout << std::endl;

        if (displayedQuests.empty())
        {
            std::cout << (showingCompleted ? "Aucune quête rendue pour l'instant." : "Aucune quête active pour l'instant.") << std::endl;
            std::cout << std::endl;
        }
        else
        {
            for (std::size_t i = first; i < last; ++i)
            {
                const Quest& quest = *displayedQuests[i];
                const bool firstPersonalQuest = !showingCompleted
                    && i > 0
                    && displayedQuests[i - 1]->guildQuest
                    && !quest.guildQuest;

                if (!showingCompleted && i == first)
                {
                    std::cout << (quest.guildQuest ? "--- Quêtes actives de guilde ---" : "--- Demandes actives / événements / clients ---") << std::endl;
                }
                else if (firstPersonalQuest)
                {
                    std::cout << "--- Demandes actives / événements / clients ---" << std::endl;
                }

                displayQuestLine(quest, static_cast<int>(i - first + 1));
                std::cout << std::endl;
            }
        }

        PagedMenu::printNavigation(pageIndex, totalPages);
        if (!showingCompleted)
        {
            std::cout << "1 : Voir les quêtes terminées / rendues"
                      << (completedQuests.empty() ? " (aucune)" : "")
                      << std::endl;
        }
        else
        {
            std::cout << "1 : Revenir aux quêtes actives" << std::endl;
        }

        std::cout << "=======================================" << std::endl;
        std::cout << "> ";

        int choice = Console::askNumberBetween(0, 99, "Choix invalide.");
        Console::clear();

        if (choice == 0)
        {
            return;
        }

        if (choice == 1)
        {
            showingCompleted = !showingCompleted;
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

        std::cout << "Ce choix ne correspond à aucune action du journal." << std::endl;
        std::cout << std::endl;
    }
}

// EN: openGuild declares or implements a focused behavior used by this module.
// FR: openGuild déclare ou implémente un comportement précis utilisé par ce module.
void QuestMenu::openGuild(Player& player)
{
    while (true)
    {
        MenuScreen screen("GUILDE", "quest.guild");
        screen.addLine("La guilde centralise les quêtes officielles.");
        screen.addLine("Tu peux avoir jusqu'à 3 quêtes de guilde actives.");
        screen.addOption(0, "Retour", "", true, "quest.guild.back");
        screen.addOption(1, "Voir le panneau de quêtes", "", true, "quest.guild.board");
        screen.addOption(2, "Rendre une quête de guilde terminée", "", true, "quest.guild.turn_in");
        screen.addOption(3, "Consulter le journal", "", true, "quest.guild.journal");
        int choice = TerminalInterface::askMenuChoice(screen, 0, 3, "Choix invalide.");
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

    std::cout << "========== PANNEAU DE GUILDE ==========" << std::endl;
    std::cout << "Quêtes actives : " << questLog.getActiveGuildQuestCount() << "/3" << std::endl;
    std::cout << "Offres visibles : " << board.size() << "/" << questLog.getGuildBoardTargetSize() << std::endl;

    int remainingBeforeRefresh = questLog.getGuildBoardCombatsBeforeRefresh(player.getCombatsStarted());
    if (remainingBeforeRefresh <= 0)
    {
        std::cout << "Le panneau sera réécrit au prochain passage." << std::endl;
    }
    else
    {
        std::cout << "Le panneau actuel reste affiché encore " << remainingBeforeRefresh
                  << " combat" << (remainingBeforeRefresh > 1 ? "s" : "") << "." << std::endl;
    }

    if (questLog.getGuildBoardPendingReplacements() > 0)
    {
        std::cout << "Des places prises seront remplacées après le prochain combat." << std::endl;
    }

    std::cout << "0 : Retour" << std::endl;

    for (int i = 0; i < static_cast<int>(board.size()); ++i)
    {
        std::cout << i + 1 << " : [Rang " << board[i].rank << "] " << board[i].title
                  << " | " << questRewardText(board[i]);

        if (questLog.hasQuest(board[i].id))
        {
            std::cout << " | déjà prise";
        }

        std::cout << std::endl;
    }

    if (board.empty())
    {
        std::cout << "Le panneau est vide pour l'instant. Repasse après un combat." << std::endl;
    }

    std::cout << "=======================================" << std::endl;
    std::cout << "> ";

    int choice = Console::askNumberBetween(0, static_cast<int>(board.size()), "Choix invalide.");
    Console::clear();

    if (choice == 0)
    {
        return;
    }

    Quest selectedQuest = board[choice - 1];

    if (!questLog.canAcceptGuildQuest())
    {
        std::cout << "Tu as déjà 3 quêtes de guilde actives." << std::endl;
        std::cout << "Termine ou rends-en une avant d'en accepter une autre." << std::endl;
    }
    else if (questLog.addQuest(selectedQuest))
    {
        questLog.removeGuildBoardOfferAt(choice - 1, player.getCombatsStarted());
        std::cout << "Quête acceptée : " << selectedQuest.title << std::endl;
        std::cout << "L'annonce est retirée du panneau. Une nouvelle place sera préparée après ton prochain combat." << std::endl;
    }
    else
    {
        std::cout << "Impossible d'accepter cette quête. Elle est peut-être déjà active." << std::endl;
    }

    std::cout << std::endl;
    Console::waitForEnter();
    Console::clear();
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
    while (true)
    {
        MenuScreen screen("LIEUX VISITABLES", "quest.locations");
        screen.addOption(0, "Retour", "", true, "quest.locations.back");
        screen.addOption(1, "Guilde", "", true, "quest.locations.guild");
        screen.addOption(2, "Forge", "", true, "quest.locations.forge");
        screen.addOption(3, "Herboristerie", "", true, "quest.locations.herbalist");
        screen.addOption(4, "Place du village", "", true, "quest.locations.village_square");
        screen.addOption(5, "Route commerciale", "", true, "quest.locations.trade_road");
        screen.addOption(6, "Boutique de monstres", "", true, "quest.locations.monster_shop");
        screen.addOption(7, "Boutique de matériaux", "", true, "quest.locations.material_shop");
        screen.addOption(8, "Armurerie défensive", "", true, "quest.locations.armor_shop");
        screen.addOption(9, "Forge d'armes", "", true, "quest.locations.weapon_shop");
        screen.addOption(10, "Boutique de consommables", "", true, "quest.locations.consumable_shop");
        screen.addOption(11, "Bibliothèque", "", true, "quest.locations.library");
        screen.addOption(12, "Ouvrir l'inventaire", "", true, "quest.locations.inventory");
        int choice = TerminalInterface::askMenuChoice(screen, 0, 12, "Choix invalide.");
        Console::clear();

        if (choice == 0)
        {
            return;
        }

        if (choice == 1)
        {
            openGuild(player);
        }
        else if (choice == 2)
        {
            talkToClient(player, "Forgeron");
        }
        else if (choice == 3)
        {
            talkToClient(player, "Alchimiste");
        }
        else if (choice == 4)
        {
            talkToClient(player, "Villageois nerveux");
        }
        else if (choice == 5)
        {
            talkToClient(player, "Marchand inquiet");
        }
        else if (choice == 6)
        {
            talkToClient(player, "Vendeur de composants");
        }
        else if (choice == 7)
        {
            talkToClient(player, "Vendeur de matériaux");
        }
        else if (choice == 8)
        {
            talkToClient(player, "Armurier");
        }
        else if (choice == 9)
        {
            talkToClient(player, "Vendeur d'armes");
        }
        else if (choice == 10)
        {
            talkToClient(player, "Vendeur de consommables");
        }
        else if (choice == 11)
        {
            talkToClient(player, "Bibliothécaire");
        }
        else if (choice == 12)
        {
            InventoryMenu::open(player);
            Console::clear();
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
            {"Maître de guilde", "PNJ important"},
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
        screen.addOption(0, "Retour", "", true, "quest.notable_npc.back");

        bool printedRecommendedHeader = false;
        for (int i = 0; i < static_cast<int>(entries.size()); ++i)
        {
            if (!printedRecommendedHeader && entries[i].second == "Recommandé par un habitant")
            {
                printedRecommendedHeader = true;
                screen.addLine("--- Recommandés par un habitant ---");
            }

            screen.addOption(
                i + 1,
                entries[i].first + " (" + entries[i].second + ")",
                "",
                true,
                "quest.notable_npc.select." + std::to_string(i + 1)
            );
        }

        if (recommendedClients.empty())
        {
            screen.addLine("--- Recommandés par un habitant ---");
            screen.addLine("Aucun nom recommandé pour l'instant.");
        }

        int choice = TerminalInterface::askMenuChoice(screen, 0, static_cast<int>(entries.size()), "Choix invalide.");
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
        MenuScreen screen(clientName, "quest.client");
        screen.addOption(0, "Retour", "", true, "quest.client.back");
        screen.addOption(1, "Parler", "", true, "quest.client.talk");
        screen.addOption(2, "Voir / rendre une demande terminée", "", true, "quest.client.turn_in");
        int choice = TerminalInterface::askMenuChoice(screen, 0, 2, "Choix invalide.");
        Console::clear();

        if (choice == 0)
        {
            return;
        }

        if (choice == 2)
        {
            completeQuestAtClient(player, clientName);
            continue;
        }

        if (isRecommendedClientName(clientName))
        {
            int usedRequests = player.getQuestLog().getClientQuestCount(clientName);
            if (usedRequests >= 5)
            {
                std::cout << clientName << " n'a plus de nouvelles demandes à confier." << std::endl;
                std::cout << "Son nom quitte naturellement la liste des contacts recommandés." << std::endl;
                std::cout << std::endl;
                Console::waitForEnter();
                Console::clear();
                return;
            }

            std::cout << "Demandes confiées par ce contact : " << usedRequests << "/5." << std::endl;
        }

        Quest offeredQuest;
        Random questRandom;

        if (questRandom.between(1, 100) <= 70)
        {
            std::string targetedBiome = randomBiomeForClient(questRandom, clientName);
            std::cout << clientName << " n'a rien de sérieux à confier pour le moment." << std::endl;
            std::cout << "Cette fois, il vise une zone précise : " << targetedBiome << "." << std::endl;
            offeredQuest = QuestCatalog::createBiomeRequest(player.getLevel(), targetedBiome, clientName);
        }
        else if (clientName == "Forgeron")
        {
            std::cout << "Le forgeron essuie ses mains noircies et te jauge du regard." << std::endl;
            offeredQuest = QuestCatalog::createForgemasterMaterialRequest(player.getLevel());
        }
        else if (clientName == "Alchimiste")
        {
            std::cout << "L'alchimiste sourit comme si son idée allait forcément exploser." << std::endl;
            offeredQuest = QuestCatalog::createAlchemistIngredientRequest(player.getLevel());
        }
        else if (clientName == "Villageois nerveux")
        {
            std::cout << "Le villageois te rattrape presque en courant." << std::endl;
            offeredQuest = QuestCatalog::createVillagerMonsterFearRequest(player.getLevel());
        }
        else if (clientName == "Marchand inquiet")
        {
            std::cout << "Le marchand tient une caisse vide et un sourire beaucoup trop forcé." << std::endl;
            offeredQuest = QuestCatalog::createMerchantDeliveryRequest(player.getLevel());
        }
        else if (clientName == "Vendeur de composants")
        {
            std::cout << "Le vendeur aligne des bocaux pas vraiment rassurants." << std::endl;
            offeredQuest = QuestCatalog::createMonsterMaterialVendorRequest(player.getLevel());
        }
        else if (clientName == "Vendeur de matériaux")
        {
            std::cout << "Le vendeur tapote une étagère presque vide." << std::endl;
            offeredQuest = QuestCatalog::createMaterialVendorRequest(player.getLevel());
        }
        else if (clientName == "Herboriste")
        {
            std::cout << "L'herboriste trie des feuilles avec une précision maniaque." << std::endl;
            offeredQuest = QuestCatalog::createHerbalistRequest(player.getLevel());
        }
        else if (clientName == "Armurier")
        {
            std::cout << "L'armurier soupire devant une pile de protections abîmées." << std::endl;
            offeredQuest = QuestCatalog::createArmorerRequest(player.getLevel());
        }
        else if (clientName == "Vendeur d'armes")
        {
            std::cout << "Le vendeur d'armes vérifie ses lames une par une." << std::endl;
            offeredQuest = QuestCatalog::createWeaponVendorRequest(player.getLevel());
        }
        else if (clientName == "Vendeur de consommables")
        {
            std::cout << "Le vendeur de consommables recompte ses flacons avec inquiétude." << std::endl;
            offeredQuest = QuestCatalog::createConsumableVendorRequest(player.getLevel());
        }
        else if (isRecommendedClientName(clientName))
        {
            std::cout << clientName << " t'accueille grâce à une recommandation griffonnée sur un billet." << std::endl;
            std::cout << "Ce contact n'a pas encore pignon sur rue, mais il a déjà une demande précise." << std::endl;
            offeredQuest = QuestCatalog::createBiomeRequest(player.getLevel(), randomBiomeForClient(questRandom, clientName), clientName);
        }
        else
        {
            std::cout << "La bibliothécaire te montre des notes incomplètes." << std::endl;
            offeredQuest = QuestCatalog::createLibrarianRequest(player.getLevel());
        }

        if (!player.getQuestLog().canAcceptPersonalQuestForClient(offeredQuest.client))
        {
            std::cout << std::endl;
            std::cout << offeredQuest.client << " a déjà deux demandes actives dans ton journal." << std::endl;
            std::cout << "Il préfère attendre que tu lui rendes au moins une mission avant d'en confier une autre." << std::endl;
            std::cout << std::endl;
            Console::waitForEnter();
            Console::clear();
            continue;
        }

        std::cout << std::endl;
        std::cout << "Demande proposée : [Rang " << offeredQuest.rank << "] " << offeredQuest.title << std::endl;
        std::cout << offeredQuest.objective << std::endl;
        std::cout << "Récompenses : " << questRewardText(offeredQuest) << std::endl;

        if (!offeredQuest.requiredMaterialId.empty() && offeredQuest.requiredMaterialQuantity > 0)
        {
            std::cout << "Matériaux à rapporter : " << offeredQuest.requiredMaterialName
                      << " x" << offeredQuest.requiredMaterialQuantity
                      << " (possédé : " << player.getInventory().countMaterialById(offeredQuest.requiredMaterialId)
                      << ", équiv. normale : " << player.getInventory().countMaterialQualityPointsById(offeredQuest.requiredMaterialId) / 2 << ")"
                      << std::endl;
        }

        std::cout << std::endl;
        std::cout << "1 : Accepter" << std::endl;
        std::cout << "0 : Refuser" << std::endl;
        std::cout << "> ";

        int accept = Console::askNumberBetween(0, 1, "Choix invalide.");
        Console::clear();

        if (accept == 1)
        {
            if (player.getQuestLog().addQuest(offeredQuest))
            {
                player.getQuestLog().refreshMaterialDeliveryQuests(player.getInventory());
                std::cout << "Demande acceptée : " << offeredQuest.title << std::endl;
            }
            else
            {
                std::cout << "Cette demande est déjà active ou impossible à ajouter." << std::endl;
            }

            std::cout << std::endl;
            Console::waitForEnter();
            Console::clear();
        }
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
        std::cout << "Aucune quête prête à être rendue à " << clientName << "." << std::endl;
        std::cout << std::endl;
        Console::waitForEnter();
        Console::clear();
        return;
    }

    std::cout << "========== QUÊTES À RENDRE ==========" << std::endl;
    std::cout << "0 : Retour" << std::endl;

    for (int i = 0; i < static_cast<int>(readyIndexes.size()); ++i)
    {
        const Quest& quest = quests[readyIndexes[i]];
        std::cout << i + 1 << " : [Rang " << quest.rank << "] " << quest.title
                  << " | " << questRewardText(quest);

        if (isMaterialDeliveryQuest(quest))
        {
            std::cout << " | " << quest.requiredMaterialName
                      << " " << player.getInventory().countMaterialById(quest.requiredMaterialId)
                      << " (équiv. " << player.getInventory().countMaterialQualityPointsById(quest.requiredMaterialId) / 2 << ")/" << quest.requiredMaterialQuantity;
        }

        std::cout << std::endl;
    }

    std::cout << "=====================================" << std::endl;
    std::cout << "> ";

    int choice = Console::askNumberBetween(0, static_cast<int>(readyIndexes.size()), "Choix invalide.");
    Console::clear();

    if (choice == 0)
    {
        return;
    }

    Quest& quest = quests[readyIndexes[choice - 1]];

    if (!quest.requiredMaterialId.empty() && quest.requiredMaterialQuantity > 0)
    {
        int owned = player.getInventory().countMaterialQualityPointsById(quest.requiredMaterialId) / 2;

        if (owned < quest.requiredMaterialQuantity)
        {
            std::cout << "Il manque des matériaux pour rendre cette quête." << std::endl;
            std::cout << quest.requiredMaterialName << " requis : " << quest.requiredMaterialQuantity
                      << " (possédé : " << owned << ")" << std::endl;
            std::cout << std::endl;
            Console::waitForEnter();
            Console::clear();
            return;
        }

        player.getInventory().removeMaterialQuantityByIdFlexible(quest.requiredMaterialId, quest.requiredMaterialQuantity);
        std::cout << "Matériaux remis : " << quest.requiredMaterialName
                  << " x" << quest.requiredMaterialQuantity << std::endl;
    }

    quest.completed = true;
    quest.progress = quest.target;
    quest.turnedIn = true;
    player.gainExperience(quest.rewardExperience);
    player.getInventory().earnGold(quest.rewardGold);
    applyQuestExtraReward(player, quest);

    std::cout << "Quête validée : " << quest.title << std::endl;
    std::cout << "XP gagnée : " << quest.rewardExperience << std::endl;
    if (quest.rewardGold > 0)
    {
        std::cout << "Or gagné : " << quest.rewardGold << " pièces" << std::endl;
    }
    else
    {
        std::cout << "Prime en or : aucune" << std::endl;
    }

    if (!quest.rewardMaterialId.empty() && quest.rewardMaterialQuantity > 0)
    {
        std::cout << "Objet reçu : " << quest.rewardMaterialName << " x" << quest.rewardMaterialQuantity << std::endl;

        if (quest.rewardMaterialId == "client_recommendation")
        {
            std::string recommendedClient = extractRecommendedClientName(quest);
            if (!recommendedClient.empty())
            {
                std::cout << "Nouveau contact ajouté aux PNJ notables : " << recommendedClient << " [Recommandé par un habitant]" << std::endl;
            }
            else
            {
                std::cout << "Un nouveau contact pourra apparaître dans la section des PNJ recommandés." << std::endl;
            }
        }
    }

    if (!quest.rewardNote.empty())
    {
        std::cout << quest.rewardNote << std::endl;
    }
    std::cout << std::endl;
    Console::waitForEnter();
    Console::clear();
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

    std::cout << std::endl;
    Console::waitForEnter();
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

            if (hasPotentialQuestForBiome(player, biomes[i]))
            {
                label += " [Objectif de quête probable]";
            }

            screen.addOption(i + 1, label, "", true, "exploration.biome." + std::to_string(i + 1));
        }

        TerminalInterface::renderMenuScreen(screen);
        std::cout << "> ";

        int choice = Console::askNumberBetween(0, static_cast<int>(biomes.size()), "Choix invalide.");
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
                "",
                true,
                "exploration.intensity." + std::to_string(i + 1)
            );
        }

        TerminalInterface::renderMenuScreen(intensityScreen);
        std::cout << "> ";

        int intensityChoice = Console::askNumberBetween(0, static_cast<int>(intensities.size()), "Choix invalide.");
        Console::clear();

        if (intensityChoice == 0)
        {
            continue;
        }

        const ExplorationIntensity& intensity = intensities[intensityChoice - 1];
        Random random;
        QuestSearchHint questHint = getQuestSearchHintForBiome(player, biome);
        int roll = adjustExplorationEventRoll(random.between(1, 100), intensity);
        roll = adjustExplorationRollForActiveQuests(roll, random, questHint);
        bool carefulRecovery = chooseCarefulRecovery(random, intensity);

        std::cout << "========== " << biome.name << " ==========" << std::endl;
        std::cout << "Style : " << biome.style << "." << std::endl;
        std::cout << "Niveaux locaux : " << biome.minLevel << "-" << biome.maxLevel << "." << std::endl;
        if (isBiomeEvolvedForPlayer(player, biome))
        {
            std::cout << "Adaptation de zone : ton niveau attire maintenant des menaces plus fortes ici." << std::endl;
            std::cout << "Niveaux effectifs actuels : " << evolvedBiomeMinLevel(player, biome)
                      << "-" << evolvedBiomeMaxLevel(player, biome) << "." << std::endl;
            std::cout << "Les récompenses suivent mieux ce danger, car les rencontres générées montent aussi en niveau." << std::endl;
        }
        std::cout << "Monstres surtout présents : " << biome.commonMonsters << "." << std::endl;
        std::cout << "Rares / élites typiques : " << biome.rareMonsters << "." << std::endl;
        std::cout << "Approche : " << intensity.name << "." << std::endl;
        if (questHint.hasAny)
        {
            std::cout << "Ton journal réagit légèrement : cette zone peut aider une quête active, sans garantir la trouvaille." << std::endl;
        }
        std::cout << std::endl;

        if (roll <= 24)
        {
            std::cout << "Tu fouilles calmement la zone." << std::endl;
            if (carefulRecovery)
            {
                std::cout << "Récolte propre : tu récupères la ressource de la meilleure façon possible." << std::endl;
            }
            addExplorationMaterial(player, biome.commonMaterialId, applyExplorationQuantityBonus(random.between(1, 2), intensity), chooseExplorationQuality(random, carefulRecovery));
        }
        else if (roll <= 37)
        {
            std::cout << "Une trace intéressante attire ton attention." << std::endl;
            int updated = progressExplorationQuests(player, biome.name, random.between(1, 2));
            if (updated > 0)
            {
                std::cout << "Des quêtes d'exploration progressent grâce à cette découverte." << std::endl;
            }
            else
            {
                std::cout << "Tu gardes mentalement l'endroit en tête, même si aucune quête actuelle ne l'exploite." << std::endl;
            }
        }
        else if (roll <= 48)
        {
            int gold = applyExplorationGoldReward(random.between(5, 22 + player.getLevel() * 2), player, intensity, difficulty, 1);
            std::cout << "Tu trouves un petit trésor au sol." << std::endl;
            player.getInventory().earnGold(gold);
            std::cout << "Or gagné : " << gold << std::endl;
        }
        else if (roll <= 54)
        {
            std::cout << "Tu trouves beaucoup de pièces d'or." << std::endl;
            std::cout << "Pendant une seconde, tu te vois déjà riche." << std::endl;
            std::cout << "Mais en les prenant dans ta main, les pièces fondent entre tes doigts." << std::endl;
            std::cout << "De fausses pièces. Une arnaque magique ridicule." << std::endl;
            std::cout << "Tu décides de laisser toute cette honte au sol." << std::endl;
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
        std::cout << std::endl;
        Console::waitForEnter();
        Console::clear();
    }
}
