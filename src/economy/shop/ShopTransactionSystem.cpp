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

#include <random>
#include <algorithm>
#include <cctype>
#include <string>
#include <vector>

namespace
{

    enum class BuybackItemKind
    {
        Weapon,
        Armor,
        Consumable,
        Material
    };

    struct BuybackEntry
    {
        ShopType shopType;
        BuybackItemKind kind;
        std::string label;
        int price;
        int quantity;
        Weapon weapon;
        Armor armor;
        Consumable consumable;
        Material material;
    };

    std::vector<BuybackEntry>& buybackLedger()
    {
        static std::vector<BuybackEntry> entries;
        return entries;
    }


    std::vector<std::string>& transactionNotes()
    {
        static std::vector<std::string> notes;
        return notes;
    }

    void addTransactionNote(const std::string& note)
    {
        if (!note.empty())
        {
            transactionNotes().push_back(note);
        }
    }

    int computeBuybackPrice(int sellPrice)
    {
        int safeSellPrice = std::max(1, sellPrice);
        return safeSellPrice + std::max(1, safeSellPrice / 4);
    }

    int durabilitySellPercent(int durability, int maxDurability)
    {
        if (maxDurability < 0)
        {
            return 112;
        }

        if (maxDurability <= 0 || durability <= 0)
        {
            return 18;
        }

        int percent = std::clamp(durability * 100 / maxDurability, 1, 100);

        // FR: une arme/armure neuve se revend mieux, mais une pièce cassée chute vraiment.
        if (percent >= 95) return 100;
        if (percent >= 75) return 86;
        if (percent >= 50) return 68;
        if (percent >= 25) return 45;
        return 28;
    }

    int enchantmentSellPercent(int count)
    {
        if (count <= 0)
        {
            return 100;
        }

        // FR: chaque enchantement augmente la valeur, surtout les premiers.
        int percent = 100;
        for (int i = 1; i <= count; ++i)
        {
            if (i <= 2) percent += 18;
            else if (i <= 5) percent += 12;
            else percent += 6;
        }

        return std::clamp(percent, 100, 230);
    }

    int goodBuyerSellPercent(ShopType shopType, bool weapon, bool armor, const std::string& playerClass)
    {
        int percent = 100;

        if (weapon && shopType == ShopType::Weapon) percent += 14;
        if (armor && shopType == ShopType::Armor) percent += 14;
        if ((weapon || armor) && shopType == ShopType::Blacksmith) percent += 8;

        std::string lowered = playerClass;
        std::transform(lowered.begin(), lowered.end(), lowered.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });

        if ((weapon || armor)
            && (lowered.find("forgeron") != std::string::npos
                || lowered.find("artificier") != std::string::npos))
        {
            percent += 4;
        }

