// EN: Split from Player.cpp to keep Player responsibilities maintainable.
// FR: Extrait de Player.cpp afin de garder des responsabilités Player maintenables.
#include "entity/Player.hpp"
#include "progression/language/LanguageSystem.hpp"
#include "economy/EconomyBalance.hpp"
#include "economy/GuildFinanceSystem.hpp"
#include "core/VersionInfo.hpp"
#include "core/Random.hpp"
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
#include "item/inventory/CarryLoadSystem.hpp"
#include "world/LocalReputationSystem.hpp"
#include "world/City.hpp"
#include "world/crime/VaultTheftNetworkCatalog.hpp"
#include "world/crime/CrimeNetworkSystem.hpp"
#include "world/crime/CrimeJusticeSystem.hpp"
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <random>
#include <sstream>
#include <tuple>
#include <vector>

namespace
{
    std::string vaultWeaponSubjectId(const Weapon& weapon)
    {
        return weapon.getPersistentId().empty() ? "weapon:" + weapon.getName() : weapon.getPersistentId();
    }

    std::string vaultArmorSubjectId(const Armor& armor)
    {
        return armor.getPersistentId().empty() ? "armor:" + armor.getName() : armor.getPersistentId();
    }

    std::string vaultMaterialSubjectId(const Material& material)
    {
        return "material:" + material.getId() + ":" + material.getQuality();
    }

    std::string lowerVaultTheftText(std::string value)
    {
        std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });
        return value;
    }

    bool isHardProtectedVaultMaterial(const Material& material)
    {
        if (material.isQuestRecoveryItem() || material.isPersonalRecoveryItem()) return true;
        const std::string text = lowerVaultTheftText(material.getId() + " " + material.getName() + " " + material.getCategory());
        return text.find("quête") != std::string::npos
            || text.find("quete") != std::string::npos
            || text.find("clé") != std::string::npos
            || text.find("clef") != std::string::npos
            || text.find("information") != std::string::npos
            || text.find("renseignement") != std::string::npos
            || text.find("notice") != std::string::npos;
    }

    bool looksVeryRareVaultMaterial(const Material& material)
    {
        const std::string text = lowerVaultTheftText(material.getId() + " " + material.getName() + " " + material.getCategory());
        return material.hasSpecialQuality()
            || material.isUniqueRecoveryItem()
            || material.getRecoveryPriority() >= 80
            || text.find("unique") != std::string::npos
            || text.find("artefact") != std::string::npos
            || text.find("artifact") != std::string::npos
            || text.find("boss") != std::string::npos
            || text.find("mythique") != std::string::npos
            || text.find("légendaire") != std::string::npos
            || text.find("legendaire") != std::string::npos;
    }


    int vaultEquipmentTheftWeight(const Item& item, bool enchanted)
    {
        int weight = 100;
        const int value = std::max(0, item.getValue());
        if (value >= 1200) weight = 8;
        else if (value >= 700) weight = 18;
        else if (value >= 350) weight = 40;
        else if (value >= 150) weight = 70;
        if (enchanted) weight = std::max(4, weight / 2);
        return weight;
    }

    std::string vaultTheftRecognizablePropertyLabel(const PlayerVaultTheftCase& theftCase)
    {
        if (theftCase.hasWeapon) return "l'arme « " + theftCase.stolenWeapon.getName() + " »";
        if (theftCase.hasArmor) return "l'armure « " + theftCase.stolenArmor.getName() + " »";
        return "";
    }

    void bindVaultTheftCaseToWorldHolder(PlayerVaultTheftCase& theftCase)
    {
        if (theftCase.holderType != "receiver" && theftCase.holderType != "accomplice" && theftCase.holderType != "cache") return;
        if (!theftCase.holderWorldContactId.empty())
        {
            const VaultTheftNetworkContact* existing = VaultTheftNetworkCatalog::findById(theftCase.holderWorldContactId);
            if (existing != nullptr)
            {
                theftCase.holderLabel = existing->displayName;
                theftCase.holderLocationId = existing->locationId;
                theftCase.holderLocationLabel = existing->locationLabel;
                theftCase.holderExplorationBiome = existing->explorationBiome;
                return;
            }
        }
        const VaultTheftNetworkContact* contact = VaultTheftNetworkCatalog::pickFor(theftCase.cityId, theftCase.holderType, theftCase.handoffCount);
        if (contact == nullptr) return;
        theftCase.holderWorldContactId = contact->id;
        theftCase.holderLabel = contact->displayName;
        theftCase.holderLocationId = contact->locationId;
        theftCase.holderLocationLabel = contact->locationLabel;
        theftCase.holderExplorationBiome = contact->explorationBiome;
    }

    bool tryIdentifyVaultTheftCurrentHolder(Player& player, PlayerVaultTheftCase& theftCase, std::string& lead)
    {
        if (theftCase.culpritIdentified || theftCase.clues < 3) return false;
        bindVaultTheftCaseToWorldHolder(theftCase);

        // A cache can be located without magically inventing a person who is not there.
        if (theftCase.holderType == "cache")
        {
            lead += "\nLes indices convergent vers " + (theftCase.holderLocationLabel.empty() ? std::string("une cache secondaire") : theftCase.holderLocationLabel) + ". Tu sais où chercher, mais personne n'est encore identifié comme détenteur actuel.";
            player.recordCanonicalEvent("caches_pillage_localisees", theftCase.caseId, "Cache secondaire probable localisée", 1);
            return false;
        }

        const City* city = City::findById(theftCase.cityId);
        const std::string cityName = city != nullptr ? city->getName() : theftCase.cityId;

        std::string holderName;
        std::string holderFamily;
        if (theftCase.holderType == "receiver")
        {
            holderName = theftCase.holderLabel.empty() ? "Receleur lié au cambriolage de " + cityName : theftCase.holderLabel;
            holderFamily = "Réseau de receleurs";
        }
        else if (theftCase.holderType == "accomplice")
        {
            holderName = theftCase.holderLabel.empty() ? "Complice lié au cambriolage de " + cityName : theftCase.holderLabel;
            holderFamily = "Réseau de complices";
        }
        else
        {
            holderName = "Cambrioleur du coffre de " + cityName;
            holderFamily = "Humain voleur organisé";
        }

        const int rivalLevel = std::max(1, player.getLevel());
        theftCase.identifiedRivalId = player.createRivalFromEnemy(
            holderName,
            holderFamily,
            rivalLevel,
            std::max(20, player.getMaxHp()),
            std::max(4, (player.getMinDamage() + player.getMaxDamage()) / 2),
            "bien volé lors du cambriolage d'un coffre",
            "voleur prudent qui évite le propriétaire tant qu'il le peut",
            65
        );
        theftCase.culpritIdentified = true;
        if (theftCase.holderType == "unknown") theftCase.holderType = "thief";
        theftCase.holderLabel = holderName;
        player.recordCanonicalEvent("cibles_pillage_identifiees", theftCase.caseId, holderName, 1);
        player.recordHistoricalEvent("vault_theft_culprit_identified", theftCase.caseId, holderName + " est identifié comme détenteur probable d'une partie du butin.");
        lead += "\nIDENTIFICATION : " + holderName + ". Une piste de chasse est maintenant crédible.";
        if (!theftCase.holderLocationLabel.empty()) lead += "\nAncrage monde : " + theftCase.holderLocationLabel + ".";
        return true;
    }

    int estimatedVaultTheftValueUnits(const PlayerVaultTheftCase& theftCase)
    {
        long long value = Money::economyUnitsFromCopper(std::max<long long>(0, theftCase.stolenCopper));
        if (theftCase.hasWeapon) value += std::max(0, theftCase.stolenWeapon.getValue());
        if (theftCase.hasArmor) value += std::max(0, theftCase.stolenArmor.getValue());
        if (theftCase.hasConsumable) value += std::max(0, theftCase.stolenConsumable.getValue());
        if (theftCase.stolenMaterialQuantity > 0 && !theftCase.stolenMaterialId.empty())
        {
            const Material sample = MaterialCatalog::createById(theftCase.stolenMaterialId, 1, theftCase.stolenMaterialQuality);
            value += static_cast<long long>(std::max(0, sample.getValue())) * theftCase.stolenMaterialQuantity;
        }
        return static_cast<int>(std::clamp<long long>(value, 0, 100000));
    }

    bool createVaultTheftCaseFromInventory(PlayerVaultTheftCase& theftCase, Inventory& vault, bool severe)
    {
        static std::mt19937 generator(std::random_device{}());
        bool stoleSomething = false;

        const long long storedCopper = vault.getTotalCopper();
        if (storedCopper > 0)
        {
            const int divisor = severe ? 8 : 16;
            const long long stolen = std::min<long long>(storedCopper, std::max<long long>(1, storedCopper / divisor));
            if (vault.spendCopper(stolen))
            {
                theftCase.stolenCopper = stolen;
                stoleSomething = true;
            }
        }

        if (vault.getMaterialCount() > 0)
        {
            std::vector<int> candidates;
            int totalWeight = 0;
            for (int i = 0; i < vault.getMaterialCount(); ++i)
            {
                const Material m = vault.getMaterial(i);
                if (m.getQuantity() <= 0 || !m.canChangeOwner() || isHardProtectedVaultMaterial(m)) continue;
                int w = std::max(2, std::min(300, m.getQuantity() * 8));
                if (looksVeryRareVaultMaterial(m)) w = std::max(1, w / 15);
                candidates.push_back(i);
                totalWeight += w;
            }
            if (!candidates.empty() && totalWeight > 0)
            {
                std::uniform_int_distribution<int> pick(1, totalWeight);
                int roll = pick(generator), selected = candidates.front();
                for (int index : candidates)
                {
                    const Material m = vault.getMaterial(index);
                    int w = std::max(2, std::min(300, m.getQuantity() * 8));
                    if (looksVeryRareVaultMaterial(m)) w = std::max(1, w / 15);
                    roll -= w;
                    if (roll <= 0) { selected = index; break; }
                }
                Material m = vault.getMaterial(selected);
                const int quantity = std::max(1, std::min(m.getQuantity(), severe ? 10 : 5));
                if (vault.removeMaterialQuantity(selected, quantity))
                {
                    theftCase.stolenMaterialId = m.getId();
                    theftCase.stolenMaterialName = m.getName();
                    theftCase.stolenMaterialQuality = m.getQuality();
                    theftCase.stolenMaterialQuantity = quantity;
                    stoleSomething = true;
                }
            }
        }

        auto maybeStealWeapon = [&]() {
            if (vault.getWeaponCount() <= 0) return;
            std::vector<std::pair<int,int>> weighted;
            int total = 0;
            for (int i = 0; i < vault.getWeaponCount(); ++i)
            {
                const Weapon w = vault.getWeapon(i);
                if (!w.canChangeOwner() || w.isQuestRecoveryItem() || w.isPersonalRecoveryItem()) continue;
                int weight = vaultEquipmentTheftWeight(w, w.getEnchantmentCount() > 0);
                if (w.isUniqueRecoveryItem() || w.getRecoveryPriority() >= 80) weight = std::max(1, weight / 12);
                weighted.push_back({i, weight}); total += weight;
            }
            if (weighted.empty()) return;
            const int gate = severe ? 45 : 22;
            std::uniform_int_distribution<int> gateRoll(1, 100);
            if (gateRoll(generator) > gate) return;
            std::uniform_int_distribution<int> pick(1, total);
            int roll = pick(generator), selected = weighted.front().first;
            for (auto [i,w] : weighted) { roll -= w; if (roll <= 0) { selected = i; break; } }
            theftCase.stolenWeapon = vault.getWeapon(selected);
            theftCase.hasWeapon = vault.removeWeapon(selected);
            stoleSomething = stoleSomething || theftCase.hasWeapon;
        };

        auto maybeStealArmor = [&]() {
            if (vault.getArmorCount() <= 0) return;
            std::vector<std::pair<int,int>> weighted;
            int total = 0;
            for (int i = 0; i < vault.getArmorCount(); ++i)
            {
                const Armor a = vault.getArmor(i);
                if (!a.canChangeOwner() || a.isQuestRecoveryItem() || a.isPersonalRecoveryItem()) continue;
                int weight = vaultEquipmentTheftWeight(a, a.getEnchantmentCount() > 0);
                if (a.isUniqueRecoveryItem() || a.getRecoveryPriority() >= 80) weight = std::max(1, weight / 12);
                weighted.push_back({i, weight}); total += weight;
            }
            if (weighted.empty()) return;
            const int gate = severe ? 38 : 18;
            std::uniform_int_distribution<int> gateRoll(1, 100);
            if (gateRoll(generator) > gate) return;
            std::uniform_int_distribution<int> pick(1, total);
            int roll = pick(generator), selected = weighted.front().first;
            for (auto [i,w] : weighted) { roll -= w; if (roll <= 0) { selected = i; break; } }
            theftCase.stolenArmor = vault.getArmor(selected);
            theftCase.hasArmor = vault.removeArmor(selected);
            stoleSomething = stoleSomething || theftCase.hasArmor;
        };

        maybeStealWeapon();
        maybeStealArmor();

        if (vault.getConsumableCount() > 0)
        {
            std::uniform_int_distribution<int> consumeGate(1, 100);
            if (consumeGate(generator) <= (severe ? 65 : 35))
            {
                int selectedConsumable = -1;
                for (int i = 0; i < vault.getConsumableCount(); ++i)
                {
                    if (vault.getConsumable(i).canChangeOwner()
                        && !vault.getConsumable(i).isQuestRecoveryItem()
                        && !vault.getConsumable(i).isPersonalRecoveryItem())
                    {
                        selectedConsumable = i;
                        break;
                    }
                }
                if (selectedConsumable < 0) return stoleSomething;
                const Consumable c = vault.getConsumable(selectedConsumable);
                theftCase.stolenConsumable = c;
                theftCase.hasConsumable = vault.removeConsumable(selectedConsumable);
                stoleSomething = stoleSomething || theftCase.hasConsumable;
            }
        }

        return stoleSomething;
    }
    std::string stealPrioritizedCommonVaultMaterial(Inventory& vault, bool severe)
    {
        std::vector<int> commonCandidates;
        std::vector<int> rareCandidates;
        for (int i = 0; i < vault.getMaterialCount(); ++i)
        {
            const Material material = vault.getMaterial(i);
            if (material.getQuantity() <= 0 || !material.canChangeOwner() || isHardProtectedVaultMaterial(material)) continue;
            if (looksVeryRareVaultMaterial(material)) rareCandidates.push_back(i);
            else commonCandidates.push_back(i);
        }

        auto score = [&](int index) {
            const Material material = vault.getMaterial(index);
            // Thieves prefer abundant, ordinary stacks and only weakly care about unit value.
            return material.getQuantity() * 100 - std::min(500, std::max(0, material.getValue()));
        };
        auto bestIndex = [&](const std::vector<int>& candidates) {
            int best = -1;
            int bestScore = -1000000000;
            for (int index : candidates)
            {
                const int candidateScore = score(index);
                if (candidateScore > bestScore)
                {
                    best = index;
                    bestScore = candidateScore;
                }
            }
            return best;
        };

        int selected = bestIndex(commonCandidates);
        if (selected < 0 && !rareCandidates.empty())
        {
            // Rare/artefact-like materials are assumed to be hidden much more carefully.
            // They are not impossible to steal, but the chance stays deliberately tiny.
            static std::mt19937 rareGenerator(std::random_device{}());
            std::uniform_int_distribution<int> rareRoll(1, 1000);
            const int threshold = severe ? 10 : 4; // 1.0% ignored raid, 0.4% failed defense.
            if (rareRoll(rareGenerator) <= threshold) selected = bestIndex(rareCandidates);
        }
        if (selected < 0) return {};

        const Material material = vault.getMaterial(selected);
        const int divisor = severe ? 5 : 10;
        const int cap = severe ? 8 : 4;
        const int quantity = std::max(1, std::min({material.getQuantity(), cap, std::max(1, material.getQuantity() / divisor)}));
        if (!vault.removeMaterialQuantity(selected, quantity)) return {};
        return material.getName() + " x" + std::to_string(quantity);
    }
}

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

