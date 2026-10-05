// EN: Split from Player.cpp to keep Player responsibilities maintainable.
// FR: Extrait de Player.cpp afin de garder des responsabilités Player maintenables.
#include "entity/Player.hpp"
#include "progression/language/LanguageSystem.hpp"
#include "economy/EconomyBalance.hpp"
#include "core/VersionInfo.hpp"
#include "economy/Money.hpp"
#include "item/weapon/WeaponCatalog.hpp"
#include "item/armor/ArmorCatalog.hpp"
#include "item/consumable/ConsumableCatalog.hpp"
#include "item/material/MaterialCatalog.hpp"
#include "progression/DifficultyRules.hpp"
#include "progression/Level.hpp"
#include "progression/TitleCatalog.hpp"
#include "character/RaceCatalog.hpp"
#include "combat/system/CombatClassSystem.hpp"
#include "interface/menu/common/MessageScreen.hpp"
#include "item/equipment/EquipmentWeightRules.hpp"
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <random>
#include <sstream>
#include <tuple>
#include <vector>

Inventory& Player::getInventory()
{
    return inventory;
}

// EN: getInventory declares or implements a focused behavior used by this module.
// FR: getInventory déclare ou implémente un comportement précis utilisé par ce module.
const Inventory& Player::getInventory() const
{
    return inventory;
}

Inventory& Player::getCityVault()
{
    return cityVault;
}

const Inventory& Player::getCityVault() const
{
    return cityVault;
}

int Player::calculateVaultCapacity(bool purchased, int level)
{
    if (!purchased || level <= 0)
    {
        return 0;
    }

    return EconomyBalance::cityVaultCapacityForLevel(level);
}

int Player::calculateVaultUsedSlots(const Inventory& vault)
{
    // EN: Materials consume one slot per stored stack/quality, not one slot per unit.
    // FR: Les matériaux consomment une place par pile/qualité stockée, pas une place par unité.
    return vault.getWeaponCount() * 3
        + vault.getArmorCount() * 3
        + vault.getConsumableCount()
        + static_cast<int>(vault.getMaterials().size());
}

PlayerCityVault* Player::findCityVaultRecord(const std::string& cityId)
{
    for (PlayerCityVault& record : cityVaults)
    {
        if (record.cityId == cityId)
        {
            return &record;
        }
    }

    return nullptr;
}

const PlayerCityVault* Player::findCityVaultRecord(const std::string& cityId) const
{
    for (const PlayerCityVault& record : cityVaults)
    {
        if (record.cityId == cityId)
        {
            return &record;
        }
    }

    return nullptr;
}

void Player::syncCurrentCityVaultRecord()
{
    if (currentCityId.empty())
    {
        currentCityId = "valebrume";
    }

    PlayerCityVault* record = findCityVaultRecord(currentCityId);
    const bool hasStoredContent = cityVault.getWeaponCount() > 0
        || cityVault.getArmorCount() > 0
        || cityVault.getConsumableCount() > 0
        || !cityVault.getMaterials().empty();

    if (record == nullptr)
    {
        if (!cityVaultPurchased && cityVaultLevel <= 0 && !hasStoredContent)
        {
            return;
        }

        PlayerCityVault created;
        created.cityId = currentCityId;
        created.inventory = cityVault;
        created.inventory.setTotalCopper(0);
        created.purchased = cityVaultPurchased;
        created.level = cityVaultPurchased ? std::max(1, std::min(5, cityVaultLevel)) : 0;
        cityVaults.push_back(created);
        return;
    }

    record->inventory = cityVault;
    record->inventory.setTotalCopper(0);
    record->purchased = cityVaultPurchased;
    record->level = cityVaultPurchased ? std::max(1, std::min(5, cityVaultLevel)) : 0;
}

