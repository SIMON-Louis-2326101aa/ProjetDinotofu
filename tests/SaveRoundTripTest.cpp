#include "entity/Player.hpp"
#include "quest/Quest.hpp"
#include "save/SaveManager.hpp"
#include "save/SaveSchemaVersion.hpp"
#include "item/weapon/Weapon.hpp"
#include "item/consumable/ConsumableCatalog.hpp"
#include "item/material/MaterialCatalog.hpp"
#include "story/HestiaSoulArtifactSystem.hpp"
#include "story/HestiaCompanionSystem.hpp"
#include "world/clan/ClanHeadquartersSystem.hpp"
#include "world/clan/ClanBlacksmithSystem.hpp"

#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <algorithm>

int main()
{
    namespace fs = std::filesystem;

    fs::remove_all("assets/saves");

    Player original;
    original.setRace(CharacterRace::Demon);
    original.setAppearanceProfile(18, "Test", "Morphologie standard");
    assert(original.startMorphologyProgram("Robuste"));
    assert(original.performMorphologySession());
    original.setLoadedProgress(37, 4242, 73);
    CoinBreakdown originalCoins;
    originalCoins.platinum = 2;
    originalCoins.gold = 51;
    originalCoins.electrum = 7;
    originalCoins.iron = 13;
    originalCoins.copper = 4;
    original.getInventory().setCoinStacks(originalCoins);
    const long long originalCopperTotal = original.getInventory().getTotalCopper();
    original.setLoadedRealPlayTimeSeconds(3661);

    // V3.51.51: bag tier and local vault optimizations / physical vault money are persistent.
    original.setLoadedBackpackTier(3);
    PlayerCityVault valebrumeVault;
    valebrumeVault.cityId = "valebrume";
    valebrumeVault.purchased = true;
    valebrumeVault.level = 5;
    valebrumeVault.placeOptimized = true;
    valebrumeVault.preparedStorage = true;
    // V3.51.52: paid security and a pending raid warning must survive save/reload.
    valebrumeVault.securityDaysRemaining = 11;
    valebrumeVault.raidPending = true;
    valebrumeVault.raidDeadlineDay = 42;
    valebrumeVault.lastRaidCheckDay = 35;
    CoinBreakdown vaultCoins;
    vaultCoins.gold = 12;
    vaultCoins.copper = 345;
    valebrumeVault.inventory.setCoinStacks(vaultCoins);
    original.setLoadedCityState(
        true, 5, "valebrume", valebrumeVault.inventory, {}, {valebrumeVault}, true, true
    );
    original.markCityVaultRecallMessengerSent("port_lanterne", "valebrume");
    // V3.51.54: an unknown vault-theft investigation and exact stolen equipment survive save/reload.
    PlayerVaultTheftCase theftCase;
    theftCase.caseId = "vault_theft:valebrume:test";
    theftCase.cityId = "valebrume";
    theftCase.createdDay = 12;
    theftCase.lastLeadDay = 13;
    theftCase.clues = 2;
    theftCase.holderType = "receiver";
    theftCase.holderLabel = "Serrin le brocanteur";
    theftCase.holderWorldContactId = "valebrume_receleur_brocanteur";
    theftCase.holderLocationId = "market";
    theftCase.holderLocationLabel = "Marché local de Valebrume";
    theftCase.holderExplorationBiome = "Route commerciale";
    theftCase.holderObservationCount = 2;
    theftCase.negotiationAttempts = 1;
    theftCase.confrontationAttempts = 2;
    theftCase.holderPressure = 65;
    theftCase.holderCooperation = -15;
    theftCase.holderClosedUntilDay = 17;
    theftCase.holderReportedToGuard = true;
    theftCase.lastHolderInteractionDay = 8;
    theftCase.holderAlerted = true;
    theftCase.stolenCopper = 321;
    theftCase.hasWeapon = true;
    theftCase.stolenWeapon = Weapon("Lame volée témoin", "Arme exacte conservée dans un dossier d'enquête.", 640, WeaponType::Sword, 8, 13, 4, 120);
    theftCase.stolenWeapon.setPersistentId("vault_theft_weapon_exact_001");
    theftCase.stolenWeapon.setPersonalRecoveryItem(true);
    theftCase.stolenWeapon.setRecoveryPriority(96);
    theftCase.stolenMaterialId = "runic_iron_shard";
    theftCase.stolenMaterialName = "Éclat de fer runique";
    theftCase.stolenMaterialQuality = "high";
    theftCase.stolenMaterialQuantity = 3;
    original.setLoadedVaultTheftCases({theftCase});
    assert(original.spendMana(std::min(11, original.getMana())));
    const int originalMana = original.getMana();
    original.applyManaExhaustion(1);
    original.setLanguageKnowledgeLevel("gobelin", 2, false);
    original.setLanguageStudyProgress("gobelin", 43);
    Weapon rememberedWeapon(
        "Épée témoin",
        "Arme créée uniquement pour vérifier la mémoire persistante d'un exemplaire.",
        125,
        WeaponType::Sword,
        3,
        7,
        1,
        80
    );
    rememberedWeapon.setPersistentId("roundtrip_weapon_memory_001");
    original.getInventory().addWeapon(rememberedWeapon);

    // V3.51.56: generic ownership binding is optional persistent metadata, not a text heuristic.
    Weapon soulBoundWeapon(
        "Lame liée round-trip",
        "Le nom seul ne décide rien : la métadonnée porte la liaison.",
        777,
        WeaponType::Sword,
        6,
        11,
        2,
        110
    );
    soulBoundWeapon.setPersistentId("roundtrip_soulbound_weapon_001");
    soulBoundWeapon.setSoulBound();
    original.getInventory().addWeapon(soulBoundWeapon);

    Consumable soulBoundConsumable = ConsumableCatalog::createBasicHealingPotion();
    soulBoundConsumable.setSoulBound();
    original.getInventory().addConsumable(soulBoundConsumable);

    Material soulBoundMaterial = MaterialCatalog::createById("arcane_dust", 2, "normal");
    soulBoundMaterial.setSoulBound();
    original.getInventory().addMaterial(soulBoundMaterial);
    Material freeSameMaterial = MaterialCatalog::createById("arcane_dust", 3, "normal");
    original.getInventory().addMaterial(freeSameMaterial);

    // V3.51.59: recovery semantics are persistent and remain distinct from ownership binding.
    Weapon questRecoveryWeapon(
        "Lame de quête round-trip",
        "Objet transférable mais prioritaire si l'équipe doit choisir quoi ramener.",
        910, WeaponType::Sword, 7, 12, 3, 125);
    questRecoveryWeapon.setPersistentId("roundtrip_recovery_weapon_001");
    questRecoveryWeapon.setQuestRecoveryItem(true);
    questRecoveryWeapon.setUniqueRecoveryItem(true);
    questRecoveryWeapon.setRecoveryPriority(88);
    original.getInventory().addWeapon(questRecoveryWeapon);

    Material personalRecoveryMaterial(
        "recovery_roundtrip_ore", "Minerai personnel", "Pile protégée du tri automatique.", "minerai", 12, 2);
    personalRecoveryMaterial.setPersonalRecoveryItem(true);
    personalRecoveryMaterial.setRecoveryPriority(95);
    original.getInventory().addMaterial(personalRecoveryMaterial);
    Material ordinaryRecoveryMaterial(
        "recovery_roundtrip_ore", "Minerai personnel", "Même ID mais métadonnée ordinaire.", "minerai", 12, 3);
    original.getInventory().addMaterial(ordinaryRecoveryMaterial);

    // V3.51.61: exact important loot left after KO persists as a battlefield recovery cache.
    Inventory recoveryCacheInventory;
    Weapon battlefieldWeapon(
        "Lame laissée sur le terrain",
        "Arme exacte utilisée pour vérifier une récupération différée persistante.",
        1337, WeaponType::Sword, 9, 15, 4, 140);
    battlefieldWeapon.setPersistentId("roundtrip_battlefield_weapon_001");
    battlefieldWeapon.loseDurability(17);
    battlefieldWeapon.addEnchantment("Éclair dormant");
    battlefieldWeapon.setPersonalRecoveryItem(true);
    battlefieldWeapon.setUniqueRecoveryItem(true);
    battlefieldWeapon.setRecoveryPriority(99);
    recoveryCacheInventory.addWeapon(battlefieldWeapon);
    const std::string battlefieldCacheId = original.createBattlefieldRecoveryCache(
        "roundtrip_postko", "Ruines du vieux pont", recoveryCacheInventory);
    assert(!battlefieldCacheId.empty());
    {
        auto caches = original.getBattlefieldRecoveryCaches();
        caches.front().custodyState = "civilian";
        caches.front().custodyLabel = "lot recueilli par un témoin";
        caches.front().lastMovementDay = original.getWorldDaysElapsed();
        caches.front().movementCount = 2;
        caches.front().locationKnown = true;
        original.setLoadedBattlefieldRecoveryCaches(caches);
    }
    assert(original.hasOpenBattlefieldRecoveryCacheForCity("valebrume"));

    // V3.51.61: a criminal contact remembers pressure/arrest across individual theft cases.
    PlayerCrimeContactState& persistentContact = original.getOrCreateCrimeContactState(
        "valebrume_receleur_brocanteur", "Serrin le brocanteur");
    persistentContact.encounters = 4;
    persistentContact.observations = 2;
    persistentContact.negotiations = 1;
    persistentContact.guardConfrontations = 1;
    persistentContact.pressure = 72;
    persistentContact.cooperation = 18;
    persistentContact.reportedToGuard = true;
    persistentContact.detainedUntilDay = original.getWorldDaysElapsed() + 4;
    persistentContact.provisionalDetentions = 2;
    persistentContact.releases = 1;
    persistentContact.returnsAfterFlight = 3;
    persistentContact.lastReleaseDay = original.getWorldDaysElapsed() - 1;
    persistentContact.lastReturnDay = original.getWorldDaysElapsed() - 2;
    persistentContact.knownRole = "receleur";
    persistentContact.homeCityId = "valebrume";
    persistentContact.networkId = "marche_gris";
    persistentContact.networkWarnings = 3;
    persistentContact.lastNetworkWarningDay = original.getWorldDaysElapsed();
    persistentContact.lastWarningSourceId = "rocheveille_receleur_peseur";
    persistentContact.arrested = false;
    persistentContact.reliability = 67;
    persistentContact.favorBalance = -24;
    persistentContact.rivalry = 38;
    persistentContact.fulfilledDeals = 5;
    persistentContact.brokenDeals = 2;
    persistentContact.ruptureUntilDay = original.getWorldDaysElapsed() + 6;
    persistentContact.lastInteractionDay = original.getWorldDaysElapsed();

    PlayerCityVault boundVault = original.getCityVaultRecords().front();
    Weapon vaultSoulBoundWeapon(
        "Relique liée de coffre",
        "Objet lié stocké sans changer de propriétaire.",
        880,
        WeaponType::Sword,
        7,
        12,
        2,
        130
    );
    vaultSoulBoundWeapon.setPersistentId("roundtrip_vault_soulbound_weapon_001");
    vaultSoulBoundWeapon.setSoulBound();
    boundVault.inventory.addWeapon(vaultSoulBoundWeapon);
    original.setLoadedCityState(true, 5, "valebrume", boundVault.inventory, {}, {boundVault}, true, true);
    original.recordHistoricalEvent(
        "item_memory_enemy_signature",
        rememberedWeapon.getPersistentId(),
        "L'Épée témoin a encaissé la signature d'un ennemi de test",
        true
    );

    original.rememberNpcFact(
        "Archiviste Meron",
        "discovered_writing",
        "archives_noyees_stela",
        "Stèle elfique retrouvée dans les Archives noyées",
        "temoignage_joueur",
        original.getName(),
        74,
        1,
        "stela_elfique",
        "bibliotheque"
    );

    Quest quest;
    quest.id = "roundtrip_foreign_contract";
    quest.title = "Contrat gobelin de test";
    quest.objective = "Comprendre le message gobelin.";
    quest.guildQuest = true;
    quest.accepted = true;
    quest.target = 1;
    quest.requiredLanguage = "gobelin";
    quest.requiredLanguageLevel = 2;
    quest.sourceLanguageText = "Grakka tik, vor nakka.";
    quest.rewardCoins.copper = 17;
    quest.rewardCoins.iron = 2;
    quest.rewardCoins.gold = 3;
    quest.phaseTwoChecked = true;
    quest.phaseTwoActive = true;
    quest.phaseTwoMandatory = true;
    quest.phaseTwoTitle = "Suite de test";
    quest.phaseTwoPreviousSummary = "Phase 1 terminée : message compris.";
    quest.phaseTwoReason = "Un témoin relie le message à un second relais.";
    quest.phaseTwoObjective = "Vérifier le relais secondaire.";
    quest.phaseTwoProgress = 1;
    quest.phaseTwoTarget = 2;
    quest.phaseTwoAttempts = 3;
    quest.phaseTwoRewardExperience = 19;
    quest.phaseTwoRewardGold = 7;
    quest.phaseTwoRewardKind = "information";
    quest.phaseTwoRewardDetail = "Information recoupée auprès du client.";
    quest.phaseTwoTravelRequired = true;
    quest.phaseTwoTravelSourceCityId = "valebrume";
    quest.phaseTwoTravelDestinationCityId = "rocheveille";
    quest.phaseTwoTravelDistanceKm = 35;
    quest.intercityMission = true;
    quest.intercityKind = "information";
    quest.intercitySourceCityId = "valebrume";
    quest.intercityRouteCityIds = "rocheveille|port_lanterne";
    quest.intercityPayloadName = "dossier de test scellé";
    quest.intercityStopIndex = 1;
    quest.intercityTotalDistanceKm = 88;
    quest.intercityTravelRewardGold = 23;
    quest.intercityTravelRewardExperience = 17;
    original.getQuestLog().getQuests().push_back(quest);

    Quest bountyQuest;
    bountyQuest.id = "roundtrip_targeted_bounty";
    bountyQuest.title = "Prime ciblée de round-trip";
    bountyQuest.objective = "Retrouver puis intercepter une cible mobile.";
    bountyQuest.objectiveType = "bounty";
    bountyQuest.guildQuest = true;
    bountyQuest.accepted = true;
    bountyQuest.target = 1;
    bountyQuest.bountyMission = true;
    bountyQuest.bountyTargetName = "Vela la Passeuse";
    bountyQuest.bountyTargetTemperament = "méfiant méthodique";
    bountyQuest.bountyTargetId = "vela_la_passeuse";
    bountyQuest.bountyTargetHomeCityId = "valebrume";
    bountyQuest.bountyTargetNetworkId = "courriers_nocturnes";
    bountyQuest.bountyIssuerId = "diane_guilde_valebrume";
    bountyQuest.bountyIssuerName = "Diane";
    bountyQuest.bountyIssuerOrganizationId = "guilde_aventuriers_valebrume";
    bountyQuest.bountyIssuerOrganizationName = "Guilde des aventuriers de Valebrume";
    bountyQuest.bountyIssuerPrefersLiveCapture = true;
    bountyQuest.bountyCloseRelationId = "vela_soeur";
    bountyQuest.bountyCloseRelationName = "Meline";
    bountyQuest.bountyCloseRelationType = "soeur";
    bountyQuest.bountyStage = "preparation";
    bountyQuest.bountyClues = 4;
    bountyQuest.bountyInvestigationNoise = 31;
    bountyQuest.bountyInformantExposure = 18;
    bountyQuest.bountyTargetAwareness = 72;
    bountyQuest.bountyTargetSuspectsHunt = true;
    bountyQuest.bountyTargetKnowsHunt = false;
    bountyQuest.bountyTargetResources = 63;
    bountyQuest.bountyTargetLocalSupport = 44;
    bountyQuest.bountyTargetMobility = 77;
    bountyQuest.bountyTargetEscapeAttempts = 1;
    bountyQuest.bountyTargetKnowsTerrain = true;
    bountyQuest.bountyTargetInjured = false;
    bountyQuest.bountyExitRecon = 2;
    bountyQuest.bountyInformantNetwork = 1;
    bountyQuest.bountyAmbushPreparation = 2;
    bountyQuest.bountyShadowingPreparation = 1;
    bountyQuest.bountyRouteVariation = 1;
    bountyQuest.bountyDecoyQuality = 0;
    bountyQuest.bountyCounterSurveillance = 1;
    bountyQuest.bountyRefugeConcealment = 0;
    bountyQuest.bountyDangerRating = 4;
    bountyQuest.bountyNotorietyRating = 3;
    bountyQuest.bountyDaysAtLarge = 117;
    bountyQuest.bountyBaseRewardGold = 240;
    bountyQuest.bountyRewardProfileApplied = true;
    bountyQuest.bountyRegionalProfileApplied = true;
    bountyQuest.bountyCurrentCityId = "port_lanterne";
    bountyQuest.bountyCurrentBiome = "Marché sous les ponts";
    bountyQuest.bountyVisitedCityIds = "valebrume|port_lanterne";
    bountyQuest.bountyRelocationCount = 1;
    bountyQuest.bountyTransitTrailKind = "coach_registry";
    bountyQuest.bountyTransitTrailFromCityId = "valebrume";
    bountyQuest.bountyTransitTrailReliability = 81;
    bountyQuest.bountyTransitTrailAvailable = true;
    bountyQuest.bountyRegionalProfileCityId = "port_lanterne";
    bountyQuest.bountyRegionalConsequencesApplied = true;
    bountyQuest.bountyResolution = "captured_combat";
    original.getQuestLog().getQuests().push_back(bountyQuest);

    Quest incidentQuest;
    incidentQuest.id = "roundtrip_trade_incident";
    incidentQuest.title = "Incident de convoi round-trip";
    incidentQuest.objective = "Préparer puis sécuriser un convoi.";
    incidentQuest.objectiveType = "service";
    incidentQuest.accepted = true;
    incidentQuest.target = 2;
    incidentQuest.progress = 1;
    incidentQuest.tradeIncidentMission = true;
    incidentQuest.tradeIncidentId = "incident:test:001";
    incidentQuest.tradeIncidentKind = "cargaison partiellement volée";
    incidentQuest.tradeIncidentSourceCityId = "rocheveille";
    incidentQuest.tradeIncidentDestinationCityId = "port_lanterne";
    incidentQuest.tradeIncidentGoodsType = 4;
    incidentQuest.tradeIncidentSeverity = 3;
    incidentQuest.tradeIncidentStage = "action";
    incidentQuest.tradeIncidentAttempts = 2;
    incidentQuest.tradeIncidentCreatesBountyLead = true;
    incidentQuest.tradeIncidentBountyPublished = false;
    original.getQuestLog().getQuests().push_back(incidentQuest);

    const std::string rivalId = original.createRivalFromEnemy(
        "Hobgobelin balafré", "hobgobelin", 18, 240, 31,
        "a survécu à une embuscade", "calculateur", 47
    );
    original.recordRivalEscape(rivalId, "route_commerciale");
    original.recordRivalWound(rivalId, 2);
    original.recordRivalReturn(rivalId, "marche_sous_les_ponts");
    Material rivalStolenMaterial(
        "runic_iron_shard",
        "Éclat de fer runique",
        "Matériau subtilisé pour vérifier la persistance de la dette de rival.",
        "Matériau",
        3,
        2,
        "high"
    );
    original.recordRivalStolenCopper(rivalId, 11);
    original.recordRivalStolenMaterial(rivalId, rivalStolenMaterial, 2);
    original.recordCanonicalEvent(
        "techniques_combinees_alliees",
        "Aline|Boros:Feu croisé",
        "Aline + Boros : Feu croisé",
        7
    );

    assert(original.getAmmunitionSelectionPolicy() == "ask");
    assert(original.getMagicMaterialSelectionPolicy() == "ask");
    assert(original.getLootPickupPolicy() == "all");
    original.setAmmunitionSelectionPolicy("favorite");
    original.setMagicMaterialSelectionPolicy("auto");
    original.setLootPickupPolicy("filter");
    original.setLootCategoryIgnored("armor", true);
    original.setLootMaterialIgnored("slime_residue", true);
    original.getInventory().setPinnedItemKey("material:arcane_dust", true);
    original.getInventory().setFavoriteAmmunitionId("training_arrows", true);
    original.getInventory().setLockedItemKey("consumable:Potion de soin", true);
    original.getInventory().setTrackedMaterialId("runic_iron_shard", true);
    assert(original.getInventory().isPinnedItemKey("material:arcane_dust"));
    assert(original.getInventory().isFavoriteAmmunitionId("training_arrows"));
    assert(original.getInventory().isLockedItemKey("consumable:Potion de soin"));
    assert(original.getInventory().isTrackedMaterialId("runic_iron_shard"));


    // V3.51.68: logement, Hestia et son artefact évolutif survivent au round-trip.
    PlayerHomeState roundTripHome;
    roundTripHome.owned = true;
    roundTripHome.cityId = "valebrume";
    roundTripHome.fullyPurchased = false;
    roundTripHome.rentAmount = 280;
    roundTripHome.nextRentDueDay = 17;
    roundTripHome.npcSpouseName = "Partenaire test";
    original.setLoadedHomeState(roundTripHome);

    PlayerHestiaStoryState roundTripHestia;
    roundTripHestia.rescued = true;
    roundTripHestia.materialized = true;
    roundTripHestia.residence = "player_home";
    roundTripHestia.trust = 64;
    roundTripHestia.awakeningStage = 1;
    roundTripHestia.lastVisitDay = 9;
    roundTripHestia.nextSmallFindDay = 13;
    original.setLoadedHestiaStoryState(roundTripHestia);

    PlayerHestiaArtifactState roundTripArtifact;
    roundTripArtifact.granted = true;
    roundTripArtifact.level = 16;
    roundTripArtifact.killsTowardNextLevel = 7;
    roundTripArtifact.primaryForm = "faux";
    roundTripArtifact.secondaryForm = "arbalète";
    roundTripArtifact.activeForm = "secondary";
    roundTripArtifact.secondFormUnlocked = true;
    roundTripArtifact.absorbedEssences = {{"gobelin", 5}, {"squelette", 3}};
    original.setLoadedHestiaArtifactState(roundTripArtifact);
    assert(HestiaSoulArtifactSystem::equip(original));
    original.recordCanonicalEvent("hestia_companion_config", "loadout", "class=Mage | weapon=Bâton / catalyseur | armor=Tenue légère", 1);
    original.recordCanonicalEvent("hestia_companion_training", "practice", "Séance d'apprentissage volontaire avec Hestia", 6);

    // V3.51.79: HQ progression, mission cashbox and an in-progress upgrade survive save/reload.
    original.recordCanonicalEvent("clan_hq_state", "built", "city=valebrume | built_day=1 | building_level=1 | beds=2 | quest_board=1 | counter=1", 1);
    original.recordCanonicalEvent("clan_hq_building_expansions", "level_2", "Agrandissement général terminé : QG niveau 2", 1);
    original.recordCanonicalEvent("clan_hq_extra_beds", "bed_3", "Nouvelle chambre de recrue terminée : capacité 3", 1);
    original.recordCanonicalEvent("clan_hq_quest_board_upgrades", "level_2", "Tableau de quêtes du clan amélioré au niveau 2", 1);
    original.recordCanonicalEvent("clan_hq_mission_chest_upgrades", "level_1", "Caisse de missions installée au niveau 1", 1);
    original.recordCanonicalEvent("clan_hq_mission_chest_deposits", "funds", "Versements de test", 1375);
    original.recordCanonicalEvent("clan_hq_mission_chest_withdrawals", "funds", "Retraits de test", 250);
    original.recordCanonicalEvent("clan_hq_common_room_bar_upgrades", "level_1", "Bar / salle commune installé au niveau 1", 1);
    original.recordCanonicalEvent("clan_hq_bar_visits", "day_1", "Moment social de test", 1);
    original.recordCanonicalEvent("clan_hq_comfort_decor_upgrades", "level_1", "Confort et décoration du QG niveau 1", 1);
    original.recordCanonicalEvent("clan_hq_furnishings", "comfort_decor_level_1", "Mobilier amélioré", 1);
    original.recordCanonicalEvent("clan_hq_festivals", "headquarters_party", "Fête du QG de test", 1);
    original.recordCanonicalEvent("clan_hq_festive_morale", "Aline", "Motivation de test", 1);
    original.recordCanonicalEvent("clan_hq_forge_workshop_upgrades", "level_1", "Atelier de forge du clan niveau 1", 1);
    original.recordCanonicalEvent("clan_hq_blacksmith_hires", "active", "name=Ronan Testforge | role=forgeron_clan | hired_day=2 | starting_skill=1", 1);
    original.recordCanonicalEvent("clan_hq_artisans_joined", "Ronan Testforge", "Ronan Testforge rejoint le QG comme forgeron du clan", 1);
    original.recordCanonicalEvent("clan_hq_blacksmith_xp", "Ronan Testforge", "Expérience persistante de test", 180);
    original.recordCanonicalEvent("clan_hq_blacksmith_jobs", "Ronan Testforge", "Commandes persistantes de test", 7);
    original.recordCanonicalEvent("clan_hq_blacksmith_specializations", "Ronan Testforge", "specialization=maintenance | chosen_day=5", 1);
    original.recordCanonicalEvent("clan_hq_upgrade_jobs_started", "active", "kind=extra_bed | target=4 | started_day=2 | ready_day=4 | city=valebrume", 1);

    const std::string account = "roundtrip_test_account";
    assert(SaveManager::saveAccountSnapshot(account));
    assert(SaveManager::savePlayerSnapshot(original, account, DifficultyMode::Normal, DeathRuleMode::NonDefinitive));

    const std::vector<CharacterSaveSummary> summaries = SaveManager::listPlayableCharacters(account);
    assert(!summaries.empty());
    assert(summaries.front().level == 37);
    assert(summaries.front().realPlayTimeSeconds >= 3661);
    assert(!summaries.front().lastSavedAt.empty());
    const std::vector<AccountSaveSummary> accountSummaries = SaveManager::listAccounts();
    bool foundAccountPlayTime = false;
    for (const AccountSaveSummary& accountSummary : accountSummaries)
    {
        if (accountSummary.accountName == account)
        {
            assert(accountSummary.totalRealPlayTimeSeconds >= 3661);
            foundAccountPlayTime = true;
        }
    }
    assert(foundAccountPlayTime);

    Player loaded;
    // Simulate reloading into an already-used Player: old tracking preferences must not
    // make restored possessions look like freshly acquired loot.
    loaded.getInventory().setTrackedMaterialId("runic_iron_shard", true);
    std::ostringstream loadOutput;
    std::streambuf* previousOutput = std::cout.rdbuf(loadOutput.rdbuf());
    DifficultyMode difficulty = DifficultyMode::Easy;
    DeathRuleMode deathRule = DeathRuleMode::NonDefinitive;
    const bool loadedOk = SaveManager::loadPlayerSnapshot(summaries.front(), loaded, difficulty, deathRule);
    std::cout.rdbuf(previousOutput);
    assert(loadedOk);
    assert(loadOutput.str().find("OBJET SUIVI OBTENU") == std::string::npos);


    assert(loaded.getHomeState().owned);
    assert(loaded.getHomeState().cityId == "valebrume");
    assert(loaded.getHomeState().rentAmount == 280);
    assert(loaded.getHomeState().npcSpouseName == "Partenaire test");
    assert(loaded.getHestiaStoryState().rescued);
    assert(loaded.getHestiaStoryState().residence == "player_home");
    assert(loaded.getHestiaStoryState().trust == 64);
    assert(loaded.getHestiaStoryState().awakeningStage == 1);
    assert(loaded.getHestiaArtifactState().granted);
    assert(loaded.getHestiaArtifactState().level == 16);
    assert(loaded.getHestiaArtifactState().killsTowardNextLevel == 7);
    assert(loaded.getHestiaArtifactState().primaryForm == "faux");
    assert(loaded.getHestiaArtifactState().secondaryForm == "arbalète");
    assert(loaded.getHestiaArtifactState().activeForm == "secondary");
    assert(loaded.getHestiaArtifactState().absorbedEssences.size() == 2);
    assert(loaded.getHestiaArtifactState().absorbedEssences.front().familyId == "gobelin");
    assert(loaded.getHestiaArtifactState().absorbedEssences.front().acceptedKills == 5);
    assert(HestiaSoulArtifactSystem::isEquipped(loaded));
    const HestiaCompanionProfile loadedHestiaProfile = HestiaCompanionSystem::profile(loaded);
    assert(loadedHestiaProfile.className == "Mage");
    assert(loadedHestiaProfile.weaponStyle == "Bâton / catalyseur");
    assert(loadedHestiaProfile.practiceSessions >= 6);
    const ClanHeadquartersStatus loadedClanHq = ClanHeadquartersSystem::statusFor(loaded);
    assert(loadedClanHq.built);
    assert(loadedClanHq.buildingLevel == 2);
    assert(loadedClanHq.bedCapacity == 3);
    assert(loadedClanHq.maxBedCapacity == ClanHeadquartersSystem::BuildingLevel2MaxBeds);
    assert(loadedClanHq.questBoardLevel == 2);
    assert(loadedClanHq.missionChestLevel == 1);
    assert(loadedClanHq.commonRoomBarLevel == 1);
    assert(loadedClanHq.comfortDecorLevel == 1);
    assert(loadedClanHq.forgeWorkshopLevel == 1);
    assert(loadedClanHq.missionChestStoredCopper == 1125);
    assert(loadedClanHq.missionChestCapacityCopper == ClanHeadquartersSystem::MissionChestLevel1CapacityCopper);
    assert(loadedClanHq.modulesUsed == 6);
    const ClanBlacksmithStatus loadedBlacksmith = ClanBlacksmithSystem::statusFor(loaded);
    assert(loadedBlacksmith.hired);
    assert(loadedBlacksmith.name == "Ronan Testforge");
    assert(loadedBlacksmith.experience == 180);
    assert(loadedBlacksmith.jobsCompleted == 7);
    assert(loadedBlacksmith.skillLevel == 3);
    assert(loadedBlacksmith.specialization == ClanBlacksmithSpecialization::Maintenance);
    assert(loadedBlacksmith.friendDiscountPercent == 4);
    const ClanHeadquartersUpgradeStatus loadedClanUpgrade = ClanHeadquartersSystem::upgradeStatusFor(loaded);
    assert(loadedClanUpgrade.active);
    assert(loadedClanUpgrade.kind == ClanHeadquartersUpgradeKind::ExtraBed);
    assert(loadedClanUpgrade.targetLevel == 4);

    assert(loaded.getLevel() == 37);
    assert(loaded.getExperience() == 4242);
    assert(loaded.getInventory().getTotalCopper() == originalCopperTotal);
    const CoinBreakdown loadedCoins = loaded.getInventory().getCoinStacks();
    assert(loadedCoins.platinum == 2);
    assert(loadedCoins.gold == 51);
    assert(loadedCoins.electrum == 7);
    assert(loadedCoins.iron == 13);
    assert(loadedCoins.copper == 4);
    assert(loaded.getRealPlayTimeSeconds() >= 3661);
    assert(loaded.getBackpackTier() == 3);
    assert(loaded.getBodyMorphology() == "Standard");
    assert(loaded.hasActiveMorphologyProgram());
    assert(loaded.getMorphologyProgramTarget() == "Robuste");
    assert(loaded.getMorphologyProgramSessions() == 1);
    assert(loaded.getBackpackBaseCapacity() == 70);
    assert(loaded.hasCityVault());
    assert(loaded.getCityVaultLevel() == 5);
    assert(loaded.hasCityVaultPlaceOptimization());
    assert(loaded.hasCityVaultPreparedStorage());
    assert(loaded.getCityVaultStorageReductionPercent() == 25);
    assert(loaded.getCityVaultSecurityDaysRemainingForCity("valebrume") == 11);
    assert(loaded.hasCityVaultRaidPendingForCity("valebrume"));
    assert(loaded.getCityVaultLastRecallMessengerSourceCityId() == "port_lanterne");
    assert(loaded.getCityVaultLastRecallMessengerDestinationCityId() == "valebrume");
    assert(loaded.getCityVaultLastRecallMessengerStamp() == original.getCityVaultLastRecallMessengerStamp());
    assert(loaded.getCityVaultRaidDeadlineDayForCity("valebrume") == 42);
    assert(loaded.getVaultTheftCases().size() == 1);
    const PlayerVaultTheftCase& loadedTheftCase = loaded.getVaultTheftCases().front();
    assert(loadedTheftCase.caseId == "vault_theft:valebrume:test");
    assert(!loadedTheftCase.culpritIdentified);
    assert(loadedTheftCase.clues == 2);
    assert(loadedTheftCase.holderWorldContactId == "valebrume_receleur_brocanteur");
    assert(loadedTheftCase.holderLocationId == "market");
    assert(loadedTheftCase.holderLocationLabel == "Marché local de Valebrume");
    assert(loadedTheftCase.holderExplorationBiome == "Route commerciale");
    assert(loadedTheftCase.holderObservationCount == 2);
    assert(loadedTheftCase.negotiationAttempts == 1);
    assert(loadedTheftCase.confrontationAttempts == 2);
    assert(loadedTheftCase.holderPressure == 65);
    assert(loadedTheftCase.holderCooperation == -15);
    assert(loadedTheftCase.holderClosedUntilDay == 17);
    assert(loadedTheftCase.holderReportedToGuard);
    assert(loadedTheftCase.lastHolderInteractionDay == 8);
    assert(loadedTheftCase.holderAlerted);

    // V3.51.61: battlefield recovery keeps the exact object and its recovery metadata.
    assert(loaded.getBattlefieldRecoveryCaches().size() == 1);
    const PlayerBattlefieldRecoveryCache& loadedRecoveryCache = loaded.getBattlefieldRecoveryCaches().front();
    assert(loadedRecoveryCache.cacheId == battlefieldCacheId);
    assert(loadedRecoveryCache.cityId == "valebrume");
    assert(loadedRecoveryCache.contextId == "roundtrip_postko");
    assert(loadedRecoveryCache.locationLabel == "Ruines du vieux pont");
    assert(!loadedRecoveryCache.resolved);
    assert(loadedRecoveryCache.custodyState == "civilian");
    assert(loadedRecoveryCache.custodyLabel == "lot recueilli par un témoin");
    assert(loadedRecoveryCache.movementCount == 2);
    assert(loadedRecoveryCache.locationKnown);
    assert(loadedRecoveryCache.inventory.getWeapons().size() == 1);
    const Weapon& loadedBattlefieldWeapon = loadedRecoveryCache.inventory.getWeapons().front();
    assert(loadedBattlefieldWeapon.getPersistentId() == "roundtrip_battlefield_weapon_001");
    assert(loadedBattlefieldWeapon.getDurability() == battlefieldWeapon.getDurability());
    assert(loadedBattlefieldWeapon.getEnchantments() == battlefieldWeapon.getEnchantments());
    assert(loadedBattlefieldWeapon.isPersonalRecoveryItem());
    assert(loadedBattlefieldWeapon.isUniqueRecoveryItem());
    assert(loadedBattlefieldWeapon.getRecoveryPriority() == 99);

    assert(loaded.getCrimeContactStates().size() == 1);
    const PlayerCrimeContactState* loadedContact = loaded.findCrimeContactState("valebrume_receleur_brocanteur");
    assert(loadedContact != nullptr);
    assert(loadedContact->displayName == "Serrin le brocanteur");
    assert(loadedContact->encounters == 4);
    assert(loadedContact->observations == 2);
    assert(loadedContact->negotiations == 1);
    assert(loadedContact->guardConfrontations == 1);
    assert(loadedContact->pressure == 72);
    assert(loadedContact->cooperation == 18);
    assert(loadedContact->reportedToGuard);
    assert(loadedContact->detainedUntilDay == original.getWorldDaysElapsed() + 4);
    assert(loadedContact->provisionalDetentions == 2);
    assert(loadedContact->releases == 1);
    assert(loadedContact->returnsAfterFlight == 3);
    assert(loadedContact->knownRole == "receleur");
    assert(loadedContact->homeCityId == "valebrume");
    assert(loadedContact->networkId == "marche_gris");
    assert(loadedContact->networkWarnings == 3);
    assert(loadedContact->lastNetworkWarningDay == original.getWorldDaysElapsed());
    assert(loadedContact->lastWarningSourceId == "rocheveille_receleur_peseur");
    assert(!loadedContact->arrested);
    assert(loadedContact->reliability == 67);
    assert(loadedContact->favorBalance == -24);
    assert(loadedContact->rivalry == 38);
    assert(loadedContact->fulfilledDeals == 5);
    assert(loadedContact->brokenDeals == 2);
    assert(loadedContact->ruptureUntilDay == original.getWorldDaysElapsed() + 6);
    assert(loadedTheftCase.stolenCopper == 321);
    assert(loadedTheftCase.hasWeapon);
    assert(loadedTheftCase.stolenWeapon.getPersistentId() == "vault_theft_weapon_exact_001");
    assert(loadedTheftCase.stolenWeapon.isPersonalRecoveryItem());
    assert(loadedTheftCase.stolenWeapon.getRecoveryPriority() == 96);
    assert(loadedTheftCase.stolenMaterialId == "runic_iron_shard");
    assert(loadedTheftCase.stolenMaterialQuantity == 3);
    const CoinBreakdown loadedVaultCoins = loaded.getCityVault().getCoinStacks();
    assert(loadedVaultCoins.gold == 12);
    assert(loadedVaultCoins.copper == 345);

    bool foundSoulBoundWeapon = false;
    for (const Weapon& weapon : loaded.getInventory().getWeapons())
    {
        if (weapon.getPersistentId() == "roundtrip_soulbound_weapon_001")
        {
            foundSoulBoundWeapon = true;
            assert(weapon.isSoulBound());
            assert(!weapon.canChangeOwner());
        }
    }
    assert(foundSoulBoundWeapon);

    bool foundQuestRecoveryWeapon = false;
    for (const Weapon& weapon : loaded.getInventory().getWeapons())
    {
        if (weapon.getPersistentId() != "roundtrip_recovery_weapon_001") continue;
        foundQuestRecoveryWeapon = true;
        assert(!weapon.isSoulBound());
        assert(weapon.canChangeOwner());
        assert(weapon.isQuestRecoveryItem());
        assert(weapon.isUniqueRecoveryItem());
        assert(weapon.isProtectedFromAutomaticAbandon());
        assert(weapon.getRecoveryPriority() == 88);
        assert(weapon.getAutomaticRecoveryPriorityScore() == 100);
    }
    assert(foundQuestRecoveryWeapon);

    int boundConsumables = 0;
    for (const Consumable& consumable : loaded.getInventory().getConsumables())
    {
        if (consumable.isSoulBound()) ++boundConsumables;
    }
    assert(boundConsumables >= 1);

    int boundArcaneDust = 0;
    int freeArcaneDust = 0;
    for (const Material& material : loaded.getInventory().getMaterials())
    {
        if (material.getId() != "arcane_dust" || material.getQuality() != "normal") continue;
        if (material.isSoulBound()) boundArcaneDust += material.getQuantity();
        else freeArcaneDust += material.getQuantity();
    }
    assert(boundArcaneDust == 2);
    assert(freeArcaneDust >= 3);

    int personalRecoveryOre = 0;
    int ordinaryRecoveryOre = 0;
    int recoveryOreStacks = 0;
    for (const Material& material : loaded.getInventory().getMaterials())
    {
        if (material.getId() != "recovery_roundtrip_ore") continue;
        ++recoveryOreStacks;
        if (material.isPersonalRecoveryItem())
        {
            personalRecoveryOre += material.getQuantity();
            assert(material.getRecoveryPriority() == 95);
        }
        else ordinaryRecoveryOre += material.getQuantity();
    }
    assert(recoveryOreStacks == 2);
    assert(personalRecoveryOre == 2);
    assert(ordinaryRecoveryOre == 3);

    bool foundBoundVaultWeapon = false;
    for (const Weapon& weapon : loaded.getCityVault().getWeapons())
    {
        if (weapon.getPersistentId() == "roundtrip_vault_soulbound_weapon_001")
        {
            foundBoundVaultWeapon = true;
            assert(weapon.isSoulBound());
        }
    }
    assert(foundBoundVaultWeapon);
    assert(loaded.getMana() == originalMana);
    assert(loaded.getManaExhaustionTurns() == 1);
    assert(loaded.getLanguageKnowledgeLevel("commun") >= 3);
    assert(loaded.getLanguageKnowledgeLevel("infernal") >= 3);
    assert(loaded.getLanguageKnowledgeLevel("gobelin") == 2);
    assert(loaded.getLanguageStudyProgress("gobelin") == 43);
    assert(loaded.getAmmunitionSelectionPolicy() == "favorite");
    assert(loaded.getMagicMaterialSelectionPolicy() == "auto");
    assert(loaded.getLootPickupPolicy() == "filter");
    assert(loaded.isLootCategoryIgnored("armor"));
    assert(loaded.isLootMaterialIgnored("slime_residue"));
    assert(loaded.getInventory().isPinnedItemKey("material:arcane_dust"));
    assert(loaded.getInventory().isFavoriteAmmunitionId("training_arrows"));
    assert(loaded.getInventory().isLockedItemKey("consumable:Potion de soin"));
    assert(loaded.getInventory().isTrackedMaterialId("runic_iron_shard"));
    assert(loaded.getNpcKnownFacts().size() == 1);
    assert(loaded.getNpcKnownFacts().front().npcId == "Archiviste Meron");
    assert(loaded.getNpcKnownFacts().front().confidence == 74);
    assert(loaded.getNpcKnownFacts().front().claimVariant == "stela_elfique");
    assert(loaded.getNpcKnownFacts().front().relayChannel == "bibliotheque");

    const auto& rivals = loaded.getRivalRecords();
    assert(rivals.size() == 1);
    assert(rivals.front().enemyName == "Hobgobelin balafré");
    assert(rivals.front().temperament == "calculateur");
    assert(rivals.front().escapes == 1);
    assert(rivals.front().returns == 1);
    assert(rivals.front().wounds == 2);
    assert(rivals.front().emergenceScore == 47);
    assert(rivals.front().stolenCopper == 11);
    assert(rivals.front().stolenMaterialId == "runic_iron_shard");
    assert(rivals.front().stolenMaterialQuantity == 2);
    assert(rivals.front().stolenMaterialQuality == "high");
    assert(rivals.front().visibleMark.find("cicatrice") != std::string::npos || rivals.front().visibleMark.find("silhouette") != std::string::npos);
    assert(loaded.getCanonicalJournalCategoryTotal("techniques_combinees_alliees") >= 7);
    bool foundRememberedWeapon = false;
    for (const Weapon& weapon : loaded.getInventory().getWeapons())
    {
        if (weapon.getPersistentId() != "roundtrip_weapon_memory_001") continue;
        foundRememberedWeapon = true;
        assert(weapon.getName() == "Épée témoin");
        const std::vector<PlayerHistoricalEvent> itemMemories = loaded.getHistoricalEventsForSubject(weapon.getPersistentId(), 10);
        assert(itemMemories.size() == 1);
        assert(itemMemories.front().category == "item_memory_enemy_signature");
    }
    assert(foundRememberedWeapon);

    bool foundQuest = false;
    for (const Quest& loadedQuest : loaded.getQuestLog().getQuests())
    {
        if (loadedQuest.id != quest.id) continue;
        assert(loadedQuest.requiredLanguage == "gobelin");
        assert(loadedQuest.requiredLanguageLevel == 2);
        assert(loadedQuest.sourceLanguageText == quest.sourceLanguageText);
        assert(loadedQuest.rewardCoins.copper == 17);
        assert(loadedQuest.rewardCoins.iron == 2);
        assert(loadedQuest.rewardCoins.electrum == 0);
        assert(loadedQuest.rewardCoins.gold == 3);
        assert(loadedQuest.rewardCoins.platinum == 0);
        assert(loadedQuest.phaseTwoChecked);
        assert(loadedQuest.phaseTwoActive);
        assert(loadedQuest.phaseTwoMandatory);
        assert(loadedQuest.phaseTwoTitle == "Suite de test");
        assert(loadedQuest.phaseTwoPreviousSummary == "Phase 1 terminée : message compris.");
        assert(loadedQuest.phaseTwoReason == "Un témoin relie le message à un second relais.");
        assert(loadedQuest.phaseTwoObjective == "Vérifier le relais secondaire.");
        assert(loadedQuest.phaseTwoProgress == 1);
        assert(loadedQuest.phaseTwoTarget == 2);
        assert(loadedQuest.phaseTwoAttempts == 3);
        assert(loadedQuest.phaseTwoRewardExperience == 19);
        assert(loadedQuest.phaseTwoRewardGold == 7);
        assert(loadedQuest.phaseTwoRewardKind == "information");
        assert(loadedQuest.phaseTwoRewardDetail == "Information recoupée auprès du client.");
        assert(loadedQuest.phaseTwoTravelRequired);
        assert(loadedQuest.phaseTwoTravelSourceCityId == "valebrume");
        assert(loadedQuest.phaseTwoTravelDestinationCityId == "rocheveille");
        assert(loadedQuest.phaseTwoTravelDistanceKm == 35);
        assert(loadedQuest.intercityMission);
        assert(loadedQuest.intercityKind == "information");
        assert(loadedQuest.intercitySourceCityId == "valebrume");
        assert(loadedQuest.intercityRouteCityIds == "rocheveille|port_lanterne");
        assert(loadedQuest.intercityPayloadName == "dossier de test scellé");
        assert(loadedQuest.intercityStopIndex == 1);
        assert(loadedQuest.intercityTotalDistanceKm == 88);
        assert(loadedQuest.intercityTravelRewardGold == 23);
        assert(loadedQuest.intercityTravelRewardExperience == 17);
        foundQuest = true;
    }
    assert(foundQuest);

    bool foundBountyQuest = false;
    for (const Quest& loadedQuest : loaded.getQuestLog().getQuests())
    {
        if (loadedQuest.id != bountyQuest.id) continue;
        assert(loadedQuest.objectiveType == "bounty");
        assert(loadedQuest.bountyMission);
        assert(loadedQuest.bountyTargetName == bountyQuest.bountyTargetName);
        assert(loadedQuest.bountyTargetTemperament == bountyQuest.bountyTargetTemperament);
        assert(loadedQuest.bountyTargetId == bountyQuest.bountyTargetId);
        assert(loadedQuest.bountyTargetHomeCityId == bountyQuest.bountyTargetHomeCityId);
        assert(loadedQuest.bountyTargetNetworkId == bountyQuest.bountyTargetNetworkId);
        assert(loadedQuest.bountyIssuerId == bountyQuest.bountyIssuerId);
        assert(loadedQuest.bountyIssuerName == bountyQuest.bountyIssuerName);
        assert(loadedQuest.bountyIssuerOrganizationId == bountyQuest.bountyIssuerOrganizationId);
        assert(loadedQuest.bountyIssuerOrganizationName == bountyQuest.bountyIssuerOrganizationName);
        assert(loadedQuest.bountyIssuerPrefersLiveCapture);
        assert(loadedQuest.bountyCloseRelationId == bountyQuest.bountyCloseRelationId);
        assert(loadedQuest.bountyCloseRelationName == bountyQuest.bountyCloseRelationName);
        assert(loadedQuest.bountyCloseRelationType == bountyQuest.bountyCloseRelationType);
        assert(loadedQuest.bountyStage == "preparation");
        assert(loadedQuest.bountyClues == 4);
        assert(loadedQuest.bountyInvestigationNoise == 31);
        assert(loadedQuest.bountyInformantExposure == 18);
        assert(loadedQuest.bountyTargetAwareness == 72);
        assert(loadedQuest.bountyTargetSuspectsHunt);
        assert(!loadedQuest.bountyTargetKnowsHunt);
        assert(loadedQuest.bountyTargetResources == 63);
        assert(loadedQuest.bountyTargetLocalSupport == 44);
        assert(loadedQuest.bountyTargetMobility == 77);
        assert(loadedQuest.bountyTargetEscapeAttempts == 1);
        assert(loadedQuest.bountyTargetKnowsTerrain);
        assert(!loadedQuest.bountyTargetInjured);
        assert(loadedQuest.bountyExitRecon == 2);
        assert(loadedQuest.bountyInformantNetwork == 1);
        assert(loadedQuest.bountyAmbushPreparation == 2);
        assert(loadedQuest.bountyShadowingPreparation == 1);
        assert(loadedQuest.bountyRouteVariation == 1);
        assert(loadedQuest.bountyDecoyQuality == 0);
        assert(loadedQuest.bountyCounterSurveillance == 1);
        assert(loadedQuest.bountyRefugeConcealment == 0);
        assert(loadedQuest.bountyDangerRating == 4);
        assert(loadedQuest.bountyNotorietyRating == 3);
        assert(loadedQuest.bountyDaysAtLarge == 117);
        assert(loadedQuest.bountyBaseRewardGold == 240);
        assert(loadedQuest.bountyRewardProfileApplied);
        assert(loadedQuest.bountyRegionalProfileApplied);
        assert(loadedQuest.bountyCurrentCityId == "port_lanterne");
        assert(loadedQuest.bountyCurrentBiome == "Marché sous les ponts");
        assert(loadedQuest.bountyVisitedCityIds == "valebrume|port_lanterne");
        assert(loadedQuest.bountyRelocationCount == 1);
        assert(loadedQuest.bountyTransitTrailKind == "coach_registry");
        assert(loadedQuest.bountyTransitTrailFromCityId == "valebrume");
        assert(loadedQuest.bountyTransitTrailReliability == 81);
        assert(loadedQuest.bountyTransitTrailAvailable);
        assert(loadedQuest.bountyRegionalProfileCityId == "port_lanterne");
        assert(loadedQuest.bountyRegionalConsequencesApplied);
        assert(loadedQuest.bountyResolution == "captured_combat");
        foundBountyQuest = true;
    }
    assert(foundBountyQuest);

    bool foundIncidentQuest = false;
    for (const Quest& loadedQuest : loaded.getQuestLog().getQuests())
    {
        if (loadedQuest.id != incidentQuest.id) continue;
        assert(loadedQuest.tradeIncidentMission);
        assert(loadedQuest.tradeIncidentId == "incident:test:001");
        assert(loadedQuest.tradeIncidentKind == "cargaison partiellement volée");
        assert(loadedQuest.tradeIncidentSourceCityId == "rocheveille");
        assert(loadedQuest.tradeIncidentDestinationCityId == "port_lanterne");
        assert(loadedQuest.tradeIncidentGoodsType == 4);
        assert(loadedQuest.tradeIncidentSeverity == 3);
        assert(loadedQuest.tradeIncidentStage == "action");
        assert(loadedQuest.tradeIncidentAttempts == 2);
        assert(loadedQuest.tradeIncidentCreatesBountyLead);
        assert(!loadedQuest.tradeIncidentBountyPublished);
        foundIncidentQuest = true;
    }
    assert(foundIncidentQuest);

    // Simulate a V3.49.93-style save with neither NPC memory nor languageState sections.
    {
        std::ifstream input(summaries.front().path);
        std::ostringstream buffer;
        buffer << input.rdbuf();
        std::string legacy = buffer.str();

        const auto eraseSectionBefore = [&](const std::string& marker, const std::string& nextMarker)
        {
            const std::size_t sectionStart = legacy.find(marker);
            if (sectionStart == std::string::npos) return;
            const std::size_t sectionEnd = legacy.find(nextMarker, sectionStart);
            assert(sectionEnd != std::string::npos);
            legacy.erase(sectionStart, sectionEnd - sectionStart);
        };
        eraseSectionBefore("  \"selectionPreferences\": {", "  \"inventoryPreferences\": {");
        eraseSectionBefore("  \"inventoryPreferences\": {", "  \"npcMemoryState\": {");

        const std::size_t start = legacy.find("  \"npcMemoryState\": {");
        const std::size_t next = legacy.find("  \"cheatState\": {", start);
        assert(start != std::string::npos && next != std::string::npos);
        legacy.erase(start, next - start);
        const std::string currentVersion = "\"saveVersion\": " + std::to_string(SaveSchemaVersion::Current);
        const std::size_t versionPos = legacy.find(currentVersion);
        assert(versionPos != std::string::npos);
        legacy.replace(versionPos, currentVersion.size(), "\"saveVersion\": 20");
        std::ofstream output(summaries.front().path, std::ios::trunc);
        output << legacy;
    }

    Player migrated;
    difficulty = DifficultyMode::Easy;
    deathRule = DeathRuleMode::NonDefinitive;
    assert(SaveManager::loadPlayerSnapshot(summaries.front(), migrated, difficulty, deathRule));
    assert(migrated.getLanguageKnowledgeLevel("commun") >= 3);
    assert(migrated.getLanguageKnowledgeLevel("infernal") >= 3);
    assert(migrated.getLanguageKnowledgeLevel("gobelin") == 0);
    assert(migrated.getAmmunitionSelectionPolicy() == "ask");
    assert(migrated.getMagicMaterialSelectionPolicy() == "ask");
    assert(!migrated.getInventory().isPinnedItemKey("material:arcane_dust"));
    assert(!migrated.getInventory().isFavoriteAmmunitionId("training_arrows"));
    assert(!migrated.getInventory().isLockedItemKey("consumable:Potion de soin"));
    assert(!migrated.getInventory().isTrackedMaterialId("runic_iron_shard"));
    assert(migrated.getNpcKnownFacts().empty());

    fs::remove_all("assets/saves");
    return 0;
}
