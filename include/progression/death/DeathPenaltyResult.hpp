// English: This file prepares the future non-lethal death penalty report.
// Français : Ce fichier prépare le futur rapport de pénalité après une mort non définitive.

#ifndef INCLUDE_PROGRESSION_DEATH_DEATHPENALTYRESULT_HPP
#define INCLUDE_PROGRESSION_DEATH_DEATHPENALTYRESULT_HPP

class DeathPenaltyResult
{
public:
    DeathPenaltyResult();

    int getLostGold() const;
    int getLostExperience() const;
    int getLostConsumables() const;
    int getWeaponDurabilityLost() const;
    int getArmorDurabilityLost() const;
    int getRecoveredMaterialFragments() const;

    bool wasWeaponBroken() const;
    bool wasArmorBroken() const;
    bool wasWeaponIrreparable() const;
    bool wasArmorIrreparable() const;
    bool wasWeaponStolen() const;
    bool wasArmorStolen() const;

    void addLostGold(int amount);
    void addLostExperience(int amount);
    void addLostConsumables(int amount);
    void addWeaponDurabilityLost(int amount);
    void addArmorDurabilityLost(int amount);
    void addRecoveredMaterialFragments(int amount);

    void markWeaponBroken();
    void markArmorBroken();
    void markWeaponIrreparable();
    void markArmorIrreparable();
    void markWeaponStolen();
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
