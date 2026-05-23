// EN: MaterialCatalog.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: MaterialCatalog.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.
// English: Implements early stackable materials, plants, books and information entries.
// Français : Implémente les premiers matériaux, plantes, livres et renseignements empilables.

#include "item/material/MaterialCatalog.hpp"

#include <algorithm>
#include <cctype>
#include <string>

namespace
{

    // EN: startsWith declares or implements a focused behavior used by this module.
    // FR: startsWith déclare ou implémente un comportement précis utilisé par ce module.
    bool startsWith(const std::string& value, const std::string& prefix)
    {
        return value.rfind(prefix, 0) == 0;
    }

    // EN: readTrailingDurability declares or implements a focused behavior used by this module.
    // FR: readTrailingDurability déclare ou implémente un comportement précis utilisé par ce module.
    int readTrailingDurability(const std::string& value)
    {
        std::size_t pos = value.find_last_of('_');

        if (pos == std::string::npos || pos + 1 >= value.size())
        {
            return 1;
        }

        try
        {
            return std::max(1, std::stoi(value.substr(pos + 1)));
        }
        catch (...)
        {
            return 1;
        }
    }

    // EN: createUsedRepairKit declares or implements a focused behavior used by this module.
    // FR: createUsedRepairKit déclare ou implémente un comportement précis utilisé par ce module.
    Material createUsedRepairKit(const std::string& baseId, const std::string& baseName, int maxDurability, int value, int quantity, int remainingDurability)
    {
        if (remainingDurability < 1)
        {
            remainingDurability = 1;
        }

        if (remainingDurability >= maxDurability)
        {
            remainingDurability = maxDurability - 1;
        }

        return Material(
            baseId + "_used_" + std::to_string(remainingDurability),
            baseName + " entamé [" + std::to_string(remainingDurability) + "/" + std::to_string(maxDurability) + "]",
            "Outil déjà utilisé : il est sorti du stock intact et conserve sa propre durabilité restante.",
            "Outil",
            value,
            quantity
        );
    }

    std::string normalizeId(std::string value)
    {
        std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });

        return value;
    }


    // EN: withQuality declares or implements a focused behavior used by this module.
    // FR: withQuality déclare ou implémente un comportement précis utilisé par ce module.
    Material withQuality(Material material, const std::string& quality)
    {
        if (material.getCategory() != "Outil"
            && material.getCategory() != "Livre"
            && material.getCategory() != "Renseignement")
        {
            material.setQuality(quality);
        }

        return material;
    }

}

