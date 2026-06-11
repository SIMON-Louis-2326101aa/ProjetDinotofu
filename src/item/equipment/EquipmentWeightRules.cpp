// English: Balanced equipment weight trade-offs: visible, but not brutally punitive.
// Français : Contreparties équilibrées du poids d'équipement : visibles, mais pas punitives.
#include "item/equipment/EquipmentWeightRules.hpp"

#include <algorithm>
#include <cctype>
#include <initializer_list>
#include <string>

namespace
{
    std::string lowerWeightText(std::string value)
    {
        std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });
        return value;
    }

    bool containsAny(const std::string& value, std::initializer_list<const char*> needles)
    {
        for (const char* needle : needles)
        {
            if (value.find(needle) != std::string::npos)
            {
                return true;
            }
        }
        return false;
    }
}

EquipmentWeightClass EquipmentWeightRules::getWeaponWeightClass(const Weapon& weapon)
{
    const std::string probe = lowerWeightText(weapon.getName() + " " + weapon.getDescription());

    if (containsAny(probe, {"lourde", "lourd", "massive", "massif", "grande hache", "marteau", "maul", "colosse", "hallebarde", "épée lourde", "epee lourde", "arbalète", "arbalete"}))
    {
        return EquipmentWeightClass::Heavy;
    }

    if (containsAny(probe, {"légère", "legere", "léger", "leger", "courte", "court", "dague", "couteau", "rapière", "rapiere", "aiguille", "poignard", "lancer"}))
    {
        return EquipmentWeightClass::Light;
    }

    switch (weapon.getType())
    {
        case WeaponType::Dagger:
        case WeaponType::BareHands:
            return EquipmentWeightClass::Light;
        case WeaponType::Axe:
        case WeaponType::Hammer:
            return EquipmentWeightClass::Heavy;
        case WeaponType::Spear:
        case WeaponType::Sword:
        case WeaponType::Bow:
        case WeaponType::Staff:
        case WeaponType::Unknown:
        default:
            return EquipmentWeightClass::Medium;
    }
}

EquipmentWeightClass EquipmentWeightRules::getArmorWeightClass(const Armor& armor)
{
    const std::string probe = lowerWeightText(armor.getName() + " " + armor.getDescription());

    if (containsAny(probe, {"plaques", "plate", "lourde", "lourd", "blindée", "blindee", "harnais", "cotte", "acier", "fer", "carapace"}))
    {
        return EquipmentWeightClass::Heavy;
    }

    if (containsAny(probe, {"tissu", "robe", "manteau", "tenue", "légère", "legere", "souple", "cuir fin", "cousu d'ombre"}))
    {
        return EquipmentWeightClass::Light;
    }

    switch (armor.getType())
    {
        case ArmorType::Cloth:
            return EquipmentWeightClass::Light;
        case ArmorType::Leather:
        case ArmorType::Magical:
            return EquipmentWeightClass::Medium;
        case ArmorType::Chainmail:
        case ArmorType::Plate:
            return EquipmentWeightClass::Heavy;
        case ArmorType::Unknown:
        default:
            return EquipmentWeightClass::Medium;
    }
}

std::string EquipmentWeightRules::getWeightLabel(EquipmentWeightClass weightClass)
{
    switch (weightClass)
    {
        case EquipmentWeightClass::Light: return "Léger";
        case EquipmentWeightClass::Heavy: return "Lourd";
        case EquipmentWeightClass::Medium:
        default: return "Moyen";
    }
}

std::string EquipmentWeightRules::getWeaponTradeoffText(const Weapon& weapon)
{
    const EquipmentWeightClass weightClass = getWeaponWeightClass(weapon);
    if (weightClass == EquipmentWeightClass::Light)
    {
        return "Arme légère : +précision/+fuite légère, dégâts -6%.";
    }
    if (weightClass == EquipmentWeightClass::Heavy)
    {
        return "Arme lourde : dégâts +6%, précision/fuite légèrement réduites.";
    }
    return "Arme moyenne : pas de contrepartie particulière.";
}

std::string EquipmentWeightRules::getArmorTradeoffText(const Armor& armor)
{
    const EquipmentWeightClass weightClass = getArmorWeightClass(armor);
    if (weightClass == EquipmentWeightClass::Light)
    {
        return "Armure légère : +fuite légère, protection brute -1 sur les gros impacts.";
    }
    if (weightClass == EquipmentWeightClass::Heavy)
    {
        return "Armure lourde : protection brute +1/+2, fuite légèrement réduite.";
    }
    return "Armure moyenne : équilibre sans malus notable.";
}

std::vector<std::string> EquipmentWeightRules::buildEquipmentWeightSummaryLines()
{
    return {
        "Armes légères : plus précises et un peu meilleures pour fuir, mais dégâts réduits d'environ 6%.",
        "Armes moyennes : fonctionnement neutre, sans bonus ni malus de poids.",
        "Armes lourdes : dégâts augmentés d'environ 6%, mais précision et fuite légèrement moins bonnes.",
        "Armures légères : mobilité un peu meilleure, mais protection brute plus faible contre les gros impacts.",
        "Armures moyennes : équilibre stable.",
        "Armures lourdes : meilleure absorption, mais fuite et mobilité légèrement plus difficiles.",
        "Les malus restent volontairement légers : ils doivent se ressentir sans rendre un build injouable."
    };
}

int EquipmentWeightRules::getWeaponDodgeThresholdAdjustment(const Weapon& weapon)
{
    const EquipmentWeightClass weightClass = getWeaponWeightClass(weapon);
    if (weightClass == EquipmentWeightClass::Light) return -1;
    if (weightClass == EquipmentWeightClass::Heavy) return 1;
    return 0;
}

int EquipmentWeightRules::getWeaponNormalHitThresholdAdjustment(const Weapon& weapon)
{
    const EquipmentWeightClass weightClass = getWeaponWeightClass(weapon);
    if (weightClass == EquipmentWeightClass::Light) return -1;
    if (weightClass == EquipmentWeightClass::Heavy) return 1;
    return 0;
}

int EquipmentWeightRules::getWeaponDamagePercent(const Weapon& weapon)
{
    const EquipmentWeightClass weightClass = getWeaponWeightClass(weapon);
    if (weightClass == EquipmentWeightClass::Light) return 94;
    if (weightClass == EquipmentWeightClass::Heavy) return 106;
    return 100;
}

int EquipmentWeightRules::getWeaponEscapeModifier(const Weapon& weapon)
{
    const EquipmentWeightClass weightClass = getWeaponWeightClass(weapon);
    if (weightClass == EquipmentWeightClass::Light) return 2;
    if (weightClass == EquipmentWeightClass::Heavy) return -3;
    return 0;
}

int EquipmentWeightRules::getArmorDamageReductionAdjustment(const Armor& armor, int rawDamage)
{
    const EquipmentWeightClass weightClass = getArmorWeightClass(armor);
    if (weightClass == EquipmentWeightClass::Light)
    {
        return rawDamage >= 28 ? -1 : 0;
    }
    if (weightClass == EquipmentWeightClass::Heavy)
    {
        return rawDamage >= 28 ? 2 : 1;
    }
    return 0;
}

int EquipmentWeightRules::getArmorEscapeModifier(const Armor& armor)
{
    const EquipmentWeightClass weightClass = getArmorWeightClass(armor);
    if (weightClass == EquipmentWeightClass::Light) return 3;
    if (weightClass == EquipmentWeightClass::Heavy) return -5;
    return 0;
}
