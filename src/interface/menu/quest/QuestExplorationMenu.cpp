// EN: QuestExplorationMenu.cpp owns the exploration runtime extracted from QuestMenu.
// FR: QuestExplorationMenu.cpp porte le moteur d'exploration extrait de QuestMenu.

#include "interface/menu/quest/QuestMenu.hpp"
#include "interface/menu/quest/QuestExplorationSupport.hpp"
#include "interface/menu/quest/QuestMenuInternalSupport.hpp"
#include "interface/menu/quest/QuestDeadlineSupport.hpp"
#include "interface/menu/quest/QuestClientNavigationSupport.hpp"
#include "interface/menu/quest/QuestStorySupport.hpp"
#include "interface/menu/quest/QuestPresentationSupport.hpp"

#include "adventure/flavor/ExplorationBiomeFlavor.hpp"
#include "adventure/flavor/ExplorationLanguageTrace.hpp"
#include "adventure/content/BiomeLivingContentCatalog.hpp"
#include "adventure/content/BiomeAmbientEventSystem.hpp"
#include "adventure/content/BiomeNonCombatInteractionSystem.hpp"
#include "character/RaceCatalog.hpp"
#include "combat/modes/pve/MonsterPveMode.hpp"
#include "combat/system/ElementalAffinitySystem.hpp"
#include "core/Console.hpp"
#include "core/Random.hpp"
#include "economy/Money.hpp"
#include "economy/shop/ShopTransactionSystem.hpp"
#include "entity/MonsterCatalog.hpp"
#include "interface/TerminalInterface.hpp"
#include "interface/menu/common/MessageScreen.hpp"
#include "interface/model/MenuScreen.hpp"
#include "item/material/MaterialCatalog.hpp"
#include "progression/bestiary/BestiaryRuntimeProgress.hpp"
#include "quest/QuestCatalog.hpp"
#include "story/StoryCampaign.hpp"
#include "world/City.hpp"
#include "world/CityTravelRules.hpp"

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

using QuestDeadlineSupport::expireOverdueQuestDeadlines;
using QuestClientNavigationSupport::isReadyToTurnIn;
using QuestMenuInternalSupport::appendDeadlineLine;
using QuestMenuInternalSupport::askChoiceScreen;
using QuestMenuInternalSupport::askQuestOfferDecision;
using QuestMenuInternalSupport::clientQuestAcceptedDialogueLines;
using QuestMenuInternalSupport::prepareQuestForAcceptance;
using QuestMenuInternalSupport::runTrackedExplorationWave;
using QuestMenuInternalSupport::showExplorationNotice;
using QuestMenuInternalSupport::maybeTriggerLegendaryMerchantEncounter;
using QuestMenuInternalSupport::toLowerChoiceText;
using QuestExplorationSupport::MicroChallengeResult;

namespace
{
    using namespace QuestStorySupport;
    using namespace QuestPresentationSupport;

