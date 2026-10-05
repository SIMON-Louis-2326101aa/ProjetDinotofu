// EN: Enchantment, disenchantment, safety-seal and rune-transfer services split from ShopMenu.cpp.
// FR: Enchantement, désenchantement, sceaux de sécurité et transfert de runes extraits de ShopMenu.cpp.
#include "interface/menu/shop/EnchanterServiceMenu.hpp"
#include "interface/menu/shop/ShopServiceSupport.hpp"
#include "entity/Player.hpp"
#include "core/Console.hpp"
#include "combat/modes/pve/MonsterPveMode.hpp"
#include "combat/system/CombatClassSystem.hpp"
#include "entity/Monster.hpp"
#include "interface/menu/EquipmentMenu.hpp"
#include "interface/menu/progression/LanguagePracticeMenu.hpp"
#include "item/weapon/WeaponCatalog.hpp"
#include "item/armor/ArmorCatalog.hpp"
#include "item/consumable/ConsumableCatalog.hpp"
#include "economy/shop/ShopCatalog.hpp"
#include "economy/shop/ShopItemCategory.hpp"
#include "economy/shop/ShopPriceRules.hpp"
#include "economy/shop/ShopRotationSystem.hpp"
#include "economy/shop/ShopTransactionSystem.hpp"
#include "economy/Money.hpp"
#include "interface/menu/quest/QuestMenu.hpp"
#include "interface/menu/common/MessageScreen.hpp"
#include "interface/menu/common/PagedMenu.hpp"
#include "interface/TerminalInterface.hpp"
#include "interface/model/MenuScreen.hpp"
#include "lore/LegendTriggerSystem.hpp"
#include "item/material/MaterialCatalog.hpp"
#include "item/material/Material.hpp"
#include "quest/Quest.hpp"
#include "progression/bestiary/BestiaryRuntimeProgress.hpp"
#include "story/StoryCampaign.hpp"
#include "world/LocalReputationSystem.hpp"
#include "world/npc/NpcKnowledgeSystem.hpp"
#include "world/npc/NpcInformationPropagationSystem.hpp"
#include <algorithm>
#include <cctype>
#include <iostream>
#include <random>
#include <string>
#include <vector>
#include <cstdint>

namespace
{
    using ShopServiceSupport::askShopConfirmation;
    using ShopServiceSupport::payServiceWithVoucherOrGold;
    using ShopServiceSupport::showLocalServiceResult;
    using ShopServiceSupport::showShopResult;

    struct EnchantmentOffer
    {
        std::string id;
        std::string label;
        std::string effectLabel;
        int price = 0;
        std::string materialId;
        std::string materialName;
        int materialQuantity = 0;
        std::string description;
        int riskModifier = 0;
    };

    std::vector<EnchantmentOffer> getEnchantmentOffers()
    {
        return {
            {"minor_fire_ward", "Rune mineure anti-feu", "Rune anti-feu mineure", 95, "arcane_dust", "Poussière arcanique", 2, "Aide contre les brûlures, les attaques de feu et les zones chaudes non extrêmes."},
            {"minor_cold_ward", "Rune mineure anti-froid", "Rune anti-froid mineure", 90, "mountain_blue_flower", "Fleur bleue de montagne", 2, "Aide contre le givre, le froid et les biomes glacés modérés."},
            {"thermal_balance", "Charme d'équilibre thermique", "Charme d'équilibre thermique", 140, "arcane_dust", "Poussière arcanique", 3, "Protection générale mais moins spécialisée que les runes dédiées."},
            {"draconic_heat_trace", "Trace chaude draconique", "Trace chaude draconique", 180, "draconic_scale_fragment", "Fragment d'écaille draconique", 1, "Plus chère, plus stable sur une bonne pièce, utile contre feu/chaleur."},
            {"rare_fire_ward", "Rune renforcée anti-feu", "Rune anti-feu renforcée", 260, "rare_fire_rune_core", "Cœur de rune ignifuge", 1, "Plus forte contre feu/chaleur/brûlures, mais plus difficile à stabiliser.", 8},
            {"rare_cold_ward", "Rune renforcée anti-froid", "Rune anti-froid renforcée", 250, "rare_cold_rune_core", "Cœur de rune antigel", 1, "Plus forte contre froid/givre et zones glacées avancées.", 6},
            {"stabilizing_bind", "Sceau de stabilisation", "Sceau de stabilisation runique", 210, "runic_stabilizer", "Stabilisateur runique", 1, "N'ajoute pas la meilleure résistance, mais réduit nettement le risque d'un futur équipement trop chargé.", -14}
        };
    }

    int equipmentQualityRiskPenalty(const std::string& name, int value, int maxDurability)
    {
        std::string probe = name;
        std::transform(probe.begin(), probe.end(), probe.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });

