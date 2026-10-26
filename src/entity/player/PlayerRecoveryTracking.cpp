#include "entity/Player.hpp"
#include "world/crime/VaultTheftNetworkCatalog.hpp"

#include "core/Random.hpp"

#include <algorithm>

namespace
{
    int inventoryObjectCount(const Inventory& inventory)
    {
        int count = inventory.getWeaponCount()
            + inventory.getArmorCount()
            + inventory.getConsumableCount()
            + inventory.getMaterialCount();
        const CoinBreakdown coins = inventory.getCoinStacks();
        if (coins.copper + coins.iron + coins.electrum + coins.gold + coins.platinum > 0) ++count;
        return count;
    }

    void transferInventoryContents(Inventory& destination, const Inventory& source)
    {
        for (const Weapon& weapon : source.getWeapons()) destination.addWeapon(weapon);
        for (const Armor& armor : source.getArmors()) destination.addArmor(armor);
        for (const Consumable& consumable : source.getConsumables()) destination.addConsumable(consumable);
        for (const Material& material : source.getMaterials()) destination.addMaterial(material);
        destination.earnCoinStacks(source.getCoinStacks());
    }
}

const std::vector<PlayerBattlefieldRecoveryCache>& Player::getBattlefieldRecoveryCaches() const
{
    return battlefieldRecoveryCaches;
}

bool Player::hasOpenBattlefieldRecoveryCacheForCity(const std::string& cityId) const
{
    return std::any_of(battlefieldRecoveryCaches.begin(), battlefieldRecoveryCaches.end(), [&](const PlayerBattlefieldRecoveryCache& cache) {
        return !cache.resolved && cache.cityId == cityId && inventoryObjectCount(cache.inventory) > 0;
    });
}

std::string Player::createBattlefieldRecoveryCache(
    const std::string& contextId,
    const std::string& locationLabel,
    const Inventory& cacheInventory)
{
    if (inventoryObjectCount(cacheInventory) <= 0) return "";

    PlayerBattlefieldRecoveryCache cache;
    cache.cacheId = "battlefield_recovery:" + currentCityId + ":day" + std::to_string(worldDaysElapsed)
        + ":" + std::to_string(battlefieldRecoveryCaches.size() + 1);
    cache.cityId = currentCityId;
    cache.contextId = contextId;
    cache.locationLabel = locationLabel.empty() ? "zone du combat" : locationLabel;
    cache.createdDay = worldDaysElapsed;
    cache.inventory = cacheInventory;
    battlefieldRecoveryCaches.push_back(cache);

    recordCanonicalEvent(
        "battlefield_recovery_caches",
        cache.cacheId,
        "Objets importants laissés physiquement sur le terrain : " + cache.locationLabel,
        inventoryObjectCount(cacheInventory));
    recordHistoricalEvent(
        "battlefield_recovery_created",
        cache.cacheId,
        "Une récupération différée reste possible autour de " + cache.locationLabel + ".",
        false);
    return cache.cacheId;
}