void Player::loadCurrentCityVaultRecord()
{
    if (currentCityId.empty())
    {
        currentCityId = "valebrume";
    }

    const PlayerCityVault* record = findCityVaultRecord(currentCityId);
    if (record == nullptr)
    {
        cityVault = Inventory();
        cityVault.setTotalCopper(0);
        cityVaultPurchased = false;
        cityVaultLevel = 0;
        return;
    }

    cityVault = record->inventory;
    cityVault.setTotalCopper(0);
    cityVaultPurchased = record->purchased;
    cityVaultLevel = cityVaultPurchased ? std::max(1, std::min(5, record->level)) : 0;
}

const std::vector<PlayerCityVault>& Player::getCityVaultRecords() const
{
    return cityVaults;
}

const Inventory& Player::getCityVaultForCity(const std::string& cityId) const
{
    if (cityId == currentCityId)
    {
        return cityVault;
    }

    const PlayerCityVault* record = findCityVaultRecord(cityId);
    if (record != nullptr)
    {
        return record->inventory;
    }

    static const Inventory emptyVault;
    return emptyVault;
}

bool Player::hasCityVaultInCity(const std::string& cityId) const
{
    if (cityId == currentCityId)
    {
        return hasCityVault();
    }

    const PlayerCityVault* record = findCityVaultRecord(cityId);
    return record != nullptr && record->purchased && record->level > 0;
}

int Player::getCityVaultLevelForCity(const std::string& cityId) const
{
    if (cityId == currentCityId)
    {
        return getCityVaultLevel();
    }

    const PlayerCityVault* record = findCityVaultRecord(cityId);
    if (record == nullptr || !record->purchased)
    {
        return 0;
    }

    return std::max(1, std::min(5, record->level));
}

int Player::getCityVaultCapacityForCity(const std::string& cityId) const
{
    return calculateVaultCapacity(hasCityVaultInCity(cityId), getCityVaultLevelForCity(cityId));
}

int Player::getCityVaultUsedSlotsForCity(const std::string& cityId) const
{
    if (!hasCityVaultInCity(cityId))
    {
        return 0;
    }

    return calculateVaultUsedSlots(getCityVaultForCity(cityId));
}

bool Player::hasCityVault() const
{
    return cityVaultPurchased && cityVaultLevel > 0;
}

int Player::getCityVaultLevel() const
{
    return cityVaultLevel;
}

int Player::getCityVaultCapacity() const
{
    return calculateVaultCapacity(hasCityVault(), cityVaultLevel);
}

int Player::getCityVaultUsedSlots() const
{
    return calculateVaultUsedSlots(cityVault);
}

int Player::getCityVaultPurchaseCost() const
{
    return EconomyBalance::cityVaultPurchaseCost(currentCityId);
}

int Player::getCityVaultUpgradeCost() const
{
    if (!hasCityVault() || !canUpgradeCityVault())
    {
        return 0;
    }

    return EconomyBalance::cityVaultUpgradeCost(currentCityId, cityVaultLevel);
}

bool Player::canUpgradeCityVault() const
{
    return hasCityVault() && cityVaultLevel < 5;
}

bool Player::purchaseCityVault()
{
    loadCurrentCityVaultRecord();
    if (hasCityVault())
    {
        return false;
    }
    if (!inventory.spendEconomyUnits(getCityVaultPurchaseCost()))
    {
        return false;
    }
    cityVaultPurchased = true;
    cityVaultLevel = 1;
    recordCanonicalEvent("coffres_achetes", currentCityId, "Coffre municipal de " + currentCityId);
    syncCurrentCityVaultRecord();
    return true;
}

bool Player::upgradeCityVault()
{
    loadCurrentCityVaultRecord();
    if (!canUpgradeCityVault())
    {
        return false;
    }
    const int cost = getCityVaultUpgradeCost();
    if (cost <= 0 || !inventory.spendEconomyUnits(cost))
    {
        return false;
    }
    ++cityVaultLevel;
    recordCanonicalEvent("coffres_ameliores", currentCityId, "Coffre municipal de " + currentCityId);
    syncCurrentCityVaultRecord();
    return true;
}

