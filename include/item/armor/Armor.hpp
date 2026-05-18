// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#ifndef INCLUDE_ITEM_ARMOR_ARMOR_HPP
#define INCLUDE_ITEM_ARMOR_ARMOR_HPP

#include "item/Item.hpp"
#include "item/armor/ArmorType.hpp"

class Armor : public Item
{
private:
    ArmorType type;

    int maxHpBonus;
    int damageReduction;

    int durability;
    int maxDurability;

public:
    Armor();

    Armor(
        const std::string& name,
        const std::string& description,
        int value,
        ArmorType type,
        int maxHpBonus,
        int damageReduction,
        int maxDurability
    );

    ArmorType getType() const;

    int getMaxHpBonus() const;
    int getDamageReduction() const;

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
