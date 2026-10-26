#include <cassert>
#include <string>

#include "class_system/ClassCatalog.hpp"
#include "core/Random.hpp"
#include "economy/EconomyBalance.hpp"
#include "entity/Player.hpp"
#include "item/material/Material.hpp"
#include "item/consumable/Consumable.hpp"
#include "item/weapon/Weapon.hpp"
#include "world/crime/VaultTheftNetworkCatalog.hpp"

int main()
{
    // Mayor discount is only for the first vault and requires real local history.
    Player trusted("Citoyen", ClassCatalog::createClassByName("Guerrier"));
    trusted.getInventory().clearAll();
    trusted.getInventory().earnEconomyUnits(100000);
    assert(trusted.getOwnedCityVaultCount() == 0);
    assert(trusted.getCityVaultPurchaseCost() == 2000);

    trusted.recordCanonicalEvent("reputation_locale_positive", "aide_mairie", "Bonne réputation", 45);
    trusted.recordCanonicalEvent("quetes_reussies", "missions_maire", "Missions locales", 15);
    assert(trusted.getCurrentCityVaultPurchaseDiscountTier() == 1);
    assert(trusted.getCityVaultPurchaseCost() == 1500);

    trusted.recordCanonicalEvent("reputation_locale_positive", "heroisme_local", "Réputation exceptionnelle", 40);
    assert(trusted.getCurrentCityVaultPurchaseDiscountTier() == 2);
    assert(trusted.getCityVaultPurchaseCost() == 1250);
    assert(trusted.purchaseCityVault());
    assert(trusted.getOwnedCityVaultCount() == 1);

    // Additional vault prices use cumulative +2 PO, +3 PO, +4 PO...
    trusted.setCurrentCityId("port_lanterne");
    assert(trusted.getCityVaultPurchaseCost() == 2200);
    assert(EconomyBalance::cityVaultPurchaseCost("valebrume", 2, 0) == 2500);
    assert(EconomyBalance::cityVaultPurchaseCost("valebrume", 3, 0) == 2900);
    assert(trusted.purchaseCityVault());
    assert(trusted.getOwnedCityVaultCount() == 2);

    // Reputation alone cannot grant a mayoral rebate before the character has reached one PP of liquid wealth.
    Player poorTrusted("Citoyen pauvre", ClassCatalog::createClassByName("Guerrier"));
    poorTrusted.getInventory().clearAll();
    poorTrusted.getInventory().earnEconomyUnits(999);
    poorTrusted.recordCanonicalEvent("reputation_locale_positive", "aide_mairie", "Bonne réputation", 90);
    poorTrusted.recordCanonicalEvent("quetes_reussies", "missions_maire", "Missions locales", 20);
    assert(poorTrusted.getCurrentCityVaultPurchaseDiscountTier() == 0);
    assert(poorTrusted.getCityVaultPurchaseCost() == 2000);

    // Four quests from an important local character can replace the 15 ordinary services.
    Player notableTrusted("Protégé du maire", ClassCatalog::createClassByName("Guerrier"));
    notableTrusted.getInventory().clearAll();
    notableTrusted.getInventory().earnEconomyUnits(1000);
    notableTrusted.recordCanonicalEvent("reputation_locale_positive", "aide_mairie", "Bonne réputation", 50);
    notableTrusted.recordCanonicalEvent("quetes_personnage_important", "Bram", "Services de Bram", 4);
    assert(notableTrusted.getCurrentCityVaultPurchaseDiscountTier() == 1);

    // Security is local, cheap relative to purchase, and pauses while the player stays in that city.
    assert(trusted.getCityVaultSecuritySubscriptionCost() == 30);
    assert(trusted.buyCityVaultSecuritySubscription());
    assert(trusted.getCityVaultSecurityDaysRemainingForCity("port_lanterne") == 7);
    trusted.advanceWorldDays(3);
    assert(trusted.getCityVaultSecurityDaysRemainingForCity("port_lanterne") == 7);

    // Once the player is based elsewhere, protected days are consumed.
    trusted.setCurrentCityId("valebrume");
    trusted.advanceWorldDays(1);
    assert(trusted.getCityVaultSecurityDaysRemainingForCity("port_lanterne") == 6);
    assert(!trusted.triggerCityVaultRaidAlertForCity("port_lanterne", 7)); // covered vault cannot be targeted

    // An uncovered remote vault can receive the deterministic/manual alert used by gameplay/tests.
    trusted.setCurrentCityId("port_lanterne");
    // Let coverage expire while based elsewhere.
    trusted.setCurrentCityId("valebrume");
    trusted.advanceWorldDays(6);
    assert(trusted.getCityVaultSecurityDaysRemainingForCity("port_lanterne") == 0);
    assert(trusted.triggerCityVaultRaidAlertForCity("port_lanterne", 7));
    assert(trusted.hasCityVaultRaidPendingForCity("port_lanterne"));
    assert(trusted.getCityVaultRaidDeadlineDayForCity("port_lanterne") >= trusted.getWorldDaysElapsed() + 7);

    // A failed defense preferentially loses part of a large common stack rather than rare/important goods.
    trusted.setCurrentCityId("port_lanterne");
    trusted.getCityVault().addMaterial(Material("bulk_common_ore", "Minerai commun en vrac", "Test", "minerai", 2, 50));
    Material boundRaidMaterial("bound_raid_ore", "Minerai lié", "Pile liée qui ne peut pas changer de propriétaire.", "minerai", 2, 25);
    boundRaidMaterial.setSoulBound();
    trusted.getCityVault().addMaterial(boundRaidMaterial);
    // Persist the direct vault mutations into the per-city record before resolveCurrentCityVaultRaid reloads it.
    trusted.setCurrentCityId("valebrume");
    trusted.setCurrentCityId("port_lanterne");
    const int commonBeforeRaid = trusted.getCityVault().countMaterialById("bulk_common_ore");
    const int boundBeforeRaid = trusted.getCityVault().countMaterialById("bound_raid_ore");
    assert(trusted.resolveCurrentCityVaultRaid(false));
    assert(trusted.getCityVault().countMaterialById("bulk_common_ore") < commonBeforeRaid);
    assert(trusted.getCityVault().countMaterialById("bound_raid_ore") == boundBeforeRaid);

    // Inter-city transfers work in both directions and never duplicate a pile.
    trusted.setCurrentCityId("valebrume");
    trusted.getCityVault().addMaterial(Material("vault_route_ore", "Minerai logistique", "Test", "minerai", 1, 10));
    // Sync current vault through a harmless city round-trip.
    trusted.setCurrentCityId("port_lanterne");
    trusted.setCurrentCityId("valebrume");
    const int walletBefore = static_cast<int>(trusted.getInventory().getTotalCopper());
    const int routeMaterialIndex = trusted.getCityVaultForCity("valebrume").findMaterialIndexById("vault_route_ore");
    assert(routeMaterialIndex >= 0);
    assert(trusted.transferMaterialBetweenOwnedCityVaults("valebrume", "port_lanterne", routeMaterialIndex, 10, 10));
    assert(trusted.getCityVaultForCity("valebrume").countMaterialById("vault_route_ore") == 0);
    assert(trusted.getCityVaultForCity("port_lanterne").countMaterialById("vault_route_ore") == 10);
    const int remoteRouteMaterialIndex = trusted.getCityVaultForCity("port_lanterne").findMaterialIndexById("vault_route_ore");
    assert(remoteRouteMaterialIndex >= 0);
    assert(trusted.transferMaterialBetweenOwnedCityVaults("port_lanterne", "valebrume", remoteRouteMaterialIndex, 10, 12));
    assert(trusted.getCityVaultForCity("port_lanterne").countMaterialById("vault_route_ore") == 0);
    assert(trusted.getCityVaultForCity("valebrume").countMaterialById("vault_route_ore") == 10);

    // Equipment transport preserves the exact persistent object identity in both directions.
    Weapon convoyWeapon(
        "Lame de convoi",
        "Arme de test du réseau municipal.",
        125,
        WeaponType::Sword,
        3,
        7,
        1,
        80
    );
    convoyWeapon.setPersistentId("vault_convoy_weapon_001");
    convoyWeapon.setSoulBound();
    trusted.getCityVault().addWeapon(convoyWeapon);
    trusted.setCurrentCityId("port_lanterne");
    trusted.setCurrentCityId("valebrume");
    const int weaponIndex = trusted.getCityVaultForCity("valebrume").getWeaponCount() - 1;
    assert(trusted.transferWeaponBetweenOwnedCityVaults("valebrume", "port_lanterne", weaponIndex, 13));
    assert(trusted.getCityVaultForCity("port_lanterne").getWeaponCount() >= 1);
    assert(trusted.getCityVaultForCity("port_lanterne").getWeapon(trusted.getCityVaultForCity("port_lanterne").getWeaponCount() - 1).getPersistentId() == "vault_convoy_weapon_001");
    assert(trusted.getCityVaultForCity("port_lanterne").getWeapon(trusted.getCityVaultForCity("port_lanterne").getWeaponCount() - 1).isSoulBound());
    const int remoteWeaponIndex = trusted.getCityVaultForCity("port_lanterne").getWeaponCount() - 1;
    assert(trusted.transferWeaponBetweenOwnedCityVaults("port_lanterne", "valebrume", remoteWeaponIndex, 14));
    assert(trusted.getCityVaultForCity("valebrume").getWeapon(trusted.getCityVaultForCity("valebrume").getWeaponCount() - 1).getPersistentId() == "vault_convoy_weapon_001");

    // Physical denominations can also be convoyed without conversion.
    CoinBreakdown convoyCoins;
    convoyCoins.platinum = 2;
    convoyCoins.gold = 7;
    convoyCoins.copper = 53;
    trusted.getCityVault().getCoinStacks();
    trusted.getCityVault().setCoinStacks(convoyCoins);
    trusted.setCurrentCityId("port_lanterne");
    trusted.setCurrentCityId("valebrume");
    assert(trusted.transferAllCoinsBetweenOwnedCityVaults("valebrume", "port_lanterne", 15));
    const CoinBreakdown remoteCoins = trusted.getCityVaultForCity("port_lanterne").getCoinStacks();
    assert(remoteCoins.platinum == 2);
    assert(remoteCoins.gold == 7);
    assert(remoteCoins.copper == 53);
    assert(trusted.getCityVaultForCity("valebrume").getTotalCopper() == 0);

    assert(trusted.getInventory().getTotalCopper() == walletBefore - 22 - 13 - 14 - 15);

    // V3.51.56: an ignored burglary cannot create stolen property from owner-bound goods.
    Player boundVaultOwner("Gardien lié", ClassCatalog::createClassByName("Guerrier"));
    boundVaultOwner.getInventory().clearAll();
    PlayerCityVault remoteBoundVault;
    remoteBoundVault.cityId = "port_lanterne";
    remoteBoundVault.purchased = true;
    remoteBoundVault.level = 1;
    remoteBoundVault.raidPending = true;
    remoteBoundVault.raidDeadlineDay = 0;
    remoteBoundVault.lastRaidCheckDay = -1;
    Weapon boundVaultWeapon("Lame inviolable", "Métadonnée liée à l'âme.", 500, WeaponType::Sword, 5, 8, 2, 100);
    boundVaultWeapon.setSoulBound();
    remoteBoundVault.inventory.addWeapon(boundVaultWeapon);
    Material boundVaultMaterial("bound_only_ore", "Minerai inviolable", "Métadonnée liée à l'âme.", "minerai", 5, 12);
    boundVaultMaterial.setSoulBound();
    remoteBoundVault.inventory.addMaterial(boundVaultMaterial);
    Consumable boundVaultConsumable("Potion inviolable", "Métadonnée liée à l'âme.", 40, ConsumableType::Healing, 10);
    boundVaultConsumable.setSoulBound();
    remoteBoundVault.inventory.addConsumable(boundVaultConsumable);
    Inventory emptyCurrentVault;
    boundVaultOwner.setLoadedCityState(false, 0, "valebrume", emptyCurrentVault, {}, {remoteBoundVault}, false, false);
    boundVaultOwner.advanceWorldDays(1);
    assert(boundVaultOwner.getVaultTheftCases().empty());
    const Inventory& protectedRemote = boundVaultOwner.getCityVaultForCity("port_lanterne");
    assert(protectedRemote.getWeaponCount() == 1);
    assert(protectedRemote.getWeapon(0).isSoulBound());
    assert(protectedRemote.countMaterialById("bound_only_ore") == 12);
    assert(protectedRemote.getConsumableCount() == 1);
    assert(protectedRemote.getConsumable(0).isSoulBound());

    // V3.51.54: an ignored remote burglary starts as an UNKNOWN case, not an omniscient rival.
    Player investigator("Enquêteur", ClassCatalog::createClassByName("Guerrier"));
    PlayerVaultTheftCase unknownCase;
    unknownCase.caseId = "vault_theft:valebrume:unit";
    unknownCase.cityId = "valebrume";
    unknownCase.createdDay = investigator.getWorldDaysElapsed();
    unknownCase.lastLeadDay = investigator.getWorldDaysElapsed();
    unknownCase.stolenCopper = 77;
    unknownCase.stolenMaterialId = "bulk_common_ore";
    unknownCase.stolenMaterialName = "Minerai commun en vrac";
    unknownCase.stolenMaterialQuality = "normal";
    unknownCase.stolenMaterialQuantity = 4;
    investigator.setLoadedVaultTheftCases({unknownCase});
    assert(investigator.hasOpenVaultTheftCaseForCity("valebrume"));
    assert(investigator.getRivalRecords().empty());
    investigator.investigateVaultTheftCase("valebrume");
    investigator.investigateVaultTheftCase("valebrume");
    investigator.investigateVaultTheftCase("valebrume");
    const PlayerVaultTheftCase* identifiedCase = investigator.getOpenVaultTheftCaseForCity("valebrume");
    assert(identifiedCase != nullptr);
    assert(identifiedCase->culpritIdentified);
    assert(!identifiedCase->identifiedRivalId.empty());
    assert(!investigator.getRivalRecords().empty());
    const long long beforeRecovery = investigator.getInventory().getTotalCopper();
    assert(investigator.recoverVaultTheftCaseProperty(identifiedCase->caseId));
    assert(investigator.getInventory().getTotalCopper() == beforeRecovery + 77);
    assert(investigator.getInventory().countMaterialById("bulk_common_ore") == 4);
    assert(!investigator.hasOpenVaultTheftCaseForCity("valebrume"));

    // V3.51.57: exploration can reveal a real field lead without inventing the thief's identity.
    Player fieldInvestigator("Pisteur", ClassCatalog::createClassByName("Guerrier"));
    PlayerVaultTheftCase fieldCase;
    fieldCase.caseId = "vault_theft:valebrume:field";
    fieldCase.cityId = "valebrume";
    fieldCase.createdDay = fieldInvestigator.getWorldDaysElapsed();
    fieldCase.lastLeadDay = fieldInvestigator.getWorldDaysElapsed();
    fieldCase.hasWeapon = true;
    fieldCase.stolenWeapon = Weapon("Lame à garde fendue", "Arme exacte reconnaissable sur le terrain.", 640, WeaponType::Sword, 8, 13, 4, 120);
    fieldCase.stolenWeapon.setPersistentId("vault_field_weapon_001");
    fieldInvestigator.setLoadedVaultTheftCases({fieldCase});

    const VaultTheftExplorationObservationResult weakLead = fieldInvestigator.observeVaultTheftCaseDuringExploration(
        "valebrume", "Route commerciale", VaultTheftExplorationObservation::SuspiciousTrail);
    assert(weakLead.occurred);
    assert(weakLead.cluesBefore == 0);
    assert(weakLead.cluesGained == 1);
    assert(weakLead.cluesAfter == 1);
    assert(!weakLead.recognizedStolenProperty);
    assert(!weakLead.targetIdentifiedNow);
    assert(fieldInvestigator.getRivalRecords().empty());

    const VaultTheftExplorationObservationResult strongLead = fieldInvestigator.observeVaultTheftCaseDuringExploration(
        "valebrume", "Route commerciale", VaultTheftExplorationObservation::RecognizedProperty);
    assert(strongLead.occurred);
    assert(strongLead.recognizedStolenProperty);
    assert(strongLead.cluesBefore == 1);
    assert(strongLead.cluesGained == 2);
    assert(strongLead.cluesAfter == 3);
    assert(strongLead.targetIdentifiedNow);
    assert(!strongLead.targetLabel.empty());
    assert(fieldInvestigator.getRivalRecords().size() == 1);
    const PlayerVaultTheftCase* fieldIdentified = fieldInvestigator.getOpenVaultTheftCaseForCity("valebrume");
    assert(fieldIdentified != nullptr && fieldIdentified->culpritIdentified);

    // A lead from the wrong city is ignored, and a located cache never fabricates a person to hunt.
    Player cacheInvestigator("Pisteur de cache", ClassCatalog::createClassByName("Guerrier"));
    PlayerVaultTheftCase cacheCase;
    cacheCase.caseId = "vault_theft:valebrume:cache";
    cacheCase.cityId = "valebrume";
    cacheCase.clues = 2;
    cacheCase.holderType = "cache";
    cacheCase.holderLabel = "cache secondaire";
    cacheCase.stolenCopper = 25;
    cacheInvestigator.setLoadedVaultTheftCases({cacheCase});
    const VaultTheftExplorationObservationResult wrongCityLead = cacheInvestigator.observeVaultTheftCaseDuringExploration(
        "port_lanterne", "Route commerciale", VaultTheftExplorationObservation::SuspiciousExchange);
    assert(!wrongCityLead.occurred);
    const VaultTheftExplorationObservationResult cacheLead = cacheInvestigator.observeVaultTheftCaseDuringExploration(
        "valebrume", "Route commerciale", VaultTheftExplorationObservation::SuspiciousExchange);
    assert(cacheLead.occurred);
    assert(cacheLead.cluesAfter == 3);
    assert(!cacheLead.targetIdentifiedNow);
    assert(cacheInvestigator.getRivalRecords().empty());
    const PlayerVaultTheftCase* stillCached = cacheInvestigator.getOpenVaultTheftCaseForCity("valebrume");
    assert(stillCached != nullptr && !stillCached->culpritIdentified && stillCached->holderType == "cache");
    assert(stillCached->holderWorldContactId == "valebrume_cache_sous_bois");
    assert(stillCached->holderLocationLabel == "Vieux sous-bois hors des axes");

    // V3.51.58: receivers/accomplices/caches are anchored to actual world contacts and places.
    const VaultTheftNetworkContact* portReceiver = VaultTheftNetworkCatalog::findById("port_lanterne_receleur_collecteur");
    assert(portReceiver != nullptr);
    assert(portReceiver->cityId == "port_lanterne");
    assert(portReceiver->roleType == "receiver");
    assert(portReceiver->locationLabel == "Marché sous les ponts");
    assert(!portReceiver->explorationBiome.empty());
    assert(VaultTheftNetworkCatalog::pickFor("rocheveille", "accomplice", 0) != nullptr);
    assert(VaultTheftNetworkCatalog::pickFor("lys_de_givre", "cache", 0) != nullptr);

    Player receiverInvestigator("Pisteur de receleur", ClassCatalog::createClassByName("Guerrier"));
    PlayerVaultTheftCase receiverCase;
    receiverCase.caseId = "vault_theft:port_lanterne:receiver";
    receiverCase.cityId = "port_lanterne";
    receiverCase.clues = 1;
    receiverCase.holderType = "receiver";
    receiverCase.hasWeapon = true;
    receiverCase.stolenWeapon = Weapon("Sabre du quai", "Arme exacte suivie jusque chez un receleur.", 700, WeaponType::Sword, 9, 14, 4, 120);
    receiverCase.stolenWeapon.setPersistentId("vault_receiver_weapon_001");
    receiverInvestigator.setLoadedVaultTheftCases({receiverCase});

    // Seeing the wrong biome cannot magically reveal the exact stolen property.
    const VaultTheftExplorationObservationResult wrongBiomeProperty = receiverInvestigator.observeVaultTheftCaseDuringExploration(
        "port_lanterne", "Canaux de brume bleue", VaultTheftExplorationObservation::RecognizedProperty);
    assert(wrongBiomeProperty.occurred);
    assert(!wrongBiomeProperty.recognizedStolenProperty);
    assert(wrongBiomeProperty.cluesGained == 1);
    const PlayerVaultTheftCase* anchoredReceiver = receiverInvestigator.getOpenVaultTheftCaseForCity("port_lanterne");
    assert(anchoredReceiver != nullptr);
    assert(anchoredReceiver->holderWorldContactId == "port_lanterne_receleur_collecteur");
    assert(anchoredReceiver->holderLocationId == "underbridge");
    assert(anchoredReceiver->holderExplorationBiome == "Marché sous les ponts");

    // At the actual linked place, recognizing the exact property can provide the strong evidence.
    const VaultTheftExplorationObservationResult rightBiomeProperty = receiverInvestigator.observeVaultTheftCaseDuringExploration(
        "port_lanterne", "Marché sous les ponts", VaultTheftExplorationObservation::RecognizedProperty);
    assert(rightBiomeProperty.recognizedStolenProperty);
    assert(rightBiomeProperty.cluesGained == 2);
    assert(rightBiomeProperty.targetIdentifiedNow);
    assert(rightBiomeProperty.targetLabel == "Collecteur masqué");
    const PlayerVaultTheftCase* identifiedReceiver = receiverInvestigator.getOpenVaultTheftCaseForCity("port_lanterne");
    assert(identifiedReceiver != nullptr && identifiedReceiver->culpritIdentified);
    assert(identifiedReceiver->holderLocationLabel == "Marché sous les ponts");

    // V3.51.59: a world-anchored receiver can be observed and negotiated with instead of
    // forcing a combat. Restitution is not a confession and payment only occurs on success.
    Player contactNegotiator("Négociateur", ClassCatalog::createClassByName("Guerrier"));
    contactNegotiator.getInventory().earnEconomyUnits(1000);
    PlayerVaultTheftCase negotiableCase;
    negotiableCase.caseId = "vault_theft:port_lanterne:negotiation";
    negotiableCase.cityId = "port_lanterne";
    negotiableCase.clues = 6;
    negotiableCase.holderType = "receiver";
    negotiableCase.holderWorldContactId = "port_lanterne_receleur_collecteur";
    negotiableCase.holderLabel = "Collecteur masqué";
    negotiableCase.holderLocationId = "underbridge";
    negotiableCase.holderLocationLabel = "Marché sous les ponts";
    negotiableCase.holderExplorationBiome = "Marché sous les ponts";
    negotiableCase.stolenCopper = 70;
    negotiableCase.hasWeapon = true;
    negotiableCase.stolenWeapon = Weapon("Sabre négocié", "Bien suivi sans accusation automatique.", 300, WeaponType::Sword, 6, 10, 2, 100);
    negotiableCase.stolenWeapon.setPersistentId("vault_negotiated_weapon_001");
    contactNegotiator.setLoadedVaultTheftCases({negotiableCase});
    const long long beforeNegotiationCopper = contactNegotiator.getInventory().getTotalCopper();
    Random negotiationRandom(1u); // first roll 42, safely below the high-evidence success chance.
    const VaultTheftContactInteractionResult negotiated = contactNegotiator.interactWithVaultTheftHolder(
        "port_lanterne", VaultTheftContactAction::NegotiateReturn, negotiationRandom);
    assert(negotiated.available);
    assert(negotiated.success);
    assert(negotiated.propertyRecovered);
    assert(negotiated.economyCost > 0);
    assert(contactNegotiator.getInventory().getTotalCopper() < beforeNegotiationCopper);
    assert(!contactNegotiator.hasOpenVaultTheftCaseForCity("port_lanterne"));

    // V3.51.60: a well-supported case can involve the municipal guard without turning
    // a receiver into a magically proven burglar. Cooperation/pressure are persistent facts.
    Player guardInvestigator("Enquêteur de garde", ClassCatalog::createClassByName("Guerrier"));
    PlayerVaultTheftCase guardCase;
    guardCase.caseId = "vault_theft:port_lanterne:guard";
    guardCase.cityId = "port_lanterne";
    guardCase.clues = 6;
    guardCase.holderType = "receiver";
    guardCase.holderWorldContactId = "port_lanterne_receleur_collecteur";
    guardCase.holderLabel = "Collecteur masqué";
    guardCase.holderLocationId = "underbridge";
    guardCase.holderLocationLabel = "Marché sous les ponts";
    guardCase.holderExplorationBiome = "Marché sous les ponts";
    guardCase.holderObservationCount = 2;
    guardCase.stolenCopper = 40;
    guardInvestigator.setLoadedVaultTheftCases({guardCase});
    PlayerCrimeContactState& nearbyNetworkContact = guardInvestigator.getOrCreateCrimeContactState(
        "port_lanterne_complice_quais", "Seline des amarres");
    assert(nearbyNetworkContact.pressure == 0);
    Random guardRandom(1u);
    const VaultTheftContactInteractionResult guardResult = guardInvestigator.interactWithVaultTheftHolder(
        "port_lanterne", VaultTheftContactAction::RequestGuardIntervention, guardRandom);
    assert(guardResult.available);
    assert(guardResult.guardInvolved);
    assert(guardResult.success);
    assert(guardResult.contactCooperated);
    const PlayerCrimeContactState* networkAfterGuard = guardInvestigator.findCrimeContactState("port_lanterne_complice_quais");
    assert(networkAfterGuard != nullptr);
    assert(networkAfterGuard->pressure >= 5);
    assert(networkAfterGuard->cooperation <= -1);
    const PlayerVaultTheftCase* guardAfter = guardInvestigator.getOpenVaultTheftCaseForCity("port_lanterne");
    if (guardAfter != nullptr)
    {
        assert(guardAfter->holderReportedToGuard);
        assert(guardAfter->holderPressure >= 25);
        assert(guardAfter->holderCooperation >= 35);
    }

    // A cache is a place, not a fake culprit: it can be searched and resolve the property trail
    // without manufacturing a rival/person who was never observed.
    Player cacheSearcher("Fouilleur", ClassCatalog::createClassByName("Guerrier"));
    PlayerVaultTheftCase searchableCache;
    searchableCache.caseId = "vault_theft:valebrume:searchable_cache";
    searchableCache.cityId = "valebrume";
    searchableCache.clues = 6;
    searchableCache.holderType = "cache";
    searchableCache.holderWorldContactId = "valebrume_cache_sous_bois";
    searchableCache.holderLabel = "Cache du vieux sous-bois";
    searchableCache.holderLocationId = "old_woods";
    searchableCache.holderLocationLabel = "Vieux sous-bois hors des axes";
    searchableCache.holderExplorationBiome = "Vieux sous-bois";
    searchableCache.stolenMaterialId = "cache_test_ore";
    searchableCache.stolenMaterialName = "Minerai caché";
    searchableCache.stolenMaterialQuality = "normal";
    searchableCache.stolenMaterialQuantity = 4;
    cacheSearcher.setLoadedVaultTheftCases({searchableCache});
    Random cacheRandom(1u);
    const VaultTheftContactInteractionResult searched = cacheSearcher.interactWithVaultTheftHolder(
        "valebrume", VaultTheftContactAction::SearchCache, cacheRandom);
    assert(searched.available);
    assert(searched.success);
    assert(searched.propertyRecovered);
    assert(cacheSearcher.getRivalRecords().empty());
    assert(!cacheSearcher.hasOpenVaultTheftCaseForCity("valebrume"));

    // Recall messenger surcharge can be shared briefly on the same route, then resets after > half a day.
    Player messengerTest("Messager", ClassCatalog::createClassByName("Guerrier"));
    const int fullMessenger = messengerTest.getCityVaultRecallMessengerSurchargeCopper("port_lanterne", "valebrume", 20);
    messengerTest.markCityVaultRecallMessengerSent("port_lanterne", "valebrume");
    const int sharedMessenger = messengerTest.getCityVaultRecallMessengerSurchargeCopper("port_lanterne", "valebrume", 20);
    assert(sharedMessenger < fullMessenger);
    messengerTest.advanceWorldDayUnits(3);
    assert(messengerTest.getCityVaultRecallMessengerSurchargeCopper("port_lanterne", "valebrume", 20) == fullMessenger);

    return 0;
}