int Player::calculateVaultUsedSlots(const Inventory& vault, bool placeOptimized, bool preparedStorage)
{
    const int rawLoad = CarryLoadSystem::inventoryLoad(vault);
    int reductionPercent = 0;
    if (placeOptimized) reductionPercent += 10;
    if (preparedStorage) reductionPercent += 15;
    reductionPercent = std::clamp(reductionPercent, 0, 25);
    // Both upgrades are measured from the same 100% base: -10 then -15 additional = -25% exactly.
    return std::max(0, (rawLoad * (100 - reductionPercent) + 99) / 100);
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
        || !cityVault.getMaterials().empty()
        || cityVault.getTotalCopper() > 0;

    if (record == nullptr)
    {
        if (!cityVaultPurchased && cityVaultLevel <= 0 && !hasStoredContent)
        {
            return;
        }

        PlayerCityVault created;
        created.cityId = currentCityId;
        created.inventory = cityVault;
        created.purchased = cityVaultPurchased;
        created.level = cityVaultPurchased ? std::max(1, std::min(5, cityVaultLevel)) : 0;
        created.placeOptimized = cityVaultPlaceOptimized;
        created.preparedStorage = cityVaultPreparedStorage;
        created.securityDaysRemaining = cityVaultSecurityDaysRemaining;
        created.raidPending = cityVaultRaidPending;
        created.raidDeadlineDay = cityVaultRaidDeadlineDay;
        created.lastRaidCheckDay = cityVaultLastRaidCheckDay;
        cityVaults.push_back(created);
        return;
    }

    record->inventory = cityVault;
    record->purchased = cityVaultPurchased;
    record->level = cityVaultPurchased ? std::max(1, std::min(5, cityVaultLevel)) : 0;
    record->placeOptimized = cityVaultPlaceOptimized;
    record->preparedStorage = cityVaultPreparedStorage;
    record->securityDaysRemaining = cityVaultSecurityDaysRemaining;
    record->raidPending = cityVaultRaidPending;
    record->raidDeadlineDay = cityVaultRaidDeadlineDay;
    record->lastRaidCheckDay = cityVaultLastRaidCheckDay;
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
        cityVaultPlaceOptimized = false;
        cityVaultPreparedStorage = false;
        cityVaultSecurityDaysRemaining = 0;
        cityVaultRaidPending = false;
        cityVaultRaidDeadlineDay = -1;
        cityVaultLastRaidCheckDay = -1;
        return;
    }

    cityVault = record->inventory;
    cityVaultPurchased = record->purchased;
    cityVaultLevel = cityVaultPurchased ? std::max(1, std::min(5, record->level)) : 0;
    cityVaultPlaceOptimized = cityVaultPurchased && record->placeOptimized;
    cityVaultPreparedStorage = cityVaultPurchased && cityVaultLevel >= 5 && record->preparedStorage;
    cityVaultSecurityDaysRemaining = cityVaultPurchased ? std::max(0, record->securityDaysRemaining) : 0;
    cityVaultRaidPending = cityVaultPurchased && record->raidPending;
    cityVaultRaidDeadlineDay = cityVaultRaidPending ? record->raidDeadlineDay : -1;
    cityVaultLastRaidCheckDay = record->lastRaidCheckDay;
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

    const PlayerCityVault* record = findCityVaultRecord(cityId);
    const bool optimized = cityId == currentCityId ? cityVaultPlaceOptimized : (record != nullptr && record->placeOptimized);
    const bool prepared = cityId == currentCityId ? cityVaultPreparedStorage : (record != nullptr && record->preparedStorage);
    return calculateVaultUsedSlots(getCityVaultForCity(cityId), optimized, prepared);
}

bool Player::hasCityVaultPlaceOptimizationForCity(const std::string& cityId) const
{
    if (cityId == currentCityId) return hasCityVault() && cityVaultPlaceOptimized;
    const PlayerCityVault* record = findCityVaultRecord(cityId);
    return record != nullptr && record->purchased && record->placeOptimized;
}

bool Player::hasCityVaultPreparedStorageForCity(const std::string& cityId) const
{
    if (cityId == currentCityId) return hasCityVault() && cityVaultPreparedStorage;
    const PlayerCityVault* record = findCityVaultRecord(cityId);
    return record != nullptr && record->purchased && record->level >= 5 && record->preparedStorage;
}

int Player::getOwnedCityVaultCount() const
{
    int count = 0;
    bool currentCounted = false;
    for (const PlayerCityVault& record : cityVaults)
    {
        if (record.purchased && record.level > 0)
        {
            ++count;
            if (record.cityId == currentCityId) currentCounted = true;
        }
    }
    if (hasCityVault() && !currentCounted) ++count;
    return count;
}

int Player::getCityVaultSecurityDaysRemainingForCity(const std::string& cityId) const
{
    if (cityId == currentCityId) return hasCityVault() ? std::max(0, cityVaultSecurityDaysRemaining) : 0;
    const PlayerCityVault* record = findCityVaultRecord(cityId);
    return record != nullptr && record->purchased ? std::max(0, record->securityDaysRemaining) : 0;
}

bool Player::hasCityVaultRaidPendingForCity(const std::string& cityId) const
{
    if (cityId == currentCityId) return hasCityVault() && cityVaultRaidPending;
    const PlayerCityVault* record = findCityVaultRecord(cityId);
    return record != nullptr && record->purchased && record->raidPending;
}

int Player::getCityVaultRaidDeadlineDayForCity(const std::string& cityId) const
{
    if (!hasCityVaultRaidPendingForCity(cityId)) return -1;
    if (cityId == currentCityId) return cityVaultRaidDeadlineDay;
    const PlayerCityVault* record = findCityVaultRecord(cityId);
    return record == nullptr ? -1 : record->raidDeadlineDay;
}

int Player::getCityVaultSecuritySubscriptionCost() const
{
    return EconomyBalance::cityVaultSecuritySubscriptionCost(std::max(1, cityVaultLevel));
}

bool Player::buyCityVaultSecuritySubscription()
{
    loadCurrentCityVaultRecord();
    if (!hasCityVault() || cityVaultSecurityDaysRemaining >= 35) return false;
    const int cost = getCityVaultSecuritySubscriptionCost();
    if (cost <= 0 || !inventory.spendEconomyUnits(cost)) return false;
    cityVaultSecurityDaysRemaining = std::min(35, cityVaultSecurityDaysRemaining + 7);
    recordCanonicalEvent("securite_coffres", currentCityId + ":subscription", "Sécurité et entretien du coffre", 1);
    syncCurrentCityVaultRecord();
    return true;
}

