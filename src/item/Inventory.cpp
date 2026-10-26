// EN: Inventory.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: Inventory.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "item/Inventory.hpp"

#include "progression/material/MaterialKnowledgeProgress.hpp"
#include "interface/menu/common/MessageScreen.hpp"
#include "economy/Money.hpp"

#include <algorithm>
#include <string>
#include <vector>
#include <iterator>
#include <cctype>


namespace
{
    // EN: startsWith declares or implements a focused behavior used by this module.
    // FR: startsWith déclare ou implémente un comportement précis utilisé par ce module.
    bool startsWith(const std::string& value, const std::string& prefix)
    {
        return value.rfind(prefix, 0) == 0;
    }

    // EN: isRepairKitInventoryId declares or implements a focused behavior used by this module.
    // FR: isRepairKitInventoryId déclare ou implémente un comportement précis utilisé par ce module.
    bool isRepairKitInventoryId(const std::string& id)
    {
        return id == "weak_repair_kit"
            || id == "medium_repair_kit"
            || id == "big_repair_kit"
            || id == "tinkerer_complete_repair_kit"
            || startsWith(id, "weak_repair_kit_used_")
            || startsWith(id, "medium_repair_kit_used_")
            || startsWith(id, "big_repair_kit_used_")
            || startsWith(id, "tinkerer_complete_repair_kit_used_");
    }

    // EN: isUsedRepairKitInventoryId declares or implements a focused behavior used by this module.
    // FR: isUsedRepairKitInventoryId déclare ou implémente un comportement précis utilisé par ce module.
    bool isUsedRepairKitInventoryId(const std::string& id)
    {
        return startsWith(id, "weak_repair_kit_used_")
            || startsWith(id, "medium_repair_kit_used_")
            || startsWith(id, "big_repair_kit_used_")
            || startsWith(id, "tinkerer_complete_repair_kit_used_");
    }

    // EN: maxRepairKitInventoryDurability declares or implements a focused behavior used by this module.
    // FR: maxRepairKitInventoryDurability déclare ou implémente un comportement précis utilisé par ce module.
    int maxRepairKitInventoryDurability(const std::string& id)
    {
        if (startsWith(id, "medium_repair_kit")) return 3;
        if (startsWith(id, "big_repair_kit")) return 4;
        if (startsWith(id, "tinkerer_complete_repair_kit")) return 5;
        return 2;
    }

    // EN: usedRepairKitInventoryDurability declares or implements a focused behavior used by this module.
    // FR: usedRepairKitInventoryDurability déclare ou implémente un comportement précis utilisé par ce module.
    int usedRepairKitInventoryDurability(const std::string& id)
    {
        std::size_t pos = id.find_last_of('_');

        if (pos == std::string::npos || pos + 1 >= id.size())
        {
            return 1;
        }

        try
        {
            return std::stoi(id.substr(pos + 1));
        }
        catch (...)
        {
            return 1;
        }
    }

    struct ConsumableInventoryStack
    {
        int firstIndex;
        int amount;
        std::string name;
        ConsumableType type;
        int power;
        bool percentageBasedEffect;
    };

    std::string formatConsumableInventoryStackLabel(const std::string& name, int amount)
    {
        if (amount < 1)
        {
            amount = 1;
        }

        return name + " (*" + std::to_string(amount) + ")";
    }

    std::vector<ConsumableInventoryStack> groupConsumablesForInventory(const std::vector<Consumable>& consumables)
    {
        std::vector<ConsumableInventoryStack> groups;

        for (int i = 0; i < static_cast<int>(consumables.size()); ++i)
        {
            const Consumable& consumable = consumables[i];
            bool found = false;

            for (ConsumableInventoryStack& group : groups)
            {
                if (group.name == consumable.getName()
                    && group.type == consumable.getType()
                    && group.power == consumable.getPower()
                    && group.percentageBasedEffect == consumable.isPercentageBasedEffect())
                {
                    group.amount++;
                    found = true;
                    break;
                }
            }

            if (!found)
            {
                ConsumableInventoryStack group;
                group.firstIndex = i;
                group.amount = 1;
                group.name = consumable.getName();
                group.type = consumable.getType();
                group.power = consumable.getPower();
                group.percentageBasedEffect = consumable.isPercentageBasedEffect();
                groups.push_back(group);
            }
        }

        return groups;
    }

    void showInventoryScreen(
        const std::string& title,
        const std::string& screenId,
        const std::vector<std::string>& lines,
        bool waitAndClear = false
    )
    {
        if (!lines.empty())
        {
            MessageScreen::show(title, screenId, lines, waitAndClear);
        }
    }

    std::string formatRepairKitInventoryStatus(const Material& material)
    {
        if (isUsedRepairKitInventoryId(material.getId()))
        {
            return " x" + std::to_string(material.getQuantity())
                + " | dura "
                + std::to_string(usedRepairKitInventoryDurability(material.getId()))
                + "/"
                + std::to_string(maxRepairKitInventoryDurability(material.getId()));
        }

        return " x" + std::to_string(material.getQuantity())
            + " | intact "
            + std::to_string(maxRepairKitInventoryDurability(material.getId()))
            + "/"
            + std::to_string(maxRepairKitInventoryDurability(material.getId()));
    }

}


namespace
{
    bool containsInventoryPreferenceKey(const std::vector<std::string>& keys, const std::string& key)
    {
        return std::find(keys.begin(), keys.end(), key) != keys.end();
    }

    void setInventoryPreferenceKey(std::vector<std::string>& keys, const std::string& key, bool enabled)
    {
        if (key.empty()) return;
        const auto it = std::find(keys.begin(), keys.end(), key);
        if (enabled)
        {
            if (it == keys.end()) keys.push_back(key);
        }
        else if (it != keys.end())
        {
            keys.erase(it);
        }
    }

    std::string upperTrackedText(std::string value)
    {
        const std::pair<const char*, const char*> accentedUppercase[] = {
            {"é", "É"}, {"è", "È"}, {"ê", "Ê"}, {"ë", "Ë"},
            {"à", "À"}, {"â", "Â"}, {"ä", "Ä"}, {"ù", "Ù"},
            {"û", "Û"}, {"ü", "Ü"}, {"ô", "Ô"}, {"ö", "Ö"},
            {"î", "Î"}, {"ï", "Ï"}, {"ç", "Ç"}
        };

        for (const auto& [lowercase, uppercase] : accentedUppercase)
        {
            std::size_t position = 0;
            while ((position = value.find(lowercase, position)) != std::string::npos)
            {
                value.replace(position, std::char_traits<char>::length(lowercase), uppercase);
                position += std::char_traits<char>::length(uppercase);
            }
        }

        for (char& character : value)
        {
            const unsigned char c = static_cast<unsigned char>(character);
            if (c < 128) character = static_cast<char>(std::toupper(c));
        }
        return value;
    }
}

std::string Inventory::weaponItemKey(const Weapon& weapon)
{
    return "weapon:" + (weapon.getPersistentId().empty() ? weapon.getName() : weapon.getPersistentId());
}

std::string Inventory::armorItemKey(const Armor& armor)
{
    return "armor:" + (armor.getPersistentId().empty() ? armor.getName() : armor.getPersistentId());
}

std::string Inventory::consumableItemKey(const Consumable& consumable)
{
    return "consumable:" + consumable.getName();
}

std::string Inventory::materialItemKey(const Material& material)
{
    return materialItemKeyById(material.getId());
}