bool Player::depositWeaponInCityVault(int index)
{
    loadCurrentCityVaultRecord();
    if (!hasCityVault() || !inventory.hasWeapon(index) || index == equippedWeaponIndex
        || getCityVaultUsedSlots() + 3 > getCityVaultCapacity())
    {
        return false;
    }
    const Weapon weapon = inventory.getWeapon(index);
    if (!inventory.removeWeapon(index))
    {
        return false;
    }
    if (equippedWeaponIndex > index)
    {
        --equippedWeaponIndex;
    }
    cityVault.addWeapon(weapon);
    recordCanonicalEvent("objets_deposes", "weapon:" + weapon.getName(), weapon.getName());
    syncCurrentCityVaultRecord();
    return true;
}

bool Player::depositArmorInCityVault(int index)
{
    loadCurrentCityVaultRecord();
    if (!hasCityVault() || !inventory.hasArmor(index) || index == equippedArmorIndex
        || getCityVaultUsedSlots() + 3 > getCityVaultCapacity())
    {
        return false;
    }
    const Armor armor = inventory.getArmor(index);
    if (armor.getName() == "Tenue simple" || !inventory.removeArmor(index))
    {
        return false;
    }
    if (equippedArmorIndex > index)
    {
        --equippedArmorIndex;
    }
    cityVault.addArmor(armor);
    recordCanonicalEvent("objets_deposes", "armor:" + armor.getName(), armor.getName());
    syncCurrentCityVaultRecord();
    return true;
}

bool Player::depositConsumableInCityVault(int index)
{
    loadCurrentCityVaultRecord();
    if (!hasCityVault() || !inventory.hasConsumable(index)
        || getCityVaultUsedSlots() + 1 > getCityVaultCapacity())
    {
        return false;
    }
    const Consumable consumable = inventory.getConsumable(index);
    if (!inventory.removeConsumable(index))
    {
        return false;
    }
    cityVault.addConsumable(consumable);
    recordCanonicalEvent("objets_deposes", "consumable:" + consumable.getName(), consumable.getName());
    syncCurrentCityVaultRecord();
    return true;
}

bool Player::depositMaterialInCityVault(int index, int quantity)
{
    loadCurrentCityVaultRecord();
    if (!hasCityVault() || !inventory.hasMaterial(index))
    {
        return false;
    }
    Material material = inventory.getMaterial(index);
    if (quantity <= 0)
    {
        quantity = material.getQuantity();
    }
    quantity = std::min(quantity, material.getQuantity());
    if (quantity <= 0)
    {
        return false;
    }

    bool sameStackExists = false;
    for (const Material& stored : cityVault.getMaterials())
    {
        if (stored.getId() == material.getId() && stored.getQuality() == material.getQuality())
        {
            sameStackExists = true;
            break;
        }
    }
    const int additionalSlots = sameStackExists ? 0 : 1;
    if (getCityVaultUsedSlots() + additionalSlots > getCityVaultCapacity())
    {
        return false;
    }

    material.setQuantity(quantity);
    if (!inventory.removeMaterialQuantity(index, quantity))
    {
        return false;
    }
    cityVault.addMaterial(material);
    recordCanonicalEvent("materiaux_deposes", material.getId(), material.getName(), quantity);
    syncCurrentCityVaultRecord();
    return true;
}

bool Player::withdrawWeaponFromCityVault(int index)
{
    loadCurrentCityVaultRecord();
    if (!hasCityVault() || !cityVault.hasWeapon(index))
    {
        return false;
    }
    const Weapon weapon = cityVault.getWeapon(index);
    if (!cityVault.removeWeapon(index))
    {
        return false;
    }
    inventory.addWeapon(weapon);
    recordCanonicalEvent("objets_retires", "weapon:" + weapon.getName(), weapon.getName());
    syncCurrentCityVaultRecord();
    return true;
}

