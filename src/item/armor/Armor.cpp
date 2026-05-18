// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "item/armor/Armor.hpp"

#include <iostream>

Armor::Armor() : Item()
{
    type = ArmorType::Unknown;

    maxHpBonus = 0;
    damageReduction = 0;

    maxDurability = -1;
    durability = -1;
}

Armor::Armor(
    const std::string& name,
    const std::string& description,
    int value,
    ArmorType type,
    int maxHpBonus,
    int damageReduction,
    int maxDurability
) : Item(name, description, value)
{
    this->type = type;

    this->maxHpBonus = maxHpBonus;
    this->damageReduction = damageReduction;

    this->maxDurability = maxDurability;
    this->durability = maxDurability;
}

ArmorType Armor::getType() const
{
    return type;
}

int Armor::getMaxHpBonus() const
{
    return maxHpBonus;
}

int Armor::getDamageReduction() const
{
    return damageReduction;
}

int Armor::getDurability() const
{
    return durability;
}

int Armor::getMaxDurability() const
{
    return maxDurability;
}

bool Armor::isBroken() const
{
    if (isIndestructible())
    {
        return false;
    }

    return durability <= 0;
}

bool Armor::isIndestructible() const
{
    return maxDurability < 0;
}

void Armor::loseDurability(int amount)
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

void Armor::repair(int amount)
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

void Armor::fullyRepair()
{
    if (isIndestructible())
    {
        return;
    }

    durability = maxDurability;
}

void Armor::display() const
{
    std::cout << "===== ARMURE =====" << std::endl;
    std::cout << "Nom : " << name << std::endl;
    std::cout << "Description : " << description << std::endl;
    std::cout << "Valeur : " << value << " pièces" << std::endl;

    std::cout << "Type : ";

    switch (type)
    {
        case ArmorType::Cloth:
            std::cout << "Tissu";
            break;

        case ArmorType::Leather:
            std::cout << "Cuir";
            break;

        case ArmorType::Chainmail:
            std::cout << "Maille";
            break;

        case ArmorType::Plate:
            std::cout << "Plaque";
            break;

        case ArmorType::Magical:
            std::cout << "Magique";
            break;

        default:
            std::cout << "Inconnue";
            break;
    }

    std::cout << std::endl;
    std::cout << "Bonus PV max : " << maxHpBonus << std::endl;
    std::cout << "Réduction dégâts : " << damageReduction << std::endl;

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

    std::cout << "==================" << std::endl;
    std::cout << std::endl;
}