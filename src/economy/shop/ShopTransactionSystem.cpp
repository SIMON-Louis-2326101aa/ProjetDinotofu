// EN: ShopTransactionSystem.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: ShopTransactionSystem.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.
// English: Implements shop purchases and safe resale rules.
// Français : Implémente les achats de boutique et les règles de revente sécurisée.

#include "economy/shop/ShopTransactionSystem.hpp"

#include "economy/shop/ShopPriceRules.hpp"
#include "item/armor/ArmorCatalog.hpp"
#include "item/consumable/ConsumableCatalog.hpp"
#include "item/material/MaterialCatalog.hpp"
#include "item/material/Material.hpp"
#include "item/weapon/WeaponCatalog.hpp"
#include "progression/bestiary/BestiaryRuntimeProgress.hpp"

#include <iostream>
#include <random>
#include <algorithm>
#include <cctype>
#include <string>

namespace
{
    // EN: isMaterialShop declares or implements a focused behavior used by this module.
    // FR: isMaterialShop déclare ou implémente un comportement précis utilisé par ce module.
    bool isMaterialShop(ShopType type)
    {
        return type == ShopType::MonsterMaterial
            || type == ShopType::Material
            || type == ShopType::Plant
            || type == ShopType::Library
            || type == ShopType::Blacksmith
            || type == ShopType::Alchemist
            || type == ShopType::BlackMarket;
    }

    // EN: rollShopPercent declares or implements a focused behavior used by this module.
    // FR: rollShopPercent déclare ou implémente un comportement précis utilisé par ce module.
    bool rollShopPercent(int percent)
    {
        // EN: generator declares or implements a focused behavior used by this module.
        // FR: generator déclare ou implémente un comportement précis utilisé par ce module.
        static std::mt19937 generator(std::random_device{}());
        std::uniform_int_distribution<int> distribution(1, 100);
        return distribution(generator) <= percent;
    }

    // EN: canRareQualityBeSoldByShop declares or implements a focused behavior used by this module.
    // FR: canRareQualityBeSoldByShop déclare ou implémente un comportement précis utilisé par ce module.
    bool canRareQualityBeSoldByShop(const std::string& id)
    {
        return id == "goblin_ear"
            || id == "wolf_fang"
            || id == "rusted_metal_fragment"
            || id == "worn_leather_piece"
            || id == "mountain_blue_flower"
            || id == "bitter_healing_leaf"
            || id == "cracked_bone"
            || id == "arcane_dust"
            || id == "slime_residue"
            || id == "battle_torn_badge"
            || id == "beast_hide"
            || id == "shadow_thread"
            || id == "kitsune_ember"
            || id == "draconic_scale_fragment"
            || id == "unstable_core"
            || id == "anomaly_glitch_fragment";
    }

    // EN: prefersPureQuality declares or implements a focused behavior used by this module.
    // FR: prefersPureQuality déclare ou implémente un comportement précis utilisé par ce module.
    bool prefersPureQuality(const std::string& id)
    {
        return id == "goblin_ear"
            || id == "wolf_fang"
            || id == "cracked_bone"
            || id == "slime_residue"
            || id == "battle_torn_badge"
            || id == "beast_hide"
            || id == "shadow_thread"
            || id == "kitsune_ember"
            || id == "draconic_scale_fragment"
            || id == "unstable_core"
            || id == "anomaly_glitch_fragment";
    }

    // EN: createShopMaterialWithPossibleRareQuality declares or implements a focused behavior used by this module.
    // FR: createShopMaterialWithPossibleRareQuality déclare ou implémente un comportement précis utilisé par ce module.
    bool looksLikeBlackMarketItem(const ShopItem& item)
    {
        const std::string name = item.getName();
        return name.find("interdit") != std::string::npos
            || name.find("noir") != std::string::npos
            || name.find("glitch") != std::string::npos
            || name.find("non déclarée") != std::string::npos
            || name.find("expérimental") != std::string::npos;
    }

