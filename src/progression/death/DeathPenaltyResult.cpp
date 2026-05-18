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
      armorStolen(false)
{
}

int DeathPenaltyResult::getLostGold() const { return lostGold; }
int DeathPenaltyResult::getLostExperience() const { return lostExperience; }
int DeathPenaltyResult::getLostConsumables() const { return lostConsumables; }
int DeathPenaltyResult::getWeaponDurabilityLost() const { return weaponDurabilityLost; }
int DeathPenaltyResult::getArmorDurabilityLost() const { return armorDurabilityLost; }
int DeathPenaltyResult::getRecoveredMaterialFragments() const { return recoveredMaterialFragments; }

bool DeathPenaltyResult::wasWeaponBroken() const { return weaponBroken; }
bool DeathPenaltyResult::wasArmorBroken() const { return armorBroken; }
bool DeathPenaltyResult::wasWeaponIrreparable() const { return weaponIrreparable; }
bool DeathPenaltyResult::wasArmorIrreparable() const { return armorIrreparable; }
bool DeathPenaltyResult::wasWeaponStolen() const { return weaponStolen; }
bool DeathPenaltyResult::wasArmorStolen() const { return armorStolen; }

void DeathPenaltyResult::addLostGold(int amount)
{
    if (amount > 0) { lostGold += amount; }
}

void DeathPenaltyResult::addLostExperience(int amount)
{
    if (amount > 0) { lostExperience += amount; }
}

void DeathPenaltyResult::addLostConsumables(int amount)
{
    if (amount > 0) { lostConsumables += amount; }
}

void DeathPenaltyResult::addWeaponDurabilityLost(int amount)
{
    if (amount > 0) { weaponDurabilityLost += amount; }
}

void DeathPenaltyResult::addArmorDurabilityLost(int amount)
{
    if (amount > 0) { armorDurabilityLost += amount; }
}

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