std::string Inventory::materialItemKeyById(const std::string& materialId)
{
    return "material:" + materialId;
}

bool Inventory::isPinnedItemKey(const std::string& key) const
{
    return containsInventoryPreferenceKey(pinnedItemKeys_, key);
}

bool Inventory::isFavoriteAmmunitionId(const std::string& materialId) const
{
    return containsInventoryPreferenceKey(favoriteAmmunitionIds_, materialId);
}

std::string Inventory::ammunitionFamilyForMaterialId(const std::string& materialId)
{
    if (materialId == "training_arrows" || materialId == "barbed_arrows" || materialId == "ash_arrows" || materialId == "venom_arrows") return "arrows";
    if (materialId == "training_bolts" || materialId == "piercing_bolts" || materialId == "frozen_bolts" || materialId == "shock_bolts") return "bolts";
    if (materialId == "training_throwing_knives" || materialId == "balanced_throwing_knives" || materialId == "conductive_knives" || materialId == "smoke_knives") return "throwing_knives";
    return "";
}

bool Inventory::isAmmunitionMaterialId(const std::string& materialId)
{
    return !ammunitionFamilyForMaterialId(materialId).empty();
}

bool Inventory::isLockedItemKey(const std::string& key) const
{
    return containsInventoryPreferenceKey(lockedItemKeys_, key);
}

bool Inventory::isTrackedMaterialId(const std::string& materialId) const
{
    return containsInventoryPreferenceKey(trackedMaterialIds_, materialId);
}

void Inventory::setPinnedItemKey(const std::string& key, bool enabled)
{
    setInventoryPreferenceKey(pinnedItemKeys_, key, enabled);
}

void Inventory::setFavoriteAmmunitionId(const std::string& materialId, bool enabled)
{
    const std::string family = ammunitionFamilyForMaterialId(materialId);
    if (family.empty()) return;

    if (enabled)
    {
        favoriteAmmunitionIds_.erase(
            std::remove_if(favoriteAmmunitionIds_.begin(), favoriteAmmunitionIds_.end(), [&](const std::string& existingId) {
                return ammunitionFamilyForMaterialId(existingId) == family && existingId != materialId;
            }),
            favoriteAmmunitionIds_.end()
        );
    }
    setInventoryPreferenceKey(favoriteAmmunitionIds_, materialId, enabled);
}

void Inventory::setLockedItemKey(const std::string& key, bool enabled)
{
    setInventoryPreferenceKey(lockedItemKeys_, key, enabled);
}

void Inventory::setTrackedMaterialId(const std::string& materialId, bool enabled)
{
    setInventoryPreferenceKey(trackedMaterialIds_, materialId, enabled);
}

void Inventory::togglePinnedItemKey(const std::string& key)
{
    setPinnedItemKey(key, !isPinnedItemKey(key));
}

void Inventory::toggleFavoriteAmmunitionId(const std::string& materialId)
{
    setFavoriteAmmunitionId(materialId, !isFavoriteAmmunitionId(materialId));
}

void Inventory::toggleLockedItemKey(const std::string& key)
{
    setLockedItemKey(key, !isLockedItemKey(key));
}

void Inventory::toggleTrackedMaterialId(const std::string& materialId)
{
    setTrackedMaterialId(materialId, !isTrackedMaterialId(materialId));
}

const std::vector<std::string>& Inventory::getPinnedItemKeys() const { return pinnedItemKeys_; }
const std::vector<std::string>& Inventory::getFavoriteAmmunitionIds() const { return favoriteAmmunitionIds_; }
const std::vector<std::string>& Inventory::getLockedItemKeys() const { return lockedItemKeys_; }
const std::vector<std::string>& Inventory::getTrackedMaterialIds() const { return trackedMaterialIds_; }

void Inventory::setLoadedInventoryPreferences(
    const std::vector<std::string>& pinnedKeys,
    const std::vector<std::string>& lockedKeys,
    const std::vector<std::string>& trackedMaterialIds,
    const std::vector<std::string>& favoriteAmmunitionIds)
{
    pinnedItemKeys_ = pinnedKeys;
    lockedItemKeys_ = lockedKeys;
    trackedMaterialIds_ = trackedMaterialIds;
    favoriteAmmunitionIds_.clear();
    for (const std::string& id : favoriteAmmunitionIds) setFavoriteAmmunitionId(id, true);
}

// EN: Inventory declares or implements a focused behavior used by this module.
// FR: Inventory déclare ou implémente un comportement précis utilisé par ce module.
Inventory::Inventory()
    : copperCoins_(0),
      ironCoins_(0),
      electrumCoins_(0),
      goldCoins_(0),
      platinumCoins_(0)
{
}

// EN: getGold declares or implements a focused behavior used by this module.
// FR: getGold déclare ou implémente un comportement précis utilisé par ce module.
int Inventory::getGold() const
{
    const long long gold = getTotalCopper() / Money::COPPER_PER_GOLD;
    if (gold > 2147483647LL)
    {
        return 2147483647;
    }
    return static_cast<int>(gold);
}

long long Inventory::getTotalCopper() const
{
    return std::max(0LL, copperCoins_)
        + std::max(0LL, ironCoins_) * static_cast<long long>(Money::COPPER_PER_IRON)
        + std::max(0LL, electrumCoins_) * static_cast<long long>(Money::COPPER_PER_IRON * Money::IRON_PER_ELECTRUM)
        + std::max(0LL, goldCoins_) * Money::COPPER_PER_GOLD
        + std::max(0LL, platinumCoins_) * Money::COPPER_PER_PLATINUM;
}

long long Inventory::getEconomyUnits() const
{
    return Money::economyUnitsFromCopper(getTotalCopper());
}

CoinBreakdown Inventory::getCoinStacks() const
{
    CoinBreakdown stacks;
    stacks.copper = std::max(0LL, copperCoins_);
    stacks.iron = std::max(0LL, ironCoins_);
    stacks.electrum = std::max(0LL, electrumCoins_);
    stacks.gold = std::max(0LL, goldCoins_);
    stacks.platinum = std::max(0LL, platinumCoins_);
    return stacks;
}

long long Inventory::getCoinCount(CoinType type) const
{
    switch (type)
    {
        case CoinType::Copper: return std::max(0LL, copperCoins_);
        case CoinType::Iron: return std::max(0LL, ironCoins_);
        case CoinType::Electrum: return std::max(0LL, electrumCoins_);
        case CoinType::Gold: return std::max(0LL, goldCoins_);
        case CoinType::Platinum: return std::max(0LL, platinumCoins_);
    }
    return 0;
}

bool Inventory::breakCoinsToLower(CoinType type, long long coinCount)
{
    if (coinCount <= 0 || !Money::hasLowerCoin(type) || getCoinCount(type) < coinCount)
    {
        return false;
    }

    CoinBreakdown stacks = getCoinStacks();
    long long* source = nullptr;
    long long* target = nullptr;
    switch (type)
    {
        case CoinType::Platinum: source = &stacks.platinum; target = &stacks.gold; break;
        case CoinType::Gold: source = &stacks.gold; target = &stacks.electrum; break;
        case CoinType::Electrum: source = &stacks.electrum; target = &stacks.iron; break;
        case CoinType::Iron: source = &stacks.iron; target = &stacks.copper; break;
        case CoinType::Copper: return false;
    }

    *source -= coinCount;
    *target += coinCount * 10;
    setCoinStacks(stacks);
    return true;
}

