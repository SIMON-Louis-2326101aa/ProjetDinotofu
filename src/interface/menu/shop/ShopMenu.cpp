// EN: ShopMenu.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: ShopMenu.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.
// English: Displays the first usable shop menu, with some real purchases and prepared future categories.
// Français : Affiche le premier menu de boutique utilisable, avec certains achats réels et des catégories futures préparées.

#include "interface/menu/shop/ShopMenu.hpp"
#include "interface/menu/shop/ChurchServiceMenu.hpp"
#include "interface/menu/shop/EnchanterServiceMenu.hpp"
#include "interface/menu/shop/ShopCityServiceMenu.hpp"

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

using ShopCityServiceMenu::openCityServiceSpecialMenu;
using ShopCityServiceMenu::openLodgingServiceMenu;
using ShopCityServiceMenu::openTransportServiceMenu;
using ShopCityServiceMenu::CityEventOfDay;
using ShopCityServiceMenu::cityEventForAbsoluteDay;
using ShopCityServiceMenu::hasRoyalBonusCityEventToday;
using ShopCityServiceMenu::cityRepairDaysRemaining;
using ShopCityServiceMenu::kCityEconomyDiscountCapPercent;

namespace
{
    std::string getVendorNameForShop(ShopType type);

    std::uint32_t stableShopHash(const std::string& value)
    {
        std::uint32_t hash = 2166136261u;
        for (unsigned char c : value)
        {
            hash ^= c;
            hash *= 16777619u;
        }
        return hash;
    }


    bool tryCreateWeaponPreviewForShopItem(const ShopItem& item, Weapon& outWeapon)
    {
        const std::string& id = item.getId();
        if (id == "rusty_sword") outWeapon = WeaponCatalog::createRustySword();
        else if (id == "training_dagger") outWeapon = WeaponCatalog::createTrainingDagger();
        else if (id == "training_spear") outWeapon = WeaponCatalog::createTrainingSpear();
        else if (id == "training_bow") outWeapon = WeaponCatalog::createTrainingBow();
        else if (id == "training_crossbow") outWeapon = WeaponCatalog::createTrainingCrossbow();
        else if (id == "training_throwing_bandolier") outWeapon = WeaponCatalog::createTrainingThrowingBandolier();
        else if (id == "training_staff") outWeapon = WeaponCatalog::createTrainingStaff();
        else if (id == "heavy_training_axe") outWeapon = WeaponCatalog::createHeavyTrainingAxe();
        else if (id == "iron_sword") outWeapon = WeaponCatalog::createIronSword();
        else if (id == "reinforced_dagger") outWeapon = WeaponCatalog::createReinforcedDagger();
        else if (id == "guard_spear") outWeapon = WeaponCatalog::createGuardSpear();
        else if (id == "hunting_bow") outWeapon = WeaponCatalog::createHuntingBow();
        else if (id == "apprentice_staff") outWeapon = WeaponCatalog::createApprenticeStaff();
        else if (id == "heavy_iron_axe") outWeapon = WeaponCatalog::createHeavyIronAxe();
        else if (id == "workshop_hammer") outWeapon = WeaponCatalog::createWorkshopHammer();
        else if (id == "patrol_crossbow") outWeapon = WeaponCatalog::createPatrolCrossbow();
        else if (id == "balanced_rapier") outWeapon = WeaponCatalog::createBalancedRapier();
        else if (id == "mercenary_sabre") outWeapon = WeaponCatalog::createMercenarySabre();
        else if (id == "curved_ambush_dagger") outWeapon = WeaponCatalog::createCurvedAmbushDagger();
        else if (id == "militia_longbow") outWeapon = WeaponCatalog::createMilitiaLongbow();
        else if (id == "bound_oak_staff") outWeapon = WeaponCatalog::createBoundOakStaff();
        else if (id == "runic_iron_blade") outWeapon = WeaponCatalog::createRunicIronBlade();
        else if (id == "amber_edge_dagger") outWeapon = WeaponCatalog::createAmberEdgeDagger();
        else if (id == "ashen_longbow") outWeapon = WeaponCatalog::createAshenLongbow();
        else if (id == "channeling_scepter") outWeapon = WeaponCatalog::createChannelingScepter();
        else if (id == "relay_falchion") outWeapon = WeaponCatalog::createRelayFalchion();
        else if (id == "whistling_mine_hammer") outWeapon = WeaponCatalog::createWhistlingMineHammer();
        else if (id == "singing_resin_staff") outWeapon = WeaponCatalog::createSingingResinStaff();
        else if (id == "cold_lantern_bow") outWeapon = WeaponCatalog::createColdLanternBow();
        else if (id == "red_clay_sabre") outWeapon = WeaponCatalog::createRedClaySabre();
        else if (id == "broken_map_dagger") outWeapon = WeaponCatalog::createBrokenMapDagger();
        else if (id == "firefly_iron_rapier") outWeapon = WeaponCatalog::createFireflyIronRapier();
        else if (id == "drowned_ledger_mace") outWeapon = WeaponCatalog::createDrownedLedgerMace();
        else if (id == "grey_cliff_spear") outWeapon = WeaponCatalog::createGreyCliffSpear();
        else if (id == "broken_carnival_whip") outWeapon = WeaponCatalog::createBrokenCarnivalWhip();
        else return false;
        return true;
    }

    std::string shopWeaponClassCompatibilityTag(const Player& player, const ShopItem& item)
    {
        if (item.getCategory() != ShopItemCategory::Weapon)
        {
            return "";
        }
        Weapon preview;
        if (!tryCreateWeaponPreviewForShopItem(item, preview))
        {
            return "";
        }
        if (CombatClassSystem::hasWeaponAffinity(player, preview.getType(), preview.getName()))
        {
            return " [bonus de classe]";
        }
        if (CombatClassSystem::getWeaponHandlingAccuracyAdjustment(player, preview.getType(), preview.getName()) < 0
            || CombatClassSystem::getWeaponHandlingDamagePercent(player, preview.getType(), preview.getName()) < 100)
        {
            return " [malus de classe]";
        }
        return "";
    }

    std::string shopWeaponClassCompatibilityLine(const Player& player, const ShopItem& item)
    {
        Weapon preview;
        if (item.getCategory() != ShopItemCategory::Weapon || !tryCreateWeaponPreviewForShopItem(item, preview))
        {
            return "";
        }
        if (CombatClassSystem::hasWeaponAffinity(player, preview.getType(), preview.getName()))
        {
            return "Bonus de classe : " + CombatClassSystem::getWeaponAffinityLabel(player, preview.getType(), preview.getName()) + ".";
        }
        if (CombatClassSystem::getWeaponHandlingAccuracyAdjustment(player, preview.getType(), preview.getName()) < 0
            || CombatClassSystem::getWeaponHandlingDamagePercent(player, preview.getType(), preview.getName()) < 100)
        {
            return "Malus de classe : " + CombatClassSystem::getWeaponHandlingLabel(player, preview.getType(), preview.getName()) + ".";
        }
        return "";
    }

    std::string ownedWeaponClassCompatibilityTag(const Player& player, const Weapon& weapon)
    {
        if (CombatClassSystem::hasWeaponAffinity(player, weapon.getType(), weapon.getName()))
        {
            return " [bonus de classe]";
        }
        if (CombatClassSystem::getWeaponHandlingAccuracyAdjustment(player, weapon.getType(), weapon.getName()) < 0
            || CombatClassSystem::getWeaponHandlingDamagePercent(player, weapon.getType(), weapon.getName()) < 100)
        {
            return " [malus de classe]";
        }
        return "";
    }

    int prunigilMerchantTrustScore(const Player& player)
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

    struct TemporaryMerchantDefinition
    {
        std::string name;
        std::string sellingStyle;
        ShopType sourceType = ShopType::Unknown;
        int requiredTrust = 0;
        int cycleLengthDays = 7;
        int activeDays = 2;
        int offset = 0;
        int itemCount = 4;
    };

    bool temporaryMerchantIsPresent(const TemporaryMerchantDefinition& definition, int day)
    {
        const int cycle = std::max(1, definition.cycleLengthDays);
        const int normalized = ((day + definition.offset) % cycle + cycle) % cycle;
        return normalized < std::max(1, definition.activeDays);
    }

    ShopInventory buildTemporaryMerchantShop(const TemporaryMerchantDefinition& definition, int day)
    {
        ShopInventory source = ShopCatalog::createPreviewShop(definition.sourceType);
        ShopInventory temporary(
            definition.sourceType,
            definition.name + " — " + definition.sellingStyle
        );

        const std::vector<ShopItem>& sourceItems = source.getItems();
        if (sourceItems.empty()) return temporary;

        const int wanted = std::min(definition.itemCount, static_cast<int>(sourceItems.size()));
        const int cycleIndex = (day + definition.offset) / std::max(1, definition.cycleLengthDays);
        const std::size_t start = static_cast<std::size_t>((stableShopHash(definition.name) + static_cast<std::uint32_t>(cycleIndex)) % sourceItems.size());
        for (int i = 0; i < wanted; ++i)
        {
            temporary.addItem(sourceItems[(start + static_cast<std::size_t>(i)) % sourceItems.size()]);
        }
        return temporary;
    }

    void appendTemporaryRecommendedShops(const Player& player, std::vector<ShopInventory>& shops)
    {
        const int trust = prunigilMerchantTrustScore(player);

        const std::vector<TemporaryMerchantDefinition> definitions = {
            {"Mirette la couturière", "vente posée, tissus et protections adaptées", ShopType::Armor, 6, 8, 2, 1, 4},
            {"Caldor le porteur de caisses", "vente rapide de lots utiles avant le départ", ShopType::Material, 12, 9, 3, 4, 5},
            {"Éliane du vieux pont", "vente prudente de plantes et provisions de route", ShopType::Plant, 12, 10, 2, 2, 4},
            {"Bruma la réparatrice de selles", "vente technique de réparation et d'équipement de trajet", ShopType::Material, 20, 12, 3, 7, 5}
        };

        const int day = std::max(0, player.getWorldDaysElapsed());
        for (const TemporaryMerchantDefinition& definition : definitions)
        {
            if (trust < definition.requiredTrust || !temporaryMerchantIsPresent(definition, day)) continue;
            shops.push_back(buildTemporaryMerchantShop(definition, day));
        }

        // Ces deux présences ne dépendent pas de Prunigil : elles sont ouvertes par une vraie rencontre de terrain.
        if (player.hasTitle("Témoin du marchand bleu")
            && player.getExplorationSceneCooldownRemainingDays("legendary_merchant_hero_villager") >= 34)
        {
            TemporaryMerchantDefinition hero;
            hero.name = "Hero Villager";
            hero.sellingStyle = "équipement héroïque, reliques d'expédition et défis déraisonnables";
            hero.sourceType = ShopType::Weapon;
            hero.itemCount = 6;
            ShopInventory heroShop = buildTemporaryMerchantShop(hero, day);
            const std::uint32_t potionRoll = stableShopHash("hero_villager_lucky_stock")
                + static_cast<std::uint32_t>(day / 7);
            if (potionRoll % 100 < 55)
            {
                heroShop.addItem(ShopItem(
                    "lucky_potion",
                    "Lucky Potion",
                    "Potion rare qui accorde plusieurs bonus aléatoires pendant trois tours.",
                    ShopItemCategory::Consumable,
                    285,
                    85,
                    1
                ));
            }
            if ((potionRoll / 3) % 100 < 45)
            {
                heroShop.addItem(ShopItem(
                    "unlucky_potion",
                    "Unlucky Potion",
                    "Fiole lancée sur une cible : malus aléatoires pendant trois tours, ou trois ennemis faibles dans de rares cas.",
                    ShopItemCategory::Consumable,
                    305,
                    90,
                    1
                ));
            }
            shops.push_back(heroShop);
        }

        if (player.hasTitle("Les deux du même comptoir")
            && player.getExplorationSceneCooldownRemainingDays("legendary_merchant_bob_maurice") >= 12)
        {
            TemporaryMerchantDefinition duo;
            duo.name = "Bob et Maurice";
            duo.sellingStyle = "caisses risquées, lots imprévisibles et provisions censées limiter les dégâts";
            duo.sourceType = ShopType::Consumable;
            duo.itemCount = 6;
            shops.push_back(buildTemporaryMerchantShop(duo, day));
        }
    }

    bool isTemporaryRecommendedShop(const ShopInventory& shop)
    {
        return shop.getName().find(" — ") != std::string::npos;
    }

    bool isSpecialLegendaryMerchantShop(const ShopInventory& shop)
    {
        return shop.getName().rfind("Hero Villager — ", 0) == 0
            || shop.getName().rfind("Bob et Maurice — ", 0) == 0;
    }

    std::string temporaryMerchantDisplayName(const ShopInventory& shop)
    {
        const std::size_t separator = shop.getName().find(" — ");
        if (separator == std::string::npos) return getVendorNameForShop(shop.getType());
        return shop.getName().substr(0, separator);
    }

    std::string temporaryMerchantSellingStyle(const ShopInventory& shop)
    {
        const std::size_t separator = shop.getName().find(" — ");
        if (separator == std::string::npos) return "vente itinérante";
        return shop.getName().substr(separator + std::string(" — ").size());
    }

    struct ShopPromotionOffer
    {
        bool active = false;
        bool clearance = false;
        std::string itemId;
        std::string itemName;
        int discountPercent = 0;
        int dayInOffer = 0;
        int daysRemaining = 0;
        int quantityLimit = -1;
        int purchasedQuantity = 0;
        std::string purchaseKey;

        int remainingDiscountedQuantity() const
        {
            if (!clearance || quantityLimit < 0) return 999;
            return std::max(0, quantityLimit - purchasedQuantity);
        }

        bool discountAvailable() const
        {
            return active && (!clearance || remainingDiscountedQuantity() > 0);
        }
    };

    std::vector<std::string> promotionPreferredItemIds(ShopType type)
    {
        switch (type)
        {
            case ShopType::MonsterMaterial: return {"goblin_ear", "wolf_fang"};
            case ShopType::Material: return {"rusted_metal_fragment", "worn_leather_piece"};
            case ShopType::Plant: return {"bitter_healing_leaf"};
            case ShopType::Armor: return {"worn_leather_armor"};
            case ShopType::Weapon: return {"rusty_sword", "training_bow"};
            case ShopType::Consumable: return {"survival_ration", "minor_healing_potion"};
            case ShopType::Library: return {"common_goblin_notes", "basic_magic_manual"};
            case ShopType::Blacksmith: return {"rusted_metal_fragment", "weak_repair_kit"};
            case ShopType::Alchemist: return {"minor_healing_potion", "antidote_potion"};
            case ShopType::Enchanter: return {"arcane_dust", "runic_stabilizer"};
            case ShopType::CityService: return {"city_service_stamp", "local_service_letter", "municipal_proof_letter"};
            case ShopType::Lodging: return {"survival_ration", "fire_lantern"};
            case ShopType::Transport: return {"survival_ration", "travel_distance_mark"};
            case ShopType::Church: return {"holy_water_vial", "sanctuary_candle"};
            case ShopType::BlackMarket: return {"black_market_barter_seal"};
            default: return {};
        }
    }

    ShopPromotionOffer promotionForShop(const ShopInventory& shop, const Player& player)
    {
        ShopPromotionOffer offer;
        if (shop.getType() == ShopType::Unknown || shop.getItems().empty()) return offer;

        const int day = std::max(0, player.getWorldDaysElapsed());
        const std::uint32_t shopHash = stableShopHash(shop.getName());
        const int startWeekday = static_cast<int>(shopHash % 7u);
        const int weekday = day % 7;
        const int delta = (weekday - startWeekday + 7) % 7;
        if (delta >= 3) return offer;

        const int cycleAnchorDay = day - delta;
        const int cycleIndex = cycleAnchorDay >= 0 ? cycleAnchorDay / 7 : -1;
        std::vector<std::string> availableIds;
        const std::vector<std::string> preferredIds = promotionPreferredItemIds(shop.getType());
        for (const std::string& preferredId : preferredIds)
        {
            for (const ShopItem& item : shop.getItems())
            {
                if (item.getId() == preferredId && item.getBuyPrice() > 0)
                {
                    availableIds.push_back(preferredId);
                    break;
                }
            }
        }
        if (availableIds.empty())
        {
            for (const ShopItem& item : shop.getItems())
            {
                if (item.getBuyPrice() > 0) availableIds.push_back(item.getId());
            }
        }
        if (availableIds.empty()) return offer;

        const std::size_t itemIndex = static_cast<std::size_t>((shopHash + static_cast<std::uint32_t>(cycleIndex + 31)) % availableIds.size());
        offer.itemId = availableIds[itemIndex];
        for (const ShopItem& item : shop.getItems())
        {
            if (item.getId() == offer.itemId)
            {
                offer.itemName = item.getName();
                break;
            }
        }

        const std::uint32_t cycleHash = stableShopHash(shop.getName() + "#" + std::to_string(cycleAnchorDay));
        offer.active = true;
        offer.clearance = cycleHash % 3u == 0u;
        offer.discountPercent = offer.clearance
            ? 18 + static_cast<int>(cycleHash % 8u)
            : 10 + static_cast<int>(cycleHash % 6u);
        offer.dayInOffer = delta + 1;
        offer.daysRemaining = 3 - delta;
        offer.quantityLimit = offer.clearance ? 2 + static_cast<int>((cycleHash / 7u) % 4u) : -1;
        offer.purchaseKey = "promo:" + shop.getName() + ":" + std::to_string(cycleAnchorDay) + ":" + offer.itemId;
        offer.purchasedQuantity = player.getShopPromotionPurchaseCount(offer.purchaseKey);
        return offer;
    }

    bool shopMatchesChapterThreeRoute(ShopType type, const std::string& route)
    {
        if (route == "commerce")
        {
            return type == ShopType::Material || type == ShopType::MonsterMaterial
                || type == ShopType::Weapon || type == ShopType::Armor
                || type == ShopType::Blacksmith || type == ShopType::Transport;
        }
        if (route == "secours")
        {
            return type == ShopType::Plant || type == ShopType::Consumable
                || type == ShopType::Alchemist || type == ShopType::Church
                || type == ShopType::Lodging || type == ShopType::CityService;
        }
        if (route == "recherche")
        {
            return type == ShopType::Library || type == ShopType::Alchemist
                || type == ShopType::Enchanter || type == ShopType::MonsterMaterial
                || type == ShopType::Material;
        }
        return false;
    }

    void applyChapterThreeShopConsequences(const Player& player, std::vector<ShopInventory>& shops)
    {
        if (!player.hasStoryModeStarted() || player.getStoryChapter() < 3)
        {
            return;
        }

        const std::string route = StoryCampaign::getChapterThreeRouteChoice(player);
        const std::string convoy = StoryCampaign::getChapterThreeConvoyDecision(player);

        for (ShopInventory& shop : shops)
        {
            int stockBonus = shopMatchesChapterThreeRoute(shop.getType(), route) ? 1 : 0;
            if (convoy == "marchandises") ++stockBonus;
            else if (convoy == "preuves" && (shop.getType() == ShopType::Library || shop.getType() == ShopType::Alchemist || shop.getType() == ShopType::Enchanter)) ++stockBonus;

            if (stockBonus <= 0) continue;
            for (ShopItem& item : shop.getMutableItems())
            {
                item.addStock(stockBonus);
            }
        }
    }