BattlefieldRecoveryAttemptResult Player::attemptBattlefieldRecovery(const std::string& cacheId, Random& random)
{
    BattlefieldRecoveryAttemptResult result;
    PlayerBattlefieldRecoveryCache* cache = nullptr;
    for (PlayerBattlefieldRecoveryCache& candidate : battlefieldRecoveryCaches)
    {
        if (candidate.cacheId == cacheId)
        {
            cache = &candidate;
            break;
        }
    }

    if (cache == nullptr || cache->resolved || inventoryObjectCount(cache->inventory) <= 0)
    {
        result.lines.push_back("Cette piste de récupération n'existe plus ou a déjà été résolue.");
        return result;
    }

    if (cache->custodyState.empty()) cache->custodyState = "field";
    result.cacheId = cache->cacheId;
    result.locationLabel = cache->locationLabel;
    result.custodyState = cache->custodyState;
    result.custodyLabel = cache->custodyLabel;
    result.ageDays = std::max(0, worldDaysElapsed - cache->createdDay);
    if (cache->cityId != currentCityId)
    {
        result.lines.push_back("Cette zone de récupération dépend d'une autre ville. Reviens dans son secteur avant de repartir sur la piste.");
        return result;
    }
    if (cache->lastAttemptDay == worldDaysElapsed)
    {
        result.lines.push_back("Tu as déjà fouillé cette piste aujourd'hui. Recommencer immédiatement ne fait pas apparaître magiquement les objets.");
        return result;
    }

    result.available = true;
    int chance = result.ageDays <= 1 ? 100
        : result.ageDays <= 3 ? 90
        : result.ageDays <= 7 ? 75
        : 60;

    if (cache->custodyState == "guard")
    {
        // Once the guard physically secured the exact lot, the remaining problem is locating the report,
        // not rolling again to see whether the object still exists.
        chance = 100;
        result.securedByGuard = true;
    }
    else if (cache->custodyState == "civilian")
    {
        chance = std::max(chance, 90);
        result.foundByCivilian = true;
    }
    else if (cache->custodyState == "nearby_cache")
    {
        chance -= 5;
    }
    else if (cache->custodyState == "unknown_holder")
    {
        // An unnamed finder/scavenger is a harder lead, but still not an invented thief/rival.
        chance = std::min(chance, 62);
        chance += std::min(18, cache->searchAttempts * 6);
    }

    chance += std::min(21, cache->searchAttempts * 7);
    chance -= cache->disturbanceLevel * 8;
    chance = std::clamp(chance, 35, 100);
    if (cache->custodyState == "guard") chance = 100;
    else if (cache->custodyState == "civilian") chance = std::max(chance, 90);
    result.chancePercent = chance;

    cache->lastAttemptDay = worldDaysElapsed;
    advanceWorldDayUnits(cache->custodyState == "guard" || cache->custodyState == "civilian" ? 1 : 2);
    result.success = random.between(1, 100) <= chance;
    if (result.success)
    {
        transferInventoryContents(inventory, cache->inventory);
        // The exact objects have physically moved back to the player. Keep the resolved
        // cache as history, but never retain a second copy of the same persistent IDs.
        cache->inventory.clearAll();
        result.recoveredProperty = true;
        cache->resolved = true;
        result.lines.push_back("Tu suis réellement la piste liée à " + cache->locationLabel + " et récupères le lot exact.");
        if (cache->custodyState == "guard")
            result.lines.push_back("La garde avait mis les objets en sécurité après les avoir trouvés ou reçus : ils te sont restitués comme les mêmes objets persistants.");
        else if (cache->custodyState == "civilian")
            result.lines.push_back("Un civil avait gardé le lot à l'abri au lieu de le faire disparaître. Rien n'autorise le jeu à le transformer en voleur sans preuve.");
        else if (cache->custodyState == "unknown_holder")
            result.lines.push_back("La piste du récupérateur anonyme aboutit finalement. Son identité reste inconnue si rien ne permet réellement de l'établir.");
        else
            result.lines.push_back("Les objets importants laissés après le KO reviennent comme les mêmes objets persistants, pas comme des copies recréées depuis un catalogue.");
        if (cache->movementCount > 0)
            result.lines.push_back("Le lot avait changé de place " + std::to_string(cache->movementCount) + " fois avant d'être retrouvé.");
        recordCanonicalEvent("battlefield_recovery_success", cache->cacheId, "Récupération différée réussie : " + cache->locationLabel, 1);
        recordHistoricalEvent("battlefield_recovery_resolved", cache->cacheId, "Les objets laissés sur le terrain ont finalement été récupérés.", true);
        return result;
    }

    ++cache->searchAttempts;
    cache->disturbanceLevel = std::min(3, cache->disturbanceLevel + 1);
    result.lines.push_back("Tu reviens vers " + cache->locationLabel + ", mais le lot n'est plus exactement là où l'équipe l'avait laissé.");

    const std::string previousCustody = cache->custodyState;
    if (cache->custodyState == "field" && cache->disturbanceLevel >= 2)
    {
        cache->custodyState = "nearby_cache";
        cache->custodyLabel = "déplacé à proximité par une personne inconnue";
        cache->locationKnown = false;
    }
    else if (cache->custodyState == "nearby_cache" && cache->disturbanceLevel >= 3)
    {
        const int movementRoll = random.between(1, 100);
        if (movementRoll <= 25)
        {
            cache->custodyState = "guard";
            cache->custodyLabel = "objets sécurisés par la garde locale";
            cache->locationKnown = true;
            result.securedByGuard = true;
        }
        else if (movementRoll <= 50)
        {
            cache->custodyState = "civilian";
            cache->custodyLabel = "lot recueilli par un habitant du secteur";
            cache->locationKnown = true;
            result.foundByCivilian = true;
        }
        else
        {
            cache->custodyState = "unknown_holder";
            cache->custodyLabel = "lot emporté par un récupérateur non identifié";
            cache->locationKnown = false;
        }
    }
    else if (cache->custodyState == "unknown_holder" && cache->searchAttempts >= 3)
    {
        // The trail can become more concrete without inventing a named criminal.
        cache->custodyLabel = "piste d'un récupérateur opportuniste dans le secteur";
    }

    if (cache->custodyState != previousCustody)
    {
        ++cache->movementCount;
        cache->lastMovementDay = worldDaysElapsed;
        result.custodyChanged = true;
        result.relocated = true;
        result.custodyState = cache->custodyState;
        result.custodyLabel = cache->custodyLabel;
        recordCanonicalEvent(
            "battlefield_recovery_movements",
            cache->cacheId,
            "Le lot change de garde : " + cache->custodyLabel,
            1);
        recordHistoricalEvent(
            "battlefield_recovery_moved",
            cache->cacheId,
            "La piste de récupération a évolué : " + cache->custodyLabel + ".",
            false);
    }

    if (cache->custodyState == "guard")
        result.lines.push_back("Les traces convergent vers la garde locale : quelqu'un semble leur avoir remis le lot. La prochaine démarche devient administrative plutôt qu'une fouille au hasard.");
    else if (cache->custodyState == "civilian")
        result.lines.push_back("Des témoignages indiquent qu'un habitant a mis le lot à l'abri. Rien ne prouve un vol : la piste devient une recherche de témoin honnête.");
    else if (cache->custodyState == "unknown_holder")
        result.lines.push_back("Le lot a quitté la zone immédiate avec une personne non identifiée. Le moteur conserve la piste sans fabriquer un rival ou un coupable omniscient.");
    else if (cache->custodyState == "nearby_cache")
        result.lines.push_back("Quelqu'un a probablement déplacé le lot à proximité. La piste devient une vraie recherche, pas un simple retour au point exact.");
    else
        result.lines.push_back("Des traces récentes montrent que la zone a été fouillée. Les objets importants restent suivis, mais la prochaine recherche sera moins directe.");

    if (cache->disturbanceLevel >= 3 && cache->custodyState == "field")
        result.lines.push_back("La piste est froide et très perturbée. Le jeu ne détruit toutefois pas silencieusement un objet Quête/Personnel/Unique sans chemin de récupération crédible.");

    recordCanonicalEvent("battlefield_recovery_searches", cache->cacheId, "Recherche différée infructueuse : " + cache->locationLabel, 1);
    return result;
}

