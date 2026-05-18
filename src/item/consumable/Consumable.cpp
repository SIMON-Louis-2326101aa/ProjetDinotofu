// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "item/consumable/Consumable.hpp"

#include <iostream>

Consumable::Consumable() : Item()
{
    type = ConsumableType::Unknown;
    power = 0;
}

Consumable::Consumable(
    const std::string& name,
    const std::string& description,
    int value,
    ConsumableType type,
    int power
) : Item(name, description, value)
{
    this->type = type;
    this->power = power;
}

ConsumableType Consumable::getType() const
{
    return type;
}

int Consumable::getPower() const
{
    return power;
}

bool Consumable::isHealing() const
{
    return type == ConsumableType::Healing;
}

bool Consumable::isDamage() const
{
    return type == ConsumableType::Damage;
}

void Consumable::display() const
{
    std::cout << "===== CONSOMMABLE =====" << std::endl;
    std::cout << "Nom : " << name << std::endl;
    std::cout << "Description : " << description << std::endl;
    std::cout << "Valeur : " << value << " pièces" << std::endl;

    std::cout << "Type : ";

    switch (type)
    {
        case ConsumableType::Healing:
            std::cout << "Soin";
            break;

        case ConsumableType::Damage:
            std::cout << "Dégâts";
            break;

        case ConsumableType::Buff:
            std::cout << "Buff";
            break;

        case ConsumableType::Debuff:
            std::cout << "Debuff";
            break;

        case ConsumableType::Special:
            std::cout << "Spécial";
            break;

        default:
            std::cout << "Inconnu";
            break;
    }

    std::cout << std::endl;
    std::cout << "Puissance : " << power << std::endl;
    std::cout << "=======================" << std::endl;
    std::cout << std::endl;
}