    struct BarterRequirement
    {
        std::string materialId;
        std::string label;
        int quantity;
    };

    struct SellableEntryUiInfo
    {
        std::string name;
        std::string quantity;
        std::string price;
        std::string maxQuantity;
        std::string status;
        std::string detail;
        std::string durability;
        std::string enchantmentSummary;
        std::string label;
        bool sellable = false;
    };

    std::string formatEquipmentDurabilityText(int durability, int maxDurability)
    {
        if (maxDurability < 0)
        {
            return "Durabilité : indestructible";
        }

        std::string result = "Durabilité : " + std::to_string(std::max(0, durability)) + "/" + std::to_string(std::max(0, maxDurability));
        if (durability <= 0)
        {
            result += " (cassé)";
        }
        else if (durability * 100 <= maxDurability * 25)
        {
            result += " (très abîmé)";
        }
        else if (durability * 100 <= maxDurability * 50)
        {
            result += " (abîmé)";
        }
        return result;
    }

    std::vector<BarterRequirement> getBlackMarketBarterRequirements(const ShopItem& item)
    {
        const std::string id = item.getId();

        if (id == "experimental_damage_potion")
        {
            return {
                {"slime_residue", "Résidu de slime", 4},
                {"arcane_dust", "Poussière arcanique", 2}
            };
        }

        if (id == "smoke_escape_vial")
        {
            return {
                {"slime_residue", "Résidu de slime", 3},
                {"shadow_thread", "Fil d'ombre", 1}
            };
        }

        if (id == "major_healing_potion")
        {
            return {
                {"bitter_healing_leaf", "Feuille amère de soin", 6},
                {"mountain_blue_flower", "Fleur bleue de montagne", 1}
            };
        }

        if (id == "greater_defensive_potion")
        {
            return {
                {"rusted_metal_fragment", "Fragment de métal rouillé", 4},
                {"arcane_dust", "Poussière arcanique", 2}
            };
        }

        if (id == "balanced_throwing_knives")
        {
            return {
                {"rusted_metal_fragment", "Fragment de métal rouillé", 2},
                {"worn_leather_piece", "Morceau de cuir abîmé", 1}
            };
        }

        if (id == "barbed_arrows")
        {
            return {
                {"rusted_metal_fragment", "Fragment de métal rouillé", 2},
                {"wolf_fang", "Croc de loup", 1}
            };
        }

        if (id == "piercing_bolts")
        {
            return {
                {"rusted_metal_fragment", "Fragment de métal rouillé", 3},
                {"cracked_bone", "Os fissuré", 1}
            };
        }

        if (id == "ash_arrows")
        {
            return {
                {"arcane_dust", "Poussière arcanique", 2},
                {"bitter_healing_leaf", "Feuille amère de soin", 2}
            };
        }

        if (id == "frozen_bolts")
        {
            return {
                {"mountain_blue_flower", "Fleur bleue de montagne", 2},
                {"arcane_dust", "Poussière arcanique", 1}
            };
        }

        if (id == "conductive_knives")
        {
            return {
                {"rusted_metal_fragment", "Fragment de métal rouillé", 3},
                {"unstable_core", "Noyau instable", 1}
            };
        }

        if (id == "venom_arrows")
        {
            return {
                {"bitter_healing_leaf", "Feuille amère de soin", 3},
                {"slime_residue", "Résidu de slime", 2}
            };
        }

        if (id == "shock_bolts")
        {
            return {
                {"rusted_metal_fragment", "Fragment de métal rouillé", 4},
                {"unstable_core", "Noyau instable", 1}
            };
        }

        if (id == "smoke_knives")
        {
            return {
                {"balanced_throwing_knives", "Couteaux équilibrés", 1},
                {"slime_residue", "Résidu de slime", 3}
            };
        }

        if (id == "unstable_core")
        {
            return {
                {"slime_residue", "Résidu de slime", 6},
                {"arcane_dust", "Poussière arcanique", 3}
            };
        }

        if (id == "shadow_thread")
        {
            return {
                {"wolf_fang", "Croc de loup", 3},
                {"arcane_dust", "Poussière arcanique", 2}
            };
        }

        if (id == "kitsune_ember")
        {
            return {
                {"mountain_blue_flower", "Fleur bleue de montagne", 2},
                {"arcane_dust", "Poussière arcanique", 4}
            };
        }

        if (id == "draconic_scale_fragment")
        {
            return {
                {"beast_hide", "Peau de bête", 3},
                {"rusted_metal_fragment", "Fragment de métal rouillé", 5}
            };
        }

        if (id == "precision_harvest_tools")
        {
            return {
                {"worn_leather_piece", "Morceau de cuir abîmé", 3},
                {"rusted_metal_fragment", "Fragment de métal rouillé", 6},
                {"arcane_dust", "Poussière arcanique", 1}
            };
        }

        if (id == "preservation_vials")
        {
            return {
                {"slime_residue", "Résidu de slime", 4},
                {"arcane_dust", "Poussière arcanique", 4}
            };
        }

        if (id == "anomaly_glitch_fragment")
        {
            return {
                {"unstable_core", "Noyau instable", 1},
                {"shadow_thread", "Fil d'ombre", 2},
                {"arcane_dust", "Poussière arcanique", 5}
            };
        }

        if (id == "tinkerer_complete_repair_kit")
        {
            return {
                {"medium_repair_kit", "Kit de réparation moyen", 1},
                {"draconic_scale_fragment", "Fragment d'écaille draconique", 1},
                {"rusted_metal_fragment", "Fragment de métal rouillé", 8}
            };
        }

        if (id == "sealed_debt_slip")
        {
            return {
                {"smuggler_token", "Jeton de contrebandier", 1},
                {"local_service_letter", "Lettre de service local", 2}
            };
        }

        if (id == "minor_purification_scroll")
        {
            return {
                {"holy_water_vial", "Fiole d'eau bénite", 1},
                {"arcane_dust", "Poussière arcanique", 3}
            };
        }

        if (id == "resistance_rift_scroll")
        {
            return {
                {"runic_iron_shard", "Éclat de fer runique", 1},
                {"arcane_dust", "Poussière arcanique", 4}
            };
        }

        if (id == "crawling_venom_scroll")
        {
            return {
                {"venom_arrows", "Flèches empoisonnées", 1},
                {"bitter_healing_leaf", "Feuille amère de soin", 5}
            };
        }

        if (id == "resistance_rift_grimoire")
        {
            return {
                {"runic_extraction_note", "Note d'extraction runique", 2},
                {"unstable_core", "Noyau instable", 1},
                {"arcane_dust", "Poussière arcanique", 8}
            };
        }

        return {};
    }

    bool hasBlackMarketBarterOffer(const ShopInventory& shop, const ShopItem& item)
    {
        return shop.getType() == ShopType::BlackMarket
            && ShopTransactionSystem::canBeBoughtNow(item)
            && !getBlackMarketBarterRequirements(item).empty();
    }

    int getMaxBarterQuantity(const ShopItem& item, const Player& player)
    {
        if (!ShopTransactionSystem::canBeBoughtNow(item) || item.isSoldOut())
        {
            return 0;
        }

        std::vector<BarterRequirement> requirements = getBlackMarketBarterRequirements(item);
        if (requirements.empty())
        {
            return 0;
        }

        int maxQuantity = item.getStock() > 0 ? item.getStock() : 99;

        for (const BarterRequirement& requirement : requirements)
        {
            if (requirement.quantity <= 0)
            {
                continue;
            }

            int available = player.getInventory().countMaterialById(requirement.materialId);
            maxQuantity = std::min(maxQuantity, available / requirement.quantity);
        }

        if (item.isCommonInformation())
        {
            maxQuantity = std::min(maxQuantity, 1);
        }

        return std::max(0, maxQuantity);
    }

    std::string formatBarterRequirements(const ShopItem& item, int multiplier = 1)
    {
        std::vector<BarterRequirement> requirements = getBlackMarketBarterRequirements(item);
        std::string text;

        multiplier = std::max(1, multiplier);

        for (std::size_t i = 0; i < requirements.size(); ++i)
        {
            if (i > 0)
            {
                text += " + ";
            }

            text += requirements[i].label + " x" + std::to_string(requirements[i].quantity * multiplier);
        }

        return text;
    }

    bool consumeBarterRequirements(Player& player, const ShopItem& item, int quantity)
    {
        if (quantity <= 0)
        {
            return false;
        }

        std::vector<BarterRequirement> requirements = getBlackMarketBarterRequirements(item);
        if (requirements.empty())
        {
            return false;
        }

        for (const BarterRequirement& requirement : requirements)
        {
            if (player.getInventory().countMaterialById(requirement.materialId) < requirement.quantity * quantity)
            {
                return false;
            }
        }

        for (const BarterRequirement& requirement : requirements)
        {
            if (!player.getInventory().removeMaterialQuantityById(requirement.materialId, requirement.quantity * quantity))
            {
                return false;
            }
        }

        return true;
    }

    void refundBarterRequirements(Player& player, const ShopItem& item, int quantity)
    {
        if (quantity <= 0)
        {
            return;
        }

        for (const BarterRequirement& requirement : getBlackMarketBarterRequirements(item))
        {
            if (requirement.quantity <= 0)
            {
                continue;
            }

            player.getInventory().addMaterial(
                MaterialCatalog::createById(requirement.materialId, requirement.quantity * quantity)
            );
        }
    }

    std::string getVendorNameForShop(ShopType type)
    {
        switch (type)
        {
            case ShopType::MonsterMaterial:
                return "Vendeur de composants";
            case ShopType::Material:
                return "Vendeur de matériaux";
            case ShopType::Plant:
                return "Herboriste";
            case ShopType::Armor:
                return "Armurier";
            case ShopType::Weapon:
                return "Vendeur d'armes";
            case ShopType::Consumable:
                return "Vendeur de consommables";
            case ShopType::Library:
                return "Bibliothécaire";
            case ShopType::Blacksmith:
                return "Forgeron";
            case ShopType::Alchemist:
                return "Alchimiste";
            case ShopType::Enchanter:
                return "Enchanteur";
            case ShopType::CityService:
                return "Scribe Ysolde";
            case ShopType::Lodging:
                return "Tavia l'aubergiste";
            case ShopType::Transport:
                return "Noro le palefrenier";
            case ShopType::Church:
                return "Sœur Maëlys l’exorciste";
            case ShopType::BlackMarket:
                return "Contact du marché noir";
            default:
                return "Marchand inquiet";
        }
    }


    std::string chooseRandomLine(const std::vector<std::string>& lines)
    {
        if (lines.empty())
        {
            return "";
        }

        static std::mt19937 generator(std::random_device{}());
        std::uniform_int_distribution<int> distribution(0, static_cast<int>(lines.size()) - 1);
        return lines[distribution(generator)];
    }

    std::string chooseShopIntroLine(ShopType type)
    {
        if (type == ShopType::CityService)
        {
            return chooseRandomLine({
                "Le scribe tamponne trois papiers avant même de lever les yeux. Ici, l'aventure commence par une file d'attente.",
                "Un guichet grince, une plume gratte, et quelqu'un murmure qu'un formulaire mal rempli peut tuer une quête plus vite qu'un gobelin.",
                "Le bureau sent l'encre, la cire et la peur administrative. Étrangement, c'est presque rassurant."
            });
        }

        if (type == ShopType::Lodging)
        {
            return chooseRandomLine({
                "Tavia essuie le comptoir : repas, lits, plaintes et rumeurs passent tous par la même table collante.",
                "L'auberge bruisse de voyageurs. Certains viennent dormir, d'autres viennent juste prouver qu'ils savent lire l'addition.",
                "Une marmite chante au fond. Personne ne sait si c'est bon signe, mais au moins ça sent meilleur que la route."
            });
        }

        if (type == ShopType::Transport)
        {
            return chooseRandomLine({
                "Noro parle de roues, de ponts, de pass et de gardes comme si chaque trajet était une négociation avec le hasard.",
                "Une carte de routes est couverte de traits rouges. Noro dit que ce sont les détours sûrs, puis ajoute qu'il n'y a pas vraiment de routes sûres.",
                "Le relais sent la paille et la boue. Très peu héroïque, mais très utile pour éviter de mourir entre deux villages."
            });
        }

        if (type == ShopType::Church)
        {
            return chooseRandomLine({
                "L'église est calme, mais pas vide : Sœur Maëlys surveille les cierges comme s'ils pouvaient mentir.",
                "Père Orwan parle bas près de l'autel, tandis que Frère Calixte range des notes de bénédiction.",
                "L'odeur de cire, d'encens et de pierre froide couvre presque celle des problèmes maudits."
            });
        }

        if (type == ShopType::BlackMarket)
        {
            return chooseRandomLine({
                "Un rideau se ferme derrière toi. Le contact ne demande pas ton nom, ce qui est rarement bon signe.",
                "Le contact tapote le comptoir : ici, les garanties durent moins longtemps que les mensonges.",
                "Une odeur de métal froid flotte dans l'air. Même les prix ont l'air de cacher quelque chose."
            });
        }

        if (type == ShopType::Library)
        {
            return chooseRandomLine({
                "La bibliothécaire relève les yeux : les livres dangereux sont rangés assez haut pour décourager les idiots motivés.",
                "Des pages bougent toutes seules dans un coin. Personne ne commente, donc tu fais pareil.",
                "Le silence ici pèse plus lourd qu'une armure, mais au moins il ne coûte pas encore de taxe."
            });
        }

        if (type == ShopType::Weapon || type == ShopType::Blacksmith)
        {
            return chooseRandomLine({
                "Le métal chante derrière le comptoir. Le vendeur sourit comme si une bonne lame réglait tous les débats.",
                "On te jauge les bras avant de te montrer les articles. Apparemment, le style ne suffit pas à porter une hache.",
                "Un client teste une lame dans le vide. Tout le monde fait semblant que c'était maîtrisé."
            });
        }

        if (type == ShopType::Armor)
        {
            return chooseRandomLine({
                "L'armurier tape sur une cuirasse : si ça sonne creux, c'est soit fragile, soit toi dedans.",
                "Des protections cabossées attendent réparation. Certaines ont clairement vécu une meilleure histoire que leur propriétaire.",
                "Le vendeur inspecte tes épaules comme s'il savait déjà où le prochain monstre va mordre."
            });
        }

        if (type == ShopType::Enchanter)
        {
            return chooseRandomLine({
                "Des runes faibles brillent sur le comptoir. L'enchanteur précise que 'faible' veut dire 'mieux que brûler'.",
                "L'enchanteur vérifie tes équipements comme s'il écoutait leur température intérieure.",
                "Ici, on ne promet pas l'immunité. On vend surtout quelques secondes de survie en plus."
            });
        }

        if (type == ShopType::Plant || type == ShopType::Alchemist || type == ShopType::Consumable)
        {
            return chooseRandomLine({
                "Des flacons frémissent doucement. L'étiquette 'ne pas boire' semble surtout être une suggestion juridique.",
                "L'odeur des plantes couvre presque celle des expériences ratées. Presque.",
                "Le vendeur range une fiole trop vite. Tu décides de ne pas demander ce qu'elle faisait avant ton arrivée."
            });
        }

        return chooseRandomLine({
            "Le marchand t'accueille avec le sourire prudent de quelqu'un qui a déjà vu des aventuriers compter jusqu'à trois avec difficulté.",
            "Le comptoir craque sous les marchandises. Lui, au moins, a une barre de durabilité réaliste.",
            "Quelques clients chuchotent. Visiblement, ici aussi, ton inventaire intéresse plus de monde que ta santé mentale."
        });
    }


