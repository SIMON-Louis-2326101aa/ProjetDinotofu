#include <cassert>
#include <string>

#include "class_system/ClassCatalog.hpp"
#include "core/ExchangeTransferSystem.hpp"
#include "entity/Player.hpp"
#include "item/armor/Armor.hpp"
#include "item/consumable/Consumable.hpp"
#include "item/material/Material.hpp"
#include "item/weapon/Weapon.hpp"

namespace
{
    const PlayerHistoricalEvent* findHistoricalEvent(
        const Player& player,
        const std::string& category,
        const std::string& subjectId
    )
    {
        for (auto it = player.getHistoricalEvents().rbegin(); it != player.getHistoricalEvents().rend(); ++it)
        {
            if (it->category == category && it->subjectId == subjectId)
            {
                return &(*it);
            }
        }
        return nullptr;
    }

    const PlayerJournalRecord* findCanonicalRecordContaining(
        const Player& player,
        const std::string& category,
        const std::string& keyFragment
    )
    {
        for (const PlayerJournalRecord& record : player.getCanonicalJournalRecords())
        {
            if (record.category == category && record.key.find(keyFragment) != std::string::npos)
            {
                return &record;
            }
        }
        return nullptr;
    }
}

int main()
{
    Player giver("Donneur", ClassCatalog::createClassByName("Guerrier"));
    Player receiver("Receveur", ClassCatalog::createClassByName("Guerrier"));
    giver.getInventory().clearAll();
    receiver.getInventory().clearAll();

    giver.getInventory().earnEconomyUnits(500);
    assert(ExchangeTransferSystem::transferEconomyUnits(giver, receiver, 125));
    assert(giver.getInventory().getEconomyUnits() == 375);
    assert(receiver.getInventory().getEconomyUnits() == 125);
    assert(findHistoricalEvent(giver, "exchange_given", "currency:economy_units") != nullptr);
    assert(findHistoricalEvent(receiver, "exchange_received", "currency:economy_units") != nullptr);
    assert(findCanonicalRecordContaining(giver, "echanges_donnes", "currency:economy_units") != nullptr);
    assert(findCanonicalRecordContaining(receiver, "echanges_recus", "currency:economy_units") != nullptr);

    const std::size_t giverHistoryBeforeFailedMoney = giver.getHistoricalEvents().size();
    assert(!ExchangeTransferSystem::transferEconomyUnits(giver, receiver, 999999));
    assert(giver.getInventory().getEconomyUnits() == 375);
    assert(receiver.getInventory().getEconomyUnits() == 125);
    assert(giver.getHistoricalEvents().size() == giverHistoryBeforeFailedMoney);

    giver.getInventory().addWeapon(Weapon(
        "Épée de transfert",
        "Arme témoin pour vérifier l'identité persistante d'un don.",
        220,
        WeaponType::Sword,
        2,
        5,
        1,
        90
    ));
    const std::string weaponId = giver.getInventory().getWeapon(0).getPersistentId();
    assert(!weaponId.empty());
    assert(ExchangeTransferSystem::transferWeapon(giver, receiver, 0));
    assert(giver.getInventory().getWeaponCount() == 0);
    assert(receiver.getInventory().getWeaponCount() == 1);
    assert(receiver.getInventory().getWeapon(0).getPersistentId() == weaponId);
    assert(findHistoricalEvent(giver, "exchange_given", weaponId) != nullptr);
    assert(findHistoricalEvent(receiver, "exchange_received", weaponId) != nullptr);

    giver.getInventory().addArmor(Armor(
        "Cuir de transfert",
        "Armure témoin pour vérifier l'identité persistante d'un don.",
        180,
        ArmorType::Leather,
        4,
        1,
        80
    ));
    const std::string armorId = giver.getInventory().getArmor(0).getPersistentId();
    assert(!armorId.empty());
    assert(ExchangeTransferSystem::transferArmor(giver, receiver, 0));
    assert(giver.getInventory().getArmorCount() == 0);
    assert(receiver.getInventory().getArmorCount() == 1);
    assert(receiver.getInventory().getArmor(0).getPersistentId() == armorId);
    assert(findHistoricalEvent(giver, "exchange_given", armorId) != nullptr);
    assert(findHistoricalEvent(receiver, "exchange_received", armorId) != nullptr);

    giver.getInventory().addConsumable(Consumable(
        "Potion de transfert",
        "Consommable témoin.",
        40,
        ConsumableType::Healing,
        12
    ));
    assert(ExchangeTransferSystem::transferConsumable(giver, receiver, 0));
    assert(giver.getInventory().getConsumableCount() == 0);
    assert(receiver.getInventory().getConsumableCount() == 1);
    assert(findHistoricalEvent(giver, "exchange_given", "consumable:Potion de transfert") != nullptr);
    assert(findHistoricalEvent(receiver, "exchange_received", "consumable:Potion de transfert") != nullptr);

    giver.getInventory().addMaterial(Material(
        "exchange_ore",
        "Minerai de transfert",
        "Matériau témoin.",
        "minerai",
        25,
        5,
        "high"
    ));
    assert(ExchangeTransferSystem::transferMaterial(giver, receiver, 0, 3));
    assert(giver.getInventory().getMaterialCount() == 2);
    assert(giver.getInventory().getMaterial(0).getQuantity() == 2);
    assert(receiver.getInventory().getMaterialCount() == 3);
    assert(receiver.getInventory().getMaterial(0).getQuantity() == 3);
    const PlayerHistoricalEvent* materialReceived = findHistoricalEvent(
        receiver,
        "exchange_received",
        "material:exchange_ore:high"
    );
    assert(materialReceived != nullptr);
    assert(materialReceived->label.find("x3") != std::string::npos);
    const PlayerJournalRecord* materialSummary = findCanonicalRecordContaining(
        receiver,
        "echanges_recus",
        "material:exchange_ore:high"
    );
    assert(materialSummary != nullptr);
    assert(materialSummary->count == 3);


    // V3.51.56: ownership-bound items cannot change owner through the voluntary exchange path.
    Weapon boundWeapon(
        "Lame liée témoin",
        "Arme de test liée à l'âme.",
        900,
        WeaponType::Sword,
        8,
        12,
        3,
        120
    );
    boundWeapon.setSoulBound();
    giver.getInventory().addWeapon(boundWeapon);
    const int boundWeaponIndex = giver.getInventory().getWeaponCount() - 1;
    const int receiverWeaponsBeforeBound = receiver.getInventory().getWeaponCount();
    assert(giver.getInventory().getWeapon(boundWeaponIndex).isSoulBound());
    assert(!ExchangeTransferSystem::transferWeapon(giver, receiver, boundWeaponIndex));
    assert(giver.getInventory().getWeapon(boundWeaponIndex).isSoulBound());
    assert(receiver.getInventory().getWeaponCount() == receiverWeaponsBeforeBound);

    Armor boundArmor(
        "Armure liée témoin",
        "Armure de test liée à l'âme.",
        850,
        ArmorType::Plate,
        12,
        4,
        140
    );
    boundArmor.setSoulBound();
    giver.getInventory().addArmor(boundArmor);
    const int boundArmorIndex = giver.getInventory().getArmorCount() - 1;
    const int receiverArmorsBeforeBound = receiver.getInventory().getArmorCount();
    assert(!ExchangeTransferSystem::transferArmor(giver, receiver, boundArmorIndex));
    assert(receiver.getInventory().getArmorCount() == receiverArmorsBeforeBound);

    Consumable boundConsumable(
        "Potion liée témoin",
        "Consommable de test lié à l'âme.",
        55,
        ConsumableType::Healing,
        10
    );
    boundConsumable.setSoulBound();
    giver.getInventory().addConsumable(boundConsumable);
    const int boundConsumableIndex = giver.getInventory().getConsumableCount() - 1;
    const int receiverConsumablesBeforeBound = receiver.getInventory().getConsumableCount();
    assert(!ExchangeTransferSystem::transferConsumable(giver, receiver, boundConsumableIndex));
    assert(receiver.getInventory().getConsumableCount() == receiverConsumablesBeforeBound);

    Material boundMaterial(
        "exchange_ore",
        "Minerai de transfert",
        "Pile liée séparée de la pile libre.",
        "minerai",
        25,
        2,
        "high"
    );
    boundMaterial.setSoulBound();
    const std::size_t giverMaterialStacksBefore = giver.getInventory().getMaterials().size();
    giver.getInventory().addMaterial(boundMaterial);
    assert(giver.getInventory().getMaterials().size() == giverMaterialStacksBefore + 1);
    int boundMaterialIndex = -1;
    for (int i = 0; i < static_cast<int>(giver.getInventory().getMaterials().size()); ++i)
    {
        if (giver.getInventory().getMaterial(i).isSoulBound())
        {
            boundMaterialIndex = i;
            break;
        }
    }
    assert(boundMaterialIndex >= 0);
    const int receiverMaterialUnitsBeforeBound = receiver.getInventory().getMaterialCount();
    assert(!ExchangeTransferSystem::transferMaterial(giver, receiver, boundMaterialIndex, 1));
    assert(receiver.getInventory().getMaterialCount() == receiverMaterialUnitsBeforeBound);
    assert(giver.getInventory().getMaterial(boundMaterialIndex).getQuantity() == 2);

    const int receiverWeaponCount = receiver.getInventory().getWeaponCount();
    const std::size_t receiverHistoryBeforeInvalid = receiver.getHistoricalEvents().size();
    assert(!ExchangeTransferSystem::transferWeapon(giver, receiver, 99));
    assert(receiver.getInventory().getWeaponCount() == receiverWeaponCount);
    assert(receiver.getHistoricalEvents().size() == receiverHistoryBeforeInvalid);

    return 0;
}
