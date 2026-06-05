// EN: DurabilityRules.hpp briefly defines this Dinotofu module and its responsibilities.
// FR: DurabilityRules.hpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file belongs to Dinotofu. Code identifiers are written in English; player-facing text can stay in French.
// Français : Ce fichier appartient à Dinotofu. Les identifiants du code sont en anglais ; les textes affichés au joueur peuvent rester en français.
// Description: Lightweight durability and equipment-fit warnings used before the full material/size system.

#ifndef INCLUDE_ITEM_DURABILITY_DURABILITYRULES_HPP
#define INCLUDE_ITEM_DURABILITY_DURABILITYRULES_HPP

#include "character/CharacterRace.hpp"
#include "item/armor/Armor.hpp"
#include "item/weapon/Weapon.hpp"

#include <string>
#include <vector>

class DurabilityRules
{
public:
    static std::string weaponWearStateText(const Weapon& weapon);
    static std::string armorWearStateText(const Armor& armor);

    static std::vector<std::string> describeWeaponUseWarnings(const Weapon& weapon, CharacterRace race);
    static std::vector<std::string> describeArmorFitWarnings(const Armor& armor, CharacterRace race);

    static std::vector<std::string> describeWeaponMaintenanceAdvice(const Weapon& weapon, CharacterRace race);
    static std::vector<std::string> describeArmorMaintenanceAdvice(const Armor& armor, CharacterRace race);
};

#endif