bool Inventory::combineCoinsToHigher(CoinType lowerType, long long higherCoinCount)
{
    if (higherCoinCount <= 0 || !Money::hasHigherCoin(lowerType))
    {
        return false;
    }

    const long long lowerNeeded = higherCoinCount * 10;
    if (getCoinCount(lowerType) < lowerNeeded)
    {
        return false;
    }

    CoinBreakdown stacks = getCoinStacks();
    long long* source = nullptr;
    long long* target = nullptr;
    switch (lowerType)
    {
        case CoinType::Copper: source = &stacks.copper; target = &stacks.iron; break;
        case CoinType::Iron: source = &stacks.iron; target = &stacks.electrum; break;
        case CoinType::Electrum: source = &stacks.electrum; target = &stacks.gold; break;
        case CoinType::Gold: source = &stacks.gold; target = &stacks.platinum; break;
        case CoinType::Platinum: return false;
    }

    *source -= lowerNeeded;
    *target += higherCoinCount;
    setCoinStacks(stacks);
    return true;
}

bool Inventory::convertCoinLots(CoinType sourceType, CoinType targetType, long long lotCount)
{
    if (sourceType == targetType || lotCount <= 0)
    {
        return false;
    }

    const long long sourceValue = Money::coinValueInCopper(sourceType);
    const long long targetValue = Money::coinValueInCopper(targetType);
    if (sourceValue <= 0 || targetValue <= 0)
    {
        return false;
    }

    const long long sourcePerLot = sourceValue < targetValue ? targetValue / sourceValue : 1;
    const long long targetPerLot = sourceValue > targetValue ? sourceValue / targetValue : 1;
    if (sourcePerLot <= 0 || targetPerLot <= 0 || getCoinCount(sourceType) < sourcePerLot * lotCount)
    {
        return false;
    }

    CoinBreakdown stacks = getCoinStacks();
    auto stackFor = [&stacks](CoinType type) -> long long* {
        switch (type)
        {
            case CoinType::Copper: return &stacks.copper;
            case CoinType::Iron: return &stacks.iron;
            case CoinType::Electrum: return &stacks.electrum;
            case CoinType::Gold: return &stacks.gold;
            case CoinType::Platinum: return &stacks.platinum;
        }
        return nullptr;
    };

    long long* source = stackFor(sourceType);
    long long* target = stackFor(targetType);
    if (source == nullptr || target == nullptr)
    {
        return false;
    }

    *source -= sourcePerLot * lotCount;
    *target += targetPerLot * lotCount;
    setCoinStacks(stacks);
    return true;
}

bool Inventory::compactCoinsToHighest()
{
    const long long before = getTotalCopper();
    setCoinStacks(Money::breakdownFromCopper(before));
    return getTotalCopper() == before;
}

bool Inventory::flattenCoinsToCopper()
{
    const long long before = getTotalCopper();
    CoinBreakdown stacks;
    stacks.copper = before;
    setCoinStacks(stacks);
    return getTotalCopper() == before;
}

// EN: setGold declares or implements a focused behavior used by this module.
// FR: setGold déclare ou implémente un comportement précis utilisé par ce module.
void Inventory::setGold(int amount)
{
    copperCoins_ = 0;
    ironCoins_ = 0;
    electrumCoins_ = 0;
    goldCoins_ = std::max(0, amount);
    platinumCoins_ = 0;
}

void Inventory::setTotalCopper(long long amount)
{
    const CoinBreakdown normalized = Money::breakdownFromCopper(std::max(0LL, amount));
    setCoinStacks(normalized);
}

void Inventory::setEconomyUnits(long long amount)
{
    copperCoins_ = 0;
    ironCoins_ = std::max(0LL, amount);
    electrumCoins_ = 0;
    goldCoins_ = 0;
    platinumCoins_ = 0;
}

void Inventory::setCoinStacks(const CoinBreakdown& stacks)
{
    copperCoins_ = std::max(0LL, stacks.copper);
    ironCoins_ = std::max(0LL, stacks.iron);
    electrumCoins_ = std::max(0LL, stacks.electrum);
    goldCoins_ = std::max(0LL, stacks.gold);
    platinumCoins_ = std::max(0LL, stacks.platinum);
}

// EN: earnGold declares or implements a focused behavior used by this module.
// FR: earnGold déclare ou implémente un comportement précis utilisé par ce module.
void Inventory::earnGold(int amount)
{
    if (amount > 0) goldCoins_ += amount;
}

void Inventory::earnCopper(long long amount)
{
    if (amount > 0) copperCoins_ += amount;
}

void Inventory::earnCoinStacks(const CoinBreakdown& stacks)
{
    copperCoins_ += std::max(0LL, stacks.copper);
    ironCoins_ += std::max(0LL, stacks.iron);
    electrumCoins_ += std::max(0LL, stacks.electrum);
    goldCoins_ += std::max(0LL, stacks.gold);
    platinumCoins_ += std::max(0LL, stacks.platinum);
}

void Inventory::earnEconomyUnits(long long amount)
{
    if (amount <= 0) return;

    // Authored rewards/prices still use the historical economy unit (1 PF = 10 PC),
    // but an actual payout is handed over in sensible physical denominations.
    // Existing stacks are never normalized: only the newly received amount is broken down.
    const CoinBreakdown payout = Money::breakdownFromCopper(Money::copperFromEconomyUnits(amount));
    copperCoins_ += payout.copper;
    ironCoins_ += payout.iron;
    electrumCoins_ += payout.electrum;
    goldCoins_ += payout.gold;
    platinumCoins_ += payout.platinum;
}

// EN: spendGold declares or implements a focused behavior used by this module.
// FR: spendGold déclare ou implémente un comportement précis utilisé par ce module.
bool Inventory::spendGold(int amount)
{
    return spendCopper(Money::copperFromGold(amount));
}

bool Inventory::spendCopper(long long amount)
{
    if (amount <= 0)
    {
        return true;
    }

    if (getTotalCopper() < amount)
    {
        return false;
    }

    struct StackRef
    {
        long long value;
        long long* count;
    };

    StackRef stacks[] = {
        {Money::COPPER_PER_PLATINUM, &platinumCoins_},
        {Money::COPPER_PER_GOLD, &goldCoins_},
        {static_cast<long long>(Money::COPPER_PER_IRON) * Money::IRON_PER_ELECTRUM, &electrumCoins_},
        {Money::COPPER_PER_IRON, &ironCoins_},
        {1, &copperCoins_}
    };

    long long remaining = amount;

    // Spend existing exact/lower denominations first without touching unrelated stacks.
    for (StackRef& stack : stacks)
    {
        if (remaining <= 0) break;
        if (stack.value > remaining || *stack.count <= 0) continue;

        const long long take = std::min(*stack.count, remaining / stack.value);
        *stack.count -= take;
        remaining -= take * stack.value;
    }

    if (remaining <= 0)
    {
        return true;
    }

    // No exact combination was available. Break the smallest larger physical coin and
    // receive ordinary change in lower denominations, like a merchant making change.
    StackRef* coinToBreak = nullptr;
    for (auto it = std::rbegin(stacks); it != std::rend(stacks); ++it)
    {
        if (it->value > remaining && *it->count > 0)
        {
            coinToBreak = &(*it);
            break;
        }
    }

    if (coinToBreak == nullptr)
    {
        // Should be unreachable after the total-value guard, but never lose money silently.
        setTotalCopper(getTotalCopper());
        return false;
    }

    (*coinToBreak->count)--;
    long long change = coinToBreak->value - remaining;
    const CoinBreakdown changeCoins = Money::breakdownFromCopper(change);
    copperCoins_ += changeCoins.copper;
    ironCoins_ += changeCoins.iron;
    electrumCoins_ += changeCoins.electrum;
    goldCoins_ += changeCoins.gold;
    platinumCoins_ += changeCoins.platinum;
    return true;
}