    std::vector<std::string> chooseVendorTalkLines(ShopType type)
    {
        std::vector<std::string> lines;

        if (type == ShopType::MonsterMaterial)
        {
            lines.push_back(chooseRandomLine({
                "Le vendeur aligne trois griffes sur le comptoir : deux sont utiles, la troisième est probablement juste là pour impressionner les débutants.",
                "Il explique que les bons composants ne sentent pas toujours bon, mais que les composants trop propres mentent souvent.",
                "Il conseille de noter quelle créature a donné quoi : dans ce métier, confondre une dent et une écaille finit rarement bien."
            }));
            lines.push_back("Rumeur : certaines carcasses réagissent mieux si le coup final n'a pas broyé la matière intéressante.");
            return lines;
        }

        if (type == ShopType::Material || type == ShopType::Blacksmith)
        {
            lines.push_back(chooseRandomLine({
                "Le vendeur parle densité, veines de métal et réparations comme si tout le monde rêvait de dormir dans une forge.",
                "Il te montre une fissure presque invisible : selon lui, c'est là que la moitié des aventuriers perdent leur argent avant de perdre leur bras.",
                "Le comptoir porte des marques de test. Visiblement, taper sur les choses reste une méthode scientifique locale."
            }));
            lines.push_back("Conseil : une arme qui frappe une matière trop dure s'use plus vite, même si elle gagne le duel sur le moment.");
            return lines;
        }

        if (type == ShopType::Weapon)
        {
            lines.push_back(chooseRandomLine({
                "Le vendeur affirme qu'une arme choisie au hasard est une arme qui cherche déjà son prochain propriétaire.",
                "Il parle équilibre, portée et rythme. Puis il regarde ton inventaire avec la tête de quelqu'un qui a envie de tout ranger lui-même.",
                "Il rappelle qu'une lame héroïque mérite un minimum de respect, et idéalement quelqu'un qui sait de quel côté elle coupe."
            }));
            lines.push_back("Rumeur : quelques combattants apprennent de nouvelles techniques seulement après avoir vraiment insisté avec le même type d'arme.");
            return lines;
        }

        if (type == ShopType::Armor)
        {
            lines.push_back(chooseRandomLine({
                "L'armurier assure que la meilleure armure est celle qu'on remarque avant que le monstre ne remarque tes côtes.",
                "Il décrit des protections légères, lourdes, souples, puis soupire en disant que personne ne lit les faiblesses avant le premier impact.",
                "Il tape sur une épaulière : le bruit est rassurant, ou inquiétant, selon ton optimisme."
            }));
            lines.push_back("Conseil : encaisser un choc trop violent peut abîmer l'équipement, même si les PV tiennent encore debout.");
            return lines;
        }

        if (type == ShopType::Plant || type == ShopType::Alchemist || type == ShopType::Consumable)
        {
            lines.push_back(chooseRandomLine({
                "L'herboriste parle de dosage avec le calme d'une personne qui a déjà vu quelqu'un boire une potion offensive par curiosité.",
                "L'alchimiste explique que la couleur d'une fiole ne garantit rien, sauf peut-être la couleur de la panique après usage.",
                "Le vendeur conseille de garder une potion de soin rapide séparée du reste. Il dit ça comme si la survie aimait les raccourcis propres."
            }));
            lines.push_back("Rumeur : certains mélanges rares demanderont des plantes et matériaux que les boutiques ne vendent presque jamais ensemble.");
            return lines;
        }

        if (type == ShopType::Library)
        {
            lines.push_back(chooseRandomLine({
                "La bibliothécaire baisse la voix : certains grimoires ne lancent pas un sort, ils apprennent au lecteur à le mériter.",
                "Elle range un livre qui semble respirer. Elle prétend que c'est normal. Le livre n'a pas l'air d'accord.",
                "Elle rappelle que connaître une faiblesse avant de frapper coûte moins cher qu'apprendre la même chose avec son visage."
            }));
            lines.push_back("Conseil : le bestiaire et les renseignements doivent rester la base des recommandations tactiques, sinon c'est juste de la triche mal habillée.");
            return lines;
        }

        if (type == ShopType::CityService)
        {
            lines.push_back(chooseRandomLine({
                "Le scribe explique qu'un bon tampon ne rend pas brave, mais qu'il évite souvent de refaire trois fois la même demande.",
                "Il parle dossiers, plaintes et attestations. Chaque phrase donne envie de mieux ranger l'inventaire.",
                "Il montre un casier de notes locales : apparemment, même les petits services finissent par peser dans une réputation."
            }));
            lines.push_back("Conseil : les notes locales et lettres de service comptent surtout quand elles s'accumulent avec des demandes PNJ réussies.");
            return lines;
        }

        if (type == ShopType::Lodging)
        {
            lines.push_back(chooseRandomLine({
                "Tavia jure qu'une bonne auberge sauve plus d'aventuriers qu'un long discours héroïque.",
                "Elle explique que les repas, lits et tickets d'écurie servent parfois de petites faveurs au lieu de sortir des pièces pour tout.",
                "Elle prévient qu'une plainte bien formulée va plus loin qu'un cri dans la salle commune."
            }));
            lines.push_back("Rumeur : certaines demandes de ville préféreront une preuve propre, un bon d'auberge ou une note locale plutôt qu'une prime brute.");
            return lines;
        }

        if (type == ShopType::Transport)
        {
            lines.push_back(chooseRandomLine({
                "Noro compte les roues, les jours et les gardes. Il dit que le vrai monstre, c'est la route mal préparée.",
                "Il explique qu'un pass n'empêche pas une attaque, mais qu'il évite de perdre une matinée contre un garde borné.",
                "Il te montre des reçus de péage : petits papiers, gros soupirs, routes un peu moins pénibles."
            }));
            lines.push_back("Conseil : les tickets de transport coûtent plus cher qu'une rumeur, mais restent moins absurdes qu'un convoi payé en or massif.");
            return lines;
        }

        if (type == ShopType::Church)
        {
            lines.push_back(chooseRandomLine({
                "Sœur Maëlys explique que les petites malédictions se lavent vite, mais que les longues demandent de revenir plusieurs jours sans oublier.",
                "Père Orwan précise qu'une malédiction liée à un boss ne se négocie pas toujours avec un cierge : parfois, il faut retourner battre la source.",
                "Frère Calixte parle de billets laissés aux cierges : joueurs, gardes, marchands ou habitants peuvent tous porter une trace sans savoir la nommer."
            }));
            lines.push_back("Rumeur : la Marque de proie de Lyknir peut être reconnue par l'église, mais elle ne cède vraiment que si Lyknir est vaincu.");
            return lines;
        }

        if (type == ShopType::BlackMarket)
        {
            lines.push_back(chooseRandomLine({
                "Le contact parle de stocks oubliés, d'objets sans facture et de garanties qui s'évaporent dès qu'on les relit.",
                "Il te conseille de ne pas poser de questions, ce qui est exactement le genre de phrase qui donne envie d'en poser douze.",
                "Il glisse que les meilleurs prix ne sont pas toujours en or. Parfois, ils coûtent surtout en problèmes futurs."
            }));
            lines.push_back("Rumeur : quelques marchandises spéciales n'apparaissent qu'après des combats ou événements assez rares.");
            return lines;
        }

        lines.push_back(chooseRandomLine({
            "Le marchand parle de clients, de routes et de taxes avec l'énergie de quelqu'un qui a survécu à pire qu'un monstre : la comptabilité.",
            "Il dit que le monde change vite après chaque combat, surtout les prix, les stocks et les excuses des vendeurs.",
            "Il te conseille de ne pas tout acheter juste parce que ça brille. Puis il ajoute que si tu le fais quand même, il ne jugera pas trop fort."
        }));
        lines.push_back("Conseil : reviens après quelques combats, les étals peuvent changer et certaines occasions ne restent pas longtemps.");
        return lines;
    }

    MenuScreen buildShopConfirmationScreen(
        const std::string& title,
        const std::string& screenId,
        const std::vector<std::string>& lines,
        const std::string& confirmLabel,
        const std::string& cancelLabel,
        const std::string& actionPrefix
    )
    {
        MenuScreen screen(title, screenId);
        screen.setChoiceInput("Choisis 1 pour confirmer ou 2 pour annuler.");

        for (const std::string& line : lines)
        {
            screen.addLine(line);
        }

        screen.addOption(1, confirmLabel, "Valider l'action affichée.", true, actionPrefix + ".confirm");
        screen.addOption(2, cancelLabel, "Revenir sans rien changer.", true, actionPrefix + ".cancel");
        return screen;
    }

    bool askShopConfirmation(
        const std::string& title,
        const std::string& screenId,
        const std::vector<std::string>& lines,
        const std::string& confirmLabel,
        const std::string& cancelLabel,
        const std::string& actionPrefix
    )
    {
        Console::clear();
        const MenuScreen screen = buildShopConfirmationScreen(
            title,
            screenId,
            lines,
            confirmLabel,
            cancelLabel,
            actionPrefix
        );

        const int choice = TerminalInterface::askMenuChoiceFromOptions(
            screen,
            "Choix refusé : utilise 1 pour confirmer ou 2 pour annuler."
        );
        return choice == 1;
    }

    std::vector<std::string> withTransactionNotes(std::vector<std::string> lines)
    {
        const std::vector<std::string> notes = ShopTransactionSystem::consumeLastTransactionNotes();
        if (!notes.empty())
        {
            lines.push_back("");
            lines.push_back("Détails de transaction :");
            for (const std::string& note : notes)
            {
                lines.push_back("- " + note);
            }
        }
        return lines;
    }

    void showShopResult(
        const std::string& title,
        const std::string& screenId,
        const std::vector<std::string>& lines
    )
    {
        Console::clear();
        MessageScreen::show(title, screenId, lines);
    }

    void showShopTransactionResult(
        const std::string& title,
        const std::string& screenId,
        std::vector<std::string> lines
    )
    {
        showShopResult(title, screenId, withTransactionNotes(lines));
    }


    int countMaterial(const Player& player, const std::string& id)
    {
        return player.getInventory().countMaterialById(id);
    }

    using LocalReputationSummary = LocalReputationResult;

    LocalReputationSummary localReputationForPlayer(const Player& player)
    {
        return LocalReputationSystem::evaluate(player, player.getCurrentCityId());
    }

    bool isLocalServiceShop(ShopType type)
    {
        return type == ShopType::CityService
            || type == ShopType::Lodging
            || type == ShopType::Transport
            || type == ShopType::Church;
    }

    bool isStorySupplyShop(ShopType type)
    {
        return type == ShopType::Plant
            || type == ShopType::Consumable
            || type == ShopType::Alchemist
            || type == ShopType::Material
            || type == ShopType::Blacksmith
            || type == ShopType::Weapon
            || type == ShopType::Armor
            || type == ShopType::Transport
            || type == ShopType::CityService;
    }

    bool isShopUnlockedForStory(const Player& player, ShopType type)
    {
        if (!player.hasStoryModeStarted() || player.hasStorySkip())
        {
            return true;
        }

        if (type == ShopType::BlackMarket || type == ShopType::Unknown)
        {
            return false;
        }

        const int chapter = player.getStoryChapter();
        const int step = player.getStoryStep();

        // Avant la tournée des premiers référents, aucun comptoir commercial n'est encore proposé.
        if (chapter <= 1)
        {
            return step >= 4 && (type == ShopType::Plant || type == ShopType::Blacksmith);
        }

        // Les premiers services reviennent avec les personnes réellement rencontrées.
        if (type == ShopType::Plant || type == ShopType::Blacksmith)
        {
            return true;
        }
        if (type == ShopType::Transport)
        {
            return step >= 7;
        }

        // La majorité des comptoirs ne réapparaît qu'après la relance concrète de la ville.
        if (step >= 9
            && (type == ShopType::MonsterMaterial || type == ShopType::Material || type == ShopType::Armor
                || type == ShopType::Weapon || type == ShopType::Consumable || type == ShopType::Alchemist
                || type == ShopType::CityService || type == ShopType::Lodging))
        {
            return true;
        }
        if (step >= 10 && type == ShopType::Library)
        {
            return true;
        }
        if (step >= 12 && type == ShopType::Church)
        {
            return true;
        }
        if (step >= 16 && type == ShopType::Enchanter)
        {
            return true;
        }

        return false;
    }

    int storyCitySupplyModifierPercent(const Player& player, ShopType type)
    {
        if (!player.hasStoryModeStarted() || player.getStoryChapter() < 2 || !isStorySupplyShop(type) || type == ShopType::BlackMarket)
        {
            return 0;
        }

        int discount = 0;
        if (player.getStoryStep() >= 8 && (type == ShopType::Transport || type == ShopType::CityService))
        {
            discount += 1;
        }
        if (player.getStoryStep() >= 9)
        {
            if (type == ShopType::Plant || type == ShopType::Consumable || type == ShopType::Alchemist)
            {
                discount += 2;
            }
            if (type == ShopType::Material || type == ShopType::Blacksmith || type == ShopType::Weapon || type == ShopType::Armor)
            {
                discount += 1;
            }
            if (type == ShopType::Transport || type == ShopType::CityService)
            {
                discount += 1;
            }
        }
        if (player.getStoryStep() >= 10 && (type == ShopType::Transport || type == ShopType::Material || type == ShopType::Plant))
        {
            discount += 1;
        }
        if (player.getStoryStep() >= 12 && (type == ShopType::Consumable || type == ShopType::CityService || type == ShopType::Transport))
        {
            discount += 1;
        }
        if (player.getStoryStep() >= 16 && (type == ShopType::Blacksmith || type == ShopType::Armor || type == ShopType::Weapon || type == ShopType::Material))
        {
            discount += 1;
        }
        if (player.getStoryStep() >= 18 && (type == ShopType::Transport || type == ShopType::CityService || type == ShopType::Consumable || type == ShopType::Plant))
        {
            discount += 1;
        }
        return std::min(5, discount);
    }

    std::string storyCityDevelopmentShopLine(const Player& player, ShopType type)
    {
        if (!player.hasStoryModeStarted())
        {
            return "";
        }

        std::string line = "Ville : palier " + std::to_string(player.getStoryCityDevelopmentLevel())
            + " | chapitre " + std::to_string(player.getStoryChapter())
            + ", étape " + std::to_string(player.getStoryStep()) + ". ";

        if (player.getStoryChapter() < 2)
        {
            line += "Stocks encore pauvres : la ville connaît les référents, mais les routes restent fermées.";
        }
        else if (player.getStoryStep() < 7)
        {
            line += "Relais en cours : les comptoirs restent prudents tant que la route ne ramène personne.";
        }
        else if (player.getStoryStep() < 8)
        {
            line += "Nell est sauvée : les rumeurs de route deviennent crédibles, mais la sacoche n'est pas encore exploitée.";
        }
        else if (player.getStoryStep() < 9)
        {
            line += "Sacoche exploitée : les comptoirs attendent que Mira répartisse les informations.";
        }
        else if (player.getStoryStep() < 10)
        {
            line += "Comptoirs relancés : routes courtes, plantes simples, plaques de forge et contrats terrain deviennent plus crédibles.";
        }
        else if (player.getStoryStep() < 11)
        {
            line += "Encre froide classée : les stocks respirent, mais les marchands parlent déjà d'une route réécrite.";
        }
        else if (player.getStoryStep() < 12)
        {
            line += "Route réécrite prouvée : les marchands vérifient les cartes avec Nell et Soryn avant de promettre un trajet.";
        }
        else if (player.getStoryStep() < 13)
        {
            line += "Contre-registre actif : les stocks suivent les retours réels, les marques de relais et les témoins, pas seulement les cartes.";
        }
        else if (player.getStoryStep() < 15)
        {
            line += "Nœud noir repéré : la ville prépare portes, soins et retours avant de viser ce qui garde la borne.";
        }
        else if (player.getStoryStep() < 16)
        {
            line += "Menace de borne confirmée : les comptoirs vendent avec prudence, comme avant une sortie dont personne ne connaît le vrai nom.";
        }
        else if (player.getStoryStep() < 17)
        {
            line += "Verrou de borne brisé : Bram et les marchands osent renforcer un peu plus les sorties de route, sans croire la crise terminée.";
        }
        else if (player.getStoryStep() < 18)
        {
            line += "Cicatrices du verrou classées : Soryn et Nell savent quelles marques surveiller avant chaque départ court.";
        }
        else
        {
            line += "Route gardée : les premiers retours confirmés améliorent un peu les comptoirs, mais personne ne parle encore de route sûre.";
        }

        const int storyDiscount = storyCitySupplyModifierPercent(player, type);
        if (storyDiscount > 0)
        {
            line += " Effet léger : -" + std::to_string(storyDiscount) + "% sur ce comptoir grâce aux routes courtes.";
        }

        return line;
    }

    std::string shopOpeningMomentsText(ShopType type)
    {
        switch (type)
        {
            case ShopType::Lodging:
                return "matin, midi, après-midi, soir, nuit";
            case ShopType::BlackMarket:
                return "soir, nuit";
            case ShopType::Transport:
                return "matin, midi, après-midi, soir";
            case ShopType::CityService:
                return "matin, midi, après-midi";
            case ShopType::Church:
                return "matin, midi, après-midi, soir";
            case ShopType::Enchanter:
                return "midi, après-midi, soir";
            case ShopType::Library:
                return "matin, midi, après-midi";
            case ShopType::Blacksmith:
                return "matin, midi";
            case ShopType::Weapon:
            case ShopType::Armor:
            case ShopType::Material:
                return "matin, midi, après-midi";
            case ShopType::MonsterMaterial:
                return "midi, après-midi, soir";
            case ShopType::Plant:
            case ShopType::Alchemist:
            case ShopType::Consumable:
                return "matin, midi, après-midi, soir";
            default:
                return "matin, midi, après-midi";
        }
    }

    bool shopIsOpenAtMoment(ShopType type, int momentIndex)
    {
        momentIndex = std::max(0, std::min(momentIndex, 4));

        if (type == ShopType::Lodging)
        {
            return true;
        }

        if (type == ShopType::Church)
        {
            return momentIndex <= 3;
        }

        if (type == ShopType::BlackMarket)
        {
            return momentIndex >= 3;
        }

        if (type == ShopType::Transport
            || type == ShopType::Church
            || type == ShopType::Plant
            || type == ShopType::Alchemist
            || type == ShopType::Consumable)
        {
            return momentIndex <= 3;
        }

        if (type == ShopType::Enchanter)
        {
            return momentIndex >= 1 && momentIndex <= 3;
        }

        if (type == ShopType::Blacksmith)
        {
            return momentIndex <= 1;
        }

        if (type == ShopType::MonsterMaterial)
        {
            return momentIndex >= 1 && momentIndex <= 3;
        }

        return momentIndex <= 2;
    }

    bool isCityRepairEmergencyDesk(ShopType type)
    {
        return type == ShopType::Lodging || type == ShopType::Church || type == ShopType::CityService;
    }

    bool isCityRepairRotatingOpenShop(ShopType type, const Player& player)
    {
        if (isCityRepairEmergencyDesk(type))
        {
            return true;
        }

        static const std::vector<ShopType> repairRotation = {
            ShopType::Material,
            ShopType::Blacksmith,
            ShopType::Consumable,
            ShopType::Plant,
            ShopType::Transport,
            ShopType::Armor,
            ShopType::Weapon,
            ShopType::Alchemist,
            ShopType::MonsterMaterial,
            ShopType::Library,
            ShopType::Enchanter
        };

        const int day = std::max(0, player.getWorldDaysElapsed());
        const std::size_t first = static_cast<std::size_t>(day % static_cast<int>(repairRotation.size()));
        const std::size_t second = static_cast<std::size_t>((day * 3 + 2) % static_cast<int>(repairRotation.size()));
        return type == repairRotation[first] || type == repairRotation[second];
    }

    bool shopIsOpenForPlayer(const ShopInventory& shop, const Player& player)
    {
        if (cityRepairDaysRemaining(player) > 0 && !isCityRepairRotatingOpenShop(shop.getType(), player))
        {
            return false;
        }

        return shopIsOpenAtMoment(shop.getType(), player.getWorldDayProgressUnits());
    }

    std::string shopOpenStatusLine(const ShopInventory& shop, const Player& player)
    {
        std::string line = std::string("Horaires : ") + shopOpeningMomentsText(shop.getType()) + ". ";
        const int repairDays = cityRepairDaysRemaining(player);
        if (repairDays > 0 && !isCityRepairRotatingOpenShop(shop.getType(), player))
        {
            line += "Statut actuel : fermé pour réparations de ville (" + std::to_string(repairDays) + " jour(s) restant(s)).";
            return line;
        }

        line += shopIsOpenForPlayer(shop, player)
            ? "Statut actuel : ouvert."
            : "Statut actuel : fermé, repasse à un moment compatible.";
        if (repairDays > 0)
        {
            line += isCityRepairEmergencyDesk(shop.getType())
                ? " Service maintenu en priorité pendant les réparations."
                : " Comptoir ouvert exceptionnellement malgré les réparations.";
        }
        return line;
    }

    int localReputationDiscountForShop(const Player& player, ShopType type)
    {
        if (!isLocalServiceShop(type))
        {
            return 0;
        }

        return localReputationForPlayer(player).discountPercent;
    }

    int localReputationSurchargeForShop(const Player& player, ShopType type)
    {
        if (type == ShopType::BlackMarket || type == ShopType::Unknown)
        {
            return 0;
        }

        const int surcharge = localReputationForPlayer(player).surchargePercent;
        return isLocalServiceShop(type) ? surcharge : (surcharge + 1) / 2;
    }

    bool canShopReceiveCityDefenseGratitudeDiscount(ShopType type)
    {
        return type != ShopType::BlackMarket && type != ShopType::Unknown;
    }

    int cityDefenseGratitudeDiscountPercent(const Player& player, ShopType type)
    {
        if (!canShopReceiveCityDefenseGratitudeDiscount(type))
        {
            return 0;
        }
        return player.getInventory().countMaterialById("city_defense_gratitude_days_marker") > 0 ? 4 : 0;
    }