bool Player::canReuseRecentCityVaultRecallMessenger(const std::string& sourceCityId, const std::string& destinationCityId) const
{
    if (sourceCityId.empty() || destinationCityId.empty()) return false;
    if (sourceCityId != cityVaultLastRecallMessengerSourceCityId
        || destinationCityId != cityVaultLastRecallMessengerDestinationCityId) return false;
    const int absoluteStamp = worldDaysElapsed * getWorldDayUnitsPerDay() + worldDayProgressUnits;
    const int delta = absoluteStamp - cityVaultLastRecallMessengerStamp;
    // 5 world-time units = one day. Two units are deliberately just under half a day.
    return delta >= 0 && delta <= 2;
}

int Player::getCityVaultRecallMessengerSurchargeCopper(
    const std::string& sourceCityId,
    const std::string& destinationCityId,
    int distanceKm) const
{
    const int full = EconomyBalance::cityVaultRecallMessengerSurchargeCopper(sourceCityId, destinationCityId, distanceKm);
    if (!canReuseRecentCityVaultRecallMessenger(sourceCityId, destinationCityId)) return full;
    // Same messenger, same route, still waiting to depart: only a small administration/memory supplement.
    return std::max(5, (full + 4) / 5);
}

void Player::markCityVaultRecallMessengerSent(const std::string& sourceCityId, const std::string& destinationCityId)
{
    cityVaultLastRecallMessengerStamp = worldDaysElapsed * getWorldDayUnitsPerDay() + worldDayProgressUnits;
    cityVaultLastRecallMessengerSourceCityId = sourceCityId;
    cityVaultLastRecallMessengerDestinationCityId = destinationCityId;
}

int Player::getCityVaultLastRecallMessengerStamp() const { return cityVaultLastRecallMessengerStamp; }
const std::string& Player::getCityVaultLastRecallMessengerSourceCityId() const { return cityVaultLastRecallMessengerSourceCityId; }
const std::string& Player::getCityVaultLastRecallMessengerDestinationCityId() const { return cityVaultLastRecallMessengerDestinationCityId; }

void Player::setLoadedCityVaultRecallMessengerState(
    int absoluteStamp,
    const std::string& sourceCityId,
    const std::string& destinationCityId)
{
    cityVaultLastRecallMessengerStamp = absoluteStamp;
    cityVaultLastRecallMessengerSourceCityId = sourceCityId;
    cityVaultLastRecallMessengerDestinationCityId = destinationCityId;
}

bool Player::triggerCityVaultRaidAlertForCity(const std::string& cityId, int deadlineDays)
{
    if (cityId.empty() || cityId == currentCityId || !hasCityVaultInCity(cityId)) return false;
    PlayerCityVault* record = findCityVaultRecord(cityId);
    if (record == nullptr || record->raidPending || record->securityDaysRemaining > 0) return false;
    record->raidPending = true;
    record->raidDeadlineDay = worldDaysElapsed + std::max(1, deadlineDays);
    record->lastRaidCheckDay = worldDaysElapsed;
    recordCanonicalEvent("alertes_pillage_coffres", cityId, "Tentative de pillage du coffre", 1);
    pendingWorldTimeReportLines.push_back(
        "Messager urgent : le coffre de " + cityId + " est visé par des pillards. Tu as jusqu'au jour "
        + std::to_string(record->raidDeadlineDay + 1) + " pour revenir le défendre."
    );
    return true;
}


const std::vector<PlayerVaultTheftCase>& Player::getVaultTheftCases() const
{
    return vaultTheftCases;
}

const PlayerVaultTheftCase* Player::getOpenVaultTheftCaseForCity(const std::string& cityId) const
{
    for (auto it = vaultTheftCases.rbegin(); it != vaultTheftCases.rend(); ++it)
    {
        if (!it->resolved && it->cityId == cityId) return &(*it);
    }
    return nullptr;
}

bool Player::hasOpenVaultTheftCaseForCity(const std::string& cityId) const
{
    return getOpenVaultTheftCaseForCity(cityId) != nullptr;
}

std::string Player::investigateVaultTheftCase(const std::string& cityId)
{
    PlayerVaultTheftCase* target = nullptr;
    for (auto it = vaultTheftCases.rbegin(); it != vaultTheftCases.rend(); ++it)
    {
        if (!it->resolved && it->cityId == cityId) { target = &(*it); break; }
    }
    if (target == nullptr) return "Aucun cambriolage non résolu à enquêter ici.";

    static std::mt19937 generator(std::random_device{}());
    std::uniform_int_distribution<int> clueRoll(1, 100);
    const int roll = clueRoll(generator);
    const int gained = roll <= 20 ? 2 : 1;
    target->clues = std::min(6, target->clues + gained);
    target->lastLeadDay = worldDaysElapsed;
    advanceWorldDayUnits(1);

    std::string lead;
    if (target->clues <= 1) lead = "Des voisins ont remarqué des allées et venues inhabituelles, mais personne ne peut encore décrire sûrement l'auteur.";
    else if (target->clues == 2) lead = "La garde recoupe des traces de passage et le témoignage d'un commerçant : le cambriolage semble organisé.";
    else if (target->clues == 3) lead = "Un témoin et un receleur donnent enfin une piste cohérente. L'auteur n'est plus totalement anonyme.";
    else lead = "Les témoignages, traces et circuits de revente convergent désormais vers la même personne ou le même groupe.";

    recordCanonicalEvent("enquete_pillage_coffre", target->caseId, "Indice obtenu sur le cambriolage de " + cityId, gained);

    if (!target->culpritIdentified)
    {
        tryIdentifyVaultTheftCurrentHolder(*this, *target, lead);
    }
    else
    {
        lead += "\nLa cible est déjà identifiée : " + target->holderLabel + ".";
    }
    return lead;
}

VaultTheftExplorationObservationResult Player::observeVaultTheftCaseDuringExploration(
    const std::string& cityId,
    const std::string& biomeName,
    VaultTheftExplorationObservation observation
)
{
    VaultTheftExplorationObservationResult result;
    PlayerVaultTheftCase* target = nullptr;
    for (auto it = vaultTheftCases.rbegin(); it != vaultTheftCases.rend(); ++it)
    {
        if (!it->resolved && !it->culpritIdentified && it->cityId == cityId)
        {
            target = &(*it);
            break;
        }
    }
    if (target == nullptr) return result;

    result.occurred = true;
    result.caseId = target->caseId;
    result.cluesBefore = target->clues;

    bindVaultTheftCaseToWorldHolder(*target);
    const std::string location = biomeName.empty() ? "les environs" : biomeName;
    const std::string recognizableProperty = vaultTheftRecognizablePropertyLabel(*target);
    if (observation == VaultTheftExplorationObservation::RecognizedProperty
        && !target->holderExplorationBiome.empty()
        && !biomeName.empty()
        && target->holderExplorationBiome != biomeName)
    {
        observation = VaultTheftExplorationObservation::SuspiciousTrail;
    }
    if (observation == VaultTheftExplorationObservation::RecognizedProperty && recognizableProperty.empty())
    {
        observation = VaultTheftExplorationObservation::SuspiciousExchange;
    }

    std::string lead;
    if (observation == VaultTheftExplorationObservation::RecognizedProperty)
    {
        result.cluesGained = 2;
        result.recognizedStolenProperty = true;
        result.lines.push_back("Dans " + location + ", tu aperçois un individu avec " + recognizableProperty + ".");
        result.lines.push_back("Tu reconnais réellement un bien déclaré volé. Cela prouve un lien avec le butin, pas que cette personne a forcément commis le cambriolage elle-même.");
        lead = "Bien volé reconnu directement en exploration : " + recognizableProperty;
        recordHistoricalEvent("vault_theft_property_seen", target->caseId, "Un bien volé a été reconnu directement pendant une exploration dans " + location + ".");
    }
    else if (observation == VaultTheftExplorationObservation::SuspiciousExchange)
    {
        result.cluesGained = 1;
        result.lines.push_back("Dans " + location + ", tu surprends un échange discret : un paquet change de mains dès que les passants approchent.");
        if (target->stolenMaterialQuantity > 0 && !target->stolenMaterialName.empty())
        {
            result.lines.push_back("La forme du lot et les traces visibles rappellent le stock de " + target->stolenMaterialName + " disparu du coffre, sans permettre une identification certaine.");
        }
        else if (target->hasConsumable)
        {
            result.lines.push_back("Le conditionnement ressemble à celui de " + target->stolenConsumable.getName() + ", mais tu n'as pas assez d'éléments pour affirmer qu'il s'agit du même exemplaire.");
        }
        else
        {
            result.lines.push_back("Rien ne permet encore de nommer la personne ni de relier chaque objet transporté au cambriolage.");
        }
        lead = "Échange suspect observé en exploration";
    }
    else
    {
        result.cluesGained = 1;
        result.lines.push_back("Dans " + location + ", tu retrouves une piste cohérente avec le cambriolage : traces de passage récentes, détour inhabituel et tentative évidente d'éviter les axes surveillés.");
        result.lines.push_back("Ce n'est qu'un indice de déplacement. Aucun nom ni visage n'est inventé à partir de cette trace.");
        lead = "Piste de déplacement observée en exploration";
    }

    target->clues = std::min(6, target->clues + result.cluesGained);
    target->lastLeadDay = worldDaysElapsed;
    result.cluesAfter = target->clues;
    recordCanonicalEvent("indices_pillage_exploration", target->caseId, lead + " — " + location, result.cluesGained);

    std::string identificationLine;
    result.targetIdentifiedNow = tryIdentifyVaultTheftCurrentHolder(*this, *target, identificationLine);
    if (!identificationLine.empty())
    {
        std::istringstream lines(identificationLine);
        std::string line;
        while (std::getline(lines, line))
        {
            if (!line.empty()) result.lines.push_back(line);
        }
    }
    if (target->culpritIdentified) result.targetLabel = target->holderLabel;

    result.lines.push_back("Dossier : " + std::to_string(result.cluesBefore) + " → " + std::to_string(result.cluesAfter) + " indice(s).");
    return result;
}

bool Player::recoverVaultTheftCaseProperty(const std::string& caseId)
{
    for (PlayerVaultTheftCase& theftCase : vaultTheftCases)
    {
        if (theftCase.caseId != caseId || theftCase.resolved) continue;
        bool recovered = false;
        if (theftCase.stolenCopper > 0)
        {
            inventory.earnCopper(theftCase.stolenCopper);
            theftCase.stolenCopper = 0;
            recovered = true;
        }
        if (theftCase.hasWeapon)
        {
            inventory.addWeapon(theftCase.stolenWeapon);
            theftCase.hasWeapon = false;
            recovered = true;
        }
        if (theftCase.hasArmor)
        {
            inventory.addArmor(theftCase.stolenArmor);
            theftCase.hasArmor = false;
            recovered = true;
        }
        if (theftCase.hasConsumable)
        {
            inventory.addConsumable(theftCase.stolenConsumable);
            theftCase.hasConsumable = false;
            recovered = true;
        }
        if (theftCase.stolenMaterialQuantity > 0 && !theftCase.stolenMaterialId.empty())
        {
            inventory.addMaterial(MaterialCatalog::createById(theftCase.stolenMaterialId, theftCase.stolenMaterialQuantity, theftCase.stolenMaterialQuality));
            theftCase.stolenMaterialQuantity = 0;
            recovered = true;
        }
        if (recovered)
        {
            theftCase.resolved = true;
            recordCanonicalEvent("biens_coffre_recuperes", theftCase.caseId, "Biens récupérés après enquête sur cambriolage", 1);
            recordHistoricalEvent("vault_theft_recovered", theftCase.caseId, "Les biens encore traçables du cambriolage ont été récupérés.", true);
            grantTitle("Rends-moi ça.");
        }
        return recovered;
    }
    return false;
}