bool Inventory::spendEconomyUnits(long long amount)
{
    return spendCopper(Money::copperFromEconomyUnits(amount));
}

std::string Inventory::getWalletLine() const
{
    return Money::formatCoinStacks(getCoinStacks(), true);
}

std::string Inventory::getWalletTotalLine() const
{
    return Money::formatWalletTotalFromCopper(getTotalCopper());
}

// EN: getWeaponCount declares or implements a focused behavior used by this module.
// FR: getWeaponCount déclare ou implémente un comportement précis utilisé par ce module.
int Inventory::getWeaponCount() const
{
    return static_cast<int>(weapons.size());
}

// EN: getArmorCount declares or implements a focused behavior used by this module.
// FR: getArmorCount déclare ou implémente un comportement précis utilisé par ce module.
int Inventory::getArmorCount() const
{
    return static_cast<int>(armors.size());
}

// EN: getConsumableCount declares or implements a focused behavior used by this module.
// FR: getConsumableCount déclare ou implémente un comportement précis utilisé par ce module.
int Inventory::getConsumableCount() const
{
    return static_cast<int>(consumables.size());
}

// EN: getMaterialCount declares or implements a focused behavior used by this module.
// FR: getMaterialCount déclare ou implémente un comportement précis utilisé par ce module.
int Inventory::getMaterialCount() const
{
    int total = 0;

    for (const Material& material : materials)
    {
        total += material.getQuantity();
    }

    return total;
}

// EN: countConsumables declares or implements a focused behavior used by this module.
// FR: countConsumables déclare ou implémente un comportement précis utilisé par ce module.
int Inventory::countConsumables(ConsumableType type) const
{
    int total = 0;

    for (const Consumable& consumable : consumables)
    {
        if (consumable.getType() == type)
        {
            total++;
        }
    }

    return total;
}

// EN: findMaterialIndexById declares or implements a focused behavior used by this module.
// FR: findMaterialIndexById déclare ou implémente un comportement précis utilisé par ce module.
int Inventory::findMaterialIndexById(const std::string& id) const
{
    for (int i = 0; i < static_cast<int>(materials.size()); ++i)
    {
        if (materials[i].getId() == id)
        {
            return i;
        }
    }

    return -1;
}

// EN: countMaterialById declares or implements a focused behavior used by this module.
// FR: countMaterialById déclare ou implémente un comportement précis utilisé par ce module.
int Inventory::countMaterialById(const std::string& id) const
{
    int total = 0;

    for (const Material& material : materials)
    {
        if (material.getId() == id)
        {
            total += material.getQuantity();
        }
    }

    return total;
}

int Inventory::countMaterialByIdAndQuality(const std::string& id, const std::string& quality) const
{
    int total = 0;
    for (const Material& material : materials)
    {
        if (material.getId() == id && material.getQuality() == quality)
        {
            total += material.getQuantity();
        }
    }
    return total;
}

// EN: countMaterialQualityPointsById declares or implements a focused behavior used by this module.
// FR: countMaterialQualityPointsById déclare ou implémente un comportement précis utilisé par ce module.
int Inventory::countMaterialQualityPointsById(const std::string& id) const
{
    int total = 0;

    for (const Material& material : materials)
    {
        if (material.getId() == id)
        {
            total += material.getQuantity() * material.getQualityCraftWeight();
        }
    }

    return total;
}

// EN: getWeapons declares or implements a focused behavior used by this module.
// FR: getWeapons déclare ou implémente un comportement précis utilisé par ce module.
const std::vector<Weapon>& Inventory::getWeapons() const
{
    return weapons;
}

// EN: getArmors declares or implements a focused behavior used by this module.
// FR: getArmors déclare ou implémente un comportement précis utilisé par ce module.
const std::vector<Armor>& Inventory::getArmors() const
{
    return armors;
}

// EN: getConsumables declares or implements a focused behavior used by this module.
// FR: getConsumables déclare ou implémente un comportement précis utilisé par ce module.
const std::vector<Consumable>& Inventory::getConsumables() const
{
    return consumables;
}

// EN: getMaterials declares or implements a focused behavior used by this module.
// FR: getMaterials déclare ou implémente un comportement précis utilisé par ce module.
const std::vector<Material>& Inventory::getMaterials() const
{
    return materials;
}

// EN: addWeapon declares or implements a focused behavior used by this module.
// FR: addWeapon déclare ou implémente un comportement précis utilisé par ce module.
void Inventory::addWeapon(const Weapon& weapon)
{
    Weapon stored = weapon;
    stored.ensurePersistentId("weapon");
    weapons.push_back(stored);
}

// EN: addArmor declares or implements a focused behavior used by this module.
// FR: addArmor déclare ou implémente un comportement précis utilisé par ce module.
void Inventory::addArmor(const Armor& armor)
{
    Armor stored = armor;
    stored.ensurePersistentId("armor");
    armors.push_back(stored);
}

// EN: addConsumable declares or implements a focused behavior used by this module.
// FR: addConsumable déclare ou implémente un comportement précis utilisé par ce module.
void Inventory::addConsumable(const Consumable& consumable)
{
    Consumable stored = consumable;
    stored.ensurePersistentId("consumable");
    consumables.push_back(stored);
}

// EN: addMaterial declares or implements a focused behavior used by this module.
// FR: addMaterial déclare ou implémente un comportement précis utilisé par ce module.
void Inventory::addMaterialInternal(const Material& material, bool announceTrackedAcquisition, bool recordDiscovery)
{
    if (material.getQuantity() <= 0)
    {
        return;
    }

    if (announceTrackedAcquisition && isTrackedMaterialId(material.getId()))
    {
        std::vector<std::string> acquisitionLines = {
            "OBJET SUIVI OBTENU : " + upperTrackedText(material.getName())
                + " X" + std::to_string(material.getQuantity())
        };
        if (material.hasSpecialQuality())
        {
            acquisitionLines.push_back("QUALITE : " + upperTrackedText(material.getQualityLabel()));
        }
        MessageScreen::show(
            "OBJET SUIVI OBTENU",
            "inventory.tracked_material.acquired",
            acquisitionLines,
            false
        );
    }

    if (recordDiscovery)
    {
        MaterialKnowledgeProgress::recordDiscovery(material);
    }

    for (Material& existingMaterial : materials)
    {
        if (existingMaterial.getId() == material.getId()
            && existingMaterial.getQuality() == material.getQuality()
            && existingMaterial.getOwnershipBinding() == material.getOwnershipBinding()
            && existingMaterial.hasSameRecoveryMetadata(material))
        {
            existingMaterial.addQuantity(material.getQuantity());
            return;
        }
    }

    materials.push_back(material);
}

void Inventory::addMaterial(const Material& material)
{
    addMaterialInternal(material, true, true);
}

// EN: hasWeapon declares or implements a focused behavior used by this module.
// FR: hasWeapon déclare ou implémente un comportement précis utilisé par ce module.
bool Inventory::hasWeapon(int index) const
{
    return index >= 0 && index < static_cast<int>(weapons.size());
}