    int scheduledCityActivityBuyModifierPercent(const Player& player, ShopType type)
    {
        if (cityRepairDaysRemaining(player) > 0 || type == ShopType::BlackMarket || type == ShopType::Unknown)
        {
            return 0;
        }

        // Économie contrôlée : les affiches de ville influencent un peu les prix,
        // mais jamais assez pour devenir une stratégie de farm. Les affiches royales
        // exceptionnelles peuvent aussi peser légèrement, mais seulement le jour même.
        const int day = std::max(0, player.getWorldDaysElapsed());
        if (hasRoyalBonusCityEventToday(player))
        {
            const CityEventOfDay royalEvent = cityEventForAbsoluteDay(day, &player);
            if (royalEvent.id == "royal_supply_day"
                && (type == ShopType::Consumable || type == ShopType::Plant || type == ShopType::Alchemist))
            {
                return -2;
            }
            if (royalEvent.id == "royal_patrol_gratitude"
                && (type == ShopType::Weapon || type == ShopType::Armor || type == ShopType::Blacksmith))
            {
                return 1;
            }
            if (royalEvent.id == "royal_merit_reward" && type == ShopType::CityService)
            {
                return -1;
            }
            return 0;
        }
        if (day % 7 != 0)
        {
            return 0;
        }

        const int eventIndex = (day / 7) % 8;
        if (eventIndex == 2 && (type == ShopType::Material || type == ShopType::Consumable || type == ShopType::Plant))
        {
            return -3; // Foire marchande : petits achats un peu plus faciles.
        }
        if (eventIndex == 0 && (type == ShopType::Weapon || type == ShopType::Armor || type == ShopType::Blacksmith))
        {
            return 2; // Tournoi : la demande en équipement monte légèrement.
        }
        if (eventIndex == 3 && type == ShopType::Library)
        {
            return -3; // Journée du savoir : quelques copies sont moins chères.
        }
        if (eventIndex == 7 && (type == ShopType::Plant || type == ShopType::Consumable || type == ShopType::Alchemist))
        {
            return -2; // Moissons / solstice : ressources communes mieux distribuées.
        }
        return 0;
    }

    std::string scheduledCityActivityPriceLine(const Player& player, ShopType type)
    {
        const int modifier = scheduledCityActivityBuyModifierPercent(player, type);
        if (modifier < 0)
        {
            return "Animation de ville active : " + std::to_string(-modifier) + "% de tension en moins sur certains prix.";
        }
        if (modifier > 0)
        {
            return "Animation de ville active : +" + std::to_string(modifier) + "% de demande sur certains prix.";
        }
        return "";
    }

    int cityPositiveDiscountBeforeCapPercent(const Player& player, ShopType type)
    {
        const int activityModifier = scheduledCityActivityBuyModifierPercent(player, type);
        return localReputationDiscountForShop(player, type)
            + cityDefenseGratitudeDiscountPercent(player, type)
            + storyCitySupplyModifierPercent(player, type)
            + std::max(0, -activityModifier);
    }

    int cityEconomyBuyModifierPercent(const Player& player, ShopType type)
    {
        const int activityModifier = scheduledCityActivityBuyModifierPercent(player, type);
        const int cappedDiscount = std::min(kCityEconomyDiscountCapPercent, cityPositiveDiscountBeforeCapPercent(player, type));
        const int demandPressure = std::max(0, activityModifier) + localReputationSurchargeForShop(player, type);
        return demandPressure - cappedDiscount;
    }

    bool cityEconomyDiscountCapReached(const Player& player, ShopType type)
    {
        return cityPositiveDiscountBeforeCapPercent(player, type) > kCityEconomyDiscountCapPercent;
    }

    std::string cityEconomyCapLine(const Player& player, ShopType type)
    {
        if (!cityEconomyDiscountCapReached(player, type))
        {
            return "";
        }
        return "Plafond économique de ville : les remises locales cumulées sont limitées à "
            + std::to_string(kCityEconomyDiscountCapPercent) + "% pour éviter les abus.";
    }

    int cityRepairCrisisPremiumPercent(const Player& player, ShopType type)
    {
        if (cityRepairDaysRemaining(player) <= 0
            || isCityRepairEmergencyDesk(type)
            || type == ShopType::BlackMarket
            || type == ShopType::Unknown)
        {
            return 0;
        }

        int premium = 8;
        const LocalReputationSummary summary = localReputationForPlayer(player);
        if (summary.score >= 28)
        {
            premium -= 2;
        }
        else if (summary.score >= 14)
        {
            premium -= 1;
        }
        if (countMaterial(player, "city_repair_receipt") >= 2)
        {
            premium -= 1;
        }
        if (countMaterial(player, "municipal_proof_letter") >= 1)
        {
            premium -= 1;
        }
        return std::max(4, premium);
    }

    std::string cityRepairCrisisPremiumLine(const Player& player, ShopType type)
    {
        const int premium = cityRepairCrisisPremiumPercent(player, type);
        if (premium <= 0)
        {
            return "";
        }
        std::string line = "Crise de réparation : +" + std::to_string(premium) + "% sur ce comptoir ouvert exceptionnellement";
        if (premium < 8)
        {
            line += " après aide/réputation locale";
        }
        line += ".";
        return line;
    }

    int promotionDiscountPercentForItem(const ShopInventory& shop, const ShopItem& item, const Player& player)
    {
        const ShopPromotionOffer offer = promotionForShop(shop, player);
        if (!offer.discountAvailable() || offer.itemId != item.getId()) return 0;

        const int existingCityDiscount = std::max(0, -cityEconomyBuyModifierPercent(player, shop.getType()));
        return std::max(0, std::min(offer.discountPercent, 25 - existingCityDiscount));
    }

    int applyShopBuyPriceForPlayer(const ShopInventory& shop, const ShopItem& item, const Player& player)
    {
        int price = ShopPriceRules::applyBuyModifier(
            item.getBuyPrice(),
            player.getRaceText(),
            player.getType()
        );

        const int cityEconomyModifier = cityEconomyBuyModifierPercent(player, shop.getType());
        if (cityEconomyModifier != 0)
        {
            price = std::max(1, price * (100 + cityEconomyModifier) / 100);
        }

        const int crisisPremium = cityRepairCrisisPremiumPercent(player, shop.getType());
        if (crisisPremium > 0)
        {
            // La crise ne doit pas devenir un système punitif partout, mais les rares comptoirs ouverts vendent un peu plus cher.
            // Les aides municipales réduisent un peu cette tension sans l'annuler totalement.
            price = std::max(1, price * (100 + crisisPremium) / 100);
        }

        const int promotionDiscount = promotionDiscountPercentForItem(shop, item, player);
        if (promotionDiscount > 0)
        {
            price = std::max(1, price * (100 - promotionDiscount) / 100);
        }

        return price;
    }

    std::string localReputationAccessBlockReason(const Player& player, ShopType type, const ShopItem& item)
    {
        if (type == ShopType::BlackMarket || type == ShopType::Unknown)
        {
            return "";
        }

        const LocalReputationSummary summary = localReputationForPlayer(player);
        const std::string id = item.getId();

        if (summary.score <= -35 && item.getBuyPrice() >= 500)
        {
            return "vente importante refusée : la ville te considère actuellement comme indésirable";
        }

        if (!isLocalServiceShop(type))
        {
            return "";
        }

        if ((id == "municipal_proof_letter" || id == "client_recommendation") && summary.score < 5)
        {
            return "accès local requis : être au moins connue de quelques PNJ";
        }

        if (id == "guarded_transport_pass" && summary.score < 14)
        {
            return "accès local requis : réputation utile localement";
        }

        if ((id == "rental_mount_voucher" || id == "stable_box_reservation" || id == "relay_route_badge") && summary.score < 5)
        {
            return "accès local requis : être connue de quelques PNJ avant les préparatifs sérieux";
        }

        if (id == "loaded_pack_saddle" && summary.score < 14)
        {
            return "accès local requis : réputation utile localement pour confier une charge préparée";
        }

        if (id == "local_reputation_note" && summary.warningNotes > summary.successfulPersonalServices + 1)
        {
            return "accès local bloqué : trop d'incidents récents pour acheter une note de confiance";
        }

        return "";
    }

    std::string localReputationLineForPlayer(const Player& player)
    {
        const LocalReputationSummary summary = localReputationForPlayer(player);
        std::string line = "Réputation locale : " + summary.label
            + " (score " + std::to_string(summary.score)
            + ", services réussis " + std::to_string(summary.successfulPersonalServices)
            + ", incidents " + std::to_string(summary.failedPersonalServices + summary.warningNotes) + ")";

        if (summary.discountPercent > 0)
        {
            line += " | avantage services/auberge/transport : -" + std::to_string(summary.discountPercent) + "%";
        }
        else if (summary.surchargePercent > 0)
        {
            line += " | méfiance locale : +" + std::to_string(summary.surchargePercent) + "% sur les services, impact réduit ailleurs";
        }

        line += ". " + summary.reactionLine;
        return line;
    }