// EN: createById declares or implements a focused behavior used by this module.
// FR: createById déclare ou implémente un comportement précis utilisé par ce module.
Material MaterialCatalog::createById(const std::string& id, int quantity, const std::string& quality)
{
    std::string normalized = normalizeId(id);

    if (normalized == "goblin_ear") return withQuality(createGoblinEar(quantity), quality);
    if (normalized == "wolf_fang") return withQuality(createWolfFang(quantity), quality);
    if (normalized == "rusted_metal_fragment") return withQuality(createRustedMetalFragment(quantity), quality);
    if (normalized == "worn_leather_piece") return withQuality(createWornLeatherPiece(quantity), quality);
    if (normalized == "mountain_blue_flower") return withQuality(createMountainBlueFlower(quantity), quality);
    if (normalized == "bitter_healing_leaf") return withQuality(createBitterHealingLeaf(quantity), quality);
    if (normalized == "common_goblin_notes") return withQuality(createCommonGoblinNotes(quantity), quality);
    if (normalized == "common_wolf_notes") return withQuality(createCommonWolfNotes(quantity), quality);
    if (normalized == "basic_plant_manual") return withQuality(createBasicPlantManual(quantity), quality);
    if (normalized == "basic_magic_manual") return withQuality(createBasicMagicManual(quantity), quality);
    if (normalized == "cracked_bone") return withQuality(createCrackedBone(quantity), quality);
    if (normalized == "arcane_dust") return withQuality(createArcaneDust(quantity), quality);
    if (normalized == "slime_residue") return withQuality(createSlimeResidue(quantity), quality);
    if (normalized == "variation_residue") return withQuality(createVariationResidue(quantity), quality);
    if (normalized == "battle_torn_badge") return withQuality(createBattleTornBadge(quantity), quality);

    if (startsWith(normalized, "weak_repair_kit_used_")) return withQuality(createUsedRepairKit("weak_repair_kit", "Kit de réparation faible", 2, 9, quantity, readTrailingDurability(normalized)), quality);
    if (startsWith(normalized, "medium_repair_kit_used_")) return withQuality(createUsedRepairKit("medium_repair_kit", "Kit de réparation moyen", 3, 18, quantity, readTrailingDurability(normalized)), quality);
    if (startsWith(normalized, "big_repair_kit_used_")) return withQuality(createUsedRepairKit("big_repair_kit", "Gros kit de réparation", 4, 30, quantity, readTrailingDurability(normalized)), quality);
    if (startsWith(normalized, "tinkerer_complete_repair_kit_used_")) return withQuality(createUsedRepairKit("tinkerer_complete_repair_kit", "Kit complet du bricoleur", 5, 48, quantity, readTrailingDurability(normalized)), quality);
    if (normalized == "weak_repair_kit") return withQuality(createWeakRepairKit(quantity), quality);
    if (normalized == "medium_repair_kit") return withQuality(createMediumRepairKit(quantity), quality);
    if (normalized == "big_repair_kit") return withQuality(createBigRepairKit(quantity), quality);
    if (normalized == "tinkerer_complete_repair_kit") return withQuality(createTinkererCompleteRepairKit(quantity), quality);
    if (normalized == "small_repair_kit") return withQuality(createSmallRepairKit(quantity), quality);
    if (normalized == "reinforced_repair_kit") return withQuality(createReinforcedRepairKit(quantity), quality);
    if (normalized == "special_adventurer_notes") return withQuality(createSpecialAdventurerNotes(quantity), quality);
    if (normalized == "summoning_notes") return withQuality(createSummoningNotes(quantity), quality);
    if (normalized == "boss_identity_scrap") return withQuality(createBossIdentityScrap(quantity), quality);
    if (normalized == "potion_recipe_page") return withQuality(createPotionRecipePage(quantity), quality);
    if (normalized == "repair_recipe_page") return withQuality(createRepairRecipePage(quantity), quality);
    if (normalized == "advanced_monster_notes") return withQuality(createAdvancedMonsterNotes(quantity), quality);
    if (normalized == "necromancy_warning") return withQuality(createNecromancyWarning(quantity), quality);
    if (normalized == "beast_hide") return withQuality(createBeastHide(quantity), quality);
    if (normalized == "shadow_thread") return withQuality(createShadowThread(quantity), quality);
    if (normalized == "kitsune_ember") return withQuality(createKitsuneEmber(quantity), quality);
    if (normalized == "draconic_scale_fragment") return withQuality(createDraconicScaleFragment(quantity), quality);
    if (normalized == "unstable_core") return withQuality(createUnstableCore(quantity), quality);
    if (normalized == "fitoria_feather") return withQuality(createFitoriaFeather(quantity), quality);
    if (normalized == "zelef_demon_blood") return withQuality(createZelefDemonBlood(quantity), quality);
    if (normalized == "atlas_broken_plate") return withQuality(createAtlasBrokenPlate(quantity), quality);
    if (normalized == "lyknir_hunt_shard") return withQuality(createLyknirHuntShard(quantity), quality);
    if (normalized == "grinka_avarice_coin") return withQuality(createGrinkaAvariceCoin(quantity), quality);
    if (normalized == "azelanos_dark_crown_shard") return withQuality(createAzelanosDarkCrownShard(quantity), quality);
    if (normalized == "thamarys_origin_scale") return withQuality(createThamarysOriginScale(quantity), quality);
    if (normalized == "mojo_ancient_seed") return withQuality(createMojoAncientSeed(quantity), quality);
    if (normalized == "inakari_mirror_shard") return withQuality(createInakariMirrorShard(quantity), quality);
    if (normalized == "silent_judgment_seal") return withQuality(createSilentJudgmentSeal(quantity), quality);
    if (normalized == "anomaly_glitch_fragment") return withQuality(createAnomalyGlitchFragment(quantity), quality);
    if (normalized == "dead_minute_gear") return withQuality(createDeadMinuteGear(quantity), quality);
    if (normalized == "buried_bone_lullaby") return withQuality(createBuriedBoneLullaby(quantity), quality);
    if (normalized == "boros_war_mark") return withQuality(createBorosWarMark(quantity), quality);
    if (normalized == "anastasia_bound_heart") return withQuality(createAnastasiaBoundHeart(quantity), quality);
    if (normalized == "lexior_justice_splinter") return withQuality(createLexiorJusticeSplinter(quantity), quality);
    if (normalized == "lunar_dream_fragment") return withQuality(createLunarDreamFragment(quantity), quality);
    if (normalized == "elemental_fusion_core") return withQuality(createElementalFusionCore(quantity), quality);
    if (normalized == "human_will_fragment") return withQuality(createHumanWillFragment(quantity), quality);
    if (normalized == "conscious_luck_shard") return withQuality(createConsciousLuckShard(quantity), quality);
    if (normalized == "progression_seal") return withQuality(createProgressionSeal(quantity), quality);
    if (normalized == "absent_throne_fragment") return withQuality(createAbsentThroneFragment(quantity), quality);
    if (normalized == "lost_name_fragment") return withQuality(createLostNameFragment(quantity), quality);
    if (normalized == "faceless_breath") return withQuality(createFacelessBreath(quantity), quality);
    if (normalized == "puppet_nail") return withQuality(createPuppetNail(quantity), quality);
    if (normalized == "moiran_fate_thread") return withQuality(createMoiranFateThread(quantity), quality);
    if (normalized == "lost_soul_antler") return withQuality(createLostSoulAntler(quantity), quality);
    if (normalized == "gorvald_royal_blood") return withQuality(createGorvaldRoyalBlood(quantity), quality);
    if (normalized == "serana_royal_fang") return withQuality(createSeranaRoyalFang(quantity), quality);
    if (normalized == "draiite_black_silk") return withQuality(createDraiiteBlackSilk(quantity), quality);
    if (normalized == "broken_mirror_shard") return withQuality(createBrokenMirrorShard(quantity), quality);
    if (normalized == "unstable_version_core") return withQuality(createUnstableVersionCore(quantity), quality);
    if (normalized == "precision_harvest_tools") return withQuality(createPrecisionHarvestTools(quantity), quality);
    if (normalized == "preservation_vials") return withQuality(createPreservationVials(quantity), quality);
    if (normalized == "clean_harvest_manual") return withQuality(createCleanHarvestManual(quantity), quality);
    if (normalized == "monster_dissection_guide") return withQuality(createMonsterDissectionGuide(quantity), quality);
    if (normalized == "training_arrows") return withQuality(createTrainingArrows(quantity), quality);
    if (normalized == "training_bolts") return withQuality(createTrainingBolts(quantity), quality);
    if (normalized == "training_throwing_knives") return withQuality(createTrainingThrowingKnives(quantity), quality);
    if (normalized == "barbed_arrows") return withQuality(createBarbedArrows(quantity), quality);
    if (normalized == "piercing_bolts") return withQuality(createPiercingBolts(quantity), quality);
    if (normalized == "balanced_throwing_knives") return withQuality(createBalancedThrowingKnives(quantity), quality);
    if (normalized == "ash_arrows") return withQuality(createAshArrows(quantity), quality);
    if (normalized == "frozen_bolts") return withQuality(createFrozenBolts(quantity), quality);
    if (normalized == "conductive_knives") return withQuality(createConductiveKnives(quantity), quality);

    return Material(
        id,
        "Renseignement inconnu",
        "Une entrée préparée, mais pas encore détaillée dans le catalogue.",
        "Inconnu",
        0,
        quantity
    );
}