// EN: hasArmor declares or implements a focused behavior used by this module.
// FR: hasArmor déclare ou implémente un comportement précis utilisé par ce module.
bool Inventory::hasArmor(int index) const
{
    return index >= 0 && index < static_cast<int>(armors.size());
}

// EN: hasConsumable declares or implements a focused behavior used by this module.
// FR: hasConsumable déclare ou implémente un comportement précis utilisé par ce module.
bool Inventory::hasConsumable(int index) const
{
    return index >= 0 && index < static_cast<int>(consumables.size());
}

// EN: hasMaterial declares or implements a focused behavior used by this module.
// FR: hasMaterial déclare ou implémente un comportement précis utilisé par ce module.
bool Inventory::hasMaterial(int index) const
{
    return index >= 0 && index < static_cast<int>(materials.size());
}

// EN: getWeapon declares or implements a focused behavior used by this module.
// FR: getWeapon déclare ou implémente un comportement précis utilisé par ce module.
Weapon Inventory::getWeapon(int index) const
{
    if (!hasWeapon(index))
    {
        return Weapon();
    }

    return weapons[index];
}

// EN: getMutableWeapon declares or implements a focused behavior used by this module.
// FR: getMutableWeapon déclare ou implémente un comportement précis utilisé par ce module.
Weapon* Inventory::getMutableWeapon(int index)
{
    if (!hasWeapon(index))
    {
        return nullptr;
    }

    return &weapons[index];
}

// EN: getArmor declares or implements a focused behavior used by this module.
// FR: getArmor déclare ou implémente un comportement précis utilisé par ce module.
Armor Inventory::getArmor(int index) const
{
    if (!hasArmor(index))
    {
        return Armor();
    }

    return armors[index];
}

// EN: getMutableArmor declares or implements a focused behavior used by this module.
// FR: getMutableArmor déclare ou implémente un comportement précis utilisé par ce module.
Armor* Inventory::getMutableArmor(int index)
{
    if (!hasArmor(index))
    {
        return nullptr;
    }

    return &armors[index];
}

// EN: getConsumable declares or implements a focused behavior used by this module.
// FR: getConsumable déclare ou implémente un comportement précis utilisé par ce module.
Consumable Inventory::getConsumable(int index) const
{
    if (!hasConsumable(index))
    {
        return Consumable();
    }

    return consumables[index];
}

// EN: getMaterial declares or implements a focused behavior used by this module.
// FR: getMaterial déclare ou implémente un comportement précis utilisé par ce module.
Material Inventory::getMaterial(int index) const
{
    if (!hasMaterial(index))
    {
        return Material();
    }

    return materials[index];
}

// EN: getMutableMaterial declares or implements a focused behavior used by this module.
// FR: getMutableMaterial déclare ou implémente un comportement précis utilisé par ce module.
Material* Inventory::getMutableMaterial(int index)
{
    if (!hasMaterial(index))
    {
        return nullptr;
    }

    return &materials[index];
}

// EN: findFirstConsumable declares or implements a focused behavior used by this module.
// FR: findFirstConsumable déclare ou implémente un comportement précis utilisé par ce module.
int Inventory::findFirstConsumable(ConsumableType type) const
{
    for (int i = 0; i < static_cast<int>(consumables.size()); i++)
    {
        if (consumables[i].getType() == type)
        {
            return i;
        }
    }

    return -1;
}

// EN: useFirstConsumable declares or implements a focused behavior used by this module.
// FR: useFirstConsumable déclare ou implémente un comportement précis utilisé par ce module.
bool Inventory::useFirstConsumable(ConsumableType type, Consumable& usedConsumable)
{
    int index = findFirstConsumable(type);

    if (index == -1)
    {
        return false;
    }

    usedConsumable = consumables[index];
    consumables.erase(consumables.begin() + index);

    return true;
}

// EN: removeWeapon declares or implements a focused behavior used by this module.
// FR: removeWeapon déclare ou implémente un comportement précis utilisé par ce module.
bool Inventory::removeWeapon(int index)
{
    if (!hasWeapon(index))
    {
        return false;
    }

    weapons.erase(weapons.begin() + index);
    return true;
}

// EN: removeArmor declares or implements a focused behavior used by this module.
// FR: removeArmor déclare ou implémente un comportement précis utilisé par ce module.
bool Inventory::removeArmor(int index)
{
    if (!hasArmor(index))
    {
        return false;
    }

    armors.erase(armors.begin() + index);
    return true;
}

// EN: removeConsumable declares or implements a focused behavior used by this module.
// FR: removeConsumable déclare ou implémente un comportement précis utilisé par ce module.
bool Inventory::removeConsumable(int index)
{
    if (!hasConsumable(index))
    {
        return false;
    }

    consumables.erase(consumables.begin() + index);
    return true;
}

// EN: removeMaterialQuantity declares or implements a focused behavior used by this module.
// FR: removeMaterialQuantity déclare ou implémente un comportement précis utilisé par ce module.
bool Inventory::removeMaterialQuantity(int index, int quantity)
{
    if (!hasMaterial(index) || quantity <= 0)
    {
        return false;
    }

    if (!materials[index].removeQuantity(quantity))
    {
        return false;
    }

    if (materials[index].getQuantity() <= 0)
    {
        materials.erase(materials.begin() + index);
    }

    return true;
}

// EN: degradeMaterialQualityById consumes magical/workshop quality before consuming the physical unit itself.
// FR: degradeMaterialQualityById use d'abord la qualité d'un matériau avant de dissoudre l'unité physique.
bool Inventory::degradeMaterialQualityById(const std::string& id, std::string& previousQuality, std::string& resultingQuality, bool& dissolved)
{
    previousQuality.clear();
    resultingQuality.clear();
    dissolved = false;

    int selectedIndex = -1;
    int selectedWeight = 999;
    for (int i = 0; i < static_cast<int>(materials.size()); ++i)
    {
        if (materials[i].getId() != id || materials[i].getQuantity() <= 0)
        {
            continue;
        }

        const int weight = materials[i].getQualityCraftWeight();
        if (weight < selectedWeight)
        {
            selectedWeight = weight;
            selectedIndex = i;
        }
    }

    if (selectedIndex < 0)
    {
        return false;
    }

    Material source = materials[selectedIndex];
    previousQuality = source.getQuality();

    if (previousQuality == "impure" || previousQuality == "low")
    {
        resultingQuality = "dissolved";
        dissolved = true;
    }
    else if (previousQuality == "normal")
    {
        resultingQuality = "impure";
    }
    else if (previousQuality == "exceptional")
    {
        resultingQuality = "pure";
    }
    else
    {
        // pure / high and any future high-grade synonym fall back to normal before becoming impure.
        resultingQuality = "normal";
    }

    materials[selectedIndex].removeQuantity(1);
    if (materials[selectedIndex].getQuantity() <= 0)
    {
        materials.erase(materials.begin() + selectedIndex);
    }

    if (!dissolved)
    {
        source.setQuantity(1);
        source.setQuality(resultingQuality);
        // This is the same physical unit losing quality, not a new acquisition.
        // A tracked-material banner here would falsely announce an item the player already owned.
        addMaterialInternal(source, false, false);
    }

    return true;
}