        return std::clamp(percent, 80, 135);
    }

    int equipmentSellBaseValue(const Weapon& weapon, ShopType shopType, const Player& player)
    {
        int basePrice = std::max(1, weapon.getValue() / 3);
        basePrice = basePrice * durabilitySellPercent(weapon.getDurability(), weapon.getMaxDurability()) / 100;
        basePrice = basePrice * enchantmentSellPercent(weapon.getEnchantmentCount()) / 100;
        basePrice = basePrice * goodBuyerSellPercent(shopType, true, false, player.getType()) / 100;
        return std::max(1, basePrice);
    }

    int equipmentSellBaseValue(const Armor& armor, ShopType shopType, const Player& player)
    {
        int basePrice = std::max(1, armor.getValue() / 3);
        basePrice = basePrice * durabilitySellPercent(armor.getDurability(), armor.getMaxDurability()) / 100;
        basePrice = basePrice * enchantmentSellPercent(armor.getEnchantmentCount()) / 100;
        basePrice = basePrice * goodBuyerSellPercent(shopType, false, true, player.getType()) / 100;
        return std::max(1, basePrice);
    }

    std::vector<int> visibleBuybackIndexes(ShopType shopType)
    {
        std::vector<int> indexes;
        const std::vector<BuybackEntry>& entries = buybackLedger();

        for (int i = 0; i < static_cast<int>(entries.size()); ++i)
        {
            if (entries[i].shopType == shopType)
            {
                indexes.push_back(i);
            }
        }

        return indexes;
    }

    bool resolveVisibleBuybackIndex(ShopType shopType, int visibleIndex, int& ledgerIndex)
    {
        const std::vector<int> indexes = visibleBuybackIndexes(shopType);
        if (visibleIndex < 0 || visibleIndex >= static_cast<int>(indexes.size()))
        {
            return false;
        }

        ledgerIndex = indexes[visibleIndex];
        return true;
    }

    void rememberSoldWeapon(ShopType shopType, const Weapon& weapon, int sellPrice)
    {
        BuybackEntry entry;
        entry.shopType = shopType;
        entry.kind = BuybackItemKind::Weapon;
        entry.label = weapon.getName();
        entry.price = computeBuybackPrice(sellPrice);
        entry.quantity = 1;
        entry.weapon = weapon;
        buybackLedger().push_back(entry);
    }

    void rememberSoldArmor(ShopType shopType, const Armor& armor, int sellPrice)
    {
        BuybackEntry entry;
        entry.shopType = shopType;
        entry.kind = BuybackItemKind::Armor;
        entry.label = armor.getName();
        entry.price = computeBuybackPrice(sellPrice);
        entry.quantity = 1;
        entry.armor = armor;
        buybackLedger().push_back(entry);
    }

    void rememberSoldConsumable(ShopType shopType, const Consumable& consumable, int sellPrice)
    {
        BuybackEntry entry;
        entry.shopType = shopType;
        entry.kind = BuybackItemKind::Consumable;
        entry.label = consumable.getName();
        entry.price = computeBuybackPrice(sellPrice);
        entry.quantity = 1;
        entry.consumable = consumable;
        buybackLedger().push_back(entry);
    }

    void rememberSoldMaterial(ShopType shopType, Material material, int quantity, int totalSellPrice)
    {
        material.setQuantity(quantity);

        BuybackEntry entry;
        entry.shopType = shopType;
        entry.kind = BuybackItemKind::Material;
        entry.label = material.getName();
        if (material.hasSpecialQuality())
        {
            entry.label += " [" + material.getQualityLabel() + "]";
        }
        entry.label += " x" + std::to_string(quantity);
        entry.price = computeBuybackPrice(totalSellPrice);
        entry.quantity = quantity;
        entry.material = material;
        buybackLedger().push_back(entry);
    }

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
            || type == ShopType::Enchanter
            || type == ShopType::CityService
            || type == ShopType::Lodging
            || type == ShopType::Transport
            || type == ShopType::Church
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
            || id == "predator_fang"
            || id == "boar_tusk"
            || id == "rodent_tooth"
            || id == "mountain_blue_flower"
            || id == "bitter_healing_leaf"
            || id == "arcane_dust"
            || id == "slime_residue"
            || id == "beast_hide"
            || id == "shadow_thread"
            || id == "kitsune_ember"
            || id == "draconic_scale_fragment"
            || id == "unstable_core"
            || id == "anomaly_glitch_fragment"
            || id == "runic_iron_shard"
            || id == "polished_scale_plate"
            || id == "amber_tempering_oil"

            || id == "mycelium_lantern"
            || id == "echoing_resin"
            || id == "sun_dried_clay"
            || id == "moonlit_salt"
            || id == "glass_map_fragment"
            || id == "living_vine_fiber"
            || id == "cold_iron_nail"
            || id == "tiny_gear_spring"
            || id == "cracked_bell_clapper"
            || id == "exorcism_incense"
            || id == "blue_mist_reed"
            || id == "mistglass_pearl"
            || id == "white_bone_chalk"
            || id == "buried_giant_chip"
            || id == "weeping_stone_tear"
            || id == "petrified_rose_petals";
    }

    // EN: prefersPureQuality declares or implements a focused behavior used by this module.
    // FR: prefersPureQuality déclare ou implémente un comportement précis utilisé par ce module.
    bool prefersPureQuality(const std::string& id)
    {
        return id == "goblin_ear"
            || id == "wolf_fang"
            || id == "predator_fang"
            || id == "boar_tusk"
            || id == "rodent_tooth"
            || id == "slime_residue"
            || id == "beast_hide"
            || id == "shadow_thread"
            || id == "kitsune_ember"
            || id == "draconic_scale_fragment"
            || id == "unstable_core"
            || id == "anomaly_glitch_fragment"
            || id == "runic_iron_shard"
            || id == "polished_scale_plate"

            || id == "mycelium_lantern"
            || id == "echoing_resin"
            || id == "sun_dried_clay"
            || id == "moonlit_salt"
            || id == "glass_map_fragment"
            || id == "living_vine_fiber"
            || id == "cold_iron_nail"
            || id == "tiny_gear_spring"
            || id == "cracked_bell_clapper"
            || id == "exorcism_incense"
            || id == "blue_mist_reed"
            || id == "mistglass_pearl"
            || id == "white_bone_chalk"
            || id == "buried_giant_chip"
            || id == "weeping_stone_tear"
            || id == "petrified_rose_petals";
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
            addTransactionNote("Étude : le texte est conservé, mais il ne suffit pas à transformer un non-mage en lanceur de sorts.");
            addTransactionNote("Conseil : pour utiliser une magie hors classe, il faudra plutôt compter sur des parchemins à usage unique.");
            return;
        }

        if (id == "basic_magic_manual")
        {
            if (player.getLevel() < 3)
            {
                addTransactionNote("Étude partielle : niveau 3 requis pour stabiliser la première marque.");
                return;
            }
            player.unlockActiveSkill("learned_arcane_mark", "Marque élémentaire étudiée");
            addTransactionNote("Compétence étudiée : Marque élémentaire étudiée.");
            return;
        }

        if (id == "arcane_binding_grimoire")
        {
            if (player.getLevel() < 5)
            {
                addTransactionNote("Étude partielle : niveau 5 requis pour former les chaînes arcaniques.");
                return;
            }
            player.unlockActiveSkill("learned_arcane_binding", "Entrave arcanique étudiée");
            addTransactionNote("Compétence étudiée : Entrave arcanique étudiée.");
            return;
        }

        if (id == "elemental_ward_grimoire")
        {
            if (player.getLevel() < 6)
            {
                addTransactionNote("Étude partielle : niveau 6 requis pour tenir le voile élémentaire en combat.");
                return;
            }
            player.unlockActiveSkill("learned_elemental_ward", "Voile élémentaire étudié");
            addTransactionNote("Compétence étudiée : Voile élémentaire étudié.");
            return;
        }

        if (id == "resistance_rift_grimoire")
        {
            if (player.getLevel() < 9)
            {
                addTransactionNote("Étude partielle : niveau 9 requis pour supporter la faille décrite dans le grimoire.");
                return;
            }
            player.unlockActiveSkill("learned_resistance_rift", "Faille de résistance étudiée");
            addTransactionNote("Compétence étudiée : Faille de résistance étudiée.");
            return;
        }

        if (id == "frost_needle_grimoire")
        {
            if (player.getLevel() < 4)
            {
                addTransactionNote("Étude partielle : niveau 4 requis pour former les aiguilles de givre proprement.");
                return;
            }
            player.unlockActiveSkill("learned_frost_needle", "Aiguille de givre étudiée");
            addTransactionNote("Compétence étudiée : Aiguille de givre étudiée.");
            return;
        }

        if (id == "mana_suture_grimoire")
        {
            if (player.getLevel() < 7)
            {
                addTransactionNote("Étude partielle : niveau 7 requis pour étudier la suture de mana sans danger.");
                return;
            }
            player.unlockActiveSkill("learned_mana_suture", "Suture de mana étudiée");
            addTransactionNote("Compétence étudiée : Suture de mana étudiée.");
            return;
        }

        if (id == "occult_bramble_grimoire")
        {
            if (player.getLevel() < 5)
            {
                addTransactionNote("Étude partielle : niveau 5 requis pour stabiliser les ronces occultes.");
                return;
            }
            player.unlockActiveSkill("learned_occult_bramble", "Ronces occultes étudiées");
            addTransactionNote("Compétence étudiée : Ronces occultes étudiées.");
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
        || id == "stabilizing_tea"
        || id == "miner_bracing_tonic"
        || id == "cartographer_focus_ink"
        || id == "moon_salt_purifier"
        || id == "glass_step_scroll"
        || id == "vine_snare_scroll"
        || id == "archivist_focus_ink"
        || id == "cliff_basil_tea"
        || id == "carnival_diversion_ticket"
        || id == "firefly_guard_vial"
        || id == "rusty_sword"
        || id == "training_dagger"
        || id == "training_spear"
        || id == "training_bow"
        || id == "training_crossbow"
        || id == "training_throwing_bandolier"
        || id == "training_staff"
        || id == "heavy_training_axe"
        || id == "iron_sword"
        || id == "reinforced_dagger"
        || id == "guard_spear"
        || id == "hunting_bow"
        || id == "apprentice_staff"
        || id == "heavy_iron_axe"
        || id == "workshop_hammer"
        || id == "patrol_crossbow"
        || id == "balanced_rapier"
        || id == "mercenary_sabre"
        || id == "curved_ambush_dagger"
        || id == "militia_longbow"
        || id == "bound_oak_staff"
        || id == "runic_iron_blade"
        || id == "amber_edge_dagger"
        || id == "ashen_longbow"
        || id == "channeling_scepter"
        || id == "relay_falchion"
        || id == "whistling_mine_hammer"
        || id == "singing_resin_staff"
        || id == "cold_lantern_bow"
        || id == "red_clay_sabre"
        || id == "broken_map_dagger"
        || id == "firefly_iron_rapier"
        || id == "drowned_ledger_mace"
        || id == "grey_cliff_spear"
        || id == "broken_carnival_whip"
        || id == "worn_leather_armor"
        || id == "reinforced_leather_armor"
        || id == "guard_chainmail"
        || id == "runed_apprentice_robe"
        || id == "crude_plate_armor"
        || id == "traveler_scale_vest"
        || id == "threaded_rune_robe"
        || id == "militia_half_plate"
        || id == "runic_chainmail"
        || id == "shadow_thread_coat"
        || id == "polished_scale_harness"
        || id == "damaged_cartographer_coat"
        || id == "sun_dried_clay_breastplate"
        || id == "living_fiber_robe"
        || id == "whistling_miner_harness"
        || id == "drowned_archivist_vest"
        || id == "grey_drake_harness"
        || id == "patchwork_carnival_cape"
        || id == "goblin_ear"
        || id == "wolf_fang"
        || id == "predator_fang"
        || id == "boar_tusk"
        || id == "rodent_tooth"
        || id == "rusted_metal_fragment"
        || id == "worn_leather_piece"
        || id == "runic_iron_shard"
        || id == "polished_scale_plate"
        || id == "amber_tempering_oil"
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
        || id == "minor_fire_rune_note"
        || id == "minor_cold_rune_note"
        || id == "slime_residue"
        || id == "battle_torn_badge"
        || id == "weak_repair_kit"
        || id == "medium_repair_kit"
        || id == "big_repair_kit"
        || id == "tinkerer_complete_repair_kit"
        || id == "small_repair_kit"
        || id == "reinforced_repair_kit"
        || id == "special_adventurer_notes"
        || id == "special_identity_rumors"
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
        || id == "mycelium_lantern"
        || id == "echoing_resin"
        || id == "sun_dried_clay"
        || id == "moonlit_salt"
        || id == "old_coin_bundle"
        || id == "glass_map_fragment"
        || id == "living_vine_fiber"
        || id == "cold_iron_nail"
        || id == "witch_bottle"
        || id == "forgotten_camp_tag"
        || id == "tiny_gear_spring"
        || id == "inked_contract_scrap"
        || id == "firefly_iron_shell"
        || id == "whispering_archive_page"
        || id == "grey_drake_scale"
        || id == "carnival_ticket_shred"
        || id == "mirror_glass_bead"
        || id == "tideworn_ink"
        || id == "cliff_basil_leaf"
        || id == "rusted_gear_core"
        || id == "salted_rope_knot"
        || id == "luminous_moth_wing"
        || id == "slime_color_codex"
        || id == "monster_family_evolution_notes"
        || id == "weapon_training_notes"
        || id == "magic_learning_notes"
        || id == "elemental_weakness_notes"
        || id == "legend_child_tales"
        || id == "legend_trigger_notes"
        || id == "legend_storyteller_routes"
        || id == "curse_counter_rites_notes"
        || id == "city_service_stamp"
        || id == "municipal_proof_letter"
        || id == "lodging_bed_token"
        || id == "stable_stall_ticket"
        || id == "caravan_seat_ticket"
        || id == "guarded_transport_pass"
        || id == "route_toll_receipt"
        || id == "local_reputation_note"
        || id == "local_service_letter"
        || id == "holy_water_vial"
        || id == "sanctuary_candle"
        || id == "exorcist_note"
        || id == "travel_pass_note"
        || id == "warm_meal_voucher"
        || id == "client_recommendation";
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
        addTransactionNote("Stock épuisé pour " + item.getName() + ".");
        addTransactionNote("Il faudra attendre une nouvelle rotation de boutique.");
        return false;
    }

    if (!player.getInventory().spendGold(finalPrice))
    {
        addTransactionNote("Or insuffisant pour acheter " + item.getName() + ".");
        addTransactionNote("Or disponible : " + std::to_string(player.getInventory().getGold()) + " pièces.");
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
    else if (item.getId() == "vitality_healing_potion")
    {
        player.getInventory().addConsumable(ConsumableCatalog::createVitalityHealingPotion());
    }
    else if (item.getId() == "royal_vitality_healing_potion")
    {
        player.getInventory().addConsumable(ConsumableCatalog::createRoyalVitalityHealingPotion());
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
    else if (item.getId() == "stabilizing_tea")
    {
        player.getInventory().addConsumable(ConsumableCatalog::createStabilizingTea());
    }
    else if (item.getId() == "miner_bracing_tonic")
    {
        player.getInventory().addConsumable(ConsumableCatalog::createMinerBracingTonic());
    }
    else if (item.getId() == "cartographer_focus_ink")
    {
        player.getInventory().addConsumable(ConsumableCatalog::createCartographerFocusInk());
    }
    else if (item.getId() == "moon_salt_purifier")
    {
        player.getInventory().addConsumable(ConsumableCatalog::createMoonSaltPurifier());
    }
    else if (item.getId() == "glass_step_scroll")
    {
        player.getInventory().addConsumable(ConsumableCatalog::createGlassStepScroll());
    }
    else if (item.getId() == "vine_snare_scroll")
    {
        player.getInventory().addConsumable(ConsumableCatalog::createVineSnareScroll());
    }
    else if (item.getId() == "archivist_focus_ink")
    {
        player.getInventory().addConsumable(ConsumableCatalog::createArchivistFocusInk());
    }
    else if (item.getId() == "cliff_basil_tea")
    {
        player.getInventory().addConsumable(ConsumableCatalog::createCliffBasilTea());
    }
    else if (item.getId() == "carnival_diversion_ticket")
    {
        player.getInventory().addConsumable(ConsumableCatalog::createCarnivalDiversionTicket());
    }
    else if (item.getId() == "firefly_guard_vial")
    {
        player.getInventory().addConsumable(ConsumableCatalog::createFireflyGuardVial());
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
    else if (item.getId() == "iron_sword")
    {
        player.getInventory().addWeapon(WeaponCatalog::createIronSword());
    }
    else if (item.getId() == "reinforced_dagger")
    {
        player.getInventory().addWeapon(WeaponCatalog::createReinforcedDagger());
    }
    else if (item.getId() == "guard_spear")
    {
        player.getInventory().addWeapon(WeaponCatalog::createGuardSpear());
    }
    else if (item.getId() == "hunting_bow")
    {
        player.getInventory().addWeapon(WeaponCatalog::createHuntingBow());
    }
    else if (item.getId() == "apprentice_staff")
    {
        player.getInventory().addWeapon(WeaponCatalog::createApprenticeStaff());
    }
    else if (item.getId() == "heavy_iron_axe")
    {
        player.getInventory().addWeapon(WeaponCatalog::createHeavyIronAxe());
    }
    else if (item.getId() == "workshop_hammer")
    {
        player.getInventory().addWeapon(WeaponCatalog::createWorkshopHammer());
    }
    else if (item.getId() == "patrol_crossbow")
    {
        player.getInventory().addWeapon(WeaponCatalog::createPatrolCrossbow());
    }
    else if (item.getId() == "balanced_rapier")
    {
        player.getInventory().addWeapon(WeaponCatalog::createBalancedRapier());
    }
    else if (item.getId() == "mercenary_sabre")
    {
        player.getInventory().addWeapon(WeaponCatalog::createMercenarySabre());
    }
    else if (item.getId() == "curved_ambush_dagger")
    {
        player.getInventory().addWeapon(WeaponCatalog::createCurvedAmbushDagger());
    }
    else if (item.getId() == "militia_longbow")
    {
        player.getInventory().addWeapon(WeaponCatalog::createMilitiaLongbow());
    }
    else if (item.getId() == "bound_oak_staff")
    {
        player.getInventory().addWeapon(WeaponCatalog::createBoundOakStaff());
    }
    else if (item.getId() == "runic_iron_blade")
    {
        player.getInventory().addWeapon(WeaponCatalog::createRunicIronBlade());
    }
    else if (item.getId() == "amber_edge_dagger")
    {
        player.getInventory().addWeapon(WeaponCatalog::createAmberEdgeDagger());
    }
    else if (item.getId() == "ashen_longbow")
    {
        player.getInventory().addWeapon(WeaponCatalog::createAshenLongbow());
    }
    else if (item.getId() == "channeling_scepter")
    {
        player.getInventory().addWeapon(WeaponCatalog::createChannelingScepter());
    }
    else if (item.getId() == "relay_falchion")
    {
        player.getInventory().addWeapon(WeaponCatalog::createRelayFalchion());
    }
    else if (item.getId() == "whistling_mine_hammer")
    {
        player.getInventory().addWeapon(WeaponCatalog::createWhistlingMineHammer());
    }
    else if (item.getId() == "singing_resin_staff")
    {
        player.getInventory().addWeapon(WeaponCatalog::createSingingResinStaff());
    }
    else if (item.getId() == "cold_lantern_bow")
    {
        player.getInventory().addWeapon(WeaponCatalog::createColdLanternBow());
    }
    else if (item.getId() == "red_clay_sabre")
    {
        player.getInventory().addWeapon(WeaponCatalog::createRedClaySabre());
    }
    else if (item.getId() == "broken_map_dagger")
    {
        player.getInventory().addWeapon(WeaponCatalog::createBrokenMapDagger());
    }
    else if (item.getId() == "firefly_iron_rapier")
    {
        player.getInventory().addWeapon(WeaponCatalog::createFireflyIronRapier());
    }
    else if (item.getId() == "drowned_ledger_mace")
    {
        player.getInventory().addWeapon(WeaponCatalog::createDrownedLedgerMace());
    }
    else if (item.getId() == "grey_cliff_spear")
    {
        player.getInventory().addWeapon(WeaponCatalog::createGreyCliffSpear());
    }
    else if (item.getId() == "broken_carnival_whip")
    {
        player.getInventory().addWeapon(WeaponCatalog::createBrokenCarnivalWhip());
    }
    else if (item.getId() == "worn_leather_armor")
    {
        player.getInventory().addArmor(ArmorCatalog::createWornLeatherArmor());
    }
    else if (item.getId() == "reinforced_leather_armor")
    {
        player.getInventory().addArmor(ArmorCatalog::createReinforcedLeatherArmor());
    }
    else if (item.getId() == "guard_chainmail")
    {
        player.getInventory().addArmor(ArmorCatalog::createGuardChainmail());
    }
    else if (item.getId() == "runed_apprentice_robe")
    {
        player.getInventory().addArmor(ArmorCatalog::createRunedApprenticeRobe());
    }
    else if (item.getId() == "crude_plate_armor")
    {
        player.getInventory().addArmor(ArmorCatalog::createCrudePlateArmor());
    }
    else if (item.getId() == "traveler_scale_vest")
    {
        player.getInventory().addArmor(ArmorCatalog::createTravelerScaleVest());
    }
    else if (item.getId() == "threaded_rune_robe")
    {
        player.getInventory().addArmor(ArmorCatalog::createThreadedRuneRobe());
    }
    else if (item.getId() == "militia_half_plate")
    {
        player.getInventory().addArmor(ArmorCatalog::createMilitiaHalfPlate());
    }
    else if (item.getId() == "runic_chainmail")
    {
        player.getInventory().addArmor(ArmorCatalog::createRunicChainmail());
    }
    else if (item.getId() == "shadow_thread_coat")
    {
        player.getInventory().addArmor(ArmorCatalog::createShadowThreadCoat());
    }
    else if (item.getId() == "polished_scale_harness")
    {
        player.getInventory().addArmor(ArmorCatalog::createPolishedScaleHarness());
    }
    else if (item.getId() == "damaged_cartographer_coat")
    {
        player.getInventory().addArmor(ArmorCatalog::createDamagedCartographerCoat());
    }
    else if (item.getId() == "sun_dried_clay_breastplate")
    {
        player.getInventory().addArmor(ArmorCatalog::createSunDriedClayBreastplate());
    }
    else if (item.getId() == "living_fiber_robe")
    {
        player.getInventory().addArmor(ArmorCatalog::createLivingFiberRobe());
    }
    else if (item.getId() == "whistling_miner_harness")
    {
        player.getInventory().addArmor(ArmorCatalog::createWhistlingMinerHarness());
    }
    else if (item.getId() == "drowned_archivist_vest")
    {
        player.getInventory().addArmor(ArmorCatalog::createDrownedArchivistVest());
    }
    else if (item.getId() == "grey_drake_harness")
    {
        player.getInventory().addArmor(ArmorCatalog::createGreyDrakeHarness());
    }
    else if (item.getId() == "patchwork_carnival_cape")
    {
        player.getInventory().addArmor(ArmorCatalog::createPatchworkCarnivalCape());
    }
    else if (item.getId() == "cold_survival_parka")
    {
        player.getInventory().addArmor(ArmorCatalog::createColdSurvivalParka());
    }
    else if (item.getId() == "heat_survival_suit")
    {
        player.getInventory().addArmor(ArmorCatalog::createHeatSurvivalSuit());
    }
    else if (item.getId() == "insulated_explorer_coat")
    {
        player.getInventory().addArmor(ArmorCatalog::createInsulatedExplorerCoat());
    }
    else
    {
        Material boughtMaterial = createShopMaterialWithPossibleRareQuality(item);
        player.getInventory().addMaterial(boughtMaterial);

        if (boughtMaterial.hasSpecialQuality())
        {
            addTransactionNote("Trouvaille rare en boutique : qualité " + boughtMaterial.getQualityLabel() + ".");
            if (boughtMaterial.getQuality() == "exceptional")
            {
                addTransactionNote("Le marché noir te glisse quelque chose qu'aucun vendeur honnête n'aurait posé sur un comptoir.");
            }
            else
            {
                addTransactionNote("Les objets exceptionnels restent normalement impossibles à acheter hors circuits douteux.");
            }
        }
    }

    item.consumeOneStock();

    if (item.isCommonInformation())
    {
        BestiaryRuntimeProgress::unlockCommonInformation(item.getId());
        addTransactionNote("Renseignement ajouté au bestiaire pour cette session.");
    }

    applyMagicLearningEffect(player, item);

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
        basePrice = equipmentSellBaseValue(player.getInventory().getWeapon(index), shopType, player);
    }
    else if (shopType == ShopType::Armor && player.getInventory().hasArmor(index))
    {
        basePrice = equipmentSellBaseValue(player.getInventory().getArmor(index), shopType, player);
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
        addTransactionNote("Quantité invalide.");
        return false;
    }

    int maxQuantity = getMaxSellQuantityForEntry(player, shopType, index);
    if (maxQuantity <= 0 || quantity > maxQuantity)
    {
        addTransactionNote("Quantité impossible à vendre.");
        addTransactionNote("Maximum vendable : " + std::to_string(maxQuantity) + ".");
        return false;
    }

    if (quantity == 1)
    {
        return sellInventoryEntry(player, shopType, index, finalSellPrice);
    }

    if (!isMaterialShop(shopType))
    {
        addTransactionNote("Le comptoir refuse les lots pour cet objet. Vente d'un seul exemplaire.");
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
        addTransactionNote("La vente a échoué. Rien n'a été perdu.");
        return false;
    }

    int totalSellPrice = finalSellPrice * quantity;
    rememberSoldMaterial(shopType, soldMaterial, quantity, totalSellPrice);
    player.getInventory().earnGold(totalSellPrice);

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
        addTransactionNote("Impossible de vendre cette entrée.");
        addTransactionNote("L'équipement porté, les objets de base et les entrées invalides sont protégés.");
        return false;
    }

    std::string soldName = "Objet";
    bool removed = false;
    bool rememberWeapon = false;
    bool rememberArmor = false;
    bool rememberConsumable = false;
    bool rememberMaterial = false;
    Weapon soldWeapon;
    Armor soldArmor;
    Consumable soldConsumable;
    Material soldMaterial;

    if (shopType == ShopType::Weapon)
    {
        soldWeapon = player.getInventory().getWeapon(index);
        soldName = soldWeapon.getName();
        removed = player.getInventory().removeWeapon(index);
        rememberWeapon = removed;
    }
    else if (shopType == ShopType::Armor)
    {
        soldArmor = player.getInventory().getArmor(index);
        soldName = soldArmor.getName();
        removed = player.getInventory().removeArmor(index);
        rememberArmor = removed;
    }
    else if (shopType == ShopType::Consumable)
    {
        soldConsumable = player.getInventory().getConsumable(index);
        soldName = soldConsumable.getName();
        removed = player.getInventory().removeConsumable(index);
        rememberConsumable = removed;
    }
    else if (isMaterialShop(shopType))
    {
        soldMaterial = player.getInventory().getMaterial(index);
        soldName = soldMaterial.getName();
        if (soldMaterial.hasSpecialQuality())
        {
            soldName += " [" + soldMaterial.getQualityLabel() + "]";
        }
        removed = player.getInventory().removeMaterialQuantity(index, 1);
        rememberMaterial = removed;
    }

    if (!removed)
    {
        addTransactionNote("La vente a échoué. Rien n'a été perdu.");
        return false;
    }

    if (rememberWeapon)
    {
        rememberSoldWeapon(shopType, soldWeapon, finalSellPrice);
    }
    else if (rememberArmor)
    {
        rememberSoldArmor(shopType, soldArmor, finalSellPrice);
    }
    else if (rememberConsumable)
    {
        rememberSoldConsumable(shopType, soldConsumable, finalSellPrice);
    }
    else if (rememberMaterial)
    {
        rememberSoldMaterial(shopType, soldMaterial, 1, finalSellPrice);
    }

    player.getInventory().earnGold(finalSellPrice);

    return true;
}