// EN: createGoblinEar declares or implements a focused behavior used by this module.
// FR: createGoblinEar déclare ou implémente un comportement précis utilisé par ce module.
Material MaterialCatalog::createGoblinEar(int quantity)
{
    return Material("goblin_ear", "Oreille de gobelin", "Matériau commun récupéré sur certains gobelins.", "Matériau de monstre", 4, quantity);
}

// EN: createWolfFang declares or implements a focused behavior used by this module.
// FR: createWolfFang déclare ou implémente un comportement précis utilisé par ce module.
Material MaterialCatalog::createWolfFang(int quantity)
{
    return Material("wolf_fang", "Croc de loup", "Matériau utile pour de petites améliorations d'armes.", "Matériau de monstre", 6, quantity);
}

// EN: createRustedMetalFragment declares or implements a focused behavior used by this module.
// FR: createRustedMetalFragment déclare ou implémente un comportement précis utilisé par ce module.
Material MaterialCatalog::createRustedMetalFragment(int quantity)
{
    return Material("rusted_metal_fragment", "Fragment de métal rouillé", "Reste d'équipement brisé, encore utile pour des réparations basiques.", "Matériau", 3, quantity);
}

// EN: createWornLeatherPiece declares or implements a focused behavior used by this module.
// FR: createWornLeatherPiece déclare ou implémente un comportement précis utilisé par ce module.
Material MaterialCatalog::createWornLeatherPiece(int quantity)
{
    return Material("worn_leather_piece", "Morceau de cuir abîmé", "Matière simple pour armures légères.", "Matériau", 5, quantity);
}

// EN: createMountainBlueFlower declares or implements a focused behavior used by this module.
// FR: createMountainBlueFlower déclare ou implémente un comportement précis utilisé par ce module.
Material MaterialCatalog::createMountainBlueFlower(int quantity)
{
    return Material("mountain_blue_flower", "Fleur bleue de montagne", "Une fleur rare et calme, souvent liée aux remèdes et aux secrets anciens.", "Plante", 10, quantity);
}

// EN: createBitterHealingLeaf declares or implements a focused behavior used by this module.
// FR: createBitterHealingLeaf déclare ou implémente un comportement précis utilisé par ce module.
Material MaterialCatalog::createBitterHealingLeaf(int quantity)
{
    return Material("bitter_healing_leaf", "Feuille amère de soin", "Plante commune utilisée dans les potions simples.", "Plante", 5, quantity);
}

// EN: createCommonGoblinNotes declares or implements a focused behavior used by this module.
// FR: createCommonGoblinNotes déclare ou implémente un comportement précis utilisé par ce module.
Material MaterialCatalog::createCommonGoblinNotes(int quantity)
{
    return Material("common_goblin_notes", "Notes communes sur les gobelins", "Renseignement acheté : les gobelins communs préfèrent survivre plutôt que mourir héroïquement.", "Renseignement", 0, quantity);
}

// EN: createCommonWolfNotes declares or implements a focused behavior used by this module.
// FR: createCommonWolfNotes déclare ou implémente un comportement précis utilisé par ce module.
Material MaterialCatalog::createCommonWolfNotes(int quantity)
{
    return Material("common_wolf_notes", "Notes communes sur les loups", "Renseignement acheté : les loups testent souvent les proies blessées ou isolées.", "Renseignement", 0, quantity);
}

// EN: createBasicPlantManual declares or implements a focused behavior used by this module.
// FR: createBasicPlantManual déclare ou implémente un comportement précis utilisé par ce module.
Material MaterialCatalog::createBasicPlantManual(int quantity)
{
    return Material("basic_plant_manual", "Petit guide des plantes communes", "Livre de terrain préparant le futur journal des plantes.", "Livre", 10, quantity);
}

// EN: createBasicMagicManual declares or implements a focused behavior used by this module.
// FR: createBasicMagicManual déclare ou implémente un comportement précis utilisé par ce module.
Material MaterialCatalog::createBasicMagicManual(int quantity)
{
    return Material("basic_magic_manual", "Manuel de magie basique", "Livre simple qui servira plus tard aux premières notions de magie.", "Livre", 10, quantity);
}

// EN: createCrackedBone declares or implements a focused behavior used by this module.
// FR: createCrackedBone déclare ou implémente un comportement précis utilisé par ce module.
Material MaterialCatalog::createCrackedBone(int quantity)
{
    return Material("cracked_bone", "Os fissuré", "Matériau récupéré sur les morts-vivants. Plus tard, il servira à la nécromancie et à certains crafts sombres.", "Matériau de monstre", 7, quantity);
}

// EN: createArcaneDust declares or implements a focused behavior used by this module.
// FR: createArcaneDust déclare ou implémente un comportement précis utilisé par ce module.
Material MaterialCatalog::createArcaneDust(int quantity)
{
    return Material("arcane_dust", "Poussière arcanique", "Résidu magique instable, utile pour les futurs enchantements et composants de sorts.", "Matériau magique", 12, quantity);
}

// EN: createSlimeResidue declares or implements a focused behavior used by this module.
// FR: createSlimeResidue déclare ou implémente un comportement précis utilisé par ce module.
Material MaterialCatalog::createSlimeResidue(int quantity)
{
    return Material("slime_residue", "Résidu de slime", "Matière collante utile pour potions, pièges et réparations de fortune.", "Matériau de monstre", 4, quantity);
}

// EN: createVariationResidue declares or implements a focused behavior used by this module.
// FR: createVariationResidue déclare ou implémente un comportement précis utilisé par ce module.
Material MaterialCatalog::createVariationResidue(int quantity)
{
    return Material("variation_residue", "Résidu de variation", "Fragment instable laissé par une anomalie locale ou un monstre évolué.", "Matériau anormal", 18, quantity);
}