// EN: degrades one physical unit from the exact quality stack selected by the player.
// FR: dégrade une unité physique de la qualité exacte choisie par le joueur.
bool Inventory::degradeMaterialQualityByIdAndQuality(const std::string& id, const std::string& quality, std::string& previousQuality, std::string& resultingQuality, bool& dissolved)
{
    previousQuality.clear();
    resultingQuality.clear();
    dissolved = false;

    int selectedIndex = -1;
    for (int i = 0; i < static_cast<int>(materials.size()); ++i)
    {
        if (materials[i].getId() == id && materials[i].getQuality() == quality && materials[i].getQuantity() > 0)
        {
            selectedIndex = i;
            break;
        }
    }
    if (selectedIndex < 0)
    {
        return false;
    }

    Material source = materials[selectedIndex];
    previousQuality = source.getQuality();
    if (previousQuality == "impure" || previousQuality == "low")
    {
        resultingQuality = "dissolved";
        dissolved = true;
    }
    else if (previousQuality == "normal")
    {
        resultingQuality = "impure";
    }
    else if (previousQuality == "exceptional")
    {
        resultingQuality = "pure";
    }
    else
    {
        resultingQuality = "normal";
    }

    materials[selectedIndex].removeQuantity(1);
    if (materials[selectedIndex].getQuantity() <= 0)
    {
        materials.erase(materials.begin() + selectedIndex);
    }
    if (!dissolved)
    {
        source.setQuantity(1);
        source.setQuality(resultingQuality);
        // Quality degradation must not trigger the tracked-acquisition banner.
        addMaterialInternal(source, false, false);
    }
    return true;
}

// EN: removeMaterialQuantityById declares or implements a focused behavior used by this module.
// FR: removeMaterialQuantityById déclare ou implémente un comportement précis utilisé par ce module.
bool Inventory::removeMaterialQuantityById(const std::string& id, int quantity)
{
    if (quantity <= 0)
    {
        return true;
    }

    if (countMaterialById(id) < quantity)
    {
        return false;
    }

    int remaining = quantity;

    for (int pass = 0; pass < 4 && remaining > 0; ++pass)
    {
        for (int i = 0; i < static_cast<int>(materials.size()) && remaining > 0; ++i)
        {
            if (materials[i].getId() != id)
            {
                continue;
            }

            bool passMatches = (pass == 0 && materials[i].getQualityCraftWeight() == 1)
                || (pass == 1 && materials[i].getQualityCraftWeight() == 2)
                || (pass == 2 && materials[i].getQualityCraftWeight() == 3)
                || (pass == 3 && materials[i].getQualityCraftWeight() == 4);

            if (!passMatches)
            {
                continue;
            }

            int removed = std::min(materials[i].getQuantity(), remaining);
            materials[i].removeQuantity(removed);
            remaining -= removed;

            if (materials[i].getQuantity() <= 0)
            {
                materials.erase(materials.begin() + i);
                --i;
            }
        }
    }

    return remaining <= 0;
}

// EN: removes only from a material stack with the exact selected quality.
// FR: retire uniquement une pile de matériau de la qualité exacte choisie.
bool Inventory::removeMaterialQuantityByIdAndQuality(const std::string& id, const std::string& quality, int quantity)
{
    if (quantity <= 0)
    {
        return true;
    }
    if (countMaterialByIdAndQuality(id, quality) < quantity)
    {
        return false;
    }

    int remaining = quantity;
    for (int i = 0; i < static_cast<int>(materials.size()) && remaining > 0;)
    {
        if (materials[i].getId() != id || materials[i].getQuality() != quality)
        {
            ++i;
            continue;
        }
        const int take = std::min(remaining, materials[i].getQuantity());
        materials[i].removeQuantity(take);
        remaining -= take;
        if (materials[i].getQuantity() <= 0)
        {
            materials.erase(materials.begin() + i);
        }
        else
        {
            ++i;
        }
    }
    return remaining == 0;
}

// EN: removeMaterialQuantityByIdFlexible declares or implements a focused behavior used by this module.
// FR: removeMaterialQuantityByIdFlexible déclare ou implémente un comportement précis utilisé par ce module.
bool Inventory::removeMaterialQuantityByIdFlexible(const std::string& id, int normalQualityQuantity)
{
    if (normalQualityQuantity <= 0)
    {
        return true;
    }

    int requiredPoints = normalQualityQuantity * 2;

    if (countMaterialQualityPointsById(id) < requiredPoints)
    {
        return false;
    }

    for (int pass = 0; pass < 4 && requiredPoints > 0; ++pass)
    {
        for (int i = 0; i < static_cast<int>(materials.size()) && requiredPoints > 0; ++i)
        {
            if (materials[i].getId() != id)
            {
                continue;
            }

            bool passMatches = (pass == 0 && materials[i].getQualityCraftWeight() == 1)
                || (pass == 1 && materials[i].getQualityCraftWeight() == 2)
                || (pass == 2 && materials[i].getQualityCraftWeight() == 3)
                || (pass == 3 && materials[i].getQualityCraftWeight() == 4);

            if (!passMatches)
            {
                continue;
            }

            int weight = materials[i].getQualityCraftWeight();
            int toRemove = std::min(materials[i].getQuantity(), (requiredPoints + weight - 1) / weight);
            materials[i].removeQuantity(toRemove);
            requiredPoints -= toRemove * weight;

            if (materials[i].getQuantity() <= 0)
            {
                materials.erase(materials.begin() + i);
                --i;
            }
        }
    }

    return true;
}

// EN: clearAll declares or implements a focused behavior used by this module.
// FR: clearAll déclare ou implémente un comportement précis utilisé par ce module.
void Inventory::clearAll()
{
    weapons.clear();
    armors.clear();
    consumables.clear();
    materials.clear();
    copperCoins_ = 0;
    ironCoins_ = 0;
    electrumCoins_ = 0;
    goldCoins_ = 0;
    platinumCoins_ = 0;
}

// EN: displayWeaponList declares or implements a focused behavior used by this module.
// FR: displayWeaponList déclare ou implémente un comportement précis utilisé par ce module.
void Inventory::displayWeaponList() const
{
    std::vector<std::string> lines;

    if (weapons.empty())
    {
        lines.push_back("Aucune arme dans l'inventaire.");
        showInventoryScreen("ARMES", "inventory.weapons", lines, false);
        return;
    }

    std::vector<int> orderedWeaponIndices;
    for (int i = 0; i < static_cast<int>(weapons.size()); ++i) orderedWeaponIndices.push_back(i);
    std::stable_sort(orderedWeaponIndices.begin(), orderedWeaponIndices.end(), [&](int left, int right) {
        return isPinnedItemKey(weaponItemKey(weapons[static_cast<std::size_t>(left)]))
            && !isPinnedItemKey(weaponItemKey(weapons[static_cast<std::size_t>(right)]));
    });

    for (int i : orderedWeaponIndices)
    {
        const Weapon& weapon = weapons[static_cast<std::size_t>(i)];
        const std::string key = weaponItemKey(weapon);
        std::string line = "[" + std::to_string(i) + "] ";
        if (isPinnedItemKey(key)) line += "[ÉPINGLÉ] ";
        if (isLockedItemKey(key)) line += "[VERROUILLÉ] ";
        line += weapon.getName();

        if (!weapon.isIndestructible())
        {
            line += " (" + std::to_string(weapon.getDurability()) + "/" + std::to_string(weapon.getMaxDurability()) + ")";
        }

        if (weapon.isBroken())
        {
            line += " - Cassée";
        }

        lines.push_back(line);
    }

    showInventoryScreen("ARMES", "inventory.weapons", lines, false);
}