int ShopTransactionSystem::getBuybackEntryCount(ShopType shopType)
{
    return static_cast<int>(visibleBuybackIndexes(shopType).size());
}

std::string ShopTransactionSystem::getBuybackEntryLabel(ShopType shopType, int visibleIndex)
{
    int ledgerIndex = -1;
    if (!resolveVisibleBuybackIndex(shopType, visibleIndex, ledgerIndex))
    {
        return "Entrée de rachat inconnue";
    }

    const BuybackEntry& entry = buybackLedger()[ledgerIndex];
    return entry.label
        + " | Rachat : " + std::to_string(entry.price) + " or"
        + " | Disponible jusqu'au prochain combat";
}


std::string ShopTransactionSystem::getBuybackEntryName(ShopType shopType, int visibleIndex)
{
    int ledgerIndex = -1;
    if (!resolveVisibleBuybackIndex(shopType, visibleIndex, ledgerIndex))
    {
        return "Entrée de rachat inconnue";
    }

    return buybackLedger()[ledgerIndex].label;
}

std::string ShopTransactionSystem::getBuybackEntryKindLabel(ShopType shopType, int visibleIndex)
{
    int ledgerIndex = -1;
    if (!resolveVisibleBuybackIndex(shopType, visibleIndex, ledgerIndex))
    {
        return "Inconnu";
    }

    switch (buybackLedger()[ledgerIndex].kind)
    {
        case BuybackItemKind::Weapon:
            return "Arme";
        case BuybackItemKind::Armor:
            return "Armure";
        case BuybackItemKind::Consumable:
            return "Consommable";
        case BuybackItemKind::Material:
            return "Matériau";
        default:
            return "Inconnu";
    }
}

