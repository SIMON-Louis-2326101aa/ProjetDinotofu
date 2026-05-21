// EN: Armor.hpp briefly defines this Dinotofu module and its responsibilities.
// FR: Armor.hpp résume brièvement ce module de Dinotofu et ses responsabilités.
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
    // EN: Armor declares or implements a focused behavior used by this module.
    // FR: Armor déclare ou implémente un comportement précis utilisé par ce module.
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

    // EN: getType declares or implements a focused behavior used by this module.
    // FR: getType déclare ou implémente un comportement précis utilisé par ce module.
    ArmorType getType() const;

    // EN: getMaxHpBonus declares or implements a focused behavior used by this module.
    // FR: getMaxHpBonus déclare ou implémente un comportement précis utilisé par ce module.
    int getMaxHpBonus() const;
    // EN: getDamageReduction declares or implements a focused behavior used by this module.
    // FR: getDamageReduction déclare ou implémente un comportement précis utilisé par ce module.
    int getDamageReduction() const;

    // EN: getDurability declares or implements a focused behavior used by this module.
    // FR: getDurability déclare ou implémente un comportement précis utilisé par ce module.
    int getDurability() const;
    // EN: getMaxDurability declares or implements a focused behavior used by this module.
    // FR: getMaxDurability déclare ou implémente un comportement précis utilisé par ce module.
    int getMaxDurability() const;

    // EN: isBroken declares or implements a focused behavior used by this module.
    // FR: isBroken déclare ou implémente un comportement précis utilisé par ce module.
    bool isBroken() const;
    // EN: isIndestructible declares or implements a focused behavior used by this module.
    // FR: isIndestructible déclare ou implémente un comportement précis utilisé par ce module.
    bool isIndestructible() const;

    // EN: loseDurability declares or implements a focused behavior used by this module.
    // FR: loseDurability déclare ou implémente un comportement précis utilisé par ce module.
    void loseDurability(int amount);
    // EN: repair declares or implements a focused behavior used by this module.
    // FR: repair déclare ou implémente un comportement précis utilisé par ce module.
    void repair(int amount);
    // EN: fullyRepair declares or implements a focused behavior used by this module.
    // FR: fullyRepair déclare ou implémente un comportement précis utilisé par ce module.
    void fullyRepair();

    // EN: display declares or implements a focused behavior used by this module.
    // FR: display déclare ou implémente un comportement précis utilisé par ce module.
    void display() const override;
};

#endif