VaultTheftContactInteractionResult Player::interactWithVaultTheftHolder(
    const std::string& cityId,
    VaultTheftContactAction action,
    Random& random)
{
    VaultTheftContactInteractionResult result;
    PlayerVaultTheftCase* target = nullptr;
    for (auto it = vaultTheftCases.rbegin(); it != vaultTheftCases.rend(); ++it)
    {
        if (!it->resolved && it->cityId == cityId) { target = &(*it); break; }
    }
    if (target == nullptr)
    {
        result.lines.push_back("Aucun dossier de cambriolage non résolu n'est actif dans cette ville.");
        return result;
    }

    bindVaultTheftCaseToWorldHolder(*target);
    result.caseId = target->caseId;
    result.contactLabel = target->holderLabel;
    result.locationLabel = target->holderLocationLabel;
    result.cluesBefore = target->clues;

    const bool contactHolder = target->holderType == "receiver" || target->holderType == "accomplice";
    const bool cacheHolder = target->holderType == "cache";
    if (!contactHolder && !cacheHolder)
    {
        result.lines.push_back("La piste actuelle ne mène encore ni à un contact local ni à une cache exploitable.");
        result.lines.push_back("Continue l'enquête ou l'exploration : le jeu ne crée pas artificiellement un receleur juste pour ouvrir ce menu.");
        return result;
    }

    result.available = true;
    const std::string place = target->holderLocationLabel.empty() ? "le secteur indiqué par les indices" : target->holderLocationLabel;

    PlayerCrimeContactState* persistentContact = nullptr;
    if (contactHolder && !target->holderWorldContactId.empty())
    {
        persistentContact = &getOrCreateCrimeContactState(target->holderWorldContactId, target->holderLabel);
        if (persistentContact->knownRole.empty())
            persistentContact->knownRole = target->holderType == "receiver" ? "receleur" : "complice";
        target->holderPressure = std::max(target->holderPressure, persistentContact->pressure);
        if (std::abs(persistentContact->cooperation) > std::abs(target->holderCooperation))
            target->holderCooperation = persistentContact->cooperation;
        if (persistentContact->arrested)
        {
            result.available = false;
            result.lines.push_back(target->holderLabel + " est sous arrestation durable à cause d'un dossier déjà établi.");
            result.lines.push_back("Le même PNJ ne réapparaît donc pas normalement dans la rue comme si rien ne s'était passé. Le dossier doit suivre la chaîne de possession restante.");
            return result;
        }
        if (persistentContact->detainedUntilDay > worldDaysElapsed)
        {
            result.available = false;
            result.contactTemporarilyClosed = true;
            result.lines.push_back(target->holderLabel + " est encore en détention provisoire auprès de la garde.");
            result.lines.push_back("Sans preuve suffisante pour une condamnation durable, cette détention reste temporaire. Réexamen prévu vers le jour " + std::to_string(persistentContact->detainedUntilDay) + ".");
            return result;
        }
        if (persistentContact->fledUntilDay > worldDaysElapsed)
        {
            result.available = false;
            result.contactTemporarilyClosed = true;
            result.lines.push_back(target->holderLabel + " a quitté le secteur après les pressions précédentes.");
            result.lines.push_back("Retour possible estimé : jour " + std::to_string(persistentContact->fledUntilDay) + ". La mémoire du PNJ dépasse désormais un seul dossier.");
            return result;
        }
    }

    auto spreadGuardPressureToKnownLocalNetwork = [&](int pressureGain, int cooperationLoss, const std::string& reason)
    {
        if (!contactHolder || target->holderWorldContactId.empty()) return;
        int affected = 0;
        for (PlayerCrimeContactState& state : crimeContactStates)
        {
            if (state.contactId.empty() || state.contactId == target->holderWorldContactId || state.arrested) continue;
            const VaultTheftNetworkContact* known = VaultTheftNetworkCatalog::findById(state.contactId);
            if (known == nullptr || known->cityId != target->cityId) continue;
            state.pressure = std::clamp(state.pressure + std::max(0, pressureGain), 0, 100);
            state.cooperation = std::clamp(state.cooperation - std::max(0, cooperationLoss), -100, 100);
            ++affected;
        }
        if (affected > 0)
        {
            recordCanonicalEvent(
                "crime_network_guard_reactions",
                target->cityId + ":day_" + std::to_string(worldDaysElapsed),
                reason + " | contacts locaux affectés=" + std::to_string(affected),
                affected);
        }
    };

    const bool alreadyInteractedToday = target->lastHolderInteractionDay == worldDaysElapsed;
    if (alreadyInteractedToday)
    {
        result.available = false;
        result.lines.push_back("Tu as déjà forcé cette piste aujourd'hui autour de " + place + ".");
        result.lines.push_back("Revenir immédiatement ne crée pas de nouveaux renseignements : laisse le réseau bouger ou fais avancer le temps.");
        return result;
    }

    if (contactHolder && target->holderClosedUntilDay > worldDaysElapsed)
    {
        result.available = false;
        result.contactTemporarilyClosed = true;
        result.lines.push_back(target->holderLabel + " a fermé le contact après les dernières pressions.");
        result.lines.push_back("Le réseau reste jouable : attendre, poursuivre d'autres pistes ou laisser la garde travailler vaut mieux que spammer la même personne.");
        result.lines.push_back("Réouverture estimée : jour " + std::to_string(target->holderClosedUntilDay) + ".");
        return result;
    }

    if (action == VaultTheftContactAction::Observe)
    {
        ++target->holderObservationCount;
        if (persistentContact != nullptr)
        {
            ++persistentContact->encounters;
            ++persistentContact->observations;
            persistentContact->lastInteractionDay = worldDaysElapsed;
        }
        advanceWorldDayUnits(1);
        target->lastHolderInteractionDay = worldDaysElapsed;
        const int roll = random.between(1, 100);
        const int gained = roll <= 25 ? 2 : 1;
        target->clues = std::min(6, target->clues + gained);
        target->lastLeadDay = worldDaysElapsed;
        if (roll >= 92)
        {
            target->holderAlerted = true;
            if (persistentContact != nullptr) persistentContact->pressure = std::clamp(persistentContact->pressure + 5, 0, 100);
        }
        result.success = true;
        result.cluesAfter = target->clues;
        result.lines.push_back("Tu observes discrètement " + place + " au lieu de lancer une accusation sans preuve.");
        result.lines.push_back(gained == 2
            ? "Deux détails concordent : trajet, horaires et façon de dissimuler le paquet renforcent nettement le dossier."
            : "Un détail exploitable rejoint le dossier sans révéler plus que ce que tu as réellement vu.");
        if (target->holderAlerted)
            result.lines.push_back("Le contact semble avoir remarqué une présence inhabituelle : les prochaines approches seront plus difficiles.");
        recordCanonicalEvent("observations_reseau_pillage", target->caseId, "Observation locale de " + place, gained);
        std::string identificationLine;
        tryIdentifyVaultTheftCurrentHolder(*this, *target, identificationLine);
        if (!identificationLine.empty()) result.lines.push_back(identificationLine);
        return result;
    }

    if (action == VaultTheftContactAction::SearchCache)
    {
        if (!cacheHolder || target->clues < 3)
        {
            result.available = false;
            result.lines.push_back(cacheHolder
                ? "La cache est soupçonnée, mais les indices ne suffisent pas encore pour la fouiller sans chercher au hasard."
                : "La piste actuelle mène à une personne, pas à une cache abandonnée.");
            return result;
        }
        ++target->holderObservationCount;
        advanceWorldDayUnits(2);
        target->lastHolderInteractionDay = worldDaysElapsed;
        const int chance = std::clamp(48 + target->clues * 7 + target->holderObservationCount * 4, 55, 92);
        result.success = random.between(1, 100) <= chance;
        if (result.success)
        {
            result.propertyRecovered = recoverVaultTheftCaseProperty(target->caseId);
            result.lines.push_back("La fouille de " + place + " retrouve les biens encore traçables sans inventer de voleur présent sur place.");
            result.lines.push_back(result.propertyRecovered
                ? "Les objets récupérables retournent réellement dans ton inventaire."
                : "La cache était bien liée au dossier, mais aucun bien encore enregistré n'y restait.");
            recordCanonicalEvent("caches_pillage_fouillees", target->caseId, "Cache fouillée avec succès : " + place, 1);
        }
        else
        {
            target->clues = std::min(6, target->clues + 1);
            result.lines.push_back("La cache ne livre pas encore le butin, mais la fouille confirme un passage récent et ajoute une vraie piste.");
            recordCanonicalEvent("caches_pillage_fouillees", target->caseId, "Fouille infructueuse mais informative : " + place, 1);
        }
        result.cluesAfter = target->clues;
        return result;
    }

    if (action == VaultTheftContactAction::RequestGuardIntervention)
    {
        if (!contactHolder || target->clues < 4)
        {
            result.available = false;
            result.lines.push_back(contactHolder
                ? "La garde refuse une confrontation directe avec un dossier aussi mince : il faut au moins 4 indices cohérents."
                : "Une cache n'est pas une personne à interroger : localise-la puis fouille-la.");
            return result;
        }

        result.guardInvolved = true;
        ++target->confrontationAttempts;
        target->holderReportedToGuard = true;
        target->holderPressure = std::clamp(target->holderPressure + 25, 0, 100);
        if (persistentContact != nullptr)
        {
            ++persistentContact->encounters;
            ++persistentContact->guardConfrontations;
            persistentContact->reportedToGuard = true;
            persistentContact->pressure = std::max(persistentContact->pressure, target->holderPressure);
            persistentContact->lastInteractionDay = worldDaysElapsed;
        }
        advanceWorldDayUnits(2);
        target->lastHolderInteractionDay = worldDaysElapsed;

        int chance = 28 + target->clues * 9 + target->holderObservationCount * 4 + target->holderCooperation / 3;
        if (target->holderType == "accomplice") chance += 8;
        if (target->holderAlerted) chance -= 12;
        chance -= std::max(0, target->confrontationAttempts - 1) * 5;
        chance = std::clamp(chance, 45, 94);
        result.success = random.between(1, 100) <= chance;

        if (result.success)
        {
            result.contactCooperated = true;
            target->holderCooperation = std::clamp(target->holderCooperation + 35, 0, 100);
            if (persistentContact != nullptr)
            {
                persistentContact->cooperation = std::max(persistentContact->cooperation, target->holderCooperation);
                persistentContact->pressure = std::max(persistentContact->pressure, target->holderPressure);
            }
            const int gained = target->clues >= 5 ? 1 : 2;
            target->clues = std::min(6, target->clues + gained);
            target->lastLeadDay = worldDaysElapsed;
            result.lines.push_back("La garde confronte " + target->holderLabel + " à " + place + " à partir des faits du dossier, pas d'une accusation inventée.");
            result.lines.push_back("Le contact coopère partiellement : horaires, intermédiaire et provenance du paquet deviennent des éléments consignés.");

            const int recoveryChance = std::clamp(35 + target->clues * 8 + target->holderCooperation / 4, 55, 96);
            if (random.between(1, 100) <= recoveryChance)
            {
                result.propertyRecovered = recoverVaultTheftCaseProperty(target->caseId);
                result.lines.push_back(result.propertyRecovered
                    ? "Sous contrôle de la garde, les biens encore traçables sont restitués sans combat ni exécution du contact."
                    : "La confrontation confirme le réseau, mais les biens ne sont déjà plus matériellement présents chez ce contact.");
            }
            else
            {
                result.lines.push_back("Le contact donne une piste exploitable mais les biens ont déjà circulé : le dossier reste ouvert.");
            }
            if (persistentContact != nullptr && target->clues >= 5 && target->holderPressure >= 50)
            {
                const int arrestChance = std::clamp(10 + target->clues * 5 + target->holderPressure / 4, 25, 70);
                if (random.between(1, 100) <= arrestChance)
                {
                    const int previousDetentionEnd = persistentContact->detainedUntilDay;
                    persistentContact->detainedUntilDay = std::max(persistentContact->detainedUntilDay, worldDaysElapsed + 4);
                    if (persistentContact->detainedUntilDay != previousDetentionEnd) ++persistentContact->provisionalDetentions;
                    result.lines.push_back("Les éléments accumulés suffisent à placer " + target->holderLabel + " en détention provisoire. Cela ne transforme toujours pas automatiquement le receleur en auteur du cambriolage ni en condamné permanent.");
                    result.lines.push_back("Sans nouvelles preuves, la garde devra réexaminer sa situation vers le jour " + std::to_string(persistentContact->detainedUntilDay) + ".");
                    recordCanonicalEvent("contacts_criminels_detenus", persistentContact->contactId, "Contact placé temporairement en détention après confrontation documentée", 1);
                }

                const CrimeJusticeResolutionResult justice = CrimeJusticeSystem::tryApplyDurableArrest(
                    *this, *target, persistentContact->contactId);
                if (!justice.lines.empty())
                    result.lines.insert(result.lines.end(), justice.lines.begin(), justice.lines.end());
            }
            spreadGuardPressureToKnownLocalNetwork(5, 1, "Une confrontation documentée de la garde rend le réseau local plus prudent");
            CrimeNetworkSystem::propagateKnownWarning(*this, target->holderWorldContactId, 0, 2, 1,
                "Une intervention documentée circule discrètement vers les relais inter-ville déjà connus");
            recordCanonicalEvent("interrogatoires_garde_pillage", target->caseId, "Contact coopératif : " + target->holderLabel, gained);
        }
        else
        {
            target->holderAlerted = true;
            target->holderCooperation = std::max(-100, target->holderCooperation - 20);
            if (persistentContact != nullptr)
            {
                persistentContact->cooperation = std::min(persistentContact->cooperation, target->holderCooperation);
                persistentContact->pressure = std::max(persistentContact->pressure, target->holderPressure);
            }
            if (target->confrontationAttempts >= 2 || target->holderPressure >= 70)
            {
                target->holderClosedUntilDay = worldDaysElapsed + 3;
                if (persistentContact != nullptr) persistentContact->fledUntilDay = std::max(persistentContact->fledUntilDay, worldDaysElapsed + 5);
                result.contactTemporarilyClosed = true;
            }
            result.lines.push_back(target->holderLabel + " nie, se ferme et ne remet aucun bien sur une simple pression.");
            result.lines.push_back("La garde a désormais une trace officielle de la confrontation, mais l'échec rend le réseau plus prudent.");
            if (result.contactTemporarilyClosed)
                result.lines.push_back("Le contact ferme temporairement ses portes : insister immédiatement serait du farm, pas une enquête crédible.");
            spreadGuardPressureToKnownLocalNetwork(9, 3, "Une confrontation ratée alerte une partie du réseau criminel local");
            CrimeNetworkSystem::propagateKnownWarning(*this, target->holderWorldContactId, 0, 4, 2,
                "Une confrontation ratée fait circuler une alerte plus forte vers les relais inter-ville déjà connus");
            recordCanonicalEvent("interrogatoires_garde_pillage", target->caseId, "Confrontation refusée par " + target->holderLabel, 1);
        }
        result.cluesAfter = target->clues;
        return result;
    }

    if (!contactHolder || target->clues < 3)
    {
        result.available = false;
        result.lines.push_back(contactHolder
            ? "Tu n'as pas encore assez de preuves pour négocier une restitution sans révéler que tu enquêtes au hasard."
            : "Une cache ne négocie pas : il faut la localiser puis la fouiller.");
        return result;
    }

    const int estimatedValue = estimatedVaultTheftValueUnits(*target);
    result.economyCost = std::clamp(estimatedValue / 4, 20, 450);
    if (inventory.getTotalCopper() < Money::copperFromEconomyUnits(result.economyCost))
    {
        result.available = false;
        result.lines.push_back("Le contact accepterait peut-être une restitution discrète, mais tu n'as pas les fonds nécessaires pour cette proposition.");
        result.lines.push_back("Montant de négociation estimé : " + Money::formatEconomyUnits(result.economyCost) + ".");
        return result;
    }

    ++target->negotiationAttempts;
    if (persistentContact != nullptr)
    {
        ++persistentContact->encounters;
        ++persistentContact->negotiations;
        persistentContact->lastInteractionDay = worldDaysElapsed;
    }
    advanceWorldDayUnits(1);
    target->lastHolderInteractionDay = worldDaysElapsed;
    int chance = 30 + target->clues * 8 + target->holderObservationCount * 5;
    if (target->holderType == "receiver") chance += 10;
    if (target->holderAlerted) chance -= 15;
    chance -= std::max(0, target->negotiationAttempts - 1) * 4;
    chance = std::clamp(chance, 35, 88);
    result.success = random.between(1, 100) <= chance;

    if (result.success && inventory.spendEconomyUnits(result.economyCost))
    {
        result.propertyRecovered = recoverVaultTheftCaseProperty(target->caseId);
        if (persistentContact != nullptr) persistentContact->cooperation = std::clamp(persistentContact->cooperation + 15, -100, 100);
        result.lines.push_back("Après discussion, " + target->holderLabel + " accepte de rendre ce qui est encore traçable contre " + Money::formatEconomyUnits(result.economyCost) + ".");
        result.lines.push_back("Ce n'est ni une confession automatique ni la preuve que ce contact a commis le cambriolage : seulement une restitution négociée du bien qu'il détenait.");
        recordCanonicalEvent("restitutions_negociees_pillage", target->caseId, "Restitution négociée avec " + target->holderLabel, 1);
    }
    else
    {
        if (target->negotiationAttempts >= 2) target->holderAlerted = true;
        if (persistentContact != nullptr) persistentContact->pressure = std::clamp(persistentContact->pressure + 5, 0, 100);
        result.lines.push_back(target->holderLabel + " refuse la proposition et coupe court à l'échange.");
        result.lines.push_back("Aucun argent n'est prélevé sur un accord refusé. La tentative consomme seulement du temps et peut rendre le contact plus méfiant.");
        recordCanonicalEvent("negociations_pillage_refusees", target->caseId, "Restitution refusée par " + target->holderLabel, 1);
    }
    result.cluesAfter = target->clues;
    return result;
}