// EN: createBattleTornBadge declares or implements a focused behavior used by this module.
// FR: createBattleTornBadge déclare ou implémente un comportement précis utilisé par ce module.
Material MaterialCatalog::createBattleTornBadge(int quantity)
{
    return Material("battle_torn_badge", "Insigne abîmé d'aventurier", "Preuve laissée par un combattant humain ou semi-humain. Peut servir plus tard aux réputations, guildes ou contrats.", "Trophée", 15, quantity);
}

// EN: createWeakRepairKit declares or implements a focused behavior used by this module.
// FR: createWeakRepairKit déclare ou implémente un comportement précis utilisé par ce module.
Material MaterialCatalog::createWeakRepairKit(int quantity)
{
    return Material("weak_repair_kit", "Kit de réparation faible", "Outil fragile intact : permet une réparation autonome jusqu'à +25% environ. Durabilité maximale : 2 utilisations.", "Outil", 18, quantity);
}

// EN: createMediumRepairKit declares or implements a focused behavior used by this module.
// FR: createMediumRepairKit déclare ou implémente un comportement précis utilisé par ce module.
Material MaterialCatalog::createMediumRepairKit(int quantity)
{
    return Material("medium_repair_kit", "Kit de réparation moyen", "Outil correct intact : permet une réparation autonome jusqu'à +50% environ. Durabilité maximale : 3 utilisations.", "Outil", 45, quantity);
}

// EN: createBigRepairKit declares or implements a focused behavior used by this module.
// FR: createBigRepairKit déclare ou implémente un comportement précis utilisé par ce module.
Material MaterialCatalog::createBigRepairKit(int quantity)
{
    return Material("big_repair_kit", "Gros kit de réparation", "Outil robuste intact : permet une réparation autonome jusqu'à +75% environ. Durabilité maximale : 4 utilisations.", "Outil", 90, quantity);
}

// EN: createTinkererCompleteRepairKit declares or implements a focused behavior used by this module.
// FR: createTinkererCompleteRepairKit déclare ou implémente un comportement précis utilisé par ce module.
Material MaterialCatalog::createTinkererCompleteRepairKit(int quantity)
{
    return Material("tinkerer_complete_repair_kit", "Kit complet du bricoleur", "Outil presque complet intact : permet une réparation autonome jusqu'à +95% environ. Durabilité maximale : 5 utilisations.", "Outil", 160, quantity);
}

// EN: createSmallRepairKit declares or implements a focused behavior used by this module.
// FR: createSmallRepairKit déclare ou implémente un comportement précis utilisé par ce module.
Material MaterialCatalog::createSmallRepairKit(int quantity)
{
    return createWeakRepairKit(quantity);
}

// EN: createReinforcedRepairKit declares or implements a focused behavior used by this module.
// FR: createReinforcedRepairKit déclare ou implémente un comportement précis utilisé par ce module.
Material MaterialCatalog::createReinforcedRepairKit(int quantity)
{
    return createMediumRepairKit(quantity);
}


// EN: createSpecialAdventurerNotes declares or implements a focused behavior used by this module.
// FR: createSpecialAdventurerNotes déclare ou implémente un comportement précis utilisé par ce module.
Material MaterialCatalog::createSpecialAdventurerNotes(int quantity)
{
    return Material("special_adventurer_notes", "Notes sur les groupes spéciaux", "Renseignement : certains aventuriers ont des relations qui changent le danger réel du combat.", "Renseignement", 0, quantity);
}

// EN: createSummoningNotes declares or implements a focused behavior used by this module.
// FR: createSummoningNotes déclare ou implémente un comportement précis utilisé par ce module.
Material MaterialCatalog::createSummoningNotes(int quantity)
{
    return Material("summoning_notes", "Manuel d'invocations instables", "Livre préparant les futurs slots, coûts de mana, sacrifices et évolutions d'invocations.", "Livre", 12, quantity);
}

// EN: createBossIdentityScrap declares or implements a focused behavior used by this module.
// FR: createBossIdentityScrap déclare ou implémente un comportement précis utilisé par ce module.
Material MaterialCatalog::createBossIdentityScrap(int quantity)
{
    return Material("boss_identity_scrap", "Fragment d'identité de boss", "Indice incomplet : un boss ne révèle vraiment son nom que lorsqu'il le prononce lui-même.", "Renseignement", 0, quantity);
}

// EN: createPotionRecipePage declares or implements a focused behavior used by this module.
// FR: createPotionRecipePage déclare ou implémente un comportement précis utilisé par ce module.
Material MaterialCatalog::createPotionRecipePage(int quantity)
{
    return Material("potion_recipe_page", "Page de recettes d'alchimie", "Page de bibliothèque détaillant les mélanges simples : soin, rage et versions renforcées.", "Livre", 14, quantity);
}

// EN: createRepairRecipePage declares or implements a focused behavior used by this module.
// FR: createRepairRecipePage déclare ou implémente un comportement précis utilisé par ce module.
Material MaterialCatalog::createRepairRecipePage(int quantity)
{
    return Material("repair_recipe_page", "Page de réparation de fortune", "Page de bibliothèque expliquant comment assembler un kit de réparation basique avec du métal, du cuir et du slime.", "Livre", 14, quantity);
}

// EN: createAdvancedMonsterNotes declares or implements a focused behavior used by this module.
// FR: createAdvancedMonsterNotes déclare ou implémente un comportement précis utilisé par ce module.
Material MaterialCatalog::createAdvancedMonsterNotes(int quantity)
{
    return Material("advanced_monster_notes", "Notes avancées sur les monstres", "Renseignement décrivant les loots et comportements de monstres plus dangereux.", "Renseignement", 0, quantity);
}