int ShopTransactionSystem::getBuybackEntryQuantity(ShopType shopType, int visibleIndex)
{
    int ledgerIndex = -1;
    if (!resolveVisibleBuybackIndex(shopType, visibleIndex, ledgerIndex))
    {
        return 0;
    }

    return buybackLedger()[ledgerIndex].quantity;
}

int ShopTransactionSystem::getBuybackEntryPrice(ShopType shopType, int visibleIndex)
{
    int ledgerIndex = -1;
    if (!resolveVisibleBuybackIndex(shopType, visibleIndex, ledgerIndex))
    {
        return 0;
    }

    return buybackLedger()[ledgerIndex].price;
}

bool ShopTransactionSystem::buyBackEntry(
    Player& player,
    ShopType shopType,
    int visibleIndex
)
{
    int ledgerIndex = -1;
    if (!resolveVisibleBuybackIndex(shopType, visibleIndex, ledgerIndex))
    {
        addTransactionNote("Rachat impossible : l'entrée n'existe plus.");
        return false;
    }

    BuybackEntry entry = buybackLedger()[ledgerIndex];

    if (!player.getInventory().spendGold(entry.price))
    {
        addTransactionNote("Or insuffisant pour racheter " + entry.label + ".");
        addTransactionNote("Or disponible : " + std::to_string(player.getInventory().getGold()) + " pièces.");
        return false;
    }

    if (entry.kind == BuybackItemKind::Weapon)
    {
        player.getInventory().addWeapon(entry.weapon);
    }
    else if (entry.kind == BuybackItemKind::Armor)
    {
        player.getInventory().addArmor(entry.armor);
    }
    else if (entry.kind == BuybackItemKind::Consumable)
    {
        player.getInventory().addConsumable(entry.consumable);
    }
    else if (entry.kind == BuybackItemKind::Material)
    {
        player.getInventory().addMaterial(entry.material);
    }

    buybackLedger().erase(buybackLedger().begin() + ledgerIndex);

    return true;
}

void ShopTransactionSystem::clearBuybackAfterCombat()
{
    buybackLedger().clear();
}

void ShopTransactionSystem::clearLastTransactionNotes()
{
    transactionNotes().clear();
}

std::vector<std::string> ShopTransactionSystem::consumeLastTransactionNotes()
{
    std::vector<std::string> notes = transactionNotes();
    transactionNotes().clear();
    return notes;
}

void ShopTransactionSystem::displayUnsupportedPurchaseMessage(const ShopItem& item)
{
    addTransactionNote(item.getName() + " existe dans la boutique, mais ton sac ne peut pas le recevoir correctement.");
    addTransactionNote("Aucun or n'a été dépensé.");
}