        int penalty = 0;
        if (value < 25) penalty += 12;
        if (maxDurability >= 0 && maxDurability < 65) penalty += 14;
        if (probe.find("rouill") != std::string::npos || probe.find("cass") != std::string::npos || probe.find("bois d'urgence") != std::string::npos) penalty += 14;
        if (probe.find("mains nues") != std::string::npos) penalty += 100;
        if (probe.find("exception") != std::string::npos || probe.find("particular") != std::string::npos || probe.find("runique") != std::string::npos) penalty -= 6;
        return penalty;
    }

    int enchantmentBreakRiskPercent(int currentEnchantments, int qualityPenalty, bool armorTarget)
    {
        int risk = armorTarget ? 2 : 3;

        if (currentEnchantments <= 0)
        {
            risk += qualityPenalty;
        }
        else if (currentEnchantments == 1)
        {
            risk += 10 + qualityPenalty / 2;
        }
        else if (currentEnchantments == 2)
        {
            risk += 22 + qualityPenalty / 2;
        }
        else if (currentEnchantments == 3)
        {
            risk += 38 + qualityPenalty / 2;
        }
        else if (currentEnchantments == 4)
        {
            risk += 58 + qualityPenalty / 2;
        }
        else
        {
            risk += 82 + (currentEnchantments - 5) * 6 + qualityPenalty / 2;
        }

        return std::max(1, std::min(95, risk));
    }

    std::string enchantmentRiskWarningText(int currentEnchantments)
    {
        if (currentEnchantments < 4)
        {
            return "Note : sous 5 enchantements, l'objet reste raisonnablement travaillable s'il est de bonne qualité.";
        }

        if (currentEnchantments == 4)
        {
            return "Note : le 5e enchantement approche de la limite raisonnable. Au-delà, la magie devient vraiment dure à stabiliser.";
        }

        return "Note : plus de 5 enchantements, c'est de l'acharnement runique. L'objet peut tenir, mais le risque devient violent.";
    }

    void grantEnchantmentBreakSalvage(Player& player, bool armorTarget, int itemValue, int enchantmentCount, std::vector<std::string>& lines)
    {
        const int safeValue = std::max(1, itemValue);
        const int metalAmount = std::max(1, std::min(4, 1 + safeValue / 180 + enchantmentCount / 3));
        const int arcaneAmount = std::max(1, std::min(5, 1 + enchantmentCount));

        player.getInventory().addMaterial(MaterialCatalog::createById("rusted_metal_fragment", metalAmount));
        player.getInventory().addMaterial(MaterialCatalog::createById("arcane_dust", arcaneAmount));

        lines.push_back("Récupération minimale : Fragment de métal rouillé x" + std::to_string(metalAmount) + ".");
        lines.push_back("Récupération arcanique : Poussière arcanique x" + std::to_string(arcaneAmount) + ".");

        if (armorTarget)
        {
            const int leatherAmount = std::max(1, std::min(3, 1 + safeValue / 220));
            player.getInventory().addMaterial(MaterialCatalog::createById("worn_leather_piece", leatherAmount));
            lines.push_back("Restes d'armure : Morceau de cuir abîmé x" + std::to_string(leatherAmount) + ".");
        }
        else if (enchantmentCount >= 3)
        {
            player.getInventory().addMaterial(MaterialCatalog::createById("runic_iron_shard", 1));
            lines.push_back("Éclat stabilisé récupéré : Éclat de fer runique x1.");
        }
    }

    void maybeApplyRunicBacklashCurse(Player& player, int destroyedEnchantments, int riskPercent, const std::string& destroyedItemName, std::vector<std::string>& lines)
    {
        static std::mt19937 generator(std::random_device{}());
        const int chancePercent = std::max(15, std::min(70, 25 + destroyedEnchantments * 8 + riskPercent / 5));
        std::uniform_int_distribution<int> distribution(1, 100);
        if (distribution(generator) > chancePercent)
        {
            lines.push_back("Contrecoup évité : la rupture runique n'a pas réussi à s'accrocher à ton corps.");
            return;
        }

        const int level = std::max(1, std::min(3, 1 + destroyedEnchantments / 2 + (riskPercent >= 55 ? 1 : 0)));
        PlayerCurse curse;
        curse.id = "runic_backlash";
        curse.name = "Contrecoup runique niv." + std::to_string(level);
        curse.severity = level >= 3 ? "majeure" : (level == 2 ? "moyenne" : "mineure");
        curse.origin = "Échec d'enchantement sur " + destroyedItemName;
        curse.description = "Une partie de la rune brisée s'est accrochée au personnage. Les effets exacts restent volontairement à diagnostiquer.";
        curse.removalHint = "faire diagnostiquer la trace, puis demander un exorcisme à l'église.";
        curse.symptomCategories = level >= 3 ? "mana,equipment,health,spirit" : "mana,equipment,health";
        curse.discoveredSymptomCategories = "";
        curse.excludedSymptomCategories = "";
        curse.diagnosisLevel = 0;
        curse.appliedAtDay = player.getWorldDaysElapsed();
        curse.expiresAtDay = player.getWorldDaysElapsed() + 2 + level * 2;
        curse.exorcismProgress = 0;
        curse.exorcismRequiredVisits = level >= 3 ? 3 : (level == 2 ? 2 : 1);
        curse.curseLevel = level;
        curse.maxCurseLevel = level >= 3 ? 4 : (level == 2 ? 3 : 1);
        curse.evolvesOverTime = level >= 2;
        curse.escalationIntervalDays = level >= 3 ? 2 : 3;
        curse.nextEscalationDay = curse.evolvesOverTime ? player.getWorldDaysElapsed() + curse.escalationIntervalDays : -1;
        curse.churchRemovalMaxLevel = 3;
        curse.becomesSpecialRemovalWhenTooHigh = true;
        curse.highLevelRemovalHint = "stabiliser la rune maudite chez un enchanteur, puis accomplir un rite total à l'église.";
        curse.removableByChurch = true;
        curse.bossIdRequiredToBreak = 0;
        curse.lifeLong = false;
        player.addOrRefreshCurse(curse);

        lines.push_back("Une trace inconnue s'accroche au personnage après la rupture runique.");
        if (curse.evolvesOverTime)
        {
            lines.push_back("Instabilité : cette trace peut empirer avec les jours si elle n'est pas traitée assez vite.");
        }
        lines.push_back("Statut : ????? — l'église devra la diagnostiquer pour savoir ce que c'est réellement.");
    }

    bool consumeEnchantmentCost(Player& player, const EnchantmentOffer& offer, std::vector<std::string>& lines)
    {
        if (player.getInventory().countMaterialById(offer.materialId) < offer.materialQuantity)
        {
            lines.push_back("Composant manquant : " + offer.materialName + " x" + std::to_string(offer.materialQuantity) + ".");
            return false;
        }

        if (!player.getInventory().spendEconomyUnits(offer.price))
        {
            lines.push_back("Paiement refusé : il faut " + Money::formatEconomyUnits(offer.price) + ".");
            return false;
        }

        if (!player.getInventory().removeMaterialQuantityById(offer.materialId, offer.materialQuantity))
        {
            lines.push_back("Composant introuvable au moment de graver la rune. L'or n'a pas pu être récupéré automatiquement.");
            return false;
        }

        lines.push_back("Coût payé : " + Money::formatEconomyUnits(offer.price) + " + " + offer.materialName + " x" + std::to_string(offer.materialQuantity) + ".");
        return true;
    }

    int askEnchantmentOfferChoice(const std::vector<EnchantmentOffer>& offers)
    {
        MenuScreen screen("CHOIX DE RUNE", "shop.enchanter.offer");
        screen.addLine("Choisis l'effet à tenter. Chaque enchantement supplémentaire augmente le risque de casse définitive.");
        screen.addOption(0, "Retour", "Revenir au service de l'enchanteur.", true, "shop.enchanter.offer.back");
        for (std::size_t i = 0; i < offers.size(); ++i)
        {
            const EnchantmentOffer& offer = offers[i];
            screen.addOption(
                static_cast<int>(i + 1),
                offer.label + " | " + Money::formatEconomyUnits(offer.price) + " + " + offer.materialName + " x" + std::to_string(offer.materialQuantity),
                offer.description,
                true,
                "shop.enchanter.offer." + std::to_string(i + 1)
            );
        }
        Console::clear();
        return TerminalInterface::askMenuChoiceFromOptions(screen, "Choisis une rune, ou 0 pour revenir.");
    }

    bool rollEnchantmentBreak(int riskPercent)
    {
        static std::mt19937 generator(std::random_device{}());
        std::uniform_int_distribution<int> distribution(1, 100);
        return distribution(generator) <= riskPercent;
    }

    bool consumeRunicSafetySeal(Player& player, const std::string& itemName, std::vector<std::string>& lines)
    {
        if (!player.getInventory().removeMaterialQuantityById("runic_safety_seal", 1))
        {
            return false;
        }

        lines.push_back("Sceau anti-casse : le sceau se brise à la place de l'équipement.");
        lines.push_back("Objet sauvé : " + itemName + ". La rune tentée n'est pas gravée, mais l'objet n'est pas perdu.");
        lines.push_back("Limite : protection consommée. Le prochain échec critique sera de nouveau dangereux sans nouveau sceau.");
        return true;
    }

    void enchantWeapon(Player& player, int weaponIndex, const EnchantmentOffer& offer)
    {
        Weapon* weapon = player.getInventory().getMutableWeapon(weaponIndex);
        if (weapon == nullptr)
        {
            showShopResult("ENCHANTEMENT IMPOSSIBLE", "shop.enchanter.weapon.missing", {"Arme introuvable."});
            return;
        }

        if (weapon->getName() == "Mains nues")
        {
            showShopResult("ENCHANTEMENT REFUSÉ", "shop.enchanter.weapon.bare_hands", {"L'enchanteur regarde tes mains.", "Verdict : on ne grave pas une rune sur des phalanges. Enfin, pas dans cette boutique."});
            return;
        }

        const int risk = std::max(1, std::min(95, enchantmentBreakRiskPercent(
            weapon->getEnchantmentCount(),
            equipmentQualityRiskPenalty(weapon->getName(), weapon->getValue(), weapon->getMaxDurability()),
            false
        ) + offer.riskModifier));

        const bool confirmed = askShopConfirmation(
            "CONFIRMER L'ENCHANTEMENT",
            "shop.enchanter.weapon.confirm",
            {
                "Arme : " + weapon->getName(),
                "Enchantements actuels : " + std::to_string(weapon->getEnchantmentCount()) + " — " + weapon->getEnchantmentSummaryText(),
                "Rune : " + offer.label,
                "Coût : " + Money::formatEconomyUnits(offer.price) + " + " + offer.materialName + " x" + std::to_string(offer.materialQuantity),
                "Risque de casse définitive : " + std::to_string(risk) + "%.",
                weapon->getEnchantmentCount() == 0
                    ? "Note : premier enchantement très stable sur une bonne arme, mais les armes nulles restent dangereuses."
                    : enchantmentRiskWarningText(weapon->getEnchantmentCount())
            },
            "Tenter l'enchantement",
            "Annuler",
            "shop.enchanter.weapon"
        );

        if (!confirmed)
        {
            showShopResult("ENCHANTEMENT ANNULÉ", "shop.enchanter.weapon.cancelled", {"Aucune rune n'a été gravée."});
            return;
        }

        std::vector<std::string> lines;
        if (!consumeEnchantmentCost(player, offer, lines))
        {
            showShopResult("ENCHANTEMENT REFUSÉ", "shop.enchanter.weapon.failed_cost", lines);
            return;
        }

        const std::string weaponName = weapon->getName();
        const int weaponValue = weapon->getValue();
        const int weaponEnchantments = weapon->getEnchantmentCount();
        if (rollEnchantmentBreak(risk))
        {
            lines.push_back("Échec critique : les runes se mordent entre elles.");
            if (consumeRunicSafetySeal(player, weaponName, lines))
            {
                showShopResult("SCEAU ANTI-CASSE BRISÉ", "shop.enchanter.weapon.safety_seal", lines);
                return;
            }
            if (player.getEquippedWeaponIndex() >= weaponIndex)
            {
                player.unequipWeapon();
            }
            player.getInventory().removeWeapon(weaponIndex);
            lines.push_back("Arme brisée définitivement : " + weaponName + ".");
            lines.push_back("Réparation impossible : la structure de l'arme est détruite.");
            grantEnchantmentBreakSalvage(player, false, weaponValue, weaponEnchantments, lines);
            maybeApplyRunicBacklashCurse(player, weaponEnchantments, risk, weaponName, lines);
            showShopResult("ARME DÉTRUITE", "shop.enchanter.weapon.destroyed", lines);
            return;
        }

        weapon = player.getInventory().getMutableWeapon(weaponIndex);
        if (weapon != nullptr)
        {
            weapon->addEnchantment(offer.effectLabel);
            lines.push_back("Enchantement réussi : " + offer.effectLabel + " gravé sur " + weapon->getName() + ".");
            lines.push_back("Nouveau total : " + std::to_string(weapon->getEnchantmentCount()) + " enchantement(s).");
            lines.push_back("Effet : l'équipement est maintenant reconnu par les règles de résistance combat/exploration via son résumé runique.");
        }
        showShopResult("ENCHANTEMENT RÉUSSI", "shop.enchanter.weapon.success", lines);
    }

    void enchantArmor(Player& player, int armorIndex, const EnchantmentOffer& offer)
    {
        Armor* armor = player.getInventory().getMutableArmor(armorIndex);
        if (armor == nullptr)
        {
            showShopResult("ENCHANTEMENT IMPOSSIBLE", "shop.enchanter.armor.missing", {"Armure introuvable."});
            return;
        }

        const int risk = std::max(1, std::min(95, enchantmentBreakRiskPercent(
            armor->getEnchantmentCount(),
            equipmentQualityRiskPenalty(armor->getName(), armor->getValue(), armor->getMaxDurability()),
            true
        ) + offer.riskModifier));

        const bool confirmed = askShopConfirmation(
            "CONFIRMER L'ENCHANTEMENT",
            "shop.enchanter.armor.confirm",
            {
                "Armure : " + armor->getName(),
                "Enchantements actuels : " + std::to_string(armor->getEnchantmentCount()) + " — " + armor->getEnchantmentSummaryText(),
                "Rune : " + offer.label,
                "Coût : " + Money::formatEconomyUnits(offer.price) + " + " + offer.materialName + " x" + std::to_string(offer.materialQuantity),
                "Risque de casse définitive : " + std::to_string(risk) + "%.",
                armor->getEnchantmentCount() == 0
                    ? "Note : premier enchantement très stable sur une bonne armure, mais les tenues trop faibles peuvent céder."
                    : enchantmentRiskWarningText(armor->getEnchantmentCount())
            },
            "Tenter l'enchantement",
            "Annuler",
            "shop.enchanter.armor"
        );

        if (!confirmed)
        {
            showShopResult("ENCHANTEMENT ANNULÉ", "shop.enchanter.armor.cancelled", {"Aucune rune n'a été gravée."});
            return;
        }

        std::vector<std::string> lines;
        if (!consumeEnchantmentCost(player, offer, lines))
        {
            showShopResult("ENCHANTEMENT REFUSÉ", "shop.enchanter.armor.failed_cost", lines);
            return;
        }

        const std::string armorName = armor->getName();
        const int armorValue = armor->getValue();
        const int armorEnchantments = armor->getEnchantmentCount();
        if (rollEnchantmentBreak(risk))
        {
            lines.push_back("Échec critique : la trame de l'armure se fend et refuse toute réparation.");
            if (consumeRunicSafetySeal(player, armorName, lines))
            {
                showShopResult("SCEAU ANTI-CASSE BRISÉ", "shop.enchanter.armor.safety_seal", lines);
                return;
            }
            if (player.getEquippedArmorIndex() >= armorIndex)
            {
                player.unequipArmor();
            }
            player.getInventory().removeArmor(armorIndex);
            lines.push_back("Armure détruite définitivement : " + armorName + ".");
            lines.push_back("Réparation impossible : l'enchantement a brûlé les attaches et les coutures internes.");
            grantEnchantmentBreakSalvage(player, true, armorValue, armorEnchantments, lines);
            maybeApplyRunicBacklashCurse(player, armorEnchantments, risk, armorName, lines);
            showShopResult("ARMURE DÉTRUITE", "shop.enchanter.armor.destroyed", lines);
            return;
        }

        armor = player.getInventory().getMutableArmor(armorIndex);
        if (armor != nullptr)
        {
            armor->addEnchantment(offer.effectLabel);
            lines.push_back("Enchantement réussi : " + offer.effectLabel + " gravé sur " + armor->getName() + ".");
            lines.push_back("Nouveau total : " + std::to_string(armor->getEnchantmentCount()) + " enchantement(s).");
            lines.push_back("Effet : l’armure compte dans les protections de température et les affinités élémentaires.");
        }
        showShopResult("ENCHANTEMENT RÉUSSI", "shop.enchanter.armor.success", lines);
    }

    void stabilizeRunicBacklashAtEnchanter(Player& player)
    {
        std::vector<std::string> lines;
        if (!player.hasHighRunicBacklashNeedingEnchanter())
        {
            showShopResult(
                "STABILISATION INUTILE",
                "shop.enchanter.runic_backlash.none",
                {
                    "L'enchanteur observe les mains du personnage.",
                    "Verdict : aucun contrecoup runique assez grave ne demande son atelier pour l'instant.",
                    "Les petites malédictions restent plutôt le travail de Sœur Maëlys."
                }
            );
            return;
        }

        if (!payServiceWithVoucherOrGold(player, "runic_stabilizer", "Stabilisateur runique", 240, lines))
        {
            showShopResult("STABILISATION REFUSÉE", "shop.enchanter.runic_backlash.failed_cost", lines);
            return;
        }

        if (!player.stabilizeHighRunicBacklashForEnchanter())
        {
            lines.push_back("La rune ne répond pas comme prévu. Rien n'a été stabilisé.");
            showShopResult("STABILISATION ÉCHOUÉE", "shop.enchanter.runic_backlash.invalid", lines);
            return;
        }

        lines.push_back("L'enchanteur ne retire pas la malédiction : il empêche seulement la rune de s'enfoncer plus loin.");
        lines.push_back("Résultat : l'église peut reprendre le relais, mais il faudra un rite total/progressif.");
        lines.push_back("Phrase de l'enchanteur : Je ne bénis pas. Je rends juste la blessure lisible pour quelqu'un qui sait prier.");
        showLocalServiceResult("CONTRECOUP STABILISÉ", "shop.enchanter.runic_backlash.success", player, lines, 1);
    }


    void inspectRunicOverload(Player& player)
    {
        std::vector<std::string> lines;
        lines.push_back("L'enchanteur pose les outils sans toucher aux runes : ici, on regarde avant de graver.");
        const std::vector<Weapon>& weapons = player.getInventory().getWeapons();
        for (std::size_t i = 0; i < weapons.size(); ++i)
        {
            const Weapon& weapon = weapons[i];
            if (weapon.getEnchantmentCount() <= 0) continue;
            const int risk = enchantmentBreakRiskPercent(weapon.getEnchantmentCount(), equipmentQualityRiskPenalty(weapon.getName(), weapon.getValue(), weapon.getMaxDurability()), false);
            lines.push_back("Arme : " + weapon.getName() + " | runes " + std::to_string(weapon.getEnchantmentCount()) + " | prochain risque estimé " + std::to_string(risk) + "%.");
        }
        const std::vector<Armor>& armors = player.getInventory().getArmors();
        for (std::size_t i = 0; i < armors.size(); ++i)
        {
            const Armor& armor = armors[i];
            if (armor.getEnchantmentCount() <= 0) continue;
            const int risk = enchantmentBreakRiskPercent(armor.getEnchantmentCount(), equipmentQualityRiskPenalty(armor.getName(), armor.getValue(), armor.getMaxDurability()), true);
            lines.push_back("Armure : " + armor.getName() + " | runes " + std::to_string(armor.getEnchantmentCount()) + " | prochain risque estimé " + std::to_string(risk) + "%.");
        }
        if (lines.size() <= 1)
        {
            lines.push_back("Aucun équipement enchanté à analyser pour le moment.");
        }
        lines.push_back("Conseil : un Sceau de stabilisation ne rend pas l'objet invincible, mais réduit nettement le danger du prochain empilement.");
        showShopResult("LECTURE RUNIQUE", "shop.enchanter.overload_reading", lines);
    }

    bool payDisenchantmentCost(Player& player, std::vector<std::string>& lines)
    {
        const int price = 70;
        const int dustCost = 2;
        if (player.getInventory().countMaterialById("arcane_dust") < dustCost)
        {
            lines.push_back("Composant manquant : Poussière arcanique x" + std::to_string(dustCost) + " requis.");
            return false;
        }
        if (!player.getInventory().spendEconomyUnits(price))
        {
            lines.push_back("Paiement refusé : il faut " + Money::formatEconomyUnits(price) + ".");
            lines.push_back("Argent disponible : " + player.getInventory().getWalletLine() + ".");
            return false;
        }
        player.getInventory().removeMaterialQuantityById("arcane_dust", dustCost);
        lines.push_back("Coût payé : " + Money::formatEconomyUnits(price) + " + Poussière arcanique x" + std::to_string(dustCost) + ".");
        return true;
    }

    void openDisenchantmentMenu(Player& player)
    {
        MenuScreen typeScreen("DÉSENCHANTEMENT PRUDENT", "shop.enchanter.disenchant.type");
        typeScreen.addLine("L'enchanteur peut retirer seulement la dernière rune gravée. Ce n'est pas un transfert gratuit ni une annulation parfaite.");
        typeScreen.addLine("But : sauver une pièce trop chargée avant de retenter autre chose, au prix de temps, or et poussière.");
        typeScreen.addOption(0, "Retour", "Revenir à l'atelier.", true, "shop.enchanter.disenchant.back");
        typeScreen.addOption(1, "Désenchanter une arme", "Retire la dernière rune d'une arme enchantée.", player.getInventory().getWeaponCount() > 0, "shop.enchanter.disenchant.weapon");
        typeScreen.addOption(2, "Désenchanter une armure", "Retire la dernière rune d'une armure enchantée.", player.getInventory().getArmorCount() > 0, "shop.enchanter.disenchant.armor");
        Console::clear();
        const int typeChoice = TerminalInterface::askMenuChoiceFromOptions(typeScreen, "Choisis le type d'objet.");
        if (typeChoice == 0) return;

        if (typeChoice == 1)
        {
            MenuScreen weaponScreen("ARME À DÉSENCHANTER", "shop.enchanter.disenchant.weapon.list");
            weaponScreen.addLine("Seules les armes avec au moins une rune peuvent être choisies.");
            weaponScreen.addOption(0, "Retour", "Revenir au choix.", true, "shop.enchanter.disenchant.weapon.back");
            const std::vector<Weapon>& weapons = player.getInventory().getWeapons();
            for (std::size_t i = 0; i < weapons.size(); ++i)
            {
                const Weapon& weapon = weapons[i];
                weaponScreen.addOption(static_cast<int>(i + 1), weapon.getName() + " | runes " + std::to_string(weapon.getEnchantmentCount()), weapon.getEnchantmentSummaryText(), weapon.getEnchantmentCount() > 0, "shop.enchanter.disenchant.weapon." + std::to_string(i));
            }
            Console::clear();
            const int weaponChoice = TerminalInterface::askMenuChoiceFromOptions(weaponScreen, "Choisis une arme.");
            if (weaponChoice <= 0 || weaponChoice > static_cast<int>(weapons.size())) return;
            Weapon* weapon = player.getInventory().getMutableWeapon(weaponChoice - 1);
            if (weapon == nullptr || weapon->getEnchantmentCount() <= 0)
            {
                showShopResult("DÉSENCHANTEMENT IMPOSSIBLE", "shop.enchanter.disenchant.weapon.invalid", {"Cette arme ne porte aucune rune retirable."});
                return;
            }
            std::vector<std::string> lines;
            const std::string weaponName = weapon->getName();
            const std::string before = weapon->getEnchantmentSummaryText();
            if (!payDisenchantmentCost(player, lines))
            {
                showShopResult("DÉSENCHANTEMENT REFUSÉ", "shop.enchanter.disenchant.weapon.cost", lines);
                return;
            }
            weapon = player.getInventory().getMutableWeapon(weaponChoice - 1);
            if (weapon == nullptr || !weapon->removeLastEnchantment())
            {
                lines.push_back("La rune n'a pas pu être retirée proprement. Aucun effet appliqué.");
                showShopResult("DÉSENCHANTEMENT ÉCHOUÉ", "shop.enchanter.disenchant.weapon.failed", lines);
                return;
            }
            player.getInventory().addMaterial(MaterialCatalog::createById("runic_extraction_note", 1));
            lines.push_back("Arme traitée : " + weaponName + ".");
            lines.push_back("Avant : " + before + ".");
            lines.push_back("Après : " + weapon->getEnchantmentSummaryText() + ".");
            lines.push_back("Note obtenue : Note d'extraction runique x1.");
            showLocalServiceResult("RUNE RETIRÉE", "shop.enchanter.disenchant.weapon.success", player, lines, 1);
            return;
        }

        if (typeChoice == 2)
        {
            MenuScreen armorScreen("ARMURE À DÉSENCHANTER", "shop.enchanter.disenchant.armor.list");
            armorScreen.addLine("Seules les armures avec au moins une rune peuvent être choisies.");
            armorScreen.addOption(0, "Retour", "Revenir au choix.", true, "shop.enchanter.disenchant.armor.back");
            const std::vector<Armor>& armors = player.getInventory().getArmors();
            for (std::size_t i = 0; i < armors.size(); ++i)
            {
                const Armor& armor = armors[i];
                armorScreen.addOption(static_cast<int>(i + 1), armor.getName() + " | runes " + std::to_string(armor.getEnchantmentCount()), armor.getEnchantmentSummaryText(), armor.getEnchantmentCount() > 0, "shop.enchanter.disenchant.armor." + std::to_string(i));
            }
            Console::clear();
            const int armorChoice = TerminalInterface::askMenuChoiceFromOptions(armorScreen, "Choisis une armure.");
            if (armorChoice <= 0 || armorChoice > static_cast<int>(armors.size())) return;
            Armor* armor = player.getInventory().getMutableArmor(armorChoice - 1);
            if (armor == nullptr || armor->getEnchantmentCount() <= 0)
            {
                showShopResult("DÉSENCHANTEMENT IMPOSSIBLE", "shop.enchanter.disenchant.armor.invalid", {"Cette armure ne porte aucune rune retirable."});
                return;
            }
            std::vector<std::string> lines;
            const std::string armorName = armor->getName();
            const std::string before = armor->getEnchantmentSummaryText();
            if (!payDisenchantmentCost(player, lines))
            {
                showShopResult("DÉSENCHANTEMENT REFUSÉ", "shop.enchanter.disenchant.armor.cost", lines);
                return;
            }
            armor = player.getInventory().getMutableArmor(armorChoice - 1);
            if (armor == nullptr || !armor->removeLastEnchantment())
            {
                lines.push_back("La rune n'a pas pu être retirée proprement. Aucun effet appliqué.");
                showShopResult("DÉSENCHANTEMENT ÉCHOUÉ", "shop.enchanter.disenchant.armor.failed", lines);
                return;
            }
            player.getInventory().addMaterial(MaterialCatalog::createById("runic_extraction_note", 1));
            lines.push_back("Armure traitée : " + armorName + ".");
            lines.push_back("Avant : " + before + ".");
            lines.push_back("Après : " + armor->getEnchantmentSummaryText() + ".");
            lines.push_back("Note obtenue : Note d'extraction runique x1.");
            showLocalServiceResult("RUNE RETIRÉE", "shop.enchanter.disenchant.armor.success", player, lines, 1);
        }
    }

    bool payRunicSafetySealCost(Player& player, std::vector<std::string>& lines)
    {
        const int price = 125;
        const int dustCost = 2;
        if (player.getInventory().countMaterialById("runic_stabilizer") < 1)
        {
            lines.push_back("Composant manquant : Stabilisateur runique x1 requis.");
            return false;
        }
        if (player.getInventory().countMaterialById("arcane_dust") < dustCost)
        {
            lines.push_back("Composant manquant : Poussière arcanique x" + std::to_string(dustCost) + " requis.");
            return false;
        }
        if (!player.getInventory().spendEconomyUnits(price))
        {
            lines.push_back("Paiement refusé : il faut " + Money::formatEconomyUnits(price) + ".");
            lines.push_back("Argent disponible : " + player.getInventory().getWalletLine() + ".");
            return false;
        }
        player.getInventory().removeMaterialQuantityById("runic_stabilizer", 1);
        player.getInventory().removeMaterialQuantityById("arcane_dust", dustCost);
        lines.push_back("Coût payé : " + Money::formatEconomyUnits(price) + " + Stabilisateur runique x1 + Poussière arcanique x" + std::to_string(dustCost) + ".");
        return true;
    }

    void prepareRunicSafetySeal(Player& player)
    {
        std::vector<std::string> lines;
        if (player.getInventory().countMaterialById("runic_safety_seal") > 0)
        {
            lines.push_back("Tu portes déjà un Sceau anti-casse runique prêt à se sacrifier.");
            lines.push_back("Limite : un seul sceau est conseillé à la fois. L'enchanteur refuse d'empiler des protections qui vibrent entre elles.");
            showShopResult("SCEAU DÉJÀ PRÊT", "shop.enchanter.safety_seal.already", lines);
            return;
        }
        if (!payRunicSafetySealCost(player, lines))
        {
            showShopResult("SCEAU REFUSÉ", "shop.enchanter.safety_seal.cost", lines);
            return;
        }
        player.getInventory().addMaterial(MaterialCatalog::createById("runic_safety_seal", 1));
        lines.push_back("L'enchanteur trace un sceau fragile autour de l'équipement à travailler ensuite.");
        lines.push_back("Effet : au prochain échec critique d'enchantement, le sceau se brise pour sauver l'objet. La rune tentée ne sera pas gravée.");
        lines.push_back("Ce n'est pas une assurance infinie : le sceau est consommé dès qu'il évite une casse.");
        showLocalServiceResult("SCEAU ANTI-CASSE PRÊT", "shop.enchanter.safety_seal.success", player, lines, 1);
    }

    bool payRuneTransferCost(Player& player, std::vector<std::string>& lines)
    {
        const int price = 95;
        const int dustCost = 3;
        if (player.getInventory().countMaterialById("runic_extraction_note") < 1)
        {
            lines.push_back("Composant manquant : Note d'extraction runique x1 requis.");
            return false;
        }
        if (player.getInventory().countMaterialById("arcane_dust") < dustCost)
        {
            lines.push_back("Composant manquant : Poussière arcanique x" + std::to_string(dustCost) + " requis.");
            return false;
        }
        if (!player.getInventory().spendEconomyUnits(price))
        {
            lines.push_back("Paiement refusé : il faut " + Money::formatEconomyUnits(price) + ".");
            lines.push_back("Argent disponible : " + player.getInventory().getWalletLine() + ".");
            return false;
        }
        player.getInventory().removeMaterialQuantityById("runic_extraction_note", 1);
        player.getInventory().removeMaterialQuantityById("arcane_dust", dustCost);
        lines.push_back("Coût payé : " + Money::formatEconomyUnits(price) + " + Note d'extraction runique x1 + Poussière arcanique x" + std::to_string(dustCost) + ".");
        return true;
    }

    bool rollRuneTransferFailure(int riskPercent)
    {
        static std::mt19937 generator(std::random_device{}());
        std::uniform_int_distribution<int> distribution(1, 100);
        return distribution(generator) <= riskPercent;
    }

    void transferWeaponRune(Player& player)
    {
        const std::vector<Weapon>& weapons = player.getInventory().getWeapons();
        MenuScreen sourceScreen("ARME SOURCE", "shop.enchanter.transfer.weapon.source");
        sourceScreen.addLine("Choisis l'arme qui perdra sa dernière rune si le transfert est tenté.");
        sourceScreen.addOption(0, "Retour", "Annuler.", true, "shop.enchanter.transfer.weapon.back");
        for (std::size_t i = 0; i < weapons.size(); ++i)
        {
            const Weapon& weapon = weapons[i];
            sourceScreen.addOption(static_cast<int>(i + 1), weapon.getName() + " | runes " + std::to_string(weapon.getEnchantmentCount()), weapon.getEnchantmentSummaryText(), weapon.getEnchantmentCount() > 0, "shop.enchanter.transfer.weapon.source." + std::to_string(i));
        }
        Console::clear();
        const int sourceChoice = TerminalInterface::askMenuChoiceFromOptions(sourceScreen, "Choisis l'arme source.");
        if (sourceChoice <= 0 || sourceChoice > static_cast<int>(weapons.size())) return;

        MenuScreen targetScreen("ARME CIBLE", "shop.enchanter.transfer.weapon.target");
        targetScreen.addLine("Choisis l'arme qui recevra une version instable de cette rune.");
        targetScreen.addOption(0, "Retour", "Annuler.", true, "shop.enchanter.transfer.weapon.target.back");
        for (std::size_t i = 0; i < weapons.size(); ++i)
        {
            const Weapon& weapon = weapons[i];
            const bool valid = static_cast<int>(i + 1) != sourceChoice && weapon.getName() != "Mains nues";
            targetScreen.addOption(static_cast<int>(i + 1), weapon.getName() + " | runes " + std::to_string(weapon.getEnchantmentCount()), weapon.getEnchantmentSummaryText(), valid, "shop.enchanter.transfer.weapon.target." + std::to_string(i));
        }
        Console::clear();
        const int targetChoice = TerminalInterface::askMenuChoiceFromOptions(targetScreen, "Choisis l'arme cible.");
        if (targetChoice <= 0 || targetChoice > static_cast<int>(weapons.size()) || targetChoice == sourceChoice) return;

        Weapon* source = player.getInventory().getMutableWeapon(sourceChoice - 1);
        Weapon* target = player.getInventory().getMutableWeapon(targetChoice - 1);
        if (source == nullptr || target == nullptr || source->getEnchantmentCount() <= 0)
        {
            showShopResult("TRANSFERT IMPOSSIBLE", "shop.enchanter.transfer.weapon.invalid", {"Source ou cible invalide."});
            return;
        }
        const std::vector<std::string> sourceRunes = source->getEnchantments();
        const std::string movedRune = sourceRunes.empty() ? "Rune inconnue" : sourceRunes.back();
        const int risk = std::min(80, 25 + source->getEnchantmentCount() * 5 + target->getEnchantmentCount() * 10);
        const bool confirmed = askShopConfirmation(
            "CONFIRMER LE TRANSFERT",
            "shop.enchanter.transfer.weapon.confirm",
            {
                "Source : " + source->getName(),
                "Cible : " + target->getName(),
                "Rune déplacée : " + movedRune,
                "Risque de perdre la rune pendant le transfert : " + std::to_string(risk) + "%.",
                "Attention : le sceau anti-casse protège surtout contre la casse d'objet, pas contre une rune qui se dissout."
            },
            "Tenter le transfert",
            "Annuler",
            "shop.enchanter.transfer.weapon"
        );
        if (!confirmed) return;

        std::vector<std::string> lines;
        if (!payRuneTransferCost(player, lines))
        {
            showShopResult("TRANSFERT REFUSÉ", "shop.enchanter.transfer.weapon.cost", lines);
            return;
        }
        source = player.getInventory().getMutableWeapon(sourceChoice - 1);
        target = player.getInventory().getMutableWeapon(targetChoice - 1);
        if (source == nullptr || target == nullptr || !source->removeLastEnchantment())
        {
            lines.push_back("La rune source n'a pas pu être décrochée proprement.");
            showShopResult("TRANSFERT ÉCHOUÉ", "shop.enchanter.transfer.weapon.detach", lines);
            return;
        }
        if (rollRuneTransferFailure(risk))
        {
            player.getInventory().addMaterial(MaterialCatalog::createById("arcane_dust", 1));
            lines.push_back("La rune se dissout entre les deux armes. Source affaiblie, cible inchangée.");
            lines.push_back("Récupération : Poussière arcanique x1.");
            showLocalServiceResult("RUNE PERDUE", "shop.enchanter.transfer.weapon.failed", player, lines, 1);
            return;
        }
        target->addEnchantment("Transfert instable : " + movedRune);
        player.getInventory().addMaterial(MaterialCatalog::createById("runic_transfer_note", 1));
        lines.push_back("Rune transférée : " + movedRune + ".");
        lines.push_back("Source : la dernière rune a été retirée.");
        lines.push_back("Cible : rune reçue sous forme instable, donc les prochains empilements doivent être surveillés.");
        lines.push_back("Note obtenue : Note de transfert runique x1.");
        showLocalServiceResult("RUNE TRANSFÉRÉE", "shop.enchanter.transfer.weapon.success", player, lines, 1);
    }

    void transferArmorRune(Player& player)
    {
        const std::vector<Armor>& armors = player.getInventory().getArmors();
        MenuScreen sourceScreen("ARMURE SOURCE", "shop.enchanter.transfer.armor.source");
        sourceScreen.addLine("Choisis l'armure qui perdra sa dernière rune si le transfert est tenté.");
        sourceScreen.addOption(0, "Retour", "Annuler.", true, "shop.enchanter.transfer.armor.back");
        for (std::size_t i = 0; i < armors.size(); ++i)
        {
            const Armor& armor = armors[i];
            sourceScreen.addOption(static_cast<int>(i + 1), armor.getName() + " | runes " + std::to_string(armor.getEnchantmentCount()), armor.getEnchantmentSummaryText(), armor.getEnchantmentCount() > 0, "shop.enchanter.transfer.armor.source." + std::to_string(i));
        }
        Console::clear();
        const int sourceChoice = TerminalInterface::askMenuChoiceFromOptions(sourceScreen, "Choisis l'armure source.");
        if (sourceChoice <= 0 || sourceChoice > static_cast<int>(armors.size())) return;

        MenuScreen targetScreen("ARMURE CIBLE", "shop.enchanter.transfer.armor.target");
        targetScreen.addLine("Choisis l'armure qui recevra une version instable de cette rune.");
        targetScreen.addOption(0, "Retour", "Annuler.", true, "shop.enchanter.transfer.armor.target.back");
        for (std::size_t i = 0; i < armors.size(); ++i)
        {
            const Armor& armor = armors[i];
            const bool valid = static_cast<int>(i + 1) != sourceChoice;
            targetScreen.addOption(static_cast<int>(i + 1), armor.getName() + " | runes " + std::to_string(armor.getEnchantmentCount()), armor.getEnchantmentSummaryText(), valid, "shop.enchanter.transfer.armor.target." + std::to_string(i));
        }
        Console::clear();
        const int targetChoice = TerminalInterface::askMenuChoiceFromOptions(targetScreen, "Choisis l'armure cible.");
        if (targetChoice <= 0 || targetChoice > static_cast<int>(armors.size()) || targetChoice == sourceChoice) return;

        Armor* source = player.getInventory().getMutableArmor(sourceChoice - 1);
        Armor* target = player.getInventory().getMutableArmor(targetChoice - 1);
        if (source == nullptr || target == nullptr || source->getEnchantmentCount() <= 0)
        {
            showShopResult("TRANSFERT IMPOSSIBLE", "shop.enchanter.transfer.armor.invalid", {"Source ou cible invalide."});
            return;
        }
        const std::vector<std::string> sourceRunes = source->getEnchantments();
        const std::string movedRune = sourceRunes.empty() ? "Rune inconnue" : sourceRunes.back();
        const int risk = std::min(80, 22 + source->getEnchantmentCount() * 5 + target->getEnchantmentCount() * 9);
        const bool confirmed = askShopConfirmation(
            "CONFIRMER LE TRANSFERT",
            "shop.enchanter.transfer.armor.confirm",
            {
                "Source : " + source->getName(),
                "Cible : " + target->getName(),
                "Rune déplacée : " + movedRune,
                "Risque de perdre la rune pendant le transfert : " + std::to_string(risk) + "%.",
                "Attention : l'opération est moins chère qu'une nouvelle rune rare, mais jamais gratuite."
            },
            "Tenter le transfert",
            "Annuler",
            "shop.enchanter.transfer.armor"
        );
        if (!confirmed) return;

        std::vector<std::string> lines;
        if (!payRuneTransferCost(player, lines))
        {
            showShopResult("TRANSFERT REFUSÉ", "shop.enchanter.transfer.armor.cost", lines);
            return;
        }
        source = player.getInventory().getMutableArmor(sourceChoice - 1);
        target = player.getInventory().getMutableArmor(targetChoice - 1);
        if (source == nullptr || target == nullptr || !source->removeLastEnchantment())
        {
            lines.push_back("La rune source n'a pas pu être décrochée proprement.");
            showShopResult("TRANSFERT ÉCHOUÉ", "shop.enchanter.transfer.armor.detach", lines);
            return;
        }
        if (rollRuneTransferFailure(risk))
        {
            player.getInventory().addMaterial(MaterialCatalog::createById("arcane_dust", 1));
            lines.push_back("La rune se dissout entre les deux armures. Source affaiblie, cible inchangée.");
            lines.push_back("Récupération : Poussière arcanique x1.");
            showLocalServiceResult("RUNE PERDUE", "shop.enchanter.transfer.armor.failed", player, lines, 1);
            return;
        }
        target->addEnchantment("Transfert instable : " + movedRune);
        player.getInventory().addMaterial(MaterialCatalog::createById("runic_transfer_note", 1));
        lines.push_back("Rune transférée : " + movedRune + ".");
        lines.push_back("Source : la dernière rune a été retirée.");
        lines.push_back("Cible : rune reçue sous forme instable, donc les prochains empilements doivent être surveillés.");
        lines.push_back("Note obtenue : Note de transfert runique x1.");
        showLocalServiceResult("RUNE TRANSFÉRÉE", "shop.enchanter.transfer.armor.success", player, lines, 1);
    }

    void openRuneTransferMenu(Player& player)
    {
        MenuScreen typeScreen("TRANSFERT DE RUNE RISQUÉ", "shop.enchanter.transfer.type");
        typeScreen.addLine("Le transfert retire la dernière rune d'un équipement source et tente de la poser sur une cible.");
        typeScreen.addLine("Coût : " + Money::formatEconomyUnits(95) + " + Note d'extraction runique x1 + Poussière arcanique x3.");
        typeScreen.addLine("Échec : la rune peut se dissoudre. Ce service n'est donc pas une duplication gratuite.");
        typeScreen.addOption(0, "Retour", "Revenir à l'atelier.", true, "shop.enchanter.transfer.back");
        typeScreen.addOption(1, "Transférer entre armes", "Déplace la dernière rune d'une arme vers une autre arme.", player.getInventory().getWeaponCount() >= 2, "shop.enchanter.transfer.weapon");
        typeScreen.addOption(2, "Transférer entre armures", "Déplace la dernière rune d'une armure vers une autre armure.", player.getInventory().getArmorCount() >= 2, "shop.enchanter.transfer.armor");
        Console::clear();
        const int choice = TerminalInterface::askMenuChoiceFromOptions(typeScreen, "Choisis le type de transfert.");
        if (choice == 1) transferWeaponRune(player);
        else if (choice == 2) transferArmorRune(player);
    }




}