    std::string normalizeShopLearningText(std::string value)
    {
        std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });
        return value;
    }

    bool isMagicStudyCompatible(const Player& player)
    {
        const std::string className = normalizeShopLearningText(player.getType());
        return className.find("mage") != std::string::npos
            || className.find("magicien") != std::string::npos
            || className.find("ensorc") != std::string::npos
            || className.find("sorc") != std::string::npos
            || className.find("arcan") != std::string::npos
            || className.find("pyrom") != std::string::npos
            || className.find("cryo") != std::string::npos
            || className.find("occult") != std::string::npos
            || className.find("invoc") != std::string::npos
            || className.find("nécro") != std::string::npos
            || className.find("necro") != std::string::npos
            || className.find("pact") != std::string::npos
            || className.find("clerc") != std::string::npos
            || className.find("prêtre") != std::string::npos
            || className.find("pretre") != std::string::npos
            || className.find("paladin") != std::string::npos
            || className.find("druide") != std::string::npos
            || className.find("shaman") != std::string::npos
            || className.find("templier") != std::string::npos;
    }

    void applyMagicLearningEffect(Player& player, const ShopItem& item)
    {
        const std::string id = item.getId();

        if (id != "basic_magic_manual"
            && id != "arcane_binding_grimoire"
            && id != "elemental_ward_grimoire"
            && id != "resistance_rift_grimoire"
            && id != "frost_needle_grimoire"
            && id != "mana_suture_grimoire"
            && id != "occult_bramble_grimoire")
        {
            return;
        }

        if (!isMagicStudyCompatible(player))
        {
            std::cout << "Le texte est conservé, mais il ne suffit pas à transformer un non-mage en lanceur de sorts." << std::endl;
            std::cout << "Pour utiliser une magie hors classe, il faudra plutôt compter sur des parchemins à usage unique." << std::endl;
            return;
        }

        if (id == "basic_magic_manual")
        {
            if (player.getLevel() < 3)
            {
                std::cout << "Le manuel est compris en partie, mais il faudra atteindre le niveau 3 pour stabiliser la première marque." << std::endl;
                return;
            }
            player.unlockActiveSkill("learned_arcane_mark", "Marque élémentaire étudiée");
            return;
        }

        if (id == "arcane_binding_grimoire")
        {
            if (player.getLevel() < 5)
            {
                std::cout << "Les chaînes arcaniques restent trop lourdes à former. Niveau 5 requis pour les comprendre vraiment." << std::endl;
                return;
            }
            player.unlockActiveSkill("learned_arcane_binding", "Entrave arcanique étudiée");
            return;
        }

        if (id == "elemental_ward_grimoire")
        {
            if (player.getLevel() < 6)
            {
                std::cout << "Le voile répond, mais trop faiblement. Niveau 6 requis pour le tenir en combat." << std::endl;
                return;
            }
            player.unlockActiveSkill("learned_elemental_ward", "Voile élémentaire étudié");
            return;
        }

        if (id == "resistance_rift_grimoire")
        {
            if (player.getLevel() < 9)
            {
                std::cout << "La faille décrite dans le grimoire tire trop fort sur le corps. Niveau 9 requis." << std::endl;
                return;
            }
            player.unlockActiveSkill("learned_resistance_rift", "Faille de résistance étudiée");
            return;
        }

        if (id == "frost_needle_grimoire")
        {
            if (player.getLevel() < 4)
            {
                std::cout << "Les aiguilles de givre se brisent dans la paume. Niveau 4 requis pour les former proprement." << std::endl;
                return;
            }
            player.unlockActiveSkill("learned_frost_needle", "Aiguille de givre étudiée");
            return;
        }

        if (id == "mana_suture_grimoire")
        {
            if (player.getLevel() < 7)
            {
                std::cout << "Le fil de mana casse avant de refermer la blessure. Niveau 7 requis pour l'étudier sans danger." << std::endl;
                return;
            }
            player.unlockActiveSkill("learned_mana_suture", "Suture de mana étudiée");
            return;
        }

        if (id == "occult_bramble_grimoire")
        {
            if (player.getLevel() < 5)
            {
                std::cout << "Les ronces se dessinent, mais elles cassent avant de saisir une cible. Niveau 5 requis." << std::endl;
                return;
            }
            player.unlockActiveSkill("learned_occult_bramble", "Ronces occultes étudiées");
            return;
        }
    }

    Material createShopMaterialWithPossibleRareQuality(const ShopItem& item)
    {
        Material material = MaterialCatalog::createById(item.getId(), 1);

        if (looksLikeBlackMarketItem(item))
        {
            if (rollShopPercent(18))
            {
                material.setQuality("exceptional");
            }
            else if (rollShopPercent(45))
            {
                material.setQuality(prefersPureQuality(item.getId()) ? "pure" : "high");
            }
            else if (rollShopPercent(20))
            {
                material.setQuality("impure");
            }

            return material;
        }

        if (canRareQualityBeSoldByShop(item.getId()) && rollShopPercent(4))
        {
            material.setQuality(prefersPureQuality(item.getId()) ? "pure" : "high");
        }

        return material;
    }
}

