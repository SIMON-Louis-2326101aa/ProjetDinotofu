// EN: Weapon.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: Weapon.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "item/weapon/Weapon.hpp"

#include <iostream>

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

// EN: display declares or implements a focused behavior used by this module.
// FR: display déclare ou implémente un comportement précis utilisé par ce module.
void Weapon::display() const
{
    std::cout << "===== ARME =====" << std::endl;
    std::cout << "Nom : " << name << std::endl;
    std::cout << "Description : " << description << std::endl;
    std::cout << "Valeur : " << value << " pièces" << std::endl;

    std::cout << "Type : ";

    switch (type)
    {
        case WeaponType::Sword:
            std::cout << "Épée";
            break;

        case WeaponType::Dagger:
            std::cout << "Dague";
            break;

        case WeaponType::Axe:
            std::cout << "Hache";
            break;

        case WeaponType::Hammer:
            std::cout << "Marteau";
            break;

        case WeaponType::Spear:
            std::cout << "Lance";
            break;

        case WeaponType::Staff:
            std::cout << "Bâton";
            break;

        case WeaponType::Bow:
            std::cout << "Arc";
            break;

        case WeaponType::BareHands:
            std::cout << "Mains nues";
            break;

        default:
            std::cout << "Inconnu";
            break;
    }

    std::cout << std::endl;
    std::cout << "Bonus dégâts min : " << minDamageBonus << std::endl;
    std::cout << "Bonus dégâts max : " << maxDamageBonus << std::endl;
    std::cout << "Bonus critique : " << criticalBonus << std::endl;

    if (isIndestructible())
    {
        std::cout << "Durabilité : Indestructible" << std::endl;
    }
    else
    {
        std::cout << "Durabilité : " << durability << "/" << maxDurability << std::endl;

        if (isBroken())
        {
            std::cout << "État : Cassée, ses bonus ne s'appliquent plus." << std::endl;
        }
    }

    std::cout << "================" << std::endl;
    std::cout << std::endl;
}