void Player::setLoadedVaultTheftCases(const std::vector<PlayerVaultTheftCase>& cases)
{
    vaultTheftCases.clear();
    for (PlayerVaultTheftCase c : cases)
    {
        if (c.caseId.empty() || c.cityId.empty()) continue;
        c.createdDay = std::max(0, c.createdDay);
        c.lastLeadDay = std::max(0, c.lastLeadDay);
        c.clues = std::clamp(c.clues, 0, 6);
        c.handoffCount = std::max(0, c.handoffCount);
        c.holderObservationCount = std::max(0, c.holderObservationCount);
        c.negotiationAttempts = std::max(0, c.negotiationAttempts);
        c.confrontationAttempts = std::max(0, c.confrontationAttempts);
        c.holderPressure = std::clamp(c.holderPressure, 0, 100);
        c.holderCooperation = std::clamp(c.holderCooperation, -100, 100);
        c.holderClosedUntilDay = std::max(-1, c.holderClosedUntilDay);
        c.lastHolderInteractionDay = std::max(-1, c.lastHolderInteractionDay);
        c.stolenCopper = std::max<long long>(0, c.stolenCopper);
        c.stolenMaterialQuantity = std::max(0, c.stolenMaterialQuantity);
        vaultTheftCases.push_back(c);
    }
}

bool Player::resolveCurrentCityVaultRaid(bool defendedSuccessfully)
{
    loadCurrentCityVaultRecord();
    if (!hasCityVault() || !cityVaultRaidPending) return false;
    if (defendedSuccessfully)
    {
        recordCanonicalEvent("defenses_coffres_reussies", currentCityId, "Coffre défendu", 1);
    }
    else
    {
        recordCanonicalEvent("defenses_coffres_echouees", currentCityId, "Défense du coffre échouée", 1);
        const long long storedCopper = cityVault.getTotalCopper();
        if (storedCopper > 0)
        {
            const long long stolenCopper = std::min<long long>(storedCopper, std::max<long long>(1, storedCopper / 20));
            cityVault.spendCopper(stolenCopper);
            recordCanonicalEvent("pertes_pillage_coffre", currentCityId, "Monnaie volée après une défense échouée", static_cast<int>(std::min<long long>(stolenCopper, 2147483647LL)));
        }
        const std::string stolenMaterial = stealPrioritizedCommonVaultMaterial(cityVault, false);
        if (!stolenMaterial.empty())
        {
            recordCanonicalEvent("objets_voles_pillage_coffre", currentCityId, "Stock commun volé : " + stolenMaterial, 1);
        }
    }
    cityVaultRaidPending = false;
    cityVaultRaidDeadlineDay = -1;
    cityVaultLastRaidCheckDay = worldDaysElapsed;
    syncCurrentCityVaultRecord();
    return true;
}

int Player::getCurrentCityVaultPurchaseDiscountTier() const
{
    if (getOwnedCityVaultCount() > 0) return 0;

    // A mayoral rebate is civic recognition, not a way to hand a near-free vault to a character
    // who has never even reached one platinum worth of liquid wealth in total. Guild deposits count
    // because the requirement is economic standing, not the physical denomination carried in a pocket.
    const long long liquidCopper = inventory.getTotalCopper() + GuildFinanceSystem::depositedBalanceCopper(*this);
    if (liquidCopper < Money::copperFromEconomyUnits(1000)) return 0;

    const LocalReputationResult reputation = LocalReputationSystem::evaluate(*this, currentCityId);
    int completedLocalMissions = 0;
    int importantLocalMissions = 0;
    for (const PlayerJournalRecord& record : canonicalJournalRecords)
    {
        if (record.locationId != currentCityId || record.count <= 0) continue;
        if (record.category == "quetes_reussies" || record.category == "services_locaux_reussis")
        {
            completedLocalMissions += record.count;
        }
        else if (record.category == "quetes_personnage_important")
        {
            importantLocalMissions += record.count;
        }
    }

    const bool civicServiceEnough = completedLocalMissions >= 15 || importantLocalMissions >= 4;
    if (!civicServiceEnough) return 0;
    if (reputation.score >= 80) return 2;
    if (reputation.score >= 45) return 1;
    return 0;
}