// EN: canBeBoughtNow declares or implements a focused behavior used by this module.
// FR: canBeBoughtNow déclare ou implémente un comportement précis utilisé par ce module.
bool ShopTransactionSystem::canBeBoughtNow(const ShopItem& item)
{
    const std::string id = item.getId();

    return id == "minor_healing_potion"
        || id == "basic_healing_potion"
        || id == "reinforced_healing_potion"
        || id == "greater_healing_potion"
        || id == "major_healing_potion"
        || id == "minor_damage_potion"
        || id == "basic_damage_potion"
        || id == "reinforced_damage_potion"
        || id == "greater_damage_potion"
        || id == "experimental_damage_potion"
        || id == "defensive_potion"
        || id == "greater_defensive_potion"
        || id == "precision_potion"
        || id == "weakening_debuff_potion"
        || id == "fragility_debuff_potion"
        || id == "antidote_potion"
        || id == "burn_salve_potion"
        || id == "frost_resistance_potion"
        || id == "shock_resistance_potion"
        || id == "elemental_ward_potion"
        || id == "smoke_escape_vial"
        || id == "arcane_spark_scroll"
        || id == "elemental_ward_scroll"
        || id == "resistance_rift_scroll"
        || id == "wandering_ember_scroll"
        || id == "minor_purification_scroll"
        || id == "crawling_venom_scroll"
        || id == "rusty_sword"
        || id == "training_dagger"
        || id == "training_spear"
        || id == "training_bow"
        || id == "training_crossbow"
        || id == "training_throwing_bandolier"
        || id == "training_staff"
        || id == "heavy_training_axe"
        || id == "worn_leather_armor"
        || id == "goblin_ear"
        || id == "wolf_fang"
        || id == "rusted_metal_fragment"
        || id == "worn_leather_piece"
        || id == "mountain_blue_flower"
        || id == "bitter_healing_leaf"
        || id == "common_goblin_notes"
        || id == "common_wolf_notes"
        || id == "basic_plant_manual"
        || id == "class_identity_manual"
        || id == "biome_field_notes"
        || id == "basic_magic_manual"
        || id == "arcane_binding_grimoire"
        || id == "elemental_ward_grimoire"
        || id == "resistance_rift_grimoire"
        || id == "frost_needle_grimoire"
        || id == "mana_suture_grimoire"
        || id == "occult_bramble_grimoire"
        || id == "cracked_bone"
        || id == "arcane_dust"
        || id == "slime_residue"
        || id == "battle_torn_badge"
        || id == "weak_repair_kit"
        || id == "medium_repair_kit"
        || id == "big_repair_kit"
        || id == "tinkerer_complete_repair_kit"
        || id == "small_repair_kit"
        || id == "reinforced_repair_kit"
        || id == "special_adventurer_notes"
        || id == "summoning_notes"
        || id == "boss_identity_scrap"
        || id == "potion_recipe_page"
        || id == "repair_recipe_page"
        || id == "advanced_monster_notes"
        || id == "necromancy_warning"
        || id == "beast_hide"
        || id == "shadow_thread"
        || id == "kitsune_ember"
        || id == "draconic_scale_fragment"
        || id == "unstable_core"
        || id == "precision_harvest_tools"
        || id == "preservation_vials"
        || id == "anomaly_glitch_fragment"
        || id == "clean_harvest_manual"
        || id == "monster_dissection_guide"
        || id == "training_arrows"
        || id == "training_bolts"
        || id == "training_throwing_knives"
        || id == "barbed_arrows"
        || id == "piercing_bolts"
        || id == "balanced_throwing_knives"
        || id == "ash_arrows"
        || id == "frozen_bolts"
        || id == "conductive_knives"
        || id == "venom_arrows"
        || id == "shock_bolts"
        || id == "smoke_knives"
        || id == "slime_color_codex"
        || id == "monster_family_evolution_notes"
        || id == "weapon_training_notes"
        || id == "magic_learning_notes"
        || id == "elemental_weakness_notes";
}

