// EN: ShopMenu.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: ShopMenu.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.
// English: Displays the first usable shop menu, with some real purchases and prepared future categories.
// Français : Affiche le premier menu de boutique utilisable, avec certains achats réels et des catégories futures préparées.

#include "interface/menu/shop/ShopMenu.hpp"

#include "core/Console.hpp"
#include "combat/modes/pve/MonsterPveMode.hpp"
#include "entity/Monster.hpp"
#include "interface/menu/EquipmentMenu.hpp"
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

#include <algorithm>
#include <cctype>
#include <iostream>
#include <random>
#include <string>
#include <vector>
#include <cstdint>

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

    struct CityEventOfDay
    {
        std::string id;
        std::string name;
        std::string mood;
        std::string action;
    };

    CityEventOfDay cityEventForAbsoluteDay(int day, const Player* player);
    bool hasRoyalBonusCityEventToday(const Player& player);

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

    struct LocalReputationSummary
    {
        int score = 0;
        int successfulPersonalServices = 0;
        int failedPersonalServices = 0;
        int warningNotes = 0;
        std::string label = "inconnue";
        int discountPercent = 0;
    };

    LocalReputationSummary localReputationForPlayer(const Player& player)
    {
        LocalReputationSummary summary;
        for (const Quest& quest : player.getQuestLog().getQuests())
        {
            if (!quest.guildQuest)
            {
                if (quest.turnedIn)
                {
                    ++summary.successfulPersonalServices;
                }
                if (quest.failed)
                {
                    ++summary.failedPersonalServices;
                }
            }
        }

        int proofScore = 0;
        proofScore += countMaterial(player, "local_service_letter") * 2;
        proofScore += countMaterial(player, "client_recommendation") * 3;
        proofScore += countMaterial(player, "guild_favor_token") * 3;
        proofScore += countMaterial(player, "municipal_proof_letter") * 2;
        proofScore += countMaterial(player, "local_reputation_note") * 1;
        proofScore += countMaterial(player, "city_service_stamp") * 1;
        proofScore += countMaterial(player, "city_defense_medal") * 4;
        proofScore += countMaterial(player, "warm_meal_voucher") * 1;
        proofScore += countMaterial(player, "lodging_bed_token") * 1;
        proofScore += countMaterial(player, "stable_stall_ticket") * 1;
        proofScore += countMaterial(player, "travel_pass_note") * 1;
        proofScore += countMaterial(player, "route_toll_receipt") * 1;
        proofScore += countMaterial(player, "caravan_seat_ticket") * 2;
        proofScore += countMaterial(player, "guarded_transport_pass") * 3;
        proofScore += countMaterial(player, "stable_box_reservation") * 2;
        proofScore += countMaterial(player, "rental_mount_voucher") * 2;
        proofScore += countMaterial(player, "relay_route_badge") * 2;

        summary.warningNotes = countMaterial(player, "local_service_warning");
        const int serviceScore = summary.successfulPersonalServices * 3;
        const int warningScore = summary.warningNotes * 2;
        const int penaltyScore = summary.failedPersonalServices * 2 + warningScore;
        summary.score = std::max(0, proofScore + serviceScore - penaltyScore);

        if (summary.score >= 65)
        {
            summary.label = "partenaire fiable de la ville";
            summary.discountPercent = 15;
        }
        else if (summary.score >= 45)
        {
            summary.label = "habituée des comptoirs";
            summary.discountPercent = 12;
        }
        else if (summary.score >= 28)
        {
            summary.label = "fiable en ville";
            summary.discountPercent = 8;
        }
        else if (summary.score >= 14)
        {
            summary.label = "utile localement";
            summary.discountPercent = 5;
        }
        else if (summary.score >= 5)
        {
            summary.label = "connue de quelques PNJ";
            summary.discountPercent = 0;
        }

        return summary;
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

    int cityRepairDaysRemaining(const Player& player)
    {
        return player.getInventory().countMaterialById("city_repair_days_marker");
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

    constexpr int kCityEconomyDiscountCapPercent = 18;

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
        const int demandPressure = std::max(0, activityModifier);
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
        if (!isLocalServiceShop(type))
        {
            return "";
        }

        const LocalReputationSummary summary = localReputationForPlayer(player);
        const std::string id = item.getId();

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

        line += ".";
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

        return screen;
    }

    int getMaxBuyQuantity(const ShopItem& item, const Player& player, int finalPrice);

    MenuScreen buildVendorTalkScreen(const ShopInventory& shop, const Player& player)
    {
        const bool temporaryRecommended = isTemporaryRecommendedShop(shop);
        const std::string vendorName = temporaryRecommended ? temporaryMerchantDisplayName(shop) : getVendorNameForShop(shop.getType());
        MenuScreen screen("DISCUSSION", "shop.vendor_talk");
        screen.addLine(vendorName + " prend quelques secondes pour parler boutique, rumeurs et besoins du moment.");
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

    std::string worldTimeLineForPlayer(const Player& player)
    {
        return player.formatWorldDateTimeLine();
    }

    struct LocalSubscriptionOffer
    {
        std::string id;
        std::string name;
        int price;
        std::string description;
    };

    std::string subscriptionStatusLine(const Player& player, const LocalSubscriptionOffer& offer)
    {
        const int expiresAt = player.getLocalSubscriptionExpiresAtDay(offer.id);
        if (expiresAt < 0)
        {
            return "Abonnement : inactif.";
        }

        std::string line = "Abonnement : actif jusqu'à la fin du jour " + std::to_string(expiresAt + 1) + ".";
        if (player.isLocalSubscriptionCancellationRequested(offer.id))
        {
            line += " Annulation demandée : il reste actif jusqu'à cette date, puis disparaîtra.";
        }
        else
        {
            line += " Renouvellement possible par période de 7 jours.";
        }

        return line;
    }

    bool subscriptionCoversService(const Player& player, const std::string& serviceKind)
    {
        if (serviceKind == "lodging")
        {
            return player.hasActiveLocalSubscription("lodging_modest_weekly")
                || player.hasActiveLocalSubscription("guild_adventurer_standard_weekly")
                || player.hasActiveLocalSubscription("guild_adventurer_silver_weekly");
        }

        if (serviceKind == "stable")
        {
            return player.hasActiveLocalSubscription("stable_relay_weekly")
                || player.hasActiveLocalSubscription("trade_route_weekly");
        }

        if (serviceKind == "transport")
        {
            return player.hasActiveLocalSubscription("trade_route_weekly");
        }

        if (serviceKind == "city")
        {
            return player.hasActiveLocalSubscription("guild_adventurer_standard_weekly")
                || player.hasActiveLocalSubscription("merchant_cotisation_weekly");
        }

        return false;
    }

    std::string serviceCostLine(const Player& player, const std::string& voucherId, const std::string& voucherName, int fallbackPrice)
    {
        const int voucherCount = player.getInventory().countMaterialById(voucherId);
        if (voucherCount > 0)
        {
            return "Coût prévu : " + voucherName + " x1 déjà présent dans l'inventaire.";
        }

        return "Coût prévu : " + Money::formatGoldWithRaw(fallbackPrice) + " si aucun bon/ticket n'est présenté.";
    }

    bool payServiceWithVoucherOrGold(
        Player& player,
        const std::string& voucherId,
        const std::string& voucherName,
        int fallbackPrice,
        std::vector<std::string>& resultLines
    );

    bool payServiceWithSubscriptionVoucherOrGold(
        Player& player,
        const std::string& serviceKind,
        const std::string& voucherId,
        const std::string& voucherName,
        int fallbackPrice,
        std::vector<std::string>& resultLines
    )
    {
        if (subscriptionCoversService(player, serviceKind))
        {
            resultLines.push_back("Abonnement utilisé : aucun bon ni paiement direct n'est consommé pour ce service.");
            return true;
        }

        return payServiceWithVoucherOrGold(player, voucherId, voucherName, fallbackPrice, resultLines);
    }

    void openSingleSubscriptionMenu(Player& player, const LocalSubscriptionOffer& offer)
    {
        bool stay = true;

        while (stay)
        {
            MenuScreen screen("ABONNEMENT", "shop.subscription.single");
            screen.addLine(offer.name);
            screen.addLine(offer.description);
            screen.addLine("Durée : 7 jours. Si tu annules, l'effet reste actif jusqu'à la fin de la période déjà payée.");
            screen.addLine("Prix de période : " + Money::formatGoldWithRaw(offer.price) + ".");
            screen.addLine("Temps actuel : " + player.formatWorldDateTimeLine());
            screen.addLine(subscriptionStatusLine(player, offer));
            screen.addOption(0, "Retour", "Revenir aux abonnements.", true, "shop.subscription.back");
            screen.addOption(1, player.hasActiveLocalSubscription(offer.id) ? "Renouveler 7 jours" : "Prendre l'abonnement 7 jours", "Paye ou renouvelle une période de 7 jours à partir d'aujourd'hui.", true, "shop.subscription.activate");
            screen.addOption(2, "Demander l'annulation", "L'abonnement reste actif jusqu'à la date de fin déjà payée.", player.hasActiveLocalSubscription(offer.id), "shop.subscription.cancel");

            Console::clear();
            const int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choisis une action d'abonnement.");

            if (choice == 0)
            {
                stay = false;
                continue;
            }

            if (choice == 1)
            {
                std::vector<std::string> lines;
                if (!player.getInventory().spendGold(offer.price))
                {
                    lines.push_back("Paiement refusé : il manque " + Money::formatGoldWithRaw(offer.price) + ".");
                    lines.push_back("Argent disponible : " + Money::formatCurrencyOverviewFromCopper(player.getInventory().getTotalCopper()) + ".");
                    showShopResult("ABONNEMENT REFUSÉ", "shop.subscription.failed", lines);
                    continue;
                }

                player.activateLocalSubscription(offer.id, offer.name, 7, offer.price);
                lines.push_back("Abonnement actif : " + offer.name + ".");
                lines.push_back("Fin de période : fin du jour " + std::to_string(player.getLocalSubscriptionExpiresAtDay(offer.id) + 1) + ".");
                lines.push_back("Annulation possible : l'effet restera jusqu'à la fin de la période déjà payée.");
                showShopResult("ABONNEMENT VALIDÉ", "shop.subscription.success", lines);
            }
            else if (choice == 2)
            {
                std::vector<std::string> lines;
                if (player.requestLocalSubscriptionCancellation(offer.id))
                {
                    lines.push_back("Annulation enregistrée pour : " + offer.name + ".");
                    lines.push_back("L'abonnement reste actif jusqu'à la fin du jour " + std::to_string(player.getLocalSubscriptionExpiresAtDay(offer.id) + 1) + ".");
                    lines.push_back("Aucun remboursement : le comptoir appelle ça une cotisation, évidemment.");
                    showShopResult("ABONNEMENT ANNULÉ", "shop.subscription.cancelled", lines);
                }
                else
                {
                    lines.push_back("Aucun abonnement actif à annuler.");
                    showShopResult("ANNULATION IMPOSSIBLE", "shop.subscription.cancel.failed", lines);
                }
            }
        }
    }

    void openSubscriptionMenu(Player& player, const std::string& title, const std::vector<LocalSubscriptionOffer>& offers)
    {
        bool stay = true;
        while (stay)
        {
            MenuScreen screen(title, "shop.subscription.list");
            screen.addLine("Temps actuel : " + player.formatWorldDateTimeLine());
            screen.addLine("Les abonnements durent 7 jours. Une annulation ne coupe jamais l'effet immédiatement.");
            screen.addOption(0, "Retour", "Revenir au service précédent.", true, "shop.subscription.list.back");

            for (std::size_t i = 0; i < offers.size(); ++i)
            {
                std::string label = offers[i].name + " | " + Money::formatGoldWithRaw(offers[i].price);
                if (player.hasActiveLocalSubscription(offers[i].id))
                {
                    label += " | actif jusqu'au jour " + std::to_string(player.getLocalSubscriptionExpiresAtDay(offers[i].id) + 1);
                    if (player.isLocalSubscriptionCancellationRequested(offers[i].id))
                    {
                        label += " | annulation demandée";
                    }
                }
                screen.addOption(static_cast<int>(i + 1), label, offers[i].description, true, "shop.subscription.open." + std::to_string(i + 1));
            }

            Console::clear();
            const int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choisis un abonnement, ou 0 pour revenir.");
            if (choice == 0)
            {
                stay = false;
                continue;
            }
            if (choice > 0 && static_cast<std::size_t>(choice) <= offers.size())
            {
                openSingleSubscriptionMenu(player, offers[static_cast<std::size_t>(choice - 1)]);
            }
        }
    }

    bool payServiceWithVoucherOrGold(
        Player& player,
        const std::string& voucherId,
        const std::string& voucherName,
        int fallbackPrice,
        std::vector<std::string>& resultLines
    )
    {
        if (player.getInventory().countMaterialById(voucherId) > 0)
        {
            player.getInventory().removeMaterialQuantityById(voucherId, 1);
            resultLines.push_back("Justificatif utilisé : " + voucherName + " x1.");
            return true;
        }

        if (!player.getInventory().spendGold(fallbackPrice))
        {
            resultLines.push_back("Paiement refusé : il manque " + Money::formatGoldWithRaw(fallbackPrice) + ".");
            return false;
        }

        resultLines.push_back("Paiement effectué : " + Money::formatGoldWithRaw(fallbackPrice) + ".");
        return true;
    }

    void advanceLocalServiceTime(Player& player, int units, std::vector<std::string>& resultLines)
    {
        const int beforeDay = player.getWorldDaysElapsed();
        const int beforeProgress = player.getWorldDayProgressUnits();
        player.advanceWorldDayUnits(units);

        resultLines.push_back("Temps écoulé : +" + std::to_string(units) + " segment(s) de journée.");
        resultLines.push_back(player.formatWorldTimeChange(beforeDay, beforeProgress));
        std::vector<std::string> timeReportLines = player.consumeWorldTimeReportLines();
        resultLines.insert(resultLines.end(), timeReportLines.begin(), timeReportLines.end());

        const int expired = player.getQuestLog().expireOverdueQuests(player.getWorldDaysElapsed());
        if (expired > 0)
        {
            resultLines.push_back("Attention : " + std::to_string(expired) + " quête(s) ont dépassé leur date limite pendant ce service.");
        }
    }

    void showLocalServiceResult(
        const std::string& title,
        const std::string& screenId,
        Player& player,
        std::vector<std::string> resultLines,
        int timeUnits
    )
    {
        if (timeUnits > 0)
        {
            advanceLocalServiceTime(player, timeUnits, resultLines);
        }

        resultLines.push_back(localReputationLineForPlayer(player));
        showShopResult(title, screenId, resultLines);
    }


    const PlayerCurse* findActiveCurseById(const Player& player, const std::string& curseId)
    {
        const std::vector<PlayerCurse>& curses = player.getActiveCurses();
        for (const PlayerCurse& curse : curses)
        {
            if (curse.id == curseId)
            {
                return &curse;
            }
        }

        return nullptr;
    }

    struct CurseSymptomCategory
    {
        std::string id;
        std::string label;
        std::string detail;
    };

    std::vector<std::string> splitChurchTokenList(const std::string& value)
    {
        std::vector<std::string> tokens;
        std::string current;
        for (char c : value)
        {
            if (c == ',')
            {
                current.erase(std::remove_if(current.begin(), current.end(), [](unsigned char ch) { return std::isspace(ch) != 0; }), current.end());
                if (!current.empty() && std::find(tokens.begin(), tokens.end(), current) == tokens.end()) tokens.push_back(current);
                current.clear();
            }
            else
            {
                current.push_back(c);
            }
        }
        current.erase(std::remove_if(current.begin(), current.end(), [](unsigned char ch) { return std::isspace(ch) != 0; }), current.end());
        if (!current.empty() && std::find(tokens.begin(), tokens.end(), current) == tokens.end()) tokens.push_back(current);
        return tokens;
    }

    bool churchTokenListContains(const std::string& value, const std::string& token)
    {
        const std::vector<std::string> tokens = splitChurchTokenList(value);
        return std::find(tokens.begin(), tokens.end(), token) != tokens.end();
    }

    std::vector<CurseSymptomCategory> getChurchSymptomCategories()
    {
        return {
            {"health", "Santé", "fatigue, malaise, récupération, douleur vague"},
            {"attack", "Attaque", "force offensive, impact, rage, coups moins naturels"},
            {"mana", "Mana / magie", "circulation magique, réserve, instabilité de sort"},
            {"precision", "Précision", "gestes, visée, concentration, coordination"},
            {"defense", "Défense", "résistance, posture, sensation d'être exposé"},
            {"sleep", "Sommeil", "rêves, repos, cauchemars, réveils étranges"},
            {"luck", "Chance", "petits hasards, mauvaises séries, objets qui tombent mal"},
            {"equipment", "Équipement", "réaction des armes, armures, runes ou objets portés"},
            {"spirit", "Esprit", "peur, présence, voix, pensées pas tout à fait claires"},
            {"corruption", "Corruption", "trace sombre, sensation de souillure, anomalie interne"},
            {"travel", "Voyage", "route, orientation, poursuite, odeur, impression d'être suivi"},
            {"social", "Présence sociale", "regards des PNJ, gêne, aura, réactions autour de soi"},
            {"interface", "Interface", "choix affichés, vision de combat, menus qui mentent ou clignotent"},
            {"hallucination", "Hallucinations", "fausses cibles, faux PvE, voix, doubles ou silhouettes impossibles"}
        };
    }

    std::string churchSymptomCategoryLabel(const std::string& id)
    {
        for (const CurseSymptomCategory& category : getChurchSymptomCategories())
        {
            if (category.id == id) return category.label;
        }
        return id;
    }

    std::string curseKnownName(const PlayerCurse& curse)
    {
        return curse.diagnosisLevel <= 0 ? "?????" : curse.name;
    }

    std::string curseDiagnosisLevelText(const PlayerCurse& curse)
    {
        if (curse.diagnosisLevel <= 0) return "inconnue — ?????";
        if (curse.diagnosisLevel == 1) return "niveau 1/3 — globale et vague";
        if (curse.diagnosisLevel == 2) return "niveau 2/3 — approfondie";
        return "niveau 3/3 — totale";
    }

    std::string churchCurseDetailText(const PlayerCurse& curse)
    {
        if (curse.diagnosisLevel <= 0)
        {
            return "Trace inconnue : ?????. Diagnostic nécessaire avant tout exorcisme.";
        }

        std::string detail = "Connaissance : " + curseDiagnosisLevelText(curse) + ". ";
        detail += "Niveau de malédiction : " + std::to_string(std::max(1, curse.curseLevel)) + "/" + std::to_string(std::max(1, curse.maxCurseLevel)) + ". ";
        const std::vector<std::string> discovered = splitChurchTokenList(curse.discoveredSymptomCategories);
        if (!discovered.empty())
        {
            detail += "Symptômes confirmés : ";
            for (std::size_t i = 0; i < discovered.size(); ++i)
            {
                if (i > 0) detail += ", ";
                detail += churchSymptomCategoryLabel(discovered[i]);
            }
            detail += ". ";
        }
        else
        {
            detail += "Symptômes encore vagues. ";
        }

        if (curse.diagnosisLevel >= 2)
        {
            if (curse.evolvesOverTime && curse.curseLevel < curse.maxCurseLevel && curse.nextEscalationDay >= 0)
            {
                detail += "Risque d'aggravation : après le jour " + std::to_string(curse.nextEscalationDay + 1) + ". ";
            }
            if (curse.expiresAtDay >= 0)
            {
                detail += "Expiration naturelle : fin du jour " + std::to_string(curse.expiresAtDay + 1) + ". ";
            }
            else if (curse.lifeLong)
            {
                detail += "Durée : vie entière si rien de spécial n'est fait. ";
            }
            else
            {
                detail += "Durée : indéfinie. ";
            }
        }
        else
        {
            detail += "Durée : ?????. ";
        }

        if (curse.bossIdRequiredToBreak > 0)
        {
            detail += curse.diagnosisLevel >= 3
                ? "Verrou de source : " + (curse.removalHint.empty() ? "vaincre la source." : curse.removalHint)
                : "Verrou de source soupçonné : diagnostic total conseillé.";
        }
        else if (curse.removableByChurch)
        {
            if (curse.evolvesOverTime && curse.becomesSpecialRemovalWhenTooHigh && curse.curseLevel <= curse.churchRemovalMaxLevel)
            {
                detail += "Attention : à trop haut niveau, l'église seule ne suffira plus. ";
            }
            if (curse.exorcismRequiredVisits > 1)
            {
                detail += "Exorcisme progressif : " + std::to_string(curse.exorcismProgress) + "/" + std::to_string(curse.exorcismRequiredVisits) + " passage(s).";
            }
            else
            {
                detail += "Exorcisme court possible ici.";
            }
        }
        else
        {
            detail += curse.diagnosisLevel >= 3 && !curse.removalHint.empty() ? curse.removalHint : "Condition spéciale encore floue.";
        }

        return detail;
    }

    bool rollChurchDiagnosisFailure()
    {
        static std::mt19937 generator(std::random_device{}());
        std::uniform_int_distribution<int> distribution(1, 100);
        return distribution(generator) <= 10;
    }

    int chooseCurseForChurchService(const Player& player, const std::string& title, const std::string& prompt, bool requireCurse)
    {
        const std::vector<PlayerCurse>& curses = player.getActiveCurses();
        if (curses.empty())
        {
            showShopResult(title, "shop.church.no_curse", {"Aucune trace active à analyser."});
            return -1;
        }

        MenuScreen screen(title, "shop.church.choose_curse");
        screen.addLine("Choisis la trace à étudier. Les traces inconnues restent volontairement affichées en ?????.");
        screen.addOption(0, "Retour", "Revenir aux services d'église.", true, "shop.church.choose_curse.back");
        for (std::size_t i = 0; i < curses.size(); ++i)
        {
            const PlayerCurse& curse = curses[i];
            screen.addOption(
                static_cast<int>(i + 1),
                curseKnownName(curse) + " | " + curseDiagnosisLevelText(curse),
                churchCurseDetailText(curse),
                true,
                "shop.church.choose_curse." + std::to_string(i)
            );
        }
        Console::clear();
        const int choice = TerminalInterface::askMenuChoiceFromOptions(screen, prompt);
        if (choice <= 0 || choice > static_cast<int>(curses.size()))
        {
            return -1;
        }
        (void)requireCurse;
        return choice - 1;
    }

    std::string chooseSymptomCategoryForChurchDiagnosis(const PlayerCurse& curse)
    {
        const std::vector<CurseSymptomCategory> categories = getChurchSymptomCategories();
        MenuScreen screen("CIBLER UN SYMPTÔME", "shop.church.symptom_category");
        screen.addLine("Choisis une catégorie volontairement vague. L'église ne te spoil pas un -20% dégâts ou un effet exact.");
        screen.addLine("Mauvaise piste : elle sera écartée, avec environ 20% d'autres mauvaises pistes pour ne pas rendre la recherche abusive.");
        screen.addOption(0, "Retour", "Ne rien diagnostiquer pour l'instant.", true, "shop.church.symptom.back");
        for (std::size_t i = 0; i < categories.size(); ++i)
        {
            const CurseSymptomCategory& category = categories[i];
            const bool alreadyConfirmed = churchTokenListContains(curse.discoveredSymptomCategories, category.id);
            const bool excluded = churchTokenListContains(curse.excludedSymptomCategories, category.id);
            std::string label = category.label;
            if (alreadyConfirmed) label += " (déjà confirmé)";
            if (excluded) label += " (piste écartée)";
            screen.addOption(
                static_cast<int>(i + 1),
                label,
                category.detail,
                !excluded && !alreadyConfirmed,
                "shop.church.symptom." + category.id
            );
        }
        Console::clear();
        const int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Quelle catégorie veux-tu faire tester ?");
        if (choice <= 0 || choice > static_cast<int>(categories.size()))
        {
            return "";
        }
        return categories[static_cast<std::size_t>(choice - 1)].id;
    }

    void addChurchDiagnosisLines(Player& player, std::vector<std::string>& lines)
    {
        std::vector<std::string> curseLines = player.describeActiveCurses();
        lines.push_back("État des traces :");
        lines.insert(lines.end(), curseLines.begin(), curseLines.end());
        if (player.getActiveCurseCount() == 0)
        {
            lines.push_back("Frère Calixte ajoute que les PNJ pourront eux aussi porter des traces plus tard, surtout après certains événements ou boss.");
        }
        else
        {
            lines.push_back("Règle de Maëlys : l'église refuse d'exorciser une trace tant qu'elle n'est pas connue au moins au niveau 1.");
        }
    }

    void runTargetedChurchDiagnosis(Player& player)
    {
        const int curseIndex = chooseCurseForChurchService(player, "DIAGNOSTIC CIBLÉ", "Quelle trace veux-tu étudier ?", true);
        if (curseIndex < 0) return;
        const PlayerCurse target = player.getActiveCurses()[static_cast<std::size_t>(curseIndex)];
        const std::string categoryId = chooseSymptomCategoryForChurchDiagnosis(target);
        if (categoryId.empty()) return;

        std::vector<std::string> lines;
        if (!payServiceWithVoucherOrGold(player, "sanctuary_candle", "Cierge de veille", 32, lines))
        {
            showShopResult("DIAGNOSTIC REFUSÉ", "shop.church.diagnosis.targeted.failed_cost", lines);
            return;
        }

        if (rollChurchDiagnosisFailure())
        {
            lines.push_back("Sœur Maëlys ferme les yeux, puis secoue la tête : le signe se brouille au dernier moment.");
            lines.push_back("Diagnostic échoué : rien n'est confirmé ni écarté. Cela arrive environ une fois sur dix.");
            showLocalServiceResult("DIAGNOSTIC ÉCHOUÉ", "shop.church.diagnosis.targeted.failed_roll", player, lines, 1);
            return;
        }

        if (player.revealCurseSymptomCategory(target.id, categoryId))
        {
            lines.push_back("Piste confirmée : la catégorie " + churchSymptomCategoryLabel(categoryId) + " réagit à la trace.");
            lines.push_back("Connaissance minimale atteinte : niveau 1/3 si elle ne l'était pas déjà.");
            lines.push_back("Maëlys reste prudente : elle sait où chercher, pas encore tout ce que la malédiction fait exactement.");
        }
        else
        {
            player.excludeCurseSymptomCategory(target.id, categoryId);
            const int removed = player.autoExcludeWrongCurseSymptomCategories(target.id, 20);
            lines.push_back("Piste saine : rien n'indique que la catégorie " + churchSymptomCategoryLabel(categoryId) + " soit touchée.");
            lines.push_back("Maëlys ajoute : \"Cela ne veut pas dire que tout va bien. Le problème peut être ailleurs.\"");
            lines.push_back("Aide au diagnostic : " + std::to_string(std::max(1, removed)) + " autre(s) mauvaise(s) piste(s) ont été écartée(s).");
        }
        showLocalServiceResult("DIAGNOSTIC CIBLÉ", "shop.church.diagnosis.targeted.result", player, lines, 1);
    }

    void revealFirstUnknownCategory(Player& player, const PlayerCurse& target, std::vector<std::string>& lines)
    {
        const std::vector<std::string> categories = splitChurchTokenList(target.symptomCategories);
        for (const std::string& category : categories)
        {
            if (churchTokenListContains(target.discoveredSymptomCategories, category))
            {
                continue;
            }
            if (player.revealCurseSymptomCategory(target.id, category))
            {
                lines.push_back("Symptôme vague confirmé : " + churchSymptomCategoryLabel(category) + ".");
                return;
            }
        }
    }

    void runGeneralChurchDiagnosis(Player& player)
    {
        const int curseIndex = chooseCurseForChurchService(player, "DIAGNOSTIC GÉNÉRAL", "Quelle trace veux-tu faire lire globalement ?", true);
        if (curseIndex < 0) return;
        const PlayerCurse target = player.getActiveCurses()[static_cast<std::size_t>(curseIndex)];
        std::vector<std::string> lines;
        if (!payServiceWithVoucherOrGold(player, "exorcism_incense", "Encens d'exorcisme", 86, lines))
        {
            showShopResult("DIAGNOSTIC REFUSÉ", "shop.church.diagnosis.general.failed_cost", lines);
            return;
        }
        if (rollChurchDiagnosisFailure())
        {
            lines.push_back("L'encens tourne dans le mauvais sens. Père Orwan refuse de conclure sur une lecture sale.");
            lines.push_back("Diagnostic échoué : aucun niveau gagné. Chance d'échec : 10%.");
            showLocalServiceResult("DIAGNOSTIC GÉNÉRAL ÉCHOUÉ", "shop.church.diagnosis.general.failed_roll", player, lines, 1);
            return;
        }

        player.setCurseDiagnosisLevel(target.id, 1);
        revealFirstUnknownCategory(player, target, lines);
        lines.push_back("Diagnostic général : la trace est maintenant connue au niveau 1/3.");
        lines.push_back("Ce niveau suffit pour autoriser un exorcisme si la malédiction est retirable par l'église, mais il ne révèle pas encore toute l'origine ni les détails.");
        showLocalServiceResult("DIAGNOSTIC GÉNÉRAL", "shop.church.diagnosis.general.success", player, lines, 1);
    }

    void runDeepChurchDiagnosis(Player& player)
    {
        const int curseIndex = chooseCurseForChurchService(player, "DIAGNOSTIC APPROFONDI", "Quelle trace veux-tu approfondir ?", true);
        if (curseIndex < 0) return;
        const PlayerCurse target = player.getActiveCurses()[static_cast<std::size_t>(curseIndex)];
        std::vector<std::string> lines;
        if (target.diagnosisLevel < 1)
        {
            showShopResult("RECHERCHE REFUSÉE", "shop.church.diagnosis.deep.locked", {"Maëlys refuse : il faut d'abord connaître la trace au niveau 1."});
            return;
        }
        if (!payServiceWithVoucherOrGold(player, "exorcism_incense", "Encens d'exorcisme", 128, lines))
        {
            showShopResult("DIAGNOSTIC REFUSÉ", "shop.church.diagnosis.deep.failed_cost", lines);
            return;
        }
        if (rollChurchDiagnosisFailure())
        {
            lines.push_back("Le cercle tient, puis se coupe net. Frère Calixte note : lecture instable, aucun résultat fiable.");
            lines.push_back("Diagnostic échoué : aucun niveau gagné. Chance d'échec : 10%.");
            showLocalServiceResult("DIAGNOSTIC APPROFONDI ÉCHOUÉ", "shop.church.diagnosis.deep.failed_roll", player, lines, 1);
            return;
        }

        player.setCurseDiagnosisLevel(target.id, 2);
        revealFirstUnknownCategory(player, target, lines);
        lines.push_back("Diagnostic approfondi : niveau 2/3 atteint.");
        lines.push_back("Origine mieux cernée : " + (target.origin.empty() ? "source encore floue" : target.origin) + ".");
        lines.push_back("La durée devient lisible, mais les effets exacts et conditions complètes demandent encore une recherche totale.");
        showLocalServiceResult("DIAGNOSTIC APPROFONDI", "shop.church.diagnosis.deep.success", player, lines, 1);
    }

    void runTotalChurchDiagnosis(Player& player)
    {
        const int curseIndex = chooseCurseForChurchService(player, "DIAGNOSTIC TOTAL", "Quelle trace veux-tu comprendre totalement ?", true);
        if (curseIndex < 0) return;
        const PlayerCurse target = player.getActiveCurses()[static_cast<std::size_t>(curseIndex)];
        std::vector<std::string> lines;
        if (target.diagnosisLevel < 2)
        {
            showShopResult("RECHERCHE REFUSÉE", "shop.church.diagnosis.total.locked", {"Père Orwan refuse : il faut d'abord atteindre le diagnostic approfondi niveau 2."});
            return;
        }
        if (!payServiceWithVoucherOrGold(player, "white_bone_chalk", "Craie d'os blanc", 185, lines))
        {
            showShopResult("DIAGNOSTIC REFUSÉ", "shop.church.diagnosis.total.failed_cost", lines);
            return;
        }
        if (rollChurchDiagnosisFailure())
        {
            lines.push_back("La craie se fend avant la fin du cercle. Maëlys efface tout : mieux vaut rater que mentir.");
            lines.push_back("Diagnostic échoué : aucun niveau gagné. Chance d'échec : 10%.");
            showLocalServiceResult("DIAGNOSTIC TOTAL ÉCHOUÉ", "shop.church.diagnosis.total.failed_roll", player, lines, 1);
            return;
        }

        player.setCurseDiagnosisLevel(target.id, 3);
        const std::vector<std::string> categories = splitChurchTokenList(target.symptomCategories);
        for (const std::string& category : categories)
        {
            player.revealCurseSymptomCategory(target.id, category);
        }
        lines.push_back("Diagnostic total : niveau 3/3 atteint.");
        lines.push_back("Lecture complète : " + (target.description.empty() ? "aucun détail exact n'est lisible pour l'instant." : target.description));
        if (!target.removalHint.empty())
        {
            lines.push_back("Condition de retrait : " + target.removalHint);
        }
        showLocalServiceResult("DIAGNOSTIC TOTAL", "shop.church.diagnosis.total.success", player, lines, 1);
    }

    struct ChurchCaseOption
    {
        int menuChoice = 0;
        int type = 0;
        std::string label;
        std::string detail;
        std::string sceneIntro;
        std::string investigationLine;
        std::string expectedClueLine;
        std::string screenId;
        std::string requiredProofId;
        std::string requiredProofName;
        std::string rewardProofId;
        std::string rewardProofName;
    };

    bool playerHasMaterial(const Player& player, const std::string& materialId, int quantity = 1)
    {
        return materialId.empty() || player.getInventory().countMaterialById(materialId) >= std::max(1, quantity);
    }

    std::string churchMaterialStatusLine(const Player& player, const std::string& materialId, const std::string& label, int quantity = 1)
    {
        if (materialId.empty())
        {
            return "Aucune preuve préalable demandée.";
        }
        return label + " : " + std::to_string(player.getInventory().countMaterialById(materialId)) + "/" + std::to_string(std::max(1, quantity));
    }

    std::vector<ChurchCaseOption> getChurchCaseOptions(const Player& player)
    {
        (void)player;
        return {
            {
                1,
                1,
                "Lysa parle en dormant — veiller avec sa mère",
                "Sœur Maëlys demande une première veillée discrète : on écoute avant de conclure.",
                "Mira, la mère de Lysa, jure que sa fille prononce des phrases qu'elle n'a jamais apprises.",
                "Tu dois apporter de l'encens pour que Maëlys reste dans la chambre sans effrayer l'enfant.",
                "Indice attendu : les mots exacts entendus pendant la nuit.",
                "shop.church.requests.sleep.start",
                "exorcism_incense",
                "Encens d'exorcisme",
                "church_night_testimony",
                "Témoignage nocturne signé"
            },
            {
                2,
                2,
                "Lysa parle en dormant — lire la chambre",
                "La chambre doit être examinée avant de viser la personne. Le patient n'est pas forcément la source.",
                "Maëlys relit le témoignage de Mira et refuse de poser la main sur Lysa sans preuve plus sûre.",
                "La veillée a donné assez de détails pour inspecter le lit, les murs et les objets proches.",
                "Indice attendu : savoir si la trace vient de la chambre ou de Lysa elle-même.",
                "shop.church.requests.sleep.diagnosis",
                "church_night_testimony",
                "Témoignage nocturne signé",
                "church_sleep_diagnosis",
                "Diagnostic de chambre endormie"
            },
            {
                3,
                3,
                "Lysa parle en dormant — trouver l'objet accroché",
                "La lecture pointe vers un objet banal. Il faut l'isoler sans accuser la famille.",
                "Père Orwan prépare une fiole et demande de rester calme : détruire le mauvais objet empirerait tout.",
                "Le diagnostic de chambre donne une zone, pas encore le coupable exact.",
                "Indice attendu : l'objet qui porte réellement la trace.",
                "shop.church.requests.sleep.resolve",
                "church_sleep_diagnosis",
                "Diagnostic de chambre endormie",
                "identified_source_object",
                "Objet source identifié"
            },
            {
                4,
                4,
                "Ronan s'étouffe quand il ment — nommer le serment",
                "Un charretier panique dès qu'il parle d'une promesse ancienne. Il faut d'abord retrouver les mots exacts.",
                "Ronan répète qu'il n'a rien promis, mais son souffle se coupe toujours au même moment.",
                "Père Orwan demande un sceau pour recueillir sa parole sans qu'elle soit contestée plus tard.",
                "Indice attendu : le nom du serment ou de la personne liée.",
                "shop.church.requests.oath.named",
                "sanctuary_wax_seal",
                "Sceau de cire sanctuaire",
                "named_oath_testimony",
                "Témoignage de serment nommé"
            },
            {
                5,
                5,
                "Le dortoir fait le même rêve — comparer les récits",
                "Plusieurs apprentis décrivent un rêve identique. Calixte veut comparer les détails sans choisir un coupable.",
                "Ivo, Nelle et Sali dessinent tous la même porte, mais aucun ne se souvient l'avoir vue éveillé.",
                "Une note de bénédiction sert de prétexte propre pour recueillir les récits sans affoler le dortoir.",
                "Indice attendu : le motif commun du rêve.",
                "shop.church.requests.dream.pattern",
                "blessing_note",
                "Note de bénédiction",
                "shared_dream_pattern",
                "Motif de rêve partagé"
            },
            {
                6,
                6,
                "Le dortoir fait le même rêve — chercher la contre-légende",
                "Le motif existe peut-être déjà dans les archives. Une histoire ancienne a souvent une fin ancienne.",
                "Frère Calixte reconnaît une porte dessinée dans un vieux conte, mais il lui manque la version qui la referme.",
                "Le motif partagé sert de clé de recherche, pas de preuve définitive.",
                "Indice attendu : une version de la légende qui sait comment se terminer.",
                "shop.church.requests.dream.legend",
                "shared_dream_pattern",
                "Motif de rêve partagé",
                "copied_counter_legend",
                "Contre-légende copiée"
            },
            {
                7,
                8,
                "Elian regrette un pacte — nommer la contrepartie",
                "Un aventurier a accepté une aide trop facile. L'église ne rompt rien tant que le prix exact n'est pas avoué.",
                "Elian affirme qu'il a seulement 'accepté un coup de chance'. Maëlys entend surtout une dette qui respire derrière ses mots.",
                "La craie d'os sert à tracer une limite pendant qu'il raconte ce qu'il a promis, sans que la promesse lui ferme la bouche.",
                "Indice attendu : la contrepartie exacte du pacte volontaire.",
                "shop.church.requests.pact.witness",
                "white_bone_chalk",
                "Craie d'os blanc",
                "pact_break_witness",
                "Témoin de pacte rompu"
            },
            {
                8,
                7,
                "Le vieux seuil répond — dessiner ce qu'il faut sceller",
                "Un seuil ou un puits réagit aux passages. L'église veut un croquis précis avant de parler de scellement.",
                "Père Orwan insiste : sceller un lieu sans le comprendre, c'est juste enfermer la peur avec la prochaine victime.",
                "Une note d'exorciste permet de préparer le cercle, mais pas de deviner le lieu à la place du joueur.",
                "Indice attendu : l'emplacement exact de la source scellable.",
                "shop.church.requests.source.sketch",
                "exorcist_note",
                "Note d'exorciste",
                "sealable_source_sketch",
                "Croquis de source scellable"
            }
        };
    }

    Quest makeChurchNpcCurseQuest(Player& player, int type)
    {
        Quest quest;
        quest.origin = "Église et exorcisme";
        quest.location = "Église et salle des prières";
        quest.accepted = true;
        quest.availableFromDay = player.getWorldDaysElapsed();
        quest.expiresAtDay = player.getWorldDaysElapsed() + 6;
        quest.target = 1;
        quest.progress = 0;
        quest.guildQuest = false;
        quest.objectiveType = "material";
        quest.targetFamily = "Cas d'église";
        quest.requiredMaterialQuantity = 1;
        quest.rewardMaterialQuantity = 1;

        if (type == 1)
        {
            quest.id = "church_sleep_watch_" + std::to_string(player.getWorldDaysElapsed()) + "_" + std::to_string(player.getLevel());
            quest.rank = player.getLevel() >= 25 ? "C" : "D";
            quest.title = "Lysa parle en dormant — veillée";
            quest.client = "Sœur Maëlys l'exorciste";
            quest.objective = "Apporter un encens d'exorcisme et accompagner Mira pendant la veillée. Il faut noter les mots de Lysa, pas décider déjà ce qu'elle a.";
            quest.requiredMaterialId = "exorcism_incense";
            quest.requiredMaterialName = "Encens d'exorcisme";
            quest.rewardExperience = 18 + player.getLevel() * 2;
            quest.rewardGold = 48 + player.getLevel() * 3;
            quest.rewardMaterialId = "church_night_testimony";
            quest.rewardMaterialName = "Témoignage nocturne signé";
            quest.rewardNote = "Mira a enfin raconté ce qu'elle entend. Le cas est crédible, mais la cause reste inconnue.";
            return quest;
        }

        if (type == 2)
        {
            quest.id = "church_sleep_room_reading_" + std::to_string(player.getWorldDaysElapsed()) + "_" + std::to_string(player.getLevel());
            quest.rank = player.getLevel() >= 28 ? "C" : "D";
            quest.title = "Lysa parle en dormant — chambre";
            quest.client = "Sœur Maëlys l'exorciste";
            quest.objective = "Ramener les mots entendus pendant la veillée pour lire la chambre, les objets et les traces de passage avant de viser l'enfant.";
            quest.requiredMaterialId = "church_night_testimony";
            quest.requiredMaterialName = "Témoignage nocturne signé";
            quest.rewardExperience = 22 + player.getLevel() * 2;
            quest.rewardGold = 58 + player.getLevel() * 3;
            quest.rewardMaterialId = "church_sleep_diagnosis";
            quest.rewardMaterialName = "Diagnostic de chambre endormie";
            quest.rewardNote = "La lecture vise le lieu : Maëlys comprend mieux où chercher sans accuser Lysa.";
            return quest;
        }

        if (type == 3)
        {
            quest.id = "church_sleep_source_" + std::to_string(player.getWorldDaysElapsed()) + "_" + std::to_string(player.getLevel());
            quest.rank = player.getLevel() >= 32 ? "C" : "D";
            quest.title = "Lysa parle en dormant — objet accroché";
            quest.client = "Sœur Maëlys l'exorciste";
            quest.objective = "Utiliser le diagnostic de chambre pour isoler l'objet banal qui accroche le sommeil de Lysa. La famille doit rester protégée de la panique.";
            quest.requiredMaterialId = "church_sleep_diagnosis";
            quest.requiredMaterialName = "Diagnostic de chambre endormie";
            quest.requiredMaterialQuantity = 1;
            quest.rewardExperience = 28 + player.getLevel() * 2;
            quest.rewardGold = 70 + player.getLevel() * 3;
            quest.rewardMaterialId = "identified_source_object";
            quest.rewardMaterialName = "Objet source identifié";
            quest.rewardNote = "Le patient n'est pas le problème : l'objet source peut maintenant être traité par une solution spéciale.";
            return quest;
        }

        if (type == 4)
        {
            quest.id = "church_oath_named_" + std::to_string(player.getWorldDaysElapsed()) + "_" + std::to_string(player.getLevel());
            quest.rank = player.getLevel() >= 35 ? "B" : "C";
            quest.title = "Ronan s'étouffe quand il ment — serment nommé";
            quest.client = "Père Orwan";
            quest.objective = "Fournir un sceau sanctuaire pour écouter Ronan sans le forcer. Le but est de retrouver le nom de la promesse qui serre sa gorge.";
            quest.requiredMaterialId = "sanctuary_wax_seal";
            quest.requiredMaterialName = "Sceau de cire sanctuaire";
            quest.rewardExperience = 30 + player.getLevel() * 2;
            quest.rewardGold = 78 + player.getLevel() * 4;
            quest.rewardMaterialId = "named_oath_testimony";
            quest.rewardMaterialName = "Témoignage de serment nommé";
            quest.rewardNote = "Le serment a un nom. Une malédiction de parole totalement diagnostiquée pourra être brisée avec cette preuve.";
            return quest;
        }

        if (type == 5)
        {
            quest.id = "church_dream_pattern_" + std::to_string(player.getWorldDaysElapsed()) + "_" + std::to_string(player.getLevel());
            quest.rank = "C";
            quest.title = "Le dortoir fait le même rêve — motif";
            quest.client = "Frère Calixte";
            quest.objective = "Apporter une note de bénédiction et comparer les récits d'Ivo, Nelle et Sali sans décider trop vite qu'un seul patient est fautif.";
            quest.requiredMaterialId = "blessing_note";
            quest.requiredMaterialName = "Note de bénédiction";
            quest.rewardExperience = 24 + player.getLevel() * 2;
            quest.rewardGold = 64 + player.getLevel() * 3;
            quest.rewardMaterialId = "shared_dream_pattern";
            quest.rewardMaterialName = "Motif de rêve partagé";
            quest.rewardNote = "Les trois récits partagent le même motif. Il peut maintenant guider une recherche de contre-légende.";
            return quest;
        }

        if (type == 6)
        {
            quest.id = "church_dream_counter_legend_" + std::to_string(player.getWorldDaysElapsed()) + "_" + std::to_string(player.getLevel());
            quest.rank = player.getLevel() >= 30 ? "C" : "D";
            quest.title = "Le dortoir fait le même rêve — contre-légende";
            quest.client = "Frère Calixte";
            quest.objective = "Ramener le motif de rêve partagé aux archives pour recopier la version de l'histoire qui sait comment se terminer.";
            quest.requiredMaterialId = "shared_dream_pattern";
            quest.requiredMaterialName = "Motif de rêve partagé";
            quest.rewardExperience = 26 + player.getLevel() * 2;
            quest.rewardGold = 58 + player.getLevel() * 3;
            quest.rewardMaterialId = "copied_counter_legend";
            quest.rewardMaterialName = "Contre-légende copiée";
            quest.rewardNote = "La contre-légende n'est pas un sort gratuit : elle donne une condition concrète pour refermer une histoire maudite.";
            return quest;
        }

        if (type == 8)
        {
            quest.id = "church_pact_witness_" + std::to_string(player.getWorldDaysElapsed()) + "_" + std::to_string(player.getLevel());
            quest.rank = player.getLevel() >= 34 ? "B" : "C";
            quest.title = "Elian regrette un pacte — contrepartie";
            quest.client = "Sœur Maëlys l'exorciste";
            quest.objective = "Apporter de la craie d'os blanc et écouter Elian jusqu'à ce que la contrepartie du pacte soit nommée. Sans le prix exact, l'église refuse de rompre au hasard.";
            quest.requiredMaterialId = "white_bone_chalk";
            quest.requiredMaterialName = "Craie d'os blanc";
            quest.rewardExperience = 30 + player.getLevel() * 2;
            quest.rewardGold = 76 + player.getLevel() * 4;
            quest.rewardMaterialId = "pact_break_witness";
            quest.rewardMaterialName = "Témoin de pacte rompu";
            quest.rewardNote = "La contrepartie a été nommée. Une marque de pacte totalement diagnostiquée pourra être rompue sans viser la mauvaise faute.";
            return quest;
        }

        quest.id = "church_sealable_source_" + std::to_string(player.getWorldDaysElapsed()) + "_" + std::to_string(player.getLevel());
        quest.rank = player.getLevel() >= 40 ? "B" : "C";
        quest.title = "Le vieux seuil répond — croquis scellable";
        quest.client = "Père Orwan";
        quest.objective = "Apporter une note d'exorciste pour préparer un croquis précis du seuil. Il servira seulement si la source est déjà totalement comprise.";
        quest.requiredMaterialId = "exorcist_note";
        quest.requiredMaterialName = "Note d'exorciste";
        quest.rewardExperience = 32 + player.getLevel() * 2;
        quest.rewardGold = 88 + player.getLevel() * 4;
        quest.rewardMaterialId = "sealable_source_sketch";
        quest.rewardMaterialName = "Croquis de source scellable";
        quest.rewardNote = "Le croquis ne bat pas un boss. Il donne une condition concrète pour sceller une trace de seuil diagnostiquée à fond.";
        return quest;
    }

    void openChurchTroubledPrayerRequests(Player& player)
    {
        const std::vector<ChurchCaseOption> options = getChurchCaseOptions(player);
        MenuScreen screen("PRIÈRES ET DEMANDES INQUIÉTANTES", "shop.church.troubled_requests");
        screen.addLine("Frère Calixte lit les billets déposés près des cierges : ici, on avance par preuves, pas par fiches froides.");
        screen.addLine("Chaque demande est une petite scène : écouter une personne, chercher un indice, puis revenir avec quelque chose de concret.");
        screen.addLine("Les objets affichés servent juste de trace de suivi dans l'inventaire ; en jeu, ce sont des indices ou des notes de terrain.");
        screen.addOption(0, "Retour", "Revenir aux services d'église.", true, "shop.church.requests.back");
        for (const ChurchCaseOption& option : options)
        {
            const bool available = playerHasMaterial(player, option.requiredProofId);
            std::string detail = option.detail;
            detail += " | Besoin actuel : " + churchMaterialStatusLine(player, option.requiredProofId, option.requiredProofName) + ".";
            detail += " | Indice à obtenir : " + option.expectedClueLine;
            screen.addOption(option.menuChoice, option.label, detail, available, option.screenId);
        }

        Console::clear();
        const int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choisis une demande à prendre au sérieux.");
        if (choice <= 0)
        {
            return;
        }

        auto it = std::find_if(options.begin(), options.end(), [choice](const ChurchCaseOption& option) {
            return option.menuChoice == choice;
        });
        if (it == options.end())
        {
            return;
        }

        Quest quest = makeChurchNpcCurseQuest(player, it->type);
        std::vector<std::string> lines;
        if (!playerHasMaterial(player, it->requiredProofId))
        {
            lines.push_back("Pré-requis manquant : " + it->requiredProofName + ".");
            lines.push_back("L'église refuse de sauter une étape : sinon on invente un diagnostic au lieu d'aider quelqu'un.");
            showShopResult("DEMANDE REFUSÉE", "shop.church.requests.missing_proof", lines);
            return;
        }

        if (!player.getQuestLog().canAcceptPersonalQuestForClient(quest.client))
        {
            lines.push_back(quest.client + " t'a déjà confié assez d'urgences pour l'instant.");
            lines.push_back("Termine ou abandonne une autre demande avant de prendre un nouveau cas d'église.");
            showShopResult("DEMANDE REFUSÉE", "shop.church.requests.blocked", lines);
            return;
        }

        if (player.getQuestLog().addQuest(quest))
        {
            player.getQuestLog().refreshMaterialDeliveryQuests(player.getInventory());
            lines.push_back("Demande acceptée : " + quest.title + ".");
            lines.push_back("Référent : " + quest.client + ".");
            lines.push_back("Scène : " + it->sceneIntro);
            lines.push_back("Ce que tu vas faire : " + it->investigationLine);
            lines.push_back(it->expectedClueLine);
            lines.push_back("Objectif journal : " + quest.objective);
            lines.push_back("Indice conservé ensuite : " + quest.rewardMaterialName + ".");
            lines.push_back("Important : tu aides une personne, pas une case à cocher. L'objet de suivi existe pour ne pas perdre le fil de l'enquête.");
            showShopResult("DEMANDE ACCEPTÉE", "shop.church.requests.accepted", lines);
        }
        else
        {
            lines.push_back("Le journal refuse cette demande : elle est peut-être déjà active ou incompatible avec tes demandes actuelles.");
            showShopResult("DEMANDE NON AJOUTÉE", "shop.church.requests.failed", lines);
        }
    }

    void showCurseLegendArchive()
    {
        MenuScreen screen("LÉGENDES DE MALÉDICTION", "shop.church.curse_legends");
        screen.addLine("La bibliothèque ne donne pas de faiblesse gratuite, mais elle peut expliquer les familles de solutions spéciales.");
        screen.addOption(0, "Retour", "Revenir aux services d'église.", true, "shop.church.legends.back");
        screen.addOption(1, "Lire : Les trois manières de rompre une trace", "Objet source, serment, contre-légende.", true, "shop.church.legends.counter_rites");
        screen.addOption(2, "Lire : Le patient n'est pas la malédiction", "Cas humains, symptômes flous et prudence d'exorciste.", true, "shop.church.legends.cursed_patients");

        Console::clear();
        const int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choisis une lecture.");
        if (choice == 1)
        {
            LegendTriggerSystem::displayArchiveEntry("curse_counter_rites");
        }
        else if (choice == 2)
        {
            LegendTriggerSystem::displayArchiveEntry("curse_cursed_patients");
        }
    }

    struct SpecialCurseProof
    {
        std::string solutionId;
        std::string materialId;
        std::string materialName;
    };

    SpecialCurseProof getSpecialCurseProof(const std::string& solutionId)
    {
        if (solutionId == "destroy_source_object") return {solutionId, "identified_source_object", "Objet source identifié"};
        if (solutionId == "break_oath") return {solutionId, "named_oath_testimony", "Témoignage de serment nommé"};
        if (solutionId == "read_counter_legend") return {solutionId, "copied_counter_legend", "Contre-légende copiée"};
        if (solutionId == "seal_source") return {solutionId, "sealable_source_sketch", "Croquis de source scellable"};
        if (solutionId == "break_pact") return {solutionId, "pact_break_witness", "Témoin de pacte rompu"};
        if (solutionId == "confirm_source_defeated") return {solutionId, "source_defeat_notice", "Note de source vaincue"};
        return {solutionId, "", ""};
    }

    SpecialCurseProof getSpecialCurseOutcome(const std::string& solutionId)
    {
        if (solutionId == "destroy_source_object") return {solutionId, "purified_source_ashes", "Cendres de source purifiée"};
        if (solutionId == "break_oath") return {solutionId, "broken_oath_record", "Acte de serment brisé"};
        if (solutionId == "read_counter_legend") return {solutionId, "closed_counter_legend", "Contre-légende refermée"};
        if (solutionId == "seal_source") return {solutionId, "sealed_source_mark", "Marque de source scellée"};
        if (solutionId == "break_pact") return {solutionId, "released_pact_record", "Acte de pacte libéré"};
        if (solutionId == "confirm_source_defeated") return {solutionId, "confirmed_source_silence", "Silence de source confirmé"};
        return {solutionId, "", ""};
    }

    bool hasSpecialCurseProof(const Player& player, const std::string& solutionId)
    {
        const SpecialCurseProof proof = getSpecialCurseProof(solutionId);
        return proof.materialId.empty() || player.getInventory().countMaterialById(proof.materialId) > 0;
    }

    std::string specialCurseProofLine(const Player& player, const std::string& solutionId)
    {
        const SpecialCurseProof proof = getSpecialCurseProof(solutionId);
        if (proof.materialId.empty()) return "Preuve : aucune.";
        return "Preuve suivie : " + proof.materialName + " " + std::to_string(player.getInventory().countMaterialById(proof.materialId)) + "/1.";
    }

    void runSpecialCurseSolution(Player& player)
    {
        MenuScreen screen("SOLUTIONS SPÉCIALES", "shop.church.special_solution");
        screen.addLine("Ces actions demandent deux choses : comprendre la malédiction au niveau 3/3, puis posséder l'indice concret obtenu en enquête.");
        screen.addLine("Ici, Maëlys ne soigne pas au hasard : elle vérifie la source, le serment, la légende ou le seuil avant d'agir.");
        screen.addOption(0, "Retour", "Revenir aux services d'église.", true, "shop.church.special.back");
        screen.addOption(1, "Détruire l'objet source", "Scène : isoler l'objet responsable, puis le briser dans un cercle sûr. " + specialCurseProofLine(player, "destroy_source_object"), player.hasCurseEligibleForSpecialSolution("destroy_source_object") && hasSpecialCurseProof(player, "destroy_source_object"), "shop.church.special.destroy_object");
        screen.addOption(2, "Briser un serment", "Scène : faire perdre son droit à une promesse qui serre encore. " + specialCurseProofLine(player, "break_oath"), player.hasCurseEligibleForSpecialSolution("break_oath") && hasSpecialCurseProof(player, "break_oath"), "shop.church.special.break_oath");
        screen.addOption(3, "Lire une contre-légende", "Scène : lire la version de l'histoire où la trace accepte de finir. " + specialCurseProofLine(player, "read_counter_legend"), player.hasCurseEligibleForSpecialSolution("read_counter_legend") && hasSpecialCurseProof(player, "read_counter_legend"), "shop.church.special.counter_legend");
        screen.addOption(4, "Sceller une source", "Scène : refermer un passage déjà compris, sans prétendre vaincre ce qui vit derrière. " + specialCurseProofLine(player, "seal_source"), player.hasCurseEligibleForSpecialSolution("seal_source") && hasSpecialCurseProof(player, "seal_source"), "shop.church.special.seal_source");
        screen.addOption(5, "Rompre un pacte volontaire", "Scène : faire tomber la contrepartie nommée, pas annuler gratuitement un choix. " + specialCurseProofLine(player, "break_pact"), player.hasCurseEligibleForSpecialSolution("break_pact") && hasSpecialCurseProof(player, "break_pact"), "shop.church.special.break_pact");
        screen.addOption(6, "Confirmer une source vaincue", "Scène : retirer la trace d'un seuil après une vraie victoire contre sa source. " + specialCurseProofLine(player, "confirm_source_defeated"), player.hasCurseEligibleForSpecialSolution("confirm_source_defeated") && hasSpecialCurseProof(player, "confirm_source_defeated"), "shop.church.special.source_defeated");

        Console::clear();
        const int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choisis une solution spéciale.");
        if (choice <= 0 || choice > 6)
        {
            return;
        }

        std::string solutionId;
        std::string title;
        std::string voucherId;
        std::string voucherName;
        int cost = 0;
        std::vector<std::string> lines;

        if (choice == 1)
        {
            solutionId = "destroy_source_object";
            title = "OBJET SOURCE DÉTRUIT";
            voucherId = "sanctuary_wax_seal";
            voucherName = "Sceau de cire sanctuaire";
            cost = 120;
            lines.push_back("Sœur Maëlys ne frappe pas la trace : elle isole ce à quoi elle était accrochée.");
        }
        else if (choice == 2)
        {
            solutionId = "break_oath";
            title = "SERMENT BRISÉ";
            voucherId = "exorcist_note";
            voucherName = "Note d'exorciste";
            cost = 140;
            lines.push_back("Père Orwan demande une vérité, pas une incantation : le serment doit perdre son droit de serrer.");
        }
        else if (choice == 3)
        {
            solutionId = "read_counter_legend";
            title = "CONTRE-LÉGENDE LUE";
            voucherId = "exorcist_note";
            voucherName = "Note d'exorciste";
            cost = 95;
            lines.push_back("Frère Calixte ouvre la version contraire de l'histoire : celle qui explique comment elle se termine.");
        }
        else if (choice == 4)
        {
            solutionId = "seal_source";
            title = "SOURCE SCELLÉE";
            voucherId = "sanctuary_wax_seal";
            voucherName = "Sceau de cire sanctuaire";
            cost = 160;
            lines.push_back("L'église ne prétend pas vaincre le boss à ta place : elle ferme seulement la trace de seuil déjà comprise.");
        }
        else if (choice == 5)
        {
            solutionId = "break_pact";
            title = "PACTE ROMPU";
            voucherId = "white_bone_chalk";
            voucherName = "Craie d'os blanc";
            cost = 145;
            lines.push_back("Sœur Maëlys ne juge pas le choix : elle force seulement le pacte à nommer son vrai prix.");
        }
        else
        {
            solutionId = "confirm_source_defeated";
            title = "SOURCE VAINCUE CONFIRMÉE";
            voucherId = "exorcist_note";
            voucherName = "Note d'exorciste";
            cost = 110;
            lines.push_back("Père Orwan vérifie la victoire : la source a été affrontée, la trace n'a plus le droit de prétendre qu'elle attend encore.");
        }

        if (!player.hasCurseEligibleForSpecialSolution(solutionId))
        {
            lines.push_back("Aucune trace totalement diagnostiquée ne correspond encore à cette solution.");
            lines.push_back("Il faut d'abord atteindre le diagnostic total 3/3 sur la malédiction concernée.");
            showShopResult("SOLUTION SANS CIBLE", "shop.church.special.no_eligible_curse", lines);
            return;
        }

        const SpecialCurseProof proof = getSpecialCurseProof(solutionId);
        if (!proof.materialId.empty() && player.getInventory().countMaterialById(proof.materialId) <= 0)
        {
            lines.push_back("Preuve manquante : " + proof.materialName + ".");
            lines.push_back("Père Orwan refuse : sans preuve concrète, on ferait juste semblant d'avoir compris la condition spéciale.");
            showShopResult("SOLUTION REFUSÉE", "shop.church.special.missing_proof", lines);
            return;
        }

        if (!payServiceWithVoucherOrGold(player, voucherId, voucherName, cost, lines))
        {
            showShopResult("SOLUTION REFUSÉE", "shop.church.special.failed_cost", lines);
            return;
        }

        if (!proof.materialId.empty())
        {
            player.getInventory().removeMaterialQuantityById(proof.materialId, 1);
            lines.push_back("Preuve consommée : " + proof.materialName + ".");
        }

        const int removed = player.resolveSpecialCurseSolution(solutionId);
        if (removed <= 0)
        {
            lines.push_back("Aucune trace totalement diagnostiquée ne correspond à cette solution.");
            lines.push_back("Il faut atteindre le niveau 3/3 sur la malédiction concernée avant de tenter ce type d'acte.");
            showShopResult("SOLUTION SANS EFFET", "shop.church.special.no_target", lines);
            return;
        }

        lines.push_back("Trace(s) rompue(s) : " + std::to_string(removed) + ".");
        lines.push_back("La condition spéciale était la vraie clé : l'église n'a pas soigné au hasard, elle a retiré le point d'accroche.");
        const SpecialCurseProof outcome = getSpecialCurseOutcome(solutionId);
        if (!outcome.materialId.empty())
        {
            player.getInventory().addMaterial(MaterialCatalog::createById(outcome.materialId, 1));
            lines.push_back("Suite visible obtenue : " + outcome.materialName + " x1.");
        }
        lines.push_back("Note : les objets d'enquête consommés ne sont pas des papiers administratifs, mais les preuves qui permettaient d'agir sans inventer la réponse.");
        showLocalServiceResult(title, "shop.church.special.success", player, lines, 1);
    }


    void openChurchCaseDialogueScenes(Player& player)
    {
        const auto hasProof = [&player](const std::string& id) {
            return player.getInventory().countMaterialById(id) > 0;
        };

        const bool lysaSolved = hasProof("purified_source_ashes");
        const bool lysaObjectKnown = hasProof("identified_source_object") || lysaSolved;
        const bool lysaRoomKnown = hasProof("church_sleep_diagnosis") || lysaObjectKnown;
        const bool ronanSolved = hasProof("broken_oath_record");
        const bool ronanNamed = hasProof("named_oath_testimony") || ronanSolved;
        const bool dormitorySolved = hasProof("closed_counter_legend");
        const bool dormitoryPatternKnown = hasProof("shared_dream_pattern") || hasProof("copied_counter_legend") || dormitorySolved;
        const bool elianSolved = hasProof("released_pact_record");
        const bool elianNamed = hasProof("pact_break_witness") || elianSolved;
        const bool thresholdSolved = hasProof("sealed_source_mark") || hasProof("confirmed_source_silence");
        const bool thresholdSketched = hasProof("sealable_source_sketch") || thresholdSolved;

        MenuScreen screen("PARLER DES CAS D'ÉGLISE", "shop.church.case_dialogues");
        screen.addLine("Ces scènes ne sont pas un registre : ce sont des moments courts pour comprendre les personnes derrière les demandes.");
        screen.addLine("Les dialogues changent si tu as déjà trouvé une preuve ou résolu une trace liée au cas.");
        screen.addOption(0, "Retour", "Revenir aux services d'église.", true, "shop.church.case_dialogues.back");
        screen.addOption(1, "Mira et Lysa", lysaSolved ? "Suite : la chambre est apaisée." : (lysaRoomKnown ? "Suite : l'enquête avance dans la chambre." : "Une mère inquiète, une enfant qui parle avec des mots trop vieux."), true, "shop.church.case_dialogues.lysa");
        screen.addOption(2, "Ronan", ronanSolved ? "Suite : le serment ne serre plus sa gorge." : (ronanNamed ? "Suite : le serment a enfin un nom." : "Un charretier qui s'étouffe dès qu'un ancien serment remonte."), true, "shop.church.case_dialogues.ronan");
        screen.addOption(3, "Ivo, Nelle et Sali", dormitorySolved ? "Suite : la porte du rêve se referme." : (dormitoryPatternKnown ? "Suite : le motif du rêve a été compris." : "Trois apprentis, un même rêve, une porte dessinée sans l'avoir vue."), true, "shop.church.case_dialogues.dormitory");
        screen.addOption(4, "Elian", elianSolved ? "Suite : il peut appeler son choix par son nom." : (elianNamed ? "Suite : la contrepartie a été dite." : "Un aventurier qui appelle dette ce qu'il appelait chance."), true, "shop.church.case_dialogues.elian");
        screen.addOption(5, "Le vieux seuil", thresholdSolved ? "Suite : le lieu ne répond plus de la même façon." : (thresholdSketched ? "Suite : le bon point d'accroche est dessiné." : "Père Orwan explique pourquoi un lieu doit être compris avant d'être scellé."), true, "shop.church.case_dialogues.threshold");

        Console::clear();
        const int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "De quel cas veux-tu parler ?");
        if (choice <= 0) return;

        if (choice == 1)
        {
            std::vector<std::string> lines;
            if (lysaSolved)
            {
                lines = {
                    "Mira ne serre plus les mains comme avant. Elle les garde ouvertes, posées sur la couverture de Lysa.",
                    "Lysa dort vraiment cette fois. Pas profondément, pas miraculeusement, mais avec sa propre respiration.",
                    "Sœur Maëlys murmure : « Voilà pourquoi on a cherché l'objet avant de toucher l'enfant. »",
                    "Suite visible : le cas n'est plus une fiche d'enquête, c'est une famille qui respire mieux."
                };
            }
            else if (lysaObjectKnown)
            {
                lines = {
                    "Mira montre la petite boîte retrouvée sous le lit. Elle a l'air banale, et c'est justement ce qui la rend dangereuse.",
                    "Maëlys refuse de la briser dans la panique : « Une source identifiée doit être détruite dans un cercle sûr. »",
                    "Suite possible : utiliser la solution spéciale 'Détruire l'objet source'."
                };
            }
            else if (lysaRoomKnown)
            {
                lines = {
                    "La chambre de Lysa est devenue plus importante que Lysa elle-même dans l'enquête.",
                    "Mira hésite : « Donc... ce n'est pas ma fille qui est maudite ? »",
                    "Sœur Maëlys répond : « Pas forcément. Un patient peut seulement être l'endroit où la trace parle. »"
                };
            }
            else
            {
                lines = {
                    "Mira garde les mains jointes pour ne pas trembler : « Elle dort, mais ce n'est pas sa voix. Je connais ma fille. »",
                    "Sœur Maëlys répond doucement : « Alors on commencera par écouter la chambre. Pas par accuser l'enfant. »",
                    "Lysa, à moitié réveillée, demande seulement si quelqu'un a déplacé la petite boîte sous son lit.",
                    "Indice clair : le problème peut être accroché à un objet banal, pas forcément au patient."
                };
            }
            showShopResult("MIRA ET LYSA", "shop.church.case_dialogues.lysa.result", lines);
            return;
        }

        if (choice == 2)
        {
            std::vector<std::string> lines;
            if (ronanSolved)
            {
                lines = {
                    "Ronan parle plus bas qu'avant, mais il finit ses phrases.",
                    "Père Orwan ne sourit pas trop vite : « Un serment brisé laisse parfois de la honte, mais plus de corde. »",
                    "Ronan souffle enfin le nom qu'il évitait, sans s'étouffer.",
                    "Suite visible : le serment n'est plus accroché à sa gorge."
                };
            }
            else if (ronanNamed)
            {
                lines = {
                    "Ronan n'arrive plus à prétendre que rien n'existe. Le nom du serment est sur la table.",
                    "Père Orwan pose le témoignage devant lui : « Maintenant, on sait quoi briser. Pas avant. »",
                    "Suite possible : utiliser la solution spéciale 'Briser un serment'."
                };
            }
            else
            {
                lines = {
                    "Ronan rit trop fort : « Je n'ai rien promis à personne. »",
                    "Son rire se casse aussitôt, comme si une corde invisible serrait sa gorge.",
                    "Père Orwan pose un sceau sur la table : « Ne mens pas pour être courageux. Donne un nom à ce qui t'étrangle. »",
                    "Indice clair : un serment maudit demande une vérité précise, pas un exorcisme lancé au hasard."
                };
            }
            showShopResult("RONAN", "shop.church.case_dialogues.ronan.result", lines);
            return;
        }

        if (choice == 3)
        {
            std::vector<std::string> lines;
            if (dormitorySolved)
            {
                lines = {
                    "Ivo dessine encore la porte, mais cette fois il ajoute une serrure fermée.",
                    "Nelle rit nerveusement : « Dans mon rêve, la cloche sonnait plus loin. »",
                    "Frère Calixte range la contre-légende : « Une histoire refermée peut rester dans les archives. Pas dans les enfants. »",
                    "Suite visible : le dortoir n'est pas guéri par force, il a reçu la bonne fin de l'histoire."
                };
            }
            else if (dormitoryPatternKnown)
            {
                lines = {
                    "Les trois enfants décrivent maintenant la même porte sans se couper la parole.",
                    "Calixte tient le motif comme une clé de recherche : « On ne soigne pas un rêve partagé comme une fièvre. On cherche l'histoire qui l'a commencé. »",
                    "Suite possible : trouver ou lire une contre-légende."
                };
            }
            else
            {
                lines = {
                    "Ivo dessine une porte noire. Nelle ajoute une poignée en forme de lune. Sali complète sans réfléchir : « Et derrière, il y a une cloche. »",
                    "Frère Calixte pâlit : « Trois enfants n'inventent pas la même erreur avec les mêmes détails. »",
                    "Il demande de comparer les récits avant d'ouvrir les archives : une légende se referme avec sa contre-légende.",
                    "Indice clair : les cauchemars partagés doivent être compris comme une histoire, pas comme une maladie isolée."
                };
            }
            showShopResult("LE DORTOIR", "shop.church.case_dialogues.dormitory.result", lines);
            return;
        }

        if (choice == 4)
        {
            std::vector<std::string> lines;
            if (elianSolved)
            {
                lines = {
                    "Elian ne dit plus que c'était seulement de la chance.",
                    "Sœur Maëlys lui laisse le temps de répondre : « Tu as choisi. Tu as payé. Maintenant, tu peux repartir sans mentir à ton propre choix. »",
                    "Il garde la tête basse, mais ses pas ne résonnent plus comme une dette.",
                    "Suite visible : un pacte volontaire peut être rompu, mais pas effacé de l'histoire du personnage."
                };
            }
            else if (elianNamed)
            {
                lines = {
                    "Elian a enfin nommé la contrepartie. Ce n'était pas un mot héroïque, seulement un prix.",
                    "Maëlys trace une ligne de craie : « Maintenant que le pacte a un nom, il peut perdre sa prise. »",
                    "Suite possible : utiliser la solution spéciale 'Rompre un pacte volontaire'."
                };
            }
            else
            {
                lines = {
                    "Elian regarde ses bottes : « J'avais besoin d'un coup de chance. J'ai dit oui. C'est tout. »",
                    "Sœur Maëlys ne le juge pas : « Un pacte ne se rompt pas en niant qu'il a aidé. Il faut nommer ce qu'il réclame. »",
                    "La craie d'os blanc sert à garder la parole ouverte pendant que la dette tente de se refermer.",
                    "Indice clair : un pacte volontaire vient d'un choix du joueur ou d'un PNJ, donc la rupture demande d'assumer la contrepartie."
                };
            }
            showShopResult("ELIAN", "shop.church.case_dialogues.elian.result", lines);
            return;
        }

        if (choice == 5)
        {
            std::vector<std::string> lines;
            if (thresholdSolved)
            {
                lines = {
                    "Père Orwan ne déplie plus le plan : il le pose fermé sur la table.",
                    "« Le seuil existe encore. Un lieu ne disparaît pas parce qu'on le bénit. Mais il ne répond plus à ton passage. »",
                    "Frère Calixte ajoute une note discrète dans les archives, sans appeler ça une victoire.",
                    "Suite visible : le lieu est suivi comme un endroit scellé ou confirmé, pas comme une menace oubliée."
                };
            }
            else if (thresholdSketched)
            {
                lines = {
                    "Le croquis montre le bon point d'accroche : une fissure, pas toute la porte.",
                    "Père Orwan insiste : « On ne ferme pas un lieu entier quand une seule couture saigne. »",
                    "Suite possible : sceller la source ou confirmer qu'elle a été vaincue."
                };
            }
            else
            {
                lines = {
                    "Père Orwan déplie un vieux plan : « Sceller un seuil sans le dessiner, c'est fermer une porte en laissant la clé au monstre. »",
                    "Frère Calixte ajoute que certains lieux ne sont pas maléfiques : ils répètent seulement un passage resté ouvert trop longtemps.",
                    "Le croquis sert donc à viser le bon point d'accroche, pas à inventer une faiblesse gratuite.",
                    "Indice clair : les solutions de source demandent exploration, preuve et diagnostic total."
                };
            }
            showShopResult("LE VIEUX SEUIL", "shop.church.case_dialogues.threshold.result", lines);
        }
    }


    void runFullChurchExorcism(Player& player)
    {
        std::vector<std::string> lines;
        if (!player.getInventory().spendGold(180))
        {
            showShopResult("EXORCISME COMPLET REFUSÉ", "shop.church.full_exorcism.failed_cost", {"Paiement refusé : il faut " + Money::formatGoldWithRaw(180) + "."});
            return;
        }
        lines.push_back("Coût payé : " + Money::formatGoldWithRaw(180) + ".");
        int acted = 0;
        int removed = 0;
        const std::vector<PlayerCurse> curses = player.getActiveCurses();
        for (const PlayerCurse& curse : curses)
        {
            if (!curse.removableByChurch || curse.bossIdRequiredToBreak > 0 || curse.diagnosisLevel < 1)
            {
                continue;
            }
            ++acted;
            if (player.advanceChurchExorcism(curse.id) && !player.hasActiveCurse(curse.id))
            {
                ++removed;
            }
        }
        if (acted <= 0)
        {
            lines.push_back("Aucune malédiction ne peut être retirée : il faut au moins un niveau 1, et les verrous de boss ne cèdent pas ici.");
        }
        else
        {
            lines.push_back("Exorcisme complet : " + std::to_string(acted) + " trace(s) retirable(s) traitée(s).");
            lines.push_back("Retirées totalement pendant ce passage : " + std::to_string(removed) + ". Les rites longs peuvent demander de revenir.");
        }
        showLocalServiceResult("EXORCISME COMPLET", "shop.church.full_exorcism.result", player, lines, 2);
    }

    void openChurchServiceMenu(Player& player)
    {
        bool stay = true;

        while (stay)
        {
            MenuScreen screen("ÉGLISE ET EXORCISME", "shop.church.services");
            screen.addLine("Sœur Maëlys l'exorciste diagnostique les traces. Père Orwan bénit les routes. Frère Calixte écoute les prières laissées aux cierges.");
            screen.addLine("Horaires : matin, midi, après-midi, soir. Fermée la nuit.");
            screen.addLine("Règle : une malédiction inconnue reste affichée ????? et ne peut pas être exorcisée avant le niveau 1.");
            screen.addLine("Chaque diagnostic peut échouer : 10% de chance de lecture inutilisable.");
            screen.addLine("Temps : " + worldTimeLineForPlayer(player));
            screen.addLine("Argent : " + Money::formatCurrencyOverviewFromCopper(player.getInventory().getTotalCopper()));
            screen.addLine("Traces actives : " + std::to_string(player.getActiveCurseCount()) + ".");
            screen.addLine(serviceCostLine(player, "sanctuary_candle", "Cierge de veille", 32));
            screen.addLine(serviceCostLine(player, "exorcism_incense", "Encens d'exorcisme", 86));
            screen.addLine(serviceCostLine(player, "holy_water_vial", "Fiole d'eau bénite", 58));
            screen.addOption(0, "Retour", "Revenir au comptoir de l'église.", true, "shop.church.back");
            screen.addOption(1, "Voir le statut des malédictions", "Affiche ????? si la trace n'est pas encore diagnostiquée.", true, "shop.church.status");
            screen.addOption(2, "Diagnostic ciblé par symptôme", "Choisir une catégorie vague : santé, attaque, mana, précision, etc.", player.getActiveCurseCount() > 0, "shop.church.diagnosis.targeted");
            screen.addOption(3, "Diagnostic général", "Plus cher, mais révèle plus facilement une première piste globale.", player.getActiveCurseCount() > 0, "shop.church.diagnosis.general");
            screen.addOption(4, "Diagnostic approfondi", "Niveau 2/3 : demande une trace déjà connue niveau 1.", player.getActiveCurseCount() > 0, "shop.church.diagnosis.deep");
            screen.addOption(5, "Diagnostic total", "Niveau 3/3 : révèle la lecture complète si le niveau 2 est déjà atteint.", player.getActiveCurseCount() > 0, "shop.church.diagnosis.total");
            screen.addOption(6, "Exorcisme complet", "Tente un passage sur toutes les traces connues niveau 1 et retirable par l'église.", player.getActiveCurseCount() > 0, "shop.church.full_exorcism");
            screen.addOption(7, "Bénédiction de route", "Père Orwan prépare une note de bénédiction. Ne retire pas une vraie malédiction.", true, "shop.church.route_blessing");
            screen.addOption(8, "Rite d'apaisement court", "Rite léger : donne une preuve sacrée locale, consomme 1 segment.", true, "shop.church.blessing");
            screen.addOption(9, "Prières et demandes inquiétantes", "Prendre une demande d'église liée à un habitant, un serment ou un cauchemar partagé.", true, "shop.church.troubled_requests");
            screen.addOption(10, "Solutions spéciales", "Objet à détruire, serment à briser, contre-légende, source à sceller.", player.getActiveCurseCount() > 0, "shop.church.special_solution");
            screen.addOption(11, "Lire les légendes de malédiction", "Bibliothèque/archives : donne du contexte sans révéler de chiffres.", true, "shop.church.curse_legends");
            screen.addOption(12, "Parler des cas d'église", "Scènes courtes avec Mira/Lysa, Ronan, le dortoir, Elian ou le vieux seuil.", true, "shop.church.case_dialogues");

            const std::vector<PlayerCurse>& curses = player.getActiveCurses();
            for (std::size_t i = 0; i < curses.size(); ++i)
            {
                const PlayerCurse& curse = curses[i];
                const bool canExorciseHere = curse.removableByChurch && curse.bossIdRequiredToBreak <= 0 && curse.diagnosisLevel >= 1;
                screen.addOption(
                    static_cast<int>(20 + i),
                    "Exorciser : " + curseKnownName(curse),
                    churchCurseDetailText(curse),
                    canExorciseHere,
                    "shop.church.exorcise." + std::to_string(i)
                );
            }

            Console::clear();
            const int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choisis un service d'église, ou 0 pour revenir.");

            if (choice == 0)
            {
                stay = false;
                continue;
            }

            if (choice == 1)
            {
                std::vector<std::string> lines;
                addChurchDiagnosisLines(player, lines);
                showShopResult("STATUT DES MALÉDICTIONS", "shop.church.status", lines);
                continue;
            }
            if (choice == 2) { runTargetedChurchDiagnosis(player); continue; }
            if (choice == 3) { runGeneralChurchDiagnosis(player); continue; }
            if (choice == 4) { runDeepChurchDiagnosis(player); continue; }
            if (choice == 5) { runTotalChurchDiagnosis(player); continue; }
            if (choice == 6) { runFullChurchExorcism(player); continue; }
            if (choice == 9) { openChurchTroubledPrayerRequests(player); continue; }
            if (choice == 10) { runSpecialCurseSolution(player); continue; }
            if (choice == 11) { showCurseLegendArchive(); continue; }
            if (choice == 12) { openChurchCaseDialogueScenes(player); continue; }

            if (choice == 7)
            {
                std::vector<std::string> lines;
                if (!payServiceWithVoucherOrGold(player, "sanctuary_candle", "Cierge de veille", 36, lines))
                {
                    showShopResult("BÉNÉDICTION REFUSÉE", "shop.church.route_blessing.failed", lines);
                    continue;
                }
                player.getInventory().addMaterial(MaterialCatalog::createById("blessing_note", 1));
                lines.push_back("Père Orwan trace un signe court au bas d'un papier et refuse d'en faire une promesse absolue.");
                lines.push_back("Preuve obtenue : Note de bénédiction x1.");
                lines.push_back("Limite : utile pour des routes, quêtes et futurs dialogues, mais cela ne retire pas une vraie malédiction.");
                showLocalServiceResult("BÉNÉDICTION DE ROUTE", "shop.church.route_blessing.success", player, lines, 1);
                continue;
            }

            if (choice == 8)
            {
                std::vector<std::string> lines;
                if (!payServiceWithVoucherOrGold(player, "sanctuary_candle", "Cierge de veille", 28, lines))
                {
                    showShopResult("RITE REFUSÉ", "shop.church.blessing.failed", lines);
                    continue;
                }

                player.getInventory().addMaterial(MaterialCatalog::createById("sanctuary_wax_seal", 1));
                lines.push_back("Rite d'apaisement : Frère Calixte grave ton nom sur une cire froide, puis la casse avant qu'elle ne colle à ton ombre.");
                lines.push_back("Preuve obtenue : Sceau de cire sanctuaire x1.");
                lines.push_back("Limite : cela ne retire pas les grandes malédictions, mais peut aider des quêtes, PNJ ou rites futurs.");
                showLocalServiceResult("RITE D'APAISEMENT", "shop.church.blessing.success", player, lines, 1);
                continue;
            }

            const int curseIndex = choice - 20;
            if (curseIndex >= 0 && curseIndex < static_cast<int>(curses.size()))
            {
                const PlayerCurse target = curses[static_cast<std::size_t>(curseIndex)];
                std::vector<std::string> lines;
                if (target.diagnosisLevel < 1)
                {
                    lines.push_back("Sœur Maëlys refuse : elle ne retire pas une trace affichée ?????.");
                    lines.push_back("Il faut au moins un diagnostic niveau 1 avant de tenter un exorcisme.");
                    showShopResult("EXORCISME REFUSÉ", "shop.church.exorcise.unknown", lines);
                    continue;
                }
                if (!target.removableByChurch || target.bossIdRequiredToBreak > 0)
                {
                    lines.push_back("Père Orwan refuse de mentir : cette malédiction ne partira pas par un simple rite d'église.");
                    lines.push_back(target.diagnosisLevel >= 3 && !target.removalHint.empty() ? "Condition : " + target.removalHint : "Condition : recherche totale conseillée.");
                    showShopResult("EXORCISME IMPOSSIBLE", "shop.church.exorcise.locked", lines);
                    continue;
                }

                if (!payServiceWithVoucherOrGold(player, "holy_water_vial", "Fiole d'eau bénite", 58, lines))
                {
                    showShopResult("EXORCISME REFUSÉ", "shop.church.exorcise.failed", lines);
                    continue;
                }

                const int beforeProgress = target.exorcismProgress;
                if (!player.advanceChurchExorcism(target.id))
                {
                    lines.push_back("Le rite n'accroche pas : la trace ne réagit pas comme une malédiction retirable ici.");
                    showShopResult("EXORCISME ÉCHOUÉ", "shop.church.exorcise.invalid", lines);
                    continue;
                }

                if (!player.hasActiveCurse(target.id))
                {
                    lines.push_back("Exorcisme terminé : " + target.name + " ne pèse plus dans les traces actives.");
                    lines.push_back("Maëlys rappelle que certaines malédictions rares peuvent revenir si leur source n'est pas traitée.");
                    showLocalServiceResult("MALÉDICTION RETIRÉE", "shop.church.exorcise.removed", player, lines, 1);
                    continue;
                }

                const PlayerCurse* updated = findActiveCurseById(player, target.id);
                if (updated != nullptr)
                {
                    lines.push_back("Exorcisme progressif : " + std::to_string(beforeProgress) + " -> " + std::to_string(updated->exorcismProgress) + "/" + std::to_string(updated->exorcismRequiredVisits) + ".");
                    lines.push_back("Important : il faudra revenir un autre jour/passage pour continuer. Le jeu ne bloque pas toute la journée, donc c'est au joueur d'y penser.");
                }
                showLocalServiceResult("EXORCISME EN COURS", "shop.church.exorcise.progress", player, lines, 1);
                continue;
            }
        }
    }

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

        if (!player.getInventory().spendGold(offer.price))
        {
            lines.push_back("Paiement refusé : il faut " + Money::formatGoldWithRaw(offer.price) + ".");
            return false;
        }

        if (!player.getInventory().removeMaterialQuantityById(offer.materialId, offer.materialQuantity))
        {
            lines.push_back("Composant introuvable au moment de graver la rune. L'or n'a pas pu être récupéré automatiquement.");
            return false;
        }

        lines.push_back("Coût payé : " + Money::formatGoldWithRaw(offer.price) + " + " + offer.materialName + " x" + std::to_string(offer.materialQuantity) + ".");
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
                offer.label + " | " + Money::formatGoldWithRaw(offer.price) + " + " + offer.materialName + " x" + std::to_string(offer.materialQuantity),
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
                "Coût : " + Money::formatGoldWithRaw(offer.price) + " + " + offer.materialName + " x" + std::to_string(offer.materialQuantity),
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
                "Coût : " + Money::formatGoldWithRaw(offer.price) + " + " + offer.materialName + " x" + std::to_string(offer.materialQuantity),
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
            lines.push_back("Effet : l'armure peut maintenant compter dans les protections de température et affinités élémentaires.");
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
        if (!player.getInventory().spendGold(price))
        {
            lines.push_back("Paiement refusé : il faut " + Money::formatGoldWithRaw(price) + ".");
            lines.push_back("Argent disponible : " + Money::formatCurrencyOverviewFromCopper(player.getInventory().getTotalCopper()) + ".");
            return false;
        }
        player.getInventory().removeMaterialQuantityById("arcane_dust", dustCost);
        lines.push_back("Coût payé : " + Money::formatGoldWithRaw(price) + " + Poussière arcanique x" + std::to_string(dustCost) + ".");
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
        if (!player.getInventory().spendGold(price))
        {
            lines.push_back("Paiement refusé : il faut " + Money::formatGoldWithRaw(price) + ".");
            lines.push_back("Argent disponible : " + Money::formatCurrencyOverviewFromCopper(player.getInventory().getTotalCopper()) + ".");
            return false;
        }
        player.getInventory().removeMaterialQuantityById("runic_stabilizer", 1);
        player.getInventory().removeMaterialQuantityById("arcane_dust", dustCost);
        lines.push_back("Coût payé : " + Money::formatGoldWithRaw(price) + " + Stabilisateur runique x1 + Poussière arcanique x" + std::to_string(dustCost) + ".");
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
        if (!player.getInventory().spendGold(price))
        {
            lines.push_back("Paiement refusé : il faut " + Money::formatGoldWithRaw(price) + ".");
            lines.push_back("Argent disponible : " + Money::formatCurrencyOverviewFromCopper(player.getInventory().getTotalCopper()) + ".");
            return false;
        }
        player.getInventory().removeMaterialQuantityById("runic_extraction_note", 1);
        player.getInventory().removeMaterialQuantityById("arcane_dust", dustCost);
        lines.push_back("Coût payé : " + Money::formatGoldWithRaw(price) + " + Note d'extraction runique x1 + Poussière arcanique x" + std::to_string(dustCost) + ".");
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
        typeScreen.addLine("Coût : 95 pièces + Note d'extraction runique x1 + Poussière arcanique x3.");
        typeScreen.addLine("Échec : la rune peut se dissoudre. Ce service n'est donc pas une duplication gratuite.");
        typeScreen.addOption(0, "Retour", "Revenir à l'atelier.", true, "shop.enchanter.transfer.back");
        typeScreen.addOption(1, "Transférer entre armes", "Déplace la dernière rune d'une arme vers une autre arme.", player.getInventory().getWeaponCount() >= 2, "shop.enchanter.transfer.weapon");
        typeScreen.addOption(2, "Transférer entre armures", "Déplace la dernière rune d'une armure vers une autre armure.", player.getInventory().getArmorCount() >= 2, "shop.enchanter.transfer.armor");
        Console::clear();
        const int choice = TerminalInterface::askMenuChoiceFromOptions(typeScreen, "Choisis le type de transfert.");
        if (choice == 1) transferWeaponRune(player);
        else if (choice == 2) transferArmorRune(player);
    }

    void openEnchanterServiceMenu(Player& player)
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
            screen.addLine("Argent : " + Money::formatCurrencyOverviewFromCopper(player.getInventory().getTotalCopper()));
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
                if (!player.getInventory().spendGold(price))
                {
                    lines.push_back("Paiement refusé : il faut " + Money::formatGoldWithRaw(price) + ".");
                    lines.push_back("Argent disponible : " + Money::formatCurrencyOverviewFromCopper(player.getInventory().getTotalCopper()) + ".");
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


    void reduceCityRepairDays(Player& player, int days)
    {
        const int owned = player.getInventory().countMaterialById("city_repair_days_marker");
        if (owned <= 0 || days <= 0)
        {
            return;
        }
        player.getInventory().removeMaterialQuantityById("city_repair_days_marker", std::min(owned, days));
        if (player.getInventory().countMaterialById("city_repair_days_marker") <= 0)
        {
            const int notices = player.getInventory().countMaterialById("city_damage_notice");
            if (notices > 0)
            {
                player.getInventory().removeMaterialQuantityById("city_damage_notice", notices);
            }
        }
    }


    constexpr int kRegularCityEventIntervalDays = 7;
    constexpr int kRoyalBonusCityEventChancePercent = 32;

    int safeCityDay(const Player& player)
    {
        return std::max(0, player.getWorldDaysElapsed());
    }

    CityEventOfDay regularCityEventForWeekIndex(int weekIndex)
    {
        const int eventIndex = std::max(0, weekIndex) % 8;
        switch (eventIndex)
        {
            case 0:
                return {"guild_tournament", "Tournoi de guilde", "La place est balisée, les soigneurs restent proches et les parieurs parlent trop fort.", "S'inscrire comme participant encadré"};
            case 1:
                return {"monster_hunt", "Chasse aux monstres locale", "Les pisteurs comparent des traces au lieu de vendre des promesses héroïques.", "Aider à identifier les pistes"};
            case 2:
                return {"merchant_fair", "Foire marchande", "Des étals temporaires prennent toute la rue et les prix changent avec le sourire du vendeur.", "Tenir un comptoir ou négocier"};
            case 3:
                return {"knowledge_day", "Journée du savoir", "L'archiviste sort des cartes, des notes et des vieux avertissements que personne ne lit assez.", "Assister aux explications"};
            case 4:
                return {"mission_exchange", "Bourse aux missions", "La guilde trie les demandes trop petites pour un grand contrat, mais trop urgentes pour attendre.", "Aider au tri des demandes"};
            case 5:
                return {"honor_ceremony", "Cérémonie d'honneur", "Les gardes lisent les noms de ceux qui ont tenu une rue, une porte ou juste une promesse.", "Présenter ses services récents"};
            case 6:
                return {"village_games", "Jeux du village", "Ce n'est pas un entraînement militaire, mais tout le monde regarde quand quelqu'un tombe dans la boue.", "Participer sans casser le mobilier"};
            default:
                return {"harvest_festival", "Fête des moissons / solstice", "Les cuisines fument, les greniers s'ouvrent et les anciens surveillent qui aide vraiment.", "Porter, compter et distribuer"};
        }
    }

    int projectedCityMarkerDays(const Player* player, const std::string& materialId, int targetDay)
    {
        if (player == nullptr)
        {
            return 0;
        }
        const int daysAhead = std::max(0, std::max(0, targetDay) - safeCityDay(*player));
        return std::max(0, player->getInventory().countMaterialById(materialId) - daysAhead);
    }

    int cityScheduledWeekPressurePercent(int targetDay)
    {
        const int safeTargetDay = std::max(0, targetDay);
        const int dayInWeek = safeTargetDay % kRegularCityEventIntervalDays;
        int pressure = 0;

        // Même si le joueur ne participe pas, la ville doit quand même gérer l'affiche régulière
        // de la semaine : gardes, place publique, stocks, annonces, familles qui viennent voir.
        // Cela compte dans la chance du second événement royal : on ne rajoute pas une fête au-dessus
        // d'une ville déjà occupée par sa propre affiche.
        if (dayInWeek >= 1 && dayInWeek <= 4)
        {
            pressure += 12;
        }
        else if (dayInWeek >= 5)
        {
            pressure += 6;
        }

        // Petite variation fixe de semaine : rumeurs de passage, inventaire municipal, pluie, route fermée.
        // Ça évite que chaque semaine soit mathématiquement identique sans créer un event de plus.
        const int weekIndex = safeTargetDay / kRegularCityEventIntervalDays;
        pressure += (weekIndex * 11 + 3) % 7;
        return std::min(18, pressure);
    }

    int cityOtherEventPressurePercent(const Player* player, int targetDay)
    {
        if (player == nullptr)
        {
            return 0;
        }

        int pressure = cityScheduledWeekPressurePercent(targetDay);
        if (projectedCityMarkerDays(player, "city_repair_days_marker", targetDay) > 0
            || player->getInventory().countMaterialById("city_damage_notice") > 0)
        {
            // Une ville déjà occupée à réparer ne rajoute pas une seconde fête royale par-dessus.
            pressure += 100;
        }
        if (projectedCityMarkerDays(player, "city_defense_gratitude_days_marker", targetDay) > 0)
        {
            // Une défense récente compte déjà comme gros événement local : le bonus royal devient moins probable.
            pressure += 22;
        }
        const int recentEventDays = projectedCityMarkerDays(player, "city_event_recent_days_marker", targetDay);
        if (recentEventDays > 0)
        {
            // Une foire, un tournoi ou une affiche royale déjà gérée dans la semaine fatigue les organisateurs.
            pressure += std::min(30, 12 + recentEventDays * 6);
        }
        return std::min(100, pressure);
    }

    int royalBonusCityEventChanceForPlayer(const Player* player, int targetDay)
    {
        return std::max(0, kRoyalBonusCityEventChancePercent - cityOtherEventPressurePercent(player, targetDay));
    }

    std::string royalBonusChanceContextLine(const Player& player, int targetDay)
    {
        const int pressure = cityOtherEventPressurePercent(&player, targetDay);
        const int chance = royalBonusCityEventChanceForPlayer(&player, targetDay);
        if (pressure >= 100)
        {
            return "Second événement royal : impossible pour cette date, la ville gère déjà une crise, des dégâts ou des réparations.";
        }
        return "Second événement royal : chance actuelle " + std::to_string(chance)
            + "% après prise en compte des affiches régulières, événements récents, défenses, réparations et tensions de semaine.";
    }

    bool hasRoyalBonusCityEventThisWeekIndex(int weekIndex, const Player* player = nullptr, int targetDay = 0)
    {
        const int seed = (std::max(0, weekIndex) * 73 + 29) % 100;
        return seed < royalBonusCityEventChanceForPlayer(player, targetDay);
    }

    int royalBonusCityEventOffsetForWeekIndex(int weekIndex)
    {
        return 2 + ((std::max(0, weekIndex) * 17 + 5) % 3); // entre 2 et 4 jours après l'affiche régulière
    }

    CityEventOfDay royalBonusCityEventForWeekIndex(int weekIndex)
    {
        switch ((std::max(0, weekIndex) * 5 + 1) % 4)
        {
            case 0:
                return {"royal_merit_reward", "Récompense royale des mérites du peuple", "Un héraut affirme que le roi veut remercier les artisans, gardes et petites mains restées debout.", "Aider à distribuer sans te servir au passage"};
            case 1:
                return {"royal_whim_games", "Jeux ordonnés par caprice royal", "Personne ne sait si le roi était inspiré ou juste de bonne humeur, mais les fanions sont déjà posés.", "Participer à une épreuve courte et encadrée"};
            case 2:
                return {"royal_supply_day", "Distribution royale de provisions", "Les greniers ouvrent un peu : pas assez pour devenir riche, assez pour calmer la rue.", "Aider à compter et porter les caisses"};
            default:
                return {"royal_patrol_gratitude", "Patrouille honorifique du roi", "Un officier veut montrer que la couronne n'oublie pas les portes qui tiennent.", "Escorter une ronde symbolique"};
        }
    }

    bool hasRegularCityEventOnDay(int day)
    {
        return std::max(0, day) % kRegularCityEventIntervalDays == 0;
    }

    bool hasRoyalBonusCityEventOnDay(int day, const Player* player = nullptr)
    {
        const int safeDay = std::max(0, day);
        if (hasRegularCityEventOnDay(safeDay))
        {
            return false;
        }
        const int weekIndex = safeDay / kRegularCityEventIntervalDays;
        return hasRoyalBonusCityEventThisWeekIndex(weekIndex, player, safeDay)
            && safeDay % kRegularCityEventIntervalDays == royalBonusCityEventOffsetForWeekIndex(weekIndex);
    }

    bool hasRegularCityEventToday(const Player& player)
    {
        return hasRegularCityEventOnDay(safeCityDay(player));
    }

    bool hasRoyalBonusCityEventToday(const Player& player)
    {
        return hasRoyalBonusCityEventOnDay(safeCityDay(player), &player);
    }

    bool hasAnyCityEventToday(const Player& player)
    {
        return hasRegularCityEventToday(player) || hasRoyalBonusCityEventToday(player);
    }


    CityEventOfDay cityEventForAbsoluteDay(int day, const Player* player = nullptr)
    {
        const int safeDay = std::max(0, day);
        const int weekIndex = safeDay / kRegularCityEventIntervalDays;
        if (hasRoyalBonusCityEventOnDay(safeDay, player))
        {
            return royalBonusCityEventForWeekIndex(weekIndex);
        }
        return regularCityEventForWeekIndex(weekIndex);
    }

    CityEventOfDay cityEventForTodayOrNext(const Player& player)
    {
        const int today = safeCityDay(player);
        if (hasAnyCityEventToday(player))
        {
            return cityEventForAbsoluteDay(today, &player);
        }
        for (int offset = 1; offset <= kRegularCityEventIntervalDays; ++offset)
        {
            if (hasRegularCityEventOnDay(today + offset) || hasRoyalBonusCityEventOnDay(today + offset, &player))
            {
                return cityEventForAbsoluteDay(today + offset, &player);
            }
        }
        return regularCityEventForWeekIndex((today + kRegularCityEventIntervalDays) / kRegularCityEventIntervalDays);
    }

    int daysUntilNextRegularCityEvent(const Player& player)
    {
        const int remainder = safeCityDay(player) % kRegularCityEventIntervalDays;
        return remainder == 0 ? 0 : kRegularCityEventIntervalDays - remainder;
    }

    int daysUntilNextAnyCityEvent(const Player& player)
    {
        const int today = safeCityDay(player);
        for (int offset = 0; offset <= kRegularCityEventIntervalDays; ++offset)
        {
            if (hasRegularCityEventOnDay(today + offset) || hasRoyalBonusCityEventOnDay(today + offset, &player))
            {
                return offset;
            }
        }
        return daysUntilNextRegularCityEvent(player);
    }

    bool isRoyalBonusCityEvent(const CityEventOfDay& event)
    {
        return event.id.rfind("royal_", 0) == 0;
    }

    void markCityEventAsRecentlyHandled(Player& player, bool royalEvent)
    {
        const int previousDays = player.getInventory().countMaterialById("city_event_recent_days_marker");
        if (previousDays > 0)
        {
            player.getInventory().removeMaterialQuantityById("city_event_recent_days_marker", previousDays);
        }
        const int newDays = std::min(4, std::max(previousDays, royalEvent ? 3 : 2));
        player.getInventory().addMaterial(MaterialCatalog::createById("city_event_recent_days_marker", newDays));
    }

    std::vector<std::string> cityRaceReactionLines(const Player& player)
    {
        std::vector<std::string> lines;
        switch (player.getRace())
        {
            case CharacterRace::SemiFox:
            case CharacterRace::Kitsune:
                lines.push_back("Réaction locale : certains marchands gardent un œil sur les balances, mais écoutent volontiers une bonne négociation.");
                break;
            case CharacterRace::SemiDog:
            case CharacterRace::SemiWolf:
                lines.push_back("Réaction locale : les gardes et pisteurs parlent plus vite de patrouille que de paperasse.");
                break;
            case CharacterRace::SemiCat:
                lines.push_back("Réaction locale : on te confie facilement les toits, les caves et les passages trop étroits pour les armures.");
                break;
            case CharacterRace::SemiLizard:
                lines.push_back("Réaction locale : les ouvriers pensent aux canaux, aux mares et aux pierres humides quand ils te voient arriver.");
                break;
            case CharacterRace::SemiBird:
                lines.push_back("Réaction locale : les messagers demandent surtout ce que tu as vu depuis les hauteurs.");
                break;
            case CharacterRace::Gnome:
            case CharacterRace::Dwarf:
                lines.push_back("Réaction locale : les artisans te parlent outils, mesures et réparations avant même de parler prime.");
                break;
            case CharacterRace::Vampire:
            case CharacterRace::Tiefling:
            case CharacterRace::DarkElf:
                lines.push_back("Réaction locale : l'accueil reste poli, mais les temples et guichets vérifient deux fois les sceaux.");
                break;
            default:
                break;
        }
        return lines;
    }

    void appendCityEventRaceAid(Player& player, const CityEventOfDay& event, Random& random, std::vector<std::string>& lines)
    {
        bool helped = false;
        switch (player.getRace())
        {
            case CharacterRace::SemiFox:
            case CharacterRace::Kitsune:
                if (event.id == "merchant_fair" || event.id == "mission_exchange" || event.id == "royal_merit_reward")
                {
                    player.getInventory().addMaterial(MaterialCatalog::createById("local_reputation_note", 1));
                    lines.push_back("Atout racial : ta ruse aide à repérer une clause bizarre ou un client trop sûr de lui. Note de réputation locale x1.");
                    helped = true;
                }
                break;
            case CharacterRace::SemiDog:
            case CharacterRace::SemiWolf:
                if (event.id == "monster_hunt" || event.id == "royal_patrol_gratitude")
                {
                    player.getInventory().addMaterial(MaterialCatalog::createById("advanced_monster_notes", 1));
                    lines.push_back("Atout racial : flair et rondes propres donnent une piste plus fiable. Notes avancées de monstres x1.");
                    helped = true;
                }
                break;
            case CharacterRace::SemiCat:
                if (event.id == "village_games" || event.id == "honor_ceremony")
                {
                    player.getInventory().addMaterial(MaterialCatalog::createById("city_service_stamp", 1));
                    lines.push_back("Atout racial : les passages étroits et les toits deviennent une vraie aide logistique. Tampon de service municipal x1.");
                    helped = true;
                }
                break;
            case CharacterRace::SemiLizard:
                if (event.id == "harvest_festival" || event.id == "royal_supply_day")
                {
                    player.getInventory().addMaterial(MaterialCatalog::createById("bitter_healing_leaf", 1));
                    lines.push_back("Atout racial : tu aides près des canaux et réserves humides sans glisser toutes les deux secondes. Feuille amère de soin x1.");
                    helped = true;
                }
                break;
            case CharacterRace::SemiBird:
                if (event.id == "monster_hunt" || event.id == "mission_exchange" || event.id == "royal_patrol_gratitude")
                {
                    player.getInventory().addMaterial(MaterialCatalog::createById("route_scout_note", 1));
                    lines.push_back("Atout racial : un regard depuis les hauteurs évite une fausse piste. Note d'éclaireur de route x1.");
                    helped = true;
                }
                break;
            case CharacterRace::Gnome:
            case CharacterRace::Dwarf:
                if (event.id == "merchant_fair" || event.id == "knowledge_day")
                {
                    player.getInventory().addMaterial(MaterialCatalog::createById("municipal_proof_letter", 1));
                    lines.push_back("Atout racial : mesures, outils et détails propres convainquent le bureau. Attestation municipale x1.");
                    helped = true;
                }
                break;
            default:
                break;
        }

        if (!helped && random.between(1, 100) <= 12)
        {
            lines.push_back("Petit détail local : rien de spécial à exploiter aujourd'hui, mais l'aide reste notée sans bonus abusif.");
        }
    }

    void appendCityEventReward(Player& player, const CityEventOfDay& event, std::vector<std::string>& lines)
    {
        Random random;
        if (event.id == "royal_merit_reward")
        {
            const int reward = 10 + player.getLevel() / 2;
            player.getInventory().earnGold(reward);
            player.getInventory().addMaterial(MaterialCatalog::createById("local_reputation_note", 1));
            lines.push_back("Le héraut insiste : ce n'est pas une foire permanente, juste une grâce rare pour services rendus au peuple.");
            lines.push_back("Récompense : " + Money::formatGoldWithRaw(reward) + " + Note de réputation locale x1.");
        }
        else if (event.id == "royal_whim_games")
        {
            player.getInventory().addMaterial(MaterialCatalog::createById("city_service_stamp", 1));
            player.getInventory().addMaterial(MaterialCatalog::createById("survival_ration", 1));
            lines.push_back("L'épreuve est courte et encadrée : assez sérieuse pour compter, pas assez rentable pour être farmée.");
            lines.push_back("Récompense : Tampon de service municipal x1 + ration de survie x1.");
        }
        else if (event.id == "royal_supply_day")
        {
            player.getInventory().addMaterial(MaterialCatalog::createById("survival_ration", random.between(1, 2)));
            player.getInventory().addMaterial(MaterialCatalog::createById("bitter_healing_leaf", 1));
            lines.push_back("Tu aides surtout à éviter les resquilleurs et les doubles comptes. Le roi paie l'affiche, pas ton futur château.");
            lines.push_back("Ressources : ration(s) de survie + feuille médicinale amère x1.");
        }
        else if (event.id == "royal_patrol_gratitude")
        {
            const int reward = 12 + player.getLevel();
            player.getInventory().earnGold(reward);
            player.getInventory().addMaterial(MaterialCatalog::createById("city_service_stamp", 1));
            lines.push_back("La patrouille n'a rien d'une guerre, mais la présence d'un aventurier rassure les rues." );
            lines.push_back("Récompense : " + Money::formatGoldWithRaw(reward) + " + Tampon de service municipal x1.");
        }
        else if (event.id == "guild_tournament")
        {
            const int reward = 14 + player.getLevel();
            player.getInventory().earnGold(reward);
            player.getInventory().addMaterial(MaterialCatalog::createById("guild_favor_token", 1));
            lines.push_back("Tu combats ou arbitres en cadre sécurisé : assez réel pour apprendre, pas assez libre pour finir en massacre.");
            lines.push_back("Récompense : " + Money::formatGoldWithRaw(reward) + " + Jeton de faveur de guilde x1.");
        }
        else if (event.id == "monster_hunt")
        {
            player.getInventory().addMaterial(MaterialCatalog::createById("advanced_monster_notes", 1));
            if (random.between(1, 100) <= 45)
            {
                player.getInventory().addMaterial(MaterialCatalog::createById("wolf_fang", 1));
                lines.push_back("Un pisteur te laisse un croc marqué pour comparer les morsures plus tard.");
            }
            lines.push_back("Note obtenue : notes avancées de monstres x1.");
        }
        else if (event.id == "merchant_fair")
        {
            const int reward = 10 + player.getLevel() / 2;
            player.getInventory().earnGold(reward);
            player.getInventory().addMaterial(MaterialCatalog::createById("local_service_letter", 1));
            lines.push_back("Tu aides à tenir un étal, contrôler une facture ou calmer une dispute de prix sans reprendre les mauvais prix des fiches.");
            lines.push_back("Récompense : " + Money::formatGoldWithRaw(reward) + " + Lettre de service local x1.");
        }
        else if (event.id == "knowledge_day")
        {
            if (player.getInventory().spendGold(12))
            {
                player.getInventory().addMaterial(MaterialCatalog::createById("magic_learning_notes", 1));
                lines.push_back("Tu paies une place modeste pour copier des notes propres au lieu d'écouter depuis la fenêtre.");
                lines.push_back("Notes obtenues : notes d'apprentissage magique x1.");
            }
            else
            {
                player.getInventory().addMaterial(MaterialCatalog::createById("client_recommendation", 1));
                lines.push_back("Tu n'as pas payé la copie complète, mais l'archiviste te donne une recommandation pour revenir mieux préparé.");
            }
        }
        else if (event.id == "mission_exchange")
        {
            player.getInventory().addMaterial(MaterialCatalog::createById("client_recommendation", 1));
            player.getInventory().addMaterial(MaterialCatalog::createById("local_service_letter", 1));
            lines.push_back("Tu tries les demandes qui sentent la vraie urgence et celles qui sentent juste le client qui crie fort.");
            lines.push_back("Documents obtenus : recommandation de client x1, lettre de service local x1.");
        }
        else if (event.id == "honor_ceremony")
        {
            player.getInventory().addMaterial(MaterialCatalog::createById("city_service_stamp", 1));
            if (player.getInventory().countMaterialById("city_defense_medal") > 0)
            {
                player.getInventory().addMaterial(MaterialCatalog::createById("local_reputation_note", 1));
                lines.push_back("Ton attestation de défense donne du poids à ton nom pendant la cérémonie.");
            }
            lines.push_back("Tampon de service municipal x1 obtenu.");
        }
        else if (event.id == "village_games")
        {
            const int reward = random.between(6, 18 + player.getLevel());
            player.getInventory().earnGold(reward);
            player.getInventory().addMaterial(MaterialCatalog::createById("survival_ration", 1));
            lines.push_back("Tu gagnes surtout de la boue, quelques rires, et assez de nourriture pour ne pas appeler ça une perte de temps.");
            lines.push_back("Récompense : " + Money::formatGoldWithRaw(reward) + " + ration de survie x1.");
        }
        else
        {
            player.getInventory().addMaterial(MaterialCatalog::createById("bitter_healing_leaf", random.between(1, 3)));
            player.getInventory().addMaterial(MaterialCatalog::createById("survival_ration", 1));
            lines.push_back("Tu aides à porter, compter, distribuer. Pas glorieux, mais les greniers ne se remplissent pas en criant 'quête'.");
            lines.push_back("Ressources obtenues : plantes communes et ration de survie.");
        }

        appendCityEventRaceAid(player, event, random, lines);
    }

    void participateInCityEventOfDay(Player& player)
    {
        if (!hasAnyCityEventToday(player))
        {
            const CityEventOfDay nextEvent = cityEventForTodayOrNext(player);
            showShopResult(
                "AUCUN ÉVÉNEMENT ACTIF",
                "shop.city.events.none_today",
                {
                    "Il n'y a pas d'événement de guilde, de village ou d'ordre royal aujourd'hui. La ville respire aussi entre deux affiches.",
                    "Rythme actuel : une affiche régulière environ par semaine. Un second événement royal reste rare : sa chance baisse aussi avec l'affiche régulière, les autres événements, les défenses, crises et réparations.",
                    "Prochaine affiche dans " + std::to_string(daysUntilNextAnyCityEvent(player)) + " jour(s) : " + nextEvent.name + "."
                }
            );
            return;
        }

        const CityEventOfDay event = cityEventForTodayOrNext(player);
        std::vector<std::string> lines = {
            std::string(isRoyalBonusCityEvent(event) ? "Événement exceptionnel actif : " : "Événement régulier actif : ") + event.name + ".",
            event.mood,
            "Action choisie : " + event.action + "."
        };
        if (isRoyalBonusCityEvent(event))
        {
            lines.push_back("Origine : ordre du roi, récompense des mérites du peuple ou caprice officiel. Ça peut arriver en plus de l'affiche hebdomadaire, mais la ville évite d'empiler ça sur une crise ou une défense récente.");
        }
        std::vector<std::string> raceLines = cityRaceReactionLines(player);
        lines.insert(lines.end(), raceLines.begin(), raceLines.end());
        appendCityEventReward(player, event, lines);
        markCityEventAsRecentlyHandled(player, isRoyalBonusCityEvent(event));
        lines.push_back(isRoyalBonusCityEvent(event)
            ? "Organisation : l'affiche royale occupe le bureau pendant quelques jours, ce qui rend un autre événement encore moins probable."
            : "Organisation : l'événement compte comme animation récente de ville et réduit un peu la chance d'un second événement royal cette semaine.");
        showLocalServiceResult("ÉVÉNEMENT DE VILLE", "shop.city.events.participate", player, lines, 1);
    }

    void showLocalEconomyReport(const Player& player)
    {
        std::vector<std::string> lines;
        const int repairDays = cityRepairDaysRemaining(player);
        if (repairDays > 0)
        {
            lines.push_back("Économie locale : crise active, réparations restantes " + std::to_string(repairDays) + " jour(s).");
            lines.push_back("Effet concret : commerces fermés ou ouverts au compte-gouttes ; les demandes utiles valent souvent mieux que négocier trois pièces.");
            lines.push_back("Ressources recherchées : métal rouillé, cuir abîmé, feuilles de soin, rations, poussière arcanique selon le quartier touché.");
            lines.push_back("Prix de crise : les comptoirs ouverts hors services prioritaires montent un peu leurs prix, mais ton aide locale peut réduire cette tension jusqu'à un minimum raisonnable.");
        }
        else
        {
            const CityEventOfDay event = cityEventForTodayOrNext(player);
            lines.push_back("Économie locale : stable pour l'instant.");
            const int gratitudeDays = player.getInventory().countMaterialById("city_defense_gratitude_days_marker");
            const int recentEventDays = player.getInventory().countMaterialById("city_event_recent_days_marker");
            if (gratitudeDays > 0)
            {
                lines.push_back("Reconnaissance de défense : petites remises d'achat encore actives pendant " + std::to_string(gratitudeDays) + " jour(s), sauf marché noir et exceptions louches.");
            }
            if (recentEventDays > 0)
            {
                lines.push_back("Organisation locale : un événement récent occupe encore le bureau pendant " + std::to_string(recentEventDays) + " jour(s), donc un second événement royal devient moins probable.");
            }
            if (hasAnyCityEventToday(player))
            {
                lines.push_back(std::string(isRoyalBonusCityEvent(event) ? "Événement exceptionnel actif aujourd'hui : " : "Événement régulier actif aujourd'hui : ") + event.name + ".");
                if (isRoyalBonusCityEvent(event))
                {
                    lines.push_back("Raison annoncée : ordre royal, mérite du peuple ou caprice de cour. C'est rare, donc les récompenses restent modestes.");
                }
                lines.push_back("Marché : certaines familles de boutiques peuvent bouger de 2 ou 3%, mais rien qui permette de casser l'économie.");
            }
            else
            {
                lines.push_back("Pas d'événement actif aujourd'hui. Prochaine affiche dans " + std::to_string(daysUntilNextAnyCityEvent(player)) + " jour(s) : " + event.name + ".");
            }
            lines.push_back("Équilibre : les événements donnent surtout documents, ressources et petites primes ; les gros gains doivent rester liés aux vrais risques.");
            lines.push_back("Prix : les petites variations de ville restent limitées à quelques pourcents. Acheter/revendre en boucle ne doit pas devenir une stratégie.");
            lines.push_back("Plafond : réputation, défense et affiche favorable ne peuvent pas empiler plus de " + std::to_string(kCityEconomyDiscountCapPercent) + "% de remise locale utile.");
            lines.push_back(royalBonusChanceContextLine(player, safeCityDay(player)));
            lines.push_back("Troc : les ressources locales gardent une valeur utile, surtout quand la ville répare ou manque de bras.");
        }
        const std::vector<std::string> raceLines = cityRaceReactionLines(player);
        lines.insert(lines.end(), raceLines.begin(), raceLines.end());
        showShopResult("ÉCONOMIE LOCALE", "shop.city.economy.report", lines);
    }

    void showCityAndGuildNoticeBoard(const Player& player)
    {
        std::vector<std::string> lines;
        const int repairDays = cityRepairDaysRemaining(player);
        lines.push_back("Panneau commun : les annonces de ville et de guilde servent surtout à lire le contexte, pas à donner une récompense gratuite.");
        lines.push_back("Date locale : " + player.formatWorldDateTimeLine() + ".");

        if (repairDays > 0)
        {
            lines.push_back("Priorité affichée : réparations encore " + std::to_string(repairDays) + " jour(s). Les demandes utiles restent réparation, garde, récolte et remise en route.");
            lines.push_back("La guilde suspend les annonces festives : elle préfère des bras fiables à une nouvelle cérémonie.");
        }
        else
        {
            const CityEventOfDay event = cityEventForTodayOrNext(player);
            if (hasAnyCityEventToday(player))
            {
                lines.push_back(std::string(isRoyalBonusCityEvent(event) ? "Affiche active exceptionnelle : " : "Affiche active régulière : ") + event.name + ".");
                lines.push_back("Rappel : participer consomme du temps et donne surtout documents, ressources ou petite réputation, pas une pluie d'or.");
            }
            else
            {
                lines.push_back("Aucune affiche active aujourd'hui. Prochaine affiche dans " + std::to_string(daysUntilNextAnyCityEvent(player)) + " jour(s) : " + event.name + ".");
                lines.push_back(royalBonusChanceContextLine(player, safeCityDay(player)));
            }
            const int weekIndex = safeCityDay(player) / kRegularCityEventIntervalDays;
            switch ((weekIndex * 13 + 4) % 5)
            {
                case 0:
                    lines.push_back("Rumeur de guilde : une patrouille cherche des volontaires, mais rien n'est encore assez grave pour devenir une quête officielle.");
                    break;
                case 1:
                    lines.push_back("Rumeur marchande : les artisans surveillent le stock de métal et de cuir avant la prochaine affiche.");
                    break;
                case 2:
                    lines.push_back("Rumeur de quartier : quelques familles demandent surtout des bras fiables, pas un héros qui casse la porte.");
                    break;
                case 3:
                    lines.push_back("Rumeur de garde : les rondes ont été renforcées autour des comptoirs sensibles.");
                    break;
                default:
                    lines.push_back("Rumeur calme : la ville respire un peu. C'est aussi important qu'une quête.");
                    break;
            }
        }

        const int recentEventDays = player.getInventory().countMaterialById("city_event_recent_days_marker");
        if (recentEventDays > 0)
        {
            lines.push_back("Organisation : le bureau reste occupé encore " + std::to_string(recentEventDays) + " jour(s) par une affiche récente.");
        }

        if (player.getInventory().countMaterialById("city_defense_gratitude_days_marker") > 0)
        {
            lines.push_back("Défense récente : les commerçants reconnaissent encore ton nom, mais la remise reste courte et plafonnée.");
        }

        switch (player.getRace())
        {
            case CharacterRace::SemiFox:
            case CharacterRace::Kitsune:
                lines.push_back("Piste raciale : contrats, foires et bourses aux missions sont les meilleurs moments pour repérer une clause louche.");
                break;
            case CharacterRace::SemiDog:
            case CharacterRace::SemiWolf:
                lines.push_back("Piste raciale : les rondes, chasses locales et alertes de route t'emploient mieux que les comptoirs fermés.");
                break;
            case CharacterRace::SemiCat:
                lines.push_back("Piste raciale : les toits, greniers et passages étroits reviennent souvent pendant les réparations ou fêtes bondées.");
                break;
            case CharacterRace::SemiLizard:
                lines.push_back("Piste raciale : canaux, mares et pierres humides sont souvent oubliés par les ouvriers pressés.");
                break;
            case CharacterRace::SemiBird:
                lines.push_back("Piste raciale : observation, messagers et hauteurs donnent parfois une meilleure aide qu'un combat.");
                break;
            case CharacterRace::Gnome:
            case CharacterRace::Dwarf:
                lines.push_back("Piste raciale : mesures, outils et réparations précises comptent beaucoup quand l'économie locale se tend.");
                break;
            default:
                lines.push_back("Piste générale : les petites preuves locales servent surtout à ouvrir des services, pas à remplacer les vraies quêtes.");
                break;
        }

        showShopResult("PANNEAU VILLE / GUILDE", "shop.city.notice_board", lines);
    }

    void openCityRepairWorkOrderMenu(Player& player)
    {
        bool stay = true;
        while (stay)
        {
            MenuScreen screen("DEMANDES DE RÉPARATION", "shop.city.repair.orders");
            screen.addLine("Ces demandes sont courtes : elles servent à rendre la crise visible sans bloquer le jeu pendant trois heures.");
            screen.addLine("Réparations restantes : " + std::to_string(cityRepairDaysRemaining(player)) + " jour(s).");
            screen.addBackOption("Retour", "shop.city.repair.orders.back");
            screen.addOption(1, "Réparer l'infirmerie", "Demande : Feuille amère de soin x2 + Morceau de cuir abîmé x1.", true, "shop.city.repair.order.infirmary");
            screen.addOption(2, "Renforcer les étals du marché", "Demande : Fragment de métal rouillé x4 + Morceau de cuir abîmé x1.", true, "shop.city.repair.order.market");
            screen.addOption(3, "Rationner les ouvriers", "Demande : Ration de survie x2. Réduit surtout la panique et la fatigue.", true, "shop.city.repair.order.rations");
            screen.addOption(4, "Stabiliser une faille mineure", "Demande : Poussière arcanique x2. Utile si la crise a une trace magique.", true, "shop.city.repair.order.arcane");
            screen.addOption(5, "Aider selon tes atouts", "Petite intervention selon race/classe : toit, garde, canal, mesure, négociation. Gain modeste, pas farm gratuit.", true, "shop.city.repair.order.personal_skill");
            screen.addOption(6, "Consolider des murs fissurés", "Demande : Argile rouge séchée x2 + Fragment de métal rouillé x2.", true, "shop.city.repair.order.walls");
            screen.addOption(7, "Organiser une collecte de quartier", "Pas d'or direct : transforme un peu de temps en ressources simples pour les réparations.", true, "shop.city.repair.order.collection");
            screen.addOption(8, "Remettre une route de relais en état", "Demande : Reçu de péage de route x1 + Fragment de métal rouillé x2.", true, "shop.city.repair.order.relay_road");
            screen.addOption(9, "Préparer une réserve de secours", "Demande : Ration de survie x3 + Feuille amère de soin x1.", true, "shop.city.repair.order.reserve");
            screen.addOption(10, "Rouvrir le panneau des quêtes", "Demande : Lettre de service local x1 + Tampon municipal x1. Aide la guilde à relancer les petites missions.", true, "shop.city.repair.order.quest_board");
            screen.addOption(11, "Dégager une rue encombrée", "Sortie courte : peu d'or, quelques matériaux, parfois une journée de réparation gagnée.", true, "shop.city.repair.order.clear_street");
            screen.addOption(12, "Sécuriser un entrepôt fragile", "Demande : Kit de réparation faible x1 + Fragment de métal rouillé x1. Protège les stocks sans grosse prime.", true, "shop.city.repair.order.storage");
            screen.addOption(13, "Contrôler les prix abusifs", "Action courte : aide le guichet à repérer les profiteurs de crise. Gain surtout réputation/ordre local.", true, "shop.city.repair.order.price_check");
            screen.addOption(14, "Protéger un dépôt de rations", "Demande : Ration de survie x2 + Kit de réparation faible x1. Protège les stocks utiles.", true, "shop.city.repair.order.ration_depot");
            screen.addOption(15, "Faire une ronde avec la guilde", "Petite ronde utile : surtout sécurité, notes et chance limitée de réduire la crise.", true, "shop.city.repair.order.guild_round");

            Console::clear();
            const int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choisis une demande de réparation.");
            Console::clear();
            if (choice == 0)
            {
                stay = false;
                continue;
            }

            std::vector<std::string> lines;
            auto missing = [&](const std::string& id, int qty) {
                return player.getInventory().countMaterialById(id) < qty;
            };
            auto consume = [&](const std::string& id, int qty) {
                player.getInventory().removeMaterialQuantityById(id, qty);
            };

            if (choice == 1)
            {
                if (missing("bitter_healing_leaf", 2) || missing("worn_leather_piece", 1))
                {
                    showShopResult("MATÉRIAUX MANQUANTS", "shop.city.repair.order.infirmary.failed", {"Il faut Feuille amère de soin x2 et Morceau de cuir abîmé x1.", "L'infirmerie ne demande pas une belle histoire : elle demande de quoi panser les gens."});
                    continue;
                }
                consume("bitter_healing_leaf", 2);
                consume("worn_leather_piece", 1);
                reduceCityRepairDays(player, 1);
                player.getInventory().addMaterial(MaterialCatalog::createById("city_repair_receipt", 1));
                player.getInventory().addMaterial(MaterialCatalog::createById("local_reputation_note", 1));
                lines = {"Tu fournis bandages, sangles et plantes. L'infirmerie respire un peu mieux.", "Réparations réduites de 1 jour. Reçu d'aide aux réparations x1."};
                showLocalServiceResult("INFIRMERIE AIDÉE", "shop.city.repair.order.infirmary.success", player, lines, 1);
            }
            else if (choice == 2)
            {
                if (missing("rusted_metal_fragment", 4) || missing("worn_leather_piece", 1))
                {
                    showShopResult("MATÉRIAUX MANQUANTS", "shop.city.repair.order.market.failed", {"Il faut Fragment de métal rouillé x4 et Morceau de cuir abîmé x1.", "Les étals peuvent être moches, mais ils doivent tenir debout."});
                    continue;
                }
                consume("rusted_metal_fragment", 4);
                consume("worn_leather_piece", 1);
                reduceCityRepairDays(player, 2);
                player.getInventory().addMaterial(MaterialCatalog::createById("city_service_stamp", 1));
                lines = {"Tu aides à renforcer plusieurs comptoirs. Les marchands arrêtent de tenir leurs caisses à deux mains.", "Réparations réduites de 2 jours. Tampon de service municipal x1."};
                showLocalServiceResult("MARCHÉ RENFORCÉ", "shop.city.repair.order.market.success", player, lines, 1);
            }
            else if (choice == 3)
            {
                if (missing("survival_ration", 2))
                {
                    showShopResult("RATIONS MANQUANTES", "shop.city.repair.order.rations.failed", {"Il faut Ration de survie x2.", "Un chantier sans nourriture finit par réparer les disputes avant les murs."});
                    continue;
                }
                consume("survival_ration", 2);
                reduceCityRepairDays(player, 1);
                player.getInventory().addMaterial(MaterialCatalog::createById("local_service_letter", 1));
                lines = {"Les ouvriers mangent quelque chose qui ressemble presque à un repas. Le chantier évite une journée de retard.", "Réparations réduites de 1 jour. Lettre de service local x1."};
                showLocalServiceResult("OUVRIERS RAVITAILLÉS", "shop.city.repair.order.rations.success", player, lines, 1);
            }
            else if (choice == 4)
            {
                if (missing("arcane_dust", 2))
                {
                    showShopResult("COMPOSANTS MANQUANTS", "shop.city.repair.order.arcane.failed", {"Il faut Poussière arcanique x2.", "Le scribe refuse d'appeler ça de la magie noire. Il dit seulement que le mur grésille."});
                    continue;
                }
                consume("arcane_dust", 2);
                reduceCityRepairDays(player, 2);
                player.getInventory().addMaterial(MaterialCatalog::createById("municipal_proof_letter", 1));
                lines = {"La faille mineure se calme assez pour que les ouvriers approchent sans perdre leurs outils dans une lumière bizarre.", "Réparations réduites de 2 jours. Preuve municipale x1."};
                showLocalServiceResult("TRACE STABILISÉE", "shop.city.repair.order.arcane.success", player, lines, 1);
            }

            else if (choice == 5)
            {
                Random random;
                const int roll = random.between(1, 100);
                lines.push_back("Le contremaître ne te donne pas un grand discours : il cherche surtout où tes atouts peuvent éviter une erreur de plus.");
                switch (player.getRace())
                {
                    case CharacterRace::SemiCat:
                        lines.push_back("On t'envoie vérifier des toits, poutres basses et passages où une armure coincerait tout le monde.");
                        break;
                    case CharacterRace::SemiLizard:
                        lines.push_back("On te confie les canaux, pierres humides et écoulements qui ruinent souvent les réparations propres.");
                        break;
                    case CharacterRace::SemiBird:
                        lines.push_back("On te demande d'observer les hauteurs avant qu'un ouvrier découvre trop tard qu'un pan de mur bouge.");
                        break;
                    case CharacterRace::SemiDog:
                    case CharacterRace::SemiWolf:
                        lines.push_back("Les gardes te placent sur les odeurs, traces et rondes autour des entrepôts encore ouverts.");
                        break;
                    case CharacterRace::Gnome:
                    case CharacterRace::Dwarf:
                        lines.push_back("Les artisans te collent près des mesures, cales et outils : moins glorieux, plus utile.");
                        break;
                    default:
                        lines.push_back("Tu aides là où il manque des bras : porter, compter, surveiller, calmer quelqu'un qui veut déjà rouvrir trop tôt.");
                        break;
                }
                if (roll <= 45)
                {
                    reduceCityRepairDays(player, 1);
                    lines.push_back("Résultat : une erreur de chantier évitée. Réparations réduites de 1 jour.");
                }
                else
                {
                    lines.push_back("Résultat : la journée avance sans miracle, mais le quartier reste organisé.");
                }
                player.getInventory().addMaterial(MaterialCatalog::createById("local_reputation_note", 1));
                showLocalServiceResult("AIDE CIBLÉE", "shop.city.repair.order.personal_skill.success", player, lines, 1);
            }
            else if (choice == 6)
            {
                if (missing("sun_dried_clay", 2) || missing("rusted_metal_fragment", 2))
                {
                    showShopResult("MATÉRIAUX MANQUANTS", "shop.city.repair.order.walls.failed", {"Il faut Argile rouge séchée x2 et Fragment de métal rouillé x2.", "Les murs fissurés n'ont pas besoin d'un héros légendaire : ils ont besoin de cales, d'argile et de métal."});
                    continue;
                }
                consume("sun_dried_clay", 2);
                consume("rusted_metal_fragment", 2);
                reduceCityRepairDays(player, 2);
                player.getInventory().addMaterial(MaterialCatalog::createById("city_repair_receipt", 1));
                player.getInventory().addMaterial(MaterialCatalog::createById("municipal_proof_letter", 1));
                lines = {"Tu aides à bloquer les fissures les plus dangereuses avant qu'elles ne deviennent une nouvelle crise.", "Réparations réduites de 2 jours. Reçu d'aide x1 et attestation municipale x1."};
                showLocalServiceResult("MURS CONSOLIDÉS", "shop.city.repair.order.walls.success", player, lines, 1);
            }
            else if (choice == 7)
            {
                Random random;
                lines = {
                    "Tu ne fais pas tomber une bourse d'or du ciel : tu organises des voisins, des paniers et des bras disponibles.",
                    "La collecte rapporte surtout des ressources utiles, pas une prime gratuite."
                };
                player.getInventory().addMaterial(MaterialCatalog::createById("rusted_metal_fragment", random.between(1, 2)));
                player.getInventory().addMaterial(MaterialCatalog::createById("worn_leather_piece", 1));
                if (random.between(1, 100) <= 40)
                {
                    player.getInventory().addMaterial(MaterialCatalog::createById("bitter_healing_leaf", 1));
                    lines.push_back("Une vieille voisine ajoute une feuille de soin en disant que les ouvriers font n'importe quoi avec leurs mains.");
                }
                if (random.between(1, 100) <= 25)
                {
                    reduceCityRepairDays(player, 1);
                    lines.push_back("La bonne organisation évite une demi-journée perdue : réparations réduites de 1 jour.");
                }
                player.getInventory().addMaterial(MaterialCatalog::createById("city_service_stamp", 1));
                showLocalServiceResult("COLLECTE ORGANISÉE", "shop.city.repair.order.collection.success", player, lines, 1);
            }
            else if (choice == 8)
            {
                if (missing("route_toll_receipt", 1) || missing("rusted_metal_fragment", 2))
                {
                    showShopResult("MATÉRIAUX MANQUANTS", "shop.city.repair.order.relay_road.failed", {"Il faut Reçu de péage de route x1 et Fragment de métal rouillé x2.", "Le relais ne peut pas rouvrir proprement si la route devant lui ressemble à une mâchoire cassée."});
                    continue;
                }
                consume("route_toll_receipt", 1);
                consume("rusted_metal_fragment", 2);
                reduceCityRepairDays(player, 1);
                player.getInventory().addMaterial(MaterialCatalog::createById("relay_route_badge", 1));
                player.getInventory().addMaterial(MaterialCatalog::createById("local_reputation_note", 1));
                lines = {"Tu aides le relais à remettre un passage praticable sans transformer ça en grande expédition.", "Réparations réduites de 1 jour. Badge de route du relais x1, note de réputation locale x1."};
                showLocalServiceResult("ROUTE DU RELAIS RÉPARÉE", "shop.city.repair.order.relay_road.success", player, lines, 1);
            }
            else if (choice == 9)
            {
                if (missing("survival_ration", 3) || missing("bitter_healing_leaf", 1))
                {
                    showShopResult("RÉSERVE INCOMPLÈTE", "shop.city.repair.order.reserve.failed", {"Il faut Ration de survie x3 et Feuille amère de soin x1.", "Une réserve de secours vide rassure seulement les affiches, pas les gens."});
                    continue;
                }
                consume("survival_ration", 3);
                consume("bitter_healing_leaf", 1);
                reduceCityRepairDays(player, 1);
                player.getInventory().addMaterial(MaterialCatalog::createById("city_repair_receipt", 1));
                player.getInventory().addMaterial(MaterialCatalog::createById("municipal_proof_letter", 1));
                lines = {"Tu montes une petite réserve pour éviter que le chantier s'arrête à la première toux ou au premier repas raté.", "Réparations réduites de 1 jour. Reçu d'aide x1, attestation municipale x1."};
                showLocalServiceResult("RÉSERVE DE SECOURS PRÊTE", "shop.city.repair.order.reserve.success", player, lines, 1);
            }
            else if (choice == 10)
            {
                if (missing("local_service_letter", 1) || missing("city_service_stamp", 1))
                {
                    showShopResult("DOSSIER INCOMPLET", "shop.city.repair.order.quest_board.failed", {"Il faut Lettre de service local x1 et Tampon de service municipal x1.", "Le panneau des quêtes ne repart pas avec juste un clou : il faut aussi des demandes vérifiées."});
                    continue;
                }
                consume("local_service_letter", 1);
                consume("city_service_stamp", 1);
                reduceCityRepairDays(player, 1);
                player.getInventory().addMaterial(MaterialCatalog::createById("guild_favor_token", 1));
                player.getInventory().addMaterial(MaterialCatalog::createById("client_recommendation", 1));
                lines = {"Tu aides à trier les demandes urgentes, retirer les fausses primes et remettre une affiche propre devant la guilde.", "Réparations réduites de 1 jour. Jeton de faveur de guilde x1, recommandation de client x1."};
                showLocalServiceResult("PANNEAU DES QUÊTES ROUVERT", "shop.city.repair.order.quest_board.success", player, lines, 1);
            }
            else if (choice == 11)
            {
                Random random;
                lines = {
                    "Tu dégages une rue où tout le monde disait 'on verra demain' depuis trop longtemps.",
                    "Le gain reste modeste : la ville récupère surtout un passage utilisable."
                };
                player.getInventory().addMaterial(MaterialCatalog::createById("rusted_metal_fragment", random.between(1, 2)));
                if (random.between(1, 100) <= 55)
                {
                    player.getInventory().addMaterial(MaterialCatalog::createById("worn_leather_piece", 1));
                    lines.push_back("Tu récupères aussi quelques sangles abîmées sous les débris.");
                }
                if (random.between(1, 100) <= 30)
                {
                    reduceCityRepairDays(player, 1);
                    lines.push_back("La rue rouverte évite un détour de chantier : réparations réduites de 1 jour.");
                }
                player.getInventory().addMaterial(MaterialCatalog::createById("city_service_stamp", 1));
                showLocalServiceResult("RUE DÉGAGÉE", "shop.city.repair.order.clear_street.success", player, lines, 1);
            }
            else if (choice == 12)
            {
                if (missing("weak_repair_kit", 1) || missing("rusted_metal_fragment", 1))
                {
                    showShopResult("MATÉRIEL MANQUANT", "shop.city.repair.order.storage.failed", {"Il faut Kit de réparation faible x1 et Fragment de métal rouillé x1.", "L'entrepôt n'a pas besoin d'un miracle : il faut juste éviter que les stocks tombent sous la pluie ou les voleurs."});
                    continue;
                }
                consume("weak_repair_kit", 1);
                consume("rusted_metal_fragment", 1);
                reduceCityRepairDays(player, 1);
                player.getInventory().addMaterial(MaterialCatalog::createById("city_repair_receipt", 1));
                player.getInventory().addMaterial(MaterialCatalog::createById("client_recommendation", 1));
                lines = {"Tu renforces une porte, poses deux cales, et le responsable arrête de surveiller ses sacs comme s'ils allaient fuir.", "Réparations réduites de 1 jour. Reçu d'aide x1, recommandation de client x1."};
                showLocalServiceResult("ENTREPÔT SÉCURISÉ", "shop.city.repair.order.storage.success", player, lines, 1);
            }
            else if (choice == 13)
            {
                Random random;
                lines = {
                    "Le guichet ne demande pas de casser des dents : seulement de comparer les affiches, les stocks et les excuses des vendeurs pressés.",
                    "Objectif : limiter les abus de crise sans transformer chaque achat en débat politique."
                };
                switch (player.getRace())
                {
                    case CharacterRace::SemiFox:
                    case CharacterRace::Kitsune:
                        lines.push_back("Ton flair pour les clauses tordues aide à repérer deux faux frais cachés.");
                        break;
                    case CharacterRace::Gnome:
                    case CharacterRace::Dwarf:
                        lines.push_back("Tes mesures et tes comptes rapides rendent les prix gonflés plus difficiles à défendre.");
                        break;
                    default:
                        lines.push_back("Tu aides surtout à vérifier que les prix affichés correspondent encore aux stocks réels.");
                        break;
                }
                if (random.between(1, 100) <= 35)
                {
                    reduceCityRepairDays(player, 1);
                    lines.push_back("Quelques abus stoppés évitent une file de colère devant le bureau : réparations réduites de 1 jour.");
                }
                player.getInventory().addMaterial(MaterialCatalog::createById("municipal_proof_letter", 1));
                player.getInventory().addMaterial(MaterialCatalog::createById("local_reputation_note", 1));
                showLocalServiceResult("PRIX CONTRÔLÉS", "shop.city.repair.order.price_check.success", player, lines, 1);
            }
            else if (choice == 14)
            {
                if (missing("survival_ration", 2) || missing("weak_repair_kit", 1))
                {
                    showShopResult("DÉPÔT NON PROTÉGÉ", "shop.city.repair.order.ration_depot.failed", {"Il faut Ration de survie x2 et Kit de réparation faible x1.", "Le dépôt n'a pas besoin d'un champion, juste de caisses qui ferment et de réserves qui ne disparaissent pas."});
                    continue;
                }
                consume("survival_ration", 2);
                consume("weak_repair_kit", 1);
                reduceCityRepairDays(player, 1);
                player.getInventory().addMaterial(MaterialCatalog::createById("city_repair_receipt", 1));
                player.getInventory().addMaterial(MaterialCatalog::createById("local_service_letter", 1));
                lines = {"Tu sécurises les réserves sans transformer ça en trésor caché : moins de gaspillage, moins de disputes, plus de travail possible.", "Réparations réduites de 1 jour. Reçu d'aide x1, lettre de service local x1."};
                showLocalServiceResult("DÉPÔT PROTÉGÉ", "shop.city.repair.order.ration_depot.success", player, lines, 1);
            }
            else if (choice == 15)
            {
                Random random;
                lines = {"La guilde ne promet pas une aventure héroïque : juste une ronde où il faut rester éveillé, regarder les bonnes portes et ne pas paniquer au moindre bruit."};
                switch (player.getRace())
                {
                    case CharacterRace::SemiDog:
                    case CharacterRace::SemiWolf:
                        lines.push_back("Ton flair rend la ronde plus propre : une odeur de passage récent évite une fausse alerte.");
                        player.getInventory().addMaterial(MaterialCatalog::createById("advanced_monster_notes", 1));
                        break;
                    case CharacterRace::SemiBird:
                        lines.push_back("Depuis les hauteurs, tu signales un détour dangereux avant qu'un chariot s'y coince.");
                        player.getInventory().addMaterial(MaterialCatalog::createById("route_scout_note", 1));
                        break;
                    case CharacterRace::SemiCat:
                        lines.push_back("Tu passes par deux toits et un grenier que personne n'avait pensé à vérifier.");
                        player.getInventory().addMaterial(MaterialCatalog::createById("city_service_stamp", 1));
                        break;
                    default:
                        lines.push_back("Tu aides surtout à tenir la présence visible dont les quartiers ont besoin pendant la crise.");
                        player.getInventory().addMaterial(MaterialCatalog::createById("city_service_stamp", 1));
                        break;
                }
                if (random.between(1, 100) <= 30)
                {
                    reduceCityRepairDays(player, 1);
                    lines.push_back("La ronde évite un nouveau retard au chantier : réparations réduites de 1 jour.");
                }
                player.getInventory().addMaterial(MaterialCatalog::createById("local_reputation_note", 1));
                showLocalServiceResult("RONDE DE GUILDE", "shop.city.repair.order.guild_round.success", player, lines, 1);
            }
        }
    }

    void openCityServiceSpecialMenu(Player& player)
    {
        bool stay = true;
        while (stay)
        {
            const int repairDays = cityRepairDaysRemaining(player);
            MenuScreen screen(repairDays > 0 ? "BUREAU DES RÉPARATIONS" : "BUREAU DE VILLE", "shop.city.special");
            screen.addLine("Temps actuel : " + player.formatWorldDateTimeLine());
            if (repairDays > 0)
            {
                screen.addLine("État : réparations en cours pendant encore " + std::to_string(repairDays) + " jour(s).");
                screen.addLine("La ville priorise les gardes, les prêtres, l'auberge et quelques comptoirs tirés au sort selon les besoins du jour.");
                screen.addLine("Demandes du moment : réparer, garder les rues, récolter du cuir/bois/métal et remettre les échoppes debout.");
            }
            else
            {
                const CityEventOfDay event = cityEventForTodayOrNext(player);
                const int recentEventDays = player.getInventory().countMaterialById("city_event_recent_days_marker");
                screen.addLine("Le bureau affiche les événements de guilde et de village : tournoi, foire, journée du savoir, bourse aux missions, cérémonie, jeux, moissons.");
                screen.addLine("Rythme : environ un événement régulier par semaine. Certaines semaines peuvent avoir un second événement rare, mais sa chance tient compte de l'affiche régulière, des autres événements et des tensions de la ville.");
                if (recentEventDays > 0)
                {
                    screen.addLine("Organisation : la ville vient déjà de gérer une affiche. Un second événement royal est moins probable pendant encore " + std::to_string(recentEventDays) + " jour(s).");
                }
                if (hasAnyCityEventToday(player))
                {
                    screen.addLine(std::string(isRoyalBonusCityEvent(event) ? "Affiche exceptionnelle aujourd'hui : " : "Affiche régulière aujourd'hui : ") + event.name + " — " + event.action + ".");
                }
                else
                {
                    screen.addLine("Aucun événement actif aujourd'hui. Prochaine affiche dans " + std::to_string(daysUntilNextAnyCityEvent(player)) + " jour(s) : " + event.name + ".");
                }
                screen.addLine("Les vrais malheurs de ville restent très rares, mais ils peuvent bloquer l'économie locale si personne ne tient la ligne.");
            }
            screen.addOption(0, "Retour", "Revenir au comptoir municipal.", true, "shop.city.special.back");
            if (repairDays > 0)
            {
                screen.addOption(1, "Apporter des matériaux de réparation", "Demande : Fragment de métal rouillé x3 + Morceau de cuir abîmé x2. Réduit les réparations.", true, "shop.city.repair.materials");
                screen.addOption(2, "Aider à récolter des ressources", "Sortie courte encadrée : récupère surtout des matériaux simples pour les réparations.", true, "shop.city.repair.gather");
                screen.addOption(3, "Aider la garde autour des boutiques ouvertes", "Patrouille courte : protège les rares comptoirs actifs et améliore un peu la confiance locale.", true, "shop.city.repair.guard");
                screen.addOption(5, "Choisir une demande précise de réparation", "Infirmerie, marché, rations, faille mineure : petites tâches utiles et lisibles.", true, "shop.city.repair.orders");
            }
            else
            {
                screen.addOption(1, "Lire le calendrier des événements", "Affiche les événements réguliers inspirés des fiches : guilde, foire, savoir, missions, moissons.", true, "shop.city.events.calendar");
                screen.addOption(2, "Demander les risques du moment", "Rumeurs sur invasion, incendie, épidémie, faille magique, disparition ou crise économique.", true, "shop.city.events.risks");
                screen.addOption(3,
                    hasAnyCityEventToday(player) ? "Participer à l'événement actif" : "Aucun événement actif aujourd'hui",
                    hasAnyCityEventToday(player) ? "Activité courte : gain modeste, document, ressource ou réputation selon l'événement." : "Les affiches ne sont pas disponibles tous les jours.",
                    hasAnyCityEventToday(player),
                    "shop.city.events.participate");
            }
            screen.addOption(4, "Cotisations de ville", "Abonnements locaux : guilde, commerce, route. Conservé ici pour ne pas perdre l'ancien service.", true, "shop.city.subscriptions");
            screen.addOption(6, "Lire l'économie locale", "Résumé des besoins actuels, crise éventuelle et réaction légère selon la race.", true, "shop.city.economy.report");
            screen.addOption(7, "Lire le panneau ville / guilde", "Contexte local : prochaine affiche, tensions, pistes selon race. Aucun gain direct.", true, "shop.city.notice_board");

            Console::clear();
            const int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choisis une action de ville.");
            Console::clear();

            if (choice == 0)
            {
                stay = false;
                continue;
            }

            if (choice == 4)
            {
                openSubscriptionMenu(
                    player,
                    "COTISATIONS DE VILLE",
                    {
                        {"guild_adventurer_standard_weekly", "Cotisation aventurier standard", 160, "Petits services de guilde, salle commune et paperasse locale couverts pendant 7 jours."},
                        {"merchant_cotisation_weekly", "Cotisation de la confrérie du commerce", 210, "Meilleurs papiers marchands, contrôles plus propres et aide de guichet pendant 7 jours."},
                        {"trade_route_weekly", "Pass de commerce hebdomadaire", 240, "Préparatifs de routes, caravanes et contrôles marchands simples pendant 7 jours."}
                    }
                );
                continue;
            }

            if (choice == 6)
            {
                showLocalEconomyReport(player);
                continue;
            }

            if (choice == 7)
            {
                showCityAndGuildNoticeBoard(player);
                continue;
            }

            if (repairDays > 0 && choice == 5)
            {
                openCityRepairWorkOrderMenu(player);
                continue;
            }

            if (repairDays > 0 && choice == 1)
            {
                std::vector<std::string> lines;
                if (player.getInventory().countMaterialById("rusted_metal_fragment") < 3
                    || player.getInventory().countMaterialById("worn_leather_piece") < 2)
                {
                    lines.push_back("Matériaux insuffisants : il faut Fragment de métal rouillé x3 et Morceau de cuir abîmé x2.");
                    lines.push_back("La ville ne demande pas de prix en or ici : elle veut des ressources utiles aux réparations.");
                    showShopResult("AIDE IMPOSSIBLE", "shop.city.repair.materials.failed", lines);
                    continue;
                }
                player.getInventory().removeMaterialQuantityById("rusted_metal_fragment", 3);
                player.getInventory().removeMaterialQuantityById("worn_leather_piece", 2);
                reduceCityRepairDays(player, 2);
                player.getInventory().addMaterial(MaterialCatalog::createById("city_repair_receipt", 1));
                player.getInventory().addMaterial(MaterialCatalog::createById("local_reputation_note", 1));
                lines.push_back("Tu remets de quoi clouer, sangler, rafistoler et sécuriser plusieurs devantures.");
                lines.push_back("Réparations réduites de 2 jour(s), sans transformer ça en quête infinie.");
                lines.push_back("Reçu obtenu : aide aux réparations x1. Note de réputation locale x1.");
                showLocalServiceResult("RÉPARATIONS AIDÉES", "shop.city.repair.materials.success", player, lines, 1);
            }
            else if (repairDays > 0 && choice == 2)
            {
                Random random;
                std::vector<std::string> lines = {
                    "Un contremaître te donne une liste simple : récupérer ce qui peut servir sans mourir pour une planche.",
                    "Ce n'est pas une grande aventure, mais une ville se relève avec ce genre de petites corvées."
                };
                player.getInventory().addMaterial(MaterialCatalog::createById("rusted_metal_fragment", random.between(1, 3)));
                player.getInventory().addMaterial(MaterialCatalog::createById("worn_leather_piece", random.between(1, 2)));
                if (random.between(1, 100) <= 35)
                {
                    player.getInventory().addMaterial(MaterialCatalog::createById("weak_repair_kit", 1));
                    lines.push_back("Un artisan te laisse un kit de réparation faible, cabossé mais utilisable.");
                }
                lines.push_back("Matériaux simples obtenus pour aider la reconstruction ou tes propres crafts.");
                showLocalServiceResult("RÉCOLTE DE CHANTIER", "shop.city.repair.gather.success", player, lines, 1);
            }
            else if (repairDays > 0 && choice == 3)
            {
                std::vector<std::string> lines = {
                    "Tu ne sauves pas la ville à toi seul, mais ta présence évite quelques vols et panique autour des rares comptoirs ouverts.",
                    "Les marchands notent ton nom pour une bonne raison cette fois."
                };
                player.getInventory().addMaterial(MaterialCatalog::createById("city_repair_receipt", 1));
                player.getInventory().addMaterial(MaterialCatalog::createById("city_service_stamp", 1));
                player.getInventory().earnGold(18 + player.getLevel());
                if (cityRepairDaysRemaining(player) <= 2)
                {
                    reduceCityRepairDays(player, 1);
                    lines.push_back("La surveillance accélère la réouverture d'un secteur : réparations réduites de 1 jour.");
                }
                lines.push_back("Récompense de patrouille : " + Money::formatGoldWithRaw(18 + player.getLevel()) + ".");
                showLocalServiceResult("PATROUILLE DE RÉPARATION", "shop.city.repair.guard.success", player, lines, 1);
            }
            else if (repairDays <= 0 && choice == 1)
            {
                const CityEventOfDay event = cityEventForTodayOrNext(player);
                std::vector<std::string> calendarLines = {
                    "Événements réguliers possibles : Tournoi de guilde, Chasse aux monstres, Grande foire marchande, Journée du savoir.",
                    "Autres affiches : Bourse aux missions, Cérémonie d'honneur, Jeux du village, Fête des moissons / solstice.",
                    "Rythme corrigé : environ un événement régulier par semaine. Un second événement exceptionnel peut apparaître, mais sa chance baisse aussi si d'autres événements viennent déjà d'occuper la ville.",
                    "Sécurité anti-farm : la présence d'un événement est déterminée par le calendrier, pas en rouvrant le menu en boucle.",
                    royalBonusChanceContextLine(player, safeCityDay(player))
                };
                if (hasAnyCityEventToday(player))
                {
                    calendarLines.push_back(std::string("Aujourd'hui : ") + (isRoyalBonusCityEvent(event) ? "affiche exceptionnelle — " : "affiche régulière — ") + event.name + " — " + event.mood);
                }
                else
                {
                    calendarLines.push_back("Aujourd'hui : aucune affiche active. Prochaine affiche dans " + std::to_string(daysUntilNextAnyCityEvent(player)) + " jour(s) : " + event.name + ".");
                }
                calendarLines.push_back("Villes douteuses : un Festival de l'Ombre peut exister, mais plutôt comme événement rare et dangereux.");
                showShopResult(
                    "CALENDRIER DE VILLE",
                    "shop.city.events.calendar",
                    calendarLines
                );
            }
            else if (repairDays <= 0 && choice == 3)
            {
                participateInCityEventOfDay(player);
            }
            else if (repairDays <= 0 && choice == 2)
            {
                std::vector<std::string> riskLines = {
                    "Malheurs rares possibles : invasion de monstres, épidémie ou malédiction, incendie, faille magique, disparition d'un notable, secte, guerre de guildes, crise économique.",
                    "Si une défense de ville échoue, la conséquence n'est pas juste du texte : plusieurs boutiques ferment pendant les réparations.",
                    "Pendant ce temps, les demandes locales se concentrent sur réparation, garde, récolte et remise en route des commerces.",
                    royalBonusChanceContextLine(player, safeCityDay(player)),
                    "Remise de défense : si la ville est sauvée, les commerçants remercient un peu, mais jamais plus de 10 jours."
                };
                const int recentEventDays = player.getInventory().countMaterialById("city_event_recent_days_marker");
                if (recentEventDays > 0)
                {
                    riskLines.push_back("Organisation : événement récent encore présent pendant " + std::to_string(recentEventDays) + " jour(s). Le bureau évite d'empiler les animations.");
                }
                showShopResult("RISQUES DU MOMENT", "shop.city.events.risks", riskLines);
            }
        }
    }

    void openLodgingServiceMenu(Player& player)
    {
        bool stayInServices = true;

        while (stayInServices)
        {
            MenuScreen screen("AUBERGE DU REPOS BRUYANT", "shop.lodging.services");
            screen.addLine("Tavia propose des services utiles, mais jamais gratuits en temps : manger ou dormir peut faire avancer les délais.");
            screen.addLine("PV : " + std::to_string(player.getHp()) + "/" + std::to_string(player.getMaxHp()));
            screen.addLine("Temps : " + worldTimeLineForPlayer(player));
            if (subscriptionCoversService(player, "lodging"))
            {
                screen.addLine("Abonnement : repas/chambre simple couverts par la période active.");
            }
            if (subscriptionCoversService(player, "stable"))
            {
                screen.addLine("Abonnement : écurie/relais couvert par la période active.");
            }
            screen.addLine(serviceCostLine(player, "warm_meal_voucher", "Bon de repas chaud", 12));
            screen.addLine(serviceCostLine(player, "lodging_bed_token", "Bon de lit d'auberge", 24));
            screen.addLine(serviceCostLine(player, "stable_stall_ticket", "Ticket d'écurie", 30));
            screen.addLine(serviceCostLine(player, "rental_mount_voucher", "Bon de monture", 65));
            screen.addLine(serviceCostLine(player, "stable_box_reservation", "Réservation de box", 44));
            if (player.getInventory().countMaterialById("owned_mount_registration") > 0)
            {
                const bool hasReinforcedSaddle = player.getInventory().countMaterialById("stable_saddle_upgrade") > 0;
                const bool hasMinorInjury = player.getInventory().countMaterialById("mount_minor_injury_marker") > 0;
                const int fatigueLimit = hasReinforcedSaddle ? 4 : 3;
                const int bond = std::min(3, player.getInventory().countMaterialById("mount_bond_marker"));
                const bool hasAnyAdvancedMountCare = player.getInventory().countMaterialById("mount_comfort_bridle") > 0
                    || player.getInventory().countMaterialById("mount_weather_blanket") > 0
                    || player.getInventory().countMaterialById("mount_pack_harness") > 0
                    || player.getInventory().countMaterialById("mount_road_shoes") > 0
                    || player.getInventory().countMaterialById("mount_route_memory_marker") > 0
                    || player.getInventory().countMaterialById("mount_surefoot_training_marker") > 0;
                screen.addLine("Monture personnelle : enregistrée | fatigue " + std::to_string(player.getInventory().countMaterialById("mount_fatigue_marker")) + "/" + std::to_string(fatigueLimit)
                    + " | lien " + std::to_string(bond) + "/3"
                    + (hasReinforcedSaddle ? " | selle renforcée" : "")
                    + (hasAnyAdvancedMountCare ? " | entretien/équipement avancé passif" : "")
                    + (hasMinorInjury ? " | blessure légère" : ""));
            }
            else
            {
                screen.addLine("Monture personnelle : aucune. Une monture durable coûte cher, mais évite de relouer à chaque long trajet.");
            }
            screen.addOption(0, "Retour", "Revenir au comptoir de l'auberge.", true, "shop.lodging.back");
            screen.addOption(1, "Manger un repas chaud — 12 cuivre", "Soin léger, consomme 1 segment de journée. Prix affiché avant validation.", true, "shop.lodging.meal");
            screen.addOption(2, "Dormir dans une chambre simple — 24 cuivre", "Récupération complète des PV, consomme 2 segments de journée. Prix affiché avant validation.", true, "shop.lodging.sleep");
            screen.addOption(3, "Écouter les rumeurs de comptoir", "Indice de ville sans récompense directe, consomme 1 segment.", true, "shop.lodging.rumors");
            screen.addOption(4, "Préparer une place d'écurie — 30 cuivre", "Stabilise monture, sacoches ou stockage court pour les quêtes de relais.", true, "shop.lodging.stable");
            screen.addOption(5, "Préparer sacoches et charge — 32 cuivre", "Préparation utile pour réduire un déplacement de biome plus tard.", true, "shop.lodging.saddlebags");
            screen.addOption(6, "Déposer une charge à l'écurie — 18 cuivre", "Dépôt temporaire utile pour certains services de ville/relais.", true, "shop.lodging.storage");
            screen.addOption(7, "Louer une monture de route — 65 cuivre", "Préparation plus forte pour les distances longues, consomme 1 segment.", true, "shop.lodging.mount");
            screen.addOption(8, "Réserver un box sécurisé — 44 cuivre", "Stockage/box plus sérieux, utile pour quêtes d'écurie et relais.", true, "shop.lodging.box");
            screen.addOption(9, "Abonnements de l'auberge", "Forfaits de 7 jours : repas, lit simple, écurie ou cotisation de guilde.", true, "shop.lodging.subscriptions");
            screen.addOption(10, "Enregistrer une monture personnelle — 420 cuivre", "Achat/acte durable : utile sur plusieurs trajets, mais la monture peut fatiguer.", true, "shop.lodging.owned_mount");
            screen.addOption(11, "Soin et repos de monture — 62 cuivre", "Retire la fatigue accumulée par une monture personnelle.", true, "shop.lodging.mount_rest");
            screen.addOption(12, "Examiner la monture personnelle", "Affiche son état, sa limite de fatigue et les conseils d'écurie sans consommer de temps.", player.getInventory().countMaterialById("owned_mount_registration") > 0, "shop.lodging.mount_check");
            if (player.getInventory().countMaterialById("owned_mount_registration") > 0)
            {
                screen.addLine("Gestion simplifiée : les réglages avancés de monture restent passifs dans l'inventaire, mais ne prennent plus autant de place dans le menu.");
                screen.addLine("Actions conservées : enregistrer, examiner, reposer/soigner. Pas besoin de jouer à Équitation Simulator pour profiter du bonus de route.");
            }
            Console::clear();
            const int choice = TerminalInterface::askMenuChoiceFromOptions(
                screen,
                "Choisis un service d'auberge, ou 0 pour revenir."
            );

            if (choice == 0)
            {
                stayInServices = false;
                continue;
            }

            if (choice == 1)
            {
                std::vector<std::string> lines;
                if (!payServiceWithSubscriptionVoucherOrGold(player, "lodging", "warm_meal_voucher", "Bon de repas chaud", 12, lines))
                {
                    showShopResult("REPAS REFUSÉ", "shop.lodging.meal.failed", lines);
                    continue;
                }

                const int beforeHp = player.getHp();
                const int healAmount = std::max(1, player.getMaxHp() / 5);
                player.heal(healAmount);
                lines.push_back("Repas pris : soupe chaude, pain correct et aucune assiette maudite repérée.");
                lines.push_back("PV récupérés : " + std::to_string(player.getHp() - beforeHp)
                    + " (" + std::to_string(beforeHp) + " -> " + std::to_string(player.getHp()) + ").");
                showLocalServiceResult("REPAS TERMINÉ", "shop.lodging.meal.success", player, lines, 1);
            }
            else if (choice == 2)
            {
                std::vector<std::string> lines;
                if (!payServiceWithSubscriptionVoucherOrGold(player, "lodging", "lodging_bed_token", "Bon de lit d'auberge", 24, lines))
                {
                    showShopResult("NUIT REFUSÉE", "shop.lodging.sleep.failed", lines);
                    continue;
                }

                const int beforeHp = player.getHp();
                player.heal(player.getMaxHp());
                lines.push_back("Repos : chambre simple, couverture honnête et porte qui ferme presque bien.");
                lines.push_back("PV récupérés : " + std::to_string(player.getHp() - beforeHp)
                    + " (" + std::to_string(beforeHp) + " -> " + std::to_string(player.getHp()) + ").");
                lines.push_back("Rappel : dormir peut faire échouer les quêtes urgentes si la date limite passe pendant la nuit.");
                showLocalServiceResult("NUIT TERMINÉE", "shop.lodging.sleep.success", player, lines, 2);
            }
            else if (choice == 3)
            {
                std::vector<std::string> lines;
                lines.push_back("Tavia parle bas : les bons d'auberge, tickets d'écurie et reçus de route peuvent compter plus qu'une petite prime.");
                lines.push_back("Elle conseille de garder au moins une preuve de service quand une quête mentionne relais, groupe, chambre ou départ à l'aube.");
                lines.push_back("Aucune récompense directe : tu gagnes surtout une piste propre, pas un objet gratuit.");
                showLocalServiceResult("RUMEURS DE COMPTOIR", "shop.lodging.rumors", player, lines, 1);
            }
            else if (choice == 4)
            {
                std::vector<std::string> lines;
                if (!payServiceWithSubscriptionVoucherOrGold(player, "stable", "stable_stall_ticket", "Ticket d'écurie", 30, lines))
                {
                    showShopResult("ÉCURIE REFUSÉE", "shop.lodging.stable.failed", lines);
                    continue;
                }

                player.getInventory().addMaterial(MaterialCatalog::createById("travel_pass_note", 1));
                player.getInventory().addMaterial(MaterialCatalog::createById("travel_distance_mark", 1));
                lines.push_back("Écurie préparée : box réservé, sacoches notées, départ un peu moins chaotique.");
                lines.push_back("Preuves obtenues : Note de pass de voyage x1, Marque de distance de trajet x1.");
                lines.push_back("Usage prévu : demandes de Noro, caravanes, contrôles de relais ou quêtes qui exigent une préparation crédible.");
                showLocalServiceResult("ÉCURIE PRÉPARÉE", "shop.lodging.stable.success", player, lines, 1);
            }
            else if (choice == 5)
            {
                std::vector<std::string> lines;
                if (!payServiceWithSubscriptionVoucherOrGold(player, "stable", "stable_stall_ticket", "Ticket d'écurie", 32, lines))
                {
                    showShopResult("SACOCHES REFUSÉES", "shop.lodging.saddlebags.failed", lines);
                    continue;
                }

                player.getInventory().addMaterial(MaterialCatalog::createById("prepared_saddlebags", 1));
                lines.push_back("Sacoches préparées : charge répartie, sangles vérifiées, rien ne pendouille au mauvais endroit.");
                lines.push_back("Preuve obtenue : Sacoches préparées x1.");
                lines.push_back("Usage prévu : réduit un déplacement d'exploration/biome plus tard, surtout quand la distance compte.");
                showLocalServiceResult("SACOCHES PRÉPARÉES", "shop.lodging.saddlebags.success", player, lines, 1);
            }
            else if (choice == 6)
            {
                std::vector<std::string> lines;
                if (!payServiceWithSubscriptionVoucherOrGold(player, "stable", "stable_stall_ticket", "Ticket d'écurie", 18, lines))
                {
                    showShopResult("DÉPÔT REFUSÉ", "shop.lodging.storage.failed", lines);
                    continue;
                }

                player.getInventory().addMaterial(MaterialCatalog::createById("temporary_stable_storage", 1));
                lines.push_back("Dépôt enregistré : la charge est gardée au sec, loin des bols de soupe et des bardes.");
                lines.push_back("Preuve obtenue : Dépôt temporaire d'écurie x1.");
                lines.push_back("Usage prévu : justificatif pour demandes de route, auberge, stockage ou préparation de convoi.");
                showLocalServiceResult("DÉPÔT D'ÉCURIE", "shop.lodging.storage.success", player, lines, 1);
            }
            else if (choice == 7)
            {
                std::vector<std::string> lines;
                if (!payServiceWithSubscriptionVoucherOrGold(player, "stable", "rental_mount_voucher", "Bon de monture", 65, lines))
                {
                    showShopResult("MONTURE REFUSÉE", "shop.lodging.mount.failed", lines);
                    continue;
                }

                player.getInventory().addMaterial(MaterialCatalog::createById("rental_mount_voucher", 1));
                player.getInventory().addMaterial(MaterialCatalog::createById("loaded_pack_saddle", 1));
                lines.push_back("Monture louée : animal nourri, bride vérifiée, selle chargée sans angle idiot.");
                lines.push_back("Preuves obtenues : Bon de monture de location x1, Selle de bât chargée x1.");
                lines.push_back("Usage prévu : fortes réductions de déplacement sur biomes vastes/lointains, surtout en exploration longue.");
                showLocalServiceResult("MONTURE PRÊTE", "shop.lodging.mount.success", player, lines, 1);
            }
            else if (choice == 8)
            {
                std::vector<std::string> lines;
                if (!payServiceWithSubscriptionVoucherOrGold(player, "stable", "stable_box_reservation", "Réservation de box", 44, lines))
                {
                    showShopResult("BOX REFUSÉ", "shop.lodging.box.failed", lines);
                    continue;
                }

                player.getInventory().addMaterial(MaterialCatalog::createById("stable_box_reservation", 1));
                player.getInventory().addMaterial(MaterialCatalog::createById("temporary_stable_storage", 1));
                lines.push_back("Box réservé : une place propre, une serrure honnête et une note claire dans le registre.");
                lines.push_back("Preuves obtenues : Réservation de box sécurisé x1, Dépôt temporaire d'écurie x1.");
                lines.push_back("Usage prévu : quêtes de relais, stockage, cargaison légère ou départ reporté.");
                showLocalServiceResult("BOX RÉSERVÉ", "shop.lodging.box.success", player, lines, 1);
            }
            else if (choice == 10)
            {
                std::vector<std::string> lines;
                if (player.getInventory().countMaterialById("owned_mount_registration") > 0)
                {
                    lines.push_back("Tu possèdes déjà une monture personnelle enregistrée.");
                    lines.push_back("Elle peut aider plusieurs longs trajets, mais elle accumule de la fatigue et doit parfois se reposer.");
                    showShopResult("MONTURE DÉJÀ ENREGISTRÉE", "shop.lodging.owned_mount.already", lines);
                    continue;
                }

                const int price = 240;
                if (!player.getInventory().spendGold(price))
                {
                    lines.push_back("Paiement refusé : il faut " + Money::formatGoldWithRaw(price) + ".");
                    lines.push_back("Argent disponible : " + Money::formatCurrencyOverviewFromCopper(player.getInventory().getTotalCopper()) + ".");
                    showShopResult("MONTURE REFUSÉE", "shop.lodging.owned_mount.failed", lines);
                    continue;
                }

                player.getInventory().addMaterial(MaterialCatalog::createById("owned_mount_registration", 1));
                player.getInventory().addMaterial(MaterialCatalog::createById("stable_saddle_upgrade", 1));
                player.getInventory().addMaterial(MaterialCatalog::createById("mount_temperament_calm", 1));
                lines.push_back("Monture personnelle enregistrée : l'écurie connaît maintenant ton animal, ses habitudes et son box de base.");
                lines.push_back("Preuves obtenues : Acte de monture personnelle x1, Selle renforcée de route x1, Tempérament de monture calme x1.");
                lines.push_back("Règle : la monture aide durablement les longs trajets, mais accumule de la fatigue après usage.");
                showLocalServiceResult("MONTURE ENREGISTRÉE", "shop.lodging.owned_mount.success", player, lines, 1);
            }
            else if (choice == 11)
            {
                std::vector<std::string> lines;
                const int fatigue = player.getInventory().countMaterialById("mount_fatigue_marker");
                const bool hasReinforcedSaddle = player.getInventory().countMaterialById("stable_saddle_upgrade") > 0;
                const int fatigueLimit = hasReinforcedSaddle ? 4 : 3;
                if (fatigue <= 0)
                {
                    lines.push_back("Aucune fatigue de monture à retirer pour le moment.");
                    lines.push_back("Tavia conseille quand même de garder de quoi payer un repos avant les grandes routes.");
                    showShopResult("MONTURE REPOSÉE", "shop.lodging.mount_rest.none", lines);
                    continue;
                }

                if (!payServiceWithSubscriptionVoucherOrGold(player, "stable", "mount_rest_care", "Soin et repos de monture", 45, lines))
                {
                    showShopResult("REPOS REFUSÉ", "shop.lodging.mount_rest.failed", lines);
                    continue;
                }

                player.getInventory().removeMaterialQuantityById("mount_fatigue_marker", fatigue);
                lines.push_back("Repos de monture : nourriture, eau, pansage, vérification des fers et vrai silence loin des caravanes.");
                lines.push_back("Fatigue retirée : " + std::to_string(fatigue) + "/" + std::to_string(fatigueLimit) + ".");
                showLocalServiceResult("MONTURE REPOSÉE", "shop.lodging.mount_rest.success", player, lines, 1);
            }
            else if (choice == 12)
            {
                std::vector<std::string> lines;
                const int fatigue = player.getInventory().countMaterialById("mount_fatigue_marker");
                const int bond = std::min(3, player.getInventory().countMaterialById("mount_bond_marker"));
                const bool hasName = player.getInventory().countMaterialById("mount_name_tag") > 0;
                const bool hasTemperament = player.getInventory().countMaterialById("mount_temperament_calm") > 0;
                const bool hasReinforcedSaddle = player.getInventory().countMaterialById("stable_saddle_upgrade") > 0;
                const bool hasComfortBridle = player.getInventory().countMaterialById("mount_comfort_bridle") > 0;
                const bool hasWeatherBlanket = player.getInventory().countMaterialById("mount_weather_blanket") > 0;
                const bool hasPackHarness = player.getInventory().countMaterialById("mount_pack_harness") > 0;
                const bool hasRoadShoes = player.getInventory().countMaterialById("mount_road_shoes") > 0;
                const int surefoot = std::min(2, player.getInventory().countMaterialById("mount_surefoot_training_marker"));
                const int routeMemory = std::min(2, player.getInventory().countMaterialById("mount_route_memory_marker"));
                const bool hasMinorInjury = player.getInventory().countMaterialById("mount_minor_injury_marker") > 0;
                const int fatigueLimit = hasReinforcedSaddle ? 4 : 3;
                lines.push_back("Monture personnelle : enregistrée à l'écurie.");
                lines.push_back(std::string("Nom de terrain : ") + (hasName ? "noté par Tavia." : "pas encore noté."));
                lines.push_back(std::string("Tempérament : ") + (hasTemperament ? "calme, fiable sur route." : "encore mal observé."));
                lines.push_back("Fatigue actuelle : " + std::to_string(fatigue) + "/" + std::to_string(fatigueLimit) + ".");
                lines.push_back("Lien actuel : " + std::to_string(bond) + "/3.");
                lines.push_back(hasReinforcedSaddle ? "Selle renforcée : limite de fatigue augmentée et meilleure réduction sur longues routes." : "Selle renforcée : absente. Une amélioration peut rendre les longues routes plus fiables.");
                lines.push_back(hasComfortBridle ? "Bridon confortable : installé, utile pour les trajets propres et répétés." : "Bridon confortable : absent, les longues routes restent plus rugueuses.");
                lines.push_back(hasWeatherBlanket ? "Couverture météo : installée, utile contre pluie, froid léger et bivouacs." : "Couverture météo : absente, les routes froides ou humides restent moins propres.");
                lines.push_back(hasPackHarness ? "Harnais de bât : ajusté, les petites charges pèsent moins sur les longs trajets." : "Harnais de bât : absent, les sacoches longues restent plus fatigantes.");
                lines.push_back(hasRoadShoes ? "Ferrage de route : installé, utile sur les très longues distances." : "Ferrage de route : absent, mais ce n'est pas indispensable pour jouer normalement.");
                lines.push_back("Mémoire de route : " + std::to_string(routeMemory) + "/2. Assurance : " + std::to_string(surefoot) + "/2.");
                if (hasMinorInjury)
                {
                    lines.push_back("Blessure légère : présente. L'écurie déconseille toute longue sortie avant soin.");
                }
                else if (fatigue >= fatigueLimit)
                {
                    lines.push_back("Conseil de Tavia : repos obligatoire avant de refaire confiance à l'animal sur une longue sortie.");
                }
                else if (fatigue >= fatigueLimit - 1)
                {
                    lines.push_back("Conseil de Tavia : encore utilisable, mais le prochain départ risque de la bloquer.");
                }
                else
                {
                    lines.push_back("Conseil de Tavia : état correct pour un trajet, mais pas pour enchaîner sans repos.");
                }
                showShopResult("ÉTAT DE LA MONTURE", "shop.lodging.mount_check", lines);
            }
            else if (choice == 13)
            {
                std::vector<std::string> lines;
                const int price = 8;
                if (!player.getInventory().spendGold(price))
                {
                    lines.push_back("Paiement refusé : il faut " + Money::formatGoldWithRaw(price) + ".");
                    lines.push_back("Argent disponible : " + Money::formatCurrencyOverviewFromCopper(player.getInventory().getTotalCopper()) + ".");
                    showShopResult("NOM REFUSÉ", "shop.lodging.mount_name.failed", lines);
                    continue;
                }
                player.getInventory().addMaterial(MaterialCatalog::createById("mount_name_tag", 1));
                lines.push_back("Tavia grave un nom de terrain sur une petite plaque d'écurie.");
                lines.push_back("Pas besoin de système de saisie lourd : le jeu retient surtout que la monture est reconnue comme compagnon, pas comme simple location.");
                showShopResult("NOM DE MONTURE NOTÉ", "shop.lodging.mount_name.success", lines);
            }
            else if (choice == 14)
            {
                std::vector<std::string> lines;
                const int bond = std::min(3, player.getInventory().countMaterialById("mount_bond_marker"));
                if (bond >= 3)
                {
                    lines.push_back("Lien déjà au maximum actuel : 3/3.");
                    showShopResult("LIEN STABLE", "shop.lodging.mount_bond.max", lines);
                    continue;
                }
                if (!payServiceWithSubscriptionVoucherOrGold(player, "stable", "mount_grooming_kit", "Kit de pansage de monture", 28, lines))
                {
                    showShopResult("LIEN REFUSÉ", "shop.lodging.mount_bond.failed", lines);
                    continue;
                }
                player.getInventory().addMaterial(MaterialCatalog::createById("mount_bond_marker", 1));
                lines.push_back("Tavia te laisse marcher avec l'animal, brosser les zones sensibles et répéter les arrêts sans le brusquer.");
                lines.push_back("Lien de monture : " + std::to_string(bond + 1) + "/3.");
                showLocalServiceResult("LIEN DE MONTURE", "shop.lodging.mount_bond.success", player, lines, 1);
            }
            else if (choice == 15)
            {
                std::vector<std::string> lines;
                if (player.getInventory().countMaterialById("mount_minor_injury_marker") <= 0)
                {
                    lines.push_back("Aucune blessure légère de monture détectée.");
                    showShopResult("SOIN INUTILE", "shop.lodging.mount_injury.none", lines);
                    continue;
                }
                if (!payServiceWithSubscriptionVoucherOrGold(player, "stable", "mount_rest_care", "Soin et repos de monture", 65, lines))
                {
                    showShopResult("SOIN REFUSÉ", "shop.lodging.mount_injury.failed", lines);
                    continue;
                }
                const int injuries = player.getInventory().countMaterialById("mount_minor_injury_marker");
                player.getInventory().removeMaterialQuantityById("mount_minor_injury_marker", injuries);
                lines.push_back("L'écurie vérifie les appuis, pose un baume simple et interdit de repartir en sprintant immédiatement.");
                lines.push_back("Blessure légère retirée.");
                showLocalServiceResult("MONTURE SOIGNÉE", "shop.lodging.mount_injury.success", player, lines, 1);
            }
            else if (choice == 16)
            {
                std::vector<std::string> lines;
                const int price = 86;
                if (!player.getInventory().spendGold(price))
                {
                    lines.push_back("Paiement refusé : il faut " + Money::formatGoldWithRaw(price) + ".");
                    lines.push_back("Argent disponible : " + Money::formatCurrencyOverviewFromCopper(player.getInventory().getTotalCopper()) + ".");
                    showShopResult("BRIDON REFUSÉ", "shop.lodging.mount_bridle.failed", lines);
                    continue;
                }
                player.getInventory().addMaterial(MaterialCatalog::createById("mount_comfort_bridle", 1));
                lines.push_back("Bridon confortable installé : pas un turbo magique, juste moins de frottements et de panique sur les départs répétés.");
                lines.push_back("Effet : avec un bon lien, certains trajets courts peuvent ne pas ajouter de fatigue.");
                showLocalServiceResult("BRIDON INSTALLÉ", "shop.lodging.mount_bridle.success", player, lines, 1);
            }
            else if (choice == 17)
            {
                std::vector<std::string> lines;
                const int price = 74;
                if (!player.getInventory().spendGold(price))
                {
                    lines.push_back("Paiement refusé : il faut " + Money::formatGoldWithRaw(price) + ".");
                    lines.push_back("Argent disponible : " + Money::formatCurrencyOverviewFromCopper(player.getInventory().getTotalCopper()) + ".");
                    showShopResult("COUVERTURE REFUSÉE", "shop.lodging.mount_blanket.failed", lines);
                    continue;
                }
                player.getInventory().addMaterial(MaterialCatalog::createById("mount_weather_blanket", 1));
                lines.push_back("Couverture météo installée : Tavia ajuste les attaches pour éviter que le tissu tourne sous la selle.");
                lines.push_back("Effet : aide la monture dans les routes froides/humides et limite une partie de l'exposition météo en exploration.");
                showLocalServiceResult("COUVERTURE INSTALLÉE", "shop.lodging.mount_blanket.success", player, lines, 1);
            }
            else if (choice == 18)
            {
                std::vector<std::string> lines;
                const int price = 92;
                if (!player.getInventory().spendGold(price))
                {
                    lines.push_back("Paiement refusé : il faut " + Money::formatGoldWithRaw(price) + ".");
                    lines.push_back("Argent disponible : " + Money::formatCurrencyOverviewFromCopper(player.getInventory().getTotalCopper()) + ".");
                    showShopResult("HARNAIS REFUSÉ", "shop.lodging.mount_harness.failed", lines);
                    continue;
                }
                player.getInventory().addMaterial(MaterialCatalog::createById("mount_pack_harness", 1));
                lines.push_back("Harnais de bât ajusté : les charges ne tirent plus toutes du même côté comme une mauvaise blague de gobelin.");
                lines.push_back("Effet : les longues explorations chargées deviennent plus propres, avec moins de risque de gêne légère.");
                showLocalServiceResult("HARNAIS AJUSTÉ", "shop.lodging.mount_harness.success", player, lines, 1);
            }
            else if (choice == 19)
            {
                std::vector<std::string> lines;
                const int routeMemory = std::min(2, player.getInventory().countMaterialById("mount_route_memory_marker"));
                if (routeMemory >= 2)
                {
                    lines.push_back("Mémoire de route déjà au maximum actuel : 2/2.");
                    showShopResult("ROUTE DÉJÀ CONNUE", "shop.lodging.mount_route_memory.max", lines);
                    continue;
                }
                if (!payServiceWithSubscriptionVoucherOrGold(player, "stable", "stable_stall_ticket", "Ticket d'écurie", 34, lines))
                {
                    showShopResult("ROUTE REFUSÉE", "shop.lodging.mount_route_memory.failed", lines);
                    continue;
                }
                player.getInventory().addMaterial(MaterialCatalog::createById("mount_route_memory_marker", 1));
                lines.push_back("Tavia fait répéter un départ, deux arrêts et le retour au relais. La monture comprend mieux le rythme attendu.");
                lines.push_back("Mémoire de route : " + std::to_string(routeMemory + 1) + "/2.");
                showLocalServiceResult("ROUTE RÉPÉTÉE", "shop.lodging.mount_route_memory.success", player, lines, 1);
            }

            else if (choice == 20)
            {
                std::vector<std::string> lines;
                const int surefoot = std::min(2, player.getInventory().countMaterialById("mount_surefoot_training_marker"));
                if (surefoot >= 2)
                {
                    lines.push_back("Assurance déjà au maximum actuel : 2/2.");
                    showShopResult("APPUIS STABLES", "shop.lodging.mount_surefoot.max", lines);
                    continue;
                }
                if (!payServiceWithSubscriptionVoucherOrGold(player, "stable", "mount_grooming_kit", "Kit de pansage de monture", 32, lines))
                {
                    showShopResult("SÉANCE REFUSÉE", "shop.lodging.mount_surefoot.failed", lines);
                    continue;
                }
                player.getInventory().addMaterial(MaterialCatalog::createById("mount_surefoot_training_marker", 1));
                lines.push_back("Tavia fait passer la monture sur des planches, des pierres plates et un petit pont qui craque juste assez pour apprendre sans traumatiser.");
                lines.push_back("Assurance de monture : " + std::to_string(surefoot + 1) + "/2.");
                showLocalServiceResult("APPUIS TRAVAILLÉS", "shop.lodging.mount_surefoot.success", player, lines, 1);
            }
            else if (choice == 21)
            {
                std::vector<std::string> lines;
                const int price = 88;
                if (!player.getInventory().spendGold(price))
                {
                    lines.push_back("Paiement refusé : il faut " + Money::formatGoldWithRaw(price) + ".");
                    lines.push_back("Argent disponible : " + Money::formatCurrencyOverviewFromCopper(player.getInventory().getTotalCopper()) + ".");
                    showShopResult("FERRAGE REFUSÉ", "shop.lodging.mount_road_shoes.failed", lines);
                    continue;
                }
                player.getInventory().addMaterial(MaterialCatalog::createById("mount_road_shoes", 1));
                lines.push_back("Le palefrenier ajuste les fers sans transformer la monture en machine de guerre : juste de quoi éviter les petites douleurs de route.");
                lines.push_back("Effet : moins de risques de blessure légère quand une longue sortie s'enchaîne mal.");
                showLocalServiceResult("FERRAGE INSTALLÉ", "shop.lodging.mount_road_shoes.success", player, lines, 1);
            }
            else if (choice == 9)
            {
                openSubscriptionMenu(
                    player,
                    "ABONNEMENTS DE L'AUBERGE",
                    {
                        {"lodging_modest_weekly", "Forfait hebdo auberge modeste", 110, "Couvre les repas simples et la chambre simple pendant 7 jours."},
                        {"stable_relay_weekly", "Forfait écurie et relais", 135, "Couvre les préparations d'écurie simples pendant 7 jours."},
                        {"guild_adventurer_standard_weekly", "Cotisation aventurier standard", 160, "Cotisation de guilde adaptée : lit partagé, salle commune et petits services locaux pendant 7 jours."}
                    }
                );
            }
        }
    }

    void openTransportServiceMenu(Player& player)
    {
        bool stayInServices = true;

        while (stayInServices)
        {
            MenuScreen screen("RELAIS DES ROUTES", "shop.transport.services");
            screen.addLine("Noro transforme les papiers de transport en vrais préparatifs : route, écurie, bagages, péages et convoi.");
            screen.addLine("Temps : " + worldTimeLineForPlayer(player));
            if (subscriptionCoversService(player, "transport"))
            {
                screen.addLine("Abonnement : les préparatifs de route simples sont couverts par la période active.");
            }
            screen.addLine(serviceCostLine(player, "stable_stall_ticket", "Ticket d'écurie", 30));
            screen.addLine(serviceCostLine(player, "caravan_seat_ticket", "Place de caravane", 60));
            screen.addLine(serviceCostLine(player, "guarded_transport_pass", "Pass de transport gardé", 105));
            screen.addOption(0, "Retour", "Revenir au comptoir du relais.", true, "shop.transport.back");
            screen.addOption(1, "Préparer monture et bagages", "Consomme 1 segment et produit une preuve de route simple.", true, "shop.transport.stable");
            screen.addOption(2, "Préparer un départ de caravane", "Consomme 2 segments et produit un reçu de péage.", true, "shop.transport.caravan");
            screen.addOption(3, "Organiser un convoi gardé", "Consomme 2 segments et produit une preuve locale solide.", true, "shop.transport.guarded");
            screen.addOption(4, "Faire relire l'itinéraire", "Produit une note d'éclaireur de route pour éviter un détour futur.", true, "shop.transport.scout");
            screen.addOption(5, "Obtenir un badge de route du relais", "Preuve plus forte qu'un reçu simple, consomme 1 segment.", true, "shop.transport.badge");
            screen.addOption(6, "Abonnements de transport", "Pass de 7 jours : écurie, relais, commerce ou route gardée.", true, "shop.transport.subscriptions");

            Console::clear();
            const int choice = TerminalInterface::askMenuChoiceFromOptions(
                screen,
                "Choisis un service de relais, ou 0 pour revenir."
            );

            if (choice == 0)
            {
                stayInServices = false;
                continue;
            }

            if (choice == 1)
            {
                std::vector<std::string> lines;
                if (!payServiceWithSubscriptionVoucherOrGold(player, "stable", "stable_stall_ticket", "Ticket d'écurie", 30, lines))
                {
                    showShopResult("PRÉPARATION REFUSÉE", "shop.transport.stable.failed", lines);
                    continue;
                }

                player.getInventory().addMaterial(MaterialCatalog::createById("route_toll_receipt", 1));
                player.getInventory().addMaterial(MaterialCatalog::createById("travel_distance_mark", 1));
                lines.push_back("Monture et bagages préparés : le relais sait maintenant quoi charger, garder et laisser respirer.");
                lines.push_back("Preuves obtenues : Reçu de péage de route x1, Marque de distance de trajet x1.");
                showLocalServiceResult("RELAIS PRÉPARÉ", "shop.transport.stable.success", player, lines, 1);
            }
            else if (choice == 2)
            {
                std::vector<std::string> lines;
                if (!payServiceWithSubscriptionVoucherOrGold(player, "transport", "caravan_seat_ticket", "Place de caravane", 60, lines))
                {
                    showShopResult("CARAVANE REFUSÉE", "shop.transport.caravan.failed", lines);
                    continue;
                }

                player.getInventory().addMaterial(MaterialCatalog::createById("route_toll_receipt", 1));
                player.getInventory().addMaterial(MaterialCatalog::createById("travel_distance_mark", 2));
                lines.push_back("Départ préparé : nom inscrit, place notée, caisse attachée avec une confiance relative.");
                lines.push_back("Preuves obtenues : Reçu de péage de route x1, Marque de distance de trajet x2.");
                lines.push_back("Le trajet n'est pas joué automatiquement : ce service prépare surtout les quêtes et justificatifs de transport.");
                showLocalServiceResult("CARAVANE PRÉPARÉE", "shop.transport.caravan.success", player, lines, 2);
            }
            else if (choice == 3)
            {
                std::vector<std::string> lines;
                if (!payServiceWithSubscriptionVoucherOrGold(player, "transport", "guarded_transport_pass", "Pass de transport gardé", 105, lines))
                {
                    showShopResult("CONVOI REFUSÉ", "shop.transport.guarded.failed", lines);
                    continue;
                }

                player.getInventory().addMaterial(MaterialCatalog::createById("local_reputation_note", 1));
                player.getInventory().addMaterial(MaterialCatalog::createById("travel_distance_mark", 2));
                lines.push_back("Convoi préparé : garde prévu, itinéraire noté, et quelqu'un d'autre que toi sera payé pour surveiller les roues.");
                lines.push_back("Preuves obtenues : Note de réputation locale x1, Marque de distance de trajet x2.");
                lines.push_back("Ce service coûte cher mais peut aider les demandes de haut niveau liées aux routes gardées.");
                showLocalServiceResult("CONVOI GARDÉ PRÉPARÉ", "shop.transport.guarded.success", player, lines, 2);
            }
            else if (choice == 4)
            {
                std::vector<std::string> lines;
                if (!payServiceWithSubscriptionVoucherOrGold(player, "transport", "route_toll_receipt", "Reçu de péage de route", 38, lines))
                {
                    showShopResult("ITINÉRAIRE REFUSÉ", "shop.transport.scout.failed", lines);
                    continue;
                }

                player.getInventory().addMaterial(MaterialCatalog::createById("route_scout_note", 1));
                lines.push_back("Itinéraire relu : Noro note un raccourci, une route fermée et un relais à éviter si tu tiens à tes bottes.");
                lines.push_back("Preuve obtenue : Note d'éclaireur de route x1.");
                lines.push_back("Usage prévu : réduit un déplacement d'exploration plus tard, surtout sur biomes éloignés.");
                showLocalServiceResult("ITINÉRAIRE RELU", "shop.transport.scout.success", player, lines, 1);
            }
            else if (choice == 5)
            {
                std::vector<std::string> lines;
                if (!payServiceWithSubscriptionVoucherOrGold(player, "transport", "relay_route_badge", "Badge de route du relais", 55, lines))
                {
                    showShopResult("BADGE REFUSÉ", "shop.transport.badge.failed", lines);
                    continue;
                }

                player.getInventory().addMaterial(MaterialCatalog::createById("relay_route_badge", 1));
                player.getInventory().addMaterial(MaterialCatalog::createById("route_scout_note", 1));
                lines.push_back("Badge signé : le relais confirme que ton passage est préparé et que ton itinéraire n'est pas improvisé au hasard.");
                lines.push_back("Preuves obtenues : Badge de route du relais x1, Note d'éclaireur de route x1.");
                lines.push_back("Usage prévu : routes contrôlées, quêtes de transport, et réduction de déplacements préparés.");
                showLocalServiceResult("BADGE DE ROUTE", "shop.transport.badge.success", player, lines, 1);
            }
            else if (choice == 6)
            {
                openSubscriptionMenu(
                    player,
                    "ABONNEMENTS DU RELAIS",
                    {
                        {"stable_relay_weekly", "Forfait écurie et relais", 135, "Préparations de monture, box et bagages simples pendant 7 jours."},
                        {"trade_route_weekly", "Pass de commerce hebdomadaire", 240, "Préparatifs de caravanes, routes et contrôles marchands simples pendant 7 jours."},
                        {"merchant_cotisation_weekly", "Cotisation de la confrérie du commerce", 210, "Protection contractuelle légère et meilleurs papiers de ville pendant 7 jours."}
                    }
                );
            }
        }
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

            std::string label = items[i].getName()
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
            itemData.name = items[i].getName();
            itemData.detail = items[i].getDescription();
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
            info.name = weapon.getName();
            info.durability = formatEquipmentDurabilityText(weapon.getDurability(), weapon.getMaxDurability());
            info.enchantmentSummary = "Enchantements : " + weapon.getEnchantmentSummaryText();
            info.detail = "Arme possédée par le personnage. " + info.durability + ". " + info.enchantmentSummary + ". Prix ajusté par état, enchantements et acheteur spécialisé.";
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
            buyData.name = item.getName();
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
                barterData.name = item.getName();
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
                (shop.getType() == ShopType::Lodging || shop.getType() == ShopType::Transport || shop.getType() == ShopType::CityService || shop.getType() == ShopType::Church || shop.getType() == ShopType::Enchanter)
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
                    openChurchServiceMenu(player);
                    continue;
                }

                if (shop.getType() == ShopType::Enchanter)
                {
                    openEnchanterServiceMenu(player);
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

    bool stayInMenu = true;

    while (stayInMenu)
    {
        Console::clear();
        int choice = TerminalInterface::askMenuChoiceFromOptions(
            buildShopListScreen(shops, &player),
            "Veuillez choisir une boutique affichée, ou 0 pour revenir."
        );

        if (choice == 0)
        {
            stayInMenu = false;
            continue;
        }

        openSingleShop(player, shops[choice - 1]);
    }
}