bool Player::withdrawArmorFromCityVault(int index)
{
    loadCurrentCityVaultRecord();
    if (!hasCityVault() || !cityVault.hasArmor(index))
    {
        return false;
    }
    const Armor armor = cityVault.getArmor(index);
    if (!cityVault.removeArmor(index))
    {
        return false;
    }
    inventory.addArmor(armor);
    recordCanonicalEvent("objets_retires", "armor:" + armor.getName(), armor.getName());
    syncCurrentCityVaultRecord();
    return true;
}

bool Player::withdrawConsumableFromCityVault(int index)
{
    loadCurrentCityVaultRecord();
    if (!hasCityVault() || !cityVault.hasConsumable(index))
    {
        return false;
    }
    const Consumable consumable = cityVault.getConsumable(index);
    if (!cityVault.removeConsumable(index))
    {
        return false;
    }
    inventory.addConsumable(consumable);
    recordCanonicalEvent("objets_retires", "consumable:" + consumable.getName(), consumable.getName());
    syncCurrentCityVaultRecord();
    return true;
}

bool Player::withdrawMaterialFromCityVault(int index, int quantity)
{
    loadCurrentCityVaultRecord();
    if (!hasCityVault() || !cityVault.hasMaterial(index))
    {
        return false;
    }
    Material material = cityVault.getMaterial(index);
    if (quantity <= 0)
    {
        quantity = material.getQuantity();
    }
    quantity = std::min(quantity, material.getQuantity());
    if (quantity <= 0)
    {
        return false;
    }
    material.setQuantity(quantity);
    if (!cityVault.removeMaterialQuantity(index, quantity))
    {
        return false;
    }
    inventory.addMaterial(material);
    recordCanonicalEvent("materiaux_retires", material.getId(), material.getName(), quantity);
    syncCurrentCityVaultRecord();
    return true;
}

bool Player::transferMaterialBetweenCityVaults(const std::string& destinationCityId, int materialIndex, int quantity, int costCopper)
{
    if (destinationCityId.empty() || destinationCityId == currentCityId || costCopper < 0)
    {
        return false;
    }

    loadCurrentCityVaultRecord();
    if (!hasCityVault() || !cityVault.hasMaterial(materialIndex) || !hasCityVaultInCity(destinationCityId))
    {
        return false;
    }

    PlayerCityVault* destinationRecord = findCityVaultRecord(destinationCityId);
    if (destinationRecord == nullptr || !destinationRecord->purchased || destinationRecord->level <= 0)
    {
        return false;
    }

    Material material = cityVault.getMaterial(materialIndex);
    if (quantity <= 0)
    {
        quantity = material.getQuantity();
    }
    quantity = std::min(quantity, material.getQuantity());
    if (quantity <= 0)
    {
        return false;
    }

    bool sameStackExists = false;
    for (const Material& stored : destinationRecord->inventory.getMaterials())
    {
        if (stored.getId() == material.getId() && stored.getQuality() == material.getQuality())
        {
            sameStackExists = true;
            break;
        }
    }

    const int additionalSlots = sameStackExists ? 0 : 1;
    const int destinationCapacity = calculateVaultCapacity(destinationRecord->purchased, destinationRecord->level);
    const int destinationUsed = calculateVaultUsedSlots(destinationRecord->inventory);
    if (destinationUsed + additionalSlots > destinationCapacity)
    {
        return false;
    }

    if (!inventory.spendCopper(costCopper))
    {
        return false;
    }

    material.setQuantity(quantity);
    if (!cityVault.removeMaterialQuantity(materialIndex, quantity))
    {
        inventory.earnCopper(costCopper);
        return false;
    }

    destinationRecord->inventory.addMaterial(material);
    destinationRecord->inventory.setTotalCopper(0);
    recordCanonicalEvent("transports_coffres_municipaux", currentCityId + "->" + destinationCityId, "Transport de coffre municipal", 1);
    recordCanonicalEvent("materiaux_transportes_coffres", material.getId(), material.getName(), quantity);
    recordCanonicalEvent("couts_transport_coffres", destinationCityId, "Transport vers coffre municipal", costCopper);
    syncCurrentCityVaultRecord();
    return true;
}