// EN: createNecromancyWarning declares or implements a focused behavior used by this module.
// FR: createNecromancyWarning déclare ou implémente un comportement précis utilisé par ce module.
Material MaterialCatalog::createNecromancyWarning(int quantity)
{
    return Material("necromancy_warning", "Avertissement nécromantique", "Renseignement sombre : manipuler les os et les ombres peut attirer des effets qui ne pardonnent pas.", "Renseignement", 0, quantity);
}

// EN: createBeastHide declares or implements a focused behavior used by this module.
// FR: createBeastHide déclare ou implémente un comportement précis utilisé par ce module.
Material MaterialCatalog::createBeastHide(int quantity)
{
    return Material("beast_hide", "Peau de bête robuste", "Matériau solide récupéré sur des bêtes résistantes. Utile pour armures, réparations épaisses et futurs crafts de tanks.", "Matériau de monstre", 13, quantity);
}

// EN: createShadowThread declares or implements a focused behavior used by this module.
// FR: createShadowThread déclare ou implémente un comportement précis utilisé par ce module.
Material MaterialCatalog::createShadowThread(int quantity)
{
    return Material("shadow_thread", "Fil d'ombre", "Composant sombre et rare, lié aux assassins, aux morts-vivants et aux futures ombres de Hazak.", "Matériau sombre", 28, quantity);
}

// EN: createKitsuneEmber declares or implements a focused behavior used by this module.
// FR: createKitsuneEmber déclare ou implémente un comportement précis utilisé par ce module.
Material MaterialCatalog::createKitsuneEmber(int quantity)
{
    return Material("kitsune_ember", "Braise kitsune", "Braise magique instable, utile aux futures flammes d'Aoi, aux invocations et aux potions avancées.", "Matériau magique", 34, quantity);
}

// EN: createDraconicScaleFragment declares or implements a focused behavior used by this module.
// FR: createDraconicScaleFragment déclare ou implémente un comportement précis utilisé par ce module.
Material MaterialCatalog::createDraconicScaleFragment(int quantity)
{
    return Material("draconic_scale_fragment", "Fragment d'écaille draconique", "Fragment extrêmement résistant, prévu pour les protections rares et certains crafts semi-dragons.", "Matériau rare", 45, quantity);
}

// EN: createUnstableCore declares or implements a focused behavior used by this module.
// FR: createUnstableCore déclare ou implémente un comportement précis utilisé par ce module.
Material MaterialCatalog::createUnstableCore(int quantity)
{
    return Material("unstable_core", "Noyau instable", "Cœur magique dangereux, prévu pour invocations, alchimie risquée et enchantements expérimentaux.", "Matériau rare", 40, quantity);
}


// EN: createFitoriaFeather declares or implements a focused behavior used by this module.
// FR: createFitoriaFeather déclare ou implémente un comportement précis utilisé par ce module.
Material MaterialCatalog::createFitoriaFeather(int quantity)
{
    return Material("fitoria_feather", "Plume lumineuse de Fitoria", "Fragment de boss : plume presque sacrée, utile plus tard pour bénédictions, soins rares et équipements de lumière.", "Fragment de boss", 120, quantity);
}

// EN: createZelefDemonBlood declares or implements a focused behavior used by this module.
// FR: createZelefDemonBlood déclare ou implémente un comportement précis utilisé par ce module.
Material MaterialCatalog::createZelefDemonBlood(int quantity)
{
    return Material("zelef_demon_blood", "Sang démoniaque de Zelef", "Fragment de boss : résidu sombre encore vivant, prévu pour alchimie dangereuse, malédictions et armes démoniaques.", "Fragment de boss", 135, quantity);
}

// EN: createAtlasBrokenPlate declares or implements a focused behavior used by this module.
// FR: createAtlasBrokenPlate déclare ou implémente un comportement précis utilisé par ce module.
Material MaterialCatalog::createAtlasBrokenPlate(int quantity)
{
    return Material("atlas_broken_plate", "Plaque brisée d'Atlas", "Fragment de boss : morceau d'une protection universelle déchue, prévu pour armures lourdes, reliques et réparations extrêmes.", "Fragment de boss", 150, quantity);
}

// EN: createLyknirHuntShard declares or implements a focused behavior used by this module.
// FR: createLyknirHuntShard déclare ou implémente un comportement précis utilisé par ce module.
Material MaterialCatalog::createLyknirHuntShard(int quantity)
{
    return Material("lyknir_hunt_shard", "Fragment de chasse silencieuse", "Fragment de boss : morceau d'une traque sans lune, lié aux marques de proie, aux meutes et aux futurs crafts de chasseurs.", "Fragment de boss", 165, quantity);
}

// EN: createGrinkaAvariceCoin declares or implements a focused behavior used by this module.
// FR: createGrinkaAvariceCoin déclare ou implémente un comportement précis utilisé par ce module.
Material MaterialCatalog::createGrinkaAvariceCoin(int quantity)
{
    return Material("grinka_avarice_coin", "Pièce d'avarice tordue", "Fragment de boss : pièce impossible à compter deux fois de la même façon, liée aux dettes, taxes et contrats gobelins.", "Fragment de boss", 155, quantity);
}

// EN: createAzelanosDarkCrownShard declares or implements a focused behavior used by this module.
// FR: createAzelanosDarkCrownShard déclare ou implémente un comportement précis utilisé par ce module.
Material MaterialCatalog::createAzelanosDarkCrownShard(int quantity)
{
    return Material("azelanos_dark_crown_shard", "Éclat d'Azelanos instable", "Fragment de boss : éclat d'une couronne noire affaiblie, chargé d'obscurité croissante.", "Fragment de boss", 190, quantity);
}

// EN: createThamarysOriginScale declares or implements a focused behavior used by this module.
// FR: createThamarysOriginScale déclare ou implémente un comportement précis utilisé par ce module.
Material MaterialCatalog::createThamarysOriginScale(int quantity)
{
    return Material("thamarys_origin_scale", "Écaille d'origine de Thamarys", "Fragment de boss : écaille issue d'un souffle draconique ancien, prévue pour armes et armures draconiques rares.", "Fragment de boss", 225, quantity);
}

