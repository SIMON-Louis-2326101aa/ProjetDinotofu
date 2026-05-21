// EN: Consumable.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: Consumable.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "item/consumable/Consumable.hpp"

#include <iostream>

// EN: Consumable declares or implements a focused behavior used by this module.
// FR: Consumable déclare ou implémente un comportement précis utilisé par ce module.
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
// EN: Item declares or implements a focused behavior used by this module.
// FR: Item déclare ou implémente un comportement précis utilisé par ce module.
) : Item(name, description, value)
{
    this->type = type;
    this->power = power;
}

// EN: getType declares or implements a focused behavior used by this module.
// FR: getType déclare ou implémente un comportement précis utilisé par ce module.
ConsumableType Consumable::getType() const
{
    return type;
}

// EN: getPower declares or implements a focused behavior used by this module.
// FR: getPower déclare ou implémente un comportement précis utilisé par ce module.
int Consumable::getPower() const
{
    return power;
}

// EN: isHealing declares or implements a focused behavior used by this module.
// FR: isHealing déclare ou implémente un comportement précis utilisé par ce module.
bool Consumable::isHealing() const
{
    return type == ConsumableType::Healing;
}

// EN: isDamage declares or implements a focused behavior used by this module.
// FR: isDamage déclare ou implémente un comportement précis utilisé par ce module.
bool Consumable::isDamage() const
{
    return type == ConsumableType::Damage;
}

// EN: display declares or implements a focused behavior used by this module.
// FR: display déclare ou implémente un comportement précis utilisé par ce module.
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