bool ShopTransactionSystem::buyItem(
    Player& player,
    ShopItem& item,
    int finalPrice
)
{
    if (!canBeBoughtNow(item))
    {
        displayUnsupportedPurchaseMessage(item);
        return false;
    }

    if (item.isSoldOut())
    {
        std::cout << "Stock épuisé pour " << item.getName() << "." << std::endl;
        std::cout << "Il faudra attendre une nouvelle rotation de boutique." << std::endl;
        std::cout << std::endl;
        return false;
    }

    if (!player.getInventory().spendGold(finalPrice))
    {
        std::cout << "Tu n'as pas assez d'or pour acheter "
                  << item.getName()
                  << "."
                  << std::endl;
        std::cout << "Or disponible : "
                  << player.getInventory().getGold()
                  << " pièces."
                  << std::endl;
        std::cout << std::endl;
        return false;
    }

    if (item.getId() == "minor_healing_potion")
    {
        player.getInventory().addConsumable(ConsumableCatalog::createMinorHealingPotion());
    }
    else if (item.getId() == "basic_healing_potion")
    {
        player.getInventory().addConsumable(ConsumableCatalog::createBasicHealingPotion());
    }
    else if (item.getId() == "reinforced_healing_potion")
    {
        player.getInventory().addConsumable(ConsumableCatalog::createReinforcedHealingPotion());
    }
    else if (item.getId() == "greater_healing_potion")
    {
        player.getInventory().addConsumable(ConsumableCatalog::createGreaterHealingPotion());
    }
    else if (item.getId() == "major_healing_potion")
    {
        player.getInventory().addConsumable(ConsumableCatalog::createMajorHealingPotion());
    }
    else if (item.getId() == "minor_damage_potion")
    {
        player.getInventory().addConsumable(ConsumableCatalog::createMinorDamagePotion());
    }
    else if (item.getId() == "basic_damage_potion")
    {
        player.getInventory().addConsumable(ConsumableCatalog::createBasicDamagePotion());
    }
    else if (item.getId() == "reinforced_damage_potion")
    {
        player.getInventory().addConsumable(ConsumableCatalog::createReinforcedDamagePotion());
    }
    else if (item.getId() == "greater_damage_potion")
    {
        player.getInventory().addConsumable(ConsumableCatalog::createGreaterDamagePotion());
    }
    else if (item.getId() == "experimental_damage_potion")
    {
        player.getInventory().addConsumable(ConsumableCatalog::createExperimentalDamagePotion());
    }
    else if (item.getId() == "defensive_potion")
    {
        player.getInventory().addConsumable(ConsumableCatalog::createDefensivePotion());
    }
    else if (item.getId() == "greater_defensive_potion")
    {
        player.getInventory().addConsumable(ConsumableCatalog::createGreaterDefensivePotion());
    }
    else if (item.getId() == "precision_potion")
    {
        player.getInventory().addConsumable(ConsumableCatalog::createPrecisionPotion());
    }
    else if (item.getId() == "weakening_debuff_potion")
    {
        player.getInventory().addConsumable(ConsumableCatalog::createWeakeningDebuffPotion());
    }
    else if (item.getId() == "fragility_debuff_potion")
    {
        player.getInventory().addConsumable(ConsumableCatalog::createFragilityDebuffPotion());
    }
    else if (item.getId() == "antidote_potion")
    {
        player.getInventory().addConsumable(ConsumableCatalog::createAntidotePotion());
    }
    else if (item.getId() == "burn_salve_potion")
    {
        player.getInventory().addConsumable(ConsumableCatalog::createBurnSalvePotion());
    }
    else if (item.getId() == "frost_resistance_potion")
    {
        player.getInventory().addConsumable(ConsumableCatalog::createFrostResistancePotion());
    }
    else if (item.getId() == "shock_resistance_potion")
    {
        player.getInventory().addConsumable(ConsumableCatalog::createShockResistancePotion());
    }
    else if (item.getId() == "elemental_ward_potion")
    {
        player.getInventory().addConsumable(ConsumableCatalog::createElementalWardPotion());
    }
    else if (item.getId() == "smoke_escape_vial")
    {
        player.getInventory().addConsumable(ConsumableCatalog::createSmokeEscapeVial());
    }
    else if (item.getId() == "arcane_spark_scroll")
    {
        player.getInventory().addConsumable(ConsumableCatalog::createArcaneSparkScroll());
    }
    else if (item.getId() == "elemental_ward_scroll")
    {
        player.getInventory().addConsumable(ConsumableCatalog::createElementalWardScroll());
    }
    else if (item.getId() == "resistance_rift_scroll")
    {
        player.getInventory().addConsumable(ConsumableCatalog::createResistanceRiftScroll());
    }
    else if (item.getId() == "wandering_ember_scroll")
    {
        player.getInventory().addConsumable(ConsumableCatalog::createWanderingEmberScroll());
    }
    else if (item.getId() == "minor_purification_scroll")
    {
        player.getInventory().addConsumable(ConsumableCatalog::createMinorPurificationScroll());
    }
    else if (item.getId() == "crawling_venom_scroll")
    {
        player.getInventory().addConsumable(ConsumableCatalog::createCrawlingVenomScroll());
    }
    else if (item.getId() == "rusty_sword")
    {
        player.getInventory().addWeapon(WeaponCatalog::createRustySword());
    }
    else if (item.getId() == "training_dagger")
    {
        player.getInventory().addWeapon(WeaponCatalog::createTrainingDagger());
    }
    else if (item.getId() == "training_spear")
    {
        player.getInventory().addWeapon(WeaponCatalog::createTrainingSpear());
    }
    else if (item.getId() == "training_bow")
    {
        player.getInventory().addWeapon(WeaponCatalog::createTrainingBow());
    }
    else if (item.getId() == "training_crossbow")
    {
        player.getInventory().addWeapon(WeaponCatalog::createTrainingCrossbow());
    }
    else if (item.getId() == "training_throwing_bandolier")
    {
        player.getInventory().addWeapon(WeaponCatalog::createTrainingThrowingBandolier());
    }
    else if (item.getId() == "training_staff")
    {
        player.getInventory().addWeapon(WeaponCatalog::createTrainingStaff());
    }
    else if (item.getId() == "heavy_training_axe")
    {
        player.getInventory().addWeapon(WeaponCatalog::createHeavyTrainingAxe());
    }
    else if (item.getId() == "worn_leather_armor")
    {
        player.getInventory().addArmor(ArmorCatalog::createWornLeatherArmor());
    }
    else
    {
        Material boughtMaterial = createShopMaterialWithPossibleRareQuality(item);
        player.getInventory().addMaterial(boughtMaterial);

        if (boughtMaterial.hasSpecialQuality())
        {
            std::cout << "Trouvaille rare en boutique : qualité "
                      << boughtMaterial.getQualityLabel()
                      << "." << std::endl;
            if (boughtMaterial.getQuality() == "exceptional")
            {
                std::cout << "Le marché noir te glisse quelque chose qu'aucun vendeur honnête n'aurait posé sur un comptoir." << std::endl;
            }
            else
            {
                std::cout << "Les objets exceptionnels restent normalement impossibles à acheter hors circuits douteux." << std::endl;
            }
        }
    }

    item.consumeOneStock();

    std::cout << "Achat réussi : " << item.getName() << "." << std::endl;
    std::cout << "Or restant : "
              << player.getInventory().getGold()
              << " pièces."
              << std::endl;

    if (item.getStock() >= 0)
    {
        std::cout << "Stock restant chez le marchand : "
                  << item.getStock()
                  << "."
                  << std::endl;
    }

    if (item.isCommonInformation())
    {
        BestiaryRuntimeProgress::unlockCommonInformation(item.getId());
        std::cout << "Renseignement ajouté au bestiaire pour cette session." << std::endl;
    }

    applyMagicLearningEffect(player, item);

    std::cout << std::endl;

    return true;
}

