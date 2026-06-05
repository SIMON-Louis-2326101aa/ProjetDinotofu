// EN: Armor.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: Armor.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "item/armor/Armor.hpp"

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

void Armor::downgradeExceptionalQualityAfterBreak()
{
    if (!looksExceptionalAndNotAlreadyDowngraded(name, description))
    {
        return;
    }

    name += " - haute qualité abîmée";
    description += " Qualité dégradée : l'armure était exceptionnelle, mais sa durabilité est tombée à 0 avant réparation. Elle reste haute qualité après réparation, sans récupérer son grade exceptionnel.";
    value = std::max(1, value * 85 / 100);
}

// EN: Armor declares or implements a focused behavior used by this module.
// FR: Armor déclare ou implémente un comportement précis utilisé par ce module.
Armor::Armor() : Item()
{
    type = ArmorType::Unknown;

    maxHpBonus = 0;
    damageReduction = 0;

    maxDurability = -1;
    durability = -1;
    enchantments.clear();
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
    this->enchantments.clear();
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

int Armor::getEnchantmentCount() const
{
    return static_cast<int>(enchantments.size());
}

std::vector<std::string> Armor::getEnchantments() const
{
    return enchantments;
}

std::string Armor::getEnchantmentSummaryText() const
{
    return joinEnchantmentDisplayText(enchantments);
}

std::string Armor::getEnchantmentsSaveText() const
{
    return joinEnchantmentSaveText(enchantments);
}

void Armor::addEnchantment(const std::string& enchantmentLabel)
{
    if (enchantmentLabel.empty())
    {
        return;
    }

    enchantments.push_back(enchantmentLabel);
    value = std::max(1, value + 16 + static_cast<int>(enchantments.size()) * 7);
}

bool Armor::removeLastEnchantment()
{
    if (enchantments.empty())
    {
        return false;
    }

    enchantments.pop_back();
    value = std::max(1, value - 18);
    return true;
}

void Armor::loadEnchantmentsFromSaveText(const std::string& saveText)
{
    enchantments = splitEnchantmentSaveText(saveText);
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

    lines.push_back("==================");
    return lines;
}

// EN: display declares or implements a focused behavior used by this module.
// FR: display déclare ou implémente un comportement précis utilisé par ce module.
void Armor::display() const
{
    MessageScreen::show("ARMURE", "item.armor.display", toDisplayLines(), false);
}
