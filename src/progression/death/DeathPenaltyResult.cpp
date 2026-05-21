// EN: DeathPenaltyResult.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: DeathPenaltyResult.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file prepares the future non-lethal death penalty report.
// Français : Ce fichier prépare le futur rapport de pénalité après une mort non définitive.

#include "progression/death/DeathPenaltyResult.hpp"

DeathPenaltyResult::DeathPenaltyResult()
    : lostGold(0),
      lostExperience(0),
      lostConsumables(0),
      weaponDurabilityLost(0),
      armorDurabilityLost(0),
      recoveredMaterialFragments(0),
      weaponBroken(false),
      armorBroken(false),
      weaponIrreparable(false),
      armorIrreparable(false),
      weaponStolen(false),
      // EN: armorStolen declares or implements a focused behavior used by this module.
      // FR: armorStolen déclare ou implémente un comportement précis utilisé par ce module.
      armorStolen(false)
{
}

// EN: getLostGold declares or implements a focused behavior used by this module.
// FR: getLostGold déclare ou implémente un comportement précis utilisé par ce module.
int DeathPenaltyResult::getLostGold() const { return lostGold; }
// EN: getLostExperience declares or implements a focused behavior used by this module.
// FR: getLostExperience déclare ou implémente un comportement précis utilisé par ce module.
int DeathPenaltyResult::getLostExperience() const { return lostExperience; }
// EN: getLostConsumables declares or implements a focused behavior used by this module.
// FR: getLostConsumables déclare ou implémente un comportement précis utilisé par ce module.
int DeathPenaltyResult::getLostConsumables() const { return lostConsumables; }
// EN: getWeaponDurabilityLost declares or implements a focused behavior used by this module.
// FR: getWeaponDurabilityLost déclare ou implémente un comportement précis utilisé par ce module.
int DeathPenaltyResult::getWeaponDurabilityLost() const { return weaponDurabilityLost; }
// EN: getArmorDurabilityLost declares or implements a focused behavior used by this module.
// FR: getArmorDurabilityLost déclare ou implémente un comportement précis utilisé par ce module.
int DeathPenaltyResult::getArmorDurabilityLost() const { return armorDurabilityLost; }
// EN: getRecoveredMaterialFragments declares or implements a focused behavior used by this module.
// FR: getRecoveredMaterialFragments déclare ou implémente un comportement précis utilisé par ce module.
int DeathPenaltyResult::getRecoveredMaterialFragments() const { return recoveredMaterialFragments; }

// EN: wasWeaponBroken declares or implements a focused behavior used by this module.
// FR: wasWeaponBroken déclare ou implémente un comportement précis utilisé par ce module.
bool DeathPenaltyResult::wasWeaponBroken() const { return weaponBroken; }
// EN: wasArmorBroken declares or implements a focused behavior used by this module.
// FR: wasArmorBroken déclare ou implémente un comportement précis utilisé par ce module.
bool DeathPenaltyResult::wasArmorBroken() const { return armorBroken; }
// EN: wasWeaponIrreparable declares or implements a focused behavior used by this module.
// FR: wasWeaponIrreparable déclare ou implémente un comportement précis utilisé par ce module.
bool DeathPenaltyResult::wasWeaponIrreparable() const { return weaponIrreparable; }
// EN: wasArmorIrreparable declares or implements a focused behavior used by this module.
// FR: wasArmorIrreparable déclare ou implémente un comportement précis utilisé par ce module.
bool DeathPenaltyResult::wasArmorIrreparable() const { return armorIrreparable; }
// EN: wasWeaponStolen declares or implements a focused behavior used by this module.
// FR: wasWeaponStolen déclare ou implémente un comportement précis utilisé par ce module.
bool DeathPenaltyResult::wasWeaponStolen() const { return weaponStolen; }
// EN: wasArmorStolen declares or implements a focused behavior used by this module.
// FR: wasArmorStolen déclare ou implémente un comportement précis utilisé par ce module.
bool DeathPenaltyResult::wasArmorStolen() const { return armorStolen; }

// EN: addLostGold declares or implements a focused behavior used by this module.
// FR: addLostGold déclare ou implémente un comportement précis utilisé par ce module.
void DeathPenaltyResult::addLostGold(int amount)
{
    if (amount > 0) { lostGold += amount; }
}

// EN: addLostExperience declares or implements a focused behavior used by this module.
// FR: addLostExperience déclare ou implémente un comportement précis utilisé par ce module.
void DeathPenaltyResult::addLostExperience(int amount)
{
    if (amount > 0) { lostExperience += amount; }
}

// EN: addLostConsumables declares or implements a focused behavior used by this module.
// FR: addLostConsumables déclare ou implémente un comportement précis utilisé par ce module.
void DeathPenaltyResult::addLostConsumables(int amount)
{
    if (amount > 0) { lostConsumables += amount; }
}

// EN: addWeaponDurabilityLost declares or implements a focused behavior used by this module.
// FR: addWeaponDurabilityLost déclare ou implémente un comportement précis utilisé par ce module.
void DeathPenaltyResult::addWeaponDurabilityLost(int amount)
{
    if (amount > 0) { weaponDurabilityLost += amount; }
}

// EN: addArmorDurabilityLost declares or implements a focused behavior used by this module.
// FR: addArmorDurabilityLost déclare ou implémente un comportement précis utilisé par ce module.
void DeathPenaltyResult::addArmorDurabilityLost(int amount)
{
    if (amount > 0) { armorDurabilityLost += amount; }
}

// EN: addRecoveredMaterialFragments declares or implements a focused behavior used by this module.
// FR: addRecoveredMaterialFragments déclare ou implémente un comportement précis utilisé par ce module.
void DeathPenaltyResult::addRecoveredMaterialFragments(int amount)
{
    if (amount > 0) { recoveredMaterialFragments += amount; }
}

void DeathPenaltyResult::markWeaponBroken() { weaponBroken = true; }
void DeathPenaltyResult::markArmorBroken() { armorBroken = true; }
void DeathPenaltyResult::markWeaponIrreparable() { weaponIrreparable = true; }
void DeathPenaltyResult::markArmorIrreparable() { armorIrreparable = true; }
void DeathPenaltyResult::markWeaponStolen() { weaponStolen = true; }
void DeathPenaltyResult::markArmorStolen() { armorStolen = true; }
