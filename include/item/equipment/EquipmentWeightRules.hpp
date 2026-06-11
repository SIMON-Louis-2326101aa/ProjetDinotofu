// English: Derives equipment weight classes from existing weapon/armor data without changing old saves.
// Français : Déduit les classes de poids des armes/armures existantes sans changer les anciennes sauvegardes.
#ifndef INCLUDE_ITEM_EQUIPMENT_EQUIPMENTWEIGHTRULES_HPP
#define INCLUDE_ITEM_EQUIPMENT_EQUIPMENTWEIGHTRULES_HPP

#include "item/armor/Armor.hpp"
#include "item/equipment/EquipmentWeightClass.hpp"
#include "item/weapon/Weapon.hpp"

#include <string>
#include <vector>

class EquipmentWeightRules
{
public:
    static EquipmentWeightClass getWeaponWeightClass(const Weapon& weapon);
    static EquipmentWeightClass getArmorWeightClass(const Armor& armor);

    static std::string getWeightLabel(EquipmentWeightClass weightClass);
    static std::string getWeaponTradeoffText(const Weapon& weapon);
    static std::string getArmorTradeoffText(const Armor& armor);
    static std::vector<std::string> buildEquipmentWeightSummaryLines();

    static int getWeaponDodgeThresholdAdjustment(const Weapon& weapon);
    static int getWeaponNormalHitThresholdAdjustment(const Weapon& weapon);
    static int getWeaponDamagePercent(const Weapon& weapon);
    static int getWeaponEscapeModifier(const Weapon& weapon);

    static int getArmorDamageReductionAdjustment(const Armor& armor, int rawDamage);
    static int getArmorEscapeModifier(const Armor& armor);
};

#endif
