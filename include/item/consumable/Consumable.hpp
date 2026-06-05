// EN: Consumable.hpp briefly defines this Dinotofu module and its responsibilities.
// FR: Consumable.hpp résume brièvement ce module de Dinotofu et ses responsabilités.
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
    bool percentageBasedHealing;

public:
    // EN: Consumable declares or implements a focused behavior used by this module.
    // FR: Consumable déclare ou implémente un comportement précis utilisé par ce module.
    Consumable();

    Consumable(
        const std::string& name,
        const std::string& description,
        int value,
        ConsumableType type,
        int power,
        bool percentageBasedHealing = false
    );

    // EN: getType declares or implements a focused behavior used by this module.
    // FR: getType déclare ou implémente un comportement précis utilisé par ce module.
    ConsumableType getType() const;
    // EN: getPower declares or implements a focused behavior used by this module.
    // FR: getPower déclare ou implémente un comportement précis utilisé par ce module.
    int getPower() const;
    bool isPercentageBasedHealing() const;
    int getHealingAmountForMaxHp(int maxHp) const;
    std::string getPowerDisplayText() const;

    // EN: isHealing declares or implements a focused behavior used by this module.
    // FR: isHealing déclare ou implémente un comportement précis utilisé par ce module.
    bool isHealing() const;
    // EN: isDamage declares or implements a focused behavior used by this module.
    // FR: isDamage déclare ou implémente un comportement précis utilisé par ce module.
    bool isDamage() const;

    std::vector<std::string> toDisplayLines() const override;

    // EN: display declares or implements a focused behavior used by this module.
    // FR: display déclare ou implémente un comportement précis utilisé par ce module.
    void display() const override;
};

#endif