    std::string lowerShopContextText(std::string value)
    {
        std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });
        return value;
    }

    bool equippedShopTitleContains(const Player& player, const std::vector<std::string>& needles)
    {
        for (const std::string& title : player.getActiveTitles())
        {
            const std::string loweredTitle = lowerShopContextText(title);
            for (const std::string& needle : needles)
            {
                if (loweredTitle.find(lowerShopContextText(needle)) != std::string::npos)
                {
                    return true;
                }
            }
        }
        return false;
    }

    std::vector<std::string> equippedTitleShopLines(const Player& player, ShopType type)
    {
        std::vector<std::string> lines;
        if (player.getActiveTitles().empty())
        {
            return lines;
        }

        if (equippedShopTitleContains(player, {"prix", "marchand", "millionnaire", "banquier", "coffre"}))
        {
            lines.push_back("Titres équipés : le vendeur prend tes questions de prix un peu plus au sérieux. Effet faible : meilleure lecture du contexte, pas de grosse remise gratuite.");
        }
        if ((type == ShopType::Blacksmith || type == ShopType::Weapon || type == ShopType::Armor || type == ShopType::Material)
            && equippedShopTitleContains(player, {"matériaux", "materiaux", "automates", "reliques", "vend pas tout", "briseur"}))
        {
            lines.push_back("Titres équipés : l'atelier accepte de commenter davantage l'usure ou la qualité, sans révéler de recette rare gratuitement.");
        }
        if ((type == ShopType::Church || type == ShopType::Library || type == ShopType::Enchanter)
            && equippedShopTitleContains(player, {"anomal", "maléd", "maled", "menu", "fissuré", "fissure", "exorcisé", "exorcise"}))
        {
            lines.push_back("Titres équipés : le comptoir reconnaît une expérience étrange. Effet faible : dialogues plus méfiants ou plus précis selon le lieu.");
        }
        if (type == ShopType::Transport && equippedShopTitleContains(player, {"route", "livreur", "éclaireur", "eclaireur", "cartographe", "corniche"}))
        {
            lines.push_back("Titres équipés : les contacts de route donnent des indications plus nettes, mais les tarifs restent contrôlés par l'économie locale.");
        }
        if (type == ShopType::CityService && equippedShopTitleContains(player, {"aide", "médiateur", "mediateur", "main fiable", "négociateur", "negociateur", "crise"}))
        {
            lines.push_back("Titres équipés : le service de ville commence avec un peu plus de confiance. Bonus volontairement social et très léger.");
        }

        if (lines.empty())
        {
            lines.push_back("Titres équipés : " + player.getActiveTitleSummary() + ". Ici, ils servent surtout au style et au lore du personnage.");
        }
        return lines;
    }

    MenuScreen buildShopListScreen(const std::vector<ShopInventory>& shops, const Player* player)
    {
        MenuScreen screen("BOUTIQUES", "shop.hub");

        if (player != nullptr)
        {
            screen.addLine("Argent : " + Money::formatGoldWithRaw(player->getInventory().getGold()));
            screen.addLine("Date actuelle : " + player->formatWorldDateLine());
            screen.addLine("Moment actuel : " + player->formatWorldDayPartLine());
            screen.addLine("Les stocks changent après les combats, et certaines ventes restent rares.");
            screen.addLine("La revente protège l’équipement porté et les objets de base.");
            screen.addLine("Le marché noir vend parfois des composants interdits, expérimentaux ou instables.");
            for (const std::string& consequence : StoryCampaign::buildChapterThreeConsequenceLines(*player))
            {
                screen.addLine(consequence);
            }
            if (player->hasStoryModeStarted())
            {
                screen.addLine(storyCityDevelopmentShopLine(*player, ShopType::CityService));
                if (shops.empty())
                {
                    screen.addLine("Aucun comptoir n'est encore accessible : les premières boutiques apparaîtront avec les personnes rencontrées et les réparations de la ville.");
                }
                else
                {
                    screen.addLine("Les boutiques absentes ne sont pas encore ouvertes, reconstruites ou accessibles dans l'histoire.");
                }
            }
            if (cityRepairDaysRemaining(*player) > 0)
            {
                screen.addLine("État de ville : réparations en cours (" + std::to_string(cityRepairDaysRemaining(*player)) + " jour(s)). La plupart des boutiques restent fermées sauf services d'urgence et 1-2 comptoirs du jour.");
                screen.addLine("Pendant cette période, les demandes tournent surtout autour de réparation, garde, nettoyage et récolte de ressources.");
                screen.addLine("Les rares comptoirs non prioritaires appliquent une tension de crise modulée par ton aide locale.");
            }
            screen.addLine(localReputationLineForPlayer(*player));
            const int temporaryCount = static_cast<int>(std::count_if(shops.begin(), shops.end(), [](const ShopInventory& shop)
            {
                return isTemporaryRecommendedShop(shop);
            }));
            if (prunigilMerchantTrustScore(*player) >= 6)
            {
                screen.addLine(
                    temporaryCount > 0
                        ? "Recommandations de Prunigil : " + std::to_string(temporaryCount) + " vendeur(s) temporaire(s) présent(s) aujourd'hui."
                        : "Recommandations de Prunigil : aucun vendeur temporaire n'est présent aujourd'hui."
                );
            }
        }
        else
        {
            screen.addLine("Les boutiques seront renouvelées après chaque combat.");
        }

        screen.addOption(0, "Retour", "", true, "shop.back");

        for (std::size_t i = 0; i < shops.size(); ++i)
        {
            std::string label = shops[i].getName();
            std::string detail;
            bool open = true;
            if (player != nullptr)
            {
                open = shopIsOpenForPlayer(shops[i], *player);
                label += open ? " [ouvert]" : " [fermé]";
                detail = std::string("Horaires : ") + shopOpeningMomentsText(shops[i].getType());
            }

            screen.addOption(
                static_cast<int>(i + 1),
                label,
                detail,
                true,
                "shop.open." + std::to_string(i + 1)
            );
        }

        return screen;
    }

    MenuScreen buildShopMainScreen(const ShopInventory& shop, const Player& player)
    {
        const bool temporaryRecommended = isTemporaryRecommendedShop(shop);
        const std::string vendorName = temporaryRecommended ? temporaryMerchantDisplayName(shop) : getVendorNameForShop(shop.getType());
        MenuScreen screen(shop.getName(), "shop.single");
        screen.addLine("Argent disponible : " + Money::formatCurrencyOverviewFromCopper(player.getInventory().getTotalCopper()));
        screen.addLine("Temps actuel : " + player.formatWorldDateTimeLine());
        const bool shopOpen = shopIsOpenForPlayer(shop, player);
        screen.addLine("Interlocuteur : " + vendorName);
        if (temporaryRecommended)
        {
            screen.addLine("Comptoir temporaire recommandé par Prunigil.");
            screen.addLine("Style de vente : " + temporaryMerchantSellingStyle(shop) + ".");
        }
        screen.addLine(shopOpenStatusLine(shop, player));
        const std::string storyLine = storyCityDevelopmentShopLine(player, shop.getType());
        if (!storyLine.empty())
        {
            screen.addLine(storyLine);
        }
        screen.addLine("Accueil : " + (shopOpen ? chooseShopIntroLine(shop.getType()) : "Le comptoir est rangé. Les achats, services et discussions attendront l'ouverture."));

        if (shopOpen)
        {
            for (const std::string& titleLine : equippedTitleShopLines(player, shop.getType()))
            {
                screen.addLine(titleLine);
            }
        }

        if (shopOpen && player.getActiveCurseCount() > 0)
        {
            const int socialPressure = player.getCursePressureForCategory("social");
            const int knownSocialPressure = player.getKnownCursePressureForCategory("social");
            const int equipmentPressure = player.getCursePressureForCategory("equipment");
            const int knownEquipmentPressure = player.getKnownCursePressureForCategory("equipment");
            const int corruptionPressure = player.getCursePressureForCategory("corruption");

            if (socialPressure > 0)
            {
                screen.addLine(knownSocialPressure > 0
                    ? "Réaction PNJ : le marchand remarque ton aura sociale diagnostiquée et garde une distance polie."
                    : "Réaction PNJ : l'accueil reste correct, mais un léger malaise traverse le comptoir sans raison claire.");
            }

            if (equipmentPressure > 0 && (shop.getType() == ShopType::Blacksmith || shop.getType() == ShopType::Enchanter || shop.getType() == ShopType::Armor || shop.getType() == ShopType::Weapon))
            {
                screen.addLine(knownEquipmentPressure > 0
                    ? "Réaction d'atelier : l'équipement semble répondre avec retard, piste équipement déjà crédible."
                    : "Réaction d'atelier : un outil tinte tout seul quand ton équipement approche.");
            }

            if (corruptionPressure > 0 && shop.getType() == ShopType::Church)
            {
                screen.addLine("Réaction d'église : la cire froide posée près des cierges se plie légèrement vers ton nom.");
            }
        }

        const int buybackCount = ShopTransactionSystem::getBuybackEntryCount(shop.getType());
        if (buybackCount > 0)
        {
            screen.addLine("Rachat disponible : " + std::to_string(buybackCount) + " vente(s) récupérable(s) avant le prochain combat.");
        }
        else
        {
            screen.addLine("Rachat disponible : aucune vente récente dans cette boutique.");
        }

        screen.addOption(0, "Retour", "", true, "shop.single.back");
        screen.addOption(1, "Acheter", shopOpen ? "Voir le stock et les prix de cette boutique." : "Boutique fermée à ce moment de la journée.", shopOpen, "shop.single.buy");
        screen.addOption(2, "Vendre", shopOpen ? "Proposer des objets compatibles avec ce marchand." : "Boutique fermée à ce moment de la journée.", shopOpen, "shop.single.sell");
        screen.addOption(3, "Discuter avec " + vendorName, shopOpen ? "" : "Interlocuteur indisponible pour l'instant.", shopOpen, "shop.single.talk");
        const bool specialLegendaryMerchant = isSpecialLegendaryMerchantShop(shop);
        screen.addOption(
            4,
            specialLegendaryMerchant ? "Défis et demandes de " + vendorName
                : (temporaryRecommended ? "Aucune quête permanente" : "Quêtes de " + vendorName),
            specialLegendaryMerchant
                ? "Ce vendeur temporaire peut confier une demande tant qu'il est présent."
                : (temporaryRecommended
                    ? "Ce vendeur ne reste pas assez longtemps pour tenir un tableau de quêtes permanent."
                    : (shopOpen ? "" : "Le contact n'est pas disponible maintenant.")),
            shopOpen && (!temporaryRecommended || specialLegendaryMerchant),
            "shop.single.quest"
        );
        screen.addOption(
            5,
            "Racheter une vente récente",
            buybackCount > 0
                ? "Seulement avant le prochain combat, avec un surcoût de récupération."
                : "Aucune vente récente n'est récupérable ici pour le moment.",
            shopOpen && buybackCount > 0,
            "shop.single.buyback"
        );

        if (!temporaryRecommended && shop.getType() == ShopType::Lodging)
        {
            screen.addOption(
                6,
                "Utiliser l'auberge",
                "Manger, dormir ou préparer une écurie. Ces actions font avancer la journée.",
                shopOpen,
                "shop.single.lodging_services"
            );
        }
        else if (!temporaryRecommended && shop.getType() == ShopType::Transport)
        {
            screen.addOption(
                6,
                "Organiser le relais",
                "Préparer écurie, route ou caravane. Ces actions font avancer la journée.",
                shopOpen,
                "shop.single.transport_services"
            );
        }
        else if (!temporaryRecommended && shop.getType() == ShopType::CityService)
        {
            screen.addOption(
                6,
                "Cotisations et abonnements",
                "Gérer les cotisations de 7 jours liées aux services de ville et de guilde.",
                shopOpen,
                "shop.single.city_subscriptions"
            );
        }
        else if (!temporaryRecommended && shop.getType() == ShopType::Church)
        {
            screen.addOption(
                6,
                "Église et exorcisme",
                "Diagnostiquer les malédictions, lancer un rite court, ou comprendre une condition spéciale.",
                shopOpen,
                "shop.single.church_services"
            );
        }
        else if (!temporaryRecommended && shop.getType() == ShopType::Enchanter)
        {
            screen.addOption(
                6,
                "Atelier d'enchantement",
                "Graver plusieurs runes sur armes/armures avec risque progressif de casse définitive.",
                shopOpen,
                "shop.single.enchanter_services"
            );
        }
        else if (!temporaryRecommended && shop.getType() == ShopType::Library)
        {
            screen.addOption(
                6,
                "Pratiquer une langue",
                "Après le niveau conversation, travailler la fluidité avec un bibliothécaire plutôt que l'acheter instantanément.",
                shopOpen,
                "shop.single.language_practice"
            );
        }

        return screen;
    }

    int getMaxBuyQuantity(const ShopItem& item, const Player& player, int finalPrice);

    MenuScreen buildVendorTalkScreen(const ShopInventory& shop, Player& player)
    {
        const bool temporaryRecommended = isTemporaryRecommendedShop(shop);
        const std::string vendorName = temporaryRecommended ? temporaryMerchantDisplayName(shop) : getVendorNameForShop(shop.getType());
        const NpcPropagationResult localPropagation = NpcInformationPropagationSystem::propagateOneLocalFact(player, vendorName);
        MenuScreen screen("DISCUSSION", "shop.vendor_talk");
        screen.addLine(vendorName + " prend quelques secondes pour parler boutique, rumeurs et besoins du moment.");
        for (const std::string& introLine : NpcKnowledgeSystem::spontaneousIntroLines(player, vendorName))
        {
            screen.addLine(introLine);
        }
        if (localPropagation.transferred)
        {
            screen.addLine("Rumeur locale : " + localPropagation.sourceNpc + " a transmis une information qui a réellement atteint ce comptoir.");
        }
        for (const std::string& memoryLine : NpcKnowledgeSystem::conversationMemoryLines(player, vendorName, 2))
        {
            screen.addLine(memoryLine);
        }
        if (player.hasTitle("Porte-marque de la guilde"))
        {
            screen.addLine("Le vendeur reconnaît la marque de la guilde, mais précise qu'elle donne du crédit à ta parole, pas une remise automatique.");
        }
        if (player.hasTitle("Survivant des caisses"))
        {
            screen.addLine("Une caisse est discrètement éloignée de toi : ta précédente expérience avec le stock de Bob et Maurice a circulé.");
        }
        if (player.hasTitle("Triplement maudit"))
        {
            screen.addLine("Le vendeur garde une distance professionnelle et vérifie deux fois que rien dans son stock ne réagit à tes malédictions.");
        }

        if (temporaryRecommended)
        {
            if (vendorName == "Hero Villager")
            {
                screen.addLine("Hmmm... Les objets sont à vendre. Les défis, eux, doivent être mérités... Huuuh.");
                screen.addLine("Sa voix commence et se termine par un grognement bref, comme si chaque phrase devait être validée deux fois.");
            }
            else if (vendorName == "Bob et Maurice")
            {
                screen.addLine("Bob : Hannnn... hummm... hammmm.");
                screen.addLine("Maurice : « Mon collègue Bob a dit que ton sac manque clairement de caisses dangereuses. »");
                screen.addLine("Maurice : Huuuhhhhh... hannn.");
                screen.addLine("Bob : « Maurice demande si tu comptes lire les avertissements avant ou après l'explosion. »");
            }
            else
            {
                screen.addLine("« Prunigil m'a parlé de toi. Je reste peu de temps, alors regarde bien le stock avant mon départ. »");
                screen.addLine("Ce comptoir temporaire n'a pas de tableau de quêtes permanent, mais sa présence dépend de tes recommandations.");
            }
            screen.addLine("Style annoncé : " + temporaryMerchantSellingStyle(shop) + ".");
        }
        else
        {
            const std::vector<std::string> talkLines = chooseVendorTalkLines(shop.getType());
            for (const std::string& line : talkLines)
            {
                screen.addLine(line);
            }
            screen.addLine("S'il a une vraie demande, utilise l'option de quêtes juste en dessous.");
        }

        screen.addOption(0, "Continuer", "", true, "shop.vendor_talk.continue");
        return screen;
    }


    MenuScreen buildShopStockScreen(const ShopInventory& shop, const Player& player, std::size_t pageIndex, std::size_t itemsPerPage)
    {
        const std::vector<ShopItem>& items = shop.getItems();
        const std::size_t totalPages = PagedMenu::pageCount(items.size(), itemsPerPage);
        const std::size_t first = PagedMenu::firstIndex(pageIndex, itemsPerPage);
        const std::size_t last = PagedMenu::lastIndexExclusive(items.size(), pageIndex, itemsPerPage);

        MenuScreen screen(shop.getName(), "shop.stock");
        screen.addLine("Argent disponible : " + Money::formatCurrencyOverviewFromCopper(player.getInventory().getTotalCopper()));
        screen.addLine("Race : " + player.getRaceText());
        const int localDiscount = localReputationDiscountForShop(player, shop.getType());
        if (localDiscount > 0)
        {
            screen.addLine("Avantage de réputation locale : -" + std::to_string(localDiscount) + "% sur cette boutique.");
        }
        const int defenseDiscount = cityDefenseGratitudeDiscountPercent(player, shop.getType());
        if (defenseDiscount > 0)
        {
            screen.addLine("Reconnaissance de défense : -" + std::to_string(defenseDiscount) + "% sur les achats pendant encore " + std::to_string(player.getInventory().countMaterialById("city_defense_gratitude_days_marker")) + " jour(s).");
        }
        const std::string activityPriceLine = scheduledCityActivityPriceLine(player, shop.getType());
        if (!activityPriceLine.empty())
        {
            screen.addLine(activityPriceLine);
        }
        const std::string capLine = cityEconomyCapLine(player, shop.getType());
        if (!capLine.empty())
        {
            screen.addLine(capLine);
        }

        const ShopPromotionOffer promotion = promotionForShop(shop, player);
        if (promotion.active)
        {
            std::string promotionLine = promotion.clearance ? "Déstockage du comptoir" : "Article en réduction";
            promotionLine += " — jour " + std::to_string(promotion.dayInOffer) + "/3 : " + promotion.itemName;
            promotionLine += " (-" + std::to_string(promotion.discountPercent) + "%).";
            if (promotion.clearance)
            {
                promotionLine += " Lot d'invendus : " + std::to_string(promotion.remainingDiscountedQuantity())
                    + "/" + std::to_string(promotion.quantityLimit) + " unité(s) encore remisées.";
            }
            promotionLine += " Fin dans " + std::to_string(promotion.daysRemaining) + " jour(s).";
            screen.addLine(promotionLine);
        }

        if (items.empty())
        {
            screen.addLine("Aucun article disponible pour le moment.");
            screen.addOption(0, "Retour", "", true, "shop.stock.back");
            return screen;
        }

        screen.setPagination(pageIndex, totalPages);
        screen.addLine(PagedMenu::pageInfoText(pageIndex, totalPages, items.size()));
        screen.addLine("Affichage : " + PagedMenu::rangeText(first, last, items.size()));

        for (std::size_t i = first; i < last; ++i)
        {
            const int localIndex = static_cast<int>(i - first + 1);
            int finalPrice = applyShopBuyPriceForPlayer(shop, items[i], player);

            const bool canBuyNow = ShopTransactionSystem::canBeBoughtNow(items[i]);
            const bool soldOut = items[i].isSoldOut();
            const std::string accessBlockReason = localReputationAccessBlockReason(player, shop.getType(), items[i]);
            const bool locallyBlocked = !accessBlockReason.empty();
            const bool barterOffer = hasBlackMarketBarterOffer(shop, items[i]);
            const int barterMax = barterOffer ? getMaxBarterQuantity(items[i], player) : 0;
            const std::string categoryLabel = shopItemCategoryToText(items[i].getCategory());

            std::string label = items[i].getName() + shopWeaponClassCompatibilityTag(player, items[i])
                + " | Catégorie : " + categoryLabel
                + " | Prix : " + Money::formatGoldWithRaw(finalPrice);

            const int itemPromotionDiscount = promotionDiscountPercentForItem(shop, items[i], player);
            if (promotion.active && promotion.itemId == items[i].getId())
            {
                if (itemPromotionDiscount > 0)
                {
                    label += promotion.clearance
                        ? " | Déstockage -" + std::to_string(itemPromotionDiscount) + "%"
                        : " | Offre 3 jours -" + std::to_string(itemPromotionDiscount) + "%";
                }
                else if (promotion.clearance)
                {
                    label += " | Lot remisé épuisé";
                }
            }

            if (items[i].getStock() >= 0)
            {
                label += " | Stock : " + std::to_string(items[i].getStock());
            }
            else
            {
                label += " | Stock : non limité";
            }

            if (soldOut)
            {
                label += " | Épuisé";
            }
            else if (!canBuyNow)
            {
                label += " | Indisponible";
            }
            else if (locallyBlocked)
            {
                label += " | Accès réputation requis";
            }

            if (barterOffer)
            {
                label += " | Troc/unité : " + formatBarterRequirements(items[i]);
                label += barterMax > 0
                    ? " | Troc possible x" + std::to_string(barterMax)
                    : " | Troc impossible maintenant";
            }

            MenuOptionItemData itemData;
            itemData.structured = true;
            itemData.kind = "shop";
            itemData.section = categoryLabel;
            itemData.actionType = barterOffer ? "buy" : "buy";
            itemData.name = items[i].getName() + shopWeaponClassCompatibilityTag(player, items[i]);
            itemData.detail = items[i].getDescription();
            const std::string classCompatibilityLine = shopWeaponClassCompatibilityLine(player, items[i]);
            if (!classCompatibilityLine.empty())
            {
                itemData.reward = classCompatibilityLine;
            }
            itemData.price = Money::formatGoldWithRaw(finalPrice);
            itemData.stock = items[i].getStock() >= 0 ? std::to_string(items[i].getStock()) : "non limité";
            if (soldOut)
            {
                itemData.status = "Épuisé";
            }
            else if (!canBuyNow)
            {
                itemData.status = "Indisponible";
            }
            else if (locallyBlocked)
            {
                itemData.status = "Bloqué : " + accessBlockReason;
            }
            else if (getMaxBuyQuantity(items[i], player, finalPrice) <= 0)
            {
                itemData.status = "Argent insuffisant";
            }
            if (barterOffer)
            {
                itemData.reward = "Demande/unité : " + formatBarterRequirements(items[i]);
                itemData.maxQuantity = barterMax > 0 ? std::to_string(barterMax) : "0";
            }
            itemData.important = soldOut || !canBuyNow || locallyBlocked || barterOffer;

            screen.addOption(localIndex, label, "", true, "shop.stock.select." + std::to_string(i), itemData);
        }

        PagedMenu::addNavigationOptions(
            screen,
            pageIndex,
            totalPages,
            "shop.stock.back",
            "shop.stock.previous",
            "shop.stock.next",
            "Revenir au menu de la boutique.",
            "Voir les articles précédents.",
            "Voir les articles suivants."
        );

        return screen;
    }

    // EN: inspectShopItem declares or implements a focused behavior used by this module.
    // FR: inspectShopItem déclare ou implémente un comportement précis utilisé par ce module.

    int getMaxBuyQuantity(const ShopItem& item, const Player& player, int finalPrice)
    {
        if (!ShopTransactionSystem::canBeBoughtNow(item) || item.isSoldOut())
        {
            return 0;
        }

        int affordable = finalPrice <= 0 ? 99 : static_cast<int>(player.getInventory().getTotalCopper() / Money::copperFromGold(finalPrice));
        if (affordable <= 0)
        {
            return 0;
        }

        int stockLimit = item.getStock() > 0 ? item.getStock() : 99;
        int maxQuantity = std::min(stockLimit, affordable);

        if (item.isCommonInformation())
        {
            maxQuantity = std::min(maxQuantity, 1);
        }

        return std::max(0, maxQuantity);
    }


    SellableEntryUiInfo getSellableEntryUiInfo(const Player& player, ShopType shopType, int index)
    {
        SellableEntryUiInfo info;

        if (shopType == ShopType::Weapon && player.getInventory().hasWeapon(index))
        {
            const Weapon weapon = player.getInventory().getWeapon(index);
            info.name = weapon.getName() + ownedWeaponClassCompatibilityTag(player, weapon);
            info.durability = formatEquipmentDurabilityText(weapon.getDurability(), weapon.getMaxDurability());
            info.enchantmentSummary = "Enchantements : " + weapon.getEnchantmentSummaryText();
            info.detail = "Arme possédée par le personnage. " + info.durability + ". " + info.enchantmentSummary + ". Prix ajusté par état, enchantements et acheteur spécialisé.";
            if (CombatClassSystem::hasWeaponAffinity(player, weapon.getType(), weapon.getName()))
            {
                info.status = "Bonus de classe";
                info.detail += " Synergie : " + CombatClassSystem::getWeaponAffinityLabel(player, weapon.getType(), weapon.getName()) + ".";
            }
            else if (CombatClassSystem::getWeaponHandlingAccuracyAdjustment(player, weapon.getType(), weapon.getName()) < 0
                || CombatClassSystem::getWeaponHandlingDamagePercent(player, weapon.getType(), weapon.getName()) < 100)
            {
                info.status = "Malus de classe";
                info.detail += " Avertissement : " + CombatClassSystem::getWeaponHandlingLabel(player, weapon.getType(), weapon.getName()) + ".";
            }
        }
        else if (shopType == ShopType::Armor && player.getInventory().hasArmor(index))
        {
            const Armor armor = player.getInventory().getArmor(index);
            info.name = armor.getName();
            info.durability = formatEquipmentDurabilityText(armor.getDurability(), armor.getMaxDurability());
            info.enchantmentSummary = "Enchantements : " + armor.getEnchantmentSummaryText();
            info.detail = "Armure ou tenue possédée par le personnage. " + info.durability + ". " + info.enchantmentSummary + ". Prix ajusté par état, enchantements et acheteur spécialisé.";
        }
        else if (shopType == ShopType::Consumable && player.getInventory().hasConsumable(index))
        {
            info.name = player.getInventory().getConsumable(index).getName();
            info.detail = "Consommable présent dans l'inventaire.";
        }
        else if (player.getInventory().hasMaterial(index))
        {
            Material material = player.getInventory().getMaterial(index);
            info.name = material.getName();
            info.quantity = "x" + std::to_string(material.getQuantity());
            info.detail = "Matériau présent dans l'inventaire.";

            if (material.hasSpecialQuality())
            {
                info.status = "Qualité : " + material.getQualityLabel();
            }
        }
        else
        {
            info.name = "Entrée inconnue";
            info.status = "Invalide";
            info.detail = "Cette entrée ne peut pas être résolue dans l'inventaire.";
        }

        info.sellable = ShopTransactionSystem::canShopBuyInventoryEntry(player, shopType, index);
        if (!info.sellable)
        {
            if (info.status.empty())
            {
                info.status = "Protégé";
            }
            info.label = info.name;
            if (!info.quantity.empty())
            {
                info.label += " " + info.quantity;
            }
            if (!info.durability.empty())
            {
                info.label += " | " + info.durability;
            }
            if (!info.enchantmentSummary.empty())
            {
                info.label += " | " + info.enchantmentSummary;
            }
            info.label += " | Statut : " + info.status;
            return info;
        }

        const int sellPrice = ShopTransactionSystem::getSellPriceForEntry(player, shopType, index);
        const int maxQuantity = ShopTransactionSystem::getMaxSellQuantityForEntry(player, shopType, index);
        info.price = Money::formatGoldWithRaw(sellPrice);
        info.maxQuantity = "x" + std::to_string(maxQuantity);

        info.label = info.name;
        if (!info.quantity.empty())
        {
            info.label += " " + info.quantity;
        }
        if (!info.durability.empty())
        {
            info.label += " | " + info.durability;
        }
        if (!info.enchantmentSummary.empty())
        {
            info.label += " | " + info.enchantmentSummary;
        }
        if (!info.status.empty())
        {
            info.label += " | " + info.status;
        }
        info.label += " | Revente : " + info.price;
        info.label += " | Max : " + info.maxQuantity;
        return info;
    }

    std::string sellableEntryLabel(const Player& player, ShopType shopType, int index)
    {
        return getSellableEntryUiInfo(player, shopType, index).label;
    }

    MenuScreen buildShopItemScreen(const ShopInventory& shop, const ShopItem& item, const Player& player, bool withActions)
    {
        int finalBuyPrice = applyShopBuyPriceForPlayer(shop, item, player);

        int finalSellPrice = ShopPriceRules::applySellModifier(
            item.getSellPrice(),
            player.getRaceText(),
            player.getType()
        );

        MenuScreen screen("ARTICLE", "shop.item");
        screen.addLine("Nom : " + item.getName());
        screen.addLine("Catégorie : " + std::string(shopItemCategoryToText(item.getCategory())));
        screen.addLine("Description : " + item.getDescription());
        const std::string classCompatibilityLine = shopWeaponClassCompatibilityLine(player, item);
        if (!classCompatibilityLine.empty())
        {
            screen.addLine(classCompatibilityLine);
        }
        screen.addLine("Prix d'achat : " + Money::formatGoldWithRaw(finalBuyPrice));
        const ShopPromotionOffer promotion = promotionForShop(shop, player);
        if (promotion.active && promotion.itemId == item.getId())
        {
            const int itemPromotionDiscount = promotionDiscountPercentForItem(shop, item, player);
            if (itemPromotionDiscount > 0)
            {
                screen.addLine((promotion.clearance ? "Déstockage" : "Offre du comptoir")
                    + std::string(" : -") + std::to_string(itemPromotionDiscount)
                    + "% | jour " + std::to_string(promotion.dayInOffer) + "/3.");
                if (promotion.clearance)
                {
                    screen.addLine("Invendus encore remisés : " + std::to_string(promotion.remainingDiscountedQuantity())
                        + "/" + std::to_string(promotion.quantityLimit) + ".");
                }
            }
            else if (promotion.clearance)
            {
                screen.addLine("Le lot d'invendus remisé est épuisé : l'article reste disponible au tarif normal si le stock le permet.");
            }
        }
        const int localDiscount = localReputationDiscountForShop(player, shop.getType());
        if (localDiscount > 0)
        {
            screen.addLine("Avantage de réputation locale appliqué : -" + std::to_string(localDiscount) + "%.");
        }
        const int defenseDiscount = cityDefenseGratitudeDiscountPercent(player, shop.getType());
        if (defenseDiscount > 0)
        {
            screen.addLine("Reconnaissance de défense appliquée : -" + std::to_string(defenseDiscount) + "%.");
        }
        const int activityModifier = scheduledCityActivityBuyModifierPercent(player, shop.getType());
        if (activityModifier < 0)
        {
            screen.addLine("Animation locale : -" + std::to_string(-activityModifier) + "% sur cette famille de prix.");
        }
        else if (activityModifier > 0)
        {
            screen.addLine("Demande locale : +" + std::to_string(activityModifier) + "% sur cette famille de prix.");
        }
        const std::string capLine = cityEconomyCapLine(player, shop.getType());
        if (!capLine.empty())
        {
            screen.addLine(capLine);
        }
        const std::string crisisPremiumLine = cityRepairCrisisPremiumLine(player, shop.getType());
        if (!crisisPremiumLine.empty())
        {
            screen.addLine(crisisPremiumLine);
        }
        const int netCityModifier = cityEconomyBuyModifierPercent(player, shop.getType());
        if (netCityModifier < 0)
        {
            screen.addLine("Effet économique local final : -" + std::to_string(-netCityModifier) + "% après plafond éventuel.");
        }
        else if (netCityModifier > 0)
        {
            screen.addLine("Effet économique local final : +" + std::to_string(netCityModifier) + "% après demandes locales.");
        }
        screen.addLine(item.getStock() >= 0
            ? "Stock : " + std::to_string(item.getStock())
            : "Stock : non limité");

        int maxBuyQuantity = getMaxBuyQuantity(item, player, finalBuyPrice);
        const std::string accessBlockReason = localReputationAccessBlockReason(player, shop.getType(), item);
        if (!accessBlockReason.empty())
        {
            screen.addLine("Accès local : " + accessBlockReason + ".");
        }
        if (maxBuyQuantity > 0 && accessBlockReason.empty())
        {
            screen.addLine("Quantité achetable maintenant : max x" + std::to_string(maxBuyQuantity));
            screen.addLine("Achat maximum estimé : " + Money::formatGoldWithRaw(finalBuyPrice * maxBuyQuantity));
        }

        if (hasBlackMarketBarterOffer(shop, item))
        {
            int maxBarterQuantity = getMaxBarterQuantity(item, player);
            screen.addLine("Troc du marché noir par unité : " + formatBarterRequirements(item));

            if (maxBarterQuantity > 0)
            {
                screen.addLine("Quantité échangeable maintenant : max x" + std::to_string(maxBarterQuantity));
                screen.addLine("Demande totale au maximum : " + formatBarterRequirements(item, maxBarterQuantity));
            }
            else
            {
                screen.addLine("Troc impossible maintenant : composants insuffisants.");
            }
        }

        screen.addLine("Prix de revente estimé : " + Money::formatGoldWithRaw(finalSellPrice));

        if (player.getRaceText().find("Démon") != std::string::npos
            || player.getRaceText().find("démon") != std::string::npos)
        {
            screen.addLine("Note : ton apparence démoniaque influence déjà certains prix.");
        }

        if (ShopPriceRules::hasCraftClassTradeBonus(player.getType()))
        {
            screen.addLine("Note : ta classe d'artisanat négocie légèrement mieux les prix.");
        }

        if (!ShopTransactionSystem::canBeBoughtNow(item))
        {
            screen.addLine("Statut : le marchand refuse de sortir cet article pour le moment.");
        }

        if (withActions)
        {
            const bool barterAvailable = hasBlackMarketBarterOffer(shop, item);
            const bool canBuyWithGold = maxBuyQuantity > 0 && accessBlockReason.empty();
            const int maxBarterQuantity = barterAvailable ? getMaxBarterQuantity(item, player) : 0;

            MenuOptionItemData buyData;
            buyData.structured = true;
            buyData.kind = "shop";
            buyData.section = shopItemCategoryToText(item.getCategory());
            buyData.actionType = "buy";
            buyData.name = item.getName() + shopWeaponClassCompatibilityTag(player, item);
            buyData.detail = item.getDescription();
            buyData.price = Money::formatGoldWithRaw(finalBuyPrice);
            buyData.stock = item.getStock() >= 0 ? std::to_string(item.getStock()) : "non limité";
            buyData.maxQuantity = std::to_string(maxBuyQuantity);
            buyData.status = canBuyWithGold ? "Disponible" : (accessBlockReason.empty() ? std::string("Bloqué") : "Bloqué : " + accessBlockReason);
            buyData.important = !canBuyWithGold;

            screen.addOption(0, "Retour", "", true, "shop.item.back");
            screen.addOption(
                1,
                "Acheter avec le portefeuille",
                canBuyWithGold
                    ? "Acheter cet article avec l'ensemble des pièces disponibles, converties automatiquement."
                    : "Achat impossible maintenant : argent, stock ou disponibilité insuffisante.",
                canBuyWithGold,
                "shop.item.buy",
                buyData
            );
            screen.addOption(2, "Inspecter encore", "Relire les détails sans transaction.", true, "shop.item.inspect");

            if (barterAvailable)
            {
                MenuOptionItemData barterData;
                barterData.structured = true;
                barterData.kind = "shop";
                barterData.section = "Marché noir";
                barterData.actionType = "barter";
                barterData.name = item.getName() + shopWeaponClassCompatibilityTag(player, item);
                barterData.detail = item.getDescription();
                barterData.reward = "Demande/unité : " + formatBarterRequirements(item);
                barterData.stock = item.getStock() >= 0 ? std::to_string(item.getStock()) : "non limité";
                barterData.maxQuantity = std::to_string(maxBarterQuantity);
                barterData.status = maxBarterQuantity > 0 ? "Troc possible" : "Composants insuffisants";
                barterData.important = maxBarterQuantity <= 0;

                screen.addOption(
                    3,
                    "Troquer des objets",
                    maxBarterQuantity > 0
                        ? "Échanger les composants demandés contre cet article."
                        : "Troc impossible maintenant : composants ou stock insuffisants.",
                    maxBarterQuantity > 0,
                    "shop.item.barter",
                    barterData
                );
            }
        }

        return screen;
    }

    // EN: openSellMenu declares or implements a focused behavior used by this module.
    // FR: openSellMenu déclare ou implémente un comportement précis utilisé par ce module.
    void openSellMenu(Player& player, const ShopInventory& shop)
    {
        bool selling = true;
        std::size_t pageIndex = 0;
        const std::size_t itemsPerPage = 10;

        while (selling)
        {
            Console::clear();
            int maxChoice = ShopTransactionSystem::getSellableEntryCount(player, shop.getType());
            MenuScreen sellScreen("REVENTE", "shop.sell");
            sellScreen.addLine("Boutique : " + shop.getName());
            sellScreen.addLine("Argent actuel : " + Money::formatCurrencyOverviewFromCopper(player.getInventory().getTotalCopper()));
            sellScreen.addLine("Les entrées protégées ou incompatibles restent visibles, mais ne peuvent pas être vendues.");
            sellScreen.addLine("Prix : la durabilité baisse la valeur, les enchantements l'augmentent, et un bon acheteur paie mieux.");

            if (maxChoice <= 0)
            {
                sellScreen.addLine("Rien à vendre ici pour le moment.");
                sellScreen.addBackOption("Retour", "shop.sell.back");
                TerminalInterface::askMenuChoiceFromOptions(
                    sellScreen,
                    "Entre 0 pour revenir."
                );
                Console::clear();
                return;
            }

            const std::size_t totalEntries = static_cast<std::size_t>(maxChoice);
            const std::size_t totalPages = PagedMenu::pageCount(totalEntries, itemsPerPage);
            if (pageIndex >= totalPages)
            {
                pageIndex = totalPages > 0 ? totalPages - 1 : 0;
            }

            const std::size_t first = PagedMenu::firstIndex(pageIndex, itemsPerPage);
            const std::size_t last = PagedMenu::lastIndexExclusive(totalEntries, pageIndex, itemsPerPage);
            const int localCount = static_cast<int>(last - first);

            sellScreen.setPagination(pageIndex, totalPages);
            sellScreen.addLine(PagedMenu::pageInfoText(pageIndex, totalPages, totalEntries));
            sellScreen.addLine("Affichage : " + PagedMenu::rangeText(first, last, totalEntries));

            for (std::size_t i = first; i < last; ++i)
            {
                const int localIndex = static_cast<int>(i - first + 1);
                const int inventoryIndex = static_cast<int>(i);
                const SellableEntryUiInfo entryInfo = getSellableEntryUiInfo(player, shop.getType(), inventoryIndex);
                MenuOptionItemData itemData;
                itemData.structured = true;
                itemData.kind = "shop";
                itemData.section = "Revente";
                itemData.actionType = "sell";
                itemData.name = entryInfo.name;
                itemData.quantity = entryInfo.quantity;
                itemData.detail = entryInfo.detail;
                itemData.status = entryInfo.status;
                itemData.price = entryInfo.price;
                itemData.maxQuantity = entryInfo.maxQuantity;
                itemData.important = !entryInfo.sellable || !entryInfo.status.empty();

                sellScreen.addOption(
                    localIndex,
                    entryInfo.label,
                    entryInfo.sellable
                        ? "Vendre cet objet ou une quantité si plusieurs exemplaires sont disponibles."
                        : "Cette entrée reste visible, mais le marchand ne peut pas l'acheter.",
                    entryInfo.sellable,
                    "shop.sell.select." + std::to_string(inventoryIndex),
                    itemData
                );
            }

            PagedMenu::addNavigationOptions(
                sellScreen,
                pageIndex,
                totalPages,
                "shop.sell.back",
                "shop.sell.previous",
                "shop.sell.next",
                "Revenir au menu de la boutique.",
                "Voir les objets précédents.",
                "Voir les objets suivants."
            );

            int choice = TerminalInterface::askMenuChoiceFromOptions(
                sellScreen,
                "Choix refusé : sélectionne une entrée affichée, 98/99 pour tourner les pages, ou 0 pour revenir."
            );

            if (choice == 0)
            {
                selling = false;
                continue;
            }

            if (choice == 98 && pageIndex > 0)
            {
                pageIndex--;
                continue;
            }

            if (choice == 99 && pageIndex + 1 < totalPages)
            {
                pageIndex++;
                continue;
            }

            if (choice < 1 || choice > localCount)
            {
                showShopResult(
                    "CHOIX INDISPONIBLE",
                    "shop.sell.invalid_choice",
                    {
                        "Cette entrée n'existe pas sur la page de revente actuelle.",
                        "Utilise uniquement les choix affichés par la boutique."
                    }
                );
                continue;
            }

            int index = static_cast<int>(first) + choice - 1;

            if (!ShopTransactionSystem::canShopBuyInventoryEntry(player, shop.getType(), index))
            {
                showShopResult(
                    "VENTE IMPOSSIBLE",
                    "shop.sell.blocked",
                    {
                        "Entrée : " + sellableEntryLabel(player, shop.getType(), index),
                        "Statut : protégée ou refusée par cette boutique.",
                        "Raison possible : équipement porté, objet de base, entrée invalide ou mauvais type de marchand.",
                        "Aucun objet n'a été retiré de ton inventaire."
                    }
                );
                continue;
            }

            int sellPrice = ShopTransactionSystem::getSellPriceForEntry(
                player,
                shop.getType(),
                index
            );

            int maxQuantity = ShopTransactionSystem::getMaxSellQuantityForEntry(player, shop.getType(), index);
            int quantity = 1;

            if (maxQuantity > 1)
            {
                quantity = MessageScreen::askQuantity(
                    "QUANTITÉ À VENDRE",
                    "shop.sell.quantity",
                    {
                        "Boutique : " + shop.getName(),
                        "Maximum vendable : x" + std::to_string(maxQuantity),
                        "Prix unitaire estimé : " + Money::formatGoldWithRaw(sellPrice)
                    },
                    1,
                    maxQuantity,
                    "Veuillez choisir une quantité valide."
                );
            }

            const long long copperBeforeSale = player.getInventory().getTotalCopper();
            const std::string selectedEntryLabel = sellableEntryLabel(player, shop.getType(), index);
            const int totalSellPrice = sellPrice * quantity;

            const bool confirmSale = askShopConfirmation(
                "CONFIRMER LA VENTE",
                "shop.sell.confirm",
                {
                    "Boutique : " + shop.getName(),
                    "Objet : " + selectedEntryLabel,
                    "Quantité : x" + std::to_string(quantity),
                    "Prix unitaire : " + Money::formatGoldWithRaw(sellPrice),
                    "Total reçu : " + Money::formatGoldWithRaw(totalSellPrice),
                    "Rachat : l'objet restera récupérable ici jusqu'au prochain combat avec un surcoût."
                },
                "Confirmer la vente",
                "Annuler la vente",
                "shop.sell"
            );

            if (!confirmSale)
            {
                showShopResult(
                    "VENTE ANNULÉE",
                    "shop.sell.cancelled",
                    {
                        "Objet : " + selectedEntryLabel,
                        "Aucun objet n'a quitté ton inventaire.",
                        "Le marchand range déjà sa bourse, légèrement déçu."
                    }
                );
                continue;
            }

            ShopTransactionSystem::clearLastTransactionNotes();
            const bool saleSucceeded = ShopTransactionSystem::sellInventoryEntryQuantity(
                player,
                shop.getType(),
                index,
                sellPrice,
                quantity
            );

            showShopTransactionResult(
                saleSucceeded ? "VENTE TERMINÉE" : "VENTE REFUSÉE",
                saleSucceeded ? "shop.sell.result.success" : "shop.sell.result.failed",
                saleSucceeded
                    ? std::vector<std::string>{
                        "Objet vendu : " + selectedEntryLabel,
                        "Quantité : x" + std::to_string(quantity),
                        "Argent reçu : " + Money::formatGoldWithRaw(totalSellPrice),
                        "Argent avant : " + Money::formatCurrencyOverviewFromCopper(copperBeforeSale),
                        "Argent actuel : " + Money::formatCurrencyOverviewFromCopper(player.getInventory().getTotalCopper()),
                        "Rachat : disponible dans cette boutique jusqu'au prochain combat."
                    }
                    : std::vector<std::string>{
                        "Objet demandé : " + selectedEntryLabel,
                        "Quantité demandée : x" + std::to_string(quantity),
                        "La transaction a été refusée ou interrompue.",
                        "Aucune confirmation de rachat n'est ajoutée pour cette tentative."
                    }
            );
        }
    }


    void openBuybackMenu(Player& player, const ShopInventory& shop)
    {
        bool buyingBack = true;
        std::size_t pageIndex = 0;
        const std::size_t itemsPerPage = 10;

        while (buyingBack)
        {
            Console::clear();
            const int count = ShopTransactionSystem::getBuybackEntryCount(shop.getType());
            MenuScreen screen("RACHAT", "shop.buyback");
            screen.addLine("Boutique : " + shop.getName());
            screen.addLine("Argent actuel : " + Money::formatCurrencyOverviewFromCopper(player.getInventory().getTotalCopper()));
            screen.addLine("Les objets vendus ici peuvent être rachetés jusqu'au prochain combat.");
            screen.addLine("Le prix est plus haut que la revente : frais, paperasse, mauvaise foi du marchand, bref la vie.");

            if (count <= 0)
            {
                screen.addLine("Aucun objet à racheter dans cette boutique.");
                screen.addBackOption("Retour", "shop.buyback.back");
                TerminalInterface::askMenuChoiceFromOptions(
                    screen,
                    "Entre 0 pour revenir."
                );
                return;
            }

            const std::size_t totalEntries = static_cast<std::size_t>(count);
            const std::size_t totalPages = PagedMenu::pageCount(totalEntries, itemsPerPage);
            if (pageIndex >= totalPages)
            {
                pageIndex = totalPages > 0 ? totalPages - 1 : 0;
            }

            const std::size_t first = PagedMenu::firstIndex(pageIndex, itemsPerPage);
            const std::size_t last = PagedMenu::lastIndexExclusive(totalEntries, pageIndex, itemsPerPage);
            const int localCount = static_cast<int>(last - first);

            screen.setPagination(pageIndex, totalPages);
            screen.addLine(PagedMenu::pageInfoText(pageIndex, totalPages, totalEntries));
            screen.addLine("Affichage : " + PagedMenu::rangeText(first, last, totalEntries));

            for (std::size_t i = first; i < last; ++i)
            {
                const int localIndex = static_cast<int>(i - first + 1);
                const int buybackIndex = static_cast<int>(i);
                const std::string name = ShopTransactionSystem::getBuybackEntryName(shop.getType(), buybackIndex);
                const std::string kindLabel = ShopTransactionSystem::getBuybackEntryKindLabel(shop.getType(), buybackIndex);
                const int quantity = ShopTransactionSystem::getBuybackEntryQuantity(shop.getType(), buybackIndex);
                const int price = ShopTransactionSystem::getBuybackEntryPrice(shop.getType(), buybackIndex);
                const bool affordable = player.getInventory().getTotalCopper() >= Money::copperFromGold(price);
                const std::string label = name
                    + (quantity > 1 ? " x" + std::to_string(quantity) : "")
                    + " | Type : " + kindLabel
                    + " | Rachat : " + Money::formatGoldWithRaw(price)
                    + " | " + (affordable ? "Récupérable" : "Argent insuffisant");

                MenuOptionItemData itemData;
                itemData.structured = true;
                itemData.kind = "shop";
                itemData.section = "Rachat";
                itemData.actionType = "buyback";
                itemData.name = name;
                itemData.quantity = quantity > 1 ? "x" + std::to_string(quantity) : "";
                itemData.price = Money::formatGoldWithRaw(price);
                itemData.status = affordable ? "Avant prochain combat" : "Argent insuffisant";
                itemData.detail = "Récupérer un objet vendu récemment dans cette boutique.";
                itemData.important = !affordable;

                screen.addOption(
                    localIndex,
                    label,
                    affordable
                        ? itemData.detail
                        : "Entrée visible, mais la valeur totale du portefeuille ne suffit pas pour la récupérer maintenant.",
                    affordable,
                    "shop.buyback.select." + std::to_string(buybackIndex),
                    itemData
                );
            }

            PagedMenu::addNavigationOptions(
                screen,
                pageIndex,
                totalPages,
                "shop.buyback.back",
                "shop.buyback.previous",
                "shop.buyback.next",
                "Revenir au menu de la boutique.",
                "Voir les rachats précédents.",
                "Voir les rachats suivants."
            );

            int choice = TerminalInterface::askMenuChoiceFromOptions(
                screen,
                "Choix refusé : sélectionne une entrée affichée, 98/99 pour tourner les pages, ou 0 pour revenir."
            );
            if (choice == 0)
            {
                buyingBack = false;
                continue;
            }

            if (choice == 98 && pageIndex > 0)
            {
                pageIndex--;
                continue;
            }

            if (choice == 99 && pageIndex + 1 < totalPages)
            {
                pageIndex++;
                continue;
            }

            if (choice < 1 || choice > localCount)
            {
                showShopResult(
                    "CHOIX INDISPONIBLE",
                    "shop.buyback.invalid_choice",
                    {
                        "Cette entrée n'existe pas sur la page de rachat actuelle.",
                        "Utilise uniquement les choix affichés par la boutique."
                    }
                );
                continue;
            }

            const int buybackIndex = static_cast<int>(first) + choice - 1;
            const std::string buybackName = ShopTransactionSystem::getBuybackEntryName(shop.getType(), buybackIndex);
            const int buybackQuantity = ShopTransactionSystem::getBuybackEntryQuantity(shop.getType(), buybackIndex);
            const int buybackPrice = ShopTransactionSystem::getBuybackEntryPrice(shop.getType(), buybackIndex);
            const long long copperBeforeBuyback = player.getInventory().getTotalCopper();

            const bool confirmBuyback = askShopConfirmation(
                "CONFIRMER LE RACHAT",
                "shop.buyback.confirm",
                {
                    "Boutique : " + shop.getName(),
                    "Objet : " + buybackName,
                    "Prix de récupération : " + Money::formatGoldWithRaw(buybackPrice),
                    "Argent disponible : " + Money::formatCurrencyOverviewFromCopper(copperBeforeBuyback),
                    "Limite : cette occasion disparaît au prochain combat."
                },
                "Racheter l'objet",
                "Annuler le rachat",
                "shop.buyback"
            );

            if (!confirmBuyback)
            {
                showShopResult(
                    "RACHAT ANNULÉ",
                    "shop.buyback.cancelled",
                    {
                        "Objet : " + buybackName,
                        "L'objet reste disponible tant qu'aucun combat n'est lancé.",
                        "Aucune pièce n'a été dépensée."
                    }
                );
                continue;
            }

            ShopTransactionSystem::clearLastTransactionNotes();
            const bool buybackSucceeded = ShopTransactionSystem::buyBackEntry(player, shop.getType(), buybackIndex);
            showShopTransactionResult(
                buybackSucceeded ? "RACHAT TERMINÉ" : "RACHAT REFUSÉ",
                buybackSucceeded ? "shop.buyback.result.success" : "shop.buyback.result.failed",
                buybackSucceeded
                    ? std::vector<std::string>{
                        "Objet récupéré : " + buybackName,
                        "Quantité récupérée : x" + std::to_string(std::max(1, buybackQuantity)),
                        "Prix payé : " + Money::formatGoldWithRaw(buybackPrice),
                        "Argent avant : " + Money::formatCurrencyOverviewFromCopper(copperBeforeBuyback),
                        "Argent actuel : " + Money::formatCurrencyOverviewFromCopper(player.getInventory().getTotalCopper()),
                        "L'entrée de rachat a été retirée de cette boutique."
                    }
                    : std::vector<std::string>{
                        "Objet demandé : " + buybackName,
                        "Prix demandé : " + Money::formatGoldWithRaw(buybackPrice),
                        "Argent actuel : " + Money::formatCurrencyOverviewFromCopper(player.getInventory().getTotalCopper()),
                        "Raison possible : argent insuffisant ou entrée déjà disparue."
                    }
            );
        }
    }

    bool isBobMauriceTemporaryShop(const ShopInventory& shop)
    {
        return shop.getName().rfind("Bob et Maurice — ", 0) == 0;
    }

    void addBobMauriceCrateEquipment(Player& trialPlayer, Random& random, std::vector<std::string>& openedLines)
    {
        const std::vector<Weapon> weapons = {
            WeaponCatalog::createTrainingDagger(),
            WeaponCatalog::createTrainingSpear(),
            WeaponCatalog::createTrainingBow(),
            WeaponCatalog::createTrainingStaff(),
            WeaponCatalog::createHeavyTrainingAxe(),
            WeaponCatalog::createBalancedRapier()
        };
        const std::vector<Armor> armors = {
            ArmorCatalog::createWornLeatherArmor(),
            ArmorCatalog::createApprenticeRobe(),
            ArmorCatalog::createPaddedVest(),
            ArmorCatalog::createHeavyPaddedArmor(),
            ArmorCatalog::createReinforcedLeatherArmor(),
            ArmorCatalog::createTravelerScaleVest()
        };
        const std::vector<Consumable> consumables = {
            ConsumableCatalog::createBasicHealingPotion(),
            ConsumableCatalog::createMinorHealingPotion(),
            ConsumableCatalog::createBasicDamagePotion(),
            ConsumableCatalog::createDefensivePotion(),
            ConsumableCatalog::createPrecisionPotion(),
            ConsumableCatalog::createSmokeEscapeVial(),
            ConsumableCatalog::createLuckyPotion(),
            ConsumableCatalog::createUnluckyPotion()
        };

        const int firstWeapon = random.between(0, static_cast<int>(weapons.size()) - 1);
        int secondWeapon = random.between(0, static_cast<int>(weapons.size()) - 1);
        if (secondWeapon == firstWeapon) secondWeapon = (secondWeapon + 1) % static_cast<int>(weapons.size());
        const int firstArmor = random.between(0, static_cast<int>(armors.size()) - 1);
        int secondArmor = random.between(0, static_cast<int>(armors.size()) - 1);
        if (secondArmor == firstArmor) secondArmor = (secondArmor + 1) % static_cast<int>(armors.size());

        trialPlayer.getInventory().addWeapon(weapons[firstWeapon]);
        trialPlayer.getInventory().addWeapon(weapons[secondWeapon]);
        trialPlayer.getInventory().addArmor(armors[firstArmor]);
        trialPlayer.getInventory().addArmor(armors[secondArmor]);

        openedLines.push_back("Caisse d'armes : " + weapons[firstWeapon].getName() + " ou " + weapons[secondWeapon].getName() + ".");
        openedLines.push_back("Caisse de protections : " + armors[firstArmor].getName() + " ou " + armors[secondArmor].getName() + ".");

        const int consumableCount = random.between(2, 4);
        std::string consumableLine = "Petite caisse de survie : ";
        for (int i = 0; i < consumableCount; ++i)
        {
            const Consumable& item = consumables[random.between(0, static_cast<int>(consumables.size()) - 1)];
            trialPlayer.getInventory().addConsumable(item);
            if (i > 0) consumableLine += ", ";
            consumableLine += item.getName();
        }
        consumableLine += ".";
        openedLines.push_back(consumableLine);
    }

    bool prepareBobMauriceTrialEquipment(Player& trialPlayer)
    {
        while (true)
        {
            MenuScreen screen("SOUS-INVENTAIRE DES CAISSES", "shop.bob_maurice.crate_trial.equipment");
            screen.addLine("Seuls les objets sortis des caisses existent dans cet inventaire temporaire.");
            screen.addLine("Arme équipée : " + (trialPlayer.hasEquippedWeapon() ? trialPlayer.getEquippedWeapon().getName() : std::string("aucune")) + ".");
            screen.addLine("Armure équipée : " + (trialPlayer.hasEquippedArmor() ? trialPlayer.getEquippedArmor().getName() : std::string("aucune")) + ".");
            screen.addLine("Le véritable inventaire, l'argent et l'équipement du personnage restent hors de cette épreuve.");
            screen.addOption(0, "Abandonner le défi", "Rend les objets temporaires à Bob et Maurice.", true, "shop.bob_maurice.crate_trial.cancel");
            screen.addOption(1, "Choisir une arme des caisses", "Inspecter puis équiper l'une des deux armes obtenues.", true, "shop.bob_maurice.crate_trial.weapon");
            screen.addOption(2, "Choisir une armure des caisses", "Inspecter puis équiper l'une des deux protections obtenues.", true, "shop.bob_maurice.crate_trial.armor");
            screen.addOption(3, "Partir au combat", "Lancer l'épreuve amicale avec l'équipement temporaire.", trialPlayer.hasEquippedWeapon() && trialPlayer.hasEquippedArmor(), "shop.bob_maurice.crate_trial.start");

            const int choice = TerminalInterface::askMenuChoiceFromOptions(
                screen,
                "Équipe une arme et une armure issues des caisses avant de lancer le combat."
            );
            Console::clear();

            if (choice == 0) return false;
            if (choice == 1)
            {
                EquipmentMenu::equipWeaponFromInventory(trialPlayer);
                continue;
            }
            if (choice == 2)
            {
                EquipmentMenu::equipArmorFromInventory(trialPlayer);
                continue;
            }
            if (choice == 3 && trialPlayer.hasEquippedWeapon() && trialPlayer.hasEquippedArmor())
            {
                return true;
            }
        }
    }

    void runBobMauriceCrateTrial(Player& player)
    {
        if (player.getExplorationSceneCooldownRemainingDays("bob_maurice_crate_trial") > 0)
        {
            return;
        }

        const bool accepted = askShopConfirmation(
            "UN DERNIER PETIT DÉFI",
            "shop.bob_maurice.crate_trial.offer",
            {
                "Bob : Hannnn... hummm... HUUUHHH.",
                "Maurice : « Mon collègue Bob a dit qu'avant de partir, tu pourrais ouvrir quelques caisses et affronter un mini-boss avec uniquement ce qui tombe dedans. »",
                "Maurice : Hammmm... huuuhhhhh...",
                "Bob : « Maurice demande de préciser que la défaite ne compte pas comme une mort. Il dit aussi que les monstres n'ont pas été prévenus du mot amical. »",
                "Règle : un sous-inventaire temporaire remplace tout ton équipement pendant l'épreuve."
            },
            "Ouvrir les caisses",
            "Partir normalement",
            "shop.bob_maurice.crate_trial"
        );
        if (!accepted)
        {
            return;
        }

        Random random;
        Player trialPlayer = player;
        trialPlayer.unequipWeapon();
        trialPlayer.unequipArmor();
        trialPlayer.getInventory().clearAll();
        trialPlayer.reviveWithHealthPercentage(100);
        for (Quest& quest : trialPlayer.getQuestLog().getQuests())
        {
            if (quest.guildChallenge
                || quest.origin == "Défi du Hero Villager"
                || quest.id.rfind("bob_maurice_protection_", 0) == 0)
            {
                quest.accepted = false;
            }
        }

        std::vector<std::string> openedLines = {
            "Bob pose trois caisses au sol sans jamais expliquer d'où elles viennent.",
            "Maurice vérifie discrètement qu'aucune ne respire. Deux sur trois passent le contrôle."
        };
        addBobMauriceCrateEquipment(trialPlayer, random, openedLines);
        MessageScreen::show("CAISSES OUVERTES", "shop.bob_maurice.crate_trial.opened", openedLines, false);

        if (!prepareBobMauriceTrialEquipment(trialPlayer))
        {
            MessageScreen::show(
                "DÉFI ANNULÉ",
                "shop.bob_maurice.crate_trial.cancelled",
                {
                    "Bob : Hmmmm...",
                    "Maurice : « Mon collègue Bob a dit qu'il allait remettre les objets dans les mauvaises caisses pour la prochaine fois. »",
                    "Aucun objet temporaire n'est conservé."
                },
                false
            );
            return;
        }

        const int level = std::max(2, player.getLevel() + 1);
        std::vector<Monster> enemies;
        enemies.emplace_back(
            "Le Champion de la caisse cabossée",
            "Mini-boss amical / contenu non contractuel",
            Race::Aberration,
            level,
            95 + level * 22,
            7 + level * 2,
            11 + level * 3,
            17 + level * 4,
            0,
            0,
            false,
            true,
            false,
            false
        );
        if (player.getLevel() >= 10)
        {
            enemies.emplace_back(
                "Le Petit supplément non demandé",
                "Créature de caisse / assistant du mini-boss",
                Race::Bete,
                std::max(2, level - 1),
                45 + level * 12,
                4 + level,
                8 + level * 2,
                12 + level * 3,
                0,
                0,
                false,
                false,
                false,
                false
            );
        }

        player.startExplorationSceneCooldown("bob_maurice_crate_trial", 7);
        Console::useCombatTheme();
        const bool victory = MonsterPveMode::runExplorationWave(
            trialPlayer,
            random,
            DifficultyMode::Normal,
            DeathRuleMode::NonDefinitive,
            enemies,
            "Défi des caisses de Bob et Maurice",
            true
        );
        Console::useNormalTheme();

        if (victory)
        {
            const int experienceReward = 16 + player.getLevel() * 2;
            const int copperReward = 45 + player.getLevel() * 4;
            player.gainExperience(experienceReward);
            player.getInventory().earnCopper(copperReward);
            player.getInventory().addMaterial(MaterialCatalog::createById("guild_challenge_mark", 1));
            const bool newTitle = player.grantTitle("Survivant des caisses");
            MessageScreen::show(
                "DÉFI DES CAISSES RÉUSSI",
                "shop.bob_maurice.crate_trial.success",
                {
                    "Bob : HANN... hummm... hammmm !",
                    "Maurice : « Mon collègue Bob a dit que tout était parfaitement équilibré. C'est faux, mais il est très content. »",
                    "Expérience : +" + std::to_string(experienceReward) + ".",
                    "Récompense : " + Money::formatCurrencyOverviewFromCopper(copperReward) + ".",
                    "Marque de défi : +1.",
                    newTitle ? "Titre obtenu : Survivant des caisses." : "Titre déjà connu : Survivant des caisses.",
                    "Tous les objets des caisses disparaissent avec le sous-inventaire temporaire."
                },
                false
            );
            return;
        }

        if (!trialPlayer.isDead())
        {
            MessageScreen::show(
                "ÉPREUVE QUITTÉE",
                "shop.bob_maurice.crate_trial.escaped",
                {
                    "Bob : Hmmmm... hannn.",
                    "Maurice : « Mon collègue Bob a dit que fuir reste une décision commerciale parfaitement valable. Il est un peu vexé. »",
                    "Aucune récompense n'est accordée et aucun objet temporaire n'est conservé.",
                    "Le Hero Villager n'intervient pas : personne n'a été mis hors combat."
                },
                false
            );
            return;
        }

        player.grantTitle("Témoin du marchand bleu");
        BestiaryRuntimeProgress::recordEncounter(
            "Le marchand bleu qui juge les routes",
            "Légendes / contes",
            "Rumeur confirmée après son intervention dans l'épreuve des caisses."
        );
        MessageScreen::show(
            "UN MIRAGE EN ARMURE BLEUE",
            "shop.bob_maurice.crate_trial.hero_rescue",
            {
                "Au moment où le mini-boss tente de poursuivre le combat, l'air se découpe derrière lui.",
                "Un homme très musclé apparaît : t-shirt bleu-vert, pantalon violet et armure de diamant bleu.",
                "Hmmm... Le défi est terminé. Pas la peine d'insister... Huuuh.",
                "Il traverse le champ de bataille en un mouvement. Le ou les ennemis s'effondrent avant même que le bruit du coup arrive.",
                "Sa silhouette se fragmente ensuite en carrés bleutés et s'efface comme un mirage.",
                "Cette défaite amicale n'a provoqué aucune mort, aucune pénalité et aucune perte réelle."
            },
            false
        );
    }

    // EN: openSingleShop declares or implements a focused behavior used by this module.
    // FR: openSingleShop déclare ou implémente un comportement précis utilisé par ce module.
    void openSingleShop(Player& player, ShopInventory& shop)
    {
        bool stayInShop = true;
        bool bobMauriceTrialOfferedThisVisit = false;

        if (shop.getType() == ShopType::Library)
        {
            Random loreRandom;
            LegendTriggerSystem::maybeDisplayLibraryLoreWhisper(loreRandom);
        }

        while (stayInShop)
        {
            Console::clear();
            const int buybackCount = ShopTransactionSystem::getBuybackEntryCount(shop.getType());
            int shopChoice = TerminalInterface::askMenuChoiceFromOptions(
                buildShopMainScreen(shop, player),
                (shop.getType() == ShopType::Lodging || shop.getType() == ShopType::Transport || shop.getType() == ShopType::CityService || shop.getType() == ShopType::Church || shop.getType() == ShopType::Enchanter || shop.getType() == ShopType::Library)
                    ? "Veuillez choisir acheter, vendre, discuter, quêtes, service spécial, ou 0 pour revenir."
                    : (buybackCount > 0
                        ? "Veuillez choisir acheter, vendre, discuter, quêtes, rachat, ou 0 pour revenir."
                        : "Veuillez choisir acheter, vendre, discuter, quêtes, ou 0 pour revenir.")
            );

            if (shopChoice == 0)
            {
                if (isBobMauriceTemporaryShop(shop)
                    && !bobMauriceTrialOfferedThisVisit
                    && player.getExplorationSceneCooldownRemainingDays("bob_maurice_crate_trial") <= 0)
                {
                    bobMauriceTrialOfferedThisVisit = true;
                    runBobMauriceCrateTrial(player);
                }
                stayInShop = false;
                continue;
            }

            if (!shopIsOpenForPlayer(shop, player))
            {
                showShopResult(
                    "BOUTIQUE FERMÉE",
                    "shop.single.closed",
                    {
                        shop.getName() + " est fermée pour le moment.",
                        shopOpenStatusLine(shop, player),
                        "Temps actuel : " + player.formatWorldDateTimeLine()
                    }
                );
                continue;
            }

            if (shopChoice == 2)
            {
                openSellMenu(player, shop);
                continue;
            }

            if (shopChoice == 3)
            {
                Console::clear();
                TerminalInterface::renderMenuScreen(buildVendorTalkScreen(shop, player));
                Console::waitForEnter();
                continue;
            }

            if (shopChoice == 4)
            {
                if (isTemporaryRecommendedShop(shop) && !isSpecialLegendaryMerchantShop(shop))
                {
                    showShopResult(
                        "COMPTOIR TEMPORAIRE",
                        "shop.temporary.no_quest_board",
                        {
                            temporaryMerchantDisplayName(shop) + " ne tient pas de tableau de quêtes permanent.",
                            "Ce vendeur est ici grâce à une recommandation de Prunigil et repartira selon son propre calendrier."
                        }
                    );
                    continue;
                }
                Console::clear();
                QuestMenu::talkToClient(
                    player,
                    isTemporaryRecommendedShop(shop) ? temporaryMerchantDisplayName(shop) : getVendorNameForShop(shop.getType())
                );
                continue;
            }

            if (shopChoice == 5)
            {
                openBuybackMenu(player, shop);
                continue;
            }

            if (shopChoice == 6)
            {
                if (shop.getType() == ShopType::Lodging)
                {
                    openLodgingServiceMenu(player);
                    continue;
                }

                if (shop.getType() == ShopType::Transport)
                {
                    openTransportServiceMenu(player);
                    continue;
                }

                if (shop.getType() == ShopType::CityService)
                {
                    openCityServiceSpecialMenu(player);
                    continue;
                }

                if (shop.getType() == ShopType::Church)
                {
                    ChurchServiceMenu::open(player);
                    continue;
                }

                if (shop.getType() == ShopType::Enchanter)
                {
                    EnchanterServiceMenu::open(player);
                    continue;
                }

                if (shop.getType() == ShopType::Library)
                {
                    LanguagePracticeMenu::open(player);
                    continue;
                }

                showShopResult(
                    "SERVICE INDISPONIBLE",
                    "shop.single.service_unavailable",
                    {
                        "Cette boutique ne propose pas encore d'action spéciale hors achat/vente.",
                        "Utilise les choix affichés par le menu pour éviter les actions fantômes."
                    }
                );
                continue;
            }

            bool buyMenuOpen = true;
            std::size_t pageIndex = 0;
            const std::size_t itemsPerPage = 10;

            while (buyMenuOpen)
            {
                Console::clear();
                const MenuScreen stockScreen = buildShopStockScreen(shop, player, pageIndex, itemsPerPage);

                const std::vector<ShopItem>& items = shop.getItems();
                const std::size_t totalPages = PagedMenu::pageCount(items.size(), itemsPerPage);
                const std::size_t first = PagedMenu::firstIndex(pageIndex, itemsPerPage);
                const std::size_t last = PagedMenu::lastIndexExclusive(items.size(), pageIndex, itemsPerPage);
                const int localCount = static_cast<int>(last - first);

                int itemChoice = TerminalInterface::askMenuChoiceFromOptions(
                    stockScreen,
                    "Veuillez choisir un article affiché, 98/99 pour tourner les pages, ou 0 pour revenir."
                );

                if (itemChoice == 0)
                {
                    buyMenuOpen = false;
                    continue;
                }

                if (itemChoice == 98 && pageIndex > 0)
                {
                    pageIndex--;
                    continue;
                }

                if (itemChoice == 99 && pageIndex + 1 < totalPages)
                {
                    pageIndex++;
                    continue;
                }

                if (itemChoice < 1 || itemChoice > localCount)
                {
                    showShopResult(
                        "CHOIX INDISPONIBLE",
                        "shop.stock.invalid_choice",
                        {
                            "Cette entrée n'existe pas sur la page actuelle.",
                            "Utilise les choix affichés par le menu pour continuer."
                        }
                    );
                    continue;
                }

                ShopItem& item = shop.getMutableItems()[first + static_cast<std::size_t>(itemChoice - 1)];
                bool itemMenuOpen = true;

                while (itemMenuOpen)
                {
                    Console::clear();
                    bool barterAvailable = hasBlackMarketBarterOffer(shop, item);
                    int actionChoice = TerminalInterface::askMenuChoiceFromOptions(
                        buildShopItemScreen(shop, item, player, true),
                        barterAvailable
                            ? "Choisis une action disponible : retour, achat, inspection ou troc."
                            : "Choisis une action disponible : retour, achat ou inspection."
                    );

                    if (actionChoice == 0)
                    {
                        itemMenuOpen = false;
                    }
                    else if (actionChoice == 1)
                    {
                        int finalPrice = applyShopBuyPriceForPlayer(shop, item, player);

                        int maxQuantity = getMaxBuyQuantity(item, player, finalPrice);
                        const ShopPromotionOffer activePromotion = promotionForShop(shop, player);
                        const bool discountedPromotionPurchase = activePromotion.discountAvailable()
                            && activePromotion.itemId == item.getId()
                            && promotionDiscountPercentForItem(shop, item, player) > 0;
                        if (discountedPromotionPurchase && activePromotion.clearance)
                        {
                            maxQuantity = std::min(maxQuantity, activePromotion.remainingDiscountedQuantity());
                        }
                        const std::string accessBlockReason = localReputationAccessBlockReason(player, shop.getType(), item);

                        if (!accessBlockReason.empty())
                        {
                            showShopResult(
                                "ACCÈS LOCAL REFUSÉ",
                                "shop.buy.local_reputation_blocked",
                                {
                                    "Article : " + item.getName(),
                                    "Raison : " + accessBlockReason + ".",
                                    localReputationLineForPlayer(player),
                                    "Améliore cette réputation avec des services PNJ propres, des preuves de ville, des bons d'auberge ou des tickets de transport."
                                }
                            );
                        }
                        else if (maxQuantity <= 0)
                        {
                            showShopResult(
                                "ACHAT IMPOSSIBLE",
                                "shop.buy.blocked",
                                {
                                    "Article : " + item.getName(),
                                    "Argent disponible : " + Money::formatCurrencyOverviewFromCopper(player.getInventory().getTotalCopper()),
                                    "Statut : achat refusé pour le moment.",
                                    "Raison possible : argent insuffisant, stock épuisé ou article indisponible."
                                }
                            );
                        }
                        else
                        {
                            int quantity = 1;

                            if (maxQuantity > 1)
                            {
                                quantity = MessageScreen::askQuantity(
                                    "QUANTITÉ À ACHETER",
                                    "shop.buy.quantity",
                                    {
                                        "Article : " + item.getName(),
                                        "Maximum achetable : x" + std::to_string(maxQuantity),
                                        "Prix unitaire : " + Money::formatGoldWithRaw(finalPrice)
                                    },
                                    1,
                                    maxQuantity,
                                    "Veuillez choisir une quantité valide."
                                );
                            }

                            const long long copperBeforePurchase = player.getInventory().getTotalCopper();
                            const int stockBeforePurchase = item.getStock();
                            const int expectedTotalPrice = finalPrice * quantity;

                            const bool confirmPurchase = askShopConfirmation(
                                "CONFIRMER L'ACHAT",
                                "shop.buy.confirm",
                                {
                                    "Article : " + item.getName(),
                                    "Quantité : x" + std::to_string(quantity),
                                    "Prix unitaire : " + Money::formatGoldWithRaw(finalPrice),
                                    "Total prévu : " + Money::formatGoldWithRaw(expectedTotalPrice),
                                    "Argent disponible : " + Money::formatCurrencyOverviewFromCopper(copperBeforePurchase),
                                    "Argent après achat prévu : " + Money::formatCurrencyOverviewFromCopper(std::max(0LL, copperBeforePurchase - Money::copperFromGold(expectedTotalPrice))),
                                    stockBeforePurchase >= 0
                                        ? "Stock avant achat : " + std::to_string(stockBeforePurchase)
                                        : "Stock avant achat : non limité",
                                    stockBeforePurchase >= 0
                                        ? "Stock après achat prévu : " + std::to_string(std::max(0, stockBeforePurchase - quantity))
                                        : "Stock après achat prévu : non limité"
                                },
                                "Confirmer l'achat",
                                "Annuler l'achat",
                                "shop.buy"
                            );

                            if (!confirmPurchase)
                            {
                                showShopResult(
                                    "ACHAT ANNULÉ",
                                    "shop.buy.cancelled",
                                    {
                                        "Article : " + item.getName(),
                                        "Quantité demandée : x" + std::to_string(quantity),
                                        "Aucune pièce n'a été dépensée.",
                                        "Le stock du marchand n'a pas changé."
                                    }
                                );
                            }
                            else
                            {
                                ShopTransactionSystem::clearLastTransactionNotes();
                                int boughtCount = 0;
                                for (int i = 0; i < quantity; ++i)
                                {
                                    if (ShopTransactionSystem::buyItem(player, item, finalPrice))
                                    {
                                        boughtCount++;
                                        if (discountedPromotionPurchase && activePromotion.clearance)
                                        {
                                            player.recordShopPromotionPurchase(activePromotion.purchaseKey, 1);
                                        }
                                    }
                                    else
                                    {
                                        break;
                                    }
                                }

                                showShopTransactionResult(
                                    boughtCount > 0 ? "ACHAT TERMINÉ" : "ACHAT REFUSÉ",
                                    boughtCount > 0 ? "shop.buy.result.success" : "shop.buy.result.failed",
                                    boughtCount > 0
                                        ? std::vector<std::string>{
                                            "Article : " + item.getName(),
                                            "Quantité obtenue : x" + std::to_string(boughtCount) + " / x" + std::to_string(quantity),
                                            "Argent dépensé : " + Money::formatGoldWithRaw(finalPrice * boughtCount),
                                            "Argent avant : " + Money::formatCurrencyOverviewFromCopper(copperBeforePurchase),
                                            "Argent actuel : " + Money::formatCurrencyOverviewFromCopper(player.getInventory().getTotalCopper()),
                                            item.getStock() >= 0
                                                ? "Stock restant : " + std::to_string(item.getStock())
                                                : "Stock restant : non limité",
                                            discountedPromotionPurchase
                                                ? (activePromotion.clearance
                                                    ? "Déstockage utilisé : " + std::to_string(boughtCount) + " unité(s) remisée(s) sur ce lot."
                                                    : "Offre du comptoir appliquée pendant cette transaction.")
                                                : "Tarif normal appliqué."
                                        }
                                        : std::vector<std::string>{
                                            "Article : " + item.getName(),
                                            "Quantité demandée : x" + std::to_string(quantity),
                                            "Aucun exemplaire n'a été ajouté.",
                                            "Raison possible : stock, argent ou compatibilité d'inventaire."
                                        }
                                );
                            }
                        }

                        itemMenuOpen = false;
                    }
                    else if (actionChoice == 3)
                    {
                        int maxBarterQuantity = getMaxBarterQuantity(item, player);

                        if (maxBarterQuantity <= 0)
                        {
                            showShopResult(
                                "TROC IMPOSSIBLE",
                                "shop.barter.blocked",
                                {
                                    "Article : " + item.getName(),
                                    "Demande : " + formatBarterRequirements(item),
                                    "Statut : composants insuffisants.",
                                    "Le contact garde l'article sous le comptoir."
                                }
                            );
                        }
                        else
                        {
                            int quantity = 1;

                            if (maxBarterQuantity > 1)
                            {
                                quantity = MessageScreen::askQuantity(
                                    "QUANTITÉ À TROQUER",
                                    "shop.barter.quantity",
                                    {
                                        "Article : " + item.getName(),
                                        "Maximum échangeable : x" + std::to_string(maxBarterQuantity),
                                        "Demande par unité : " + formatBarterRequirements(item)
                                    },
                                    1,
                                    maxBarterQuantity,
                                    "Veuillez choisir une quantité valide."
                                );
                            }

                            const std::string barterRequirements = formatBarterRequirements(item);
                            const std::string totalBarterRequirements = formatBarterRequirements(item, quantity);
                            const int stockBeforeBarter = item.getStock();
                            const bool confirmBarter = askShopConfirmation(
                                "CONFIRMER LE TROC",
                                "shop.barter.confirm",
                                {
                                    "Article : " + item.getName(),
                                    "Quantité : x" + std::to_string(quantity),
                                    quantity > 1
                                        ? "Demande par unité : " + barterRequirements
                                        : "Demande : " + barterRequirements,
                                    quantity > 1
                                        ? "Demande totale : " + totalBarterRequirements
                                        : "Demande totale : " + barterRequirements,
                                    "Maximum échangeable maintenant : x" + std::to_string(maxBarterQuantity),
                                    stockBeforeBarter >= 0
                                        ? "Stock avant échange : " + std::to_string(stockBeforeBarter)
                                        : "Stock avant échange : non limité",
                                    stockBeforeBarter >= 0
                                        ? "Stock après échange prévu : " + std::to_string(std::max(0, stockBeforeBarter - quantity))
                                        : "Stock après échange prévu : non limité",
                                    "Le marché noir ne promet jamais que l'offre reviendra."
                                },
                                "Confirmer le troc",
                                "Annuler le troc",
                                "shop.barter"
                            );

                            if (!confirmBarter)
                            {
                                showShopResult(
                                    "TROC ANNULÉ",
                                    "shop.barter.cancelled",
                                    {
                                        "Article : " + item.getName(),
                                        "Aucun composant n'a été retiré.",
                                        "Le contact fait semblant de n'avoir jamais proposé l'échange."
                                    }
                                );
                            }
                            else
                            {
                                ShopTransactionSystem::clearLastTransactionNotes();
                                int tradedCount = 0;

                                for (int i = 0; i < quantity; ++i)
                                {
                                    if (getMaxBarterQuantity(item, player) <= 0)
                                    {
                                        break;
                                    }

                                    if (!consumeBarterRequirements(player, item, 1))
                                    {
                                        break;
                                    }

                                    if (!ShopTransactionSystem::buyItem(player, item, 0))
                                    {
                                        refundBarterRequirements(player, item, 1);
                                        break;
                                    }

                                    tradedCount++;
                                }

                                showShopTransactionResult(
                                    tradedCount > 0 ? "TROC TERMINÉ" : "TROC REFUSÉ",
                                    tradedCount > 0 ? "shop.barter.result.success" : "shop.barter.result.failed",
                                    tradedCount > 0
                                        ? std::vector<std::string>{
                                            "Article obtenu : " + item.getName(),
                                            "Quantité obtenue : x" + std::to_string(tradedCount) + " / x" + std::to_string(quantity),
                                            quantity > 1
                                                ? "Coût par unité : " + barterRequirements
                                                : "Coût : " + barterRequirements,
                                            "Coût total consommé : " + formatBarterRequirements(item, tradedCount),
                                            item.getStock() >= 0
                                                ? "Stock restant : " + std::to_string(item.getStock())
                                                : "Stock restant : non limité",
                                            "Le contact range les composants sans demander ton nom."
                                        }
                                        : std::vector<std::string>{
                                            "Article demandé : " + item.getName(),
                                            "Demande : " + barterRequirements,
                                            "Aucun exemplaire n'a été obtenu.",
                                            "Raison possible : composant manquant, stock ou refus d'inventaire."
                                        }
                                );
                            }
                        }

                        itemMenuOpen = false;
                    }
                    else
                    {
                        Console::clear();
                        TerminalInterface::renderMenuScreen(buildShopItemScreen(shop, item, player, false));
                        Console::waitForEnter();
                    }
                }
            }
        }
    }
}


