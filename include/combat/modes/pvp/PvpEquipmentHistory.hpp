// EN: PvpEquipmentHistory records real PvP equipment transfers without inventing losses.
// FR: PvpEquipmentHistory trace les transferts réels d'équipement JcJ sans inventer de pertes.
#ifndef INCLUDE_COMBAT_MODES_PVP_PVPEQUIPMENTHISTORY_HPP
#define INCLUDE_COMBAT_MODES_PVP_PVPEQUIPMENTHISTORY_HPP

#include <string>

class Player;
class Weapon;
class Armor;

namespace PvpEquipmentHistory
{
    std::string weaponSubjectId(const Weapon& weapon);
    std::string armorSubjectId(const Armor& armor);

    void recordWeaponTransfer(
        Player& winner,
        Player& loser,
        const Weapon& weapon,
        bool lethalDuel,
        bool equippedAtTheft
    );

    void recordArmorTransfer(
        Player& winner,
        Player& loser,
        const Armor& armor,
        bool lethalDuel,
        bool equippedAtTheft
    );
}

#endif