int ShopTransactionSystem::getSellableEntryCount(
    const Player& player,
    ShopType shopType
)
{
    if (shopType == ShopType::Weapon)
    {
        return player.getInventory().getWeaponCount();
    }

    if (shopType == ShopType::Armor)
    {
        return player.getInventory().getArmorCount();
    }

    if (shopType == ShopType::Consumable)
    {
        return player.getInventory().getConsumableCount();
    }

    if (isMaterialShop(shopType))
    {
        return static_cast<int>(player.getInventory().getMaterials().size());
    }

    return 0;
}

bool ShopTransactionSystem::canShopBuyInventoryEntry(
    const Player& player,
    ShopType shopType,
    int index
)
{
    if (shopType == ShopType::Weapon)
    {
        return player.getInventory().hasWeapon(index)
            && index != player.getEquippedWeaponIndex()
            && player.getInventory().getWeapon(index).getName() != "Mains nues";
    }

    if (shopType == ShopType::Armor)
    {
        return player.getInventory().hasArmor(index)
            && index != player.getEquippedArmorIndex()
            && player.getInventory().getArmor(index).getName() != "Tenue simple";
    }

    if (shopType == ShopType::Consumable)
    {
        return player.getInventory().hasConsumable(index);
    }

    if (isMaterialShop(shopType))
    {
        return player.getInventory().hasMaterial(index);
    }

    return false;
}