const std::string& Player::getCurrentCityId() const
{
    return currentCityId;
}

void Player::setCurrentCityId(const std::string& cityId)
{
    if (!cityId.empty() && cityId != currentCityId)
    {
        syncCurrentCityVaultRecord();
        currentCityId = cityId;
        loadCurrentCityVaultRecord();
    }
}

const std::vector<std::string>& Player::getRegisteredGuildCityIds() const
{
    return registeredGuildCityIds;
}

bool Player::isRegisteredAtCityGuild(const std::string& cityId) const
{
    return std::find(registeredGuildCityIds.begin(), registeredGuildCityIds.end(), cityId) != registeredGuildCityIds.end();
}

bool Player::isRegisteredAtCurrentCityGuild() const
{
    return isRegisteredAtCityGuild(currentCityId);
}

bool Player::registerAtCurrentCityGuild()
{
    if (currentCityId.empty() || isRegisteredAtCurrentCityGuild())
    {
        return false;
    }
    registeredGuildCityIds.push_back(currentCityId);
    return true;
}

void Player::setLoadedCityState(
    bool vaultPurchased,
    int vaultLevel,
    const std::string& loadedCurrentCityId,
    const Inventory& loadedVault,
    const std::vector<std::string>& loadedRegisteredGuildCityIds,
    const std::vector<PlayerCityVault>& loadedCityVaults
)
{
    currentCityId = loadedCurrentCityId.empty() ? "valebrume" : loadedCurrentCityId;
    cityVaults.clear();

    for (const PlayerCityVault& record : loadedCityVaults)
    {
        if (record.cityId.empty())
        {
            continue;
        }

        PlayerCityVault cleanRecord;
        cleanRecord.cityId = record.cityId;
        cleanRecord.inventory = record.inventory;
        cleanRecord.inventory.setTotalCopper(0);
        cleanRecord.purchased = record.purchased;
        cleanRecord.level = record.purchased ? std::max(1, std::min(5, record.level)) : 0;

        PlayerCityVault* existing = findCityVaultRecord(cleanRecord.cityId);
        if (existing == nullptr)
        {
            cityVaults.push_back(cleanRecord);
        }
        else
        {
            *existing = cleanRecord;
        }
    }

    if (cityVaults.empty() && (vaultPurchased || vaultLevel > 0
        || loadedVault.getWeaponCount() > 0
        || loadedVault.getArmorCount() > 0
        || loadedVault.getConsumableCount() > 0
        || !loadedVault.getMaterials().empty()))
    {
        PlayerCityVault legacyRecord;
        legacyRecord.cityId = currentCityId;
        legacyRecord.inventory = loadedVault;
        legacyRecord.inventory.setTotalCopper(0);
        legacyRecord.purchased = vaultPurchased;
        legacyRecord.level = vaultPurchased ? std::max(1, std::min(5, vaultLevel)) : 0;
        cityVaults.push_back(legacyRecord);
    }

    registeredGuildCityIds.clear();
    for (const std::string& cityId : loadedRegisteredGuildCityIds)
    {
        if (!cityId.empty() && !isRegisteredAtCityGuild(cityId))
        {
            registeredGuildCityIds.push_back(cityId);
        }
    }
    if (hasTitle("Aventurier") && registeredGuildCityIds.empty())
    {
        registeredGuildCityIds.push_back("valebrume");
    }

    loadCurrentCityVaultRecord();
    syncCurrentCityVaultRecord();
}

// EN: getQuestLog declares or implements a focused behavior used by this module.
// FR: getQuestLog déclare ou implémente un comportement précis utilisé par ce module.
