// EN: Weapon.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: Weapon.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "item/weapon/Weapon.hpp"

#include "interface/menu/common/MessageScreen.hpp"

#include <vector>
#include <string>

// EN: Weapon declares or implements a focused behavior used by this module.
// FR: Weapon déclare ou implémente un comportement précis utilisé par ce module.
Weapon::Weapon() : Item()
{
    type = WeaponType::BareHands;

    minDamageBonus = 0;
    maxDamageBonus = 0;
    criticalBonus = 0;

    maxDurability = -1;
    durability = -1;
}

Weapon::Weapon(
    const std::string& name,
    const std::string& description,
    int value,
    WeaponType type,
    int minDamageBonus,
    int maxDamageBonus,
    int criticalBonus,
    int maxDurability
// EN: Item declares or implements a focused behavior used by this module.
// FR: Item déclare ou implémente un comportement précis utilisé par ce module.
) : Item(name, description, value)
{
    this->type = type;

    this->minDamageBonus = minDamageBonus;
    this->maxDamageBonus = maxDamageBonus;
    this->criticalBonus = criticalBonus;

    this->maxDurability = maxDurability;
    this->durability = maxDurability;
}

// EN: getType declares or implements a focused behavior used by this module.
// FR: getType déclare ou implémente un comportement précis utilisé par ce module.
WeaponType Weapon::getType() const
{
    return type;
}

// EN: getMinDamageBonus declares or implements a focused behavior used by this module.
// FR: getMinDamageBonus déclare ou implémente un comportement précis utilisé par ce module.
int Weapon::getMinDamageBonus() const
{
    return minDamageBonus;
}

// EN: getMaxDamageBonus declares or implements a focused behavior used by this module.
// FR: getMaxDamageBonus déclare ou implémente un comportement précis utilisé par ce module.
int Weapon::getMaxDamageBonus() const
{
    return maxDamageBonus;
}

// EN: getCriticalBonus declares or implements a focused behavior used by this module.
// FR: getCriticalBonus déclare ou implémente un comportement précis utilisé par ce module.
int Weapon::getCriticalBonus() const
{
    return criticalBonus;
}

// EN: getDurability declares or implements a focused behavior used by this module.
// FR: getDurability déclare ou implémente un comportement précis utilisé par ce module.
int Weapon::getDurability() const
{
    return durability;
}

// EN: getMaxDurability declares or implements a focused behavior used by this module.
// FR: getMaxDurability déclare ou implémente un comportement précis utilisé par ce module.
int Weapon::getMaxDurability() const
{
    return maxDurability;
}

// EN: isBroken declares or implements a focused behavior used by this module.
// FR: isBroken déclare ou implémente un comportement précis utilisé par ce module.
bool Weapon::isBroken() const
{
    if (isIndestructible())
    {
        return false;
    }

    return durability <= 0;
}

// EN: isIndestructible declares or implements a focused behavior used by this module.
// FR: isIndestructible déclare ou implémente un comportement précis utilisé par ce module.
bool Weapon::isIndestructible() const
{
    return maxDurability < 0;
}

// EN: loseDurability declares or implements a focused behavior used by this module.
// FR: loseDurability déclare ou implémente un comportement précis utilisé par ce module.
void Weapon::loseDurability(int amount)
{
    if (isIndestructible() || amount <= 0)
    {
        return;
    }

    durability -= amount;

    if (durability < 0)
    {
        durability = 0;
    }
}

// EN: repair declares or implements a focused behavior used by this module.
// FR: repair déclare ou implémente un comportement précis utilisé par ce module.
void Weapon::repair(int amount)
{
    if (isIndestructible() || amount <= 0)
    {
        return;
    }

    durability += amount;

    if (durability > maxDurability)
    {
        durability = maxDurability;
    }
}

// EN: fullyRepair declares or implements a focused behavior used by this module.
// FR: fullyRepair déclare ou implémente un comportement précis utilisé par ce module.
void Weapon::fullyRepair()
{
    if (isIndestructible())
    {
        return;
    }

    durability = maxDurability;
}

namespace
{
    std::string weaponTypeLabel(WeaponType type)
    {
        switch (type)
        {
            case WeaponType::Sword:
                return "Épée";
            case WeaponType::Dagger:
                return "Dague";
            case WeaponType::Axe:
                return "Hache";
            case WeaponType::Hammer:
                return "Marteau";
            case WeaponType::Spear:
                return "Lance";
            case WeaponType::Staff:
                return "Bâton";
            case WeaponType::Bow:
                return "Arc";
            case WeaponType::BareHands:
                return "Mains nues";
            default:
                return "Inconnu";
        }
    }
}

std::vector<std::string> Weapon::toDisplayLines() const
{
    std::vector<std::string> lines = {
        "===== ARME =====",
        "Nom : " + name,
        "Description : " + description,
        "Valeur : " + std::to_string(value) + " pièces",
        "Type : " + weaponTypeLabel(type),
        "Bonus dégâts min : " + std::to_string(minDamageBonus),
        "Bonus dégâts max : " + std::to_string(maxDamageBonus),
        "Bonus critique : " + std::to_string(criticalBonus)
    };

    if (isIndestructible())
    {
        lines.push_back("Durabilité : Indestructible");
    }
    else
    {
        lines.push_back("Durabilité : " + std::to_string(durability) + "/" + std::to_string(maxDurability));
        if (isBroken())
        {
            lines.push_back("État : Cassée, ses bonus ne s'appliquent plus.");
        }
    }

    lines.push_back("================");
    return lines;
}

// EN: display declares or implements a focused behavior used by this module.
// FR: display déclare ou implémente un comportement précis utilisé par ce module.
void Weapon::display() const
{
    MessageScreen::show("ARME", "item.weapon.display", toDisplayLines(), false);
}