// EN: createMojoAncientSeed declares or implements a focused behavior used by this module.
// FR: createMojoAncientSeed déclare ou implémente un comportement précis utilisé par ce module.
Material MaterialCatalog::createMojoAncientSeed(int quantity)
{
    return Material("mojo_ancient_seed", "Graine ancienne de Mojo", "Fragment de boss : graine vivante gardant une mémoire de la forêt, prévue pour bénédictions naturelles et crafts respectueux.", "Fragment de boss", 205, quantity);
}

// EN: createInakariMirrorShard declares or implements a focused behavior used by this module.
// FR: createInakariMirrorShard déclare ou implémente un comportement précis utilisé par ce module.
Material MaterialCatalog::createInakariMirrorShard(int quantity)
{
    return Material("inakari_mirror_shard", "Fragment de miroir d'Inakari", "Fragment de boss : éclat de reflet kitsune, lié aux illusions, aux talismans et aux futures protections mentales.", "Fragment de boss", 200, quantity);
}



// EN: createSilentJudgmentSeal declares or implements a focused behavior used by this module.
// FR: createSilentJudgmentSeal déclare ou implémente un comportement précis utilisé par ce module.
Material MaterialCatalog::createSilentJudgmentSeal(int quantity)
{
    return Material("silent_judgment_seal", "Sceau du Jugement Silencieux", "Fragment de boss : sceau muet lié aux ombres, aux fautes répétées et aux futurs crafts de justice.", "Fragment de boss", 230, quantity);
}

// EN: createAnomalyGlitchFragment declares or implements a focused behavior used by this module.
// FR: createAnomalyGlitchFragment déclare ou implémente un comportement précis utilisé par ce module.
Material MaterialCatalog::createAnomalyGlitchFragment(int quantity)
{
    return Material("anomaly_glitch_fragment", "Fragment glitch de l'Anomalie", "Fragment de boss : morceau instable d'interface corrompue. Il semble parfois changer de nom tout seul.", "Fragment de boss", 250, quantity);
}

// EN: createDeadMinuteGear declares or implements a focused behavior used by this module.
// FR: createDeadMinuteGear déclare ou implémente un comportement précis utilisé par ce module.
Material MaterialCatalog::createDeadMinuteGear(int quantity)
{
    return Material("dead_minute_gear", "Rouage de minute morte", "Fragment de boss : rouage temporel qui conserve des blessures avant même qu'elles existent.", "Fragment de boss", 235, quantity);
}

// EN: createBuriedBoneLullaby declares or implements a focused behavior used by this module.
// FR: createBuriedBoneLullaby déclare ou implémente un comportement précis utilisé par ce module.
Material MaterialCatalog::createBuriedBoneLullaby(int quantity)
{
    return Material("buried_bone_lullaby", "Fragment de berceuse enterrée", "Fragment de boss : os ancien rempli d'une chanson sans voix, lié aux tombes et au sable.", "Fragment de boss", 240, quantity);
}

// EN: createBorosWarMark declares or implements a focused behavior used by this module.
// FR: createBorosWarMark déclare ou implémente un comportement précis utilisé par ce module.
Material MaterialCatalog::createBorosWarMark(int quantity)
{
    return Material("boros_war_mark", "Marque du duel éternel", "Fragment de boss : marque guerrière laissée par un avatar affaibli de Boros.", "Fragment de boss", 275, quantity);
}

// EN: createAnastasiaBoundHeart declares or implements a focused behavior used by this module.
// FR: createAnastasiaBoundHeart déclare ou implémente un comportement précis utilisé par ce module.
Material MaterialCatalog::createAnastasiaBoundHeart(int quantity)
{
    return Material("anastasia_bound_heart", "Coeur lié fissuré", "Fragment de boss : matière émotionnelle liée aux promesses, à la survie et à l'amour de la vie.", "Fragment de boss", 270, quantity);
}

// EN: createLexiorJusticeSplinter declares or implements a focused behavior used by this module.
// FR: createLexiorJusticeSplinter déclare ou implémente un comportement précis utilisé par ce module.
Material MaterialCatalog::createLexiorJusticeSplinter(int quantity)
{
    return Material("lexior_justice_splinter", "Éclat de verdict de Lexior", "Fragment de boss : éclat de justice froide capable de sceller ce qui a trop servi.", "Fragment de boss", 290, quantity);
}

// EN: createLunarDreamFragment declares or implements a focused behavior used by this module.
// FR: createLunarDreamFragment déclare ou implémente un comportement précis utilisé par ce module.
Material MaterialCatalog::createLunarDreamFragment(int quantity)
{
    return Material("lunar_dream_fragment", "Fragment de rêve lunaire", "Fragment de boss : morceau de rêve et de cauchemar où Luna et Onyrae se superposent.", "Fragment de boss", 285, quantity);
}

// EN: createElementalFusionCore declares or implements a focused behavior used by this module.
// FR: createElementalFusionCore déclare ou implémente un comportement précis utilisé par ce module.
Material MaterialCatalog::createElementalFusionCore(int quantity)
{
    return Material("elemental_fusion_core", "Noyau de fusion élémentaire", "Fragment de boss : coeur instable où Feu, Terre, Eau et Vent refusent de rester séparés.", "Fragment de boss", 310, quantity);
}

// EN: createHumanWillFragment declares or implements a focused behavior used by this module.
// FR: createHumanWillFragment déclare ou implémente un comportement précis utilisé par ce module.
Material MaterialCatalog::createHumanWillFragment(int quantity)
{
    return Material("human_will_fragment", "Fragment de volonté humaine", "Fragment de boss : morceau d'adaptation humaine qui refuse de rester à terre.", "Fragment de boss", 300, quantity);
}