// EN: openShopOfType declares or implements a focused behavior used by this module.
// FR: openShopOfType déclare ou implémente un comportement précis utilisé par ce module.
void ShopMenu::openShopOfType(Player& player, ShopType type)
{
    ShopInventory shop = ShopCatalog::createPreviewShop(type);
    std::vector<ShopInventory> shops;
    shops.push_back(shop);
    applyChapterThreeShopConsequences(player, shops);
    if (shops.empty())
    {
        MessageScreen::show(
            "BOUTIQUE INDISPONIBLE",
            "shop.direct.unavailable",
            {
                "Ce comptoir n'a pas pu être préparé pour le moment.",
                "Retourne par la liste des boutiques si tu veux vérifier les autres services."
            }
        );
        return;
    }

    openSingleShop(player, shops.front());
}

// EN: displayPreview declares or implements a focused behavior used by this module.
// FR: displayPreview déclare ou implémente un comportement précis utilisé par ce module.
void ShopMenu::displayPreview()
{
    std::vector<ShopInventory> shops = ShopCatalog::createAllPreviewShops();
    TerminalInterface::renderMenuScreen(buildShopListScreen(shops, nullptr));
}

// EN: open declares or implements a focused behavior used by this module.
// FR: open déclare ou implémente un comportement précis utilisé par ce module.
void ShopMenu::open(Player& player)
{
    std::vector<ShopInventory> shops = ShopCatalog::createAllPreviewShops();
    applyChapterThreeShopConsequences(player, shops);

    if (player.hasStoryModeStarted() && !player.hasStorySkip())
    {
        shops.erase(
            std::remove_if(shops.begin(), shops.end(), [&](const ShopInventory& shop)
            {
                return !isShopUnlockedForStory(player, shop.getType());
            }),
            shops.end()
        );
    }

    appendTemporaryRecommendedShops(player, shops);

    if (ShopRotationSystem::shouldRefreshShops())
    {
        MessageScreen::show(
            "ROTATION DES BOUTIQUES",
            "shop.rotation.refreshed",
            {
                "Les marchands changent leurs étals après ton dernier combat.",
                "De nouveaux articles peuvent apparaître, disparaître ou revenir plus cher.",
                "Les rachats des ventes précédentes disparaissent avec cette nouvelle rotation."
            }
        );
        ShopRotationSystem::markShopsRefreshed();
    }

    while (true)
    {
        MenuScreen screen("BOUTIQUES ET COMPTOIRS", "shop.unified.menu");
        screen.addLine("Accès unique : plus de doublon entre comptoirs regroupés et liste complète.");
        screen.addLine("Choisis une catégorie rapide ou ouvre la liste complète si tu veux tout vérifier.");
        screen.addLine("Tu n'as plus besoin de retrouver le PNJ exact pour accéder au bon type de service.");
        screen.addBackOption("Retour", "shop.unified.back");
        screen.addOption(1, "Liste complète", "Toutes les boutiques disponibles, avec horaires, stocks et services.", true, "shop.unified.full_list");
        screen.addOption(2, "Potions / consommables", "Soins fixes, soins en %, rage, garde, antidotes, fioles et rations.", true, "shop.unified.consumable");
        screen.addOption(3, "Armes", "Armes simples, armes de progression et comparaisons.", true, "shop.unified.weapon");
        screen.addOption(4, "Armures", "Protections, survie et pièces défensives.", true, "shop.unified.armor");
        screen.addOption(5, "Forge / réparations", "Réparations, kits, matériaux d'atelier et services de forgeron.", true, "shop.unified.blacksmith");
        screen.addOption(6, "Matériaux / composants", "Matériaux de base, composants rares et ressources de terrain.", true, "shop.unified.materials");
        screen.addOption(7, "Plantes / alchimie", "Plantes, recettes, potions avancées et variantes proportionnelles.", true, "shop.unified.alchemy");
        screen.addOption(8, "Bibliothèque / infos", "Livres, renseignements, dossiers et savoirs achetables.", true, "shop.unified.library");
        screen.addOption(9, "Transport / voyage", "Rations de route, trajets et services de déplacement.", true, "shop.unified.transport");
        screen.addOption(10, "Auberge", "Repos, chambres, repas et services d'auberge.", true, "shop.unified.lodging");
        screen.addOption(11, "Marché noir", "Comptoir risqué, troc et marchandises moins officielles.", true, "shop.unified.black_market");
        screen.addOption(12, "Église / soins", "Soins, bénédictions, malédictions et services religieux.", true, "shop.unified.church");
        screen.addOption(13, "Services de ville", "Papiers, services municipaux, petites démarches et demandes locales.", true, "shop.unified.city_service");
        screen.addOption(14, "Enchantements", "Améliorations magiques, effets spéciaux et services d'enchanteur.", true, "shop.unified.enchanter");
        screen.addOption(15, "Matériaux de monstres", "Pièces de monstres, revente spécialisée et composants de chasse.", true, "shop.unified.monster_materials");

        const int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choisis une catégorie de boutique.");
        Console::clear();

        if (choice == 0) return;

        if (choice == 1)
        {
            bool stayInList = true;
            while (stayInList)
            {
                Console::clear();
                const int listChoice = TerminalInterface::askMenuChoiceFromOptions(
                    buildShopListScreen(shops, &player),
                    "Veuillez choisir une boutique affichée, ou 0 pour revenir aux catégories."
                );

                if (listChoice == 0)
                {
                    stayInList = false;
                    continue;
                }

                openSingleShop(player, shops[listChoice - 1]);
            }
        }
        else if (choice == 2) ShopMenu::openShopOfType(player, ShopType::Consumable);
        else if (choice == 3) ShopMenu::openShopOfType(player, ShopType::Weapon);
        else if (choice == 4) ShopMenu::openShopOfType(player, ShopType::Armor);
        else if (choice == 5) ShopMenu::openShopOfType(player, ShopType::Blacksmith);
        else if (choice == 6) ShopMenu::openShopOfType(player, ShopType::Material);
        else if (choice == 7) ShopMenu::openShopOfType(player, ShopType::Alchemist);
        else if (choice == 8) ShopMenu::openShopOfType(player, ShopType::Library);
        else if (choice == 9) ShopMenu::openShopOfType(player, ShopType::Transport);
        else if (choice == 10) ShopMenu::openShopOfType(player, ShopType::Lodging);
        else if (choice == 11) ShopMenu::openShopOfType(player, ShopType::BlackMarket);
        else if (choice == 12) ShopMenu::openShopOfType(player, ShopType::Church);
        else if (choice == 13) ShopMenu::openShopOfType(player, ShopType::CityService);
        else if (choice == 14) ShopMenu::openShopOfType(player, ShopType::Enchanter);
        else if (choice == 15) ShopMenu::openShopOfType(player, ShopType::MonsterMaterial);
    }
}

