// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "item/weapon/Weapon.hpp"

#include <iostream>

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
) : Item(name, description, value)
{
    this->type = type;

    this->minDamageBonus = minDamageBonus;
    this->maxDamageBonus = maxDamageBonus;
    this->criticalBonus = criticalBonus;

    this->maxDurability = maxDurability;
    this->durability = maxDurability;
}

WeaponType Weapon::getType() const
{
    return type;
}

int Weapon::getMinDamageBonus() const
{
    return minDamageBonus;
}

int Weapon::getMaxDamageBonus() const
{
    return maxDamageBonus;
}

int Weapon::getCriticalBonus() const
{
    return criticalBonus;
}

int Weapon::getDurability() const
{
    return durability;
}

int Weapon::getMaxDurability() const
{
    return maxDurability;
}

bool Weapon::isBroken() const
{
    if (isIndestructible())
    {
        return false;
    }

    return durability <= 0;
}

bool Weapon::isIndestructible() const
{
    return maxDurability < 0;
}

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

void Weapon::fullyRepair()
{
    if (isIndestructible())
    {
        return;
    }

    durability = maxDurability;
}

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