// EN: displayArmorList declares or implements a focused behavior used by this module.
// FR: displayArmorList déclare ou implémente un comportement précis utilisé par ce module.
void Inventory::displayArmorList() const
{
    std::vector<std::string> lines;

    if (armors.empty())
    {
        lines.push_back("Aucune armure dans l'inventaire.");
        showInventoryScreen("ARMURES", "inventory.armors", lines, false);
        return;
    }

    std::vector<int> orderedArmorIndices;
    for (int i = 0; i < static_cast<int>(armors.size()); ++i) orderedArmorIndices.push_back(i);
    std::stable_sort(orderedArmorIndices.begin(), orderedArmorIndices.end(), [&](int left, int right) {
        return isPinnedItemKey(armorItemKey(armors[static_cast<std::size_t>(left)]))
            && !isPinnedItemKey(armorItemKey(armors[static_cast<std::size_t>(right)]));
    });

    for (int i : orderedArmorIndices)
    {
        const Armor& armor = armors[static_cast<std::size_t>(i)];
        const std::string key = armorItemKey(armor);
        std::string line = "[" + std::to_string(i) + "] ";
        if (isPinnedItemKey(key)) line += "[ÉPINGLÉ] ";
        if (isLockedItemKey(key)) line += "[VERROUILLÉ] ";
        line += armor.getName();

        if (!armor.isIndestructible())
        {
            line += " (" + std::to_string(armor.getDurability()) + "/" + std::to_string(armor.getMaxDurability()) + ")";
        }

        if (armor.isBroken())
        {
            line += " - Cassée";
        }

        lines.push_back(line);
    }

    showInventoryScreen("ARMURES", "inventory.armors", lines, false);
}


// EN: displayConsumableList declares or implements a focused behavior used by this module.
// FR: displayConsumableList déclare ou implémente un comportement précis utilisé par ce module.
void Inventory::displayConsumableList() const
{
    std::vector<std::string> lines;
    bool hasRepairKit = false;

    for (const Material& material : materials)
    {
        if (material.getCategory() == "Outil" && isRepairKitInventoryId(material.getId()))
        {
            hasRepairKit = true;
            break;
        }
    }

    if (consumables.empty() && !hasRepairKit)
    {
        lines.push_back("Aucun consommable dans l'inventaire.");
        showInventoryScreen("CONSOMMABLES", "inventory.consumables", lines, false);
        return;
    }

    std::vector<ConsumableInventoryStack> consumableGroups = groupConsumablesForInventory(consumables);
    std::stable_sort(consumableGroups.begin(), consumableGroups.end(), [&](const ConsumableInventoryStack& left, const ConsumableInventoryStack& right) {
        const bool leftFavorite = isPinnedItemKey(consumableItemKey(consumables[static_cast<std::size_t>(left.firstIndex)]));
        const bool rightFavorite = isPinnedItemKey(consumableItemKey(consumables[static_cast<std::size_t>(right.firstIndex)]));
        return leftFavorite && !rightFavorite;
    });

    for (int i = 0; i < static_cast<int>(consumableGroups.size()); i++)
    {
        const ConsumableInventoryStack& group = consumableGroups[i];
        const std::string key = consumableItemKey(consumables[static_cast<std::size_t>(group.firstIndex)]);
        std::string prefix;
        if (isPinnedItemKey(key)) prefix += "[ÉPINGLÉ] ";
        if (isLockedItemKey(key)) prefix += "[VERROUILLÉ] ";
        lines.push_back(
            "[" + std::to_string(group.firstIndex) + "] " + prefix
            + formatConsumableInventoryStackLabel(group.name, group.amount)
            + " | Puissance : " + std::to_string(group.power)
        );
    }

    if (hasRepairKit)
    {
        if (!lines.empty())
        {
            lines.push_back("");
        }

        lines.push_back("Kits de réparation :");

        for (const Material& material : materials)
        {
            if (material.getCategory() != "Outil" || !isRepairKitInventoryId(material.getId()))
            {
                continue;
            }

            lines.push_back("- " + material.getName() + formatRepairKitInventoryStatus(material));
        }

        lines.push_back("Astuce : sélectionne une arme ou une armure pour lancer une réparation.");
    }

    showInventoryScreen("CONSOMMABLES", "inventory.consumables", lines, false);
}


// EN: displayMaterialList declares or implements a focused behavior used by this module.
// FR: displayMaterialList déclare ou implémente un comportement précis utilisé par ce module.
void Inventory::displayMaterialList() const
{
    std::vector<std::string> lines;

    if (materials.empty())
    {
        lines.push_back("Aucun matériau, plante ou renseignement stocké.");
        showInventoryScreen("MATÉRIAUX / PLANTES / INFOS", "inventory.materials", lines, false);
        return;
    }

    std::vector<int> orderedMaterialIndices;
    for (int i = 0; i < static_cast<int>(materials.size()); ++i) orderedMaterialIndices.push_back(i);
    std::stable_sort(orderedMaterialIndices.begin(), orderedMaterialIndices.end(), [&](int left, int right) {
        return isPinnedItemKey(materialItemKey(materials[static_cast<std::size_t>(left)]))
            && !isPinnedItemKey(materialItemKey(materials[static_cast<std::size_t>(right)]));
    });

    for (int i : orderedMaterialIndices)
    {
        const Material& material = materials[static_cast<std::size_t>(i)];
        const std::string key = materialItemKey(material);
        std::string line = "[" + std::to_string(i) + "] ";
        if (isPinnedItemKey(key)) line += "[ÉPINGLÉ] ";
        if (isLockedItemKey(key)) line += "[VERROUILLÉ] ";
        if (isTrackedMaterialId(material.getId())) line += "[SUIVI] ";
        line += material.getName();

        if (material.getCategory() == "Outil" && isRepairKitInventoryId(material.getId()))
        {
            line += formatRepairKitInventoryStatus(material);
        }
        else
        {
            line += " x" + std::to_string(material.getQuantity());
            if (material.hasSpecialQuality())
            {
                line += " | " + material.getQualityLabel();
            }
        }

        line += " | " + material.getCategory();
        lines.push_back(line);
    }

    showInventoryScreen("MATÉRIAUX / PLANTES / INFOS", "inventory.materials", lines, false);
}


// EN: displaySummary declares or implements a focused behavior used by this module.
// FR: displaySummary déclare ou implémente un comportement précis utilisé par ce module.
void Inventory::displaySummary() const
{
    showInventoryScreen(
        "INVENTAIRE",
        "inventory.summary",
        {
            "Argent séparé : " + getWalletLine(),
            "Argent total : " + getWalletTotalLine(),
            "Armes : " + std::to_string(getWeaponCount()),
            "Armures : " + std::to_string(getArmorCount()),
            "Consommables : " + std::to_string(getConsumableCount()),
            "Matériaux / plantes / infos : " + std::to_string(getMaterialCount()),
            "Potions de soin : " + std::to_string(countConsumables(ConsumableType::Healing)),
            "Potions de rage : " + std::to_string(countConsumables(ConsumableType::Damage))
        },
        false
    );
}


