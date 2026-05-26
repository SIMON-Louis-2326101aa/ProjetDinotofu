// EN: CombatClassSystem.hpp briefly defines this Dinotofu module and its responsibilities.
// FR: CombatClassSystem.hpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#ifndef INCLUDE_COMBAT_SYSTEM_COMBATCLASSSYSTEM_HPP
#define INCLUDE_COMBAT_SYSTEM_COMBATCLASSSYSTEM_HPP

#include "entity/Entity.hpp"
#include "item/weapon/WeaponType.hpp"

#include <string>

class CombatClassSystem
{
public:
    // EN: getBaseEscapeChance declares or implements a focused behavior used by this module.
    // FR: getBaseEscapeChance déclare ou implémente un comportement précis utilisé par ce module.
    static int getBaseEscapeChance(const Entity& entity);
    // EN: getBaseDamageReductionPercentage declares or implements a focused behavior used by this module.
    // FR: getBaseDamageReductionPercentage déclare ou implémente un comportement précis utilisé par ce module.
    static int getBaseDamageReductionPercentage(const Entity& entity);
    static int getOutgoingDamagePercent(const Entity& entity);
    static int getOutgoingFlatBonus(const Entity& entity);
    static std::string normalizeClassText(const std::string& classText);
    static bool hasWeaponAffinity(const Entity& entity, WeaponType weaponType, const std::string& weaponName);
    static int getWeaponAffinityDamageBonus(const Entity& entity, WeaponType weaponType, const std::string& weaponName, int currentDamage);
    static std::string getWeaponAffinityLabel(const Entity& entity, WeaponType weaponType, const std::string& weaponName);

private:
};

#endif
