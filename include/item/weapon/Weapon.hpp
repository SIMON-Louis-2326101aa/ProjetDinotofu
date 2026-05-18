// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#ifndef INCLUDE_ITEM_WEAPON_WEAPON_HPP
#define INCLUDE_ITEM_WEAPON_WEAPON_HPP

#include "item/Item.hpp"
#include "item/weapon/WeaponType.hpp"

class Weapon : public Item
{
private:
    WeaponType type;

    int minDamageBonus;
    int maxDamageBonus;
    int criticalBonus;

    int durability;
    int maxDurability;

public:
    Weapon();

    Weapon(
        const std::string& name,
        const std::string& description,
        int value,
        WeaponType type,
        int minDamageBonus,
        int maxDamageBonus,
        int criticalBonus,
        int maxDurability
    );

    WeaponType getType() const;

    int getMinDamageBonus() const;
    int getMaxDamageBonus() const;
    int getCriticalBonus() const;

    int getDurability() const;
    int getMaxDurability() const;

    bool isBroken() const;
    bool isIndestructible() const;

    void loseDurability(int amount);
    void repair(int amount);
    void fullyRepair();

    void display() const override;
};

#endif
