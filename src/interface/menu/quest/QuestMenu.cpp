// EN: QuestMenu.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: QuestMenu.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu.
// Description: Implements quest hub and read-only quest journal for Dinotofu.

#include "interface/menu/quest/QuestMenu.hpp"

#include "core/Console.hpp"
#include "core/Random.hpp"
#include "quest/QuestCatalog.hpp"
#include "item/material/MaterialCatalog.hpp"

#include <iostream>
#include <vector>
#include <algorithm>

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
        std::cout << "    Récompenses : XP +" << quest.rewardExperience << " | Or +" << quest.rewardGold << std::endl;

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

    std::string lowerCopy(std::string value)
    {
        std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });

        return value;
    }

    std::string miniBossNameForBiome(const ExplorationBiome& biome, bool evolved)
    {
        if (biome.name == "Forêt ancienne") return evolved ? "Loup ancien à mousse noire" : "Gardien de ronces";
        if (biome.name == "Montagne froide") return evolved ? "Yéti aux éclats de givre" : "Briseur de roche gelée";
        if (biome.name == "Marais trouble") return evolved ? "Slime putride couronné" : "Noyeur du marais";
        if (biome.name == "Route commerciale") return evolved ? "Pillard vétéran marqué" : "Chef de bande opportuniste";
        if (biome.name == "Ruines effondrées") return evolved ? "Sentinelle osseuse éveillée" : "Gardien fissuré des ruines";
        if (biome.name == "Plaine sauvage") return evolved ? "Alpha aux crocs longs" : "Bête territoriale";
        return evolved ? "Créature évoluée locale" : "Menace locale isolée";
    }

    std::string miniBossQuestFamilyForBiome(const ExplorationBiome& biome, bool evolved)
    {
        if (evolved) return "Mini-boss / menace évoluée";
        if (biome.name == "Route commerciale") return "Humanoïdes / embuscades";
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
        if (biome.name == "Plaine sauvage") return "un alpha invisible tourne autour de toi, assez loin pour ne laisser qu'une pression";
        return "une variation d'énergie anormale refuse de porter un nom stable";
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

    // EN: applyExplorationGoldBonus declares or implements a focused behavior used by this module.
    // FR: applyExplorationGoldBonus déclare ou implémente un comportement précis utilisé par ce module.
    int applyExplorationGoldBonus(int baseGold, const ExplorationIntensity& intensity)
    {
        return std::max(1, baseGold * intensity.goldPercent / 100);
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
            std::vector<std::string> biomes = {"Forêt ancienne", "Marais trouble", "Montagne froide", "Plaine sauvage"};
            return biomes[random.between(0, static_cast<int>(biomes.size()) - 1)];
        }

        if (clientName == "Marchand inquiet")
        {
            std::vector<std::string> biomes = {"Route commerciale", "Plaine sauvage", "Ruines effondrées"};
            return biomes[random.between(0, static_cast<int>(biomes.size()) - 1)];
        }

        if (clientName == "Vendeur de composants" || clientName == "Villageois nerveux")
        {
            std::vector<std::string> biomes = {"Forêt ancienne", "Marais trouble", "Route commerciale", "Ruines effondrées", "Plaine sauvage"};
            return biomes[random.between(0, static_cast<int>(biomes.size()) - 1)];
        }

        if (clientName == "Bibliothécaire")
        {
            std::vector<std::string> biomes = {"Forêt ancienne", "Montagne froide", "Marais trouble", "Ruines effondrées"};
            return biomes[random.between(0, static_cast<int>(biomes.size()) - 1)];
        }

        std::vector<std::string> biomes = {"Forêt ancienne", "Montagne froide", "Marais trouble", "Route commerciale", "Ruines effondrées", "Plaine sauvage"};
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
    void simulateUnexpectedExplorationFight(Player& player, Random& random, const ExplorationBiome& biome, const ExplorationIntensity& intensity)
    {
        int damage = random.between(4 + player.getLevel(), 12 + player.getLevel() * 2);
        int actualDamage = std::min(damage, std::max(0, player.getHp() - 1));
        int experience = 12 + player.getLevel() * 5 + random.between(0, 12);
        std::string quality = chooseExplorationQuality(random, false);

        std::cout << "Un mouvement anormal coupe ta fouille." << std::endl;
        std::cout << "Un ennemi surgit sans prévenir : ce n'était pas une sortie de combat volontaire." << std::endl;

        if (actualDamage > 0)
        {
            player.takeDamage(actualDamage);
            std::cout << "Tu repousses la menace, mais tu reçois " << actualDamage << " dégâts." << std::endl;
        }
        else
        {
            std::cout << "Tu repousses la menace de justesse, sans pouvoir encaisser davantage." << std::endl;
        }

        player.gainExperience(experience);
        player.recordCombatStarted();
        player.recordVictory();
        player.recordEnemyKills(1);
        std::cout << "XP gagnée : " << experience << std::endl;

        if (random.between(1, 100) <= 70)
        {
            addExplorationMaterial(player, biome.commonMaterialId, applyExplorationQuantityBonus(1, intensity), quality);
        }

        int updated = player.getQuestLog().progressCombatQuestsByFamily(1, "Créatures locales");
        if (updated > 0)
        {
            std::cout << "Une quête de combat progresse grâce à cette menace imprévue." << std::endl;
        }
    }

    // EN: openExplorationChest declares or implements a focused behavior used by this module.
    // FR: openExplorationChest déclare ou implémente un comportement précis utilisé par ce module.
    void openExplorationChest(Player& player, Random& random, const ExplorationBiome& biome, const ExplorationIntensity& intensity)
    {
        std::cout << "Un coffre est posé là, presque trop calmement." << std::endl;
        std::cout << "1 : Ouvrir le coffre" << std::endl;
        std::cout << "0 : Le laisser tranquille" << std::endl;
        std::cout << "> ";

        int choice = Console::askNumberBetween(0, 1, "Choix invalide.");
        std::cout << std::endl;

        if (choice == 0)
        {
            std::cout << "Tu décides que survivre vaut parfois mieux que satisfaire ta curiosité." << std::endl;
            return;
        }

        int roll = random.between(1, 100);

        if (roll <= 16)
        {
            int damage = std::min(random.between(5, 18 + player.getLevel()), std::max(0, player.getHp() - 1));
            std::cout << "Un mécanisme claque. Piège." << std::endl;
            if (damage > 0)
            {
                player.takeDamage(damage);
                std::cout << "Tu prends " << damage << " dégâts, mais tu restes debout." << std::endl;
            }
        }
        else if (roll <= 28)
        {
            std::cout << "Le coffre se déplie d'un coup. Ce n'était pas un coffre. Mimic." << std::endl;
            simulateUnexpectedExplorationFight(player, random, biome, intensity);
            player.getInventory().earnGold(random.between(8, 24 + player.getLevel() * 2));
            std::cout << "Dans les restes visqueux, tu récupères quelques pièces." << std::endl;
        }
        else if (roll <= 42)
        {
            std::cout << "Le coffre est vide. Quelqu'un a déjà eu l'idée avant toi." << std::endl;
        }
        else if (roll <= 76)
        {
            int gold = applyExplorationGoldBonus(random.between(8, 30 + player.getLevel() * 3), intensity);
            std::cout << "Vrai coffre, petit contenu." << std::endl;
            player.getInventory().earnGold(gold);
            std::cout << "Or gagné : " << gold << std::endl;
            addExplorationMaterial(player, biome.commonMaterialId, applyExplorationQuantityBonus(1, intensity), chooseExplorationQuality(random, true));
        }
        else
        {
            int gold = applyExplorationGoldBonus(random.between(35 + player.getLevel() * 3, 90 + player.getLevel() * 8), intensity);
            std::cout << "Vrai coffre, et pour une fois il n'a pas décidé de te mordre." << std::endl;
            player.getInventory().earnGold(gold);
            std::cout << "Or gagné : " << gold << std::endl;
            addExplorationMaterial(player, biome.rareMaterialId, applyExplorationQuantityBonus(1, intensity), chooseExplorationQuality(random, true));
        }
    }

    // EN: triggerRareExplorationDiscovery declares or implements a focused behavior used by this module.
    // FR: triggerRareExplorationDiscovery déclare ou implémente un comportement précis utilisé par ce module.
    void triggerRareExplorationDiscovery(Player& player, Random& random, const ExplorationBiome& biome, const ExplorationIntensity& intensity)
    {
        int roll = random.between(1, 100);

        if (roll <= 22)
        {
            std::cout << "Découverte rare : un filon / bouquet intact n'a pas encore été touché par la zone." << std::endl;
            std::cout << "Tu prends le temps de récupérer proprement ce qui peut l'être." << std::endl;
            addExplorationMaterial(player, biome.rareMaterialId, applyExplorationQuantityBonus(random.between(1, 2), intensity), chooseExplorationQuality(random, true));
            addExplorationMaterial(player, biome.commonMaterialId, applyExplorationQuantityBonus(1, intensity), chooseExplorationQuality(random, true));
            progressExplorationQuests(player, biome.name, 2);
            return;
        }

        if (roll <= 40)
        {
            int gold = applyExplorationGoldBonus(random.between(45 + player.getLevel() * 4, 120 + player.getLevel() * 9), intensity);
            std::cout << "Découverte rare : une cache ancienne est dissimulée sous des marques presque effacées." << std::endl;
            std::cout << "Ce n'est pas un trésor de roi, mais ce n'est clairement pas une trouvaille normale." << std::endl;
            player.getInventory().earnGold(gold);
            std::cout << "Or gagné : " << gold << std::endl;
            addExplorationMaterial(player, biome.rareMaterialId, applyExplorationQuantityBonus(1, intensity), chooseExplorationQuality(random, true));
            progressExplorationQuests(player, biome.name, 1);
            return;
        }

        if (roll <= 58)
        {
            std::cout << "Découverte rare : tu trouves des traces parfaitement conservées." << std::endl;
            std::cout << "Elles ne donnent pas un objet immédiat, mais elles valent beaucoup pour les quêtes et le registre." << std::endl;
            int updated = progressExplorationQuests(player, biome.name, 3);
            if (updated > 0)
            {
                std::cout << "Plusieurs notes de quête progressent grâce à ces traces." << std::endl;
            }
            else
            {
                std::cout << "Tu notes mentalement le lieu : une future demande pourrait clairement s'y rattacher." << std::endl;
            }
            return;
        }

        if (roll <= 76)
        {
            std::cout << "Découverte rare : une petite anomalie de matériaux pulse au sol." << std::endl;
            std::cout << "Tu n'en comprends pas tout, mais tu arrives à détacher un résidu stable." << std::endl;
            addExplorationMaterial(player, "variation_residue", applyExplorationQuantityBonus(1, intensity), chooseExplorationQuality(random, true));
            progressExplorationQuests(player, biome.name, 2);
            return;
        }

        if (roll <= 90)
        {
            std::cout << "Découverte rare : le silence se fissure autour de toi." << std::endl;
            std::cout << "Ce n'est pas une entrée de boss complète, mais le registre réagit comme s'il venait de sentir une respiration." << std::endl;
            std::cout << "Trace perçue : " << bossTraceForBiome(biome) << "." << std::endl;

            bool newEntityDetected = player.unlockNextBossVariation();
            if (newEntityDetected)
            {
                std::cout << "Une nouvelle entité a été détectée dans le registre des variations d'énergie anormale." << std::endl;
                std::cout << "Nom : ???" << std::endl;
                std::cout << "Statut : repérée par exploration rare." << std::endl;
            }
            else
            {
                std::cout << "Le registre garde la trace, mais aucune nouvelle entrée ne se stabilise." << std::endl;
            }
            progressExplorationQuests(player, biome.name, 2);
            return;
        }

        std::cout << "Découverte rarissime : quelque chose t'a vu avant que tu ne le voies." << std::endl;
        std::cout << "Ton instinct refuse de rester. Tu récupères ce que tu peux avant de reculer." << std::endl;
        int damage = std::min(random.between(6 + player.getLevel(), 18 + player.getLevel() * 2), std::max(0, player.getHp() - 1));
        if (damage > 0)
        {
            player.takeDamage(damage);
            std::cout << "La pression te laisse " << damage << " dégâts, sans t'achever." << std::endl;
        }
        addExplorationMaterial(player, biome.rareMaterialId, applyExplorationQuantityBonus(1, intensity), "exceptional");
        progressExplorationQuests(player, biome.name, 3);
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
            intro = client + " te confie une demande liée à " + biomeName + ". Cette fois, ce n'est pas une quête générique copiée sur le panneau.";
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
        std::cout << "Récompenses : XP +" << offeredQuest.rewardExperience << " | Or +" << offeredQuest.rewardGold << std::endl;

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
    void simulateExplorationMiniBoss(Player& player, Random& random, const ExplorationBiome& biome, const ExplorationIntensity& intensity)
    {
        bool evolved = random.between(1, 100) <= 45;
        int damage = random.between(10 + player.getLevel() * 2, 24 + player.getLevel() * 4);
        int actualDamage = std::min(damage, std::max(0, player.getHp() - 1));
        int experience = 45 + player.getLevel() * 12 + (evolved ? 25 : 0);
        int gold = applyExplorationGoldBonus(random.between(15 + player.getLevel() * 3, 45 + player.getLevel() * 6) + (evolved ? 25 : 0), intensity);

        std::string miniBossName = miniBossNameForBiome(biome, evolved);
        std::string questFamily = miniBossQuestFamilyForBiome(biome, evolved);

        std::cout << "L'air se tasse autour de toi." << std::endl;
        std::cout << "Mini-boss d'exploration : " << miniBossName << "." << std::endl;
        if (evolved)
        {
            std::cout << "Cette chose ressemble à une version évoluée d'un monstre local." << std::endl;
        }

        if (actualDamage > 0)
        {
            player.takeDamage(actualDamage);
            std::cout << "Tu finis par la repousser, mais tu reçois " << actualDamage << " dégâts." << std::endl;
        }
        else
        {
            std::cout << "Tu remportes l'échange de justesse, déjà trop proche de tomber." << std::endl;
        }

        player.gainExperience(experience);
        player.getInventory().earnGold(gold);
        player.recordCombatStarted();
        player.recordVictory();
        player.recordEnemyKills(1);
        std::cout << "XP gagnée : " << experience << std::endl;
        std::cout << "Or récupéré : " << gold << std::endl;

        addExplorationMaterial(player, evolved ? biome.rareMaterialId : biome.commonMaterialId, applyExplorationQuantityBonus(1, intensity), chooseExplorationQuality(random, evolved));

        int updated = player.getQuestLog().progressCombatQuestsByFamily(evolved ? 2 : 1, questFamily);
        if (updated > 0)
        {
            std::cout << "Des quêtes de combat progressent grâce à cette rencontre." << std::endl;
        }
    }

    // EN: simulateAfterCombatMiniBoss declares or implements a focused behavior used by this module.
    // FR: simulateAfterCombatMiniBoss déclare ou implémente un comportement précis utilisé par ce module.
    void simulateAfterCombatMiniBoss(Player& player, Random& random)
    {
        bool evolved = random.between(1, 100) <= 35;
        std::string miniBossName = evolved ? "forme évoluée attirée par le sang" : "menace opportuniste";
        std::string questFamily = evolved ? "Mini-boss / menace évoluée" : "Élite / menace";
        int damage = random.between(8 + player.getLevel() * 2, 20 + player.getLevel() * 3);
        int actualDamage = std::min(damage, std::max(0, player.getHp() - 1));
        int experience = 35 + player.getLevel() * 10 + (evolved ? 35 : 0);
        int gold = random.between(10 + player.getLevel() * 2, 35 + player.getLevel() * 5) + (evolved ? 30 : 0);

        std::cout << "========== ÉVÉNEMENT APRÈS-COMBAT ==========" << std::endl;
        std::cout << "Tu pensais pouvoir souffler, mais quelque chose a suivi le bruit du combat." << std::endl;
        std::cout << "Mini-boss détecté : " << miniBossName << "." << std::endl;
        std::cout << std::endl;

        if (actualDamage > 0)
        {
            player.takeDamage(actualDamage);
            std::cout << "Tu repousses l'apparition, mais elle te laisse " << actualDamage << " dégâts." << std::endl;
        }
        else
        {
            std::cout << "Tu repousses l'apparition sans pouvoir vraiment encaisser davantage." << std::endl;
        }

        player.recordCombatStarted();
        player.recordVictory();
        player.recordEnemyKills(1);
        player.gainExperience(experience);
        player.getInventory().earnGold(gold);

        std::cout << "XP gagnée : " << experience << std::endl;
        std::cout << "Or récupéré : " << gold << std::endl;

        int updated = player.getQuestLog().progressCombatQuestsByFamily(evolved ? 2 : 1, questFamily);
        if (updated > 0)
        {
            std::cout << "Des quêtes de combat progressent grâce à ce mini-boss." << std::endl;
        }
    }

    // EN: openDangerousExplorationSite declares or implements a focused behavior used by this module.
    // FR: openDangerousExplorationSite déclare ou implémente un comportement précis utilisé par ce module.
    void openDangerousExplorationSite(Player& player, Random& random, const ExplorationBiome& biome, const ExplorationIntensity& intensity)
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

            int damage = random.between(18 + player.getLevel() * 3, 42 + player.getLevel() * 5);
            int actualDamage = std::min(damage, std::max(0, player.getHp() - 1));
            int experience = 80 + player.getLevel() * 18;
            int gold = applyExplorationGoldBonus(random.between(30 + player.getLevel() * 5, 90 + player.getLevel() * 8), intensity);

            player.takeDamage(actualDamage);
            player.recordCombatStarted();
            player.recordVictory();
            player.recordEnemyKills(random.between(2, 4));
            player.gainExperience(experience);
            player.getInventory().earnGold(gold);

            std::cout << "Tu survis à la vague, mais tu encaisses " << actualDamage << " dégâts." << std::endl;
            std::cout << "XP gagnée : " << experience << std::endl;
            std::cout << "Or récupéré : " << gold << std::endl;
            addExplorationMaterial(player, biome.rareMaterialId, applyExplorationQuantityBonus(1, intensity), chooseExplorationQuality(random, true));
            player.getQuestLog().progressCombatQuestsByFamily(3, "Menace avancée");
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
        std::cout << "Le vrai combat de boss devra être lancé depuis le registre des Boss, mais cette découverte l'a réveillé." << std::endl;

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

}

// EN: openQuestHub declares or implements a focused behavior used by this module.
// FR: openQuestHub déclare ou implémente un comportement précis utilisé par ce module.
void QuestMenu::openQuestHub(Player& player)
{
    while (true)
    {
        std::cout << "========== QUÊTES ==========" << std::endl;
        std::cout << "0 : Retour" << std::endl;
        std::cout << "1 : Consulter le journal de quêtes" << std::endl;
        std::cout << "2 : Aller à la guilde" << std::endl;
        std::cout << "3 : Faire avancer une quête active" << std::endl;
        std::cout << "============================" << std::endl;
        std::cout << "> ";

        int choice = Console::askNumberBetween(0, 3, "Choix invalide.");
        Console::clear();

        if (choice == 0)
        {
            return;
        }

        if (choice == 1)
        {
            displayQuestJournal(player);
            Console::waitForEnter();
            Console::clear();
        }
        else if (choice == 2)
        {
            openGuild(player);
        }
        else if (choice == 3)
        {
            simulateQuestProgress(player);
        }
    }
}

// EN: consultOnly declares or implements a focused behavior used by this module.
// FR: consultOnly déclare ou implémente un comportement précis utilisé par ce module.
void QuestMenu::consultOnly(const Player& player)
{
    displayQuestJournal(player);
    std::cout << "Note : depuis ce menu, tu peux seulement consulter." << std::endl;
    std::cout << "Pour accepter ou valider une quête, retourne voir la guilde ou le client." << std::endl;
    std::cout << std::endl;
    Console::waitForEnter();
    Console::clear();
}

// EN: displayQuestJournal declares or implements a focused behavior used by this module.
// FR: displayQuestJournal déclare ou implémente un comportement précis utilisé par ce module.
void QuestMenu::displayQuestJournal(const Player& player)
{
    const std::vector<Quest>& quests = player.getQuestLog().getQuests();

    std::cout << "========== JOURNAL DE QUÊTES ==========" << std::endl;

    if (quests.empty())
    {
        std::cout << "Aucune quête acceptée pour l'instant." << std::endl;
        std::cout << "La guilde propose des quêtes de farm, et certains PNJ peuvent aussi te demander un service." << std::endl;
        std::cout << "=======================================" << std::endl;
        std::cout << std::endl;
        return;
    }

    std::cout << "Quêtes de guilde actives : " << player.getQuestLog().getActiveGuildQuestCount() << "/3" << std::endl;
    std::cout << std::endl;

    int displayIndex = 1;
    std::cout << "--- Quêtes de guilde ---" << std::endl;

    bool hasGuildQuest = false;

    for (const Quest& quest : quests)
    {
        if (quest.guildQuest && !quest.turnedIn)
        {
            hasGuildQuest = true;
            displayQuestLine(quest, displayIndex++);
            std::cout << std::endl;
        }
    }

    if (!hasGuildQuest)
    {
        std::cout << "Aucune quête de guilde active." << std::endl;
        std::cout << std::endl;
    }

    std::cout << "--- Quêtes personnelles / événements / clients ---" << std::endl;

    bool hasPersonalQuest = false;

    for (const Quest& quest : quests)
    {
        if (!quest.guildQuest && !quest.turnedIn)
        {
            hasPersonalQuest = true;
            displayQuestLine(quest, displayIndex++);
            std::cout << std::endl;
        }
    }

    if (!hasPersonalQuest)
    {
        std::cout << "Aucune demande personnelle active." << std::endl;
        std::cout << std::endl;
    }

    std::cout << "--- Quêtes terminées / rendues ---" << std::endl;

    bool hasTurnedInQuest = false;

    for (const Quest& quest : quests)
    {
        if (quest.turnedIn)
        {
            hasTurnedInQuest = true;
            displayQuestLine(quest, displayIndex++);
            std::cout << std::endl;
        }
    }

    if (!hasTurnedInQuest)
    {
        std::cout << "Aucune quête rendue pour l'instant." << std::endl;
        std::cout << std::endl;
    }

    std::cout << "=======================================" << std::endl;
    std::cout << std::endl;
}

// EN: openGuild declares or implements a focused behavior used by this module.
// FR: openGuild déclare ou implémente un comportement précis utilisé par ce module.
void QuestMenu::openGuild(Player& player)
{
    while (true)
    {
        std::cout << "========== GUILDE ==========" << std::endl;
        std::cout << "La guilde centralise les quêtes officielles." << std::endl;
        std::cout << "Tu peux avoir jusqu'à 3 quêtes de guilde actives." << std::endl;
        std::cout << std::endl;
        std::cout << "0 : Retour" << std::endl;
        std::cout << "1 : Voir le panneau de quêtes" << std::endl;
        std::cout << "2 : Rendre une quête de guilde terminée" << std::endl;
        std::cout << "3 : Consulter le journal" << std::endl;
        std::cout << "============================" << std::endl;
        std::cout << "> ";

        int choice = Console::askNumberBetween(0, 3, "Choix invalide.");
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
            Console::waitForEnter();
            Console::clear();
        }
    }
}