void Player::setLoadedBattlefieldRecoveryCaches(const std::vector<PlayerBattlefieldRecoveryCache>& caches)
{
    battlefieldRecoveryCaches = caches;
}

std::vector<PlayerCrimeContactState>& Player::getCrimeContactStates()
{
    return crimeContactStates;
}

const std::vector<PlayerCrimeContactState>& Player::getCrimeContactStates() const
{
    return crimeContactStates;
}

const PlayerCrimeContactState* Player::findCrimeContactState(const std::string& contactId) const
{
    for (const PlayerCrimeContactState& state : crimeContactStates)
        if (state.contactId == contactId) return &state;
    return nullptr;
}

PlayerCrimeContactState& Player::getOrCreateCrimeContactState(const std::string& contactId, const std::string& displayName)
{
    for (PlayerCrimeContactState& state : crimeContactStates)
    {
        if (state.contactId != contactId) continue;
        if (!displayName.empty()) state.displayName = displayName;
        if (const VaultTheftNetworkContact* catalogContact = VaultTheftNetworkCatalog::findById(contactId))
        {
            if (state.homeCityId.empty()) state.homeCityId = catalogContact->cityId;
            if (state.networkId.empty()) state.networkId = catalogContact->networkId;
        }

        // V3.51.62: timers now have actual lifecycle consequences instead of silently expiring.
        // A provisional detention can end without becoming a permanent arrest; a fugitive can
        // return to the area later, while keeping memory of what happened in previous cases.
        if (!state.arrested && state.detainedUntilDay >= 0 && state.detainedUntilDay <= worldDaysElapsed
            && state.lastReleaseDay < state.detainedUntilDay)
        {
            state.lastReleaseDay = worldDaysElapsed;
            ++state.releases;
            state.pressure = std::max(0, state.pressure - 8);
            state.cooperation = std::clamp(state.cooperation + (state.reportedToGuard ? 3 : 1), -100, 100);
            recordCanonicalEvent(
                "crime_contact_releases",
                state.contactId,
                state.displayName + " sort de détention provisoire sans condamnation automatique.",
                1);
        }
        if (state.fledUntilDay >= 0 && state.fledUntilDay <= worldDaysElapsed
            && state.lastReturnDay < state.fledUntilDay)
        {
            state.lastReturnDay = worldDaysElapsed;
            ++state.returnsAfterFlight;
            state.pressure = std::max(0, state.pressure - 5);
            state.cooperation = std::clamp(state.cooperation - 2, -100, 100);
            recordCanonicalEvent(
                "crime_contact_returns",
                state.contactId,
                state.displayName + " est de nouveau signalé dans le secteur après une fuite.",
                1);
        }
        return state;
    }
    PlayerCrimeContactState state;
    state.contactId = contactId;
    state.displayName = displayName;
    state.firstSeenDay = worldDaysElapsed;
    if (const VaultTheftNetworkContact* catalogContact = VaultTheftNetworkCatalog::findById(contactId))
    {
        state.homeCityId = catalogContact->cityId;
        state.networkId = catalogContact->networkId;
    }
    crimeContactStates.push_back(state);
    return crimeContactStates.back();
}

void Player::setLoadedCrimeContactStates(const std::vector<PlayerCrimeContactState>& states)
{
    crimeContactStates = states;
}
