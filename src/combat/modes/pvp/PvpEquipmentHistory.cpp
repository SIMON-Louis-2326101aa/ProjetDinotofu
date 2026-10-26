// EN: PvpEquipmentHistory records durable identity/provenance for equipment actually taken in PvP.
// FR: PvpEquipmentHistory conserve l'identité et la provenance durables des équipements réellement pris en JcJ.
#include "combat/modes/pvp/PvpEquipmentHistory.hpp"

#include "entity/Player.hpp"
#include "item/armor/Armor.hpp"
#include "item/weapon/Weapon.hpp"

namespace
{
    std::string duelContext(bool lethalDuel)
    {
        return lethalDuel ? "un duel JcJ à mort définitive" : "un duel JcJ mortel non définitif";
    }

    std::string takenState(bool equippedAtTheft)
    {
        return equippedAtTheft ? " alors qu'il était équipé" : " depuis l'inventaire";
    }

    void recordTransfer(
        Player& winner,
        Player& loser,
        const std::string& subjectId,
        const std::string& itemName,
        const std::string& itemKind,
        bool lethalDuel,
        bool equippedAtTheft
    )
    {
        if (subjectId.empty() || itemName.empty())
        {
            return;
        }

        const std::string context = duelContext(lethalDuel);
        loser.recordHistoricalEvent(
            "equipment_lost",
            subjectId,
            "Équipement perdu en JcJ : " + itemName + " a été récupéré par " + winner.getName()
                + takenState(equippedAtTheft) + " après " + context + ".",
            true
        );

        winner.recordCanonicalEvent(
            "butin_notable",
            subjectId,
            itemName + " (" + itemKind + ") récupéré sur " + loser.getName() + " après " + context,
            1
        );
    }
}

std::string PvpEquipmentHistory::weaponSubjectId(const Weapon& weapon)
{
    return weapon.getPersistentId().empty() ? "weapon:" + weapon.getName() : weapon.getPersistentId();
}

std::string PvpEquipmentHistory::armorSubjectId(const Armor& armor)
{
    return armor.getPersistentId().empty() ? "armor:" + armor.getName() : armor.getPersistentId();
}

void PvpEquipmentHistory::recordWeaponTransfer(
    Player& winner,
    Player& loser,
    const Weapon& weapon,
    bool lethalDuel,
    bool equippedAtTheft
)
{
    recordTransfer(
        winner,
        loser,
        weaponSubjectId(weapon),
        weapon.getName(),
        "arme",
        lethalDuel,
        equippedAtTheft
    );
}

void PvpEquipmentHistory::recordArmorTransfer(
    Player& winner,
    Player& loser,
    const Armor& armor,
    bool lethalDuel,
    bool equippedAtTheft
)
{
    recordTransfer(
        winner,
        loser,
        armorSubjectId(armor),
        armor.getName(),
        "armure",
        lethalDuel,
        equippedAtTheft
    );
}
