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

    bool endsWith(const std::string& value, const std::string& suffix)
    {
        return value.size() >= suffix.size()
            && value.compare(value.size() - suffix.size(), suffix.size(), suffix) == 0;
    }

    Material createGuildRankNoticeMaterial(const std::string& normalized, int quantity)
    {
        if (normalized == "guild_rank_e_notice") return Material("guild_rank_e_notice", "Notice de rang E", "Notice de carte magique : la guilde reconnaît quelques contrats réussis et autorise des missions un peu moins ridicules.", "Renseignement de guilde", 0, quantity);
        if (normalized == "guild_rank_d_notice") return Material("guild_rank_d_notice", "Notice de rang D", "Notice de carte magique : l'aventurier commence à sortir du dossier débutant.", "Renseignement de guilde", 0, quantity);
        if (normalized == "guild_rank_c_notice") return Material("guild_rank_c_notice", "Notice de rang C", "Notice de carte magique : la guilde accepte de confier des demandes plus sérieuses.", "Renseignement de guilde", 0, quantity);
        if (normalized == "guild_rank_b_notice") return Material("guild_rank_b_notice", "Notice de rang B", "Notice de carte magique : le dossier commence à porter un poids réel au comptoir.", "Renseignement de guilde", 0, quantity);
        if (normalized == "guild_rank_a_notice") return Material("guild_rank_a_notice", "Notice de rang A", "Notice de carte magique : seuls les contrats dangereux devraient encore faire sourire.", "Renseignement de guilde", 0, quantity);
        if (normalized == "guild_rank_s_notice") return Material("guild_rank_s_notice", "Notice de rang S", "Notice de carte magique : le registre n'utilise plus ce rang pour faire joli.", "Renseignement de guilde", 0, quantity);
        if (normalized == "guild_rank_ss_notice") return Material("guild_rank_ss_notice", "Notice de rang SS", "Notice de carte magique : la fiche fait hésiter les maîtres de guilde avant d'être signée.", "Renseignement de guilde", 0, quantity);
        if (normalized == "guild_rank_sss_notice") return Material("guild_rank_sss_notice", "Notice de rang SSS", "Notice de carte magique : le comptoir connaît ton nom, ce qui n'est pas toujours rassurant.", "Renseignement de guilde", 0, quantity);
        if (normalized == "guild_rank_world_hero_notice") return Material("guild_rank_world_hero_notice", "Notice de héros mondial", "Notice de carte magique : plusieurs régions peuvent reconnaître ce dossier sans rire.", "Renseignement de guilde", 0, quantity);
        if (normalized == "guild_rank_legend_notice") return Material("guild_rank_legend_notice", "Notice de légende", "Notice de carte magique : le dossier ressemble plus à une histoire qu'à une simple fiche.", "Renseignement de guilde", 0, quantity);
        if (normalized == "guild_rank_god_notice") return Material("guild_rank_god_notice", "Notice de rang divin", "Notice de carte magique : même le registre semble hésiter à afficher cette ligne.", "Renseignement de guilde", 0, quantity);
        return Material("guild_rank_f_notice", "Inscription de rang F", "Notice de carte magique : inscription validée au rang de départ de la guilde.", "Renseignement de guilde", 0, quantity);
    }

    Material createGuildReliabilityPelletMaterial(const std::string& normalized, int quantity)
    {
        if (normalized == "guild_reliability_yellow_pellet") return Material("guild_reliability_yellow_pellet", "Pastille jaune de fiabilité", "Marque de dossier : quelques retards ou échecs sont notés, mais la guilde laisse encore une marge.", "Renseignement de guilde", 0, quantity);
        if (normalized == "guild_reliability_orange_pellet") return Material("guild_reliability_orange_pellet", "Pastille orange de fiabilité", "Marque de dossier : la guilde surveille les contrats pris trop haut ou rendus trop tard.", "Renseignement de guilde", 0, quantity);
        if (normalized == "guild_reliability_red_pellet") return Material("guild_reliability_red_pellet", "Pastille rouge de fiabilité", "Marque de dossier : la guilde limite fortement les contrats risqués jusqu'à amélioration du dossier.", "Renseignement de guilde", 0, quantity);
        return Material("guild_reliability_green_pellet", "Pastille verte de fiabilité", "Marque de dossier : fiabilité correcte, aucune sanction grave connue.", "Renseignement de guilde", 0, quantity);
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
    if (normalized == "predator_fang") return withQuality(createPredatorFang(quantity), quality);
    if (normalized == "boar_tusk") return withQuality(createBoarTusk(quantity), quality);
    if (normalized == "rodent_tooth") return withQuality(createRodentTooth(quantity), quality);
    if (normalized == "rusted_metal_fragment") return withQuality(createRustedMetalFragment(quantity), quality);
    if (normalized == "runic_iron_shard") return withQuality(createRunicIronShard(quantity), quality);
    if (normalized == "polished_scale_plate") return withQuality(createPolishedScalePlate(quantity), quality);
    if (normalized == "amber_tempering_oil") return withQuality(createAmberTemperingOil(quantity), quality);
    if (normalized == "worn_leather_piece") return withQuality(createWornLeatherPiece(quantity), quality);
    if (normalized == "mountain_blue_flower") return withQuality(createMountainBlueFlower(quantity), quality);
    if (normalized == "bitter_healing_leaf") return withQuality(createBitterHealingLeaf(quantity), quality);
    if (normalized == "common_goblin_notes") return withQuality(createCommonGoblinNotes(quantity), quality);
    if (normalized == "common_wolf_notes") return withQuality(createCommonWolfNotes(quantity), quality);
    if (normalized == "basic_plant_manual") return withQuality(createBasicPlantManual(quantity), quality);
    if (normalized == "basic_magic_manual") return withQuality(createBasicMagicManual(quantity), quality);
    if (normalized == "arcane_binding_grimoire") return withQuality(createArcaneBindingGrimoire(quantity), quality);
    if (normalized == "elemental_ward_grimoire") return withQuality(createElementalWardGrimoire(quantity), quality);
    if (normalized == "resistance_rift_grimoire") return withQuality(createResistanceRiftGrimoire(quantity), quality);
    if (normalized == "frost_needle_grimoire") return withQuality(createFrostNeedleGrimoire(quantity), quality);
    if (normalized == "mana_suture_grimoire") return withQuality(createManaSutureGrimoire(quantity), quality);
    if (normalized == "occult_bramble_grimoire") return withQuality(createOccultBrambleGrimoire(quantity), quality);
    if (normalized == "cracked_bone") return withQuality(createCrackedBone(quantity), quality);
    if (normalized == "arcane_dust") return withQuality(createArcaneDust(quantity), quality);
    if (normalized == "minor_fire_rune_note") return withQuality(createMinorFireRuneNote(quantity), quality);
    if (normalized == "minor_cold_rune_note") return withQuality(createMinorColdRuneNote(quantity), quality);
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
    if (normalized == "client_recommendation") return withQuality(createClientRecommendation(quantity), quality);
    if (normalized == "guild_favor_token") return withQuality(createGuildFavorToken(quantity), quality);
    if (normalized == "local_service_letter") return withQuality(createLocalServiceLetter(quantity), quality);
    if (normalized == "guild_card") return withQuality(createGuildCard(quantity), quality);
    if (startsWith(normalized, "guild_rank_") && endsWith(normalized, "_notice")) return withQuality(createGuildRankNoticeMaterial(normalized, quantity), quality);
    if (startsWith(normalized, "guild_reliability_") && endsWith(normalized, "_pellet")) return withQuality(createGuildReliabilityPelletMaterial(normalized, quantity), quality);
    if (normalized == "travel_pass_note") return withQuality(createTravelPassNote(quantity), quality);
    if (normalized == "warm_meal_voucher") return withQuality(createWarmMealVoucher(quantity), quality);
    if (normalized == "city_service_stamp") return withQuality(createCityServiceStamp(quantity), quality);
    if (normalized == "municipal_proof_letter") return withQuality(createMunicipalProofLetter(quantity), quality);
    if (normalized == "lodging_bed_token") return withQuality(createLodgingBedToken(quantity), quality);
    if (normalized == "stable_stall_ticket") return withQuality(createStableStallTicket(quantity), quality);
    if (normalized == "caravan_seat_ticket") return withQuality(createCaravanSeatTicket(quantity), quality);
    if (normalized == "guarded_transport_pass") return withQuality(createGuardedTransportPass(quantity), quality);
    if (normalized == "route_toll_receipt") return withQuality(createRouteTollReceipt(quantity), quality);
    if (normalized == "local_reputation_note") return withQuality(createLocalReputationNote(quantity), quality);
    if (normalized == "local_service_warning") return withQuality(createLocalServiceWarning(quantity), quality);
    if (normalized == "fire_lantern") return withQuality(createFireLantern(quantity), quality);
    if (normalized == "night_survival_kit") return withQuality(createNightSurvivalKit(quantity), quality);
    if (normalized == "survival_ration") return withQuality(createSurvivalRation(quantity), quality);
    if (normalized == "thermal_survival_blanket") return withQuality(createThermalSurvivalBlanket(quantity), quality);
    if (normalized == "cooling_survival_wrap") return withQuality(createCoolingSurvivalWrap(quantity), quality);
    if (normalized == "temperature_survival_kit") return withQuality(createTemperatureSurvivalKit(quantity), quality);
    if (normalized == "travel_distance_mark") return withQuality(createTravelDistanceMark(quantity), quality);
    if (normalized == "prepared_saddlebags") return withQuality(createPreparedSaddlebags(quantity), quality);
    if (normalized == "temporary_stable_storage") return withQuality(createTemporaryStableStorage(quantity), quality);
    if (normalized == "route_scout_note") return withQuality(createRouteScoutNote(quantity), quality);
    if (normalized == "rental_mount_voucher") return withQuality(createRentalMountVoucher(quantity), quality);
    if (normalized == "stable_box_reservation") return withQuality(createStableBoxReservation(quantity), quality);
    if (normalized == "loaded_pack_saddle") return withQuality(createLoadedPackSaddle(quantity), quality);
    if (normalized == "relay_route_badge") return withQuality(createRelayRouteBadge(quantity), quality);
    if (normalized == "owned_mount_registration") return withQuality(createOwnedMountRegistration(quantity), quality);
    if (normalized == "mount_fatigue_marker") return withQuality(createMountFatigueMarker(quantity), quality);
    if (normalized == "mount_rest_care") return withQuality(createMountRestCare(quantity), quality);
    if (normalized == "stable_saddle_upgrade") return withQuality(createStableSaddleUpgrade(quantity), quality);
    if (normalized == "mount_name_tag") return withQuality(createMountNameTag(quantity), quality);
    if (normalized == "mount_temperament_calm") return withQuality(createMountTemperamentCalm(quantity), quality);
    if (normalized == "mount_bond_marker") return withQuality(createMountBondMarker(quantity), quality);
    if (normalized == "mount_minor_injury_marker") return withQuality(createMountMinorInjuryMarker(quantity), quality);
    if (normalized == "mount_comfort_bridle") return withQuality(createMountComfortBridle(quantity), quality);
    if (normalized == "mount_grooming_kit") return withQuality(createMountGroomingKit(quantity), quality);
    if (normalized == "mount_weather_blanket") return withQuality(createMountWeatherBlanket(quantity), quality);
    if (normalized == "mount_pack_harness") return withQuality(createMountPackHarness(quantity), quality);
    if (normalized == "mount_route_memory_marker") return withQuality(createMountRouteMemoryMarker(quantity), quality);
    if (normalized == "mount_surefoot_training_marker") return withQuality(createMountSurefootTrainingMarker(quantity), quality);
    if (normalized == "mount_road_shoes") return withQuality(createMountRoadShoes(quantity), quality);
    if (normalized == "runic_extraction_note") return withQuality(createRunicExtractionNote(quantity), quality);
    if (normalized == "runic_safety_seal") return withQuality(createRunicSafetySeal(quantity), quality);
    if (normalized == "runic_transfer_note") return withQuality(createRunicTransferNote(quantity), quality);
    if (normalized == "runic_overload_limit_note") return withQuality(createRunicOverloadLimitNote(quantity), quality);
    if (normalized == "runic_stabilizer") return withQuality(createRunicStabilizer(quantity), quality);
    if (normalized == "rare_fire_rune_core") return withQuality(createRareFireRuneCore(quantity), quality);
    if (normalized == "rare_cold_rune_core") return withQuality(createRareColdRuneCore(quantity), quality);
    if (normalized == "necromancy_warning") return withQuality(createNecromancyWarning(quantity), quality);
    if (normalized == "magic_learning_notes") return withQuality(createMagicLearningNotes(quantity), quality);
    if (normalized == "elemental_weakness_notes") return withQuality(createElementalWeaknessNotes(quantity), quality);
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
    if (normalized == "venom_arrows") return withQuality(createVenomArrows(quantity), quality);
    if (normalized == "shock_bolts") return withQuality(createShockBolts(quantity), quality);
    if (normalized == "smoke_knives") return withQuality(createSmokeKnives(quantity), quality);
    if (normalized == "mycelium_lantern") return withQuality(createMyceliumLantern(quantity), quality);
    if (normalized == "echoing_resin") return withQuality(createEchoingResin(quantity), quality);
    if (normalized == "sun_dried_clay") return withQuality(createSunDriedClay(quantity), quality);
    if (normalized == "moonlit_salt") return withQuality(createMoonlitSalt(quantity), quality);
    if (normalized == "old_coin_bundle") return withQuality(createOldCoinBundle(quantity), quality);
    if (normalized == "glass_map_fragment") return withQuality(createGlassMapFragment(quantity), quality);
    if (normalized == "living_vine_fiber") return withQuality(createLivingVineFiber(quantity), quality);
    if (normalized == "cold_iron_nail") return withQuality(createColdIronNail(quantity), quality);
    if (normalized == "witch_bottle") return withQuality(createWitchBottle(quantity), quality);
    if (normalized == "forgotten_camp_tag") return withQuality(createForgottenCampTag(quantity), quality);
    if (normalized == "tiny_gear_spring") return withQuality(createTinyGearSpring(quantity), quality);
    if (normalized == "inked_contract_scrap") return withQuality(createInkedContractScrap(quantity), quality);
    if (normalized == "firefly_iron_shell") return withQuality(createFireflyIronShell(quantity), quality);
    if (normalized == "whispering_archive_page") return withQuality(createWhisperingArchivePage(quantity), quality);
    if (normalized == "grey_drake_scale") return withQuality(createGreyDrakeScale(quantity), quality);
    if (normalized == "carnival_ticket_shred") return withQuality(createCarnivalTicketShred(quantity), quality);
    if (normalized == "mirror_glass_bead") return withQuality(createMirrorGlassBead(quantity), quality);
    if (normalized == "tideworn_ink") return withQuality(createTidewornInk(quantity), quality);
    if (normalized == "cliff_basil_leaf") return withQuality(createCliffBasilLeaf(quantity), quality);
    if (normalized == "rusted_gear_core") return withQuality(createRustedGearCore(quantity), quality);
    if (normalized == "salted_rope_knot") return withQuality(createSaltedRopeKnot(quantity), quality);
    if (normalized == "luminous_moth_wing") return withQuality(createLuminousMothWing(quantity), quality);
    if (normalized == "holy_water_vial") return withQuality(createHolyWaterVial(quantity), quality);
    if (normalized == "sanctuary_candle") return withQuality(createSanctuaryCandle(quantity), quality);
    if (normalized == "exorcism_incense") return withQuality(createExorcismIncense(quantity), quality);
    if (normalized == "blessing_note") return withQuality(createBlessingNote(quantity), quality);
    if (normalized == "exorcist_note") return withQuality(createExorcistNote(quantity), quality);
    if (normalized == "cracked_bell_clapper") return withQuality(createCrackedBellClapper(quantity), quality);
    if (normalized == "sanctuary_wax_seal") return withQuality(createSanctuaryWaxSeal(quantity), quality);
    if (normalized == "church_night_testimony") return withQuality(createChurchNightTestimony(quantity), quality);
    if (normalized == "church_sleep_diagnosis") return withQuality(createChurchSleepDiagnosis(quantity), quality);
    if (normalized == "named_oath_testimony") return withQuality(createNamedOathTestimony(quantity), quality);
    if (normalized == "shared_dream_pattern") return withQuality(createSharedDreamPattern(quantity), quality);
    if (normalized == "copied_counter_legend") return withQuality(createCopiedCounterLegend(quantity), quality);
    if (normalized == "identified_source_object") return withQuality(createIdentifiedSourceObject(quantity), quality);
    if (normalized == "sealable_source_sketch") return withQuality(createSealableSourceSketch(quantity), quality);
    if (normalized == "pact_break_witness") return withQuality(createPactBreakWitness(quantity), quality);
    if (normalized == "source_defeat_notice") return withQuality(createSourceDefeatNotice(quantity), quality);
    if (normalized == "purified_source_ashes") return withQuality(createPurifiedSourceAshes(quantity), quality);
    if (normalized == "broken_oath_record") return withQuality(createBrokenOathRecord(quantity), quality);
    if (normalized == "closed_counter_legend") return withQuality(createClosedCounterLegend(quantity), quality);
    if (normalized == "sealed_source_mark") return withQuality(createSealedSourceMark(quantity), quality);
    if (normalized == "released_pact_record") return withQuality(createReleasedPactRecord(quantity), quality);
    if (normalized == "confirmed_source_silence") return withQuality(createConfirmedSourceSilence(quantity), quality);
    if (normalized == "blue_mist_reed") return withQuality(createBlueMistReed(quantity), quality);
    if (normalized == "mistglass_pearl") return withQuality(createMistglassPearl(quantity), quality);
    if (normalized == "white_bone_chalk") return withQuality(createWhiteBoneChalk(quantity), quality);
    if (normalized == "buried_giant_chip") return withQuality(createBuriedGiantChip(quantity), quality);
    if (normalized == "smuggler_token") return withQuality(createSmugglerToken(quantity), quality);
    if (normalized == "sealed_debt_slip") return withQuality(createSealedDebtSlip(quantity), quality);
    if (normalized == "black_market_barter_seal") return withQuality(createBlackMarketBarterSeal(quantity), quality);
    if (normalized == "city_repair_days_marker") return withQuality(createCityRepairDaysMarker(quantity), quality);
    if (normalized == "city_damage_notice") return withQuality(createCityDamageNotice(quantity), quality);
    if (normalized == "city_defense_medal") return withQuality(createCityDefenseMedal(quantity), quality);
    if (normalized == "city_defense_gratitude_days_marker") return withQuality(createCityDefenseGratitudeMarker(quantity), quality);
    if (normalized == "city_event_recent_days_marker") return withQuality(createCityEventRecentMarker(quantity), quality);
    if (normalized == "city_repair_receipt") return withQuality(createCityRepairReceipt(quantity), quality);
    if (normalized == "weeping_stone_tear") return withQuality(createWeepingStoneTear(quantity), quality);
    if (normalized == "petrified_rose_petals") return withQuality(createPetrifiedRosePetals(quantity), quality);

    return Material(
        id,
        "Renseignement inconnu",
        "Une entrée dont les artisans parlent encore à demi-mot.",
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
    return Material("wolf_fang", "Croc de loup", "Croc prélevé sur un loup, une meute ou une créature explicitement lupine.", "Matériau de monstre", 6, quantity);
}

Material MaterialCatalog::createPredatorFang(int quantity)
{
    return Material("predator_fang", "Croc de prédateur", "Croc récupéré sur une bête carnassière non lupine : chien sauvage, renard, chacal ou ours jeune.", "Matériau de monstre", 7, quantity);
}

Material MaterialCatalog::createBoarTusk(int quantity)
{
    return Material("boar_tusk", "Défense de sanglier", "Défense dure issue d'une bête à charge. Utile pour comparer les morsures, charges et traces de défense.", "Matériau de monstre", 7, quantity);
}

Material MaterialCatalog::createRodentTooth(int quantity)
{
    return Material("rodent_tooth", "Dent de rongeur", "Petite dent de rat ou de rongeur géant. Moins noble qu'un croc, mais utile pour les artisans précis.", "Matériau de monstre", 3, quantity);
}

// EN: createRustedMetalFragment declares or implements a focused behavior used by this module.
// FR: createRustedMetalFragment déclare ou implémente un comportement précis utilisé par ce module.
Material MaterialCatalog::createRustedMetalFragment(int quantity)
{
    return Material("rusted_metal_fragment", "Fragment de métal rouillé", "Reste d'équipement brisé, encore utile pour des réparations basiques.", "Matériau", 3, quantity);
}

Material MaterialCatalog::createRunicIronShard(int quantity)
{
    return Material("runic_iron_shard", "Éclat de fer runique", "Métal gravé de petites marques de stabilisation. Sert aux armes de palier intermédiaire et aux runes simples.", "Métal runique", 24, quantity);
}

Material MaterialCatalog::createPolishedScalePlate(int quantity)
{
    return Material("polished_scale_plate", "Plaque d'écailles polies", "Plaque préparée à partir d'écailles de monstre. Utile pour des armures résistantes, mais plus chère à entretenir.", "Matériau de monstre", 30, quantity);
}

Material MaterialCatalog::createAmberTemperingOil(int quantity)
{
    return Material("amber_tempering_oil", "Huile de trempe ambrée", "Huile d'atelier qui stabilise les effets latents d'une arme ou d'un catalyseur sans les rendre encore héroïques.", "Matériau d'atelier", 34, quantity);
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
    return Material("basic_plant_manual", "Petit guide des plantes communes", "Livre de terrain utilisé par les herboristes pour tenir un journal des plantes.", "Livre", 10, quantity);
}

// EN: createBasicMagicManual declares or implements a focused behavior used by this module.
// FR: createBasicMagicManual déclare ou implémente un comportement précis utilisé par ce module.
Material MaterialCatalog::createBasicMagicManual(int quantity)
{
    return Material("basic_magic_manual", "Manuel de magie basique", "Livre simple couvrant les premières notions de magie.", "Livre", 10, quantity);
}

Material MaterialCatalog::createArcaneBindingGrimoire(int quantity)
{
    return Material("arcane_binding_grimoire", "Grimoire d'entrave arcanique", "Livre de magie appliquée : utile aux mages capables de maintenir un sort sans se faire dévorer par leur propre souffle.", "Livre magique", 20, quantity);
}

Material MaterialCatalog::createElementalWardGrimoire(int quantity)
{
    return Material("elemental_ward_grimoire", "Grimoire du voile élémentaire", "Livre défensif décrivant comment replier l'énergie autour du corps au lieu de la projeter.", "Livre magique", 25, quantity);
}

Material MaterialCatalog::createResistanceRiftGrimoire(int quantity)
{
    return Material("resistance_rift_grimoire", "Grimoire de faille de résistance", "Ouvrage instable : ses pages parlent de brèches dans les défenses, pas de miracles sans coût.", "Livre magique rare", 35, quantity);
}

Material MaterialCatalog::createFrostNeedleGrimoire(int quantity)
{
    return Material("frost_needle_grimoire", "Grimoire d'aiguille de givre", "Sort apprenable sans parchemin courant : précis, froid et trop fin pour être scellé proprement sur du papier basique.", "Livre magique", 28, quantity);
}

Material MaterialCatalog::createManaSutureGrimoire(int quantity)
{
    return Material("mana_suture_grimoire", "Grimoire de suture de mana", "Sort apprenable sans parchemin courant : il referme lentement une blessure, mais demande un vrai canaliseur et un catalyseur fiable.", "Livre magique", 34, quantity);
}

Material MaterialCatalog::createOccultBrambleGrimoire(int quantity)
{
    return Material("occult_bramble_grimoire", "Grimoire des ronces occultes", "Sort apprenable sans parchemin courant : il serre la cible avec des ronces de mana, puis laisse un venin lent dans les ouvertures.", "Livre magique", 32, quantity);
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
    return Material("arcane_dust", "Poussière arcanique", "Résidu magique instable, utile aux enchantements et composants de sorts.", "Matériau magique", 12, quantity);
}

// EN: createMinorFireRuneNote declares or implements a focused behavior used by this module.
// FR: createMinorFireRuneNote déclare ou implémente un comportement précis utilisé par ce module.
Material MaterialCatalog::createMinorFireRuneNote(int quantity)
{
    return Material("minor_fire_rune_note", "Formule de rune anti-feu", "Note d'enchanteur décrivant une protection mineure contre la chaleur, les flammes et les brûlures.", "Formule runique", 22, quantity);
}

// EN: createMinorColdRuneNote declares or implements a focused behavior used by this module.
// FR: createMinorColdRuneNote déclare ou implémente un comportement précis utilisé par ce module.
Material MaterialCatalog::createMinorColdRuneNote(int quantity)
{
    return Material("minor_cold_rune_note", "Formule de rune anti-froid", "Note d'enchanteur décrivant une protection mineure contre le froid, le givre et les zones glacées.", "Formule runique", 22, quantity);
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
    return Material("battle_torn_badge", "Insigne abîmé d'aventurier", "Preuve laissée par un combattant humain ou semi-humain. Les guildes et certains clients y accordent de la valeur.", "Trophée", 15, quantity);
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
    return Material("summoning_notes", "Manuel d'invocations instables", "Livre décrivant les places actives, coûts de mana, sacrifices et évolutions d'invocations.", "Livre", 12, quantity);
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

// EN: createClientRecommendation declares or implements a focused behavior used by this module.
// FR: createClientRecommendation déclare ou implémente un comportement précis utilisé par ce module.
Material MaterialCatalog::createClientRecommendation(int quantity)
{
    return Material("client_recommendation", "Recommandation de client", "Billet signé, adresse griffonnée ou nom transmis par la guilde. Ce n'est pas une bourse d'or, mais une porte qui s'ouvre quelque part.", "Renseignement", 12, quantity);
}

// EN: createGuildFavorToken declares or implements a focused behavior used by this module.
// FR: createGuildFavorToken déclare ou implémente un comportement précis utilisé par ce module.
Material MaterialCatalog::createGuildFavorToken(int quantity)
{
    return Material("guild_favor_token", "Jeton de faveur de guilde", "Petit jeton marqué au sceau de la guilde. Il prouve qu'un service a été rendu proprement, même quand la prime en pièces reste maigre.", "Renseignement", 22, quantity);
}

// EN: createLocalServiceLetter declares or implements a focused behavior used by this module.
// FR: createLocalServiceLetter déclare ou implémente un comportement précis utilisé par ce module.
Material MaterialCatalog::createLocalServiceLetter(int quantity)
{
    return Material("local_service_letter", "Lettre de service local", "Lettre simple remise par un habitant, un marchand ou un artisan. Elle vaut parfois plus qu'une petite prime quand il faut se faire connaître.", "Renseignement", 8, quantity);
}

Material MaterialCatalog::createGuildCard(int quantity)
{
    return Material("guild_card", "Carte magique de guilde", "Carte personnelle liée au titre Aventurier. Elle sert de preuve d'inscription et suivra le rang, les pastilles, les sanctions et les recommandations.", "Titre / guilde", 0, quantity);
}

Material MaterialCatalog::createGuildRankFNotice(int quantity)
{
    return Material("guild_rank_f_notice", "Inscription de rang F", "Notice officielle : rang de départ attribué aux nouveaux aventuriers. Elle ne rend pas plus fort, mais elle évite de prendre une mission débilement trop dangereuse.", "Titre / guilde", 0, quantity);
}

Material MaterialCatalog::createGuildReliabilityGreenPellet(int quantity)
{
    return Material("guild_reliability_green_pellet", "Pastille verte de fiabilité", "Marque administrative positive : l'aventurier est inscrit proprement et n'a pas encore de sanction de guilde connue.", "Titre / guilde", 0, quantity);
}

Material MaterialCatalog::createTravelPassNote(int quantity)
{
    return Material("travel_pass_note", "Note de pass de voyage", "Renseignement pratique : certains trajets officiels, caravanes ou contrôles de pont demandent un pass reconnu plutôt qu'une simple bourse de pièces.", "Renseignement", 10, quantity);
}

Material MaterialCatalog::createWarmMealVoucher(int quantity)
{
    return Material("warm_meal_voucher", "Bon de repas chaud", "Bon remis par une auberge ou un relais. Il sert surtout de petite faveur locale quand une prime en pièces serait trop ridicule.", "Renseignement", 6, quantity);
}

Material MaterialCatalog::createCityServiceStamp(int quantity)
{
    return Material("city_service_stamp", "Tampon de service municipal", "Preuve administrative simple : passage au guichet, formulaire relu ou dossier accepté sans scandale. Utile pour les petits services de ville.", "Service de ville", 6, quantity);
}

Material MaterialCatalog::createMunicipalProofLetter(int quantity)
{
    return Material("municipal_proof_letter", "Attestation municipale", "Lettre officielle indiquant qu'un service local a été validé proprement. Certains PNJ la respectent plus qu'une poignée de cuivre.", "Service de ville", 14, quantity);
}

Material MaterialCatalog::createLodgingBedToken(int quantity)
{
    return Material("lodging_bed_token", "Bon de lit d'auberge", "Jeton d'auberge pour une nuit simple, sans promesse contre les ronflements, les punaises ou les bardes trop motivés.", "Auberge", 8, quantity);
}

Material MaterialCatalog::createStableStallTicket(int quantity)
{
    return Material("stable_stall_ticket", "Ticket d'écurie", "Ticket pour réserver une place d'écurie, garder une monture au sec ou justifier un passage au relais.", "Auberge", 10, quantity);
}

Material MaterialCatalog::createCaravanSeatTicket(int quantity)
{
    return Material("caravan_seat_ticket", "Place de caravane", "Ticket de transport simple : une place dans une caravane officielle, assez lente pour être crédible et assez chère pour payer les roues.", "Transport", 18, quantity);
}

Material MaterialCatalog::createGuardedTransportPass(int quantity)
{
    return Material("guarded_transport_pass", "Pass de transport gardé", "Pass plus sérieux pour un convoi escorté. Ce n'est pas une garantie de survie, mais au moins quelqu'un est payé pour crier avant toi.", "Transport", 32, quantity);
}

Material MaterialCatalog::createRouteTollReceipt(int quantity)
{
    return Material("route_toll_receipt", "Reçu de péage de route", "Reçu de passage officiel : pont, porte, route surveillée ou relais qui aime beaucoup les tampons.", "Transport", 9, quantity);
}

Material MaterialCatalog::createLocalReputationNote(int quantity)
{
    return Material("local_reputation_note", "Note de réputation locale", "Petit mot signé par un habitant ou un service local. Pris seul, ça ne rend pas célèbre ; en pile, ça commence à parler.", "Réputation", 5, quantity);
}

Material MaterialCatalog::createLocalServiceWarning(int quantity)
{
    return Material("local_service_warning", "Note d'incident local", "Trace discrète d'un service repris, mal rempli ou rendu en retard. Ce n'est pas une sanction de guilde, mais la ville s'en souvient un peu.", "Réputation", 0, quantity);
}

Material MaterialCatalog::createFireLantern(int quantity)
{
    return Material("fire_lantern", "Lanterne à feu", "Outil de voyage nocturne : éclaire assez pour réduire les mauvaises rencontres de nuit, mais ne transforme pas une exploration dangereuse en balade gratuite.", "Matériel d'exploration", 18, quantity);
}

Material MaterialCatalog::createNightSurvivalKit(int quantity)
{
    return Material("night_survival_kit", "Kit de survie nocturne", "Petit paquet de mèches, craie de chemin, couverture et sifflet. Utile pour limiter les risques quand une sortie commence ou finit de nuit.", "Matériel d'exploration", 34, quantity);
}

Material MaterialCatalog::createSurvivalRation(int quantity)
{
    return Material("survival_ration", "Ration de survie", "Ration simple et compacte pour accepter de rester dehors plus d’une journée complète sans rentrer en ville. Elle évite la faim narrative, pas les baffes.", "Matériel d'exploration", 9, quantity);
}

Material MaterialCatalog::createThermalSurvivalBlanket(int quantity)
{
    return Material("thermal_survival_blanket", "Couverture de survie thermique", "Couverture de secours contre les biomes glacés ou les nuits vraiment mordantes. Elle aide une sortie, mais ne remplace pas une tenue équipée.", "Matériel d'exploration", 22, quantity);
}

Material MaterialCatalog::createCoolingSurvivalWrap(int quantity)
{
    return Material("cooling_survival_wrap", "Voile anti-chaleur", "Voile traité contre la chaleur sèche, les braises et les routes trop proches du feu. Protection de secours consommable.", "Matériel d'exploration", 24, quantity);
}

Material MaterialCatalog::createTemperatureSurvivalKit(int quantity)
{
    return Material("temperature_survival_kit", "Kit de survie thermique", "Petit kit général : sels, couverture, voile traité et conseils de survie. Utile si la température d'un biome dépasse ton équipement.", "Matériel d'exploration", 38, quantity);
}

Material MaterialCatalog::createTravelDistanceMark(int quantity)
{
    return Material("travel_distance_mark", "Marque de distance de trajet", "Note de route indiquant qu'un déplacement a été préparé correctement. Elle sert surtout aux quêtes d'exploration éloignées et aux relais.", "Transport", 7, quantity);
}

Material MaterialCatalog::createPreparedSaddlebags(int quantity)
{
    return Material("prepared_saddlebags", "Sacoches préparées", "Préparation d'écurie : sangles vérifiées, charge répartie et petits rangements prêts pour éviter un détour inutile sur les trajets moyens ou longs.", "Écurie", 12, quantity);
}

Material MaterialCatalog::createTemporaryStableStorage(int quantity)
{
    return Material("temporary_stable_storage", "Dépôt temporaire d'écurie", "Reçu indiquant qu'une cargaison légère ou un sac encombrant a été gardé au relais. Utile pour certaines demandes de route ou de ville.", "Écurie", 9, quantity);
}

Material MaterialCatalog::createRouteScoutNote(int quantity)
{
    return Material("route_scout_note", "Note d'éclaireur de route", "Itinéraire relu avant départ : raccourci, point d'eau, pont à éviter ou relais plus sûr. Très utile avant une exploration éloignée.", "Transport", 16, quantity);
}

Material MaterialCatalog::createRentalMountVoucher(int quantity)
{
    return Material("rental_mount_voucher", "Bon de monture de location", "Bon d'écurie indiquant qu'une monture fiable est prête pour un trajet. Très utile quand le biome est loin ou vaste.", "Écurie", 26, quantity);
}

Material MaterialCatalog::createStableBoxReservation(int quantity)
{
    return Material("stable_box_reservation", "Réservation de box sécurisé", "Réservation de box : la monture, les sacs ou une petite cargaison restent gardés le temps d'un service ou d'un trajet préparé.", "Écurie", 22, quantity);
}

Material MaterialCatalog::createLoadedPackSaddle(int quantity)
{
    return Material("loaded_pack_saddle", "Selle de bât chargée", "Selle et charge déjà équilibrées : départ plus rapide, surtout quand la mission demande plusieurs détours ou une exploration longue.", "Écurie", 24, quantity);
}

Material MaterialCatalog::createRelayRouteBadge(int quantity)
{
    return Material("relay_route_badge", "Badge de route du relais", "Badge signé par le relais : utile pour éviter certains contrôles, prouver un trajet préparé et négocier des passages de route.", "Transport", 18, quantity);
}

Material MaterialCatalog::createOwnedMountRegistration(int quantity)
{
    return Material("owned_mount_registration", "Acte de monture personnelle", "Document d'écurie indiquant qu'une monture appartient ou répond régulièrement au personnage. Bonus durable de route, mais la monture peut fatiguer.", "Écurie", 95, quantity);
}

Material MaterialCatalog::createMountFatigueMarker(int quantity)
{
    return Material("mount_fatigue_marker", "Fatigue de monture", "Trace d'usure : la monture a beaucoup donné. Trop de fatigue bloque son bonus jusqu'à un vrai repos/soin d'écurie.", "Écurie", 0, quantity);
}

Material MaterialCatalog::createMountRestCare(int quantity)
{
    return Material("mount_rest_care", "Soin et repos de monture", "Service d'écurie : eau, nourriture, pansage, fers vérifiés et une vraie pause. Sert à retirer la fatigue de monture.", "Écurie", 28, quantity);
}

Material MaterialCatalog::createStableSaddleUpgrade(int quantity)
{
    return Material("stable_saddle_upgrade", "Selle renforcée de route", "Amélioration durable de préparation : meilleure répartition des charges, moins de perte de temps sur les trajets longs.", "Écurie", 46, quantity);
}

Material MaterialCatalog::createMountNameTag(int quantity)
{
    return Material("mount_name_tag", "Nom de monture noté", "Petite plaque/reconnaissance d'écurie : la monture n'est plus juste 'la bête du box', Tavia sait comment l'appeler.", "Écurie", 6, quantity);
}

Material MaterialCatalog::createMountTemperamentCalm(int quantity)
{
    return Material("mount_temperament_calm", "Tempérament de monture calme", "Observation d'écurie : monture plutôt calme, fiable sur route et moins prompte à paniquer pendant les préparatifs.", "Écurie", 0, quantity);
}

Material MaterialCatalog::createMountBondMarker(int quantity)
{
    return Material("mount_bond_marker", "Lien de monture", "Progression douce avec la monture personnelle. Plus le lien est haut, plus les longs trajets deviennent lisibles sans rendre l'animal infatigable.", "Écurie", 0, quantity);
}

Material MaterialCatalog::createMountMinorInjuryMarker(int quantity)
{
    return Material("mount_minor_injury_marker", "Blessure légère de monture", "Trace de fatigue mal gérée : l'animal peut marcher, mais l'écurie refuse de le pousser sur une vraie route tant qu'il n'est pas soigné.", "Écurie", 0, quantity);
}

Material MaterialCatalog::createMountComfortBridle(int quantity)
{
    return Material("mount_comfort_bridle", "Bridon confortable", "Équipement durable de monture : moins de frottements, meilleure réponse aux longues routes et préparation plus propre.", "Écurie", 38, quantity);
}

Material MaterialCatalog::createMountGroomingKit(int quantity)
{
    return Material("mount_grooming_kit", "Kit de pansage de monture", "Brosses, onguent simple et petit nécessaire de soin. Utile pour entretenir le lien sans remplacer un vrai repos d'écurie.", "Écurie", 18, quantity);
}

Material MaterialCatalog::createMountWeatherBlanket(int quantity)
{
    return Material("mount_weather_blanket", "Couverture météo de monture", "Couverture solide pour pluie, froid léger et nuits dehors. Elle aide surtout une monture personnelle déjà entretenue, sans remplacer le repos.", "Écurie", 32, quantity);
}

Material MaterialCatalog::createMountPackHarness(int quantity)
{
    return Material("mount_pack_harness", "Harnais de bât ajusté", "Équipement durable de monture : répartit mieux les petites charges et évite que chaque sacoche devienne une punition de route.", "Écurie", 40, quantity);
}

Material MaterialCatalog::createMountRouteMemoryMarker(int quantity)
{
    return Material("mount_route_memory_marker", "Mémoire de route de monture", "Habitude de terrain : l'animal connaît mieux les relais et les départs répétés. Utile pour les trajets préparés sans devenir une téléportation.", "Écurie", 0, quantity);
}

Material MaterialCatalog::createMountSurefootTrainingMarker(int quantity)
{
    return Material("mount_surefoot_training_marker", "Assurance de monture", "Travail d'appuis et de confiance : la monture panique moins devant les pierres, ponts et terrains mauvais. Progression légère, pas une invincibilité.", "Écurie", 0, quantity);
}

Material MaterialCatalog::createMountRoadShoes(int quantity)
{
    return Material("mount_road_shoes", "Ferrage de route simple", "Équipement durable de monture : sabots vérifiés et fers adaptés aux routes longues. Réduit les petites blessures liées aux départs répétés.", "Écurie", 44, quantity);
}

Material MaterialCatalog::createRunicExtractionNote(int quantity)
{
    return Material("runic_extraction_note", "Note d'extraction runique", "Compte-rendu d'enchanteur : une rune a été retirée proprement ou presque. Sert de suivi avant de retenter un empilement risqué.", "Enchantement", 12, quantity);
}

Material MaterialCatalog::createRunicSafetySeal(int quantity)
{
    return Material("runic_safety_seal", "Sceau anti-casse runique", "Protection d'atelier à usage unique : si une tentative d'enchantement casse, le sceau se sacrifie pour sauver l'objet au lieu de le rendre gratuitement plus fort.", "Enchantement", 52, quantity);
}

Material MaterialCatalog::createRunicTransferNote(int quantity)
{
    return Material("runic_transfer_note", "Note de transfert runique", "Preuve qu'une rune a été déplacée d'un équipement vers un autre. L'opération reste risquée et ne garantit pas de récupérer exactement la même stabilité.", "Enchantement", 28, quantity);
}

Material MaterialCatalog::createRunicOverloadLimitNote(int quantity)
{
    return Material("runic_overload_limit_note", "Note de limite de surcharge", "Diagnostic d'enchanteur : indique qu'une pièce approche une zone où empiler des runes devient une vraie mauvaise idée.", "Enchantement", 18, quantity);
}

Material MaterialCatalog::createRunicStabilizer(int quantity)
{
    return Material("runic_stabilizer", "Stabilisateur runique", "Composant d'enchanteur qui limite les vibrations entre plusieurs runes. Très utile quand un équipement approche des 5 enchantements.", "Matériau magique", 34, quantity);
}

Material MaterialCatalog::createRareFireRuneCore(int quantity)
{
    return Material("rare_fire_rune_core", "Cœur de rune ignifuge", "Cœur runique plus rare pour une protection feu/chaleur plus sérieuse qu'une simple formule mineure.", "Matériau magique", 58, quantity);
}

Material MaterialCatalog::createRareColdRuneCore(int quantity)
{
    return Material("rare_cold_rune_core", "Cœur de rune antigel", "Cœur runique plus rare pour une protection froid/givre plus fiable qu'une formule mineure.", "Matériau magique", 56, quantity);
}


// EN: createNecromancyWarning declares or implements a focused behavior used by this module.
// FR: createNecromancyWarning déclare ou implémente un comportement précis utilisé par ce module.
Material MaterialCatalog::createNecromancyWarning(int quantity)
{
    return Material("necromancy_warning", "Avertissement nécromantique", "Renseignement sombre : manipuler les os et les ombres peut attirer des effets qui ne pardonnent pas.", "Renseignement", 0, quantity);
}

Material MaterialCatalog::createMagicLearningNotes(int quantity)
{
    return Material("magic_learning_notes", "Notes sur supports magiques", "Notes de bibliothèque : parchemins, grimoires et catalyseurs ne suivent pas tous les mêmes règles.", "Renseignement", 38, quantity);
}

Material MaterialCatalog::createElementalWeaknessNotes(int quantity)
{
    return Material("elemental_weakness_notes", "Notes sur faiblesses élémentaires", "Tableau de terrain sur les résistances, faiblesses et réactions selon matière, race, classe et équipement.", "Renseignement", 44, quantity);
}

// EN: createBeastHide declares or implements a focused behavior used by this module.
// FR: createBeastHide déclare ou implémente un comportement précis utilisé par ce module.
Material MaterialCatalog::createBeastHide(int quantity)
{
    return Material("beast_hide", "Peau de bête robuste", "Matériau solide récupéré sur des bêtes résistantes. Utile pour armures, réparations épaisses et équipements de garde.", "Matériau de monstre", 13, quantity);
}

// EN: createShadowThread declares or implements a focused behavior used by this module.
// FR: createShadowThread déclare ou implémente un comportement précis utilisé par ce module.
Material MaterialCatalog::createShadowThread(int quantity)
{
    return Material("shadow_thread", "Fil d'ombre", "Composant sombre et rare, lié aux assassins, aux morts-vivants et aux ombres de Hazak.", "Matériau sombre", 28, quantity);
}

// EN: createKitsuneEmber declares or implements a focused behavior used by this module.
// FR: createKitsuneEmber déclare ou implémente un comportement précis utilisé par ce module.
Material MaterialCatalog::createKitsuneEmber(int quantity)
{
    return Material("kitsune_ember", "Braise kitsune", "Braise magique instable, utile aux flammes d'Aoi, aux invocations et aux potions avancées.", "Matériau magique", 34, quantity);
}

// EN: createDraconicScaleFragment declares or implements a focused behavior used by this module.
// FR: createDraconicScaleFragment déclare ou implémente un comportement précis utilisé par ce module.
Material MaterialCatalog::createDraconicScaleFragment(int quantity)
{
    return Material("draconic_scale_fragment", "Fragment d'écaille draconique", "Fragment extrêmement résistant, recherché pour protections rares et crafts semi-dragons.", "Matériau rare", 45, quantity);
}

// EN: createUnstableCore declares or implements a focused behavior used by this module.
// FR: createUnstableCore déclare ou implémente un comportement précis utilisé par ce module.
Material MaterialCatalog::createUnstableCore(int quantity)
{
    return Material("unstable_core", "Noyau instable", "Cœur magique dangereux, utilisé pour invocations, alchimie risquée et enchantements expérimentaux.", "Matériau rare", 40, quantity);
}


// EN: createFitoriaFeather declares or implements a focused behavior used by this module.
// FR: createFitoriaFeather déclare ou implémente un comportement précis utilisé par ce module.
Material MaterialCatalog::createFitoriaFeather(int quantity)
{
    return Material("fitoria_feather", "Plume lumineuse de Fitoria", "Fragment de boss : plume presque sacrée, utile aux bénédictions, soins rares et équipements de lumière.", "Fragment de boss", 120, quantity);
}

// EN: createZelefDemonBlood declares or implements a focused behavior used by this module.
// FR: createZelefDemonBlood déclare ou implémente un comportement précis utilisé par ce module.
Material MaterialCatalog::createZelefDemonBlood(int quantity)
{
    return Material("zelef_demon_blood", "Sang démoniaque de Zelef", "Fragment de boss : résidu sombre encore vivant, recherché pour alchimie dangereuse, malédictions et armes démoniaques.", "Fragment de boss", 135, quantity);
}

// EN: createAtlasBrokenPlate declares or implements a focused behavior used by this module.
// FR: createAtlasBrokenPlate déclare ou implémente un comportement précis utilisé par ce module.
Material MaterialCatalog::createAtlasBrokenPlate(int quantity)
{
    return Material("atlas_broken_plate", "Plaque brisée d'Atlas", "Fragment de boss : morceau d'une protection universelle déchue, recherché pour armures lourdes, reliques et réparations extrêmes.", "Fragment de boss", 150, quantity);
}

// EN: createLyknirHuntShard declares or implements a focused behavior used by this module.
// FR: createLyknirHuntShard déclare ou implémente un comportement précis utilisé par ce module.
Material MaterialCatalog::createLyknirHuntShard(int quantity)
{
    return Material("lyknir_hunt_shard", "Fragment de chasse silencieuse", "Fragment de boss : morceau d'une traque sans lune, lié aux marques de proie, aux meutes et aux crafts de chasseurs.", "Fragment de boss", 165, quantity);
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
    return Material("thamarys_origin_scale", "Écaille d'origine de Thamarys", "Fragment de boss : écaille issue d'un souffle draconique ancien, recherchée pour armes et armures draconiques rares.", "Fragment de boss", 225, quantity);
}

// EN: createMojoAncientSeed declares or implements a focused behavior used by this module.
// FR: createMojoAncientSeed déclare ou implémente un comportement précis utilisé par ce module.
Material MaterialCatalog::createMojoAncientSeed(int quantity)
{
    return Material("mojo_ancient_seed", "Graine ancienne de Mojo", "Fragment de boss : graine vivante gardant une mémoire de la forêt, recherchée pour bénédictions naturelles et crafts respectueux.", "Fragment de boss", 205, quantity);
}

// EN: createInakariMirrorShard declares or implements a focused behavior used by this module.
// FR: createInakariMirrorShard déclare ou implémente un comportement précis utilisé par ce module.
Material MaterialCatalog::createInakariMirrorShard(int quantity)
{
    return Material("inakari_mirror_shard", "Fragment de miroir d'Inakari", "Fragment de boss : éclat de reflet kitsune, lié aux illusions, aux talismans et aux protections mentales.", "Fragment de boss", 200, quantity);
}



// EN: createSilentJudgmentSeal declares or implements a focused behavior used by this module.
// FR: createSilentJudgmentSeal déclare ou implémente un comportement précis utilisé par ce module.
Material MaterialCatalog::createSilentJudgmentSeal(int quantity)
{
    return Material("silent_judgment_seal", "Sceau du Jugement Silencieux", "Fragment de boss : sceau muet lié aux ombres, aux fautes répétées et aux crafts de justice.", "Fragment de boss", 230, quantity);
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
    return Material("puppet_nail", "Clou de marionnette", "Fragment de boss : clou cousu à une douleur ancienne, utile aux crafts de contrôle et de contre-coup.", "Fragment de boss", 360, quantity);
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
    return Material("unstable_version_core", "Noyau de version instable", "Fragment de boss : règle condensée par un avatar limité du créateur, dangereusement proche d'une loi vivante.", "Fragment de boss", 420, quantity);
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
    return Material("training_arrows", "Flèches d'entraînement", "Munitions basiques fournies aux classes à arc. Les archers s'en servent pour apprendre à économiser leurs vrais tirs.", "Munition", 1, quantity);
}

Material MaterialCatalog::createTrainingBolts(int quantity)
{
    return Material("training_bolts", "Carreaux d'entraînement", "Munitions basiques pour arbalète ou arme de tir lourde.", "Munition", 2, quantity);
}

Material MaterialCatalog::createTrainingThrowingKnives(int quantity)
{
    return Material("training_throwing_knives", "Couteaux de lancer émoussés", "Projectiles pauvres mais utiles pour une défense à distance courte.", "Munition", 2, quantity);
}

Material MaterialCatalog::createBarbedArrows(int quantity)
{
    return Material("barbed_arrows", "Flèches barbelées", "Munitions spéciales artisanales conçues pour ouvrir des plaies plus difficiles à refermer.", "Munition spéciale", 5, quantity);
}

Material MaterialCatalog::createPiercingBolts(int quantity)
{
    return Material("piercing_bolts", "Carreaux perforants", "Munitions d'arbalète renforcées, pensées pour armures et monstres épais.", "Munition spéciale", 6, quantity);
}

Material MaterialCatalog::createBalancedThrowingKnives(int quantity)
{
    return Material("balanced_throwing_knives", "Couteaux de lancer équilibrés", "Projectiles courts, mieux équilibrés que les couteaux émoussés de départ. Recette utile aux classes mobiles.", "Munition spéciale", 6, quantity);
}

Material MaterialCatalog::createAshArrows(int quantity)
{
    return Material("ash_arrows", "Flèches de cendre", "Munitions expérimentales noircies par alchimie, connues pour favoriser les blessures brûlantes.", "Munition élémentaire", 8, quantity);
}

Material MaterialCatalog::createFrozenBolts(int quantity)
{
    return Material("frozen_bolts", "Carreaux givrés", "Carreaux traités au froid, conçus pour gêner les mouvements et mordre les armures gelées.", "Munition élémentaire", 9, quantity);
}

Material MaterialCatalog::createConductiveKnives(int quantity)
{
    return Material("conductive_knives", "Couteaux conducteurs", "Projectiles métalliques préparés pour les réactions électriques, surtout contre les cibles équipées de métal.", "Munition élémentaire", 9, quantity);
}

Material MaterialCatalog::createVenomArrows(int quantity)
{
    return Material("venom_arrows", "Flèches enduites de venin", "Munitions de chasse préparées avec venin et résidus. Elles peuvent accrocher un poison léger si le tir blesse réellement.", "Munition élémentaire", 10, quantity);
}

Material MaterialCatalog::createShockBolts(int quantity)
{
    return Material("shock_bolts", "Carreaux à pointe conductrice", "Carreaux expérimentaux pensés pour transmettre une décharge courte, surtout contre métal et armures lourdes.", "Munition élémentaire", 11, quantity);
}

Material MaterialCatalog::createSmokeKnives(int quantity)
{
    return Material("smoke_knives", "Couteaux fumigènes", "Projectiles de secours : peu létaux, mais conçus pour gêner la cible et préparer une fuite ou une défense urgente.", "Munition spéciale", 10, quantity);
}

Material MaterialCatalog::createMyceliumLantern(int quantity)
{
    return Material("mycelium_lantern", "Lanterne de mycélium", "Champignon-lampe encore tiède, utile pour potions nocturnes, encres vivantes et repérage en grotte.", "Plante lumineuse", 18, quantity);
}

Material MaterialCatalog::createEchoingResin(int quantity)
{
    return Material("echoing_resin", "Résine d'écho", "Résine qui garde un faible son lorsqu'on la coupe. Sert aux arcs, bâtons et pièges de détection.", "Matériau végétal", 26, quantity);
}

Material MaterialCatalog::createSunDriedClay(int quantity)
{
    return Material("sun_dried_clay", "Argile rouge séchée", "Argile dure cuite par le soleil, parfaite pour réparations, plaques légères et sceaux de fortune.", "Matériau minéral", 16, quantity);
}

Material MaterialCatalog::createMoonlitSalt(int quantity)
{
    return Material("moonlit_salt", "Sel lunaire", "Cristaux pâles récoltés dans les zones sèches la nuit. Très recherché par les alchimistes prudents.", "Cristal alchimique", 34, quantity);
}

Material MaterialCatalog::createOldCoinBundle(int quantity)
{
    return Material("old_coin_bundle", "Lot de vieilles pièces", "Anciennes pièces sans vraie valeur monétaire directe, mais utiles aux collectionneurs et aux faussaires de lore.", "Relique mineure", 22, quantity);
}

Material MaterialCatalog::createGlassMapFragment(int quantity)
{
    return Material("glass_map_fragment", "Fragment de carte de verre", "Morceau translucide gravé d'une portion de carte. Il change légèrement selon l'angle et l'heure.", "Relique cartographique", 42, quantity);
}

Material MaterialCatalog::createLivingVineFiber(int quantity)
{
    return Material("living_vine_fiber", "Fibre de vigne vivante", "Fibre végétale qui tente encore de se contracter. Utile aux liens, robes et réparations magiques souples.", "Matériau végétal", 28, quantity);
}

Material MaterialCatalog::createColdIronNail(int quantity)
{
    return Material("cold_iron_nail", "Clou de fer froid", "Clou lourd sorti d'une mine ancienne. Il semble absorber les vibrations autour de lui.", "Métal froid", 31, quantity);
}

Material MaterialCatalog::createWitchBottle(int quantity)
{
    return Material("witch_bottle", "Bouteille de sorcière vide", "Petite bouteille scellée puis vidée. Les alchimistes jurent qu'elle garde la mémoire des mélanges dangereux.", "Outil alchimique", 38, quantity);
}

Material MaterialCatalog::createForgottenCampTag(int quantity)
{
    return Material("forgotten_camp_tag", "Plaque de camp oubliée", "Plaque d'identification d'un ancien relais. Utile aux quêtes, archives et services de guilde.", "Renseignement matériel", 12, quantity);
}

Material MaterialCatalog::createTinyGearSpring(int quantity)
{
    return Material("tiny_gear_spring", "Petit ressort d'engrenage", "Ressort récupéré sur des mécanismes de mines et automates mineurs. Pratique pour pièges, arbalètes et gadgets.", "Pièce mécanique", 27, quantity);
}

Material MaterialCatalog::createInkedContractScrap(int quantity)
{
    return Material("inked_contract_scrap", "Morceau de contrat encré", "Fragment de contrat trop signé pour être honnête. Les marchands aiment, les gobelins adorent, la guilde se méfie.", "Document", 19, quantity);
}


Material MaterialCatalog::createFireflyIronShell(int quantity)
{
    return Material("firefly_iron_shell", "Carapace de luciole de fer", "Petite coque métallique lumineuse récupérée dans les vergers nocturnes. Utile aux lanternes, pièges fins et petites armures souples.", "Matériau lumineux", 29, quantity);
}

Material MaterialCatalog::createWhisperingArchivePage(int quantity)
{
    return Material("whispering_archive_page", "Page d'archive murmurante", "Page humide qui répète parfois une phrase oubliée. Les bibliothèques l'achètent pour recopier des légendes ou corriger des registres.", "Document ancien", 36, quantity);
}

Material MaterialCatalog::createGreyDrakeScale(int quantity)
{
    return Material("grey_drake_scale", "Écaille de drake gris", "Écaille légère et râpeuse, parfaite pour renforcer des harnais d'éclaireur ou des pointes de lance.", "Matériau draconique", 44, quantity);
}

Material MaterialCatalog::createCarnivalTicketShred(int quantity)
{
    return Material("carnival_ticket_shred", "Morceau de ticket de foire", "Ticket déchiré venant d'une foire abandonnée. Les illusionnistes jurent que le numéro change quand personne ne regarde.", "Relique de foire", 18, quantity);
}

Material MaterialCatalog::createMirrorGlassBead(int quantity)
{
    return Material("mirror_glass_bead", "Perle de verre miroir", "Petite perle réfléchissante utilisée dans les capes de diversion, les pièges et quelques tours d'illusion très douteux.", "Composant illusoire", 33, quantity);
}

Material MaterialCatalog::createTidewornInk(int quantity)
{
    return Material("tideworn_ink", "Encre rongée par la marée", "Encre sombre issue de registres noyés. Elle accroche bien les parchemins, mais tente de s'étaler vers les noms importants.", "Encre magique", 31, quantity);
}

Material MaterialCatalog::createCliffBasilLeaf(int quantity)
{
    return Material("cliff_basil_leaf", "Feuille de basilic des falaises", "Plante salée poussant près du vide. Les soigneurs l'utilisent pour calmer les vertiges et stabiliser les potions d'exploration.", "Plante de falaise", 24, quantity);
}

Material MaterialCatalog::createRustedGearCore(int quantity)
{
    return Material("rusted_gear_core", "Noyau d'engrenage rouillé", "Petit cœur mécanique encore utilisable malgré la rouille. Sert aux pièges, arbalètes, automates et bricolages qui grincent.", "Pièce mécanique", 35, quantity);
}

Material MaterialCatalog::createSaltedRopeKnot(int quantity)
{
    return Material("salted_rope_knot", "Nœud de corde salée", "Nœud durci par le vent, la mer ou les larmes d'un apprenti mal payé. Utile pour escalade, filets et quêtes de secours.", "Matériel d'exploration", 14, quantity);
}

Material MaterialCatalog::createLuminousMothWing(int quantity)
{
    return Material("luminous_moth_wing", "Aile de mite lumineuse", "Aile fragile qui garde une lumière pâle. Les alchimistes la veulent intacte, ce qui est évidemment le plus pénible à obtenir.", "Composant lumineux", 26, quantity);
}


Material MaterialCatalog::createHolyWaterVial(int quantity)
{
    return Material("holy_water_vial", "Fiole d'eau bénite", "Petite fiole consacrée. Elle ne retire pas une vraie malédiction seule, mais aide les rites et diagnostics d'église.", "Matériau sacré", 24, quantity);
}

Material MaterialCatalog::createSanctuaryCandle(int quantity)
{
    return Material("sanctuary_candle", "Cierge de veille", "Cierge préparé pour une prière courte, une veillée ou un cercle de protection léger.", "Matériau sacré", 12, quantity);
}

Material MaterialCatalog::createExorcismIncense(int quantity)
{
    return Material("exorcism_incense", "Encens d'exorcisme", "Encens lourd qui aide l'exorciste à lire les traces sans annoncer trop vite ce qu'il croit voir.", "Matériau sacré", 28, quantity);
}

Material MaterialCatalog::createBlessingNote(int quantity)
{
    return Material("blessing_note", "Note de bénédiction", "Petit document signé par l'église. Il prouve qu'une bénédiction de route a été donnée, sans garantir que la route obéisse.", "Renseignement sacré", 26, quantity);
}

Material MaterialCatalog::createExorcistNote(int quantity)
{
    return Material("exorcist_note", "Note d'exorciste", "Note claire : certaines malédictions partent en plusieurs visites, d'autres exigent de vaincre leur source.", "Renseignement sacré", 30, quantity);
}

Material MaterialCatalog::createCrackedBellClapper(int quantity)
{
    return Material("cracked_bell_clapper", "Battant de cloche fissuré", "Morceau de bronze sacré récupéré près de cloches fendues. Il vibre quand une prière est incomplète.", "Matériau sacré", 37, quantity);
}

Material MaterialCatalog::createSanctuaryWaxSeal(int quantity)
{
    return Material("sanctuary_wax_seal", "Sceau de cire sanctuaire", "Sceau rouge sombre qui fermait des lettres de temple. Les scribes le veulent intact pour authentifier d'anciens serments.", "Document sacré", 41, quantity);
}

Material MaterialCatalog::createChurchNightTestimony(int quantity)
{
    return Material("church_night_testimony", "Témoignage nocturne signé", "Billet recueilli par l'église après une veillée. Il ne donne pas un diagnostic complet, mais prouve qu'un cas de sommeil étrange existe vraiment.", "Preuve d'église", 18, quantity);
}

Material MaterialCatalog::createChurchSleepDiagnosis(int quantity)
{
    return Material("church_sleep_diagnosis", "Diagnostic de chambre endormie", "Lecture prudente d'une chambre où le repos refuse de rester normal. Sert à préparer une résolution plutôt qu'à accuser le patient.", "Preuve d'église", 34, quantity);
}

Material MaterialCatalog::createNamedOathTestimony(int quantity)
{
    return Material("named_oath_testimony", "Témoignage de serment nommé", "Témoignage assez précis pour donner un nom à une promesse qui serre encore quelqu'un. Utile pour briser un serment maudit.", "Preuve d'église", 39, quantity);
}

Material MaterialCatalog::createSharedDreamPattern(int quantity)
{
    return Material("shared_dream_pattern", "Motif de rêve partagé", "Notes croisées sur plusieurs rêves identiques. Le motif reste flou, mais il donne une piste à la bibliothèque.", "Preuve d'église", 31, quantity);
}

Material MaterialCatalog::createCopiedCounterLegend(int quantity)
{
    return Material("copied_counter_legend", "Contre-légende copiée", "Copie propre d'une version opposée d'une légende. Elle sert à refermer une histoire qui s'accrochait au personnage.", "Preuve d'église", 44, quantity);
}

Material MaterialCatalog::createIdentifiedSourceObject(int quantity)
{
    return Material("identified_source_object", "Objet source identifié", "Preuve qu'un objet précis porte la trace au lieu du patient. Nécessaire avant de détruire une source sans faire n'importe quoi.", "Preuve d'église", 48, quantity);
}

Material MaterialCatalog::createSealableSourceSketch(int quantity)
{
    return Material("sealable_source_sketch", "Croquis de source scellable", "Croquis annoté d'un seuil ou d'une source instable. Il permet de sceller la trace sans prétendre vaincre ce qui vit derrière.", "Preuve d'église", 52, quantity);
}

Material MaterialCatalog::createPactBreakWitness(int quantity)
{
    return Material("pact_break_witness", "Témoin de pacte rompu", "Note signée par un prêtre après avoir entendu la contrepartie exacte d'un pacte. Sert à rompre un accord maudit sans inventer la faute.", "Preuve d'église", 46, quantity);
}

Material MaterialCatalog::createSourceDefeatNotice(int quantity)
{
    return Material("source_defeat_notice", "Note de source vaincue", "Preuve simple : une présence liée à un seuil a été réellement affrontée. L'église peut alors retirer la trace sans prétendre avoir gagné le combat à ta place.", "Preuve d'église", 55, quantity);
}

Material MaterialCatalog::createPurifiedSourceAshes(int quantity)
{
    return Material("purified_source_ashes", "Cendres de source purifiée", "Petites cendres froides conservées après la destruction contrôlée d'un objet source. Elles prouvent que la trace n'a pas seulement été masquée.", "Suite d'église", 54, quantity);
}

Material MaterialCatalog::createBrokenOathRecord(int quantity)
{
    return Material("broken_oath_record", "Acte de serment brisé", "Acte court signé après la rupture d'un serment maudit. Il ne pardonne pas tout, mais il prouve que la corde ne serre plus.", "Suite d'église", 58, quantity);
}

Material MaterialCatalog::createClosedCounterLegend(int quantity)
{
    return Material("closed_counter_legend", "Contre-légende refermée", "Copie annotée d'une légende lue jusqu'à sa vraie fin. Les cauchemars partagés peuvent encore revenir en souvenir, mais plus comme ordre.", "Suite d'église", 57, quantity);
}

Material MaterialCatalog::createSealedSourceMark(int quantity)
{
    return Material("sealed_source_mark", "Marque de source scellée", "Signe discret conservé après le scellement d'un seuil ou d'un lieu compris. L'endroit reste là, mais sa trace ne suit plus le joueur.", "Suite d'église", 62, quantity);
}

Material MaterialCatalog::createReleasedPactRecord(int quantity)
{
    return Material("released_pact_record", "Acte de pacte libéré", "Trace écrite d'un pacte volontaire rompu après aveu de la contrepartie. Le choix a existé, mais il ne réclame plus le personnage.", "Suite d'église", 60, quantity);
}

Material MaterialCatalog::createConfirmedSourceSilence(int quantity)
{
    return Material("confirmed_source_silence", "Silence de source confirmé", "Note de Père Orwan après confirmation d'une source réellement vaincue. La trace n'a plus de voix à laquelle répondre.", "Suite d'église", 63, quantity);
}

Material MaterialCatalog::createBlueMistReed(int quantity)
{
    return Material("blue_mist_reed", "Roseau de brume bleue", "Roseau humide qui garde une goutte de brume froide. Utile aux potions de discrétion, de souffle et de voyage.", "Plante aquatique", 27, quantity);
}

Material MaterialCatalog::createMistglassPearl(int quantity)
{
    return Material("mistglass_pearl", "Perle de verre-brume", "Petite perle translucide formée dans les canaux. Elle brouille les reflets et plaît aux illusionnistes prudents.", "Composant de brume", 52, quantity);
}

Material MaterialCatalog::createWhiteBoneChalk(int quantity)
{
    return Material("white_bone_chalk", "Craie d'os blanc", "Craie minérale extraite d'une carrière pâle. Les mages l'utilisent pour tracer des cercles qui ne tiennent pas toujours parole.", "Minéral rituel", 24, quantity);
}

Material MaterialCatalog::createBuriedGiantChip(int quantity)
{
    return Material("buried_giant_chip", "Éclat de géant enfoui", "Fragment d'os ou de pierre trop lourd pour sa taille. Personne n'aime demander de quel géant il vient.", "Relique osseuse", 58, quantity);
}

Material MaterialCatalog::createSmugglerToken(int quantity)
{
    return Material("smuggler_token", "Jeton de contrebandier", "Jeton marqué d'un pont et d'une lune. Les marchés douteux l'échangent contre silence, raccourcis et problèmes.", "Jeton illégal", 30, quantity);
}

Material MaterialCatalog::createSealedDebtSlip(int quantity)
{
    return Material("sealed_debt_slip", "Billet de dette scellé", "Petit papier trop bien plié pour être honnête. Il vaut quelque chose seulement si le débiteur respire encore.", "Document marchand", 46, quantity);
}

Material MaterialCatalog::createBlackMarketBarterSeal(int quantity)
{
    return Material("black_market_barter_seal", "Sceau de troc douteux", "Marque discrète du marché noir : pas de valeur officielle, mais certains vendeurs l'acceptent mieux que l'or propre.", "Marché noir", 24, quantity);
}


Material MaterialCatalog::createCityRepairDaysMarker(int quantity)
{
    return Material("city_repair_days_marker", "Jours de réparations de ville", "État temporaire : après un malheur urbain, beaucoup de commerces réparent, trient ou protègent leurs stocks. La quantité indique les jours restants.", "Ville", 0, quantity);
}

Material MaterialCatalog::createCityDamageNotice(int quantity)
{
    return Material("city_damage_notice", "Avis de dégâts en ville", "Annonce placardée après une attaque, un incendie ou une catastrophe locale. Les boutiques ouvertes sont rares tant que les réparations durent.", "Ville", 0, quantity);
}

Material MaterialCatalog::createCityDefenseMedal(int quantity)
{
    return Material("city_defense_medal", "Attestation de défense de ville", "Preuve que le personnage a aidé à repousser une menace majeure. Utile pour la réputation locale et certains services municipaux.", "Ville", 34, quantity);
}

Material MaterialCatalog::createCityDefenseGratitudeMarker(int quantity)
{
    return Material("city_defense_gratitude_days_marker", "Reconnaissance de défense active", "État temporaire : après une défense de ville réussie, plusieurs commerçants acceptent une petite remise par gratitude. La quantité indique les jours restants.", "Ville", 0, quantity);
}

Material MaterialCatalog::createCityEventRecentMarker(int quantity)
{
    return Material("city_event_recent_days_marker", "Ville encore occupée par un événement", "État temporaire : la ville vient déjà de gérer une affiche, une fête ou un ordre public. La quantité indique combien de jours le bureau évite d'empiler les animations.", "Ville", 0, quantity);
}

Material MaterialCatalog::createCityRepairReceipt(int quantity)
{
    return Material("city_repair_receipt", "Reçu d'aide aux réparations", "Preuve qu'un habitant, artisan ou garde a reçu de l'aide pendant les réparations de la ville.", "Ville", 18, quantity);
}

Material MaterialCatalog::createWeepingStoneTear(int quantity)
{
    return Material("weeping_stone_tear", "Larme de pierre pleureuse", "Goutte minérale tombée d'une statue ancienne. Elle reste froide même dans une main fiévreuse.", "Minéral étrange", 33, quantity);
}

Material MaterialCatalog::createPetrifiedRosePetals(int quantity)
{
    return Material("petrified_rose_petals", "Pétales de rose pétrifiée", "Pétales devenus pierre sans perdre leur forme. Les nobles les achètent pour des hommages, les mages pour de mauvaises idées.", "Relique florale", 49, quantity);
}