std::string Player::getCurrentCityVaultPurchaseDiscountLabel() const
{
    const int tier = getCurrentCityVaultPurchaseDiscountTier();
    if (tier >= 2) return "Ristourne exceptionnelle du maire : 1,25 PP.";
    if (tier >= 1) return "Ristourne du maire : 1,5 PP.";
    return "Tarif municipal normal : 2 PP, puis surcoûts cumulatifs +2 PO, +3 PO, +4 PO, +5 PO... pour chaque implantation supplémentaire.";
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
    return calculateVaultUsedSlots(cityVault, cityVaultPlaceOptimized, cityVaultPreparedStorage);
}

bool Player::hasCityVaultPlaceOptimization() const { return hasCityVault() && cityVaultPlaceOptimized; }
bool Player::hasCityVaultPreparedStorage() const { return hasCityVault() && cityVaultPreparedStorage; }
int Player::getCityVaultStorageReductionPercent() const
{
    return (hasCityVaultPlaceOptimization() ? 10 : 0) + (hasCityVaultPreparedStorage() ? 15 : 0);
}

int Player::getCityVaultPlaceOptimizationCost() const
{
    return 1000;
}

int Player::getCityVaultPreparedStorageCost() const
{
    return 2500;
}

bool Player::canBuyCityVaultPlaceOptimization() const
{
    return hasCityVault() && cityVaultLevel >= 2 && !cityVaultPlaceOptimized;
}

bool Player::canBuyCityVaultPreparedStorage() const
{
    return hasCityVault() && cityVaultLevel >= 5 && cityVaultPlaceOptimized && !cityVaultPreparedStorage;
}

bool Player::buyCityVaultPlaceOptimization()
{
    loadCurrentCityVaultRecord();
    if (!canBuyCityVaultPlaceOptimization() || !inventory.spendEconomyUnits(getCityVaultPlaceOptimizationCost())) return false;
    cityVaultPlaceOptimized = true;
    recordCanonicalEvent("ameliorations_coffres", currentCityId + ":place_optimisee", "Place optimisée");
    grantTitle("Chaque chose à sa place");
    syncCurrentCityVaultRecord();
    refreshCarryAndStorageTitles();
    return true;
}

bool Player::buyCityVaultPreparedStorage()
{
    loadCurrentCityVaultRecord();
    if (!canBuyCityVaultPreparedStorage() || !inventory.spendEconomyUnits(getCityVaultPreparedStorageCost())) return false;
    cityVaultPreparedStorage = true;
    recordCanonicalEvent("ameliorations_coffres", currentCityId + ":rangement_anticipe", "Rangement anticipé");
    grantTitle("Optimisation maximale");
    syncCurrentCityVaultRecord();
    refreshCarryAndStorageTitles();
    return true;
}

int Player::getCityVaultPurchaseCost() const
{
    return EconomyBalance::cityVaultPurchaseCost(currentCityId, getOwnedCityVaultCount(), getCurrentCityVaultPurchaseDiscountTier());
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
    cityVaultPlaceOptimized = false;
    cityVaultPreparedStorage = false;
    cityVaultSecurityDaysRemaining = 0;
    cityVaultRaidPending = false;
    cityVaultRaidDeadlineDay = -1;
    cityVaultLastRaidCheckDay = worldDaysElapsed;
    recordCanonicalEvent("coffres_achetes", currentCityId, "Coffre municipal de " + currentCityId);
    grantTitle("Enfin de la place");
    syncCurrentCityVaultRecord();
    refreshCarryAndStorageTitles();
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
    grantTitle("On pousse les murs");
    syncCurrentCityVaultRecord();
    refreshCarryAndStorageTitles();
    return true;
}

bool Player::depositWeaponInCityVault(int index)
{
    loadCurrentCityVaultRecord();
    if (!hasCityVault() || !inventory.hasWeapon(index) || index == equippedWeaponIndex)
    {
        return false;
    }
    const Weapon weapon = inventory.getWeapon(index);
    Inventory previewVault = cityVault;
    previewVault.addWeapon(weapon);
    if (calculateVaultUsedSlots(previewVault, cityVaultPlaceOptimized, cityVaultPreparedStorage) > getCityVaultCapacity())
    {
        return false;
    }
    if (!inventory.removeWeapon(index))
    {
        return false;
    }
    if (equippedWeaponIndex > index)
    {
        --equippedWeaponIndex;
    }
    cityVault.addWeapon(weapon);
    recordCanonicalEvent("objets_deposes", vaultWeaponSubjectId(weapon), weapon.getName());
    syncCurrentCityVaultRecord();
    refreshCarryAndStorageTitles();
    return true;
}

bool Player::depositArmorInCityVault(int index)
{
    loadCurrentCityVaultRecord();
    if (!hasCityVault() || !inventory.hasArmor(index) || index == equippedArmorIndex)
    {
        return false;
    }
    const Armor armor = inventory.getArmor(index);
    Inventory previewVault = cityVault;
    previewVault.addArmor(armor);
    if (calculateVaultUsedSlots(previewVault, cityVaultPlaceOptimized, cityVaultPreparedStorage) > getCityVaultCapacity())
    {
        return false;
    }
    if (armor.getName() == "Tenue simple" || !inventory.removeArmor(index))
    {
        return false;
    }
    if (equippedArmorIndex > index)
    {
        --equippedArmorIndex;
    }
    cityVault.addArmor(armor);
    recordCanonicalEvent("objets_deposes", vaultArmorSubjectId(armor), armor.getName());
    syncCurrentCityVaultRecord();
    refreshCarryAndStorageTitles();
    return true;
}

bool Player::depositConsumableInCityVault(int index)
{
    loadCurrentCityVaultRecord();
    if (!hasCityVault() || !inventory.hasConsumable(index))
    {
        return false;
    }
    const Consumable consumable = inventory.getConsumable(index);
    Inventory previewVault = cityVault;
    previewVault.addConsumable(consumable);
    if (calculateVaultUsedSlots(previewVault, cityVaultPlaceOptimized, cityVaultPreparedStorage) > getCityVaultCapacity())
    {
        return false;
    }
    if (!inventory.removeConsumable(index))
    {
        return false;
    }
    cityVault.addConsumable(consumable);
    recordCanonicalEvent("objets_deposes", "consumable:" + consumable.getName(), consumable.getName());
    syncCurrentCityVaultRecord();
    refreshCarryAndStorageTitles();
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
    (void)sameStackExists;
    Inventory previewVault = cityVault;
    Material previewMaterial = material;
    previewMaterial.setQuantity(quantity);
    previewVault.addMaterial(previewMaterial);
    if (calculateVaultUsedSlots(previewVault, cityVaultPlaceOptimized, cityVaultPreparedStorage) > getCityVaultCapacity())
    {
        return false;
    }

    material.setQuantity(quantity);
    if (!inventory.removeMaterialQuantity(index, quantity))
    {
        return false;
    }
    cityVault.addMaterial(material);
    recordCanonicalEvent("materiaux_deposes", vaultMaterialSubjectId(material), material.getName(), quantity);
    syncCurrentCityVaultRecord();
    refreshCarryAndStorageTitles();
    return true;
}

bool Player::depositAllCoinsInCityVault()
{
    loadCurrentCityVaultRecord();
    if (!hasCityVault()) return false;
    const CoinBreakdown wallet = inventory.getCoinStacks();
    const long long pieces = std::max(0LL, wallet.platinum) + std::max(0LL, wallet.gold)
        + std::max(0LL, wallet.electrum) + std::max(0LL, wallet.iron) + std::max(0LL, wallet.copper);
    if (pieces <= 0) return false;

    Inventory preview = cityVault;
    preview.earnCoinStacks(wallet);
    if (calculateVaultUsedSlots(preview, cityVaultPlaceOptimized, cityVaultPreparedStorage) > getCityVaultCapacity()) return false;

    inventory.setCoinStacks(CoinBreakdown{});
    cityVault.earnCoinStacks(wallet);
    recordCanonicalEvent("monnaie_deposee_coffre", currentCityId, "Monnaie physique déposée au coffre", static_cast<int>(std::min<long long>(pieces, 2000000000LL)));
    syncCurrentCityVaultRecord();
    refreshCarryAndStorageTitles();
    return true;
}

bool Player::withdrawAllCoinsFromCityVault()
{
    loadCurrentCityVaultRecord();
    if (!hasCityVault()) return false;
    const CoinBreakdown stored = cityVault.getCoinStacks();
    const long long pieces = std::max(0LL, stored.platinum) + std::max(0LL, stored.gold)
        + std::max(0LL, stored.electrum) + std::max(0LL, stored.iron) + std::max(0LL, stored.copper);
    if (pieces <= 0) return false;
    inventory.earnCoinStacks(stored);
    cityVault.setCoinStacks(CoinBreakdown{});
    recordCanonicalEvent("monnaie_retiree_coffre", currentCityId, "Monnaie physique retirée du coffre", static_cast<int>(std::min<long long>(pieces, 2000000000LL)));
    syncCurrentCityVaultRecord();
    refreshCarryAndStorageTitles();
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
    recordCanonicalEvent("objets_retires", vaultWeaponSubjectId(weapon), weapon.getName());
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
    recordCanonicalEvent("objets_retires", vaultArmorSubjectId(armor), armor.getName());
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
    recordCanonicalEvent("materiaux_retires", vaultMaterialSubjectId(material), material.getName(), quantity);
    syncCurrentCityVaultRecord();
    return true;
}

bool Player::transferMaterialBetweenCityVaults(const std::string& destinationCityId, int materialIndex, int quantity, int costCopper)
{
    return transferMaterialBetweenOwnedCityVaults(currentCityId, destinationCityId, materialIndex, quantity, costCopper);
}

bool Player::transferMaterialBetweenOwnedCityVaults(const std::string& sourceCityId, const std::string& destinationCityId, int materialIndex, int quantity, int costCopper)
{
    if (sourceCityId.empty() || destinationCityId.empty() || sourceCityId == destinationCityId || costCopper < 0) return false;

    syncCurrentCityVaultRecord();
    if (!hasCityVaultInCity(sourceCityId) || !hasCityVaultInCity(destinationCityId)) return false;
    PlayerCityVault* sourceRecord = findCityVaultRecord(sourceCityId);
    PlayerCityVault* destinationRecord = findCityVaultRecord(destinationCityId);
    if (sourceRecord == nullptr || destinationRecord == nullptr || !sourceRecord->inventory.hasMaterial(materialIndex)) return false;

    Material material = sourceRecord->inventory.getMaterial(materialIndex);
    if (quantity <= 0) quantity = material.getQuantity();
    quantity = std::min(quantity, material.getQuantity());
    if (quantity <= 0) return false;

    const int destinationCapacity = calculateVaultCapacity(destinationRecord->purchased, destinationRecord->level);
    Inventory destinationPreview = destinationRecord->inventory;
    Material previewMaterial = material;
    previewMaterial.setQuantity(quantity);
    destinationPreview.addMaterial(previewMaterial);
    if (calculateVaultUsedSlots(destinationPreview, destinationRecord->placeOptimized, destinationRecord->preparedStorage) > destinationCapacity) return false;

    if (!inventory.spendCopper(costCopper)) return false;
    if (!sourceRecord->inventory.removeMaterialQuantity(materialIndex, quantity))
    {
        inventory.earnCopper(costCopper);
        return false;
    }

    material.setQuantity(quantity);
    destinationRecord->inventory.addMaterial(material);
    recordCanonicalEvent("transports_coffres_municipaux", sourceCityId + "->" + destinationCityId, "Transport de coffre municipal", 1);
    recordCanonicalEvent("materiaux_transportes_coffres", vaultMaterialSubjectId(material), material.getName(), quantity);
    recordCanonicalEvent("couts_transport_coffres", destinationCityId, "Transport entre coffres municipaux", costCopper);
    loadCurrentCityVaultRecord();
    return true;
}


