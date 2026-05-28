// EN: Armor.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: Armor.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "item/armor/Armor.hpp"

#include "interface/menu/common/MessageScreen.hpp"

#include <vector>
#include <string>

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

namespace
{
    std::string armorTypeLabel(ArmorType type)
    {
        switch (type)
        {
            case ArmorType::Cloth:
                return "Tissu";
            case ArmorType::Leather:
                return "Cuir";
            case ArmorType::Chainmail:
                return "Maille";
            case ArmorType::Plate:
                return "Plaque";
            case ArmorType::Magical:
                return "Magique";
            default:
                return "Inconnue";
        }
    }
}

std::vector<std::string> Armor::toDisplayLines() const
{
    std::vector<std::string> lines = {
        "===== ARMURE =====",
        "Nom : " + name,
        "Description : " + description,
        "Valeur : " + std::to_string(value) + " pièces",
        "Type : " + armorTypeLabel(type),
        "Bonus PV max : " + std::to_string(maxHpBonus),
        "Réduction dégâts : " + std::to_string(damageReduction)
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

    lines.push_back("==================");
    return lines;
}

// EN: display declares or implements a focused behavior used by this module.
// FR: display déclare ou implémente un comportement précis utilisé par ce module.
void Armor::display() const
{
    MessageScreen::show("ARMURE", "item.armor.display", toDisplayLines(), false);
}
