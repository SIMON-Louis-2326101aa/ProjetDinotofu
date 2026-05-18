// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#ifndef INCLUDE_ITEM_CONSUMABLE_CONSUMABLE_HPP
#define INCLUDE_ITEM_CONSUMABLE_CONSUMABLE_HPP

#include "item/Item.hpp"
#include "item/consumable/ConsumableType.hpp"

class Consumable : public Item
{
private:
    ConsumableType type;
    int power;

public:
    Consumable();

    Consumable(
        const std::string& name,
        const std::string& description,
        int value,
        ConsumableType type,
        int power
    );

    ConsumableType getType() const;
    int getPower() const;

    bool isHealing() const;
    bool isDamage() const;

    void display() const override;
};

#endif