bool Player::transferWeaponBetweenOwnedCityVaults(const std::string& sourceCityId, const std::string& destinationCityId, int weaponIndex, int costCopper)
{
    if (sourceCityId.empty() || destinationCityId.empty() || sourceCityId == destinationCityId || costCopper < 0) return false;
    syncCurrentCityVaultRecord();
    if (!hasCityVaultInCity(sourceCityId) || !hasCityVaultInCity(destinationCityId)) return false;
    PlayerCityVault* sourceRecord = findCityVaultRecord(sourceCityId);
    PlayerCityVault* destinationRecord = findCityVaultRecord(destinationCityId);
    if (sourceRecord == nullptr || destinationRecord == nullptr || !sourceRecord->inventory.hasWeapon(weaponIndex)) return false;

    const Weapon weapon = sourceRecord->inventory.getWeapon(weaponIndex);
    Inventory preview = destinationRecord->inventory;
    preview.addWeapon(weapon);
    if (calculateVaultUsedSlots(preview, destinationRecord->placeOptimized, destinationRecord->preparedStorage)
        > calculateVaultCapacity(destinationRecord->purchased, destinationRecord->level)) return false;
    if (!inventory.spendCopper(costCopper)) return false;
    if (!sourceRecord->inventory.removeWeapon(weaponIndex)) { inventory.earnCopper(costCopper); return false; }
    destinationRecord->inventory.addWeapon(weapon);
    recordCanonicalEvent("transports_coffres_municipaux", sourceCityId + "->" + destinationCityId, "Transport d'arme entre coffres", 1);
    recordCanonicalEvent("armes_transportees_coffres", vaultWeaponSubjectId(weapon), weapon.getName(), 1);
    recordCanonicalEvent("couts_transport_coffres", destinationCityId, "Transport entre coffres municipaux", costCopper);
    loadCurrentCityVaultRecord();
    return true;
}

bool Player::transferArmorBetweenOwnedCityVaults(const std::string& sourceCityId, const std::string& destinationCityId, int armorIndex, int costCopper)
{
    if (sourceCityId.empty() || destinationCityId.empty() || sourceCityId == destinationCityId || costCopper < 0) return false;
    syncCurrentCityVaultRecord();
    if (!hasCityVaultInCity(sourceCityId) || !hasCityVaultInCity(destinationCityId)) return false;
    PlayerCityVault* sourceRecord = findCityVaultRecord(sourceCityId);
    PlayerCityVault* destinationRecord = findCityVaultRecord(destinationCityId);
    if (sourceRecord == nullptr || destinationRecord == nullptr || !sourceRecord->inventory.hasArmor(armorIndex)) return false;

    const Armor armor = sourceRecord->inventory.getArmor(armorIndex);
    Inventory preview = destinationRecord->inventory;
    preview.addArmor(armor);
    if (calculateVaultUsedSlots(preview, destinationRecord->placeOptimized, destinationRecord->preparedStorage)
        > calculateVaultCapacity(destinationRecord->purchased, destinationRecord->level)) return false;
    if (!inventory.spendCopper(costCopper)) return false;
    if (!sourceRecord->inventory.removeArmor(armorIndex)) { inventory.earnCopper(costCopper); return false; }
    destinationRecord->inventory.addArmor(armor);
    recordCanonicalEvent("transports_coffres_municipaux", sourceCityId + "->" + destinationCityId, "Transport d'armure entre coffres", 1);
    recordCanonicalEvent("armures_transportees_coffres", vaultArmorSubjectId(armor), armor.getName(), 1);
    recordCanonicalEvent("couts_transport_coffres", destinationCityId, "Transport entre coffres municipaux", costCopper);
    loadCurrentCityVaultRecord();
    return true;
}

bool Player::transferConsumableBetweenOwnedCityVaults(const std::string& sourceCityId, const std::string& destinationCityId, int consumableIndex, int costCopper)
{
    if (sourceCityId.empty() || destinationCityId.empty() || sourceCityId == destinationCityId || costCopper < 0) return false;
    syncCurrentCityVaultRecord();
    if (!hasCityVaultInCity(sourceCityId) || !hasCityVaultInCity(destinationCityId)) return false;
    PlayerCityVault* sourceRecord = findCityVaultRecord(sourceCityId);
    PlayerCityVault* destinationRecord = findCityVaultRecord(destinationCityId);
    if (sourceRecord == nullptr || destinationRecord == nullptr || !sourceRecord->inventory.hasConsumable(consumableIndex)) return false;

    const Consumable consumable = sourceRecord->inventory.getConsumable(consumableIndex);
    Inventory preview = destinationRecord->inventory;
    preview.addConsumable(consumable);
    if (calculateVaultUsedSlots(preview, destinationRecord->placeOptimized, destinationRecord->preparedStorage)
        > calculateVaultCapacity(destinationRecord->purchased, destinationRecord->level)) return false;
    if (!inventory.spendCopper(costCopper)) return false;
    if (!sourceRecord->inventory.removeConsumable(consumableIndex)) { inventory.earnCopper(costCopper); return false; }
    destinationRecord->inventory.addConsumable(consumable);
    recordCanonicalEvent("transports_coffres_municipaux", sourceCityId + "->" + destinationCityId, "Transport de consommable entre coffres", 1);
    recordCanonicalEvent("consommables_transportes_coffres", consumable.getName(), consumable.getName(), 1);
    recordCanonicalEvent("couts_transport_coffres", destinationCityId, "Transport entre coffres municipaux", costCopper);
    loadCurrentCityVaultRecord();
    return true;
}

bool Player::transferAllCoinsBetweenOwnedCityVaults(const std::string& sourceCityId, const std::string& destinationCityId, int costCopper)
{
    if (sourceCityId.empty() || destinationCityId.empty() || sourceCityId == destinationCityId || costCopper < 0) return false;
    syncCurrentCityVaultRecord();
    if (!hasCityVaultInCity(sourceCityId) || !hasCityVaultInCity(destinationCityId)) return false;
    PlayerCityVault* sourceRecord = findCityVaultRecord(sourceCityId);
    PlayerCityVault* destinationRecord = findCityVaultRecord(destinationCityId);
    if (sourceRecord == nullptr || destinationRecord == nullptr || sourceRecord->inventory.getTotalCopper() <= 0) return false;

    const CoinBreakdown coins = sourceRecord->inventory.getCoinStacks();
    Inventory preview = destinationRecord->inventory;
    preview.earnCoinStacks(coins);
    if (calculateVaultUsedSlots(preview, destinationRecord->placeOptimized, destinationRecord->preparedStorage)
        > calculateVaultCapacity(destinationRecord->purchased, destinationRecord->level)) return false;
    if (!inventory.spendCopper(costCopper)) return false;
    sourceRecord->inventory.setCoinStacks(CoinBreakdown{});
    destinationRecord->inventory.earnCoinStacks(coins);
    recordCanonicalEvent("transports_coffres_municipaux", sourceCityId + "->" + destinationCityId, "Transport de monnaie entre coffres", 1);
    recordCanonicalEvent("monnaie_transportee_coffres", destinationCityId, "Monnaie physique convoyée", static_cast<int>(std::min<long long>(coins.platinum + coins.gold + coins.electrum + coins.iron + coins.copper, 2147483647LL)));
    recordCanonicalEvent("couts_transport_coffres", destinationCityId, "Transport entre coffres municipaux", costCopper);
    loadCurrentCityVaultRecord();
    return true;
}