// EN: inspectWeapon declares or implements a focused behavior used by this module.
// FR: inspectWeapon déclare ou implémente un comportement précis utilisé par ce module.
void Inventory::inspectWeapon(int index) const
{
    if (!hasWeapon(index))
    {
        MessageScreen::show("ARME INTROUVABLE", "inventory.weapon.invalid", {"Cette arme n'existe pas."}, false);
        return;
    }

    const Weapon& weapon = weapons[index];
    std::vector<std::string> lines;
    lines.push_back("Nom : " + weapon.getName());
    lines.push_back("Description : " + weapon.getDescription());
    lines.push_back(
        "Bonus dégâts : +"
        + std::to_string(weapon.getMinDamageBonus())
        + " à +"
        + std::to_string(weapon.getMaxDamageBonus())
    );
    lines.push_back("Bonus critique : +" + std::to_string(weapon.getCriticalBonus()));

    if (weapon.isIndestructible())
    {
        lines.push_back("Durabilité : Indestructible");
    }
    else
    {
        lines.push_back("Durabilité : " + std::to_string(weapon.getDurability()) + "/" + std::to_string(weapon.getMaxDurability()));
    }

    lines.push_back(std::string("État : ") + (weapon.isBroken() ? "Cassée" : "Utilisable"));
    lines.push_back("Valeur : " + Money::formatEconomyUnits(weapon.getValue()));
    showInventoryScreen("ARME", "inventory.weapon.inspect", lines, false);
}


// EN: inspectArmor declares or implements a focused behavior used by this module.
// FR: inspectArmor déclare ou implémente un comportement précis utilisé par ce module.
void Inventory::inspectArmor(int index) const
{
    if (!hasArmor(index))
    {
        MessageScreen::show("ARMURE INTROUVABLE", "inventory.armor.invalid", {"Cette armure n'existe pas."}, false);
        return;
    }

    const Armor& armor = armors[index];
    std::vector<std::string> lines;
    lines.push_back("Nom : " + armor.getName());
    lines.push_back("Description : " + armor.getDescription());
    lines.push_back("Bonus PV max : +" + std::to_string(armor.getMaxHpBonus()));
    lines.push_back("Réduction dégâts : " + std::to_string(armor.getDamageReduction()));

    if (armor.isIndestructible())
    {
        lines.push_back("Durabilité : Indestructible");
    }
    else
    {
        lines.push_back("Durabilité : " + std::to_string(armor.getDurability()) + "/" + std::to_string(armor.getMaxDurability()));
    }

    lines.push_back(std::string("État : ") + (armor.isBroken() ? "Cassée" : "Utilisable"));
    lines.push_back("Valeur : " + Money::formatEconomyUnits(armor.getValue()));
    showInventoryScreen("ARMURE", "inventory.armor.inspect", lines, false);
}


// EN: inspectConsumable declares or implements a focused behavior used by this module.
// FR: inspectConsumable déclare ou implémente un comportement précis utilisé par ce module.
void Inventory::inspectConsumable(int index) const
{
    if (!hasConsumable(index))
    {
        MessageScreen::show("CONSOMMABLE INTROUVABLE", "inventory.consumable.invalid", {"Ce consommable n'existe pas."}, false);
        return;
    }

    const Consumable& consumable = consumables[index];
    showInventoryScreen(
        "CONSOMMABLE",
        "inventory.consumable.inspect",
        {
            "Nom : " + consumable.getName(),
            "Description : " + consumable.getDescription(),
            "Puissance : " + consumable.getPowerDisplayText(),
            "Valeur : " + Money::formatEconomyUnits(consumable.getValue())
        },
        false
    );
}


// EN: inspectMaterial declares or implements a focused behavior used by this module.
// FR: inspectMaterial déclare ou implémente un comportement précis utilisé par ce module.
void Inventory::inspectMaterial(int index) const
{
    if (!hasMaterial(index))
    {
        MessageScreen::show("ENTRÉE INTROUVABLE", "inventory.material.invalid", {"Cette entrée n'existe pas."}, false);
        return;
    }

    const Material& material = materials[index];
    std::vector<std::string> lines;
    lines.push_back("Nom : " + material.getName());
    lines.push_back("Catégorie : " + material.getCategory());

    if (material.getCategory() != "Livre" && material.getCategory() != "Renseignement" && material.getCategory() != "Outil")
    {
        lines.push_back("Qualité : " + material.getQualityLabel());
    }

    if (material.getCategory() == "Outil" && isRepairKitInventoryId(material.getId()))
    {
        if (isUsedRepairKitInventoryId(material.getId()))
        {
            lines.push_back("Nombre : " + std::to_string(material.getQuantity()) + " kit(s) entamé(s)");
            lines.push_back(
                "Durabilité par kit : "
                + std::to_string(usedRepairKitInventoryDurability(material.getId()))
                + "/"
                + std::to_string(maxRepairKitInventoryDurability(material.getId()))
            );
        }
        else
        {
            lines.push_back("Nombre : " + std::to_string(material.getQuantity()) + " kit(s) intact(s)");
            lines.push_back(
                "Durabilité par kit : "
                + std::to_string(maxRepairKitInventoryDurability(material.getId()))
                + "/"
                + std::to_string(maxRepairKitInventoryDurability(material.getId()))
            );
        }
    }
    else
    {
        lines.push_back("Quantité : " + std::to_string(material.getQuantity()));
    }

    lines.push_back("Description : " + material.getDescription());

    std::string valueLine = "Valeur unitaire : "
        + Money::formatEconomyUnits(material.getValue() * material.getQualityPricePercent() / 100);
    if (material.hasSpecialQuality())
    {
        valueLine += " (base " + Money::formatEconomyUnits(material.getValue()) + ")";
    }
    lines.push_back(valueLine);

    if (material.getCategory() == "Matériau de monstre")
    {
        lines.push_back("Utilité connue : revente, artisanat, trophées et recettes liées aux monstres.");
    }
    else if (material.getCategory() == "Matériau")
    {
        lines.push_back("Utilité connue : réparation, amélioration et fabrication d'équipement.");
    }
    else if (material.getCategory() == "Plante")
    {
        lines.push_back("Utilité connue : potions, remèdes, quêtes et secrets botaniques.");
    }
    else if (material.getCategory() == "Outil")
    {
        lines.push_back("Utilité connue : réparation autonome. Un kit intact reste empilé ; un kit entamé garde la marque de son usure.");
    }
    else if (material.getCategory() == "Renseignement" || material.getCategory() == "Livre")
    {
        lines.push_back("Utilité connue : débloquer ou compléter des informations du bestiaire.");
    }

    showInventoryScreen("MATÉRIAU / INFO", "inventory.material.inspect", lines, false);
}


// EN: displayWeapons declares or implements a focused behavior used by this module.
// FR: displayWeapons déclare ou implémente un comportement précis utilisé par ce module.
void Inventory::displayWeapons() const
{
    displayWeaponList();
}

// EN: displayArmors declares or implements a focused behavior used by this module.
// FR: displayArmors déclare ou implémente un comportement précis utilisé par ce module.
void Inventory::displayArmors() const
{
    displayArmorList();
}

// EN: displayConsumables declares or implements a focused behavior used by this module.
// FR: displayConsumables déclare ou implémente un comportement précis utilisé par ce module.
void Inventory::displayConsumables() const
{
    displayConsumableList();
}

// EN: displayMaterials declares or implements a focused behavior used by this module.
// FR: displayMaterials déclare ou implémente un comportement précis utilisé par ce module.
void Inventory::displayMaterials() const
{
    displayMaterialList();
}

// EN: display declares or implements a focused behavior used by this module.
// FR: display déclare ou implémente un comportement précis utilisé par ce module.
void Inventory::display() const
{
    displaySummary();

    displayWeaponList();
    displayArmorList();
    displayConsumableList();
    displayMaterialList();
}
