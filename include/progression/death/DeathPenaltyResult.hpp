// EN: DeathPenaltyResult.hpp briefly defines this Dinotofu module and its responsibilities.
// FR: DeathPenaltyResult.hpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file prepares the future non-lethal death penalty report.
// Français : Ce fichier prépare le futur rapport de pénalité après une mort non définitive.

#ifndef INCLUDE_PROGRESSION_DEATH_DEATHPENALTYRESULT_HPP
#define INCLUDE_PROGRESSION_DEATH_DEATHPENALTYRESULT_HPP

class DeathPenaltyResult
{
public:
    // EN: DeathPenaltyResult declares or implements a focused behavior used by this module.
    // FR: DeathPenaltyResult déclare ou implémente un comportement précis utilisé par ce module.
    DeathPenaltyResult();

    // EN: getLostGold declares or implements a focused behavior used by this module.
    // FR: getLostGold déclare ou implémente un comportement précis utilisé par ce module.
    int getLostGold() const;
    // EN: getLostExperience declares or implements a focused behavior used by this module.
    // FR: getLostExperience déclare ou implémente un comportement précis utilisé par ce module.
    int getLostExperience() const;
    // EN: getLostConsumables declares or implements a focused behavior used by this module.
    // FR: getLostConsumables déclare ou implémente un comportement précis utilisé par ce module.
    int getLostConsumables() const;
    // EN: getWeaponDurabilityLost declares or implements a focused behavior used by this module.
    // FR: getWeaponDurabilityLost déclare ou implémente un comportement précis utilisé par ce module.
    int getWeaponDurabilityLost() const;
    // EN: getArmorDurabilityLost declares or implements a focused behavior used by this module.
    // FR: getArmorDurabilityLost déclare ou implémente un comportement précis utilisé par ce module.
    int getArmorDurabilityLost() const;
    // EN: getRecoveredMaterialFragments declares or implements a focused behavior used by this module.
    // FR: getRecoveredMaterialFragments déclare ou implémente un comportement précis utilisé par ce module.
    int getRecoveredMaterialFragments() const;

    // EN: wasWeaponBroken declares or implements a focused behavior used by this module.
    // FR: wasWeaponBroken déclare ou implémente un comportement précis utilisé par ce module.
    bool wasWeaponBroken() const;
    // EN: wasArmorBroken declares or implements a focused behavior used by this module.
    // FR: wasArmorBroken déclare ou implémente un comportement précis utilisé par ce module.
    bool wasArmorBroken() const;
    // EN: wasWeaponIrreparable declares or implements a focused behavior used by this module.
    // FR: wasWeaponIrreparable déclare ou implémente un comportement précis utilisé par ce module.
    bool wasWeaponIrreparable() const;
    // EN: wasArmorIrreparable declares or implements a focused behavior used by this module.
    // FR: wasArmorIrreparable déclare ou implémente un comportement précis utilisé par ce module.
    bool wasArmorIrreparable() const;
    // EN: wasWeaponStolen declares or implements a focused behavior used by this module.
    // FR: wasWeaponStolen déclare ou implémente un comportement précis utilisé par ce module.
    bool wasWeaponStolen() const;
    // EN: wasArmorStolen declares or implements a focused behavior used by this module.
    // FR: wasArmorStolen déclare ou implémente un comportement précis utilisé par ce module.
    bool wasArmorStolen() const;

    // EN: addLostGold declares or implements a focused behavior used by this module.
    // FR: addLostGold déclare ou implémente un comportement précis utilisé par ce module.
    void addLostGold(int amount);
    // EN: addLostExperience declares or implements a focused behavior used by this module.
    // FR: addLostExperience déclare ou implémente un comportement précis utilisé par ce module.
    void addLostExperience(int amount);
    // EN: addLostConsumables declares or implements a focused behavior used by this module.
    // FR: addLostConsumables déclare ou implémente un comportement précis utilisé par ce module.
    void addLostConsumables(int amount);
    // EN: addWeaponDurabilityLost declares or implements a focused behavior used by this module.
    // FR: addWeaponDurabilityLost déclare ou implémente un comportement précis utilisé par ce module.
    void addWeaponDurabilityLost(int amount);
    // EN: addArmorDurabilityLost declares or implements a focused behavior used by this module.
    // FR: addArmorDurabilityLost déclare ou implémente un comportement précis utilisé par ce module.
    void addArmorDurabilityLost(int amount);
    // EN: addRecoveredMaterialFragments declares or implements a focused behavior used by this module.
    // FR: addRecoveredMaterialFragments déclare ou implémente un comportement précis utilisé par ce module.
    void addRecoveredMaterialFragments(int amount);

    // EN: markWeaponBroken declares or implements a focused behavior used by this module.
    // FR: markWeaponBroken déclare ou implémente un comportement précis utilisé par ce module.
    void markWeaponBroken();
    // EN: markArmorBroken declares or implements a focused behavior used by this module.
    // FR: markArmorBroken déclare ou implémente un comportement précis utilisé par ce module.
    void markArmorBroken();
    // EN: markWeaponIrreparable declares or implements a focused behavior used by this module.
    // FR: markWeaponIrreparable déclare ou implémente un comportement précis utilisé par ce module.
    void markWeaponIrreparable();
    // EN: markArmorIrreparable declares or implements a focused behavior used by this module.
    // FR: markArmorIrreparable déclare ou implémente un comportement précis utilisé par ce module.
    void markArmorIrreparable();
    // EN: markWeaponStolen declares or implements a focused behavior used by this module.
    // FR: markWeaponStolen déclare ou implémente un comportement précis utilisé par ce module.
    void markWeaponStolen();
    // EN: markArmorStolen declares or implements a focused behavior used by this module.
    // FR: markArmorStolen déclare ou implémente un comportement précis utilisé par ce module.
    void markArmorStolen();

private:
    int lostGold;
    int lostExperience;
    int lostConsumables;
    int weaponDurabilityLost;
    int armorDurabilityLost;
    int recoveredMaterialFragments;

    bool weaponBroken;
    bool armorBroken;
    bool weaponIrreparable;
    bool armorIrreparable;
    bool weaponStolen;
    bool armorStolen;
};

#endif