void Player::processCityVaultSecurityEndOfDay(std::vector<std::string>& dayLines)
{
    syncCurrentCityVaultRecord();

    static std::mt19937 generator(std::random_device{}());
    std::uniform_int_distribution<int> raidRoll(1, 1000);

    for (PlayerCityVault& record : cityVaults)
    {
        if (!record.purchased || record.level <= 0 || record.cityId.empty()) continue;
        if (record.cityId == currentCityId) continue; // Player watches/maintains the local vault directly.

        const City* city = City::findById(record.cityId);
        const std::string cityName = city != nullptr ? city->getName() : record.cityId;

        if (record.securityDaysRemaining > 0)
        {
            --record.securityDaysRemaining;
            if (record.securityDaysRemaining == 0)
            {
                dayLines.push_back("Sécurité du coffre de " + cityName + " : la couverture payée vient de s'achever.");
            }
            continue;
        }

        if (record.raidPending)
        {
            if (record.raidDeadlineDay >= 0 && worldDaysElapsed >= record.raidDeadlineDay)
            {
                PlayerVaultTheftCase theftCase;
                theftCase.caseId = "vault_theft:" + record.cityId + ":day" + std::to_string(worldDaysElapsed) + ":" + std::to_string(vaultTheftCases.size() + 1);
                theftCase.cityId = record.cityId;
                theftCase.createdDay = worldDaysElapsed;
                theftCase.lastLeadDay = worldDaysElapsed;
                theftCase.holderType = "unknown";
                theftCase.holderLabel = "auteur inconnu";
                theftCase.holderWorldContactId.clear();
                theftCase.holderLocationId.clear();
                theftCase.holderLocationLabel.clear();
                theftCase.holderExplorationBiome.clear();
                const bool stoleSomething = createVaultTheftCaseFromInventory(theftCase, record.inventory, true);
                if (stoleSomething) vaultTheftCases.push_back(theftCase);
                record.raidPending = false;
                record.raidDeadlineDay = -1;
                record.lastRaidCheckDay = worldDaysElapsed;
                recordCanonicalEvent("pillages_coffres_non_defendus", record.cityId, "Pillage non défendu du coffre", 1);
                if (stoleSomething)
                {
                    recordCanonicalEvent("dossiers_enquete_pillage", theftCase.caseId, "Cambriolage à auteur inconnu — enquête disponible", 1);
                    recordHistoricalEvent("vault_theft_unknown", theftCase.caseId, "Le coffre de " + cityName + " a été cambriolé pendant ton absence. L'auteur n'est pas identifié.");
                }
                std::string lossLine = "Coffre de " + cityName + " : délai dépassé. Des biens ont été emportés pendant ton absence.";
                if (stoleSomething)
                {
                    lossLine += " L'auteur reste inconnu : des témoins, traces ou circuits de revente peuvent permettre une enquête locale.";
                    if (theftCase.hasWeapon || theftCase.hasArmor) lossLine += " Au moins un équipement manque : la piste mérite probablement d'être suivie rapidement.";
                }
                else lossLine += " Les assaillants n'ont finalement rien réussi à emporter.";
                lossLine += " Les stocks abondants et ordinaires sont les cibles les plus faciles, mais un objet précieux transférable peut aussi disparaître s'il est découvert ; seuls les biens réellement liés à l'âme/non transférables sont immunisés.";
                dayLines.push_back(lossLine);
            }
            continue;
        }

        if (record.lastRaidCheckDay == worldDaysElapsed) continue;
        record.lastRaidCheckDay = worldDaysElapsed;
        const bool hasSomethingWorthTargeting = calculateVaultUsedSlots(record.inventory, record.placeOptimized, record.preparedStorage) > 0;
        if (!hasSomethingWorthTargeting) continue;

        // 1.5% per remote unprotected day: about one alert every 67 exposed days on average,
        // always random and always preceded by a full 7-day warning.
        if (raidRoll(generator) <= 15)
        {
            record.raidPending = true;
            record.raidDeadlineDay = worldDaysElapsed + 7;
            recordCanonicalEvent("alertes_pillage_coffres", record.cityId, "Tentative de pillage du coffre", 1);
            dayLines.push_back("Messager urgent de " + cityName + " : des voleurs visent ton coffre. Tu as 7 jours pour revenir et utiliser l'option Défendre.");
        }
    }

    // Stolen property remains in the world. If the player waits, it may pass through accomplices,
    // receivers or a cache, which changes the investigation without deleting the property.
    std::uniform_int_distribution<int> handoffRoll(1, 100);
    for (PlayerVaultTheftCase& theftCase : vaultTheftCases)
    {
        if (theftCase.resolved || worldDaysElapsed - theftCase.lastLeadDay < 5) continue;
        const int chance = theftCase.culpritIdentified ? 6 : 10;
        if (handoffRoll(generator) > chance) continue;
        ++theftCase.handoffCount;
        theftCase.lastLeadDay = worldDaysElapsed;
        if (theftCase.handoffCount % 3 == 1) theftCase.holderType = "accomplice";
        else if (theftCase.handoffCount % 3 == 2) theftCase.holderType = "receiver";
        else theftCase.holderType = "cache";
        theftCase.holderWorldContactId.clear();
        theftCase.holderLocationId.clear();
        theftCase.holderLocationLabel.clear();
        theftCase.holderExplorationBiome.clear();
        theftCase.holderObservationCount = 0;
        theftCase.negotiationAttempts = 0;
        theftCase.confrontationAttempts = 0;
        theftCase.holderPressure = 0;
        theftCase.holderCooperation = 0;
        theftCase.holderClosedUntilDay = -1;
        theftCase.holderReportedToGuard = false;
        theftCase.holderAlerted = false;
        bindVaultTheftCaseToWorldHolder(theftCase);
        theftCase.culpritIdentified = false;
        theftCase.identifiedRivalId.clear();
        theftCase.clues = std::max(0, theftCase.clues - 1);
        recordCanonicalEvent("circulation_biens_voles", theftCase.caseId, "Les biens volés ont changé de main ou de cache", 1);
    }
}

int Player::getBackpackTier() const { return std::clamp(backpackTier, 0, 4); }
int Player::getBackpackBaseCapacity() const { return CarryLoadSystem::backpackBaseCapacity(getBackpackTier()); }
int Player::getNextBackpackUpgradeCost() const
{
    static const int costs[] = {120, 280, 520, 900};
    return canUpgradeBackpack() ? costs[getBackpackTier()] : 0;
}
bool Player::canUpgradeBackpack() const { return getBackpackTier() < 4; }
bool Player::upgradeBackpack()
{
    if (!canUpgradeBackpack()) return false;
    const int cost = getNextBackpackUpgradeCost();
    if (cost <= 0 || !inventory.spendEconomyUnits(cost)) return false;
    ++backpackTier;
    recordCanonicalEvent("sacs_achetes", "sac_niveau_" + std::to_string(backpackTier), "Sac à dos niveau " + std::to_string(backpackTier));
    if (backpackTier >= 4)
    {
        grantTitle("Prêt pour le long voyage");
        grantTitle("Collection de sacs");
    }
    refreshCarryAndStorageTitles();
    return true;
}
void Player::setLoadedBackpackTier(int tier) { backpackTier = std::clamp(tier, 0, 4); }

void Player::evaluateStorageSessionTitles(int carriedLoadBefore, int capacityBefore)
{
    const CarryLoadSummary after = CarryLoadSystem::evaluate(*this);
    const int beforePercent = capacityBefore > 0
        ? static_cast<int>((static_cast<long long>(std::max(0, carriedLoadBefore)) * 100) / capacityBefore)
        : 0;

    if (beforePercent >= 90
        && carriedLoadBefore > 0
        && after.used * 100 <= carriedLoadBefore * 40)
    {
        recordCanonicalEvent(
            "sessions_grand_rangement",
            getCurrentCityId(),
            "Grande majorité du sac déchargée pendant une vraie session de coffre",
            1
        );
        grantTitle("Grand ménage");
    }

    if (beforePercent > 150 && after.percent < 50)
    {
        recordCanonicalEvent(
            "sessions_rangement_radical",
            getCurrentCityId(),
            "Charge >150 % ramenée sous 50 % par stockage réel",
            1
        );
        grantTitle("Ça respire enfin");
    }

    refreshCarryAndStorageTitles();
}

void Player::evaluateExplorationReturnTitles(int expeditionPeakCarryPercent, int expeditionPeakHeavyEquipment)
{
    const CarryLoadSummary returnLoad = CarryLoadSystem::evaluate(*this);
    if (returnLoad.percent >= 180
        && returnLoad.percent < 200
        && expeditionPeakCarryPercent < 200
        && getLootPickupPolicy() == "all")
    {
        recordCanonicalEvent(
            "retours_ville_charge_extreme",
            getCurrentCityId(),
            "Retour d'exploration presque immobilisé avec ramassage automatique complet",
            1
        );
        grantTitle("Je garde tout");
    }

    if (expeditionPeakHeavyEquipment >= 4 && expeditionPeakCarryPercent < 200)
    {
        recordCanonicalEvent(
            "sorties_equipement_lourd",
            getCurrentCityId(),
            "Au moins quatre équipements lourds de réserve transportés sur une même expédition",
            expeditionPeakHeavyEquipment
        );
        grantTitle("Déménageur");
    }
}

void Player::refreshCarryAndStorageTitles()
{
    const CarryLoadSummary load = CarryLoadSystem::evaluate(*this);
    if (load.percent >= 100) grantTitle("Plein à craquer");
    if (load.percent >= 200) grantTitle("Ça rentre encore.");
    if (getBackpackTier() >= 4 && load.percent >= 100) grantTitle("Même ça, j'ai rempli");

    const std::string morphology = CarryLoadSystem::morphologyLabel(*this);
    if (morphology == "Robuste" && getBackpackTier() >= 4) grantTitle("Bête de somme");
    if (morphology == "Robuste" && load.percent >= 150 && load.percent < 200) grantTitle("Tout sur le dos");
    if (morphology == "Fine" && load.percent < 25)
    {
        switch (getRace())
        {
            case CharacterRace::Fairy:
            case CharacterRace::Gnome:
            case CharacterRace::Halfling:
                grantTitle("Ça flotte presque");
                break;
            default: break;
        }
    }

    const CoinBreakdown coins = inventory.getCoinStacks();
    const long long physicalCoins = std::max(0LL, coins.platinum) + std::max(0LL, coins.gold)
        + std::max(0LL, coins.electrum) + std::max(0LL, coins.iron) + std::max(0LL, coins.copper);
    if (physicalCoins >= 5000) grantTitle("Je préfère le tangible");

    // Keep the current city record synchronized before aggregating the local network.
    syncCurrentCityVaultRecord();
    int purchasedVaults = 0;
    int maxLevelVaults = 0;
    int storedWeapons = 0;
    int storedArmors = 0;
    long long storedMaterials = 0;
    long long storedPhysicalCoins = 0;
    std::vector<std::string> distinctMaterials;
    for (const PlayerCityVault& record : cityVaults)
    {
        if (!record.purchased) continue;
        ++purchasedVaults;
        if (record.level >= 5) ++maxLevelVaults;
        storedWeapons += record.inventory.getWeaponCount();
        storedArmors += record.inventory.getArmorCount();
        const CoinBreakdown vaultCoins = record.inventory.getCoinStacks();
        storedPhysicalCoins += std::max(0LL, vaultCoins.platinum) + std::max(0LL, vaultCoins.gold)
            + std::max(0LL, vaultCoins.electrum) + std::max(0LL, vaultCoins.iron) + std::max(0LL, vaultCoins.copper);
        for (const Material& material : record.inventory.getMaterials())
        {
            storedMaterials += std::max(0, material.getQuantity());
            if (std::find(distinctMaterials.begin(), distinctMaterials.end(), material.getId()) == distinctMaterials.end())
                distinctMaterials.push_back(material.getId());
        }
    }
    if (purchasedVaults >= 3) grantTitle("Réseau de stockage");
    if (purchasedVaults >= 5) grantTitle("Empire logistique");
    if (purchasedVaults >= 8) grantTitle("Partout chez moi");
    if (maxLevelVaults >= 3) grantTitle("Collectionneur itinérant");
    if (storedWeapons >= 100) grantTitle("Armurier malgré moi");
    if (storedArmors >= 50) grantTitle("Une pour chaque occasion");
    if (storedMaterials >= 1000) grantTitle("Ça peut toujours servir");
    if (distinctMaterials.size() >= 30) grantTitle("Réserve stratégique");
    if (storedPhysicalCoins >= 10000) grantTitle("Sous le matelas");

    if (hasCityVault())
    {
        const int used = getCityVaultUsedSlots();
        const int capacity = getCityVaultCapacity();
        if (capacity > 0 && used >= capacity)
        {
            grantTitle("Pas un espace de perdu");
            if (getCityVaultLevel() >= 5) grantTitle("Conservateur");
        }
    }
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
    const std::vector<PlayerCityVault>& loadedCityVaults,
    bool loadedVaultPlaceOptimized,
    bool loadedVaultPreparedStorage
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
        cleanRecord.purchased = record.purchased;
        cleanRecord.level = record.purchased ? std::max(1, std::min(5, record.level)) : 0;
        cleanRecord.placeOptimized = record.purchased && record.placeOptimized;
        cleanRecord.preparedStorage = record.purchased && cleanRecord.level >= 5 && record.preparedStorage;
        cleanRecord.securityDaysRemaining = record.purchased ? std::max(0, record.securityDaysRemaining) : 0;
        cleanRecord.raidPending = record.purchased && record.raidPending;
        cleanRecord.raidDeadlineDay = cleanRecord.raidPending ? record.raidDeadlineDay : -1;
        cleanRecord.lastRaidCheckDay = record.lastRaidCheckDay;

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
        legacyRecord.purchased = vaultPurchased;
        legacyRecord.level = vaultPurchased ? std::max(1, std::min(5, vaultLevel)) : 0;
        legacyRecord.placeOptimized = vaultPurchased && loadedVaultPlaceOptimized;
        legacyRecord.preparedStorage = vaultPurchased && legacyRecord.level >= 5 && loadedVaultPreparedStorage;
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
