// EN: Armor.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: Armor.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "item/armor/Armor.hpp"

#include <iostream>

// EN: Armor declares or implements a focused behavior used by this module.
// FR: Armor déclare ou implémente un comportement précis utilisé par ce module.
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
// EN: Item declares or implements a focused behavior used by this module.
// FR: Item déclare ou implémente un comportement précis utilisé par ce module.
) : Item(name, description, value)
{
    this->type = type;

    this->maxHpBonus = maxHpBonus;
    this->damageReduction = damageReduction;

    this->maxDurability = maxDurability;
    this->durability = maxDurability;
}

// EN: getType declares or implements a focused behavior used by this module.
// FR: getType déclare ou implémente un comportement précis utilisé par ce module.
ArmorType Armor::getType() const
{
    return type;
}

// EN: getMaxHpBonus declares or implements a focused behavior used by this module.
// FR: getMaxHpBonus déclare ou implémente un comportement précis utilisé par ce module.
int Armor::getMaxHpBonus() const
{
    return maxHpBonus;
}

// EN: getDamageReduction declares or implements a focused behavior used by this module.
// FR: getDamageReduction déclare ou implémente un comportement précis utilisé par ce module.
int Armor::getDamageReduction() const
{
    return damageReduction;
}

// EN: getDurability declares or implements a focused behavior used by this module.
// FR: getDurability déclare ou implémente un comportement précis utilisé par ce module.
int Armor::getDurability() const
{
    return durability;
}

// EN: getMaxDurability declares or implements a focused behavior used by this module.
// FR: getMaxDurability déclare ou implémente un comportement précis utilisé par ce module.
int Armor::getMaxDurability() const
{
    return maxDurability;
}

// EN: isBroken declares or implements a focused behavior used by this module.
// FR: isBroken déclare ou implémente un comportement précis utilisé par ce module.
bool Armor::isBroken() const
{
    if (isIndestructible())
    {
        return false;
    }

    return durability <= 0;
}

// EN: isIndestructible declares or implements a focused behavior used by this module.
// FR: isIndestructible déclare ou implémente un comportement précis utilisé par ce module.
bool Armor::isIndestructible() const
{
    return maxDurability < 0;
}

// EN: loseDurability declares or implements a focused behavior used by this module.
// FR: loseDurability déclare ou implémente un comportement précis utilisé par ce module.
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

// EN: repair declares or implements a focused behavior used by this module.
// FR: repair déclare ou implémente un comportement précis utilisé par ce module.
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

// EN: fullyRepair declares or implements a focused behavior used by this module.
// FR: fullyRepair déclare ou implémente un comportement précis utilisé par ce module.
void Armor::fullyRepair()
{
    if (isIndestructible())
    {
        return;
    }

    durability = maxDurability;
}

// EN: display declares or implements a focused behavior used by this module.
// FR: display déclare ou implémente un comportement précis utilisé par ce module.
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