int ShopTransactionSystem::getMaxSellQuantityForEntry(
    const Player& player,
    ShopType shopType,
    int index
)
{
    if (!canShopBuyInventoryEntry(player, shopType, index))
    {
        return 0;
    }

    if (isMaterialShop(shopType) && player.getInventory().hasMaterial(index))
    {
        return player.getInventory().getMaterial(index).getQuantity();
    }

    return 1;
}

int ShopTransactionSystem::getSellPriceForEntry(
    const Player& player,
    ShopType shopType,
    int index
)
{
    int basePrice = 0;

    if (shopType == ShopType::Weapon && player.getInventory().hasWeapon(index))
    {
        basePrice = player.getInventory().getWeapon(index).getValue() / 3;
    }
    else if (shopType == ShopType::Armor && player.getInventory().hasArmor(index))
    {
        basePrice = player.getInventory().getArmor(index).getValue() / 3;
    }
    else if (shopType == ShopType::Consumable && player.getInventory().hasConsumable(index))
    {
        basePrice = player.getInventory().getConsumable(index).getValue() / 3;
    }
    else if (isMaterialShop(shopType) && player.getInventory().hasMaterial(index))
    {
        const Material& material = player.getInventory().getMaterial(index);
        basePrice = material.getValue() * material.getQualityPricePercent() / 100;
    }

    if (basePrice < 1)
    {
        basePrice = 1;
    }

    return ShopPriceRules::applySellModifier(basePrice, player.getRaceText(), player.getType());
}

bool ShopTransactionSystem::sellInventoryEntryQuantity(
    Player& player,
    ShopType shopType,
    int index,
    int finalSellPrice,
    int quantity
)
{
    if (quantity <= 0)
    {
        std::cout << "Quantité invalide." << std::endl << std::endl;
        return false;
    }

    int maxQuantity = getMaxSellQuantityForEntry(player, shopType, index);
    if (maxQuantity <= 0 || quantity > maxQuantity)
    {
        std::cout << "Quantité impossible à vendre." << std::endl;
        std::cout << "Maximum vendable : " << maxQuantity << "." << std::endl << std::endl;
        return false;
    }

    if (quantity == 1)
    {
        return sellInventoryEntry(player, shopType, index, finalSellPrice);
    }

    if (!isMaterialShop(shopType))
    {
        std::cout << "Le comptoir refuse les lots pour cet objet. Vente d'un seul exemplaire." << std::endl;
        return sellInventoryEntry(player, shopType, index, finalSellPrice);
    }

    Material soldMaterial = player.getInventory().getMaterial(index);
    std::string soldName = soldMaterial.getName();
    if (soldMaterial.hasSpecialQuality())
    {
        soldName += " [" + soldMaterial.getQualityLabel() + "]";
    }

    if (!player.getInventory().removeMaterialQuantity(index, quantity))
    {
        std::cout << "La vente a échoué. Rien n'a été perdu." << std::endl << std::endl;
        return false;
    }

    int totalSellPrice = finalSellPrice * quantity;
    player.getInventory().earnGold(totalSellPrice);

    std::cout << "Vente réussie : " << soldName << " x" << quantity << "." << std::endl;
    std::cout << "+" << totalSellPrice << " pièces d'or." << std::endl;
    std::cout << "Or actuel : " << player.getInventory().getGold() << " pièces." << std::endl;
    std::cout << std::endl;

    return true;
}