// EN: createConsciousLuckShard declares or implements a focused behavior used by this module.
// FR: createConsciousLuckShard déclare ou implémente un comportement précis utilisé par ce module.
Material MaterialCatalog::createConsciousLuckShard(int quantity)
{
    return Material("conscious_luck_shard", "Éclat de hasard conscient", "Fragment de boss : chance presque vivante, capable de rendre rare ce qui était commun.", "Fragment de boss", 305, quantity);
}

// EN: createProgressionSeal declares or implements a focused behavior used by this module.
// FR: createProgressionSeal déclare ou implémente un comportement précis utilisé par ce module.
Material MaterialCatalog::createProgressionSeal(int quantity)
{
    return Material("progression_seal", "Sceau de progression", "Fragment de boss : verrou de seuil brisé, preuve qu'une limite a été forcée.", "Fragment de boss", 315, quantity);
}

// EN: createAbsentThroneFragment declares or implements a focused behavior used by this module.
// FR: createAbsentThroneFragment déclare ou implémente un comportement précis utilisé par ce module.
Material MaterialCatalog::createAbsentThroneFragment(int quantity)
{
    return Material("absent_throne_fragment", "Fragment de trône absent", "Fragment de boss : morceau d'autorité royale sans salle, sans murs et sans royaume.", "Fragment de boss", 325, quantity);
}

// EN: createLostNameFragment declares or implements a focused behavior used by this module.
// FR: createLostNameFragment déclare ou implémente un comportement précis utilisé par ce module.
Material MaterialCatalog::createLostNameFragment(int quantity)
{
    return Material("lost_name_fragment", "Fragment de nom perdu", "Fragment de boss : identité arrachée à une bête que le registre décrit différemment à chaque rencontre.", "Fragment de boss", 340, quantity);
}

// EN: createAldebarothAbyssResidue declares or implements a focused behavior used by this module.
// FR: createAldebarothAbyssResidue déclare ou implémente un comportement précis utilisé par ce module.
Material MaterialCatalog::createAldebarothAbyssResidue(int quantity)
{
    return Material("aldebaroth_abyss_residue", "Résidu d'abîme d'Aldebaroth", "Fragment de boss : négativité condensée par un démon primordial affaibli.", "Fragment de boss", 350, quantity);
}

// EN: createTwinParadoxShard declares or implements a focused behavior used by this module.
// FR: createTwinParadoxShard déclare ou implémente un comportement précis utilisé par ce module.
Material MaterialCatalog::createTwinParadoxShard(int quantity)
{
    return Material("twin_paradox_shard", "Éclat du paradoxe jumeau", "Fragment de boss : création et destruction figées dans le même morceau impossible.", "Fragment de boss", 365, quantity);
}

// EN: createOberionOriginThread declares or implements a focused behavior used by this module.
// FR: createOberionOriginThread déclare ou implémente un comportement précis utilisé par ce module.
Material MaterialCatalog::createOberionOriginThread(int quantity)
{
    return Material("oberion_origin_thread", "Fil d'origine d'Obérion", "Fragment de boss : fil issu d'un écho fragmenté du dieu universel, trop ancien pour être stable.", "Fragment de boss", 390, quantity);
}

// EN: createFacelessBreath declares or implements a focused behavior used by this module.
// FR: createFacelessBreath déclare ou implémente un comportement précis utilisé par ce module.
Material MaterialCatalog::createFacelessBreath(int quantity)
{
    return Material("faceless_breath", "Souffle sans visage", "Fragment de boss : respiration arrachée à une entité qui refuse toute identité stable.", "Fragment de boss", 355, quantity);
}

// EN: createPuppetNail declares or implements a focused behavior used by this module.
// FR: createPuppetNail déclare ou implémente un comportement précis utilisé par ce module.
Material MaterialCatalog::createPuppetNail(int quantity)
{
    return Material("puppet_nail", "Clou de marionnette", "Fragment de boss : clou cousu à une douleur ancienne, utile pour de futurs crafts de contrôle et de contre-coup.", "Fragment de boss", 360, quantity);
}

// EN: createMoiranFateThread declares or implements a focused behavior used by this module.
// FR: createMoiranFateThread déclare ou implémente un comportement précis utilisé par ce module.
Material MaterialCatalog::createMoiranFateThread(int quantity)
{
    return Material("moiran_fate_thread", "Fil de destinée de Moiran", "Fragment de boss : fil de destin coupé trop tôt, encore capable de réagir aux chemins interdits.", "Fragment de boss", 385, quantity);
}

// EN: createLostSoulAntler declares or implements a focused behavior used by this module.
// FR: createLostSoulAntler déclare ou implémente un comportement précis utilisé par ce module.
Material MaterialCatalog::createLostSoulAntler(int quantity)
{
    return Material("lost_soul_antler", "Éclat de bois d'âme perdue", "Fragment de boss : morceau de bois mystique portant des souvenirs qui n'ont pas retrouvé leur corps.", "Fragment de boss", 370, quantity);
}


// EN: createGorvaldRoyalBlood declares or implements a focused behavior used by this module.
// FR: createGorvaldRoyalBlood déclare ou implémente un comportement précis utilisé par ce module.
Material MaterialCatalog::createGorvaldRoyalBlood(int quantity)
{
    return Material("gorvald_royal_blood", "Sang royal orc", "Fragment de boss : preuve brute d'un roi orc qui mesure l'honneur à la capacité de rester debout.", "Fragment de boss", 380, quantity);
}

// EN: createSeranaRoyalFang declares or implements a focused behavior used by this module.
// FR: createSeranaRoyalFang déclare ou implémente un comportement précis utilisé par ce module.
Material MaterialCatalog::createSeranaRoyalFang(int quantity)
{
    return Material("serana_royal_fang", "Croc royal de Serana", "Fragment de boss : croc vampirique noble, encore chaud d'un banquet qui n'aurait jamais dû finir.", "Fragment de boss", 390, quantity);
}

