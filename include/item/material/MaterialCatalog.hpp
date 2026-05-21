// EN: MaterialCatalog.hpp briefly defines this Dinotofu module and its responsibilities.
// FR: MaterialCatalog.hpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.
// English: Creates early stackable materials, plants, books and information entries.
// Français : Crée les premiers matériaux, plantes, livres et renseignements empilables.

#ifndef INCLUDE_ITEM_MATERIAL_MATERIALCATALOG_HPP
#define INCLUDE_ITEM_MATERIAL_MATERIALCATALOG_HPP

#include "item/material/Material.hpp"

#include <string>

class MaterialCatalog
{
public:
    static Material createById(const std::string& id, int quantity = 1, const std::string& quality = "normal");
    static Material createGoblinEar(int quantity = 1);
    static Material createWolfFang(int quantity = 1);
    static Material createRustedMetalFragment(int quantity = 1);
    static Material createWornLeatherPiece(int quantity = 1);
    static Material createMountainBlueFlower(int quantity = 1);
    static Material createBitterHealingLeaf(int quantity = 1);
    static Material createCommonGoblinNotes(int quantity = 1);
    static Material createCommonWolfNotes(int quantity = 1);
    static Material createBasicPlantManual(int quantity = 1);
    static Material createBasicMagicManual(int quantity = 1);
    static Material createCrackedBone(int quantity = 1);
    static Material createArcaneDust(int quantity = 1);
    static Material createSlimeResidue(int quantity = 1);
    static Material createVariationResidue(int quantity = 1);
    static Material createBattleTornBadge(int quantity = 1);
    static Material createWeakRepairKit(int quantity = 1);
    static Material createMediumRepairKit(int quantity = 1);
    static Material createBigRepairKit(int quantity = 1);
    static Material createTinkererCompleteRepairKit(int quantity = 1);
    static Material createSmallRepairKit(int quantity = 1);
    static Material createReinforcedRepairKit(int quantity = 1);
    static Material createSpecialAdventurerNotes(int quantity = 1);
    static Material createSummoningNotes(int quantity = 1);
    static Material createBossIdentityScrap(int quantity = 1);
    static Material createPotionRecipePage(int quantity = 1);
    static Material createRepairRecipePage(int quantity = 1);
    static Material createAdvancedMonsterNotes(int quantity = 1);
    static Material createNecromancyWarning(int quantity = 1);
    static Material createBeastHide(int quantity = 1);
    static Material createShadowThread(int quantity = 1);
    static Material createKitsuneEmber(int quantity = 1);
    static Material createDraconicScaleFragment(int quantity = 1);
    static Material createUnstableCore(int quantity = 1);
    static Material createFitoriaFeather(int quantity = 1);
    static Material createZelefDemonBlood(int quantity = 1);
    static Material createAtlasBrokenPlate(int quantity = 1);
    static Material createLyknirHuntShard(int quantity = 1);
    static Material createGrinkaAvariceCoin(int quantity = 1);
    static Material createAzelanosDarkCrownShard(int quantity = 1);
    static Material createThamarysOriginScale(int quantity = 1);
    static Material createMojoAncientSeed(int quantity = 1);
    static Material createInakariMirrorShard(int quantity = 1);
    static Material createSilentJudgmentSeal(int quantity = 1);
    static Material createAnomalyGlitchFragment(int quantity = 1);
    static Material createDeadMinuteGear(int quantity = 1);
    static Material createBuriedBoneLullaby(int quantity = 1);
    static Material createBorosWarMark(int quantity = 1);
    static Material createAnastasiaBoundHeart(int quantity = 1);
    static Material createLexiorJusticeSplinter(int quantity = 1);
    static Material createLunarDreamFragment(int quantity = 1);
    static Material createElementalFusionCore(int quantity = 1);
    static Material createHumanWillFragment(int quantity = 1);
    static Material createConsciousLuckShard(int quantity = 1);
    static Material createProgressionSeal(int quantity = 1);
    static Material createAbsentThroneFragment(int quantity = 1);
    static Material createLostNameFragment(int quantity = 1);
    static Material createAldebarothAbyssResidue(int quantity = 1);
    static Material createTwinParadoxShard(int quantity = 1);
    static Material createOberionOriginThread(int quantity = 1);
    static Material createFacelessBreath(int quantity = 1);
    static Material createPuppetNail(int quantity = 1);
    static Material createMoiranFateThread(int quantity = 1);
    static Material createLostSoulAntler(int quantity = 1);
    static Material createGorvaldRoyalBlood(int quantity = 1);
    static Material createSeranaRoyalFang(int quantity = 1);
    static Material createDraiiteBlackSilk(int quantity = 1);
    static Material createBrokenMirrorShard(int quantity = 1);
    static Material createUnstableVersionCore(int quantity = 1);
    static Material createPrecisionHarvestTools(int quantity = 1);
    static Material createPreservationVials(int quantity = 1);
    static Material createCleanHarvestManual(int quantity = 1);
    static Material createMonsterDissectionGuide(int quantity = 1);
};

#endif
