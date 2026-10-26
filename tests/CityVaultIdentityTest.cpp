#include <cassert>
#include <string>

#include "class_system/ClassCatalog.hpp"
#include "entity/Player.hpp"
#include "item/armor/Armor.hpp"
#include "item/inventory/CarryLoadSystem.hpp"
#include "economy/EconomyBalance.hpp"
#include "item/material/Material.hpp"
#include "item/weapon/Weapon.hpp"

namespace
{
    const PlayerJournalRecord* findRecord(const Player& player, const std::string& category, const std::string& key)
    {
        for (const PlayerJournalRecord& record : player.getCanonicalJournalRecords())
        {
            if (record.category == category && record.key == key)
            {
                return &record;
            }
        }
        return nullptr;
    }
}

int main()
{
    Player player("Coffrier", ClassCatalog::createClassByName("Guerrier"));
    player.getInventory().clearAll();
    player.getInventory().earnEconomyUnits(1000000);
    assert(player.purchaseCityVault());

    player.getInventory().addWeapon(Weapon(
        "Épée jumelle",
        "Arme témoin pour le coffre municipal.",
        100,
        WeaponType::Sword,
        2,
        4,
        1,
        80
    ));
    const std::string weaponId = player.getInventory().getWeapon(0).getPersistentId();
    assert(!weaponId.empty());
    assert(player.depositWeaponInCityVault(0));
    assert(player.getInventory().getWeaponCount() == 0);
    assert(player.getCityVault().getWeaponCount() == 1);
    assert(player.getCityVault().getWeapon(0).getPersistentId() == weaponId);
    assert(findRecord(player, "objets_deposes", weaponId) != nullptr);

    assert(player.withdrawWeaponFromCityVault(0));
    assert(player.getCityVault().getWeaponCount() == 0);
    assert(player.getInventory().getWeaponCount() == 1);
    assert(player.getInventory().getWeapon(0).getPersistentId() == weaponId);
    assert(findRecord(player, "objets_retires", weaponId) != nullptr);

    player.getInventory().addArmor(Armor(
        "Cuir jumeau",
        "Armure témoin pour le coffre municipal.",
        90,
        ArmorType::Leather,
        3,
        1,
        75
    ));
    const std::string armorId = player.getInventory().getArmor(0).getPersistentId();
    assert(!armorId.empty());
    assert(player.depositArmorInCityVault(0));
    assert(player.getCityVault().getArmor(0).getPersistentId() == armorId);
    assert(findRecord(player, "objets_deposes", armorId) != nullptr);
    assert(player.withdrawArmorFromCityVault(0));
    assert(player.getInventory().getArmor(0).getPersistentId() == armorId);
    assert(findRecord(player, "objets_retires", armorId) != nullptr);

    player.getInventory().addMaterial(Material(
        "vault_ore",
        "Minerai de coffre",
        "Matériau témoin.",
        "minerai",
        25,
        5,
        "high"
    ));
    assert(player.depositMaterialInCityVault(0, 3));
    assert(findRecord(player, "materiaux_deposes", "material:vault_ore:high") != nullptr);
    assert(player.withdrawMaterialFromCityVault(0, 2));
    assert(findRecord(player, "materiaux_retires", "material:vault_ore:high") != nullptr);

    // V3.51.51: vault capacity now uses the same abstract load units as the carried inventory.
    assert(EconomyBalance::cityVaultCapacityForLevel(1) == 250);
    assert(EconomyBalance::cityVaultCapacityForLevel(2) == 300);
    assert(EconomyBalance::cityVaultCapacityForLevel(3) == 350);
    assert(EconomyBalance::cityVaultCapacityForLevel(4) == 400);
    assert(EconomyBalance::cityVaultCapacityForLevel(5) == 500);
    assert(player.getCityVaultCapacity() == 250);

    // Put enough dense material in storage for the percentage reductions to be observable.
    player.getInventory().addMaterial(Material(
        "vault_dense_bulk",
        "Bloc de minerai de coffre",
        "Pile dense pour vérifier les optimisations du rangement.",
        "minerai",
        1,
        100
    ));
    const int denseIndex = static_cast<int>(player.getInventory().getMaterials().size()) - 1;
    assert(player.depositMaterialInCityVault(denseIndex, 100));
    const int rawVaultLoad = CarryLoadSystem::inventoryLoad(player.getCityVault());
    assert(rawVaultLoad > 20);
    assert(player.getCityVaultUsedSlots() == rawVaultLoad);

    assert(player.upgradeCityVault());
    assert(player.getCityVaultLevel() == 2);
    assert(player.getCityVaultCapacity() == 300);
    assert(player.canBuyCityVaultPlaceOptimization());
    assert(player.buyCityVaultPlaceOptimization());
    assert(player.getCityVaultStorageReductionPercent() == 10);
    assert(player.getCityVaultUsedSlots() == (rawVaultLoad * 90 + 99) / 100);

    while (player.getCityVaultLevel() < 5) assert(player.upgradeCityVault());
    assert(player.getCityVaultCapacity() == 500);
    assert(player.canBuyCityVaultPreparedStorage());
    assert(player.buyCityVaultPreparedStorage());
    assert(player.getCityVaultStorageReductionPercent() == 25);
    // The second optimization is -15 percentage points FROM THE ORIGINAL LOAD, not multiplicative.
    assert(player.getCityVaultUsedSlots() == (rawVaultLoad * 75 + 99) / 100);

    // Physical money occupies little space (100 actual coins = 1 unit) and can live in the vault.
    CoinBreakdown carriedCoins;
    carriedCoins.copper = 101;
    player.getInventory().setCoinStacks(carriedCoins);
    assert(CarryLoadSystem::coinLoad(player.getInventory()) == 2);
    assert(player.depositAllCoinsInCityVault());
    const CoinBreakdown storedCoins = player.getCityVault().getCoinStacks();
    assert(storedCoins.copper == 101);
    assert(player.getInventory().getCoinStacks().copper == 0);
    assert(player.withdrawAllCoinsFromCityVault());
    assert(player.getInventory().getCoinStacks().copper == 101);
    assert(player.getCityVault().getCoinStacks().copper == 0);

    return 0;
}
