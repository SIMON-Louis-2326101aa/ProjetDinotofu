// EN: Weapon.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: Weapon.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "item/weapon/Weapon.hpp"

#include "interface/menu/common/MessageScreen.hpp"

#include <algorithm>
#include <cctype>
#include <vector>
#include <string>
#include <sstream>

namespace
{
    std::string lowerEquipmentText(std::string value)
    {
        std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });
        return value;
    }

    bool looksExceptionalAndNotAlreadyDowngraded(const std::string& name, const std::string& description)
    {
        const std::string probe = lowerEquipmentText(name + " " + description);
        if (probe.find("haute qualité") != std::string::npos || probe.find("haute qualite") != std::string::npos)
        {
            return false;
        }

        return probe.find("exceptionnel") != std::string::npos
            || probe.find("exceptionnelle") != std::string::npos
            || probe.find("particularité") != std::string::npos
            || probe.find("particularite") != std::string::npos;
    }
    std::vector<std::string> splitEnchantmentSaveText(const std::string& saveText)
    {
        std::vector<std::string> result;
        std::stringstream stream(saveText);
        std::string token;

        while (std::getline(stream, token, '|'))
        {
            if (!token.empty())
            {
                result.push_back(token);
            }
        }

        return result;
    }

    std::string joinEnchantmentSaveText(const std::vector<std::string>& values)
    {
        std::string result;
        for (std::size_t i = 0; i < values.size(); ++i)
        {
            if (i > 0)
            {
                result += "|";
            }
            result += values[i];
        }
        return result;
    }

    std::string joinEnchantmentDisplayText(const std::vector<std::string>& values)
    {
        if (values.empty())
        {
            return "aucun";
        }

        std::string result;
        for (std::size_t i = 0; i < values.size(); ++i)
        {
            if (i > 0)
            {
                result += ", ";
            }
            result += values[i];
        }
        return result;
    }
}

void Weapon::downgradeExceptionalQualityAfterBreak()
{
    if (!looksExceptionalAndNotAlreadyDowngraded(name, description))
    {
        return;
    }

    name += " - haute qualité abîmée";
    description += " Qualité dégradée : l'arme était exceptionnelle, mais sa durabilité est tombée à 0 avant réparation. Elle reste haute qualité après réparation, sans récupérer son grade exceptionnel.";
    value = std::max(1, value * 85 / 100);
}

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
    enchantments.clear();
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
    this->enchantments.clear();
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

    const bool wasAboveZero = durability > 0;
    durability -= amount;

    if (durability < 0)
    {
        durability = 0;
    }

    if (wasAboveZero && durability == 0)
    {
        downgradeExceptionalQualityAfterBreak();
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

int Weapon::getEnchantmentCount() const
{
    return static_cast<int>(enchantments.size());
}

std::vector<std::string> Weapon::getEnchantments() const
{
    return enchantments;
}

std::string Weapon::getEnchantmentSummaryText() const
{
    return joinEnchantmentDisplayText(enchantments);
}

std::string Weapon::getEnchantmentsSaveText() const
{
    return joinEnchantmentSaveText(enchantments);
}

void Weapon::addEnchantment(const std::string& enchantmentLabel)
{
    if (enchantmentLabel.empty())
    {
        return;
    }

    enchantments.push_back(enchantmentLabel);
    value = std::max(1, value + 18 + static_cast<int>(enchantments.size()) * 8);
}

bool Weapon::removeLastEnchantment()
{
    if (enchantments.empty())
    {
        return false;
    }

    enchantments.pop_back();
    value = std::max(1, value - 20);
    return true;
}

void Weapon::loadEnchantmentsFromSaveText(const std::string& saveText)
{
    enchantments = splitEnchantmentSaveText(saveText);
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

    if (!enchantments.empty())
    {
        lines.push_back("Enchantements : " + getEnchantmentSummaryText());
        lines.push_back("Instabilité runique : " + std::to_string(getEnchantmentCount()) + " enchantement(s). Les prochains essais seront plus risqués.");
    }
    else
    {
        lines.push_back("Enchantements : aucun");
    }

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