// EN: createDraiiteBlackSilk declares or implements a focused behavior used by this module.
// FR: createDraiiteBlackSilk déclare ou implémente un comportement précis utilisé par ce module.
Material MaterialCatalog::createDraiiteBlackSilk(int quantity)
{
    return Material("draiite_black_silk", "Soie noire de Draiite", "Fragment de boss : fil royal d'une toile qui transforme chaque déplacement en piège.", "Fragment de boss", 385, quantity);
}

// EN: createBrokenMirrorShard declares or implements a focused behavior used by this module.
// FR: createBrokenMirrorShard déclare ou implémente un comportement précis utilisé par ce module.
Material MaterialCatalog::createBrokenMirrorShard(int quantity)
{
    return Material("broken_mirror_shard", "Éclat du miroir fendu", "Fragment de boss : morceau de vérité et de mensonge lié aux Jumelles du Miroir Fendu.", "Fragment de boss", 405, quantity);
}
// EN: createUnstableVersionCore declares or implements a focused behavior used by this module.
// FR: createUnstableVersionCore déclare ou implémente un comportement précis utilisé par ce module.
Material MaterialCatalog::createUnstableVersionCore(int quantity)
{
    return Material("unstable_version_core", "Noyau de version instable", "Fragment de boss : règle condensée par un avatar limité du créateur, dangereusement proche d'un patch vivant.", "Fragment de boss", 420, quantity);
}

// EN: createPrecisionHarvestTools declares or implements a focused behavior used by this module.
// FR: createPrecisionHarvestTools déclare ou implémente un comportement précis utilisé par ce module.
Material MaterialCatalog::createPrecisionHarvestTools(int quantity)
{
    return Material("precision_harvest_tools", "Outils de récupération précise", "Petit set d'outils permettant de mieux récupérer plantes, composants et trophées sans les massacrer. Augmente les chances de qualité supérieure tant qu'il est possédé.", "Outil", 95, quantity);
}

// EN: createPreservationVials declares or implements a focused behavior used by this module.
// FR: createPreservationVials déclare ou implémente un comportement précis utilisé par ce module.
Material MaterialCatalog::createPreservationVials(int quantity)
{
    return Material("preservation_vials", "Fioles de conservation", "Fioles épaisses pour garder les liquides, braises et résidus instables en meilleur état. Réduit les dégradations sur les loots magiques ou organiques.", "Outil", 110, quantity);
}

// EN: createCleanHarvestManual declares or implements a focused behavior used by this module.
// FR: createCleanHarvestManual déclare ou implémente un comportement précis utilisé par ce module.
Material MaterialCatalog::createCleanHarvestManual(int quantity)
{
    return Material("clean_harvest_manual", "Technique passive : récolte propre", "Apprentissage simple : observer avant de découper. Améliore légèrement la récupération des plantes et matériaux communs.", "Livre", 80, quantity);
}

// EN: createMonsterDissectionGuide declares or implements a focused behavior used by this module.
// FR: createMonsterDissectionGuide déclare ou implémente un comportement précis utilisé par ce module.
Material MaterialCatalog::createMonsterDissectionGuide(int quantity)
{
    return Material("monster_dissection_guide", "Technique passive : dissection de monstre", "Guide pratique pour éviter les oreilles trouées, les peaux mal arrachées et les composants brûlés. Améliore les loots de monstres.", "Livre", 120, quantity);
}

Material MaterialCatalog::createTrainingArrows(int quantity)
{
    return Material("training_arrows", "Flèches d'entraînement", "Munitions basiques fournies aux classes à arc. Le craft de munitions spéciales sera appris plus tard.", "Munition", 1, quantity);
}

Material MaterialCatalog::createTrainingBolts(int quantity)
{
    return Material("training_bolts", "Carreaux d'entraînement", "Munitions basiques pour arbalète ou arme de tir lourde. Leur système actif viendra avec les armes à munitions.", "Munition", 2, quantity);
}

Material MaterialCatalog::createTrainingThrowingKnives(int quantity)
{
    return Material("training_throwing_knives", "Couteaux de lancer émoussés", "Projectiles pauvres mais utiles pour une défense à distance courte.", "Munition", 2, quantity);
}

Material MaterialCatalog::createBarbedArrows(int quantity)
{
    return Material("barbed_arrows", "Flèches barbelées", "Munitions spéciales artisanales. Elles infligeront plus tard des effets de saignement ; pour l'instant elles comptent comme munitions avancées.", "Munition spéciale", 5, quantity);
}

Material MaterialCatalog::createPiercingBolts(int quantity)
{
    return Material("piercing_bolts", "Carreaux perforants", "Munitions d'arbalète renforcées, pensées pour armures et monstres épais. Effets avancés prévus avec les armes à munitions.", "Munition spéciale", 6, quantity);
}

Material MaterialCatalog::createBalancedThrowingKnives(int quantity)
{
    return Material("balanced_throwing_knives", "Couteaux de lancer équilibrés", "Projectiles courts, mieux équilibrés que les couteaux émoussés de départ. Recette utile aux classes mobiles.", "Munition spéciale", 6, quantity);
}

Material MaterialCatalog::createAshArrows(int quantity)
{
    return Material("ash_arrows", "Flèches de cendre", "Munitions expérimentales noircies par alchimie. Elles préparent le futur système de brûlure sans l'activer partout trop tôt.", "Munition élémentaire", 8, quantity);
}

Material MaterialCatalog::createFrozenBolts(int quantity)
{
    return Material("frozen_bolts", "Carreaux givrés", "Carreaux traités au froid. Ils serviront aux futures interactions de ralentissement, glace et armures gelées.", "Munition élémentaire", 9, quantity);
}

Material MaterialCatalog::createConductiveKnives(int quantity)
{
    return Material("conductive_knives", "Couteaux conducteurs", "Projectiles métalliques préparés pour les futures réactions électriques, surtout contre les cibles équipées de métal.", "Munition élémentaire", 9, quantity);
}