    int countReadyToTurnInQuests(const Player& player)
    {
        int count = 0;
        for (const Quest& quest : player.getQuestLog().getQuests())
        {
            if (isReadyToTurnIn(player, quest))
            {
                ++count;
            }
        }
        return count;
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

    int environmentalPassiveSynergyScore(const Player& player, const std::string& hazard, std::vector<std::string>& lines)
    {
        int compatiblePassives = 0;
        auto addPassive = [&](const std::string& passiveId) {
            if (playerHasExplorationPassive(player, passiveId))
            {
                ++compatiblePassives;
            }
        };

        addPassive("temperature_adaptation");
        addPassive("cautious_pathing");
        addPassive("threat_route_planner");

        if (hazard == "froid")
        {
            addPassive("minor_cold_resistance");
            addPassive("dragon_weather_blood");
            addPassive("orcish_forced_march");
        }
        else if (hazard == "chaleur" || hazard == "feu")
        {
            addPassive("minor_fire_resistance");
            addPassive("infernal_fire_resistance");
            addPassive("semi_lizard_scales");
            addPassive("dragon_weather_blood");
        }

        if (compatiblePassives >= 3)
        {
            lines.push_back("Synergie de passifs : " + std::to_string(compatiblePassives) + " habitudes de température/terrain se complètent. La préparation naturelle devient vraiment utile.");
            return 2;
        }
        if (compatiblePassives >= 2)
        {
            lines.push_back("Synergie de passifs : deux habitudes compatibles se répondent. Ce n'est pas une immunité, mais le biome se lit mieux.");
            return 1;
        }

        return 0;
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

        score += environmentalPassiveSynergyScore(player, hazard, lines);
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

    ExplorationBossUnlockResult tryUnlockExplorationBossVariation(Player& player, Random& random, bool dangerousSite, const std::string& discoveryLocation)
    {
        const std::size_t unlockedCount = player.getUnlockedBossIds().size();
        const std::size_t uniqueBossDefeats = player.getDefeatedBossIds().size();
        const int level = player.getLevel();

        if (level < 10)
        {
            return {false, "Trace trop faible : le registre ne stabilise encore aucun emplacement de boss fiable."};
        }

        if (!player.canUseRareBossDiscovery())
        {
            const int remainingDays = std::max(1, player.getRareBossDiscoveryCooldownExpiresAtDay() - player.getWorldDaysElapsed());
            return {false, "Les traces exceptionnelles se brouillent encore. Le registre estime qu'il lui faut environ "
                + std::to_string(remainingDays) + " jour(s) avant de pouvoir isoler un autre emplacement."};
        }

        // Exceptional exploration must never rush the near-final roster.
        if (unlockedCount >= 28)
        {
            return {false, "Trace verrouillée : les présences presque finales ne laissent aucun emplacement exploitable par simple exploration."};
        }

        if (unlockedCount >= 23 && (uniqueBossDefeats < 6 || level < 24))
        {
            return {false, "Trace trop haute : le personnage manque encore de victoires confirmées pour distinguer cette présence des faux témoignages."};
        }

        // This roll is intentionally extremely rare. It is evaluated only inside an already rare
        // exploration event, then followed by a thirty-day in-world cooldown after success.
        const int chance = dangerousSite ? 3 : 1;
        if (random.between(1, 100) > chance)
        {
            return {false, "Trace instable : un emplacement semble exister, mais les indices se contredisent avant que le registre puisse le conserver."};
        }

        const bool unlocked = player.unlockNextBossVariationFromRareDiscovery(discoveryLocation, 30);
        if (unlocked)
        {
            return {true, "Découverte exceptionnelle : le registre conserve l'emplacement approximatif d'une seule présence inconnue. Son identité reste brouillée."};
        }

        return {false, "Trace finale bloquée : FireFlight et les présences terminales ne peuvent pas être révélés par une simple piste d'exploration."};
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


    MicroChallengeResult runExplorationMicroChallenge(Player& player, const ExplorationBiome& biome, const ExplorationIntensity& intensity, Random& random)
    {
        (void)intensity;

        struct Challenge
        {
            std::string id;
            std::string title;
            std::string question;
            std::vector<std::pair<int, std::string>> options;
            int correctChoice = 1;
            std::string successLine;
            std::string failureLine;
            int cooldownDays = 2;
        };

        std::vector<Challenge> challenges = {
            {
                "generic_orientation",
                "ÉPREUVE D'ORIENTATION",
                "Tu dois choisir rapidement une méthode avant de t'enfoncer plus loin.",
                {{1, "Marquer un repère discret et écouter la zone"}, {2, "Courir vers le premier bruit"}, {3, "Jeter une pierre très loin pour voir"}},
                1,
                "Bonne approche : tu avances avec un vrai repère, pas juste au feeling.",
                "Mauvaise approche : tu avances quand même, mais ton repère est moins fiable."
            },
            {
                "generic_calculation_markers",
                "ÉPREUVE DE CALCUL",
                "Tu disposes de 15 balises à répartir sur 3 chemins égaux. Combien de balises par chemin ?",
                {{1, "4 balises"}, {2, "5 balises"}, {3, "6 balises"}},
                2,
                "Calcul propre : chaque chemin reçoit assez de repères.",
                "Erreur de calcul : tu corriges la répartition, mais tu perds un peu de temps."
            },
            {
                "generic_french_notes",
                "ÉPREUVE DE FRANÇAIS",
                "Quelle phrase est correcte pour ton carnet de terrain ?",
                {{1, "Les traces sont récentes."}, {2, "Les trace sont récente."}, {3, "Les traces est récentes."}},
                1,
                "Note claire : ton carnet reste lisible et exploitable pour la guilde.",
                "Note maladroite : tu comprends l'idée, mais le rapport sera moins utile."
            },
            {
                "generic_observation",
                "ÉPREUVE D'OBSERVATION",
                "Le vent efface les traces légères. Quel indice vérifier en premier ?",
                {{1, "Les marques profondes près des pierres"}, {2, "Une feuille qui bouge"}, {3, "Le nuage le plus proche"}},
                1,
                "Observation utile : tu conserves un indice que le vent ne peut pas effacer.",
                "Observation faible : tu reviens vers les pierres après avoir suivi un faux indice."
            },
            {
                "generic_supply_weight",
                "ÉPREUVE DE CHARGEMENT",
                "Quatre sacs pèsent 3 kg chacun. Quel poids total ajoutes-tu à ton équipement ?",
                {{1, "7 kg"}, {2, "12 kg"}, {3, "16 kg"}},
                2,
                "Charge calculée : tu répartis le poids avant qu'il ne devienne un problème.",
                "Charge mal estimée : une sangle te rappelle brutalement le vrai total."
            },
            {
                "generic_safe_water",
                "ÉPREUVE DE SURVIE",
                "Tu trouves une eau claire dans une zone inconnue. Quelle réaction est la plus sûre ?",
                {{1, "La boire immédiatement"}, {2, "La filtrer ou la faire bouillir avant"}, {3, "La mélanger avec une potion"}},
                2,
                "Prudence utile : l'eau devient une ressource au lieu d'un pari.",
                "Choix risqué : tu renonces juste avant de transformer une pause en maladie."
            },
            {
                "generic_signal_code",
                "ÉPREUVE DE SIGNAL",
                "Ton groupe utilise deux coups courts puis un long pour signaler un danger. Quel motif reproduire ?",
                {{1, "Court, court, long"}, {2, "Long, court, long"}, {3, "Trois coups longs"}},
                1,
                "Signal exact : la zone reçoit le message sans attirer tout ce qui vit autour.",
                "Signal confus : personne ne sait si tu annonces un danger ou le repas."
            },
            {
                "generic_distance_pace",
                "ÉPREUVE D'ALLURE",
                "Tu parcours 2 km par heure pendant 3 heures. Quelle distance as-tu couverte ?",
                {{1, "5 km"}, {2, "6 km"}, {3, "8 km"}},
                2,
                "Allure maîtrisée : ton estimation correspond enfin au terrain.",
                "Estimation fausse : la carte te semble soudain plus longue que prévu."
            },
            {
                "generic_footprint_order",
                "ÉPREUVE DE DÉDUCTION",
                "Une empreinte nette recouvre une empreinte effacée. Laquelle est la plus récente ?",
                {{1, "L'empreinte nette au-dessus"}, {2, "L'empreinte effacée dessous"}, {3, "Impossible à savoir"}},
                1,
                "Déduction propre : tu lis l'ordre des passages sans inventer une histoire.",
                "Déduction fragile : tu reprends les couches une par une avant de continuer."
            },
            {
                "generic_inventory_count",
                "ÉPREUVE D'INVENTAIRE",
                "Tu avais 9 torches et en utilises 2. Combien t'en reste-t-il ?",
                {{1, "6"}, {2, "7"}, {3, "11"}},
                2,
                "Inventaire juste : tu sais exactement combien de nuits tu peux encore éclairer.",
                "Inventaire faux : tu recompte avant que l'obscurité ne fasse le calcul pour toi."
            }
        };

        auto addBiomeChallenge = [&](const Challenge& challenge) {
            challenges.push_back(challenge);
        };

        if (biome.name == "Plaine sauvage")
        {
            addBiomeChallenge({"plain_grass_direction", "ÉPREUVE DE PLAINE", "Les herbes sont couchées vers l'est sur une large bande. Quelle hypothèse vérifier en premier ?", {{1, "Un passage récent venant de l'ouest"}, {2, "Une pluie verticale"}, {3, "Une pierre immobile"}}, 1, "Lecture juste : tu distingues un passage d'un simple mouvement du vent.", "Lecture trop rapide : tu observes plus longtemps avant de choisir une direction."});
            addBiomeChallenge({"plain_watch_rotation", "ÉPREUVE DE GARDE", "Trois aventuriers se relaient pendant 6 heures à parts égales. Combien de temps chacun surveille-t-il ?", {{1, "1 heure"}, {2, "2 heures"}, {3, "3 heures"}}, 2, "Relais propre : personne ne s'endort en prétendant que c'était son tour.", "Relais faux : la dispute dure presque aussi longtemps que la garde."});
        }
        else if (biome.name == "Route commerciale")
        {
            addBiomeChallenge({"road_wagon_tracks", "ÉPREUVE DE ROUTE", "Deux roues parallèles laissent quatre longues traces après deux chariots identiques. Combien de chariots sont passés ?", {{1, "Un"}, {2, "Deux"}, {3, "Quatre"}}, 2, "Comptage utile : tu sépares les véhicules des simples lignes dans la boue.", "Comptage hésitant : tu vérifies l'écartement avant de poursuivre."});
            addBiomeChallenge({"road_toll_change", "ÉPREUVE DE PÉAGE", "Un passage coûte 7 pièces et tu paies avec 10. Quelle monnaie doit revenir ?", {{1, "2 pièces"}, {2, "3 pièces"}, {3, "17 pièces"}}, 2, "Monnaie exacte : le péager comprend que tu comptes aussi vite que lui.", "Monnaie fausse : le sourire du péager devient beaucoup trop large."});
        }
        else if (biome.name == "Mares gélatineuses")
        {
            addBiomeChallenge({"slime_color_count", "ÉPREUVE GÉLATINEUSE", "Deux slimes bleus rejoignent trois slimes verts. Combien de slimes vois-tu au total ?", {{1, "4"}, {2, "5"}, {3, "6"}}, 2, "Comptage net : aucune masse colorée ne se cache dans ton total.", "Comptage faux : l'un des slimes se divise juste pour se moquer."});
            addBiomeChallenge({"slime_safe_step", "ÉPREUVE D'APPUI", "Une gelée brillante recouvre seulement le centre du passage. Où poses-tu le pied ?", {{1, "Au centre pour aller vite"}, {2, "Sur le bord sec et stable"}, {3, "Dans la flaque la plus profonde"}}, 2, "Appui sûr : tes bottes restent à toi.", "Appui mauvais : tu récupères ton pied avec un bruit humiliant."});
        }
        else if (biome.name == "Forêt ancienne")
        {
            addBiomeChallenge({"forest_moss", "ÉPREUVE DE SOUS-BOIS", "Quelle marque résiste le mieux sans blesser un arbre ancien ?", {{1, "Une entaille profonde"}, {2, "Un ruban récupérable sur une branche basse"}, {3, "Du feu sur l'écorce"}}, 2, "Repère respectueux : tu retrouves ta route sans transformer la forêt en ennemi.", "Repère agressif : tu renonces avant que les branches ne semblent se rapprocher."});
            addBiomeChallenge({"forest_canopy", "ÉPREUVE D'ÉCOUTE", "Des oiseaux cessent de chanter uniquement devant toi. Quelle réaction est la plus prudente ?", {{1, "Ralentir et observer la direction"}, {2, "Crier pour les faire revenir"}, {3, "Courir droit devant"}}, 1, "Silence compris : tu repères la zone qui inquiète la faune.", "Silence ignoré : une branche cassée te rappelle que la forêt prévenait."});
        }
        else if (biome.name == "Montagne froide")
        {
            addBiomeChallenge({"mountain_layers", "ÉPREUVE DE FROID", "Tu portes déjà deux couches et en ajoutes une. Combien de couches protègent maintenant ton torse ?", {{1, "2"}, {2, "3"}, {3, "4"}}, 2, "Préparation correcte : le froid reste un ennemi, pas une condamnation.", "Préparation confuse : tu recompte tes vêtements avant de perdre la sensation de tes doigts."});
            addBiomeChallenge({"mountain_avalanche", "ÉPREUVE DE CORNICHE", "La neige craque au-dessus de toi. Où cherches-tu d'abord un abri ?", {{1, "Sous une corniche rocheuse latérale"}, {2, "Au milieu de la pente"}, {3, "Sur la plaque qui craque"}}, 1, "Réflexe utile : tu quittes l'axe le plus exposé.", "Réflexe dangereux : tu changes de direction avant que la pente décide pour toi."});
        }
        else if (biome.name == "Marais trouble")
        {
            addBiomeChallenge({"swamp_bubbles", "ÉPREUVE DE MARAIS", "Des bulles remontent régulièrement devant toi. Quel geste est le plus sûr ?", {{1, "Tester le sol avec une perche depuis le bord"}, {2, "Sauter au centre"}, {3, "Allumer une torche au-dessus des bulles"}}, 1, "Test prudent : tu évites une poche profonde et peut-être inflammable.", "Geste risqué : l'odeur te convainc de ne pas terminer ton idée."});
            addBiomeChallenge({"swamp_board_count", "ÉPREUVE DE PASSERELLE", "Une passerelle demande 12 planches réparties sur 4 appuis. Combien par appui ?", {{1, "2"}, {2, "3"}, {3, "4"}}, 2, "Répartition propre : la passerelle ne choisit pas ton pied comme faiblesse.", "Répartition fausse : tu corriges avant de tester ton poids."});
        }
        else if (biome.name == "Ruines effondrées")
        {
            addBiomeChallenge({"ruins_arch", "ÉPREUVE DE RUINES", "Une arche fissurée perd de la poussière à chaque vibration. Quel passage privilégier ?", {{1, "Sous le centre de l'arche"}, {2, "Le détour dégagé le long du mur stable"}, {3, "Le sommet de l'arche"}}, 2, "Détour intelligent : les pierres restent au-dessus de toi au lieu de te rejoindre.", "Passage mauvais : une pluie de poussière suffit à te faire changer d'avis."});
            addBiomeChallenge({"ruins_symbols", "ÉPREUVE DE SYMBOLES", "Les plaques portent I, II, III puis V. Quel symbole manque ?", {{1, "IV"}, {2, "VI"}, {3, "X"}}, 1, "Suite comprise : le mécanisme accepte ton ordre.", "Suite ratée : une dalle grince jusqu'à ce que tu reprennes depuis le début."});
        }
        else if (biome.name == "Bocage aux lanternes")
        {
            addBiomeChallenge({"bocage_harvest", "ÉPREUVE DE RÉCOLTE", "Une lanterne de mycélium pulse doucement. Que fais-tu pour ne pas l'abîmer ?", {{1, "Couper la base d'un coup sec"}, {2, "Attendre que la lumière baisse puis détacher la terre autour"}, {3, "Souffler dessus pour l'éteindre"}}, 2, "Récolte intelligente : la lanterne reste presque intacte.", "Geste approximatif : une partie de sa lumière se perd."});
        }
        else if (biome.name == "Désert d'argile rouge")
        {
            addBiomeChallenge({"desert_tracks", "ÉPREUVE DE CALCUL SEC", "Tu notes 7 groupes de 6 traces. Combien de traces cela fait-il ?", {{1, "36"}, {2, "42"}, {3, "48"}}, 2, "Comptage propre : tu identifies la vraie piste.", "Comptage faux : tu repères l'erreur un peu trop tard."});
        }
        else if (biome.name == "Quartier abandonné")
        {
            addBiomeChallenge({"district_paperwork", "ÉPREUVE DE PAPERASSE", "Sur un formulaire de guilde, quelle formulation est la plus propre ?", {{1, "Les documents ont été remis."}, {2, "Les document on été remit."}, {3, "Les documents a été remis."}}, 1, "Formulaire propre : la guilde pourra l'exploiter.", "Formulaire sale : il reste compréhensible, mais pénible à relire."});
        }
        else if (biome.name == "Mine sifflante")
        {
            addBiomeChallenge({"mine_vibration", "ÉPREUVE DE LOGIQUE", "Un rail vibre toutes les 4 secondes. Entre la première et la cinquième vibration, combien de secondes passent ?", {{1, "16 secondes"}, {2, "20 secondes"}, {3, "24 secondes"}}, 1, "Logique nickel : tu comprends le rythme de la mine.", "Logique bancale : la mine te donne la réponse en vibrant sous tes pieds."});
        }
        else if (biome.name == "Verger des lucioles de fer")
        {
            addBiomeChallenge({"orchard_light", "ÉPREUVE DE LUMIÈRE", "Trois lucioles clignotent 2, 4 puis 6 fois. Quel rythme semble logique ensuite ?", {{1, "7"}, {2, "8"}, {3, "12"}}, 2, "Suite propre : tu synchronises ta marche avec l'essaim.", "Suite ratée : les lucioles se dispersent."});
        }
        else if (biome.name == "Archives noyées")
        {
            addBiomeChallenge({"archives_sorting", "ÉPREUVE DE CLASSEMENT", "Une archive porte les cotes A-12, A-13 et A-15. Quelle cote manque probablement ?", {{1, "A-14"}, {2, "B-12"}, {3, "A-16"}}, 1, "Classement net : la page accepte d'être lue.", "Classement faux : l'archive se referme."});
        }
        else if (biome.name == "Falaises des drakes gris")
        {
            addBiomeChallenge({"cliffs_rope", "ÉPREUVE DE CORDE", "Tu as 24 mètres de corde et 4 points d'ancrage égaux. Combien de mètres par point ?", {{1, "5 mètres"}, {2, "6 mètres"}, {3, "8 mètres"}}, 2, "Ancrage propre : la falaise te respecte presque.", "Mauvais partage : la corde tient, mais ton cœur descend avant tes pieds.", 6});
            addBiomeChallenge({"cliffs_wind", "ÉPREUVE DE VENT", "Une rafale arrive depuis l'ouest. Où places-tu ton appui le plus solide ?", {{1, "Du côté ouest, contre la poussée"}, {2, "Sur une pierre mobile"}, {3, "Le plus loin possible de la paroi"}}, 1, "Appui propre : la rafale passe sans t'arracher à la corniche.", "Appui mauvais : tu récupères ton équilibre au prix d'une belle frayeur."});
        }
        else if (biome.name == "Foire abandonnée")
        {
            addBiomeChallenge({"fair_french", "ÉPREUVE DE FRANÇAIS FORAIN", "Quel panneau est écrit correctement ?", {{1, "Les tickets sont valables."}, {2, "Les ticket sont valable."}, {3, "Les tickets est valables."}}, 1, "Panneau propre : même la caisse semble moins te juger.", "Panneau faux : la foire applaudit probablement pour se moquer."});
        }
        else if (biome.name == "Temple des cloches fendues")
        {
            addBiomeChallenge({"temple_bells", "ÉPREUVE DE SERMENT", "Une cloche sonne 3 fois, puis 6, puis 9. Combien devrait-elle sonner ensuite ?", {{1, "10"}, {2, "12"}, {3, "18"}}, 2, "Rythme compris : la cloche cesse de vibrer juste assez longtemps.", "Rythme raté : la cloche attire des regards invisibles."});
        }
        else if (biome.name == "Canaux de brume bleue")
        {
            addBiomeChallenge({"canals_crates", "ÉPREUVE DE PASSAGE", "Un bac porte 3 caisses par traversée. Combien de traversées chargées pour 9 caisses ?", {{1, "2"}, {2, "3"}, {3, "4"}}, 2, "Calcul net : les caisses passent sans voyage inutile.", "Calcul faux : tu comprends pourquoi les passeurs facturent au trajet."});
        }
        else if (biome.name == "Carrière des os blancs")
        {
            addBiomeChallenge({"quarry_measure", "ÉPREUVE DE MESURE", "Une trace mesure 40 cm. Une autre est deux fois plus grande. Combien mesure la deuxième ?", {{1, "60 cm"}, {2, "80 cm"}, {3, "120 cm"}}, 2, "Mesure propre : tu sais quand une empreinte devient inquiétante.", "Mesure bancale : la carrière paraît soudain moins vide."});
        }
        else if (biome.name == "Marché sous les ponts")
        {
            addBiomeChallenge({"market_contract", "ÉPREUVE DE CONTRAT", "Quelle phrase évite le mieux une arnaque dans un reçu ?", {{1, "Payé après livraison vérifiée."}, {2, "Payer quand le vendeur dit que c'est bon."}, {3, "Payé peut-être demain hier."}}, 1, "Reçu propre : même le vendeur douteux respecte ton sérieux.", "Reçu faible : tu viens peut-être d'acheter une explication."});
        }
        else if (biome.name == "Jardin des statues qui pleurent")
        {
            addBiomeChallenge({"garden_statues", "ÉPREUVE D'OBSERVATION", "Trois statues regardent la fontaine, sauf une qui regarde la sortie. Laquelle surveiller ?", {{1, "Celle qui regarde la sortie"}, {2, "La plus jolie"}, {3, "Aucune"}}, 1, "Observation utile : tu repères celle qui connaît ton chemin de fuite.", "Observation ratée : le jardin change quand tu clignes des yeux."});
        }

        std::vector<std::size_t> availableIndexes;
        for (std::size_t index = 0; index < challenges.size(); ++index)
        {
            const std::string cooldownKey = "challenge:" + challenges[index].id;
            if (!player.wasExplorationChallengeRecentlySeen(challenges[index].id)
                && !player.isExplorationSceneOnCooldown(cooldownKey))
            {
                availableIndexes.push_back(index);
            }
        }
        if (availableIndexes.empty())
        {
            for (std::size_t index = 0; index < challenges.size(); ++index)
            {
                if (!player.isExplorationSceneOnCooldown("challenge:" + challenges[index].id))
                {
                    availableIndexes.push_back(index);
                }
            }
        }
        if (availableIndexes.empty())
        {
            for (std::size_t index = 0; index < challenges.size(); ++index) availableIndexes.push_back(index);
        }

        const std::size_t selectedIndex = availableIndexes[static_cast<std::size_t>(random.between(0, static_cast<int>(availableIndexes.size()) - 1))];
        const Challenge& challenge = challenges[selectedIndex];
        player.recordExplorationChallengeKey(challenge.id);
        player.startExplorationSceneCooldown("challenge:" + challenge.id, challenge.cooldownDays);

        int choice = askChoiceScreen(
            challenge.title,
            "exploration.micro_challenge." + challenge.id,
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

    MicroChallengeResult runGuildServiceMicroChallenge(Quest& quest, Random& random)
    {
        const std::string questText = toLowerChoiceText(quest.title + " " + quest.objective + " " + quest.location + " " + quest.targetFamily + " " + quest.client);

        struct Challenge
        {
            Challenge(
                std::string challengeTitle,
                std::string challengeQuestion,
                std::vector<std::pair<int, std::string>> challengeOptions,
                int challengeCorrectChoice,
                std::string challengeSuccessLine,
                std::string challengeFailureLine
            )
                : title(std::move(challengeTitle)),
                  question(std::move(challengeQuestion)),
                  options(std::move(challengeOptions)),
                  correctChoice(challengeCorrectChoice),
                  successLine(std::move(challengeSuccessLine)),
                  failureLine(std::move(challengeFailureLine))
            {
            }

            std::string title;
            std::string question;
            std::vector<std::pair<int, std::string>> options;
            int correctChoice = 1;
            std::string successLine;
            std::string failureLine;
            std::string id;
            std::string family;
            bool unusualDocument = false;
            std::string handlingQuestion;
            std::vector<std::pair<int, std::string>> handlingOptions;
            int correctHandlingChoice = 0;
            std::string handlingSuccessLine;
            std::string handlingFailureLine;
        };

        std::vector<Challenge> merchantSelectedChallenges;

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
            std::vector<Challenge> merchantChallenges = {
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
                    {{1, "9 électrum et 3 fer"}, {2, "7 fer"}, {3, "1 électrum"}},
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
                    "La note dit : « Vente : 7 potions à 12 fer. Montant total : 84 [unité manquante] ». Quelle unité complète correctement le total ?",
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
                    {{1, "Par la présente, je soussigné, Prunigil, marchand, promets à… [nom du client manquant]."}, {2, "Par la présente je ssoussigné le marchan Prunigil promet a qui déjà ???"}, {3, "Par la présente, je sous-signé le marchan Prunigil promet à quelqu'un."}},
                    1,
                    "Contrat corrigé : l'identité manquante reste signalée au lieu d'être inventée.",
                    "Contrat raté : juridiquement, même un gobelin refuserait de signer."
                },
                {
                    "FRANÇAIS — MISE À JOUR DE CLIENT",
                    "Quelle correction transmet correctement la nouvelle demande ?",
                    {{1, "Le client de la commande 17 est revenu. Il souhaite remplacer deux caisses et conserver le reste de la livraison."}, {2, "Le client de la commande 17 est revenue. Il veux changé deux caisse."}, {3, "Le clients veut tout changer sauf ce qu'il garde."}},
                    1,
                    "Mise à jour claire : la nouvelle demande peut être reliée au premier dossier.",
                    "Mise à jour floue : personne ne sait quelles caisses doivent encore partir."
                },
                {
                    "FRANÇAIS — LETTRE DE RECOMMANDATION",
                    "Quelle formulation est assez professionnelle pour recommander un aide-marchand ?",
                    {{1, "Je recommande cet aventurier pour son sérieux au comptoir, sa discrétion et la précision de ses vérifications."}, {2, "Je recommande cette aventurier car il est pas trop mauvais avec les papier."}, {3, "Prenez-le, il compte mieux que mon dernier apprenti."}},
                    1,
                    "Recommandation propre : un autre vendeur peut la prendre au sérieux.",
                    "Recommandation ratée : le prochain marchand risque surtout de plaindre Prunigil."
                },
                {
                    "FRANÇAIS — AVIS DE DÉSTOCKAGE",
                    "Quelle annonce explique correctement l'offre sans tromper les clients ?",
                    {{1, "Déstockage pendant trois jours : jusqu'à quatre paires de bottes usées sont proposées à prix réduit, dans la limite du stock disponible."}, {2, "Solde pour toujours pendant 3 jour sur toute les bottes qu'on a peut être."}, {3, "Tout est gratuit jusqu'à épuisement de Prunigil."}},
                    1,
                    "Annonce honnête : durée, quantité et état des invendus sont indiqués.",
                    "Annonce trompeuse : la garde commerciale finira par demander des explications."
                },
                {
                    "FRANÇAIS — VENDEUR TEMPORAIRE",
                    "Quelle affiche indique clairement la présence du vendeur ?",
                    {{1, "Mirette sera présente au marché pendant deux jours. Son stock est limité et dépend des tissus qu'elle a pu transporter."}, {2, "Mirette sera la tout le temps pendant deux jours sauf quand elle repart."}, {3, "Une vendeuse viendra quelque part bientôt, demandez à Prunigil."}},
                    1,
                    "Affiche claire : les clients savent qui vient, où et pour combien de temps.",
                    "Affiche floue : le vendeur temporaire risque de repartir sans avoir été trouvé."
                },
                {
                    "MARCHAND — LOT D'INVENDUS",
                    "Un déstockage porte sur 4 objets identiques à 18 fer chacun avec 20% de réduction. Quel total doit payer le client si la remise est arrondie au cuivre inférieur par objet ?",
                    {{1, "57 fer et 6 cuivre"}, {2, "72 fer"}, {3, "14 fer et 4 cuivre"}},
                    1,
                    "Déstockage calculé : quatre invendus quittent enfin l'étagère sans fausser la caisse.",
                    "Déstockage faux : Prunigil vient de retrouver une raison de recompter toute la soirée."
                }
            };

            auto challengeIdFromTitle = [](const std::string& title)
            {
                std::string id;
                for (unsigned char c : title)
                {
                    if (std::isalnum(c)) id.push_back(static_cast<char>(std::tolower(c)));
                    else if (!id.empty() && id.back() != '_') id.push_back('_');
                }
                while (!id.empty() && id.back() == '_') id.pop_back();
                return std::string("merchant_") + id;
            };

            auto historyContains = [&](const std::string& id)
            {
                std::stringstream stream(quest.serviceChallengeHistory);
                std::string value;
                while (std::getline(stream, value, '|'))
                {
                    if (value == id) return true;
                }
                return false;
            };

            auto configureMerchantChallenge = [&](Challenge& challenge)
            {
                challenge.id = challengeIdFromTitle(challenge.title);
                const std::string text = toLowerChoiceText(challenge.title + " " + challenge.question);

                if (text.find("caravane") != std::string::npos || text.find("livraison") != std::string::npos
                    || text.find("chariot") != std::string::npos || text.find("roue") != std::string::npos
                    || text.find("péage") != std::string::npos || text.find("peage") != std::string::npos)
                {
                    challenge.family = "transport";
                }
                else if (text.find("luxe") != std::string::npos || text.find("brocante") != std::string::npos
                    || text.find("offre") != std::string::npos || text.find("fournisseur") != std::string::npos
                    || text.find("achat suspect") != std::string::npos)
                {
                    challenge.family = "estimation";
                }
                else if (text.find("contrat") != std::string::npos || text.find("registre") != std::string::npos
                    || text.find("facture") != std::string::npos || text.find("comptable") != std::string::npos
                    || text.find("formulaire") != std::string::npos || text.find("note") != std::string::npos)
                {
                    challenge.family = "registre";
                }
                else if (text.find("monnaie") != std::string::npos || text.find("taxe") != std::string::npos
                    || text.find("réduction") != std::string::npos || text.find("reduction") != std::string::npos
                    || text.find("bénéfice") != std::string::npos || text.find("benefice") != std::string::npos
                    || text.find("stock") != std::string::npos || text.find("prix") != std::string::npos
                    || text.find("potions") != std::string::npos)
                {
                    challenge.family = "calcul";
                }
                else
                {
                    challenge.family = "francais";
                }

                challenge.unusualDocument = text.find("bidon bleu") != std::string::npos
                    || text.find("coffre maudit") != std::string::npos
                    || text.find("frigo-froid") != std::string::npos
                    || text.find("journal personnel") != std::string::npos
                    || text.find("message codé") != std::string::npos
                    || text.find("message code") != std::string::npos
                    || text.find("petite inscription") != std::string::npos;

                if (text.find("message codé") != std::string::npos || text.find("message code") != std::string::npos
                    || text.find("journal personnel") != std::string::npos)
                {
                    challenge.handlingQuestion = "Le texte ressemble à un message personnel ou confidentiel. Que fais-tu après la correction ?";
                    challenge.handlingOptions = {
                        {1, "J'avertis Prunigil, je lui lis le message, puis je poursuis sans le recopier dans le registre public."},
                        {2, "Je le publie sur le comptoir pour que tout le monde puisse aider."},
                        {3, "Je le détruis sans prévenir personne."}
                    };
                    challenge.correctHandlingChoice = 1;
                    challenge.handlingSuccessLine = "Confidentialité respectée : Prunigil est averti sans transformer une note privée en affiche publique.";
                    challenge.handlingFailureLine = "Le texte est corrigé, mais son traitement reste mauvais : Prunigil refuse de valider l'étape complète.";
                }
                else if (text.find("entrée comptable") != std::string::npos || text.find("entree comptable") != std::string::npos)
                {
                    challenge.handlingQuestion = "Le message original indique 7 fer alors que le calcul donne 8 fer. Comment archiver la correction ?";
                    challenge.handlingOptions = {
                        {1, "Je conserve le message original et j'ajoute une note séparée indiquant la différence correcte de 8 fer."},
                        {2, "Je remplace directement le 7 par un 8 sans laisser de trace."},
                        {3, "Je laisse 7 fer pour ne pas vexer l'apprenti."}
                    };
                    challenge.correctHandlingChoice = 1;
                    challenge.handlingSuccessLine = "Correction traçable : le document original reste intact et la note comptable répare le calcul.";
                    challenge.handlingFailureLine = "Le calcul est compris, mais la méthode d'archive est mauvaise : l'étape reste partiellement traitée.";
                }
                else if (text.find("contrat chaotique") != std::string::npos)
                {
                    challenge.handlingQuestion = "Le nom du client manque encore. Quelle décision est professionnelle ?";
                    challenge.handlingOptions = {
                        {1, "Je refuse la validation finale tant que l'identité du client n'est pas confirmée."},
                        {2, "J'invente un nom plausible pour gagner du temps."},
                        {3, "Je signe quand même parce que Prunigil est pressé."}
                    };
                    challenge.correctHandlingChoice = 1;
                    challenge.handlingSuccessLine = "Contrat suspendu proprement : mieux vaut une signature tardive qu'un engagement sans destinataire.";
                    challenge.handlingFailureLine = "La phrase est meilleure, mais le contrat reste juridiquement dangereux.";
                }
            };

            for (Challenge& challenge : merchantChallenges)
            {
                configureMerchantChallenge(challenge);
            }

            auto challengeMatchesQuest = [&](const Challenge& challenge)
            {
                if (questText.find("caravane") != std::string::npos || questText.find("péage") != std::string::npos || questText.find("peage") != std::string::npos)
                    return challenge.family == "transport" || challenge.family == "calcul";
                if (questText.find("brocante") != std::string::npos || questText.find("luxe") != std::string::npos || questText.find("estimation") != std::string::npos)
                    return challenge.family == "estimation" || challenge.family == "registre";
                if (questText.find("facture") != std::string::npos || questText.find("monnaie") != std::string::npos
                    || questText.find("réduction") != std::string::npos || questText.find("reduction") != std::string::npos
                    || questText.find("taxe") != std::string::npos || questText.find("auberge") != std::string::npos)
                    return challenge.family == "calcul" || challenge.family == "registre";
                if (questText.find("registre") != std::string::npos || questText.find("contrat") != std::string::npos
                    || questText.find("message") != std::string::npos)
                    return challenge.family == "registre" || challenge.family == "francais";
                return challenge.family == "registre" || challenge.family == "calcul" || challenge.family == "francais";
            };

            std::vector<Challenge> unusedAll;
            std::vector<Challenge> unusedRelevant;
            std::vector<Challenge> unusedUnusual;
            for (const Challenge& challenge : merchantChallenges)
            {
                if (historyContains(challenge.id)) continue;
                unusedAll.push_back(challenge);
                if (challengeMatchesQuest(challenge)) unusedRelevant.push_back(challenge);
                if (challenge.unusualDocument) unusedUnusual.push_back(challenge);
            }

            if (unusedAll.empty())
            {
                quest.serviceChallengeHistory.clear();
                unusedAll = merchantChallenges;
                for (const Challenge& challenge : merchantChallenges)
                {
                    if (challengeMatchesQuest(challenge)) unusedRelevant.push_back(challenge);
                    if (challenge.unusualDocument) unusedUnusual.push_back(challenge);
                }
            }

            const int poolRoll = random.between(1, 100);
            if (poolRoll <= 70 && !unusedRelevant.empty()) merchantSelectedChallenges = unusedRelevant;
            else if (poolRoll <= 90 || unusedUnusual.empty()) merchantSelectedChallenges = unusedAll;
            else merchantSelectedChallenges = unusedUnusual;
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

        const std::vector<Challenge>& selectedPool = !merchantSelectedChallenges.empty()
            ? merchantSelectedChallenges
            : challenges;
        const Challenge& challenge = selectedPool[random.between(0, static_cast<int>(selectedPool.size()) - 1)];
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
        const bool mainAnswerCorrect = choice == challenge.correctChoice;
        result.success = mainAnswerCorrect;
        result.lines.push_back(mainAnswerCorrect ? challenge.successLine : challenge.failureLine);

        if (!challenge.id.empty())
        {
            if (!quest.serviceChallengeHistory.empty()) quest.serviceChallengeHistory += "|";
            quest.serviceChallengeHistory += challenge.id;
        }

        if (mainAnswerCorrect && challenge.correctHandlingChoice > 0 && !challenge.handlingOptions.empty())
        {
            int handlingChoice = askChoiceScreen(
                "TRAITEMENT DU DOCUMENT",
                "quest.guild.service.document_handling",
                {
                    "La correction du texte ne suffit pas : il faut aussi traiter le document correctement.",
                    challenge.handlingQuestion
                },
                challenge.handlingOptions,
                1,
                3
            );
            Console::clear();

            if (handlingChoice == challenge.correctHandlingChoice)
            {
                result.lines.push_back(challenge.handlingSuccessLine);
            }
            else
            {
                result.success = false;
                result.partial = true;
                result.lines.push_back(challenge.handlingFailureLine);
            }
        }

        if (result.success)
        {
            result.lines.push_back("Le service peut progresser.");
        }
        else if (result.partial)
        {
            result.lines.push_back("Étape partiellement comprise : aucune progression, mais Prunigil ne facture pas l'erreur comme un échec complet.");
        }
        else
        {
            result.lines.push_back("Le service ne progresse pas cette fois. Tu pourras réessayer plus tard.");
        }
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
        if (roll <= 9) return "récolte exposée";
        if (roll <= 18) return "ressource dissimulée";
        if (roll <= 26) return "cueillette fragile";
        if (roll <= 31) return "piste interrompue";
        if (roll <= 36) return "marques de passage";
        if (roll <= 40) return "objet perdu";
        if (roll <= 46) return "bourse oubliée";
        if (roll <= 52) return "petit dépôt ancien";
        if (roll <= 59) return "fausses pièces";
        if (roll <= 64) return "coffre à demi enfoui";
        if (roll <= 70) return "coffre trop visible";
        if (roll <= 76) return "prédateurs territoriaux";
        if (roll <= 82) return "groupe en déplacement";
        if (roll == 83) return "champion errant";
        if (roll <= 84) return "gardien local";
        if (roll <= 87) return "voyageur en difficulté";
        if (roll <= 91) return "demande locale imprévue";
        if (roll <= 94) return "zone anormalement calme";
        if (roll <= 97) return "biome soudainement agité";
        if (roll == 98) return "passage interdit";
        if (roll <= 99) return "site instable";
        return "découverte rare";
    }

    std::string explorationEventKeyFromRoll(int roll)
    {
        if (roll <= 9) return "main_gather_exposed";
        if (roll <= 18) return "main_gather_hidden";
        if (roll <= 26) return "main_gather_fragile";
        if (roll <= 31) return "main_trace_broken";
        if (roll <= 36) return "main_trace_passage";
        if (roll <= 40) return "main_trace_lost_object";
        if (roll <= 46) return "main_treasure_purse";
        if (roll <= 52) return "main_treasure_deposit";
        if (roll <= 59) return "main_fake_coins";
        if (roll <= 64) return "main_chest_buried";
        if (roll <= 70) return "main_chest_obvious";
        if (roll <= 76) return "main_fight_territorial";
        if (roll <= 82) return "main_fight_moving_group";
        if (roll == 83) return "main_miniboss_wanderer";
        if (roll <= 84) return "main_miniboss_guardian";
        if (roll <= 87) return "main_npc_quest_traveler";
        if (roll <= 91) return "main_npc_quest_local";
        if (roll <= 94) return "main_active_event_quiet";
        if (roll <= 97) return "main_active_event_agitated";
        if (roll == 98) return "main_dangerous_site_forbidden";
        if (roll <= 99) return "main_dangerous_site_unstable";
        return "main_rare_discovery";
    }

    int explorationEventCooldownDays(const std::string& key)
    {
        if (key == "main_fake_coins") return 6;
        if (key.find("main_miniboss_") == 0) return 8;
        if (key.find("main_npc_quest_") == 0) return 2;
        if (key.find("main_dangerous_site_") == 0) return 7;
        if (key == "main_rare_discovery") return 12;
        if (key == "main_chest_obvious") return 2;
        return 0;
    }
    bool isMainMiniBossExplorationKey(const std::string& key)
    {
        return key.find("main_miniboss_") == 0;
    }

    bool isAnyMainMiniBossExplorationRecentlySeen(const Player& player)
    {
        return player.wasExplorationEventRecentlySeen("main_miniboss_wanderer")
            || player.wasExplorationEventRecentlySeen("main_miniboss_guardian")
            || player.isExplorationSceneOnCooldown("main_miniboss_wanderer")
            || player.isExplorationSceneOnCooldown("main_miniboss_guardian");
    }


    std::string activeExplorationEventKeyFromRoll(int roll)
    {
        if (roll <= 18) return "active_abandoned_camp";
        if (roll <= 32) return "active_local_den";
        if (roll <= 46) return "active_tracks";
        if (roll <= 58) return "active_hazard";
        if (roll <= 68) return "active_hidden_cache";
        if (roll <= 80) return "active_wave";
        if (roll <= 91) return "active_ancient_sign";
        return "active_distress_call";
    }

    int activeExplorationEventCooldownDays(const std::string& key)
    {
        if (key == "active_abandoned_camp") return 4;
        if (key == "active_local_den") return 3;
        if (key == "active_tracks") return 1;
        if (key == "active_hazard") return 2;
        if (key == "active_hidden_cache") return 5;
        if (key == "active_wave") return 3;
        if (key == "active_ancient_sign") return 8;
        if (key == "active_distress_call") return 2;
        return 0;
    }

    int chooseVariedMainExplorationRoll(
        Player& player,
        Random& random,
        int preferredRoll,
        const ExplorationIntensity& intensity,
        int extraShift,
        const std::set<std::string>& currentRunKeys
    )
    {
        int candidate = std::clamp(preferredRoll, 1, 100);
        for (int attempt = 0; attempt < 18; ++attempt)
        {
            const std::string key = explorationEventKeyFromRoll(candidate);
            if (isMainMiniBossExplorationKey(key) && isAnyMainMiniBossExplorationRecentlySeen(player))
            {
                candidate = adjustExplorationEventRoll(random.between(1, 100), intensity);
                candidate = std::clamp(candidate + extraShift, 1, 100);
                continue;
            }
            if (!player.wasExplorationEventRecentlySeen(key)
                && currentRunKeys.find(key) == currentRunKeys.end()
                && !player.isExplorationSceneOnCooldown(key))
            {
                return candidate;
            }
            candidate = adjustExplorationEventRoll(random.between(1, 100), intensity);
            candidate = std::clamp(candidate + extraShift, 1, 100);
        }

        // The recent-history filter is deliberately softer than a true cooldown:
        // a related theme may reappear, but the exact illogical scene cannot.
        for (int roll = 1; roll <= 100; ++roll)
        {
            const std::string key = explorationEventKeyFromRoll(roll);
            if (isMainMiniBossExplorationKey(key) && isAnyMainMiniBossExplorationRecentlySeen(player))
            {
                continue;
            }
            if (currentRunKeys.find(key) == currentRunKeys.end()
                && !player.isExplorationSceneOnCooldown(key)
                && !player.wasExplorationEventRecentlySeen(key))
            {
                return roll;
            }
        }
        for (int roll = 1; roll <= 100; ++roll)
        {
            const std::string key = explorationEventKeyFromRoll(roll);
            if (isMainMiniBossExplorationKey(key) && isAnyMainMiniBossExplorationRecentlySeen(player))
            {
                continue;
            }
            if (currentRunKeys.find(key) == currentRunKeys.end()
                && !player.isExplorationSceneOnCooldown(key))
            {
                return roll;
            }
        }
        return candidate;
    }

    int chooseVariedActiveExplorationRoll(Player& player, Random& random)
    {
        int candidate = random.between(1, 100);
        for (int attempt = 0; attempt < 16; ++attempt)
        {
            const std::string key = activeExplorationEventKeyFromRoll(candidate);
            if (!player.wasExplorationEventRecentlySeen(key)
                && !player.isExplorationSceneOnCooldown(key))
            {
                return candidate;
            }
            candidate = random.between(1, 100);
        }
        for (int roll = 1; roll <= 100; ++roll)
        {
            const std::string key = activeExplorationEventKeyFromRoll(roll);
            if (!player.wasExplorationEventRecentlySeen(key)
                && !player.isExplorationSceneOnCooldown(key))
            {
                return roll;
            }
        }
        for (int roll = 1; roll <= 100; ++roll)
        {
            const std::string key = activeExplorationEventKeyFromRoll(roll);
            if (!player.isExplorationSceneOnCooldown(key)) return roll;
        }
        return candidate;
    }

    int applyChapterThreeExplorationChoiceBias(const Player& player, Random& random, int roll)
    {
        if (!player.hasStoryModeStarted() || player.getStoryChapter() < 3)
        {
            return std::clamp(roll, 1, 100);
        }

        const std::string routeChoice = StoryCampaign::getChapterThreeRouteChoice(player);
        const std::string convoyDecision = StoryCampaign::getChapterThreeConvoyDecision(player);
        int adjusted = std::clamp(roll, 1, 100);

        if (routeChoice == "commerce" && random.between(1, 100) <= 28)
        {
            const std::vector<int> commerceRolls = {random.between(1, 26), random.between(41, 52), random.between(60, 70)};
            adjusted = commerceRolls[static_cast<std::size_t>(random.between(0, static_cast<int>(commerceRolls.size()) - 1))];
        }
        else if (routeChoice == "secours" && adjusted >= 71 && random.between(1, 100) <= 34)
        {
            adjusted = random.between(85, 91);
        }
        else if (routeChoice == "recherche" && random.between(1, 100) <= 30)
        {
            adjusted = random.between(1, 100) <= 65 ? random.between(27, 40) : random.between(92, 97);
        }

        if (convoyDecision == "marchandises" && random.between(1, 100) <= 18)
        {
            adjusted = random.between(1, 100) <= 55 ? random.between(41, 52) : random.between(60, 70);
        }
        else if (convoyDecision == "preuves" && random.between(1, 100) <= 22)
        {
            adjusted = random.between(1, 100) <= 70 ? random.between(27, 40) : random.between(92, 97);
        }
        else if (convoyDecision == "quarantaine" && adjusted >= 71 && random.between(1, 100) <= 40)
        {
            adjusted = random.between(1, 100) <= 55 ? random.between(1, 26) : random.between(27, 40);
        }

        return std::clamp(adjusted, 1, 100);
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
        const int readyAfter = countReadyToTurnInQuests(player);

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


    bool chestClassContainsAny(const Player& player, const std::vector<std::string>& needles)
    {
        const std::string classText = toLowerChoiceText(player.getType());
        for (const std::string& needle : needles)
        {
            if (classText.find(toLowerChoiceText(needle)) != std::string::npos)
            {
                return true;
            }
        }
        return false;
    }

    bool playerHasChestTool(const Player& player)
    {
        const Inventory& inventory = player.getInventory();
        return inventory.countMaterialById("rusted_metal_fragment") > 0
            || inventory.countMaterialById("weak_repair_kit") > 0
            || inventory.countMaterialById("medium_repair_kit") > 0
            || inventory.countMaterialById("big_repair_kit") > 0
            || inventory.countMaterialById("tinkerer_complete_repair_kit") > 0
            || inventory.countMaterialById("small_repair_kit") > 0
            || inventory.countMaterialById("reinforced_repair_kit") > 0;
    }

    std::string consumeImprovisedChestToolIfNeeded(Player& player)
    {
        Inventory& inventory = player.getInventory();
        if (inventory.countMaterialById("rusted_metal_fragment") > 0
            && inventory.removeMaterialQuantityById("rusted_metal_fragment", 1))
        {
            return "Outil improvisé consommé : fragment métallique rouillé x1.";
        }
        return "Aucun outil consommé : la classe ou le matériel déjà adapté suffit pour cette tentative.";
    }

    int chestOpeningGoldMultiplier(int methodChoice)
    {
        if (methodChoice == 3) return 86; // Force : parfois le contenu fragile prend cher.
        if (methodChoice == 4) return 82; // Prudence : moins de risque, mais fouille moins agressive.
        if (methodChoice == 2) return 112; // Crochetage/observation : meilleur accès aux caches propres.
        return 100;
    }

    std::vector<std::string> buildChestMethodLines(const Player& player)
    {
        std::vector<std::string> lines;
        lines.push_back("Un coffre est posé là, presque trop calmement.");
        lines.push_back("Tous les coffres ne sont pas fermés à clef : certains sont déjà entrouverts, piégés, vides ou juste suspects.");
        lines.push_back("Ta méthode change surtout le risque de piège, de casse et la qualité de la fouille.");
        if (chestClassContainsAny(player, {"voleur", "roublard", "brigand", "assassin", "rôdeur", "rodeur", "trappeur"}))
        {
            lines.push_back("Profil utile : ton style aide pour observer/crocheter sans tout casser.");
        }
        if (chestClassContainsAny(player, {"forgeron", "bricoleur", "artificier", "alchim", "récupérateur", "recuperateur", "artisan"}))
        {
            lines.push_back("Profil utile : ton côté artisan aide à comprendre les mécanismes et les charnières.");
        }
        if (playerHasChestTool(player))
        {
            lines.push_back("Matériel utile détecté : fragment métallique ou kit pouvant servir d'outil improvisé.");
        }
        if (player.hasPassiveSkill("lock_reader"))
        {
            lines.push_back("Lecture acquise : tes observations de combat aident aussi à lire serrures, pièges et fausses ouvertures.");
        }
        return lines;
    }

    // EN: openExplorationChest declares or implements a focused behavior used by this module.
    // FR: openExplorationChest déclare ou implémente un comportement précis utilisé par ce module.
    void openExplorationChest(Player& player, Random& random, const ExplorationBiome& biome, const ExplorationIntensity& intensity, DifficultyMode difficulty, DeathRuleMode deathRule)
    {
        const bool lockReader = player.hasPassiveSkill("lock_reader");
        const bool nimbleOrThief = lockReader || chestClassContainsAny(player, {"voleur", "roublard", "brigand", "assassin", "rôdeur", "rodeur", "trappeur"});
        const bool craftSpecialist = chestClassContainsAny(player, {"forgeron", "bricoleur", "artificier", "alchim", "récupérateur", "recuperateur", "artisan"});
        const bool bruteSpecialist = chestClassContainsAny(player, {"barbare", "berserker", "briseur", "colosse", "orc", "guerrier", "chevalier"});
        const bool hasTool = playerHasChestTool(player);
        const bool hasWeapon = player.hasEquippedWeapon();

        int choice = askChoiceScreen(
            "COFFRE SUSPECT",
            "exploration.chest.choice",
            buildChestMethodLines(player),
            {
                {1, "Ouvrir normalement"},
                {2, "Observer / crocheter proprement"},
                {3, "Forcer avec arme ou outil"},
                {4, "Ouvrir très prudemment"},
                {0, "Le laisser tranquille"}
            },
            0,
            4
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

        std::vector<std::string> methodLines;
        int trapThreshold = 16;
        int mimicThreshold = 28;
        int emptyThreshold = 42;
        int modestThreshold = 76;
        int rewardQuantityBonus = 0;
        bool alreadyUnlocked = false;

        if (choice == 2)
        {
            if (nimbleOrThief || craftSpecialist || hasTool)
            {
                trapThreshold -= 6;
                mimicThreshold -= 2;
                emptyThreshold -= 3;
                modestThreshold -= 5;
                rewardQuantityBonus = 1;
                methodLines.push_back("Méthode : observation/crochetage. Tu cherches les charnières, pièges et caches avant de tirer dessus.");
                if (lockReader)
                {
                    methodLines.push_back("Lecture des serrures et failles : l'expérience d'observation réduit le risque même sans être voleur pur.");
                }
                if (!nimbleOrThief && !craftSpecialist)
                {
                    methodLines.push_back(consumeImprovisedChestToolIfNeeded(player));
                }
            }
            else
            {
                trapThreshold += 8;
                mimicThreshold += 4;
                emptyThreshold += 2;
                methodLines.push_back("Méthode : crochetage improvisé sans vrai outil. C'est possible, mais franchement pas élégant.");
            }
        }
        else if (choice == 3)
        {
            if (!hasWeapon && !hasTool && !bruteSpecialist)
            {
                trapThreshold += 10;
                emptyThreshold += 6;
                methodLines.push_back("Méthode : forçage sans arme, sans outil et sans vrai profil de force. Le coffre prend ça personnellement.");
            }
            else
            {
                trapThreshold += bruteSpecialist ? 2 : 6;
                mimicThreshold -= 3;
                emptyThreshold += 4;
                rewardQuantityBonus = -1;
                methodLines.push_back("Méthode : forçage. Rapide, utile si c'est déjà fragile, mais le contenu peut souffrir.");
            }
        }
        else if (choice == 4)
        {
            trapThreshold -= 8;
            mimicThreshold -= 4;
            emptyThreshold += 8;
            modestThreshold += 6;
            rewardQuantityBonus = -1;
            methodLines.push_back("Méthode : prudence maximale. Tu limites le risque, mais tu n'oses pas fouiller les caches les plus agressives.");
        }
        else
        {
            methodLines.push_back("Méthode : ouverture normale. Simple, pas forcément stupide : certains coffres ne sont même pas verrouillés.");
        }

        trapThreshold = std::clamp(trapThreshold, 5, 34);
        mimicThreshold = std::clamp(std::max(mimicThreshold, trapThreshold + 3), trapThreshold + 3, 45);
        emptyThreshold = std::clamp(std::max(emptyThreshold, mimicThreshold + 4), mimicThreshold + 4, 62);
        modestThreshold = std::clamp(std::max(modestThreshold, emptyThreshold + 10), emptyThreshold + 10, 92);

        if (random.between(1, 100) <= (choice == 4 ? 18 : 11))
        {
            alreadyUnlocked = true;
            methodLines.push_back("Surprise correcte : le coffre n'était pas fermé. La méthode sert surtout à ne pas le manipuler n'importe comment.");
        }

        int roll = random.between(1, 100);
        if (alreadyUnlocked)
        {
            roll = std::max(roll, emptyThreshold + 1);
        }

        if (!methodLines.empty())
        {
            showExplorationNotice("MÉTHODE DE COFFRE", "exploration.chest.method", methodLines);
        }

        if (roll <= trapThreshold)
        {
            int damage = std::min(random.between(5, 18 + player.getLevel()), std::max(0, player.getHp() - 1));
            if (choice == 4)
            {
                damage = damage * 65 / 100;
            }
            else if (choice == 3)
            {
                damage = damage * 115 / 100;
            }
            if (damage > 0)
            {
                player.takeDamage(damage);
                std::vector<std::string> trapLines = {
                    "Un mécanisme claque.",
                    "Méthode utilisée : " + std::to_string(choice) + ".",
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
        else if (roll <= mimicThreshold)
        {
            showExplorationNotice(
                "MIMIC",
                "exploration.chest.mimic",
                {"Le coffre se déplie d'un coup. Ce n'était pas un coffre. Mimic.", "Bonne nouvelle relative : le forcer pouvait au moins l'énerver avant qu'il te morde."}
            );
            simulateUnexpectedExplorationFight(player, random, biome, intensity, difficulty, deathRule);
            int baseGold = random.between(8, 24 + player.getLevel() * 2);
            baseGold = baseGold * chestOpeningGoldMultiplier(choice) / 100;
            int gold = applyExplorationGoldReward(std::max(1, baseGold), player, intensity, difficulty, 1);
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
        else if (roll <= emptyThreshold)
        {
            showExplorationNotice("COFFRE VIDE", "exploration.chest.empty", {"Le coffre est vide. Quelqu'un a déjà eu l'idée avant toi.", "Au moins, cette fois, le coffre ne t'a pas expliqué sa philosophie avec des dents."});
        }
        else if (roll <= modestThreshold)
        {
            int baseGold = random.between(8, 30 + player.getLevel() * 3);
            baseGold = baseGold * chestOpeningGoldMultiplier(choice) / 100;
            int gold = applyExplorationGoldReward(std::max(1, baseGold), player, intensity, difficulty, 1);
            player.getInventory().earnGold(gold);
            const int quantity = std::max(1, applyExplorationQuantityBonus(1 + std::max(0, rewardQuantityBonus), intensity));
            std::vector<std::string> rewardLines = {
                alreadyUnlocked ? "Le coffre était déjà ouvert, mais personne n'avait regardé assez correctement." : "Le coffre est réel, mais son contenu reste modeste.",
                "Argent gagné : " + Money::formatGoldWithRaw(gold),
                addExplorationMaterial(player, biome.commonMaterialId, quantity, chooseExplorationQuality(random, true))
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
            int baseGold = random.between(35 + player.getLevel() * 3, 90 + player.getLevel() * 8);
            baseGold = baseGold * chestOpeningGoldMultiplier(choice) / 100;
            int gold = applyExplorationGoldReward(std::max(1, baseGold), player, intensity, difficulty, 2);
            player.getInventory().earnGold(gold);
            const int quantity = std::max(1, applyExplorationQuantityBonus(1 + std::max(0, rewardQuantityBonus), intensity));
            std::vector<std::string> rewardLines = {
                alreadyUnlocked ? "Le coffre n'était pas fermé : le vrai gain vient de la fouille propre." : "Le coffre est réel, et pour une fois il n'a pas décidé de te mordre.",
                "Argent gagné : " + Money::formatGoldWithRaw(gold),
                addExplorationMaterial(player, biome.rareMaterialId, quantity, chooseExplorationQuality(random, true))
            };
            if (choice == 2 && (nimbleOrThief || craftSpecialist) && random.between(1, 100) <= 28)
            {
                rewardLines.push_back(addExplorationMaterial(player, biome.commonMaterialId, 1, chooseExplorationQuality(random, true)));
                rewardLines.push_back("Bonus méthode : l'ouverture propre révèle un petit double fond.");
            }
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
                    "Trace perçue : " + ExplorationBiomeFlavor::bossTrace(biome.name) + "."
                }
            );

            ExplorationBossUnlockResult bossTrace = tryUnlockExplorationBossVariation(player, random, false, biome.name);
            std::vector<std::string> registryLines = {
                bossTrace.line,
                "Nom : ???",
                "Statut : emplacement approximatif découvert par exploration rarissime."
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
                player.rememberNpcFact(
                    acceptedQuest.client.empty() ? std::string("Contact inconnu") : acceptedQuest.client,
                    "quest_accepted",
                    acceptedQuest.id,
                    "Demande acceptée : " + acceptedQuest.title,
                    "interaction_directe",
                    player.getName(),
                    100,
                    3
                );
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
        const bool giantSlimeEncounter = player.getLevel() >= 4 && random.between(1, 100) <= 12;
        bool evolved = !giantSlimeEncounter && random.between(1, 100) <= 45;
        std::string miniBossName = giantSlimeEncounter
            ? "Géant slime des quatre divisions"
            : ExplorationBiomeFlavor::miniBossName(biome.name, evolved);
        std::string questFamily = giantSlimeEncounter
            ? "Mini-boss / slime à divisions successives"
            : ExplorationBiomeFlavor::miniBossQuestFamily(biome.name, evolved);

        Monster miniBoss = giantSlimeEncounter
            ? MonsterCatalog::createGiantSlimeMiniBoss(std::max(4, player.getLevel() + random.between(0, 2)))
            : createExplorationEliteForBiome(player, random, biome, intensity);

        std::vector<std::string> introLines = {
            "L'air se tasse autour de toi.",
            "Mini-boss d'exploration : " + miniBossName + ".",
            "Forme rencontrée : " + miniBoss.getName() + " [niveau " + std::to_string(miniBoss.getLevel()) + "].",
            "Zone : " + biome.name + " | Approche : " + intensity.name + "."
        };

        if (giantSlimeEncounter)
        {
            introLines.push_back("Quatre noyaux sont emboîtés dans sa masse : Géant slime, Gros slime, Slime, Petit slime, puis Âme du slime.");
            introLines.push_back("Chaque sous-forme se divisera à son tour. Les dégâts restent modérés, mais la file ennemie risque de déborder.");
        }
        else if (evolved)
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
            addExplorationMaterial(
                player,
                giantSlimeEncounter ? "slime_residue" : (evolved ? biome.rareMaterialId : biome.commonMaterialId),
                giantSlimeEncounter ? applyExplorationQuantityBonus(random.between(4, 7), intensity) : applyExplorationQuantityBonus(1, intensity),
                chooseExplorationQuality(random, giantSlimeEncounter || evolved)
            )
        };
        if (giantSlimeEncounter)
        {
            rewardLines.push_back("Le noyau principal s'est dissous après quatre étages complets de division.");
            rewardLines.push_back("Les Âmes de slime finales ont été comptées comme des invocations ennemies.");
        }

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
                "Lieu repéré : " + ExplorationBiomeFlavor::dangerousSiteName(biome.name) + ".",
                ExplorationBiomeFlavor::dangerousSiteWarning(biome.name),
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
                "Description rapide : " + ExplorationBiomeFlavor::bossTrace(biome.name) + ",",
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

        ExplorationBossUnlockResult bossTrace = tryUnlockExplorationBossVariation(player, random, true, biome.name);
        bossLines.push_back(bossTrace.line);
        if (bossTrace.unlocked)
        {
            bossLines.push_back("Nom : ???");
            bossLines.push_back("Statut : entrée approximative mémorisée après une découverte dangereuse rarissime.");
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
        int eventRoll = chooseVariedActiveExplorationRoll(player, random);
        const std::string activeEventKey = activeExplorationEventKeyFromRoll(eventRoll);
        player.recordExplorationEventKey(activeEventKey);
        player.startExplorationSceneCooldown(activeEventKey, activeExplorationEventCooldownDays(activeEventKey));

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
                {"Obstacle de terrain : " + ExplorationBiomeFlavor::environmentalHazard(biome.name) + "."},
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
                std::vector<std::string> lines = {ExplorationBiomeFlavor::environmentalObservation(biome.name)};
                if (ExplorationLanguageTraceCatalog::hasTrace(biome.name))
                {
                    const ExplorationLanguageTrace trace = ExplorationLanguageTraceCatalog::forBiome(biome.name);
                    lines.push_back(ExplorationLanguageTraceCatalog::renderForPlayer(player, trace));
                    player.recordHistoricalEvent(
                        "exploration_language_trace",
                        biome.name + ":" + trace.languageId,
                        "Trace linguistique observée dans " + biome.name + " (" + trace.languageId + ")."
                    );
                }
                recordBiomeFieldObservation(biome, ExplorationBiomeFlavor::environmentalObservation(biome.name));
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
                recordBiomeFieldObservation(biome, ExplorationBiomeFlavor::environmentalObservation(biome.name));
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

namespace QuestExplorationSupport
{
    std::string randomBiomeForClient(Random& random, const std::string& clientName)
    {
        return ::randomBiomeForClient(random, clientName);
    }

    MicroChallengeResult runGuildServiceMicroChallenge(Quest& quest, Random& random)
    {
        return ::runGuildServiceMicroChallenge(quest, random);
    }

    Quest buildNpcQuestByRoll(Player& player, int roll, std::string& intro, const std::string& biomeName)
    {
        return ::buildNpcQuestByRoll(player, roll, intro, biomeName);
    }

    void displayQuestOffer(Player& player, const Quest& offeredQuest, const std::string& intro)
    {
        ::displayQuestOffer(player, offeredQuest, intro);
    }

    void simulateAfterCombatMiniBoss(Player& player, Random& random, DifficultyMode difficulty, DeathRuleMode deathRule)
    {
        ::simulateAfterCombatMiniBoss(player, random, difficulty, deathRule);
    }
}

void QuestMenu::openExploration(Player& player, DifficultyMode difficulty, DeathRuleMode deathRule)
{
    expireOverdueQuestDeadlines(player, "quest.exploration", false);
    openExplorationMenu(player, difficulty, deathRule);
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

    const auto storyBiomeUnlocked = [&](const ExplorationBiome& biome)
    {
        if (!player.hasStoryModeStarted() || player.hasStorySkip())
        {
            return true;
        }

        if (biome.name == "Plaine sauvage" || biome.name == "Route commerciale" || biome.name == "Forêt ancienne")
        {
            return true;
        }

        if (player.getStoryChapter() >= 2 && biome.name == "Mares gélatineuses")
        {
            return true;
        }

        if (player.getStoryChapter() >= 2 && player.getStoryStep() >= 9
            && (biome.name == "Bocage aux lanternes" || biome.name == "Quartier abandonné"))
        {
            return true;
        }

        if (player.getStoryChapter() >= 2 && player.getStoryStep() >= 12 && biome.name == "Mine sifflante")
        {
            return true;
        }

        return false;
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
        const City* explorationOriginCity = City::findById(player.getCurrentCityId());
        if (explorationOriginCity != nullptr)
        {
            screen.addLine("Ville de départ : " + explorationOriginCity->getName() + " — les distances vers les biomes dépendront progressivement de cette ville.");
        }

        bool hasEvolvedBiome = false;
        int hiddenBiomeCount = 0;
        int storyLockedBiomeCount = 0;
        int unknownRumorCount = 0;
        std::vector<int> visibleBiomeIndexes;

        for (int i = 0; i < static_cast<int>(biomes.size()); ++i)
        {
            const ExplorationBiome& biomePreview = biomes[i];

            if (!storyBiomeUnlocked(biomePreview))
            {
                storyLockedBiomeCount++;
                continue;
            }

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
        if (storyLockedBiomeCount > 0)
        {
            screen.addLine("Mode histoire : " + std::to_string(storyLockedBiomeCount) + " zones pas encore accessibles restent cachées.");
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
                    + (explorationOriginCity != nullptr && explorationOriginCity->getDistanceToBiome(biomePreview.name) >= 0
                        ? " / " + std::to_string(explorationOriginCity->getDistanceToBiome(biomePreview.name)) + " km depuis " + explorationOriginCity->getName()
                        : "")
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
        const int readyBeforeExploration = countReadyToTurnInQuests(player);

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
        const std::vector<std::string> livingBiomeLines = BiomeLivingContentCatalog::buildCurrentObservationLines(
            biome.name, player.getWorldDaysElapsed());
        entryLines.insert(entryLines.end(), livingBiomeLines.begin(), livingBiomeLines.end());

        const BiomeAmbientEvent ambientEvent = BiomeAmbientEventSystem::buildCurrentEvent(
            biome.name, player.getWorldDaysElapsed(), player.getWorldDayProgressUnits());
        if (ambientEvent.active)
        {
            const std::string ambientKey = BiomeAmbientEventSystem::journalKey(
                biome.name, player.getWorldDaysElapsed(), ambientEvent.id);
            bool alreadyObserved = false;
            for (const PlayerJournalRecord& record : player.getCanonicalJournalRecords())
            {
                if (record.category == "micro_evenements_biome" && record.key == ambientKey)
                {
                    alreadyObserved = true;
                    break;
                }
            }

            entryLines.push_back("--- " + ambientEvent.title + " ---");
            entryLines.insert(entryLines.end(), ambientEvent.lines.begin(), ambientEvent.lines.end());
            if (!alreadyObserved)
            {
                roll = std::clamp(roll + ambientEvent.explorationRollShift, 1, 100);
                player.recordCanonicalEvent(
                    "micro_evenements_biome",
                    ambientKey,
                    ambientEvent.title + " observé dans " + biome.name,
                    1
                );
                entryLines.push_back("Effet limité à cette lecture du terrain : ajustement d'exploration "
                    + std::string(ambientEvent.explorationRollShift >= 0 ? "+" : "")
                    + std::to_string(ambientEvent.explorationRollShift) + ".");
            }
            else
            {
                entryLines.push_back("Tu avais déjà exploité ce signe aujourd'hui : aucun second bonus/malus n'est créé en revenant le regarder.");
            }
        }
        const BiomeNonCombatInteraction nonCombatInteraction = BiomeNonCombatInteractionSystem::buildCurrentInteraction(
            biome.name, player.getWorldDaysElapsed(), player.getWorldDayProgressUnits());
        if (nonCombatInteraction.active)
        {
            const std::string interactionKey = BiomeNonCombatInteractionSystem::journalKey(
                biome.name, player.getWorldDaysElapsed(), nonCombatInteraction.id);
            bool interactionAlreadyResolved = false;
            for (const PlayerJournalRecord& record : player.getCanonicalJournalRecords())
            {
                if (record.category == "interactions_non_combat_biome" && record.key == interactionKey)
                {
                    interactionAlreadyResolved = true;
                    break;
                }
            }

            if (!interactionAlreadyResolved)
            {
                std::vector<std::string> interactionLines = nonCombatInteraction.contextLines;
                interactionLines.insert(interactionLines.begin(), nonCombatInteraction.prompt);
                interactionLines.push_back("Ce choix concerne ce qui est observable maintenant ; il ne révèle aucun événement futur.");

                std::vector<std::pair<int, std::string>> interactionOptions;
                for (const BiomeNonCombatChoice& interactionChoice : nonCombatInteraction.choices)
                {
                    interactionOptions.push_back({interactionChoice.id, interactionChoice.label + " — " + interactionChoice.description});
                }

                const int interactionChoice = askChoiceScreen(
                    nonCombatInteraction.title,
                    "exploration.non_combat_interaction",
                    interactionLines,
                    interactionOptions,
                    1,
                    static_cast<int>(interactionOptions.size())
                );
                Console::clear();

                const BiomeNonCombatInteractionResult interactionResult = BiomeNonCombatInteractionSystem::resolve(
                    nonCombatInteraction, interactionChoice);
                if (interactionResult.resolved)
                {
                    roll = std::clamp(roll + interactionResult.explorationRollShift, 1, 100);
                    std::vector<std::string> resultLines = interactionResult.lines;
                    resultLines.push_back("Choix retenu : " + interactionResult.choiceLabel + ".");
                    if (interactionResult.explorationRollShift != 0)
                    {
                        resultLines.push_back("Lecture du terrain modifiée pour cette sortie : "
                            + std::string(interactionResult.explorationRollShift > 0 ? "+" : "")
                            + std::to_string(interactionResult.explorationRollShift) + ".");
                    }
                    if (interactionResult.questProgress > 0)
                    {
                        const int updated = progressExplorationQuests(player, biome.name, interactionResult.questProgress);
                        if (updated > 0)
                        {
                            resultLines.push_back("Cette interaction donne aussi une vraie information utile à une ou plusieurs quêtes d'exploration.");
                        }
                    }

                    player.recordCanonicalEvent(
                        "interactions_non_combat_biome",
                        interactionKey,
                        nonCombatInteraction.title + " — " + interactionResult.choiceLabel,
                        1
                    );
                    recordBiomeFieldObservation(
                        biome,
                        "Interaction locale : " + nonCombatInteraction.title + " / " + interactionResult.choiceLabel + "."
                    );
                    showExplorationNotice(
                        "INTERACTION LOCALE",
                        "exploration.non_combat_interaction.result",
                        resultLines
                    );
                }
            }
            else
            {
                entryLines.push_back("Interaction locale déjà traitée aujourd'hui : tu ne peux pas la rouvrir pour farmer son effet.");
            }
        }

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

        if (player.hasStoryModeStarted() && player.getStoryChapter() >= 3)
        {
            const std::vector<std::string> consequenceLines = StoryCampaign::buildChapterThreeConsequenceLines(player);
            if (!consequenceLines.empty())
            {
                entryLines.push_back("Conséquence d'histoire active : " + consequenceLines.front());
            }
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

        maybeTriggerLegendaryMerchantEncounter(player, random, biome.name);

        MicroChallengeResult microChallenge = runExplorationMicroChallenge(player, biome, intensity, random);
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
        std::set<std::string> currentRunEventKeys;
        auto runExplorationEvent = [&](int eventRoll, int eventIndex) {
            eventRoll = std::clamp(eventRoll, 1, 100);
            const std::string eventKey = explorationEventKeyFromRoll(eventRoll);
            const std::string eventLabel = explorationEventLabelFromRoll(eventRoll);
            currentRunEventKeys.insert(eventKey);
            player.recordExplorationEventKey(eventKey);
            player.startExplorationSceneCooldown(eventKey, explorationEventCooldownDays(eventKey));
            if (isMainMiniBossExplorationKey(eventKey))
            {
                player.startExplorationSceneCooldown("main_miniboss_wanderer", explorationEventCooldownDays(eventKey));
                player.startExplorationSceneCooldown("main_miniboss_guardian", explorationEventCooldownDays(eventKey));
            }
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
                std::vector<std::string> lines;
                int quantity = 1;
                bool favorQuality = carefulRecovery;
                if (eventKey == "main_gather_exposed")
                {
                    lines.push_back("Une ressource utile pousse à découvert, visible depuis le passage.");
                    quantity = random.between(1, 2);
                }
                else if (eventKey == "main_gather_hidden")
                {
                    lines.push_back("Des marques discrètes conduisent vers une petite poche de ressources cachée sous le terrain.");
                    quantity = random.between(1, 2);
                    if (random.between(1, 100) <= 24)
                    {
                        lines.push_back(addExplorationMaterial(player, biome.rareMaterialId, 1, chooseExplorationQuality(random, true)));
                    }
                }
                else
                {
                    lines.push_back("La ressource est fragile : une récolte brutale la rendrait presque inutile.");
                    quantity = 1;
                    favorQuality = true;
                }
                if (carefulRecovery)
                {
                    lines.push_back("Récolte propre : ta préparation évite de gaspiller la trouvaille.");
                }
                lines.push_back(addExplorationMaterial(player, biome.commonMaterialId, applyExplorationQuantityBonus(quantity, intensity), chooseExplorationQuality(random, favorQuality)));
                showExplorationNotice("RÉCOLTE", "exploration.run.gather." + eventKey, lines);
            }
            else if (eventRoll <= 40)
            {
                std::vector<std::string> lines;
                int progress = 1;
                if (eventKey == "main_trace_broken")
                {
                    lines.push_back("Une piste s'interrompt net, comme si ce qui laissait les traces avait changé de direction sans tourner.");
                    progress = random.between(1, 2);
                }
                else if (eventKey == "main_trace_passage")
                {
                    lines.push_back("Plusieurs marques de passage se superposent. Elles ne racontent pas la même heure ni le même groupe.");
                    recordBiomeFieldObservation(biome, "Marques superposées observées : plusieurs passages récents traversent " + biome.name + ".");
                    progress = 2;
                }
                else
                {
                    lines.push_back("Tu retrouves un objet perdu, trop abîmé pour être vendu mais assez précis pour indiquer d'où venait son propriétaire.");
                    progress = 1;
                    if (random.between(1, 100) <= 40)
                    {
                        int recoveredGold = applyExplorationGoldReward(random.between(2, 10 + player.getLevel()), player, intensity, difficulty, 0);
                        player.getInventory().earnGold(recoveredGold);
                        lines.push_back("Quelques pièces encore valables restent coincées dedans : " + Money::formatGoldWithRaw(recoveredGold) + ".");
                    }
                }
                int updated = progressExplorationQuests(player, biome.name, progress);
                if (updated > 0)
                {
                    lines.push_back("Des quêtes d'exploration progressent grâce à cette découverte.");
                }
                else
                {
                    lines.push_back("Tu conserves l'information dans tes notes, même si aucune quête actuelle ne l'exploite.");
                }
                showExplorationNotice("TRACE INTÉRESSANTE", "exploration.run.trace." + eventKey, lines);
            }
            else if (eventRoll <= 52)
            {
                const bool oldDeposit = eventKey == "main_treasure_deposit";
                int gold = applyExplorationGoldReward(
                    oldDeposit ? random.between(8, 28 + player.getLevel() * 2) : random.between(5, 20 + player.getLevel()),
                    player,
                    intensity,
                    difficulty,
                    1
                );
                player.getInventory().earnGold(gold);
                std::vector<std::string> lines = {
                    oldDeposit
                        ? "Tu découvres un petit dépôt ancien, protégé par une pierre plate et beaucoup de poussière."
                        : "Une bourse oubliée a glissé hors du passage principal.",
                    "Argent gagné : " + Money::formatGoldWithRaw(gold)
                };
                if (oldDeposit && random.between(1, 100) <= 55)
                {
                    lines.push_back(addExplorationMaterial(player, biome.commonMaterialId, 1, chooseExplorationQuality(random, true)));
                }
                showExplorationNotice("PETIT TRÉSOR", "exploration.run.gold." + eventKey, lines);
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

            const int repeatRerollShift = eventIndex > 1
                ? 4 + nightRollShift / 2 + std::max(0, temperatureRollShift / 2)
                : 0;
            if (eventIndex > 1)
            {
                eventRoll = std::clamp(eventRoll + repeatRerollShift, 1, 100);
            }

            eventRoll = applyChapterThreeExplorationChoiceBias(player, random, eventRoll);
            eventRoll = chooseVariedMainExplorationRoll(
                player,
                random,
                eventRoll,
                intensity,
                repeatRerollShift,
                currentRunEventKeys
            );
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
