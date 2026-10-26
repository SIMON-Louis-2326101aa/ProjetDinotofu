#include <cassert>
#include <string>

#include "combat/modes/pvp/PvpEquipmentHistory.hpp"
#include "entity/Player.hpp"
#include "item/armor/Armor.hpp"
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

    const PlayerJournalRecord* findCanonicalRecord(
        const Player& player,
        const std::string& category,
        const std::string& key
    )
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
    Player winner;
    Player loser;

    loser.getInventory().addWeapon(Weapon(
        "Lame JcJ témoin",
        "Arme utilisée pour vérifier l'identité exacte d'un butin JcJ.",
        220,
        WeaponType::Sword,
        2,
        5,
        1,
        90
    ));

    const Weapon weapon = loser.getInventory().getWeapon(0);
    const std::string weaponId = weapon.getPersistentId();
    assert(!weaponId.empty());
    assert(PvpEquipmentHistory::weaponSubjectId(weapon) == weaponId);

    PvpEquipmentHistory::recordWeaponTransfer(winner, loser, weapon, true, true);

    const PlayerHistoricalEvent* weaponLoss = findHistoricalEvent(loser, "equipment_lost", weaponId);
    assert(weaponLoss != nullptr);
    assert(weaponLoss->resolved);
    assert(weaponLoss->label.find("Lame JcJ témoin") != std::string::npos);
    assert(weaponLoss->label.find("mort définitive") != std::string::npos);
    assert(weaponLoss->label.find("équipé") != std::string::npos);

    const PlayerJournalRecord* weaponLoot = findCanonicalRecord(winner, "butin_notable", weaponId);
    assert(weaponLoot != nullptr);
    assert(weaponLoot->count == 1);
    assert(weaponLoot->label.find("Lame JcJ témoin") != std::string::npos);
    assert(weaponLoot->label.find("duel JcJ") != std::string::npos);

    Armor legacyArmor(
        "Cuir ancien JcJ",
        "Armure sans ID simulant une ancienne donnée avant ajout à l'inventaire.",
        140,
        ArmorType::Leather,
        4,
        0,
        80
    );
    assert(legacyArmor.getPersistentId().empty());
    assert(PvpEquipmentHistory::armorSubjectId(legacyArmor) == "armor:Cuir ancien JcJ");

    PvpEquipmentHistory::recordArmorTransfer(winner, loser, legacyArmor, false, false);

    const PlayerHistoricalEvent* armorLoss = findHistoricalEvent(loser, "equipment_lost", "armor:Cuir ancien JcJ");
    assert(armorLoss != nullptr);
    assert(armorLoss->label.find("mortel non définitif") != std::string::npos);
    assert(armorLoss->label.find("depuis l'inventaire") != std::string::npos);

    const PlayerJournalRecord* armorLoot = findCanonicalRecord(winner, "butin_notable", "armor:Cuir ancien JcJ");
    assert(armorLoot != nullptr);
    assert(armorLoot->count == 1);

    return 0;
}