bool ShopTransactionSystem::sellInventoryEntry(
    Player& player,
    ShopType shopType,
    int index,
    int finalSellPrice
)
{
    if (!canShopBuyInventoryEntry(player, shopType, index))
    {
        std::cout << "Impossible de vendre cette entrée." << std::endl;
        std::cout << "L'équipement porté, les objets de base et les entrées invalides sont protégés." << std::endl;
        std::cout << std::endl;
        return false;
    }

    std::string soldName = "Objet";
    bool removed = false;

    if (shopType == ShopType::Weapon)
    {
        soldName = player.getInventory().getWeapon(index).getName();
        removed = player.getInventory().removeWeapon(index);
    }
    else if (shopType == ShopType::Armor)
    {
        soldName = player.getInventory().getArmor(index).getName();
        removed = player.getInventory().removeArmor(index);
    }
    else if (shopType == ShopType::Consumable)
    {
        soldName = player.getInventory().getConsumable(index).getName();
        removed = player.getInventory().removeConsumable(index);
    }
    else if (isMaterialShop(shopType))
    {
        Material soldMaterial = player.getInventory().getMaterial(index);
        soldName = soldMaterial.getName();
        if (soldMaterial.hasSpecialQuality())
        {
            soldName += " [" + soldMaterial.getQualityLabel() + "]";
        }
        removed = player.getInventory().removeMaterialQuantity(index, 1);
    }

    if (!removed)
    {
        std::cout << "La vente a échoué. Rien n'a été perdu." << std::endl;
        std::cout << std::endl;
        return false;
    }

    player.getInventory().earnGold(finalSellPrice);

    std::cout << "Vente réussie : " << soldName << "." << std::endl;
    std::cout << "+" << finalSellPrice << " pièces d'or." << std::endl;
    std::cout << "Or actuel : " << player.getInventory().getGold() << " pièces." << std::endl;
    std::cout << std::endl;

    return true;
}

void ShopTransactionSystem::displaySellableEntries(
    const Player& player,
    ShopType shopType
)
{
    int count = getSellableEntryCount(player, shopType);

    if (count <= 0)
    {
        std::cout << "Rien à vendre ici pour le moment." << std::endl;
        return;
    }

    for (int i = 0; i < count; ++i)
    {
        std::cout << i + 1 << " : ";

        if (shopType == ShopType::Weapon)
        {
            std::cout << player.getInventory().getWeapon(i).getName();
        }
        else if (shopType == ShopType::Armor)
        {
            std::cout << player.getInventory().getArmor(i).getName();
        }
        else if (shopType == ShopType::Consumable)
        {
            std::cout << player.getInventory().getConsumable(i).getName();
        }
        else if (isMaterialShop(shopType))
        {
            Material material = player.getInventory().getMaterial(i);
            std::cout << material.getName() << " x" << material.getQuantity();
        }

        if (!canShopBuyInventoryEntry(player, shopType, i))
        {
            std::cout << " | Protégé";
        }
        else
        {
            std::cout << " | Revente : "
                      << getSellPriceForEntry(player, shopType, i)
                      << " or";

            int maxQuantity = getMaxSellQuantityForEntry(player, shopType, i);
            if (maxQuantity > 1)
            {
                std::cout << " | Max : x" << maxQuantity;
            }
        }

        std::cout << std::endl;
    }
}

// EN: displayUnsupportedPurchaseMessage declares or implements a focused behavior used by this module.
// FR: displayUnsupportedPurchaseMessage déclare ou implémente un comportement précis utilisé par ce module.
void ShopTransactionSystem::displayUnsupportedPurchaseMessage(const ShopItem& item)
{
    std::cout << item.getName() << " existe dans la boutique," << std::endl;
    std::cout << "mais ton sac ne peut pas le recevoir correctement." << std::endl;
    std::cout << "Aucun or n'a été dépensé." << std::endl;
    std::cout << std::endl;
}