void EnchanterServiceMenu::open(Player& player)
{
    bool stay = true;
    while (stay)
    {
        MenuScreen screen("ATELIER DE L'ENCHANTEUR", "shop.enchanter.services");
        screen.addLine("Principe : une arme ou armure peut recevoir plusieurs enchantements.");
        screen.addLine("Risque : chaque nouvel enchantement augmente la chance de casse définitive, sans réparation possible.");
        screen.addLine("Limite pratique : 5 enchantements environ. Passé 5, l'enchanteur peut tenter, mais stabiliser devient vraiment dur.");
        screen.addLine("Échec critique : l'objet est perdu, mais tu récupères au moins des restes de métal/matière et des résidus arcaniques.");
        screen.addLine("Premier essai : très fiable sur une bonne pièce, beaucoup moins sur une arme/armure claquée au sol.");
        screen.addLine("Argent : " + player.getInventory().getWalletLine());
        screen.addLine("Composants : Poussière arcanique x" + std::to_string(player.getInventory().countMaterialById("arcane_dust"))
            + ", Fleur bleue x" + std::to_string(player.getInventory().countMaterialById("mountain_blue_flower"))
            + ", Fragment draconique x" + std::to_string(player.getInventory().countMaterialById("draconic_scale_fragment"))
            + ", Stabilisateur x" + std::to_string(player.getInventory().countMaterialById("runic_stabilizer"))
            + ", Sceau anti-casse x" + std::to_string(player.getInventory().countMaterialById("runic_safety_seal"))
            + ", Note extraction x" + std::to_string(player.getInventory().countMaterialById("runic_extraction_note"))
            + ", Note surcharge x" + std::to_string(player.getInventory().countMaterialById("runic_overload_limit_note")) + ".");
        screen.addOption(0, "Retour", "Revenir au comptoir.", true, "shop.enchanter.back");
        screen.addOption(1, "Enchanter une arme", "Choisir une arme puis une rune. Risque de casse définitive.", player.getInventory().getWeaponCount() > 0, "shop.enchanter.weapon");
        screen.addOption(2, "Enchanter une armure", "Choisir une armure puis une rune. Risque de casse définitive.", player.getInventory().getArmorCount() > 0, "shop.enchanter.armor");
        screen.addOption(3, "Stabiliser un contrecoup runique grave", "Solution spéciale : prépare une malédiction trop avancée pour un rite total à l'église.", player.hasHighRunicBacklashNeedingEnchanter(), "shop.enchanter.runic_backlash");
        screen.addOption(4, "Lire la surcharge runique", "Analyse les armes/armures déjà enchantées sans ajouter de rune ni consommer de composant.", true, "shop.enchanter.overload_reading");
        screen.addOption(5, "Désenchanter prudemment", "Retire la dernière rune d'un objet contre or + poussière. Ne transfère pas gratuitement la rune.", true, "shop.enchanter.disenchant");
        screen.addOption(6, "Préparer un sceau anti-casse", "Protection à usage unique : sauve l'objet si un enchantement part en casse critique.", true, "shop.enchanter.safety_seal");
        screen.addOption(7, "Transférer une rune risquée", "Déplace la dernière rune d'un équipement vers un autre, avec risque de perdre la rune.", true, "shop.enchanter.transfer");
        screen.addOption(8, "Noter une limite de surcharge", "Transforme une lecture de surcharge en note de suivi, utile avant de pousser une pièce trop loin.", true, "shop.enchanter.overload_note");
        Console::clear();
        const int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choisis une action d'enchantement.");
        if (choice == 0)
        {
            stay = false;
            continue;
        }

        const std::vector<EnchantmentOffer> offers = getEnchantmentOffers();
        if (choice == 1)
        {
            MenuScreen weaponScreen("CHOISIR UNE ARME", "shop.enchanter.weapon.list");
            weaponScreen.addLine("Les mains nues ne peuvent pas être enchantées. Les armes faibles ont plus de risques au premier essai.");
            weaponScreen.addOption(0, "Retour", "Revenir à l'atelier.", true, "shop.enchanter.weapon.back");
            const std::vector<Weapon>& weapons = player.getInventory().getWeapons();
            for (std::size_t i = 0; i < weapons.size(); ++i)
            {
                const Weapon& weapon = weapons[i];
                const int risk = enchantmentBreakRiskPercent(weapon.getEnchantmentCount(), equipmentQualityRiskPenalty(weapon.getName(), weapon.getValue(), weapon.getMaxDurability()), false);
                weaponScreen.addOption(static_cast<int>(i + 1), weapon.getName() + " | runes " + std::to_string(weapon.getEnchantmentCount()) + " | risque prochain essai " + std::to_string(risk) + "%", weapon.getEnchantmentSummaryText(), true, "shop.enchanter.weapon." + std::to_string(i));
            }
            Console::clear();
            const int weaponChoice = TerminalInterface::askMenuChoiceFromOptions(weaponScreen, "Choisis une arme.");
            if (weaponChoice <= 0 || weaponChoice > static_cast<int>(weapons.size())) continue;
            const int offerChoice = askEnchantmentOfferChoice(offers);
            if (offerChoice <= 0 || offerChoice > static_cast<int>(offers.size())) continue;
            enchantWeapon(player, weaponChoice - 1, offers[static_cast<std::size_t>(offerChoice - 1)]);
        }
        else if (choice == 3)
        {
            stabilizeRunicBacklashAtEnchanter(player);
        }
        else if (choice == 4)
        {
            inspectRunicOverload(player);
        }
        else if (choice == 5)
        {
            openDisenchantmentMenu(player);
        }
        else if (choice == 6)
        {
            prepareRunicSafetySeal(player);
        }
        else if (choice == 7)
        {
            openRuneTransferMenu(player);
        }
        else if (choice == 8)
        {
            std::vector<std::string> lines;
            const int price = 42;
            if (!player.getInventory().spendEconomyUnits(price))
            {
                lines.push_back("Paiement refusé : il faut " + Money::formatEconomyUnits(price) + ".");
                lines.push_back("Argent disponible : " + player.getInventory().getWalletLine() + ".");
                showShopResult("NOTE REFUSÉE", "shop.enchanter.overload_note.failed", lines);
                continue;
            }
            player.getInventory().addMaterial(MaterialCatalog::createById("runic_overload_limit_note", 1));
            lines.push_back("L'enchanteur note les seuils à ne pas dépasser sur tes pièces les plus chargées.");
            lines.push_back("Ce n'est pas une protection magique : c'est un rappel utile avant de tenter l'enchantement de trop.");
            showLocalServiceResult("LIMITE DE SURCHARGE NOTÉE", "shop.enchanter.overload_note.success", player, lines, 1);
        }
        else if (choice == 2)
        {
            MenuScreen armorScreen("CHOISIR UNE ARMURE", "shop.enchanter.armor.list");
            armorScreen.addLine("Une tenue de survie ou une armure classique peut recevoir une rune, mais la magie empilée devient risquée.");
            armorScreen.addOption(0, "Retour", "Revenir à l'atelier.", true, "shop.enchanter.armor.back");
            const std::vector<Armor>& armors = player.getInventory().getArmors();
            for (std::size_t i = 0; i < armors.size(); ++i)
            {
                const Armor& armor = armors[i];
                const int risk = enchantmentBreakRiskPercent(armor.getEnchantmentCount(), equipmentQualityRiskPenalty(armor.getName(), armor.getValue(), armor.getMaxDurability()), true);
                armorScreen.addOption(static_cast<int>(i + 1), armor.getName() + " | runes " + std::to_string(armor.getEnchantmentCount()) + " | risque prochain essai " + std::to_string(risk) + "%", armor.getEnchantmentSummaryText(), true, "shop.enchanter.armor." + std::to_string(i));
            }
            Console::clear();
            const int armorChoice = TerminalInterface::askMenuChoiceFromOptions(armorScreen, "Choisis une armure.");
            if (armorChoice <= 0 || armorChoice > static_cast<int>(armors.size())) continue;
            const int offerChoice = askEnchantmentOfferChoice(offers);
            if (offerChoice <= 0 || offerChoice > static_cast<int>(offers.size())) continue;
            enchantArmor(player, armorChoice - 1, offers[static_cast<std::size_t>(offerChoice - 1)]);
        }
    }
}
