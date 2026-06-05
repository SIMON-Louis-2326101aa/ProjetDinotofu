// EN: Consumable.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: Consumable.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "item/consumable/Consumable.hpp"

#include "interface/menu/common/MessageScreen.hpp"

#include <algorithm>
#include <vector>
#include <string>

// EN: Consumable declares or implements a focused behavior used by this module.
// FR: Consumable déclare ou implémente un comportement précis utilisé par ce module.
Consumable::Consumable() : Item()
{
    type = ConsumableType::Unknown;
    power = 0;
    percentageBasedHealing = false;
}

Consumable::Consumable(
    const std::string& name,
    const std::string& description,
    int value,
    ConsumableType type,
    int power,
    bool percentageBasedHealing
// EN: Item declares or implements a focused behavior used by this module.
// FR: Item déclare ou implémente un comportement précis utilisé par ce module.
) : Item(name, description, value)
{
    this->type = type;
    this->power = power;
    this->percentageBasedHealing = percentageBasedHealing;
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

bool Consumable::isPercentageBasedHealing() const
{
    return type == ConsumableType::Healing && percentageBasedHealing;
}

int Consumable::getHealingAmountForMaxHp(int maxHp) const
{
    if (maxHp < 1)
    {
        maxHp = 1;
    }

    if (isPercentageBasedHealing())
    {
        return std::max(1, maxHp * power / 100);
    }

    return power;
}

std::string Consumable::getPowerDisplayText() const
{
    if (isPercentageBasedHealing())
    {
        return std::to_string(power) + "% des PV max";
    }

    return std::to_string(power);
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

namespace
{
    std::string consumableTypeLabel(ConsumableType type)
    {
        switch (type)
        {
            case ConsumableType::Healing:
                return "Soin";
            case ConsumableType::Damage:
                return "Dégâts";
            case ConsumableType::Buff:
                return "Buff";
            case ConsumableType::Debuff:
                return "Debuff";
            case ConsumableType::Special:
                return "Spécial";
            default:
                return "Inconnu";
        }
    }
}

std::vector<std::string> Consumable::toDisplayLines() const
{
    return {
        "===== CONSOMMABLE =====",
        "Nom : " + name,
        "Description : " + description,
        "Valeur : " + std::to_string(value) + " pièces",
        "Type : " + consumableTypeLabel(type),
        "Puissance : " + getPowerDisplayText(),
        "======================="
    };
}

// EN: display declares or implements a focused behavior used by this module.
// FR: display déclare ou implémente un comportement précis utilisé par ce module.
void Consumable::display() const
{
    MessageScreen::show("CONSOMMABLE", "item.consumable.display", toDisplayLines(), false);
}