// EN: acceptGuildQuest declares or implements a focused behavior used by this module.
// FR: acceptGuildQuest déclare ou implémente un comportement précis utilisé par ce module.
void QuestMenu::acceptGuildQuest(Player& player)
{
    std::vector<Quest> board = QuestCatalog::createGuildBoard(player.getLevel());

    std::cout << "========== PANNEAU DE GUILDE ==========" << std::endl;
    std::cout << "Quêtes actives : " << player.getQuestLog().getActiveGuildQuestCount() << "/3" << std::endl;
    std::cout << "0 : Retour" << std::endl;

    for (int i = 0; i < static_cast<int>(board.size()); ++i)
    {
        std::cout << i + 1 << " : [Rang " << board[i].rank << "] " << board[i].title
                  << " | XP +" << board[i].rewardExperience
                  << " | Or +" << board[i].rewardGold;

        if (player.getQuestLog().hasQuest(board[i].id))
        {
            std::cout << " | déjà prise";
        }

        std::cout << std::endl;
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

    if (!player.getQuestLog().canAcceptGuildQuest())
    {
        std::cout << "Tu as déjà 3 quêtes de guilde actives." << std::endl;
        std::cout << "Termine ou rends-en une avant d'en accepter une autre." << std::endl;
    }
    else if (player.getQuestLog().addQuest(selectedQuest))
    {
        std::cout << "Quête acceptée : " << selectedQuest.title << std::endl;
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
void QuestMenu::openExploration(Player& player)
{
    openExplorationMenu(player);
}

// EN: openLocations declares or implements a focused behavior used by this module.
// FR: openLocations déclare ou implémente un comportement précis utilisé par ce module.
void QuestMenu::openLocations(Player& player)
{
    while (true)
    {
        std::cout << "========== LIEUX VISITABLES ==========" << std::endl;
        std::cout << "0 : Retour" << std::endl;
        std::cout << "1 : Guilde" << std::endl;
        std::cout << "2 : Forge" << std::endl;
        std::cout << "3 : Herboristerie" << std::endl;
        std::cout << "4 : Place du village" << std::endl;
        std::cout << "5 : Route commerciale" << std::endl;
        std::cout << "6 : Boutique de monstres" << std::endl;
        std::cout << "7 : Boutique de matériaux" << std::endl;
        std::cout << "8 : Armurerie défensive" << std::endl;
        std::cout << "9 : Forge d'armes" << std::endl;
        std::cout << "10 : Boutique de consommables" << std::endl;
        std::cout << "11 : Bibliothèque" << std::endl;
        std::cout << "======================================" << std::endl;
        std::cout << "> ";

        int choice = Console::askNumberBetween(0, 11, "Choix invalide.");
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
    }
}

// EN: openNotableNpcMenu declares or implements a focused behavior used by this module.
// FR: openNotableNpcMenu déclare ou implémente un comportement précis utilisé par ce module.
void QuestMenu::openNotableNpcMenu(Player& player)
{
    while (true)
    {
        std::cout << "========== PNJ NOTABLES ==========" << std::endl;
        std::cout << "0 : Retour" << std::endl;
        std::cout << "1 : Maître de guilde (PNJ important)" << std::endl;
        std::cout << "2 : Forgeron (PNJ client)" << std::endl;
        std::cout << "3 : Alchimiste (PNJ client)" << std::endl;
        std::cout << "4 : Villageois nerveux (PNJ client / événement)" << std::endl;
        std::cout << "5 : Marchand inquiet (PNJ client)" << std::endl;
        std::cout << "6 : Vendeur de composants (PNJ client)" << std::endl;
        std::cout << "7 : Vendeur de matériaux (PNJ client)" << std::endl;
        std::cout << "8 : Herboriste (PNJ client)" << std::endl;
        std::cout << "9 : Armurier (PNJ client)" << std::endl;
        std::cout << "10 : Vendeur d'armes (PNJ client)" << std::endl;
        std::cout << "11 : Vendeur de consommables (PNJ client)" << std::endl;
        std::cout << "12 : Bibliothécaire (PNJ client)" << std::endl;
        std::cout << "==================================" << std::endl;
        std::cout << "> ";

        int choice = Console::askNumberBetween(0, 12, "Choix invalide.");
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
            talkToClient(player, "Herboriste");
        }
        else if (choice == 9)
        {
            talkToClient(player, "Armurier");
        }
        else if (choice == 10)
        {
            talkToClient(player, "Vendeur d'armes");
        }
        else if (choice == 11)
        {
            talkToClient(player, "Vendeur de consommables");
        }
        else if (choice == 12)
        {
            talkToClient(player, "Bibliothécaire");
        }
    }
}

// EN: talkToClient declares or implements a focused behavior used by this module.
// FR: talkToClient déclare ou implémente un comportement précis utilisé par ce module.
void QuestMenu::talkToClient(Player& player, const std::string& clientName)
{
    while (true)
    {
        std::cout << "========== " << clientName << " ==========" << std::endl;
        std::cout << "0 : Retour" << std::endl;
        std::cout << "1 : Parler" << std::endl;
        std::cout << "2 : Voir / rendre une demande terminée" << std::endl;
        std::cout << "================================" << std::endl;
        std::cout << "> ";

        int choice = Console::askNumberBetween(0, 2, "Choix invalide.");
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

        Quest offeredQuest;
        Random questRandom;

        if (questRandom.between(1, 100) <= 70)
        {
            std::string targetedBiome = randomBiomeForClient(questRandom, clientName);
            std::cout << clientName << " ne sort pas une demande générique de son comptoir." << std::endl;
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
        std::cout << "Récompenses : XP +" << offeredQuest.rewardExperience << " | Or +" << offeredQuest.rewardGold << std::endl;

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
                  << " | XP +" << quest.rewardExperience
                  << " | Or +" << quest.rewardGold;

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

    std::cout << "Quête validée : " << quest.title << std::endl;
    std::cout << "XP gagnée : " << quest.rewardExperience << std::endl;
    std::cout << "Or gagné : " << quest.rewardGold << " pièces" << std::endl;
    std::cout << std::endl;
    Console::waitForEnter();
    Console::clear();
}

// EN: maybeOfferRandomInterception declares or implements a focused behavior used by this module.
// FR: maybeOfferRandomInterception déclare ou implémente un comportement précis utilisé par ce module.
void QuestMenu::maybeOfferRandomInterception(Player& player)
{
    Random random;

    if (random.between(1, 100) > 22)
    {
        return;
    }

    if (random.between(1, 100) <= 25)
    {
        simulateAfterCombatMiniBoss(player, random);
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
void QuestMenu::openExplorationMenu(Player& player)
{
    std::vector<ExplorationBiome> biomes = {
        {"Forêt ancienne", "biome végétal, humide, propice aux plantes et aux bêtes discrètes", "bitter_healing_leaf", "mountain_blue_flower"},
        {"Montagne froide", "biome rocheux, dur, avec minerais, fleurs rares et vents coupants", "rusted_metal_fragment", "mountain_blue_flower"},
        {"Marais trouble", "biome sale, collant, parfait pour les résidus étranges et les mauvaises surprises", "slime_residue", "arcane_dust"},
        {"Route commerciale", "biome de passage, traces de voyageurs, caisses perdues et arnaques au sol", "battle_torn_badge", "worn_leather_piece"},
        {"Ruines effondrées", "biome ancien, instable, avec os, poussière arcanique et coffres suspects", "cracked_bone", "arcane_dust"},
        {"Plaine sauvage", "biome ouvert, plus calme, mais jamais totalement sûr", "worn_leather_piece", "wolf_fang"}
    };

    std::vector<ExplorationIntensity> intensities = {
        {"Sortie prudente", "moins de danger, mais moins de trouvailles importantes", -10, 0, 80, 3},
        {"Sortie normale", "équilibre entre découverte, gain et risque", 0, 0, 100, 0},
        {"Sortie audacieuse", "plus de danger, plus de chances de coffres, mini-boss, lieux rares et meilleurs gains", 12, 1, 125, -2}
    };

    while (true)
    {
        std::cout << "========== EXPLORATION ==========" << std::endl;
        std::cout << "Choisis le style de biome à explorer." << std::endl;
        std::cout << "Exploration = recherche passive : plantes, matériaux, traces, trésors, coffres ou événements." << std::endl;
        std::cout << "Tu ne lances pas volontairement un combat, mais un combat inattendu peut arriver." << std::endl;
        std::cout << std::endl;
        std::cout << "0 : Retour" << std::endl;

        for (int i = 0; i < static_cast<int>(biomes.size()); ++i)
        {
            std::cout << i + 1 << " : " << biomes[i].name << " — " << biomes[i].style << std::endl;
        }

        std::cout << "=================================" << std::endl;
        std::cout << "> ";

        int choice = Console::askNumberBetween(0, static_cast<int>(biomes.size()), "Choix invalide.");
        Console::clear();

        if (choice == 0)
        {
            return;
        }

        const ExplorationBiome& biome = biomes[choice - 1];

        std::cout << "========== INTENSITÉ ==========" << std::endl;
        std::cout << "Choisis comment tu veux explorer " << biome.name << "." << std::endl;
        std::cout << "0 : Retour aux biomes" << std::endl;

        for (int i = 0; i < static_cast<int>(intensities.size()); ++i)
        {
            std::cout << i + 1 << " : " << intensities[i].name << " — " << intensities[i].description << std::endl;
        }

        std::cout << "===============================" << std::endl;
        std::cout << "> ";

        int intensityChoice = Console::askNumberBetween(0, static_cast<int>(intensities.size()), "Choix invalide.");
        Console::clear();

        if (intensityChoice == 0)
        {
            continue;
        }

        const ExplorationIntensity& intensity = intensities[intensityChoice - 1];
        Random random;
        int roll = adjustExplorationEventRoll(random.between(1, 100), intensity);
        bool carefulRecovery = chooseCarefulRecovery(random, intensity);

        std::cout << "========== " << biome.name << " ==========" << std::endl;
        std::cout << "Style : " << biome.style << "." << std::endl;
        std::cout << "Approche : " << intensity.name << "." << std::endl;
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
            int gold = applyExplorationGoldBonus(random.between(5, 22 + player.getLevel() * 2), intensity);
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
            openExplorationChest(player, random, biome, intensity);
        }
        else if (roll <= 78)
        {
            simulateUnexpectedExplorationFight(player, random, biome, intensity);
        }
        else if (roll <= 87)
        {
            simulateExplorationMiniBoss(player, random, biome, intensity);
        }
        else if (roll <= 94)
        {
            offerExplorationNpcQuest(player, random, biome);
        }
        else if (roll <= 97)
        {
            openDangerousExplorationSite(player, random, biome, intensity);
        }
        else
        {
            triggerRareExplorationDiscovery(player, random, biome, intensity);
        }

        player.getQuestLog().refreshMaterialDeliveryQuests(player.getInventory());
        std::cout << std::endl;
        Console::waitForEnter();
        Console::clear();
    }
}

// EN: simulateQuestProgress declares or implements a focused behavior used by this module.
// FR: simulateQuestProgress déclare ou implémente un comportement précis utilisé par ce module.
void QuestMenu::simulateQuestProgress(Player& player)
{
    std::vector<Quest>& quests = player.getQuestLog().getQuests();
    std::vector<int> activeIndexes;

    for (int i = 0; i < static_cast<int>(quests.size()); ++i)
    {
        if (quests[i].accepted && !quests[i].completed && !quests[i].turnedIn
            // EN: quests[i].requiredMaterialId.empty declares or implements a focused behavior used by this module.
            // FR: quests[i].requiredMaterialId.empty déclare ou implémente un comportement précis utilisé par ce module.
            && quests[i].requiredMaterialId.empty())
        {
            activeIndexes.push_back(i);
        }
    }

    if (activeIndexes.empty())
    {
        std::cout << "Aucune quête active à faire avancer." << std::endl;
        std::cout << "Accepte une quête à la guilde ou auprès d'un PNJ client." << std::endl;
        std::cout << std::endl;
        Console::waitForEnter();
        Console::clear();
        return;
    }

    std::cout << "========== FAIRE AVANCER UNE QUÊTE ==========" << std::endl;
    std::cout << "Cette option simule une sortie courte en attendant les vraies zones." << std::endl;
    std::cout << "Les quêtes de livraison ne sont pas listées ici : elles se rendent avec les ressources demandées." << std::endl;
    std::cout << "0 : Retour" << std::endl;

    for (int i = 0; i < static_cast<int>(activeIndexes.size()); ++i)
    {
        const Quest& quest = quests[activeIndexes[i]];
        std::cout << i + 1 << " : [Rang " << quest.rank << "] " << quest.title
                  << " | " << (quest.targetFamily.empty() ? "Objectif" : quest.targetFamily)
                  << " | " << quest.progress << "/" << quest.target
                  << std::endl;
    }

    std::cout << "=============================================" << std::endl;
    std::cout << "> ";

    int choice = Console::askNumberBetween(0, static_cast<int>(activeIndexes.size()), "Choix invalide.");
    Console::clear();

    if (choice == 0)
    {
        return;
    }

    Quest& quest = quests[activeIndexes[choice - 1]];
    Random random;
    int roll = random.rollD20();
    int progressGain = 1;

    if (roll >= 18)
    {
        progressGain = 3;
    }
    else if (roll >= 14)
    {
        progressGain = 2;
    }

    quest.progress += progressGain;

    if (quest.progress >= quest.target)
    {
        quest.progress = quest.target;
        quest.completed = true;
    }

    std::cout << "Sortie liée à : " << quest.title << std::endl;
    std::cout << "Cible : " << (quest.targetFamily.empty() ? "objectif général" : quest.targetFamily) << std::endl;
    std::cout << "Jet d'exécution : " << roll << "/20" << std::endl;
    std::cout << "Progression gagnée : +" << progressGain << std::endl;

    if (quest.completed)
    {
        std::cout << "Objectif terminé. Retourne voir le client : " << quest.client << "." << std::endl;
    }
    else
    {
        std::cout << "Progression actuelle : " << quest.progress << "/" << quest.target << std::endl;
    }

    std::cout << std::endl;
    Console::waitForEnter();
    Console::clear();
}
